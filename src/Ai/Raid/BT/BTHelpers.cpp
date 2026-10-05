/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "PathGenerator.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

using namespace EncounterHelpers;

namespace BlackTempleHelpers
{

namespace
{

Player* GetCachedPlayer(PlayerbotAI* botAI, char const* value)
{
    Unit* unit = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>(value)->Get());
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return player && player->IsAlive() ? player : nullptr;
}

std::vector<Unit*> GetCachedUnits(PlayerbotAI* botAI, char const* value)
{
    std::vector<Unit*> units;
    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>(value)->RefGet())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive())
            units.push_back(unit);
    }

    return units;
}

}

// General

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!botAI->GetBot()->HasAura(Id(BlackTempleSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
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
    if (!unit || unit->GetEntry() != Id(BlackTempleNpcs::NPC_SISTER_OF_PLEASURE) ||
        !unit->IsAlive())
    {
        return false;
    }

    Aura* bonds = unit->GetAura(Id(BlackTempleSpells::SPELL_SHARED_BONDS));
    Unit* pain = bonds ? bonds->GetCaster() : nullptr;
    return pain && pain->IsAlive();
}

Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI)
{
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet();
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (IsLinkedSisterOfPleasure(unit))
            return unit;
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
            unit->GetEntry() == Id(BlackTempleNpcs::NPC_SHADOWMOON_REAVER))
        {
            reavers.push_back(guid);
        }
    }

    return reavers;
}

bool IsShadowmoonReaverUnsafeForMagic(Unit* unit)
{
    if (!unit || unit->GetEntry() != Id(BlackTempleNpcs::NPC_SHADOWMOON_REAVER) ||
        !unit->IsAlive())
    {
        return false;
    }

    ObjectGuid const guid = unit->GetGUID();
    uint32 const instanceId = unit->GetInstanceId();
    if (Aura* absorption = unit->GetAura(Id(BlackTempleSpells::SPELL_SPELL_ABSORPTION)))
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

void AppendShadowmoonReaverExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsCaster(bot) && !PlayerbotAI::IsHeal(bot))
        return;

    auto const& reavers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    for (ObjectGuid const& guid : reavers)
    {
        if (IsShadowmoonReaverUnsafeForMagic(botAI->GetUnit(guid)))
            exclusions.insert(guid);
    }
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
            unit->GetEntry() != Id(BlackTempleNpcs::NPC_SHADOWMOON_REAVER) &&
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

namespace
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
        bot->HasItemCount(Id(BlackTempleItems::ITEM_NAJENTUS_SPINE));
}

}

bool IsNajentusImpaled(Player* player)
{
    return player && player->IsAlive() &&
        player->HasAura(Id(BlackTempleSpells::SPELL_IMPALING_SPINE));
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

bool IsSupremusKitePhase(Unit* supremus)
{
    return supremus && supremus->HasAura(Id(BlackTempleSpells::SPELL_SNARE_SELF));
}

GuidVector FindSupremusVolcanoGuids(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BlackTempleNpcs::NPC_SUPREMUS_VOLCANO), SUPREMUS_VOLCANO_SEARCH_RADIUS);

    GuidVector volcanoes;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            volcanoes.push_back(creature->GetGUID());
    }

    return volcanoes;
}

std::vector<Unit*> GetSupremusVolcanoes(PlayerbotAI* botAI)
{
    return GetCachedUnits(botAI, "supremus volcanoes");
}

bool IsSupremusVolcanoErupting(Unit* volcano)
{
    return volcano && volcano->IsAlive() &&
        (volcano->HasAura(Id(BlackTempleSpells::SPELL_VOLCANIC_GEYSER)) ||
         volcano->HasUnitState(UNIT_STATE_CASTING));
}

bool IsInEruptingSupremusVolcano(
    std::vector<Unit*> const& volcanoes, float x, float y, float radius)
{
    for (Unit* volcano : volcanoes)
    {
        if (IsSupremusVolcanoErupting(volcano) && volcano->GetExactDist2d(x, y) < radius)
        {
            return true;
        }
    }

    return false;
}

bool IsInsideSupremusKiteBoundary(float x, float y)
{
    return x > SUPREMUS_BOUNDARY_MIN_X + SUPREMUS_KITE_BOUNDARY_MARGIN &&
        x < SUPREMUS_BOUNDARY_MAX_X - SUPREMUS_KITE_BOUNDARY_MARGIN &&
        y > SUPREMUS_BOUNDARY_MIN_Y + SUPREMUS_KITE_BOUNDARY_MARGIN &&
        y < SUPREMUS_BOUNDARY_MAX_Y - SUPREMUS_KITE_BOUNDARY_MARGIN;
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

    for (BlackTempleNpcs const entry :
         { BlackTempleNpcs::NPC_ASHTONGUE_CHANNELER, BlackTempleNpcs::NPC_ASHTONGUE_SORCERER })
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

void AppendShadeOfAkamaTankExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsTank(botAI->GetBot()))
        return;

    auto const& adds =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shade of akama adds")->RefGet();
    exclusions.insert(adds.begin(), adds.end());
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

