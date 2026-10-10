/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "GameTime.h"
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

using namespace EncounterHelpers;

namespace BtHelpers
{

namespace
{

Player* GetCachedPlayer(PlayerbotAI* botAI, char const* value)
{
    Unit* unit = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>(value)->Get());
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return player && player->IsAlive() ? player : nullptr;
}

}

// Shared

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!botAI->GetBot()->HasAura(Id(BtSpells::SPELL_MISDIRECTION)))
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
    Player* bot, Position const& centre, Position const& facing, float halfWidth,
    float halfDepth)
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

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>> shadowmoonReaverAbsorptionStart;

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

}

bool IsLinkedSisterOfPleasure(Unit* unit)
{
    if (!unit || unit->GetEntry() != Id(BtNpcs::NPC_SISTER_OF_PLEASURE) ||
        !unit->IsAlive())
    {
        return false;
    }

    Aura* bonds = unit->GetAura(Id(BtSpells::SPELL_SHARED_BONDS));
    Unit* pain = bonds ? bonds->GetCaster() : nullptr;
    return pain && pain->IsAlive();
}

Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI)
{
    std::list<Creature*> sisters;
    botAI->GetBot()->GetCreatureListWithEntryInGrid(
        sisters, Id(BtNpcs::NPC_SISTER_OF_PLEASURE), SISTER_OF_PLEASURE_SEARCH_RADIUS);
    for (Creature* sister : sisters)
    {
        if (IsLinkedSisterOfPleasure(sister))
            return sister;
    }

    return nullptr;
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
    if (Aura* absorption = unit->GetAura(Id(BtSpells::SPELL_SPELL_ABSORPTION)))
    {
        // Reconstructed from the aura, so every caller stamps the same start.
        uint32 const elapsed =
            static_cast<uint32>(absorption->GetMaxDuration() - absorption->GetDuration());
        shadowmoonReaverAbsorptionStart[instanceId][guid] = getMSTime() - elapsed;
        return true;
    }

    auto const instanceIt = shadowmoonReaverAbsorptionStart.find(instanceId);
    if (instanceIt == shadowmoonReaverAbsorptionStart.end())
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

std::unordered_map<uint32, std::vector<NajentusSpineAssignment>> najentusSpineAssignments;
std::unordered_map<uint32, ObjectGuid> najentusSpineThrower;

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

    auto const it = najentusSpineAssignments.find(bot->GetInstanceId());
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsNajentusImpaled(member))
            continue;

        bool assigned = false;
        if (it != najentusSpineAssignments.end())
        {
            for (NajentusSpineAssignment const& assignment : it->second)
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

    auto const it = najentusSpineAssignments.find(bot->GetInstanceId());

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

        if (it != najentusSpineAssignments.end() &&
            std::any_of(it->second.begin(), it->second.end(),
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
    auto const it = najentusSpineAssignments.find(bot->GetInstanceId());
    if (it == najentusSpineAssignments.end())
        return nullptr;

    for (NajentusSpineAssignment const& assignment : it->second)
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
    auto const it = najentusSpineThrower.find(bot->GetInstanceId());
    return it != najentusSpineThrower.end() && it->second == bot->GetGUID();
}

Player* GetNajentusSpineThrower(Player* bot)
{
    auto const it = najentusSpineThrower.find(bot->GetInstanceId());
    if (it == najentusSpineThrower.end())
        return nullptr;

    Player* thrower = ObjectAccessor::GetPlayer(*bot, it->second);
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

// Supremus

namespace
{

float GetSupremusHazardRadius(SupremusHazard const& hazard, SupremusHazardZone zone)
{
    return zone == SupremusHazardZone::Damage ? hazard.damageRadius : hazard.safeRadius;
}

}

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
            !GET_PLAYERBOT_AI(member) ||
            member->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)) ||
            !PlayerbotAI::IsRanged(member))
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

std::unordered_map<uint32, uint32> councilDpsWaitTimer;
std::unordered_map<ObjectGuid, uint8> zerevorHealStep;

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

