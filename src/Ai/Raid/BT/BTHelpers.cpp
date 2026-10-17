/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "CellImpl.h"
#include "DynamicObject.h"
#include "EncounterHelpers.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PathGenerator.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <mutex>
#include <tuple>

using namespace EncounterHelpers;

namespace BtHelpers
{

namespace
{

std::mutex btStateMutex;
std::unordered_map<uint32, BtInstanceState> btStates;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>> reaverAbsorptionStarts;

Player* GetCachedPlayer(PlayerbotAI* botAI, char const* value)
{
    Unit* unit = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>(value)->Get());
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return player && player->IsAlive() ? player : nullptr;
}

// A point distance yd from Illidan and angle off the line from him through the grate centre,
// positive toward the warlock tank's side.
Position GetDemonFormPoint(
    Position const& illidan, float inwardAngle, float distance, float angle)
{
    float const bearing = inwardAngle - angle;
    return Position(
        illidan.GetPositionX() + distance * std::cos(bearing),
        illidan.GetPositionY() + distance * std::sin(bearing), illidan.GetPositionZ());
}

// One row of Demon Form slots 6 yd apart, from highAngle down, each with its angle. Kept only
// 22 yd or more from the warlock position (Shadow Blast hits 20 yd round its victim) and within
// 36 yd of the grate centre.
std::vector<std::pair<float, Position>> GetDemonFormRow(
    Position const& illidan, float inwardAngle, Position const& warlockTankPosition, float distance,
    float highAngle, float lowAngle, bool halfStepIn)
{
    constexpr float slotSpacing = 6.0f;
    constexpr float warlockClearance = 22.0f;
    constexpr float maxCentreDistance = 36.0f;

    float const step = slotSpacing / distance;
    float const firstAngle = halfStepIn ? highAngle - step / 2.0f : highAngle;
    int32 const count = static_cast<int32>(std::floor((firstAngle - lowAngle) / step)) + 1;

    std::vector<std::pair<float, Position>> row;
    for (int32 i = 0; i < count; ++i)
    {
        float const angle = firstAngle - i * step;
        Position const point = GetDemonFormPoint(illidan, inwardAngle, distance, angle);
        if (point.GetExactDist2d(warlockTankPosition) >= warlockClearance &&
            ILLIDAN_GRATE_CENTRE.GetExactDist2d(point) <= maxCentreDistance)
        {
            row.emplace_back(angle, point);
        }
    }

    return row;
}

// The place of guid in first and then second, each sorted by GUID; -1 if in neither.
int32 GetRosterIndex(
    std::vector<ObjectGuid> first, std::vector<ObjectGuid> second, ObjectGuid guid)
{
    std::sort(first.begin(), first.end());
    std::sort(second.begin(), second.end());
    first.insert(first.end(), second.begin(), second.end());

    auto const it = std::find(first.begin(), first.end(), guid);
    return it == first.end() ? -1 : static_cast<int32>(std::distance(first.begin(), it));
}

// Dead or alive; null if no group member has the job (an offline tank among them).
Player* FindFlameOfAzzinothTank(Player* bot, bool isEast)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && (isEast ? IsEastFlameOfAzzinothTank(member) :
                                IsWestFlameOfAzzinothTank(member)))
        {
            return member;
        }
    }

    return nullptr;
}

}

// Shared

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell(Id(BtSpells::SPELL_MISDIRECTION), tank))
        return botAI->CastSpell(Id(BtSpells::SPELL_MISDIRECTION), tank);

    if (!botAI->GetBot()->HasAura(Id(BtSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

bool IsInRectangle(
    Position const& point, Position const& centre, Position const& facing, float halfWidth,
    float halfDepth)
{
    float const angle = centre.GetAngle(&facing);
    float const dX = point.GetPositionX() - centre.GetPositionX();
    float const dY = point.GetPositionY() - centre.GetPositionY();
    float const depth = dX * std::cos(angle) + dY * std::sin(angle);
    float const width = -dX * std::sin(angle) + dY * std::cos(angle);
    return std::fabs(depth) <= halfDepth && std::fabs(width) <= halfWidth;
}

Position GetBotPointInRectangle(
    Player* bot, Position const& centre, Position const& facing, float halfWidth, float halfDepth)
{
    // Keeps points off the edge, so a bot that arrives counts as inside.
    constexpr float edgeInset = 0.5f;

    // splitmix64 on the GUID: two independent values in [-1, 1].
    uint64 hash = bot->GetGUID().GetRawValue() + 0x9E3779B97F4A7C15ULL;
    hash = (hash ^ (hash >> 30)) * 0xBF58476D1CE4E5B9ULL;
    hash = (hash ^ (hash >> 27)) * 0x94D049BB133111EBULL;
    hash ^= hash >> 31;
    float const widthFraction = static_cast<float>(hash & 0xFFFF) / 0xFFFF * 2.0f - 1.0f;
    float const depthFraction = static_cast<float>((hash >> 16) & 0xFFFF) / 0xFFFF * 2.0f - 1.0f;

    float const angle = centre.GetAngle(&facing);
    float const width = widthFraction * std::max(halfWidth - edgeInset, 0.0f);
    float const depth = depthFraction * std::max(halfDepth - edgeInset, 0.0f);
    float x = centre.GetPositionX() + depth * std::cos(angle) - width * std::sin(angle);
    float y = centre.GetPositionY() + depth * std::sin(angle) + width * std::cos(angle);
    float z = bot->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, centre.GetPositionX(), centre.GetPositionY(), bot->GetPositionZ(), x, y, z))
    {
        return Position(centre.GetPositionX(), centre.GetPositionY(), bot->GetPositionZ());
    }

    return Position(x, y, bot->GetPositionZ());
}

// Trash

namespace
{

bool IsMagicDamageSpell(SpellInfo const* spellInfo)
{
    return (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC ||
            spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE) &&
        (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC);
}

// Walks the spell and the spells it triggers. Spell Absorption's charge proc ignores triggered
// spells unless they carry NOT_A_PROC, and ignores periodic ticks. A dummy effect on enemies counts
// whatever its flags, because a script picks the spell that follows it (Death Coil, Holy Shock,
// Penance, Starfall).
bool CanBuildChaoticCharge(SpellInfo const* spellInfo, bool triggered, uint8 depth, bool& area)
{
    constexpr uint8 maxDepth = 3;
    if (!spellInfo || depth > maxDepth)
        return false;

    bool const canProc = !triggered || spellInfo->HasAttribute(SPELL_ATTR3_NOT_A_PROC);
    bool builds = false;
    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        uint32 triggerSpell = 0;
        switch (effect.Effect)
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_POWER_BURN:
                if (canProc && IsMagicDamageSpell(spellInfo))
                {
                    builds = true;
                    area |= effect.IsTargetingArea() || effect.ChainTarget > 1;
                }
                break;
            // Wands: magic class, with the school taken from the wand.
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                if (canProc && spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                    builds = true;
                break;
            case SPELL_EFFECT_DUMMY:
                if (!IsMagicDamageSpell(spellInfo))
                    break;

                if (effect.IsTargetingArea())
                {
                    builds = true;
                    area = true;
                }
                else if (effect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY ||
                         effect.TargetA.GetTarget() == TARGET_UNIT_TARGET_ANY)
                {
                    builds = true;
                }
                break;
            case SPELL_EFFECT_TRIGGER_SPELL:
            case SPELL_EFFECT_TRIGGER_MISSILE:
                triggerSpell = effect.TriggerSpell;
                break;
            case SPELL_EFFECT_APPLY_AURA:
            case SPELL_EFFECT_PERSISTENT_AREA_AURA:
                if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    triggerSpell = effect.TriggerSpell;
                }
                break;
            default:
                break;
        }

        bool triggeredArea = false;
        if (triggerSpell && CanBuildChaoticCharge(
                sSpellMgr->GetSpellInfo(triggerSpell), true, depth + 1, triggeredArea))
        {
            builds = true;
            area |= triggeredArea;
        }
    }

    return builds;
}

} // end anonymous namespace (Trash)

Unit* GetLinkedSisterOfPain(Unit* pleasure)
{
    if (!pleasure || pleasure->GetEntry() != Id(BtNpcs::NPC_SISTER_OF_PLEASURE) ||
        !pleasure->IsAlive())
    {
        return nullptr;
    }

    Aura* bonds = pleasure->GetAura(Id(BtSpells::SPELL_SHARED_BONDS));
    Unit* pain = bonds ? bonds->GetCaster() : nullptr;
    return pain && pain->IsAlive() ? pain : nullptr;
}

GuidVector FindLinkedSisterOfPleasureGuids(PlayerbotAI* botAI)
{
    std::list<Creature*> sisters;
    botAI->GetBot()->GetCreatureListWithEntryInGrid(
        sisters, Id(BtNpcs::NPC_SISTER_OF_PLEASURE), SISTER_OF_PLEASURE_SEARCH_RADIUS);

    GuidVector pleasures;
    for (Creature* sister : sisters)
    {
        if (GetLinkedSisterOfPain(sister))
            pleasures.push_back(sister->GetGUID());
    }

    return pleasures;
}