bool CastVengefulSpiritSpell(Unit* spirit, Unit* target, uint32 spellId)
{
    static constexpr std::array spiritSpells = {
        Id(BlackTempleSpells::SPELL_SPIRIT_STRIKE),
        Id(BlackTempleSpells::SPELL_SPIRIT_LANCE),
        Id(BlackTempleSpells::SPELL_SPIRIT_CHAINS),
        Id(BlackTempleSpells::SPELL_SPIRIT_VOLLEY),
        Id(BlackTempleSpells::SPELL_SPIRIT_SHIELD),
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
            member->HasAura(Id(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)) ||
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
// Just after a cast, that group's debuffs run out before the next one.
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
            if (Aura* bloodboil = member->GetAura(Id(BlackTempleSpells::SPELL_BLOODBOIL)))
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

float FindGurtoggSecondTankThreat(PlayerbotAI* botAI)
{
    Unit* gurtogg = botAI->GetAiObjectContext()
                        ->GetValue<Unit*>("find target", "gurtogg bloodboil")
                        ->Get();
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

Position GetSufferingMeleePosition(Player* bot, Unit* suffering)
{
    float const distance = bot->GetMeleeRange(suffering);
    float const behindAngle = Position::NormalizeOrientation(suffering->GetOrientation() + M_PI);
    return Position(
        suffering->GetPositionX() + distance * std::cos(behindAngle),
        suffering->GetPositionY() + distance * std::sin(behindAngle), bot->GetPositionZ());
}

bool IsOutOfSufferingPosition(Player* bot, Unit* suffering)
{
    if (!suffering)
        return false;

    if (IsSufferingFixateTank(bot))
        return bot->GetExactDist2d(suffering) > SUFFERING_TANK_DISTANCE;

    if (PlayerbotAI::IsMelee(bot))
    {
        return bot->GetExactDist2d(GetSufferingMeleePosition(bot, suffering)) >
            SUFFERING_MELEE_POSITION_TOLERANCE;
    }

    return PlayerbotAI::IsRanged(bot) && bot->GetExactDist2d(suffering) < SUFFERING_RANGED_DISTANCE;
}

// Mother Shahraz

std::unordered_map<ObjectGuid, TankPositionState> shahrazTankStep;

TankPositionState GetShahrazTankPositionState(Player* bot)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return TankPositionState::Unknown;

    auto it = shahrazTankStep.find(mainTank->GetGUID());
    if (it != shahrazTankStep.end())
        return it->second;

    return TankPositionState::Unknown;
}

// Illidari Council

std::unordered_map<uint32, uint32> councilDpsWaitTimer;
std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
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

bool HasDangerousCouncilAura(Unit* unit)
{
    if (!unit)
        return false;

    static constexpr std::array dangerousAuras = {
        Id(BlackTempleSpells::SPELL_CONSECRATION),
        Id(BlackTempleSpells::SPELL_BLIZZARD),
        Id(BlackTempleSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 aura : dangerousAuras)
    {
        if (unit->HasAura(aura))
            return true;
    }

    return false;
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
    if (!illidan || illidan->GetHealth() == 1 ||
        illidan->HasAura(Id(BlackTempleSpells::SPELL_SHADOW_PRISON)))
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
    if (!illidan->HasAura(Id(BlackTempleSpells::SPELL_CAGED)) &&
        (illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_FORM)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_1)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_2)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_3))))
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

std::vector<Unit*> GetAllFlameCrashes(Player* bot)
{
    std::vector<Unit*> flameCrashes;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BlackTempleNpcs::NPC_FLAME_CRASH), searchRadius);

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

    return player->HasAura(Id(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_1)) ||
        player->HasAura(Id(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_2));
}

// Get the first bot hunter that doesn't have Parasitic Shadowfiend
Player* GetIllidanTrapperHunter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member) &&
            !HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

    return nullptr;
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
        creatureList, Id(BlackTempleNpcs::NPC_ILLIDAN_DB_TARGET), searchRadius);

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
        if (go && go->isSpawned() && go->GetEntry() == Id(BlackTempleObjects::GO_SHADOW_TRAP))
        {
            nearestTrap = go;
            break;
        }
    }

    return nearestTrap;
}

}