std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
std::unordered_map<ObjectGuid, ObjectGuid> illidanShadowTrapGuid;
std::unordered_map<ObjectGuid, Position> illidanShadowTrapDestination;
std::unordered_map<uint32, int> illidanLastPhase;
std::unordered_map<uint32, uint32> illidanBossDpsWaitTimer;
std::unordered_map<uint32, uint32> illidanFlameDpsWaitTimer;
std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
std::unordered_map<uint32, ObjectGuid> westFlameGuid;

int GetIllidanPhase(Unit* illidan)
{
    if (!illidan || IsIllidanDeathScene(illidan) ||
        illidan->HasAura(Id(BtSpells::SPELL_SHADOW_PRISON)))
    {
        return -1;
    }

    // Transitioning from Phase 2 to Phase 3
    float x, y, z;
    illidan->GetMotionMaster()->GetDestination(x, y, z);
    Position const dest(x, y, z);
    if ((dest.GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
         illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f) &&
        illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return 0;
    }

    // Phase 2: Flying
    if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 2;

    // Phase 1: Health > 65%
    if (illidan->GetHealthPct() > 65.0f)
        return 1;

    // Phase 4: Demon Form
    if (!illidan->HasAura(Id(BtSpells::SPELL_CAGED)) &&
        (illidan->HasAura(Id(BtSpells::SPELL_DEMON_FORM)) ||
         illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_1)) ||
         illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_2)) ||
         illidan->HasAura(Id(BtSpells::SPELL_DEMON_TRANSFORM_3))))
    {
        return 4;
    }

    // Phase 3: Normal (ground, 65-30%, not demon)
    if (illidan->GetHealthPct() > 30.0f)
        return 3;

    // Phase 5: Health <= 30%
    if (illidan->GetHealthPct() <= 30.0f)
        return 5;

    return -1;
}

bool IsIllidanDeathScene(Unit* illidan)
{
    return illidan && illidan->GetHealth() == 1;
}

std::vector<Unit*> GetAllFlameCrashes(Player* bot)
{
    std::vector<Unit*> flameCrashes;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BtNpcs::NPC_FLAME_CRASH), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            flameCrashes.push_back(creature);
    }

    return flameCrashes;
}

std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot)
{
    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    if (eastFlameGuid.find(instanceId) != eastFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, eastFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                eastFlame = unit;
        }
    }

    if (westFlameGuid.find(instanceId) != westFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, westFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                westFlame = unit;
        }
    }

    return { eastFlame, westFlame };
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
ObjectGuid FindIllidanWarlockTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    Player* fallbackWarlock = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive() ||
            member->getClass() != CLASS_WARLOCK)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock ? fallbackWarlock->GetGUID() : ObjectGuid::Empty;
}

Player* GetIllidanWarlockTank(PlayerbotAI* botAI)
{
    return GetCachedPlayer(botAI, "illidan stormrage warlock tank");
}

bool HasParasiticShadowfiend(Player* player)
{
    if (!player)
        return false;

    return player->HasAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_1)) ||
        player->HasAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_2));
}

// The first bot hunter that doesn't have Parasitic Shadowfiend
bool IsIllidanTrapperHunter(Player* bot)
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !member->IsAlive())
            continue;

        if (member->getClass() != CLASS_HUNTER || !GET_PLAYERBOT_AI(member) ||
            HasParasiticShadowfiend(member))
        {
            continue;
        }

        return member == bot;
    }

    return false;
}

ObjectGuid FindBotWithParasiticShadowfiendGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) &&
            HasParasiticShadowfiend(member))
        {
            return member->GetGUID();
        }
    }

    return ObjectGuid::Empty;
}

Player* GetBotWithParasiticShadowfiend(PlayerbotAI* botAI)
{
    return GetCachedPlayer(botAI, "illidan stormrage bot with parasitic shadowfiend");
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
    eyeBlastTrigger->GetMotionMaster()->GetDestination(destX, destY, destZ);
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
    float const length = area.start.GetExactDist2d(area.end.GetPositionX(), area.end.GetPositionY());

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

GameObject* FindNearestTrap(PlayerbotAI* botAI)
{
    GuidVector const& gos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();

    GameObject* nearestTrap = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->isSpawned() && go->GetEntry() == Id(BtObjects::GO_SHADOW_TRAP))
        {
            nearestTrap = go;
            break;
        }
    }

    return nearestTrap;
}

}