Unit* GetLinkedSisterOfPleasure(PlayerbotAI* botAI)
{
    auto const& pleasures =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("linked sisters of pleasure")->RefGet();
    for (ObjectGuid const& guid : pleasures)
    {
        Unit* pleasure = botAI->GetUnit(guid);
        if (GetLinkedSisterOfPain(pleasure))
            return pleasure;
    }

    return nullptr;
}

bool IsLinkedSisterOfPain(PlayerbotAI* botAI, Unit* pain)
{
    if (!pain || pain->GetEntry() != Id(BtNpcs::NPC_SISTER_OF_PAIN))
        return false;

    auto const& pleasures =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("linked sisters of pleasure")->RefGet();
    return std::any_of(
        pleasures.begin(), pleasures.end(),
        [botAI, pain](ObjectGuid const& guid)
        { return GetLinkedSisterOfPain(botAI->GetUnit(guid)) == pain; });
}

GuidVector FindShadowmoonReaverGuids(PlayerbotAI* botAI)
{
    GuidVector reavers;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == Id(BtNpcs::NPC_SHADOWMOON_REAVER))
        {
            reavers.push_back(guid);
        }
    }

    return reavers;
}

bool IsShadowmoonReaverUnsafeForMagic(Unit* unit)
{
    if (!unit || unit->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER) ||
        !unit->IsAlive())
    {
        return false;
    }

    ObjectGuid const guid = unit->GetGUID();
    uint32 const instanceId = unit->GetInstanceId();
    std::lock_guard lock(btStateMutex);
    if (Aura* absorption = unit->GetAura(Id(BtSpells::SPELL_SPELL_ABSORPTION)))
    {
        // Reconstructed from the aura, so every caller stamps the same start.
        uint32 const elapsed =
            static_cast<uint32>(absorption->GetMaxDuration() - absorption->GetDuration());
        reaverAbsorptionStarts[instanceId][guid] = getMSTime() - elapsed;
        return true;
    }

    auto const instanceIt = reaverAbsorptionStarts.find(instanceId);
    if (instanceIt == reaverAbsorptionStarts.end())
        return true;

    auto const it = instanceIt->second.find(guid);
    if (it == instanceIt->second.end())
        return true;

    uint32 const sinceStart = GetMSTimeDiffToNow(it->second);
    return sinceStart < REAVER_ABSORPTION_DURATION_MS ||
        sinceStart >= REAVER_ABSORPTION_MIN_RECAST_MS - REAVER_MAGIC_MARGIN_MS;
}

bool IsAnyShadowmoonReaverUnsafeForMagic(PlayerbotAI* botAI)
{
    auto const& reavers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    for (ObjectGuid const& guid : reavers)
    {
        if (IsShadowmoonReaverUnsafeForMagic(botAI->GetUnit(guid)))
            return true;
    }

    return false;
}

bool PruneShadowmoonReaverAbsorptionStarts(Player* bot)
{
    std::lock_guard lock(btStateMutex);
    auto const reaversIt = reaverAbsorptionStarts.find(bot->GetInstanceId());
    if (reaversIt == reaverAbsorptionStarts.end())
        return false;

    bool const pruned = std::erase_if(
        reaversIt->second,
        [bot](auto const& entry)
        {
            Unit* reaver = ObjectAccessor::GetUnit(*bot, entry.first);
            return !reaver || !reaver->IsAlive();
        }) > 0;

    if (reaversIt->second.empty())
        reaverAbsorptionStarts.erase(reaversIt);

    return pruned;
}

ChaoticChargeReach GetChaoticChargeReach(SpellInfo const* spellInfo)
{
    bool area = false;
    if (!CanBuildChaoticCharge(spellInfo, false, 0, area))
        return ChaoticChargeReach::None;

    return area ? ChaoticChargeReach::Area : ChaoticChargeReach::Target;
}

bool IsChargeBuildingPet(Unit* unit)
{
    if (!unit)
        return false;

    switch (unit->GetEntry())
    {
        case NPC_IMP:
        case NPC_FELHUNTER:
        case NPC_SUCCUBUS:
        case NPC_WATER_ELEMENTAL_TEMP:
        case NPC_WATER_ELEMENTAL_PERM:
            return true;
        default:
            return false;
    }
}

Unit* FindPetTargetOtherThanReaver(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const isAllowed = [bot](Unit* unit)
    {
        return unit && unit->IsAlive() &&
            unit->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER) &&
            bot->IsValidAttackTarget(unit);
    };

    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (isAllowed(currentTarget))
        return currentTarget;

    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (isAllowed(unit))
            return unit;
    }

    return nullptr;
}

// High Warlord Naj'entus

namespace // High Warlord Naj'entus
{

// A remover counts while it is alive, on the map and free to move.
bool IsValidSpineRemover(Player* bot, ObjectGuid remover)
{
    Player* player = ObjectAccessor::GetPlayer(*bot, remover);
    return player && player->IsAlive() && player->GetMapId() == BT_MAP_ID &&
        !IsNajentusImpaled(player);
}

bool CanThrowNajentusSpine(Player* bot)
{
    return bot && bot->IsAlive() && bot->GetMapId() == BT_MAP_ID && !IsNajentusImpaled(bot) &&
        bot->HasItemCount(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

std::vector<NajentusSpineAssignment> const* FindNajentusSpineAssignments(uint32 instanceId)
{
    BtInstanceState const* state = FindBtState(instanceId);
    return state && state->najentusSpineAssignments ? &*state->najentusSpineAssignments : nullptr;
}

} // end anonymous namespace (High Warlord Naj'entus)

bool IsNajentusImpaled(Player* player)
{
    return player && player->IsAlive() &&
        player->HasAura(Id(BtSpells::SPELL_IMPALING_SPINE));
}

Player* FindNajentusUnassignedImpaledPlayer(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsNajentusImpaled(member))
            continue;

        bool assigned = false;
        if (assignments)
        {
            for (NajentusSpineAssignment const& assignment : *assignments)
            {
                if (assignment.impaled == member->GetGUID() &&
                    IsValidSpineRemover(bot, assignment.remover))
                {
                    assigned = true;
                    break;
                }
            }
        }

        if (!assigned)
            return member;
    }

    return nullptr;
}

Player* FindNajentusSpineRemover(Player* bot, Player* impaled)
{
    Group* group = bot->GetGroup();
    if (!group || !impaled)
        return nullptr;

    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());

    Player* remover = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == impaled || !member->IsAlive() ||
            member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            PlayerbotAI::IsTank(member) || IsNajentusImpaled(member))
        {
            continue;
        }

        if (assignments &&
            std::any_of(assignments->begin(), assignments->end(),
                        [member](NajentusSpineAssignment const& assignment)
                        { return assignment.remover == member->GetGUID(); }))
        {
            continue;
        }

        float const dist = member->GetExactDist2d(impaled);
        if (dist < closestDist)
        {
            closestDist = dist;
            remover = member;
        }
    }

    return remover;
}

Player* GetNajentusImpaledPlayerToFree(Player* bot)
{
    auto const* assignments = FindNajentusSpineAssignments(bot->GetInstanceId());
    if (!assignments)
        return nullptr;

    for (NajentusSpineAssignment const& assignment : *assignments)
    {
        if (assignment.remover != bot->GetGUID())
            continue;

        Player* impaled = ObjectAccessor::GetPlayer(*bot, assignment.impaled);
        return IsNajentusImpaled(impaled) ? impaled : nullptr;
    }

    return nullptr;
}

bool IsNajentusSpineThrower(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && state->najentusSpineThrower == bot->GetGUID();
}

Player* GetNajentusSpineThrower(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    if (!state || !state->najentusSpineThrower)
        return nullptr;

    Player* thrower = ObjectAccessor::GetPlayer(*bot, *state->najentusSpineThrower);
    return CanThrowNajentusSpine(thrower) ? thrower : nullptr;
}

Player* FindNajentusSpineThrower(Player* bot, Unit* najentus)
{
    Group* group = bot->GetGroup();
    if (!group || !najentus)
        return nullptr;

    Player* thrower = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !GET_PLAYERBOT_AI(member) || !CanThrowNajentusSpine(member))
            continue;

        float const dist = member->GetExactDist2d(najentus);
        if (dist < closestDist)
        {
            closestDist = dist;
            thrower = member;
        }
    }

    return thrower;
}

bool IsNajentusSpineInFlight(Player* bot)
{
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    return state && state->najentusSpineThrowTime &&
        GetMSTimeDiffToNow(*state->najentusSpineThrowTime) < NAJENTUS_HURL_SPINE_FLIGHT_MS;
}

// Supremus

namespace
{

float GetSupremusHazardRadius(SupremusHazard const& hazard, SupremusHazardZone zone)
{
    return zone == SupremusHazardZone::Damage ? hazard.damageRadius : hazard.safeRadius;
}

} // end anonymous namespace (Supremus)

bool IsSupremusKitePhase(Unit* supremus)
{
    return supremus && supremus->HasAura(Id(BtSpells::SPELL_SNARE_SELF));
}

float GetSupremusFixateTimeRemaining(Unit* supremus)
{
    if (!supremus)
        return 0.0f;

    Aura* snareSelf = supremus->GetAura(Id(BtSpells::SPELL_SNARE_SELF));
    if (!snareSelf)
        return 0.0f;

    int64 const elapsed =
        GameTime::GetGameTime().count() - static_cast<int64>(snareSelf->GetApplyTime());
    int64 const remaining = std::min(
        SUPREMUS_FIXATE_INTERVAL_SECONDS,
        SUPREMUS_FIXATE_INTERVAL_SECONDS + 1 - elapsed % SUPREMUS_FIXATE_INTERVAL_SECONDS);

    return static_cast<float>(remaining);
}

float GetSupremusCatchDistance(Player* bot, Unit* supremus)
{
    if (!supremus)
        return 0.0f;

    return supremus->GetMeleeRange(bot) + SUPREMUS_KITE_SAFETY_MARGIN;
}

bool CanSupremusCatchStandingBot(Player* bot, Unit* supremus)
{
    if (!supremus)
        return false;

    float const closing = supremus->GetSpeed(MOVE_RUN) * GetSupremusFixateTimeRemaining(supremus);
    return supremus->GetExactDist2d(bot) - closing < GetSupremusCatchDistance(bot, supremus);
}

bool IsSupremusVolcanoErupting(Unit* volcano)
{
    return volcano && volcano->IsAlive() &&
        (volcano->HasAura(Id(BtSpells::SPELL_VOLCANIC_GEYSER)) ||
         volcano->HasUnitState(UNIT_STATE_CASTING));
}

std::vector<SupremusHazard> FindSupremusHazards(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    std::vector<SupremusHazard> hazards;

    std::list<Creature*> volcanoes;
    bot->GetCreatureListWithEntryInGrid(
        volcanoes, Id(BtNpcs::NPC_SUPREMUS_VOLCANO), SUPREMUS_HAZARD_SEARCH_RADIUS);
    for (Creature* volcano : volcanoes)
    {
        if (IsSupremusVolcanoErupting(volcano))
        {
            hazards.push_back({
                volcano->GetPosition(), SUPREMUS_VOLCANO_HAZARD_RADIUS,
                SUPREMUS_VOLCANO_SAFE_DISTANCE });
        }
    }

    // In the tank phase stock avoid aoe handles Molten Flame; it sidesteps, keeping bots near.
    Unit* supremus =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "supremus")->Get();
    if (!IsSupremusKitePhase(supremus))
        return hazards;

    for (Position const& patch : GetDynamicObjectPositions(
             bot, SUPREMUS_HAZARD_SEARCH_RADIUS, Id(BtSpells::SPELL_MOLTEN_FLAME_PATCH)))
    {
        hazards.push_back(
            { patch, SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS, SUPREMUS_MOLTEN_FLAME_SAFE_DISTANCE });
    }

    return hazards;
}

std::vector<SupremusHazard> const& GetSupremusHazards(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<SupremusHazard>>("supremus hazards")
        ->RefGet();
}

bool IsInSupremusHazard(
    std::vector<SupremusHazard> const& hazards, float x, float y, SupremusHazardZone zone,
    float margin)
{
    for (SupremusHazard const& hazard : hazards)
    {
        if (hazard.position.GetExactDist2d(x, y) < GetSupremusHazardRadius(hazard, zone) + margin)
            return true;
    }

    return false;
}

float GetLineLengthInSupremusHazards(
    std::vector<SupremusHazard> const& hazards, Position const& from, Position const& to,
    SupremusHazardZone zone)
{
    float const length = from.GetExactDist2d(to);
    if (length <= 0.0f)
        return 0.0f;

    float const dx = (to.GetPositionX() - from.GetPositionX()) / length;
    float const dy = (to.GetPositionY() - from.GetPositionY()) / length;
    float total = 0.0f;
    for (SupremusHazard const& hazard : hazards)
    {
        // The line crosses the circle where t^2 + 2 * along * t + outside = 0.
        float const radius = GetSupremusHazardRadius(hazard, zone);
        float const offsetX = from.GetPositionX() - hazard.position.GetPositionX();
        float const offsetY = from.GetPositionY() - hazard.position.GetPositionY();
        float const along = offsetX * dx + offsetY * dy;
        float const outside = offsetX * offsetX + offsetY * offsetY - radius * radius;
        float const discriminant = along * along - outside;
        if (discriminant <= 0.0f)
            continue;

        float const root = std::sqrt(discriminant);
        float const enter = std::max(0.0f, -along - root);
        float const exit = std::min(length, -along + root);
        if (exit > enter)
            total += exit - enter;
    }

    return total;
}

bool GetSupremusReachBlockedByFire(PlayerbotAI* botAI, Unit*& target, float& range)
{
    Player* bot = botAI->GetBot();
    target = nullptr;
    range = 0.0f;

    // Stock reach doesn't break a channel either.
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    bool const isHealer = PlayerbotAI::IsHeal(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();
    target = context->GetValue<Unit*>(isHealer ? "party member to heal" : "current target")->Get();
    if (isHealer)
        range = botAI->GetRange("heal");
    else if (PlayerbotAI::IsRanged(bot))
        range = botAI->GetRange("spell");
    else
        range = sPlayerbotAIConfig.MeleeDistance;

    if (!target || !target->IsAlive() || bot->IsWithinCombatRange(target, range))
        return false;

    // Where the walk in would stop: a yard inside range, on the straight line to the target.
    constexpr float rangeInset = 1.0f;
    float const stopDistance =
        range + bot->GetCombatReach() + target->GetCombatReach() - rangeInset;
    float const distance = bot->GetExactDist2d(target);
    if (distance <= stopDistance)
        return false;

    float const ratio = stopDistance / distance;
    Position const stop(
        target->GetPositionX() + (bot->GetPositionX() - target->GetPositionX()) * ratio,
        target->GetPositionY() + (bot->GetPositionY() - target->GetPositionY()) * ratio,
        bot->GetPositionZ());

    return GetLineLengthInSupremusHazards(
        GetSupremusHazards(botAI), bot->GetPosition(), stop, SupremusHazardZone::Safe) > 0.0f;
}

// Shade of Akama

GuidVector FindShadeOfAkamaAddGuids(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GuidVector adds;
    if (bot->GetPositionX() < SHADE_OF_AKAMA_BOUNDARY_MIN_X ||
        bot->GetPositionX() > SHADE_OF_AKAMA_BOUNDARY_MAX_X ||
        bot->GetPositionY() < SHADE_OF_AKAMA_BOUNDARY_MIN_Y ||
        bot->GetPositionY() > SHADE_OF_AKAMA_BOUNDARY_MAX_Y ||
        !IsEncounterInProgress(bot, BT_MAP_ID))
    {
        return adds;
    }

    for (BtNpcs const entry :
         { BtNpcs::NPC_ASHTONGUE_CHANNELER, BtNpcs::NPC_ASHTONGUE_SORCERER })
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, Id(entry), SHADE_OF_AKAMA_ADD_SEARCH_RADIUS);

        std::vector<Creature*> living;
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive() &&
                !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            {
                living.push_back(creature);
            }
        }

        std::sort(living.begin(), living.end(), [](Creature* first, Creature* second)
            { return first->GetGUID() < second->GetGUID(); });

        for (Creature* creature : living)
            adds.push_back(creature->GetGUID());
    }

    return adds;
}

Unit* GetShadeOfAkamaKillTarget(PlayerbotAI* botAI)
{
    auto const& adds =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shade of akama adds")->RefGet();
    for (ObjectGuid const& guid : adds)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive())
            return unit;
    }

    return nullptr;
}

bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY)
{
    if (!target)
        return false;

    return GetPathStepTowardPoint(
        bot, target->GetPosition(), stopDistance, PATH_STEP_DISTANCE, stepX, stepY);
}

bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY)
{
    if (bot->GetExactDist(destination) < stopDistance)
        return false;

    PathGenerator path(bot);
    path.CalculatePath(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
    if (!(path.GetPathType() & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT)))
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2)
        return false;

    G3D::Vector3 const targetPos(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());

    float remaining = stepDistance;
    for (std::size_t i = 1; i < points.size(); ++i)
    {
        G3D::Vector3 const& from = points[i - 1];
        G3D::Vector3 const& to = points[i];

        float const segment = (to - from).length();
        if (segment <= 0.0f)
            continue;

        float const toDist = (to - targetPos).length();
        float ratio = 1.0f;

        if (toDist < stopDistance)
        {
            float const fromDist = (from - targetPos).length();
            if (fromDist <= stopDistance)
                break;

            ratio = (fromDist - stopDistance) / (fromDist - toDist);
        }

        if (segment * ratio >= remaining)
            ratio = remaining / segment;

        remaining -= segment * ratio;

        G3D::Vector3 const step = from + (to - from) * ratio;
        stepX = step.x;
        stepY = step.y;

        if (remaining <= 0.0f || ratio < 1.0f)
            return true;
    }

    return remaining < stepDistance;
}

// Teron Gorefiend

GuidVector FindShadowyConstructGuids(PlayerbotAI* botAI)
{
    GuidVector constructs;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == Id(BtNpcs::NPC_SHADOWY_CONSTRUCT))
        {
            constructs.push_back(guid);
        }
    }

    return constructs;
}

bool CastVengefulSpiritSpell(Unit* spirit, Unit* target, uint32 spellId)
{
    static constexpr std::array spiritSpells = {
        Id(BtSpells::SPELL_SPIRIT_STRIKE),
        Id(BtSpells::SPELL_SPIRIT_LANCE),
        Id(BtSpells::SPELL_SPIRIT_CHAINS),
        Id(BtSpells::SPELL_SPIRIT_VOLLEY),
        Id(BtSpells::SPELL_SPIRIT_SHIELD),
    };

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || spirit->HasSpellCooldown(spellId) ||
        spirit->CastSpell(target, spellId, true) != SPELL_CAST_OK)
    {
        return false;
    }

    if (uint32 const cooldown =
            std::max(spellInfo->RecoveryTime, spellInfo->CategoryRecoveryTime))
    {
        spirit->AddSpellCooldown(spellId, 0, cooldown);
    }

    for (uint32 const spiritSpellId : spiritSpells)
    {
        if (!spirit->HasSpellCooldown(spiritSpellId))
            spirit->AddSpellCooldown(spiritSpellId, 0, spellInfo->StartRecoveryTime);
    }

    return true;
}

// Gurtogg Bloodboil

namespace
{

// Ranged bots in group order, the Fel Rage target left out.
std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot)
{
    std::vector<std::vector<Player*>> groups(GURTOGG_ROTATION_GROUP_COUNT);
    Group* group = bot->GetGroup();
    if (!group)
        return groups;

    size_t count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive() ||
            !GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRanged(member) ||
            member->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        {
            continue;
        }

        groups[count / GURTOGG_ROTATION_GROUP_SIZE].push_back(member);
        if (++count == GURTOGG_ROTATION_GROUP_COUNT * GURTOGG_ROTATION_GROUP_SIZE)
            break;
    }

    return groups;
}

// The group hit longest ago soaks next: the lowest mean Bloodboil time left, ties to the first.
// Just after a cast, that group's debuffs run out before the next one. Debuffs younger than the
// rotation delay count as none, so the group just hit stays the soakers until then.
GuidVector FindGurtoggBloodboilSoakerGuids(Player* bot)
{
    std::vector<std::vector<Player*>> const groups = GetGurtoggRangedRotationGroups(bot);
    std::vector<Player*> const* soakers = nullptr;
    float lowestMeanRemaining = 0.0f;
    for (std::vector<Player*> const& group : groups)
    {
        if (group.empty())
            continue;

        int32 totalRemaining = 0;
        for (Player* member : group)
        {
            Aura* bloodboil = member->GetAura(Id(BtSpells::SPELL_BLOODBOIL));
            if (!bloodboil)
                continue;

            int32 const elapsed = bloodboil->GetMaxDuration() - bloodboil->GetDuration();
            if (elapsed >= GURTOGG_ROTATION_DELAY_MS)
                totalRemaining += bloodboil->GetDuration();
        }

        float const meanRemaining = static_cast<float>(totalRemaining) / group.size();
        if (!soakers || meanRemaining < lowestMeanRemaining)
        {
            soakers = &group;
            lowestMeanRemaining = meanRemaining;
        }
    }

    GuidVector guids;
    if (soakers)
    {
        for (Player* member : *soakers)
            guids.push_back(member->GetGUID());
    }

    return guids;
}

} // namespace

Position const& GetGurtoggBloodboilPosition(Player* bot)
{
    GuidVector const soakers = FindGurtoggBloodboilSoakerGuids(bot);
    bool const isSoaker =
        std::find(soakers.begin(), soakers.end(), bot->GetGUID()) != soakers.end();
    return isSoaker ? GURTOGG_SOAKER_POSITION : GURTOGG_RANGED_POSITION;
}

// The Fel Rage target taunts him onto itself, so it is his victim.
Unit* GetGurtoggFelRageTarget(Unit* gurtogg)
{
    if (!gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)))
        return nullptr;

    Unit* victim = gurtogg->GetVictim();
    return victim && victim->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)) ? victim : nullptr;
}

Position GetGurtoggFelRageMeleePosition(Player* bot, Unit* gurtogg, Unit* felRageTarget)
{
    float const axisAngle = gurtogg->GetAngle(felRageTarget);
    float relativeAngle = Position::NormalizeOrientation(gurtogg->GetAngle(bot) - axisAngle);
    if (relativeAngle > M_PI)
        relativeAngle -= 2.0f * M_PI;

    float angle = axisAngle + static_cast<float>(M_PI);
    if (std::fabs(relativeAngle) < static_cast<float>(M_PI_2))
    {
        float const side = relativeAngle >= 0.0f ? 1.0f : -1.0f;
        angle = axisAngle + side * GURTOGG_FEL_RAGE_MELEE_SIDE_ANGLE;
    }

    return Position(
        gurtogg->GetPositionX() + GURTOGG_FEL_RAGE_MELEE_DISTANCE * std::cos(angle),
        gurtogg->GetPositionY() + GURTOGG_FEL_RAGE_MELEE_DISTANCE * std::sin(angle),
        bot->GetPositionZ());
}

float FindGurtoggSecondTankThreat(PlayerbotAI* botAI)
{
    Unit* gurtogg =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "gurtogg bloodboil")->Get();
    if (!gurtogg)
        return 0.0f;

    float highestThreat = 0.0f;
    float secondThreat = 0.0f;
    for (ThreatReference const* ref : gurtogg->GetThreatMgr().GetUnsortedThreatList())
    {
        if (!ref->IsAvailable())
            continue;

        Unit* victim = ref->GetVictim();
        Player* player = victim ? victim->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || !PlayerbotAI::IsTank(player))
            continue;

        float const threat = ref->GetThreat();
        if (threat > highestThreat)
        {
            secondThreat = highestThreat;
            highestThreat = threat;
        }
        else if (threat > secondThreat)
        {
            secondThreat = threat;
        }
    }

    return secondThreat;
}

// Reliquary of Souls

bool IsSufferingFixateTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && bot->GetHealthPct() > SUFFERING_TANK_MIN_HEALTH_PCT;
}

bool IsOutOfSufferingPosition(Player* bot, Unit* suffering)
{
    if (!suffering)
        return false;

    if (IsSufferingFixateTank(bot))
        return bot->GetExactDist2d(suffering) > SUFFERING_TANK_MAX_DISTANCE;

    if (PlayerbotAI::IsMelee(bot))
        return bot->GetExactDist2d(suffering) < SUFFERING_MELEE_MIN_DISTANCE;

    return PlayerbotAI::IsRanged(bot) && bot->GetExactDist2d(suffering) < SUFFERING_RANGED_DISTANCE;
}

// Illidari Council

// (1) First priority is an assistant Mage (real player or bot)
// (2) If no assistant Mage, then look for any Mage bot
ObjectGuid FindZerevorMageTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    Player* fallbackMage = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive() ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!fallbackMage && GET_PLAYERBOT_AI(member))
            fallbackMage = member;
    }

    return fallbackMage ? fallbackMage->GetGUID() : ObjectGuid::Empty;
}

Player* GetZerevorMageTank(PlayerbotAI* botAI)
{
    return GetCachedPlayer(botAI, "illidari council zerevor mage tank");
}

bool IsZerevorMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_MAGE && GetZerevorMageTank(botAI) == bot;
}

bool HasDangerousCouncilAura(Player* bot)
{
    static constexpr std::array dangerousAuras = {
        Id(BtSpells::SPELL_CONSECRATION),
        Id(BtSpells::SPELL_BLIZZARD),
        Id(BtSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 aura : dangerousAuras)
    {
        if (bot->HasAura(aura))
            return true;
    }

    return false;
}

bool IsVerasVanished(Unit* veras)
{
    return veras && veras->HasAura(Id(BtSpells::SPELL_VERAS_VANISH));
}

bool CanInterruptCircleOfHealing(Unit* malande)
{
    if (!malande)
        return false;

    Spell* spell = malande->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_CIRCLE_OF_HEALING));
    return spell && spell->getState() == SPELL_STATE_PREPARING &&
        spell->GetCastTime() - spell->GetCastTimeRemaining() >= CIRCLE_OF_HEALING_REACTION_MS;
}

// Zerevor has at most one Blizzard and one Flamestrike down at a time.
bool IsInZerevorPatch(Unit* zerevor, Position const& point, float margin)
{
    if (!zerevor)
        return false;

    static constexpr std::array patchSpells = {
        Id(BtSpells::SPELL_BLIZZARD),
        Id(BtSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 spellId : patchSpells)
    {
        DynamicObject* patch = zerevor->GetDynObject(spellId);
        if (patch && patch->GetExactDist2d(point) < patch->GetRadius() + margin)
            return true;
    }

    return false;
}

bool IsInCouncilPatch(Position const& point, Unit* gathios, Unit* zerevor, float margin)
{
    if (gathios)
    {
        DynamicObject* consecration = gathios->GetDynObject(Id(BtSpells::SPELL_CONSECRATION));
        if (consecration &&
            consecration->GetExactDist2d(point) < consecration->GetRadius() + margin)
        {
            return true;
        }
    }

    return IsInZerevorPatch(zerevor, point, margin);
}

// Gathios's Consecration reaches Malande only at the pull, while he is beside her on the dais.
bool IsMalandeInCouncilPatch(Unit* malande, Unit* gathios, Unit* zerevor)
{
    return malande && IsInCouncilPatch(*malande, gathios, zerevor);
}

bool IsZerevorOnMageTank(PlayerbotAI* botAI, Unit* zerevor)
{
    Player* mageTank = GetZerevorMageTank(botAI);
    return zerevor && mageTank && zerevor->GetVictim() == mageTank;
}

// Within heal range of the mage tank, clear of a patch on it, of Arcane Explosion, and of any
// patch down.
bool IsMageTankHealerPositionSafe(Position const& point, Player* mageTank, Unit* zerevor)
{
    float const toMageTank = mageTank->GetExactDist2d(point);
    return toMageTank <= MAGE_TANK_HEALER_MAX_DISTANCE &&
        toMageTank >= MAGE_TANK_HEALER_MIN_DISTANCE &&
        zerevor->GetExactDist2d(point) >= ZEREVOR_ARCANE_EXPLOSION_SAFE_DISTANCE &&
        !IsInZerevorPatch(zerevor, point, COUNCIL_PATCH_MARGIN);
}

namespace
{

// A blocked navmesh raycast cuts the point short and still returns true, so a moved point is a miss.
bool CanWalkStraightTo(Player* bot, Position const& point)
{
    constexpr float clampTolerance = 0.5f;
    float x = point.GetPositionX();
    float y = point.GetPositionY();
    float z = point.GetPositionZ();
    return bot->GetMap()->CheckCollisionAndGetValidCoords(
               bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z) &&
        point.GetExactDist2d(x, y) <= clampTolerance;
}

} // namespace

// The nearest safe point on two rings round the mage tank that the bot can walk to in a straight
// line and that has line of sight to the mage tank.
bool FindMageTankHealerPosition(
    Player* bot, Player* mageTank, Unit* zerevor, Position& destination)
{
    static constexpr std::array ringRadii = { 16.0f, 24.0f };
    constexpr uint8 pointsPerRing = 16;

    std::vector<Position> candidates;
    for (float radius : ringRadii)
    {
        for (uint8 i = 0; i < pointsPerRing; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * i / pointsPerRing;
            Position const point(
                mageTank->GetPositionX() + radius * std::cos(angle),
                mageTank->GetPositionY() + radius * std::sin(angle), bot->GetPositionZ());
            if (IsMageTankHealerPositionSafe(point, mageTank, zerevor))
                candidates.push_back(point);
        }
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
        });

    for (Position const& candidate : candidates)
    {
        if (!CanWalkStraightTo(bot, candidate) ||
            !mageTank->IsWithinLOS(
                candidate.GetPositionX(), candidate.GetPositionY(), candidate.GetPositionZ()))
        {
            continue;
        }

        destination = candidate;
        return true;
    }

    return false;
}

// The nearest point within melee range of Malande, clear of every council patch, that the bot can
// walk to in a straight line.
bool FindMalandeTankPosition(
    Player* bot, Unit* malande, Unit* gathios, Unit* zerevor, Position& destination)
{
    static constexpr std::array ringFractions = { 0.5f, 0.85f };
    constexpr uint8 pointsPerRing = 16;
    float const meleeRange = bot->GetMeleeRange(malande);

    std::vector<Position> candidates;
    for (float fraction : ringFractions)
    {
        float const radius = meleeRange * fraction;
        for (uint8 i = 0; i < pointsPerRing; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * i / pointsPerRing;
            Position const point(
                malande->GetPositionX() + radius * std::cos(angle),
                malande->GetPositionY() + radius * std::sin(angle), bot->GetPositionZ());
            if (!IsInCouncilPatch(point, gathios, zerevor, COUNCIL_PATCH_MARGIN))
                candidates.push_back(point);
        }
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
        });

    for (Position const& candidate : candidates)
    {
        if (!CanWalkStraightTo(bot, candidate))
            continue;

        destination = candidate;
        return true;
    }

    return false;
}

// Any council member but the bot's own target.
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range)
{
    static constexpr std::array memberNames = {
        "gathios the shatterer",
        "high nethermancer zerevor",
        "lady malande",
        "veras darkshadow",
    };

    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* ownTarget = AI_VALUE(Unit*, "current target");
    for (char const* name : memberNames)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", name);
        if (member && member != ownTarget && bot->GetDistance(member) < range)
            return true;
    }

    return false;
}

// Felhunters stay on Malande for Spell Lock; other pets follow their master's target, or go to
// Gathios while the master has none.
Unit* GetCouncilPetTarget(PlayerbotAI* botAI, Creature* pet)
{
    if (!pet)
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return nullptr;

    if (pet->GetEntry() == NPC_FELHUNTER)
    {
        if (Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande"))
            return malande;
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->IsAlive() ? target : gathios;
}

// PlayerbotAI::CanCastSpell passes any spell the pet knows without checking range or cooldown.
uint32 GetReadySpellLock(Creature* pet, Unit* target)
{
    if (!pet || !target || !pet->IsAlive() || pet->GetEntry() != NPC_FELHUNTER)
        return 0;

    constexpr float spellLockRange = 30.0f;
    if (!pet->IsWithinCombatRange(target, spellLockRange) || !pet->IsWithinLOSInMap(target))
        return 0;

    static constexpr std::array spellLockRanks = {
        Id(BtSpells::SPELL_SPELL_LOCK_2),
        Id(BtSpells::SPELL_SPELL_LOCK_1),
    };

    for (uint32 spellLock : spellLockRanks)
    {
        if (pet->HasSpell(spellLock))
            return pet->HasSpellCooldown(spellLock) ? 0 : spellLock;
    }

    return 0;
}

// Illidan Stormrage <The Betrayer>

namespace // Illidan Stormrage
{

std::unordered_map<ObjectGuid, IllidanInfection> const* FindIllidanInfections(uint32 instanceId)
{
    BtInstanceState const* state = FindBtState(instanceId);
    return state && state->illidanInfections ? &*state->illidanInfections : nullptr;
}

} // end anonymous namespace (Illidan Stormrage)

IllidanPhase GetIllidanPhase(Unit* illidan)
{
    if (!illidan)
        return IllidanPhase::None;

    if (illidan->GetHealth() == 1)
        return IllidanPhase::Dying;

    float const healthPct = illidan->GetHealthPct();
    if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        // He takes no damage while flying, so below 30% he is unselectable only for Maiev's
        // arrival.
        if (healthPct <= 30.0f)
            return IllidanPhase::None;

        float x, y, z;
        bool const isLanding = illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
            (illidan->GetMotionMaster()->GetDestination(x, y, z) &&
             ILLIDAN_LANDING_POSITION.GetExactDist2d(x, y) < 0.2f);

        return isLanding ? IllidanPhase::Landing : IllidanPhase::Flying;
    }

    if (healthPct > 65.0f)
        return IllidanPhase::Initial;

    if (illidan->HasAura(Id(BtSpells::SPELL_DEMON_FORM)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_1)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_2)) ||
        illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_3)))
    {
        return IllidanPhase::Demon;
    }

    return healthPct > 30.0f ? IllidanPhase::Grounded : IllidanPhase::Maiev;
}

bool IsIllidanSpeechInProgress(Player* bot)
{
    if (IsEncounterInProgress(bot, BT_MAP_ID) ||
        bot->GetExactDist(ILLIDAN_SPEECH_POSITION) > ILLIDAN_SPEECH_SEARCH_RADIUS)
    {
        return false;
    }

    Creature* illidan = bot->FindNearestCreature(
        Id(BtNpcs::NPC_ILLIDAN_STORMRAGE), ILLIDAN_SPEECH_SEARCH_RADIUS);

    return illidan && illidan->getStandState() == UNIT_STAND_STATE_STAND &&
        illidan->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) && !illidan->IsInCombat();
}

bool IsIllidanPhaseStartWithin(Unit const& illidan, uint32 durationMs)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    return !state || !state->illidanPhaseStartTime ||
        getMSTimeDiff(*state->illidanPhaseStartTime, getMSTime()) < durationMs;
}

IllidanPhase GetIllidanPhaseBefore(Unit const& illidan, IllidanPhase phase)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    if (!state || !state->illidanLastPhase)
        return IllidanPhase::None;

    if (*state->illidanLastPhase != phase)
        return *state->illidanLastPhase;

    return state->illidanPreviousPhase.value_or(IllidanPhase::None);
}

bool IsIllidanDemonFormEnding(Unit const& illidan, uint32 durationMs)
{
    BtInstanceState const* state = FindBtState(illidan.GetInstanceId());
    return state && state->illidanPhaseStartTime &&
        getMSTimeDiff(*state->illidanPhaseStartTime, getMSTime()) >=
        ILLIDAN_DEMON_FORM_DURATION_MS - durationMs;
}

IllidanDpsHold GetIllidanHoldWindow(Unit* illidan)
{
    if (!illidan || illidan->GetHealthPct() <= ILLIDAN_BURN_HEALTH_PCT)
        return IllidanDpsHold::None;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    // The tracker stamps a new phase's start on its next tick, and until then the start time is
    // the old phase's, so a change it hasn't recorded yet counts as a phase just started.
    BtInstanceState const* state = FindBtState(illidan->GetInstanceId());
    bool const isUnrecorded = !state || state->illidanLastPhase != phase;
    if (phase == IllidanPhase::Demon)
    {
        if (illidan->GetHealthPct() <= ILLIDAN_DEMON_FORM_NO_PUSH_HEALTH_PCT)
            return IllidanDpsHold::DemonFormNoPush;

        if (isUnrecorded)
            return IllidanDpsHold::DemonFormStart;

        if (IsIllidanDemonFormEnding(*illidan, ILLIDAN_DEMON_FORM_END_HOLD_MS))
            return IllidanDpsHold::DemonFormEnd;

        return IsIllidanPhaseStartWithin(*illidan, ILLIDAN_DEMON_FORM_DPS_WAIT_MS) ?
            IllidanDpsHold::DemonFormStart : IllidanDpsHold::DemonFormMiddle;
    }

    bool isAfterReset = phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded;
    if (phase == IllidanPhase::Maiev)
        isAfterReset = GetIllidanPhaseBefore(*illidan, phase) == IllidanPhase::Demon;

    bool const isStarting =
        isUnrecorded || IsIllidanPhaseStartWithin(*illidan, ILLIDAN_RESET_DPS_WAIT_MS);
    return isAfterReset && isStarting ? IllidanDpsHold::AfterReset : IllidanDpsHold::None;
}

IllidanDpsHold GetIllidanDpsHold(PlayerbotAI* botAI, Unit* illidan)
{
    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    Player* bot = botAI->GetBot();
    if (window == IllidanDpsHold::DemonFormStart)
    {
        return IsIllidanWarlockTank(botAI) ? IllidanDpsHold::None : window;
    }
    else if (window == IllidanDpsHold::DemonFormMiddle)
    {
        return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot) ?
            window : IllidanDpsHold::None;
    }
    else if (window == IllidanDpsHold::AfterReset)
    {
        return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot) ?
            IllidanDpsHold::None : window;
    }

    return window;
}

float GetGrateAngle(Position const& position)
{
    return ILLIDAN_GRATE_CENTRE.GetAngle(&position);
}

Position GetRingPoint(float angle, float radius)
{
    return Position(
        ILLIDAN_GRATE_CENTRE.GetPositionX() + radius * std::cos(angle),
        ILLIDAN_GRATE_CENTRE.GetPositionY() + radius * std::sin(angle),
        ILLIDAN_GRATE_CENTRE.GetPositionZ());
}

Position GetIllidanSettlePosition(Unit const& illidan)
{
    float const radius = ILLIDAN_GRATE_CENTRE.GetExactDist2d(&illidan) + ILLIDAN_TANK_OFFSET;
    return GetRingPoint(GetGrateAngle(illidan), radius);
}

Position GetGrateSlot(size_t index)
{
    constexpr size_t innerSlots = 6;
    constexpr size_t outerSlots = 12;
    constexpr float fullCircle = 2.0f * static_cast<float>(M_PI);

    size_t const slot = index % ILLIDAN_RANGED_SLOT_COUNT;
    if (slot == 0)
        return ILLIDAN_GRATE_CENTRE;

    if (slot <= innerSlots)
    {
        float const angle = fullCircle * static_cast<float>(slot - 1) / innerSlots;
        return GetRingPoint(angle, ILLIDAN_GRATE_INNER_RADIUS);
    }

    float const angle =
        fullCircle * (static_cast<float>(slot - 1 - innerSlots) + 0.5f) / outerSlots;
    return GetRingPoint(angle, ILLIDAN_GRATE_OUTER_RADIUS);
}

DemonFormLayout GetDemonFormLayout(Unit const& illidan)
{
    constexpr float degree = static_cast<float>(M_PI) / 180.0f;
    // Ranged DPS rows are inside 31 yd, in spell reach of him, and healer rows behind them; both
    // are filled from the warlock's side. Melee rows lie behind the healers, centred between the
    // warlock and the far end of the ranged DPS, and are filled nearest him first.
    constexpr std::array rangedRows = { 25.2f, 31.0f };
    constexpr std::array healerRows = { 36.8f, 42.6f };
    constexpr std::array meleeRows = { 48.4f, 54.2f, 60.0f };
    constexpr float rangedHighAngle = 40.0f * degree;
    constexpr float healerHighAngle = 60.0f * degree;
    constexpr float lowAngle = -90.0f * degree;
    constexpr float meleeHalfArc = 40.0f * degree;
    // Heals reach 43 yd centre to centre.
    constexpr float healReach = 41.0f;

    float const inwardAngle = illidan.GetAngle(&ILLIDAN_GRATE_CENTRE);

    DemonFormLayout layout;
    layout.warlockTankPosition = GetDemonFormPoint(
        illidan, inwardAngle, ILLIDAN_WARLOCK_POSITION_DISTANCE, ILLIDAN_WARLOCK_POSITION_ANGLE);
    layout.mainTankWaitPosition = GetDemonFormPoint(
        illidan, inwardAngle, ILLIDAN_MAIN_TANK_WAIT_DISTANCE, ILLIDAN_MAIN_TANK_WAIT_ANGLE);
    layout.mainTankPickupPosition = GetDemonFormPoint(
        illidan, inwardAngle, ILLIDAN_PICKUP_POSITION_DISTANCE, ILLIDAN_PICKUP_POSITION_ANGLE);
    layout.mainTankStepInPosition = GetDemonFormPoint(
        illidan, inwardAngle, ILLIDAN_STEP_IN_POSITION_DISTANCE, ILLIDAN_PICKUP_POSITION_ANGLE);

    auto const fromWarlockSide =
        [](std::pair<float, Position> const& a, std::pair<float, Position> const& b)
    {
        return a.first > b.first;
    };

    std::vector<std::pair<float, Position>> ranged;
    std::vector<std::pair<float, Position>> healers;
    for (size_t i = 0; i < rangedRows.size(); ++i)
    {
        std::vector<std::pair<float, Position>> const rangedRow = GetDemonFormRow(
            illidan, inwardAngle, layout.warlockTankPosition, rangedRows[i], rangedHighAngle,
            lowAngle, i % 2 == 1);
        ranged.insert(ranged.end(), rangedRow.begin(), rangedRow.end());

        std::vector<std::pair<float, Position>> const healerRow = GetDemonFormRow(
            illidan, inwardAngle, layout.warlockTankPosition, healerRows[i], healerHighAngle,
            lowAngle, i % 2 == 1);
        healers.insert(healers.end(), healerRow.begin(), healerRow.end());
    }

    if (ranged.empty())
        return layout;

    std::stable_sort(ranged.begin(), ranged.end(), fromWarlockSide);
    if (ranged.size() > ILLIDAN_RANGED_SLOT_COUNT)
        ranged.resize(ILLIDAN_RANGED_SLOT_COUNT);

    for (std::pair<float, Position> const& slot : ranged)
        layout.rangedSlots.push_back(slot.second);

    std::stable_sort(healers.begin(), healers.end(), fromWarlockSide);
    for (std::pair<float, Position> const& slot : healers)
    {
        if (layout.healerSlots.size() < ILLIDAN_HEALER_SLOT_COUNT &&
            slot.second.GetExactDist2d(layout.warlockTankPosition) <= healReach)
        {
            layout.healerSlots.push_back(slot.second);
        }
    }

    float const meleeCentreAngle = (ILLIDAN_WARLOCK_POSITION_ANGLE + ranged.back().first) / 2.0f;
    for (size_t i = 0; i < meleeRows.size(); ++i)
    {
        std::vector<std::pair<float, Position>> row = GetDemonFormRow(
            illidan, inwardAngle, layout.warlockTankPosition, meleeRows[i],
            meleeCentreAngle + meleeHalfArc, meleeCentreAngle - meleeHalfArc, i % 2 == 1);
        std::stable_sort(
            row.begin(), row.end(),
            [meleeCentreAngle](
                std::pair<float, Position> const& a, std::pair<float, Position> const& b)
            {
                return std::abs(a.first - meleeCentreAngle) <
                    std::abs(b.first - meleeCentreAngle);
            });

        for (std::pair<float, Position> const& slot : row)
        {
            if (layout.meleeSlots.size() < ILLIDAN_MELEE_SLOT_COUNT)
                layout.meleeSlots.push_back(slot.second);
        }
    }

    return layout;
}

Position GetIllidanInfectionPosition(Unit const& illidan, IllidanPhase phase)
{
    if (phase == IllidanPhase::Demon)
    {
        float const inwardAngle = illidan.GetAngle(&ILLIDAN_GRATE_CENTRE);
        return GetDemonFormPoint(
            illidan, inwardAngle, ILLIDAN_DEMON_INFECTION_POSITION_DISTANCE,
            ILLIDAN_DEMON_INFECTION_POSITION_ANGLE);
    }

    return GetRingPoint(
        GetGrateAngle(illidan) + ILLIDAN_INFECTION_POSITION_ANGLE,
        ILLIDAN_INFECTION_POSITION_DISTANCE);
}

IllidanPhase GetIllidanInfectionPositionPhase(Unit const& illidan, IllidanPhase phase)
{
    bool const isTransform = phase == IllidanPhase::Demon &&
        IsIllidanPhaseStartWithin(illidan, ILLIDAN_DEMON_TRANSFORM_MS);
    if (!isTransform && phase != IllidanPhase::Dying)
        return phase;

    IllidanPhase const previous = GetIllidanPhaseBefore(illidan, phase);
    return previous != IllidanPhase::None ? previous : IllidanPhase::Grounded;
}

Position GetIllidanRecordedInfectionPosition(
    PlayerbotAI* botAI, Unit const& illidan, IllidanPhase phase)
{
    IllidanPhase const infectionPhase = GetIllidanInfectionPositionPhase(illidan, phase);
    Player* bot = botAI->GetBot();
    if (auto const* infections = FindIllidanInfections(bot->GetInstanceId()))
    {
        // Entries from the phase before are kept while he flies.
        bool const isFlying = phase == IllidanPhase::Flying;
        auto const own = infections->find(bot->GetGUID());
        if (own != infections->end() && (isFlying || own->second.phase == infectionPhase))
            return own->second.infectionPosition;

        for (auto const& [player, infection] : *infections)
        {
            if (infection.hunter == bot->GetGUID() &&
                (isFlying || infection.phase == infectionPhase))
            {
                return infection.infectionPosition;
            }
        }
    }

    return GetIllidanInfectionPosition(illidan, infectionPhase);
}

std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot)
{
    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;

    BtInstanceState const* state = FindBtState(bot->GetMap()->GetInstanceId());

    if (state && state->eastFlameGuid)
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, *state->eastFlameGuid))
        {
            if (unit->IsAlive())
                eastFlame = unit;
        }
    }

    if (state && state->westFlameGuid)
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, *state->westFlameGuid))
        {
            if (unit->IsAlive())
                westFlame = unit;
        }
    }

    return { eastFlame, westFlame };
}

bool IsWestFlameOfAzzinothTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0);
}

bool IsEastFlameOfAzzinothTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1);
}

bool IsFlameOfAzzinothTank(Player* bot)
{
    return IsEastFlameOfAzzinothTank(bot) || IsWestFlameOfAzzinothTank(bot);
}

Unit* GetAssignedFlameOfAzzinoth(Player* bot)
{
    bool const isEastTank = IsEastFlameOfAzzinothTank(bot);
    if (!isEastTank && !IsWestFlameOfAzzinothTank(bot))
        return nullptr;

    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    Unit* ownFlame = isEastTank ? eastFlame : westFlame;
    if (ownFlame)
        return ownFlame;

    // An unrecorded own Flame hasn't spawned yet; a recorded one is dead.
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    bool const isOwnFlameRecorded =
        state && (isEastTank ? state->eastFlameGuid : state->westFlameGuid).has_value();
    Unit* otherFlame = isEastTank ? westFlame : eastFlame;
    if (!otherFlame || !isOwnFlameRecorded)
        return nullptr;

    Player* otherTank = FindFlameOfAzzinothTank(bot, !isEastTank);
    return !otherTank || !otherTank->IsAlive() ? otherFlame : nullptr;
}

ObjectGuid FindIllidanWarlockTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    ObjectGuid assistantWarlock;
    ObjectGuid botWarlock;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || member->getClass() != CLASS_WARLOCK)
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (group->IsAssistant(guid))
        {
            if (assistantWarlock.IsEmpty() || guid < assistantWarlock)
                assistantWarlock = guid;
        }
        else if (GET_PLAYERBOT_AI(member) && (botWarlock.IsEmpty() || guid < botWarlock))
        {
            botWarlock = guid;
        }
    }

    return assistantWarlock.IsEmpty() ? botWarlock : assistantWarlock;
}

Player* GetIllidanWarlockTank(PlayerbotAI* botAI)
{
    return GetCachedPlayer(botAI, "illidan stormrage warlock tank");
}

bool IsIllidanWarlockTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_WARLOCK && GetIllidanWarlockTank(botAI) == bot;
}

bool CastIllidanLifeTapIfLow(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetPowerPct(POWER_MANA) >= ILLIDAN_LIFE_TAP_MANA_PCT ||
        bot->GetHealthPct() <= ILLIDAN_LIFE_TAP_HEALTH_PCT)
    {
        return false;
    }

    return botAI->CanCastSpell("life tap", bot) && botAI->CastSpell("life tap", bot);
}

GuidVector FindIllidanTrapHunterGuids(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    GuidVector trapHunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            trapHunters.push_back(member->GetGUID());
        }
    }

    std::sort(trapHunters.begin(), trapHunters.end());
    return trapHunters;
}

bool IsIllidanTrapHunterOnDuty(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_HUNTER || bot->HasSpellCooldown(Id(BtSpells::SPELL_FROST_TRAP)) ||
        HasParasiticShadowfiend(bot))
    {
        return false;
    }

    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (!infections)
        return false;

    for (auto const& [infected, infection] : *infections)
    {
        if (infection.hunter == bot->GetGUID())
            return true;
    }

    return false;
}

bool IsIllidanInfectionPositionKeptInFlight(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (!infections)
        return false;

    if (HasParasiticShadowfiend(bot) && infections->contains(bot->GetGUID()))
        return true;

    return IsIllidanTrapHunterOnDuty(botAI);
}

bool IsFrostTrapPatchAt(Player* bot, Position const& infectionPosition, int32 minDurationMs)
{
    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(bot, ILLIDAN_ADD_SEARCH_RADIUS);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objects, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, ILLIDAN_ADD_SEARCH_RADIUS);

    for (WorldObject* object : objects)
    {
        DynamicObject* patch = object->ToDynObject();
        if (patch && patch->GetSpellId() == Id(BtSpells::SPELL_FROST_TRAP_AURA) &&
            patch->GetExactDist2d(infectionPosition) < patch->GetRadius() &&
            patch->GetDuration() >= minDurationMs)
        {
            return true;
        }
    }

    return false;
}

int32 GetIllidanRangedIndex(PlayerbotAI* botAI, size_t slotCount, bool inDemonForm)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    ObjectGuid const mainTank = PlayerbotAI::GetMainTankGuid(group);
    AiObjectContext* context = botAI->GetAiObjectContext();
    ObjectGuid const warlockTankGuid = inDemonForm ?
        context->GetValue<ObjectGuid>("illidan stormrage warlock tank")->Get() : ObjectGuid::Empty;

    std::vector<ObjectGuid> healers;
    std::vector<ObjectGuid> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (guid == mainTank || guid == warlockTankGuid)
            continue;

        if (PlayerbotAI::IsHeal(member))
        {
            if (!inDemonForm)
                healers.push_back(guid);
        }
        else if (PlayerbotAI::IsRanged(member))
        {
            rangedDps.push_back(guid);
        }
    }

    int32 const rosterIndex = GetRosterIndex(healers, rangedDps, bot->GetGUID());
    if (rosterIndex < 0)
        return -1;

    return static_cast<int32>(static_cast<size_t>(rosterIndex) % slotCount);
}

int32 GetIllidanHealerIndex(PlayerbotAI* botAI, size_t slotCount)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    std::vector<ObjectGuid> healers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && GET_PLAYERBOT_AI(member) &&
            PlayerbotAI::IsHeal(member))
        {
            healers.push_back(member->GetGUID());
        }
    }

    int32 const rosterIndex = GetRosterIndex(healers, {}, bot->GetGUID());
    if (rosterIndex < 0)
        return -1;

    return static_cast<int32>(static_cast<size_t>(rosterIndex) % slotCount);
}

int32 GetIllidanMeleeIndex(PlayerbotAI* botAI, size_t slotCount)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || slotCount == 0)
        return -1;

    ObjectGuid const mainTank = PlayerbotAI::GetMainTankGuid(group);

    std::vector<ObjectGuid> meleeDps;
    std::vector<ObjectGuid> tanks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (guid == mainTank)
            continue;

        if (PlayerbotAI::IsTank(member))
            tanks.push_back(guid);
        else if (!PlayerbotAI::IsHeal(member) && PlayerbotAI::IsMelee(member))
            meleeDps.push_back(guid);
    }

    int32 const rosterIndex = GetRosterIndex(meleeDps, tanks, bot->GetGUID());
    if (rosterIndex < 0)
        return -1;

    return static_cast<int32>(static_cast<size_t>(rosterIndex) % slotCount);
}

int32 GetIllidanGrateIndex(PlayerbotAI* botAI, Unit const& illidan)
{
    constexpr bool inDemonForm = false;
    int32 const index = GetIllidanRangedIndex(botAI, ILLIDAN_RANGED_SLOT_COUNT, inDemonForm);
    if (index < 0)
        return index;

    Player* warlockTank = GetIllidanWarlockTank(botAI);
    PlayerbotAI* warlockTankAI = warlockTank ? GET_PLAYERBOT_AI(warlockTank) : nullptr;
    if (!warlockTankAI)
        return index;

    float const inwardAngle = illidan.GetAngle(&ILLIDAN_GRATE_CENTRE);
    Position const warlockTankPosition = GetDemonFormPoint(
        illidan, inwardAngle, ILLIDAN_WARLOCK_POSITION_DISTANCE, ILLIDAN_WARLOCK_POSITION_ANGLE);
    int32 warlockSlot = 0;
    float nearestDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < ILLIDAN_RANGED_SLOT_COUNT; ++i)
    {
        float const distance = GetGrateSlot(i).GetExactDist2d(warlockTankPosition);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            warlockSlot = static_cast<int32>(i);
        }
    }

    if (warlockTank == botAI->GetBot())
        return warlockSlot;

    if (index != warlockSlot)
        return index;

    int32 const warlockIndex =
        GetIllidanRangedIndex(warlockTankAI, ILLIDAN_RANGED_SLOT_COUNT, inDemonForm);
    return warlockIndex < 0 ? index : warlockIndex;
}

bool HasParasiticShadowfiend(Player* player)
{
    return player->HasAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_1)) ||
        player->HasAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_2));
}

GuidVector FindParasiticShadowfiendGuids(PlayerbotAI* botAI)
{
    std::list<Creature*> creatures;
    botAI->GetBot()->GetCreatureListWithEntryInGrid(
        creatures, Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND), ILLIDAN_ADD_SEARCH_RADIUS);

    GuidVector shadowfiends;
    for (Creature* creature : creatures)
    {
        if (creature->IsAlive())
            shadowfiends.push_back(creature->GetGUID());
    }

    return shadowfiends;
}

Unit* GetNearestParasiticShadowfiend(PlayerbotAI* botAI, Unit const& unit)
{
    auto const& shadowfiends =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("parasitic shadowfiends")->RefGet();

    Unit* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const& guid : shadowfiends)
    {
        Unit* shadowfiend = botAI->GetUnit(guid);
        if (!shadowfiend || !shadowfiend->IsAlive())
            continue;

        float const distance = unit.GetExactDist2d(shadowfiend);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = shadowfiend;
        }
    }

    return nearest;
}

bool IsNearNewParasiticShadowfiend(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (HasParasiticShadowfiend(bot))
        return false;

    Unit* shadowfiend = GetNearestParasiticShadowfiend(botAI, *bot);
    if (!shadowfiend)
        return false;

    return bot->GetExactDist2d(shadowfiend) <= ILLIDAN_SHADOWFIEND_BIRTHER_DISTANCE &&
        shadowfiend->GetHealthPct() >= ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT;
}

GuidVector FindPlayersWithParasiticShadowfiendGuids(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    GuidVector infected;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && HasParasiticShadowfiend(member))
            infected.push_back(member->GetGUID());
    }

    return infected;
}

std::vector<ShadowDemon> FindShadowDemons(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(BtNpcs::NPC_SHADOW_DEMON), ILLIDAN_ADD_SEARCH_RADIUS);

    std::vector<ShadowDemon> shadowDemons;
    for (Creature* creature : creatures)
    {
        if (creature->IsAlive())
            shadowDemons.push_back({ creature->GetGUID(), ObjectGuid::Empty });
    }

    Group* group = bot->GetGroup();
    if (shadowDemons.empty() || !group)
        return shadowDemons;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID)
            continue;

        for (Unit* unit : { static_cast<Unit*>(member), static_cast<Unit*>(member->GetPet()) })
        {
            if (!unit || !unit->IsAlive())
                continue;

            Aura const* paralyze = unit->GetAura(Id(BtSpells::SPELL_PARALYZE));
            if (!paralyze)
                continue;

            for (ShadowDemon& shadowDemon : shadowDemons)
            {
                if (shadowDemon.demon == paralyze->GetCasterGUID())
                    shadowDemon.target = unit->GetGUID();
            }
        }
    }

    return shadowDemons;
}

Unit* GetShadowDemonForRangedToAttack(PlayerbotAI* botAI, Unit const& from)
{
    // A Demon walks 4 yd/s, so a yard is a quarter second.
    constexpr float targetDistanceStep = 1.0f;
    constexpr uint32 walkingToWarlock = 0;
    constexpr uint32 walking = 1;
    constexpr uint32 idle = 2;

    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("shadow demons")->RefGet();
    ObjectGuid const warlockTank =
        context->GetValue<ObjectGuid>("illidan stormrage warlock tank")->Get();

    Unit* best = nullptr;
    std::tuple<uint32, uint32, float> bestKey = {
        std::numeric_limits<uint32>::max(), 0, 0.0f };
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        if (!demon || !demon->IsAlive())
            continue;

        Unit* target =
            shadowDemon.target.IsEmpty() ? nullptr : botAI->GetUnit(shadowDemon.target);
        uint32 rank = idle;
        uint32 step = 0;
        if (target && target->IsAlive())
        {
            rank = shadowDemon.target == warlockTank ? walkingToWarlock : walking;
            step = static_cast<uint32>(demon->GetExactDist2d(target) / targetDistanceStep);
        }

        std::tuple<uint32, uint32, float> const key = { rank, step, from.GetExactDist2d(demon) };
        if (key < bestKey)
        {
            bestKey = key;
            best = demon;
        }
    }

    return best;
}

Unit* GetShadowDemonWalkingToWarlockTank(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    ObjectGuid const warlockTank =
        context->GetValue<ObjectGuid>("illidan stormrage warlock tank")->Get();
    if (warlockTank.IsEmpty())
        return nullptr;

    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("shadow demons")->RefGet();
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        if (shadowDemon.target != warlockTank)
            continue;

        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        Unit* target = botAI->GetUnit(shadowDemon.target);
        if (demon && demon->IsAlive() && target && target->IsAlive())
            return demon;
    }

    return nullptr;
}

Unit* GetShadowDemonForMeleeToAttack(PlayerbotAI* botAI, Unit const& from)
{
    // Demons spawn 5 yd from him and walk out toward the raid, so melee running in from their
    // slots meet one well outside Aura of Dread's 15 yd even if they leave immediately.
    AiObjectContext* context = botAI->GetAiObjectContext();
    ObjectGuid const warlockTank =
        context->GetValue<ObjectGuid>("illidan stormrage warlock tank")->Get();
    Unit* nearest = nullptr;
    std::pair<bool, float> nearestKey = { true, std::numeric_limits<float>::max() };
    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("shadow demons")->RefGet();
    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        if (!demon || !demon->IsAlive() ||
            (!warlockTank.IsEmpty() && shadowDemon.target == warlockTank))
        {
            continue;
        }

        Unit* target =
            shadowDemon.target.IsEmpty() ? nullptr : botAI->GetUnit(shadowDemon.target);
        bool const isIdle = !target || !target->IsAlive();
        std::pair<bool, float> const key = { isIdle, from.GetExactDist2d(demon) };
        if (key < nearestKey)
        {
            nearestKey = key;
            nearest = demon;
        }
    }

    return nearest;
}

Unit* GetShadowDemonForPetToAttack(PlayerbotAI* botAI, Pet const& pet)
{
    uint32 const entry = pet.GetEntry();
    if (entry == NPC_IMP || entry == NPC_WATER_ELEMENTAL_TEMP ||
        entry == NPC_WATER_ELEMENTAL_PERM)
    {
        return GetShadowDemonForRangedToAttack(botAI, pet);
    }

    return GetShadowDemonForMeleeToAttack(botAI, pet);
}

EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BtNpcs::NPC_ILLIDAN_DB_TARGET), searchRadius);

    Creature* eyeBlastTrigger = nullptr;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
        {
            eyeBlastTrigger = creature;
            break;
        }
    }

    if (!eyeBlastTrigger)
        return {};

    Position const startPos = Position(eyeBlastTrigger->GetPositionX(),
        eyeBlastTrigger->GetPositionY(), eyeBlastTrigger->GetPositionZ());

    float destX, destY, destZ;
    if (!eyeBlastTrigger->GetMotionMaster()->GetDestination(destX, destY, destZ))
        return {};

    Position const endPos(destX, destY, destZ);

    if (startPos.GetExactDist2d(endPos) < 0.1f)
        return {};

    constexpr float eyeBlastWidth = 9.0f;
    return { startPos, endPos, eyeBlastWidth };
}

bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area)
{
    float const dx = area.end.GetPositionX() - area.start.GetPositionX();
    float const dy = area.end.GetPositionY() - area.start.GetPositionY();
    float const length =
        area.start.GetExactDist2d(area.end.GetPositionX(), area.end.GetPositionY());

    if (length < 0.1f)
        return false;

    float const projectionFactor = ((pos.GetPositionX() - area.start.GetPositionX()) * dx +
        (pos.GetPositionY() - area.start.GetPositionY()) * dy) / (length * length);

    float const clampedProjectionFactor = std::clamp(projectionFactor, 0.0f, 1.0f);

    float const closestX = area.start.GetPositionX() + clampedProjectionFactor * dx;
    float const closestY = area.start.GetPositionY() + clampedProjectionFactor * dy;

    float const distToLine = pos.GetExactDist2d(closestX, closestY);

    return distToLine < area.width;
}

GameObject* FindNearestCageTrap(PlayerbotAI* botAI)
{
    GuidVector const& gos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();

    GameObject* nearestTrap = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->isSpawned() && go->GetEntry() == Id(BtObjects::GO_CAGE_TRAP))
        {
            nearestTrap = go;
            break;
        }
    }

    return nearestTrap;
}

// Shared encounter state

BtInstanceState& BtState(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    return btStates[instanceId];
}

BtInstanceState* FindBtState(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    auto it = btStates.find(instanceId);
    return it != btStates.end() ? &it->second : nullptr;
}

bool BtResetInstance(uint32 instanceId)
{
    std::lock_guard lock(btStateMutex);
    auto it = btStates.find(instanceId);
    if (it == btStates.end())
        return false;

    BtInstanceState const& state = it->second;
    bool const wasSet = state.najentusSpineAssignments || state.najentusSpineThrower ||
        state.najentusSpineThrowTime || state.councilDpsWaitTimer || state.illidanLastPhase ||
        state.illidanPreviousPhase || state.illidanPhaseStartTime ||
        state.illidanFlamePhaseStartTime || state.eastFlameGuid || state.westFlameGuid ||
        state.illidanInfections;

    btStates.erase(it);
    return wasSet;
}

}
