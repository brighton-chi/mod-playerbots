/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCActions.h"
#include "CharmInfo.h"
#include "Corpse.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SSCHelpers.h"
#include "TemporarySummon.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>
#include <unordered_map>
#include <utility>

using namespace SscHelpers;
using namespace EncounterHelpers;

// General

bool SscResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    bool reset = false;

    reset |= intendedVashjCorePasserLineup.erase(guid) > 0;
    reset |= lastVashjCoreInInventoryTime.erase(guid) > 0;

    Action* vashjSpreadAction = context->GetAction("lady vashj phase 1 spread ranged in arc");
    if (vashjSpreadAction && static_cast<LadyVashjPhase1SpreadRangedInArcAction*>(
            vashjSpreadAction)->ResetRangedPosition())
    {
        reset = true;
    }

    Action* lurkerSpreadAction = context->GetAction("the lurker below spread ranged in arc");
    if (lurkerSpreadAction && static_cast<TheLurkerBelowSpreadRangedInArcAction*>(
            lurkerSpreadAction)->ResetRangedPosition())
    {
        reset = true;
    }

    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return reset;

    reset |= vashjClusterHolders.erase(instanceId) > 0;
    reset |= vashjTaintedCoreLooter.erase(instanceId) > 0;
    reset |= lastVashjCoreImbueAttempt.erase(instanceId) > 0;
    reset |= karathressDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasWhirlwindEndTime.erase(instanceId) > 0;
    reset |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= lurkerGuardianTankAssignments.erase(instanceId) > 0;
    reset |= hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0;
    reset |= hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0;
    reset |= hydrossNatureDpsWaitTimer.erase(instanceId) > 0;
    reset |= hydrossFrostDpsWaitTimer.erase(instanceId) > 0;

    if (!AI_VALUE2(bool, "combat", "self target"))
    {
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);
        reset |= ClearTargetIcon(bot, RtiTargetValue::crossIndex);
    }

    return reset;
}

// Trash Mobs

bool UnderbogColossusEscapeToxicPoolAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetToxicPoolPosition(botAI, pool))
        return false;

    constexpr float moveDist = 5.0f;
    float stepX;
    float stepY;
    float stepZ;
    if (!FindHazardEscapeStep(bot, pool, moveDist, stepX, stepY, stepZ))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    Creature* totem =
        bot->FindNearestCreature(Id(SscNpcs::NPC_WATER_ELEMENTAL_TOTEM), searchRadius);
    return totem && MarkTargetWithSkull(bot, totem);
}

// Shared Bosses

bool SscMisdirectTargetToTankAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE2(Unit*, "find target", _targetName);
    if (!target)
        return false;

    Player* tank = _assistTankIndex == MAIN_TANK ?
        GetGroupMainTank(bot) : GetGroupAssistTank(bot, _assistTankIndex);
    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstablePositionAndSwapTanksAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    bool const myPhase = _frostTank ?
        IsHydrossInFrostPhase(hydross) : IsHydrossInNaturePhase(hydross);
    bool const markMaxed =
        _frostTank ? HasMarkOfHydrossAt100Percent(bot) : HasMarkOfCorruptionAt100Percent(bot);

    Position const& myPosition = _frostTank ?
        HYDROSS_FROST_TANK_POSITION : HYDROSS_NATURE_TANK_POSITION;
    Position const& otherPosition = _frostTank ?
        HYDROSS_NATURE_TANK_POSITION : HYDROSS_FROST_TANK_POSITION;
    std::unordered_map<uint32, uint32> const& handOverTimer =
        _frostTank ? hydrossChangeToNaturePhaseTimer : hydrossChangeToFrostPhaseTimer;

    if (!myPhase)
        return StepTo(myPosition, hydross);

    if (!markMaxed)
    {
        if (AI_VALUE(Unit*, "current target") != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() != bot || !bot->IsWithinMeleeRange(hydross))
            return false;

        return StepTo(myPosition, hydross);
    }

    if (hydross->GetVictim() != bot || !bot->IsWithinMeleeRange(hydross))
        return false;

    constexpr uint32 phaseChangeDelayMs = 1 * IN_MILLISECONDS;
    auto it = handOverTimer.find(hydross->GetInstanceId());
    if (it != handOverTimer.end() && getMSTimeDiff(it->second, getMSTime()) >= phaseChangeDelayMs)
        return StepTo(otherPosition, hydross);

    bot->AttackStop();
    bot->CastStop();
    return true;
}

bool HydrossTheUnstablePositionAndSwapTanksAction::StepTo(Position const& position, Unit* hydross)
{
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, hydross, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// To mitigate the effect of Water Tomb
bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event /*event*/)
{
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return false;

    constexpr float safeDistance = 6.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistance);
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    Player* tank = nullptr;
    if (HasNoMarkOfHydross(bot) && IsHydrossInFrostPhase(hydross))
        tank = GetGroupMainTank(bot);
    else if (HasNoMarkOfCorruption(bot) && IsHydrossInNaturePhase(hydross))
        tank = GetGroupAssistTank(bot, 0);

    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", hydross) && botAI->CastSpell("steady shot", hydross);
}

bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();
    constexpr uint32 phaseStartStopMs = 5 * IN_MILLISECONDS;
    constexpr uint32 phaseEndStopMs = 1 * IN_MILLISECONDS;
    bool const isHunter = bot->getClass() == CLASS_HUNTER;

    bool shouldStopDps = false;

    // 1 second after 100% Mark of Hydross, stop dps.
    auto itNature = hydrossChangeToNaturePhaseTimer.find(instanceId);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() &&
        getMSTimeDiff(itNature->second, now) >= phaseEndStopMs)
    {
        shouldStopDps = true;
    }

    // Keep dps stopped for 5 seconds after transitioning into nature phase.
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(instanceId);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() &&
        getMSTimeDiff(itNatureDps->second, now) < phaseStartStopMs)
    {
        shouldStopDps = !isHunter;
    }

    // 1 second after 100% Mark of Corruption, stop dps.
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(instanceId);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() &&
        getMSTimeDiff(itFrost->second, now) >= phaseEndStopMs)
    {
        shouldStopDps = true;
    }

    // Keep dps stopped for 5 seconds after transitioning into frost phase.
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(instanceId);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() &&
        getMSTimeDiff(itFrostDps->second, now) < phaseStartStopMs)
    {
        shouldStopDps = !isHunter;
    }

    if (!shouldStopDps)
        return false;

    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());

    return true;
}

bool HydrossTheUnstableManagePhaseTimersAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();

    bool updated = false;

    if (IsHydrossInFrostPhase(hydross))
    {
        updated |= hydrossFrostDpsWaitTimer.try_emplace(instanceId, now).second;
        updated |= hydrossNatureDpsWaitTimer.erase(instanceId) > 0;
        updated |= hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0;

        if (HasMarkOfHydrossAt100Percent(bot))
            updated |= hydrossChangeToNaturePhaseTimer.try_emplace(instanceId, now).second;
    }
    else // Nature phase
    {
        updated |= hydrossNatureDpsWaitTimer.try_emplace(instanceId, now).second;
        updated |= hydrossFrostDpsWaitTimer.erase(instanceId) > 0;
        updated |= hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0;

        if (HasMarkOfCorruptionAt100Percent(bot))
            updated |= hydrossChangeToFrostPhaseTimer.try_emplace(instanceId, now).second;
    }

    return updated;
}

// The Lurker Below

// Runnin', runnin', runnin', I'm runnin' over here, run, run, run-run, run.
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    // Randomize the radius for each bot so the running looks a bit more natural.
    uint32 const seed = bot->GetGUID().GetCounter();
    float const runRadius = LURKER_SPOUT_RUN_RADIUS_MIN +
        (LURKER_SPOUT_RUN_RADIUS_MAX - LURKER_SPOUT_RUN_RADIUS_MIN) * (seed % 100) / 100.0f;

    float const distance = bot->GetExactDist2d(lurker);
    float const botAngle = std::atan2(
        bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    float const relative = Position::NormalizeOrientation(botAngle - lurker->GetOrientation());
    bool const inArc =
        std::fabs(relative - static_cast<float>(M_PI)) <= LURKER_SPOUT_RUN_ARC_HALF_WIDTH;

    float const lurkerX = lurker->GetPositionX();
    float const lurkerY = lurker->GetPositionY();
    float const lurkerZ = lurker->GetPositionZ();

    // Lurker is spinning. The only nuance is that bots will not advance more than 30 degrees
    // past directly behind the boss as Sprinting/Spirit Walking can otherwise lap the spin.
    if (int8 const spin = GetLurkerSpoutSpin(lurker))
    {
        float const aheadOfBeam = spin > 0 ? relative : 2.0f * static_cast<float>(M_PI) - relative;
        float const room =
            static_cast<float>(M_PI) + LURKER_SPOUT_RUN_OVERTAKE_MARGIN - aheadOfBeam;
        float const stepAngle = std::min(LURKER_SPOUT_RUN_STEP / runRadius, room);
        constexpr float minStep = 2.0f;
        if (stepAngle * runRadius < minStep)
            return false;

        float const radialStep = std::clamp(
            runRadius - distance, -LURKER_SPOUT_RUN_STEP, LURKER_SPOUT_RUN_STEP);
        float const moveRadius = distance + radialStep;
        float const moveAngle = botAngle + spin * stepAngle;

        bot->CastStop();
        return MoveTo(
            SSC_MAP_ID, lurkerX + moveRadius * std::cos(moveAngle),
            lurkerY + moveRadius * std::sin(moveAngle), lurkerZ, false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Lurker is winding-up, bot is behind the boss: Get to the right radius and wait for the
    // direction of the spin to be determined.
    if (inArc)
    {
        if (std::fabs(distance - runRadius) < LURKER_SPOUT_RUN_RADIAL_DEADZONE)
            return false;

        return MoveTo(
            SSC_MAP_ID, lurkerX + runRadius * std::cos(botAngle),
            lurkerY + runRadius * std::sin(botAngle), lurkerZ, false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Lurker is winding-up, bot is in front of the boss: One far move to the nearer arc edge so the
    // bot is not in front of the boss when the Spout starts, whatever direction it goes. This has
    // to be a lower movement priority than the run during the spin phase.
    int8 const direction = relative < M_PI ? 1 : -1;
    float const edgeAngle = lurker->GetOrientation() + static_cast<float>(M_PI) -
        direction * LURKER_SPOUT_RUN_ARC_HALF_WIDTH;
    float const edgeX = lurkerX + runRadius * std::cos(edgeAngle);
    float const edgeY = lurkerY + runRadius * std::sin(edgeAngle);

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    bot->CastStop();
    if (DoesPathRoundLurker(bot, lurker, edgeX, edgeY, lurkerZ, direction))
    {
        return MoveTo(
            SSC_MAP_ID, edgeX, edgeY, lurkerZ, false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    float const stepAngle = botAngle + direction * LURKER_SPOUT_RUN_STEP / runRadius;
    return MoveTo(
        SSC_MAP_ID, lurkerX + runRadius * std::cos(stepAngle),
        lurkerY + runRadius * std::sin(stepAngle), lurkerZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Position the main tank in front of a pillar.
bool TheLurkerBelowPositionMainTankAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (AI_VALUE(Unit*, "current target") != lurker)
        return Attack(lurker);

    if (lurker->GetVictim() != bot)
        return false;

    Position const& position = LURKER_MAIN_TANK_POSITION;
    constexpr float arrivalDist = 1.0f;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    constexpr float pathTolerance = 3.0f;
    if (!DoesPathArrive(
            bot, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            pathTolerance))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Assign ranged positions within a 120-degree arc behind Lurker.
bool TheLurkerBelowSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (!_hasRangedPosition)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        std::vector<Player*> rangedMembers;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SSC_MAP_ID || !GET_PLAYERBOT_AI(member) ||
                !PlayerbotAI::IsRanged(member))
            {
                continue;
            }

            rangedMembers.push_back(member);
        }

        if (rangedMembers.empty())
            return false;

        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ?
            std::distance(rangedMembers.begin(), findIt) : 0;

        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.262f; // measured in game to be across from the main tank
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = lurker->GetPositionX() + LURKER_RANGED_SAFE_DISTANCE * std::sin(angle);
        float targetY = lurker->GetPositionY() + LURKER_RANGED_SAFE_DISTANCE * std::cos(angle);

        _rangedPosition = Position(targetX, targetY, lurker->GetPositionZ());
        _hasRangedPosition = true;
    }

    Position const& position = _rangedPosition;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    // Incremental movement does not work if the bot is in the water (there is no walkable height
    // and MoveTo returns false). Therefore, this block calls a MoveTo directly to the position.
    if (!IsDryGround(bot, moveX, moveY))
    {
        return MoveTo(
            SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, lurker->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// During the submerge phase, the main tank and the first two assist tanks each grab one Coilfang
// Guardian. This runs only if there are at least three bot tanks. Otherwise, normal tank assist is
// relied on to pick up the Guardians.
bool TheLurkerBelowTanksPickUpAddsAction::Execute(Event /*event*/)
{
    std::vector<Unit*> const guardians = GetLurkerGuardians(botAI);
    if (guardians.empty())
        return false;

    std::vector<Player*> const tanks = GetLurkerGuardianTanks(bot);
    auto const myIt = std::find(tanks.begin(), tanks.end(), bot);
    if (myIt == tanks.end())
        return false;

    size_t const myIndex = static_cast<size_t>(std::distance(tanks.begin(), myIt));

    Unit* guardian = botAI->GetUnit(ClaimGuardianForTank(guardians, myIndex));
    if (!guardian || !guardian->IsAlive())
        return false;

    if (AI_VALUE(Unit*, "current target") != guardian)
        return Attack(guardian);

    if (guardian->GetVictim() == bot)
        return false;

    char const* taunt = nullptr;
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT: taunt = "dark command"; break;
        case CLASS_DRUID:        taunt = "growl"; break;
        case CLASS_PALADIN:      taunt = "hand of reckoning"; break;
        case CLASS_WARRIOR:      taunt = "taunt"; break;
        default:                 return false;
    }

    return botAI->CanCastSpell(taunt, guardian) && botAI->CastSpell(taunt, guardian);
}

ObjectGuid TheLurkerBelowTanksPickUpAddsAction::ClaimGuardianForTank(
    std::vector<Unit*> const& guardians, size_t myIndex)
{
    auto& assignments = lurkerGuardianTankAssignments[bot->GetInstanceId()];
    ObjectGuid& assignedGuid = assignments[myIndex];

    if (std::any_of(guardians.begin(), guardians.end(),
            [&assignedGuid](Unit* guardian) { return guardian->GetGUID() == assignedGuid; }))
    {
        return assignedGuid;
    }

    auto const heldByAnotherTank = [&assignments, myIndex](ObjectGuid guid)
    {
        for (size_t i = 0; i < assignments.size(); ++i)
        {
            if (i != myIndex && assignments[i] == guid)
                return true;
        }

        return false;
    };

    assignedGuid = ObjectGuid::Empty;

    for (Unit* guardian : guardians)
    {
        if (!heldByAnotherTank(guardian->GetGUID()))
        {
            assignedGuid = guardian->GetGUID();
            break;
        }
    }

    return assignedGuid;
}

bool TheLurkerBelowMeleeMoveDirectlyToTargetAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->IsWithinMeleeRange(target))
        return false;

    // MoveTo rejects any destination over water (no map height), so the destination has to be dry
    // land. The Ambushers sometimes chill at the edge of their islets so we issue the move to stop
    // right on top of them. When returning to Lurker, target the move at the walkway instead.
    bool const isAmbusher = target->GetEntry() == Id(SscNpcs::NPC_COILFANG_AMBUSHER);
    float const targetDistance = isAmbusher ? 0.0f : LURKER_SPOUT_RUN_RADIUS_MAX;
    float const angle = target->GetAngle(bot);
    float const destX = target->GetPositionX() + std::cos(angle) * targetDistance;
    float const destY = target->GetPositionY() + std::sin(angle) * targetDistance;

    return MoveTo(
        SSC_MAP_ID, destX, destY, target->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Leotheras the Blind

// Warlock tank action: see GetLeotherasWarlockTank in SSCHelpers.cpp.
bool LeotherasTheBlindWarlockTankAttackBossAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_VIGILANCE)))
    {
        bot->RemoveOwnedAura(
            Id(SscSpells::SPELL_VIGILANCE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    return botAI->CanCastSpell("searing pain", leotherasDemon) &&
        botAI->CastSpell("searing pain", leotherasDemon);
}

// With a Warlock tanking the demon form, the melee tanks only auto-attack it: their abilities are
// held by the multiplier, so the rage they bank is there in full when an Inner Demon lands on
// them. Rage does not decay in combat, and white hits alone cannot outthreat Searing Pain.
bool LeotherasTheBlindTanksBuildRageOnDemonFormAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetPhase2LeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    return AI_VALUE(Unit*, "current target") != leotherasDemon && Attack(leotherasDemon);
}

// Intent is to keep enough distance from Leotheras and spread to prepare for Whirlwind
// And stay away from the Warlock tank to avoid Chaos Blasts
bool LeotherasTheBlindPositionRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistFromBoss = 15.0f;
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (leotherasHumanoid && !HasInnerDemon(bot) && leotherasHumanoid->GetVictim() != bot &&
        bot->GetExactDist2d(leotherasHumanoid) < safeDistFromBoss)
    {
        if (FleePosition(leotherasHumanoid->GetPosition(), safeDistFromBoss))
            return true;
    }

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    // Chaos Blast deals splash damage within 8y of the target.
    constexpr float safeDistFromBlast = 10.0f;
    Unit* demonVictim = leotherasDemon->GetVictim();
    Unit* warlockTank = GetLeotherasWarlockTank(bot);

    Unit* fleeFrom = nullptr;
    if (demonVictim && demonVictim != bot && bot->GetExactDist2d(demonVictim) < safeDistFromBlast)
    {
        fleeFrom = demonVictim;
    }
    else if (warlockTank && warlockTank != bot &&
        bot->GetExactDist2d(warlockTank) < safeDistFromBlast)
    {
        fleeFrom = warlockTank;
    }

    if (!fleeFrom)
        return false;

    return FleePosition(fleeFrom->GetPosition(), safeDistFromBlast, 0);
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid)
        return false;

    float const currentDistance = bot->GetExactDist2d(leotherasHumanoid);
    constexpr float safeDistance = 25.0f;
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(leotherasHumanoid, safeDistance - currentDistance);
}

// This method is likely unnecessary unless the player does not use a Warlock tank.
// But if a melee tank is used, other melee needs to run away after too many Chaos Blast stacks.
bool LeotherasTheBlindMeleeRunAwayFromChaosBlastAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE &&
        botAI->CanCastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot))
    {
        return true;
    }

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    Unit* demonVictim = leotherasDemon->GetVictim();
    if (!demonVictim || demonVictim == bot)
        return false;

    float const currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 10.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(demonVictim, safeDistance - currentDistance);
}

bool LeotherasTheBlindDestroyInnerDemonAction::Execute(Event /*event*/)
{
    Creature* innerDemon = GetPersonalInnerDemon(botAI);
    if (!innerDemon)
        return false;

    // All classes and specs swap their autoattack to the Inner Demon.
    if (AI_VALUE(Unit*, "current target") != innerDemon)
    {
        bot->CastStop();
        return Attack(innerDemon);
    }

    // Specific classes and specs have hardcoded methods to kill their Inner Demons.
    if (bot->getClass() == CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return HandleFeralTankStrategy(innerDemon);

    if (bot->getClass() == CLASS_HUNTER)
        return HandleHunterStrategy(innerDemon);

    if (PlayerbotAI::IsHeal(bot))
        return HandleHealerStrategy(innerDemon);

    return false;
}

// Bears have trouble killing their Inner Demons when damage is nerfed with IP, so this rotation
// tries to maximize single-target damage over 30s.
bool LeotherasTheBlindDestroyInnerDemonAction::HandleFeralTankStrategy(Unit* innerDemon)
{
    constexpr uint32 faerieFire = Id(SscSpells::SPELL_FAERIE_FIRE_FERAL);
    if (!innerDemon->HasAura(faerieFire) && botAI->CanCastSpell(faerieFire, innerDemon) &&
        botAI->CastSpell(faerieFire, innerDemon))
    {
        return true;
    }

    bool const isBelowEnrageRageThreshold = bot->GetPower(POWER_RAGE) < 70;
    if (isBelowEnrageRageThreshold &&
        botAI->CanCastSpell("enrage", bot) && botAI->CastSpell("enrage", bot))
    {
        return true;
    }

    if (botAI->CanCastSpell(Id(SscSpells::SPELL_DRUID_BERSERK), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_DRUID_BERSERK), bot))
    {
        return true;
    }

    if (botAI->CanCastSpell("mangle (bear)", innerDemon) &&
        botAI->CastSpell("mangle (bear)", innerDemon))
    {
        return true;
    }

    // The first cast of Faerie Fire (Feral) is to apply the armor debuff. After that, cast on CD
    // (but lower priority than Mangle) just for damage.
    if (botAI->CanCastSpell(faerieFire, innerDemon) && botAI->CastSpell(faerieFire, innerDemon))
        return true;

    Aura const* whisper = bot->GetAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER));
    if (!whisper)
        return false;

    bool const isAboveMaulRageThreshold = bot->GetPower(POWER_RAGE) > 40;
    constexpr int32 maulFreeUseMs = 4 * IN_MILLISECONDS;
    return (isAboveMaulRageThreshold || whisper->GetDuration() < maulFreeUseMs) &&
        botAI->CanCastSpell("maul", innerDemon) && botAI->CastSpell("maul", innerDemon);
}

// Hunters can have a bit of trouble since they need to take down their Inner Demons in melee.
// This action ensures they have Hawk up and use Explosive Trap, which is the main source of
// damage against the Inner Demon in melee. Immolation Trap is used if Readiness is cast so the
// trap cooldown resets (since a second Explosive Trap will not stack its DoT).
bool LeotherasTheBlindDestroyInnerDemonAction::HandleHunterStrategy(Unit* innerDemon)
{
    if (!botAI->HasAura("aspect of the dragonhawk", bot) &&
        !botAI->HasAura("aspect of the hawk", bot))
    {
        if (botAI->CanCastSpell("aspect of the dragonhawk", bot) &&
            botAI->CastSpell("aspect of the dragonhawk", bot))
        {
            return true;
        }

        return botAI->CanCastSpell("aspect of the hawk", bot) &&
            botAI->CastSpell("aspect of the hawk", bot);
    }

    if (!botAI->HasAura("explosive trap effect", innerDemon))
    {
        return botAI->CanCastSpell("explosive trap", bot) &&
            botAI->CastSpell("explosive trap", bot);
    }

    return botAI->CanCastSpell("immolation trap", bot) && botAI->CastSpell("immolation trap", bot);
}

bool LeotherasTheBlindDestroyInnerDemonAction::HandleHealerStrategy(Unit* innerDemon)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (bot->HasAura(Id(SscSpells::SPELL_TREE_OF_LIFE)))
            {
                bot->RemoveOwnedAura(
                    Id(SscSpells::SPELL_TREE_OF_LIFE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
            }

            if (botAI->CanCastSpell("barkskin", bot) && botAI->CastSpell("barkskin", bot))
                return true;

            return botAI->CanCastSpell("wrath", innerDemon) &&
                botAI->CastSpell("wrath", innerDemon);

        case CLASS_PALADIN:
            if (botAI->CanCastSpell(Id(SscSpells::SPELL_AVENGING_WRATH), bot) &&
                botAI->CastSpell(Id(SscSpells::SPELL_AVENGING_WRATH), bot))
            {
                return true;
            }

            if (botAI->CanCastSpell("consecration", bot) && botAI->CastSpell("consecration", bot))
                return true;

            if (botAI->CanCastSpell("exorcism", innerDemon) &&
                botAI->CastSpell("exorcism", innerDemon))
            {
                return true;
            }

            if (botAI->CanCastSpell("hammer of wrath", innerDemon) &&
                botAI->CastSpell("hammer of wrath", innerDemon))
            {
                return true;
            }

            if (botAI->CanCastSpell("holy shock", innerDemon) &&
                botAI->CastSpell("holy shock", innerDemon))
            {
                return true;
            }

            return botAI->CanCastSpell("judgement of light", innerDemon) &&
                botAI->CastSpell("judgement of light", innerDemon);

        case CLASS_PRIEST:
            return botAI->CanCastSpell("smite", innerDemon) &&
                botAI->CastSpell("smite", innerDemon);

        case CLASS_SHAMAN:
            if (botAI->CanCastSpell("earth shock", innerDemon) &&
                botAI->CastSpell("earth shock", innerDemon))
            {
                return true;
            }

            if (botAI->CanCastSpell("chain lightning", innerDemon) &&
                botAI->CastSpell("chain lightning", innerDemon))
            {
                return true;
            }

            return botAI->CanCastSpell("lightning bolt", innerDemon) &&
                botAI->CastSpell("lightning bolt", innerDemon);

        default:
            return false;
    }
}

// Everybody except the Warlock tank should focus on Leotheras in Phase 3.
bool LeotherasTheBlindFinalPhaseAttackBossAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid)
        return false;

    return AI_VALUE(Unit*, "current target") != leotherasHumanoid && Attack(leotherasHumanoid);
}

// Leotheras's tank needs to keep him away from the Shadow's target (due to Chaos Blasts).
bool LeotherasTheBlindFinalPhaseSeparateBossFromDemonAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid || leotherasHumanoid->GetVictim() != bot)
        return false;

    Creature* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    Unit* demonVictim = leotherasDemon->GetVictim();
    if (!demonVictim || demonVictim == bot)
        return false;

    float const currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 20.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(demonVictim, safeDistance - currentDistance, true);
}

bool LeotherasTheBlindMisdirectBossToWarlockTankAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    // If there is no Warlock tank, then misdirect to the main tank.
    Player* tank = GetLeotherasWarlockTank(bot);
    if (!tank)
        tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", leotherasDemon) &&
        botAI->CastSpell("steady shot", leotherasDemon);
}

bool LeotherasTheBlindManageDpsWaitTimersAction::Execute(Event /*event*/)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    uint32 const instanceId = leotheras->GetInstanceId();
    uint32 const now = getMSTime();

    bool changed = false;

    if (IsLeotherasHumanoidPhase(botAI))
    {
        changed |= leotherasHumanoidPhaseDpsWaitTimer.try_emplace(instanceId, now).second;

        // Whirlwind resets threat on every tick. Hold dps for a moment after it ends. Do not hold
        // dps while Whirlwind is active.
        if (Aura const* whirlwind = leotheras->GetAura(Id(SscSpells::SPELL_WHIRLWIND)))
        {
            changed |= leotherasWhirlwindEndTime.try_emplace(
                instanceId, now + whirlwind->GetDuration()).second;
        }
        else if (auto it = leotherasWhirlwindEndTime.find(instanceId);
            it != leotherasWhirlwindEndTime.end())
        {
            // This addresses the situation in which Whirlwind ends early due to the transition into
            // the final phase being triggered.
            if (now < it->second)
            {
                it->second = now;
                changed = true;
            }
            else if (now - it->second >= LEOTHERAS_HUMANOID_DPS_WAIT_MS)
            {
                leotherasWhirlwindEndTime.erase(it);
                changed = true;
            }
        }

        changed |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    }
    else if (IsLeotherasDemonPhase(botAI))
    {
        changed |= leotherasDemonPhaseDpsWaitTimer.try_emplace(instanceId, now).second;
        changed |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasWhirlwindEndTime.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    }
    else if (IsLeotherasFinalPhase(botAI))
    {
        changed |= leotherasFinalPhaseDpsWaitTimer.try_emplace(instanceId, now).second;
        changed |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasWhirlwindEndTime.erase(instanceId) > 0;
        changed |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
    }

    return changed;
}

// Fathom-Lord Karathress
// Note: 4 tanks are required for the full strategy, and having at least 2
// is crucial to separate Caribdis from the others

// Karathress is tanked by the main tank near his starting position
// Caribdis is pulled far to the West in the corner by the first assist tank
// Sharkkis is pulled North to the other side of the ramp by the second assist tank
// Tidalvess is pulled Northwest near the pillar by the third assist tank
bool FathomLordKarathressTanksPositionTargetsAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position position;

    if (PlayerbotAI::IsMainTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
        position = KARATHRESS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        position = CARIBDIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        target = GetSharkkisTankTarget(botAI);
        position = SHARKKIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, false))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        position = TIDALVESS_TANK_POSITION;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    if (IsHoldingAnotherTanksCouncilMember(botAI))
        return false;

    constexpr float arrivalDist = 4.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, arrivalDist, target, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Caribdis's tank spot is far away so a dedicated healer is needed
// Use the assistant flag to select the healer
// Her tank stands on her, so seeing her is seeing the tank. Out of sight the walk goes on until
// she is in sight, as it does for ranged; in sight it closes only to the healing distance.
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event /*event*/)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    bool const inSight = bot->IsWithinLOSInMap(caribdis);
    if (inSight && bot->GetExactDist(caribdis) < CARIBDIS_HEALER_MAX_DISTANCE)
        return false;

    float const stopDistance =
        inSight ? CARIBDIS_HEALER_DISTANCE : CARIBDIS_APPROACH_STOP_DISTANCE;

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, caribdis, stopDistance, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == SSC_MAP_ID &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* enemy = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 1)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        tank = GetGroupAssistTank(bot, 2);
    }
    else if (hunterIndex == 2)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        tank = GetGroupAssistTank(bot, 1);
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", enemy) && botAI->CastSpell("steady shot", enemy);
}

// Kill order is non-standard because bots handle Cyclones poorly and need more time
// to get her down than real players (standard is ranged DPS help with Sharkkis first)
bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;

    // Karathress inherits the totem when Tidalvess dies, so it stays first for the whole fight,
    // for melee and for the ranged near it
    Unit* totem = GetSpitfireTotem(botAI);
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    if (ShouldAttackSpitfireTotem(bot, totem))
    {
        target = totem;
    }
    else if (tidalvess)
    {
        target = tidalvess;
    }
    else if (caribdis && PlayerbotAI::IsRanged(bot))
    {
        target = caribdis;
    }
    else if (Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis"))
    {
        target = sharkkis;
    }
    else if (Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat"))
    {
        target = fathomSporebat;
    }
    else if (Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker"))
    {
        target = fathomLurker;
    }
    else if (Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        // Only melee get here with Caribdis alive; they hold rather than hand him her Blessing
        if (caribdis && karathress->GetHealthPct() <= KARATHRESS_BLESSING_HOLD_HEALTH_PCT)
        {
            if (AI_VALUE(Unit*, "current target") != karathress)
                return false;

            bot->AttackStop();
            bot->InterruptSpell(CURRENT_MELEE_SPELL);
            bot->CastStop();
            context->GetValue<Unit*>("current target")->Set(nullptr);
            bot->SetSelection(ObjectGuid());
            return true;
        }

        target = karathress;
    }

    if (!target)
        return false;

    // Caribdis is tanked out of sight of the room. Attack refuses a target out of line of sight,
    // and a bot that loses sight of her after acquiring her is still in range, so nothing else
    // walks it back: she is approached along the path whenever she is out of sight.
    if (target == caribdis && !bot->IsWithinLOSInMap(caribdis))
        return ApproachCaribdis(caribdis);

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target == caribdis)
    {
        if (MarkTargetWithCross(bot, caribdis))
            return true;

        // Flee is held for the whole fight, so ranged keep out of Tidal Surge themselves
        if (bot->GetExactDist(caribdis) < CARIBDIS_RANGED_MIN_DISTANCE)
            return FleePosition(caribdis->GetPosition(), CARIBDIS_RANGED_MIN_DISTANCE);
    }
    // While a totem stands, skull stays on it: ranged out of its reach are on something else, and
    // marking that would flip the skull against the bots on the totem
    else if ((!totem || target == totem) && MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return false;
}

bool FathomLordKarathressAssignDpsPriorityAction::ApproachCaribdis(Unit* caribdis)
{
    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, caribdis, CARIBDIS_APPROACH_STOP_DISTANCE, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    return karathressDpsWaitTimer.try_emplace(karathress->GetInstanceId(), getMSTime()).second;
}

bool FathomLordKarathressSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, CARIBDIS_RANGED_SPREAD_DISTANCE);
    return nearestPlayer &&
        FleePosition(nearestPlayer->GetPosition(), CARIBDIS_RANGED_SPREAD_DISTANCE);
}

// A Cyclone tosses every second and the arc takes longer than that, so from the second toss on
// the knockback handler is working from a mid-air position: its raycast fails and the destination
// keeps the bot's own height. Nothing brings the bot down once the Cyclone leaves, but any
// ordinary move does, since its destination is on the ground: the bot is sent to the floor
// beneath it once the last arc has finished.
bool FathomLordKarathressDropFromCycloneAction::Execute(Event /*event*/)
{
    // The knockback builds its spline from wherever the bot is, and mid-air that raycast fails:
    // the spline never finishes, so its generator is never popped off the controlled slot, and a
    // bot with that slot taken refuses every move it is given. It is cleared by hand here.
    MotionMaster* mm = bot->GetMotionMaster();
    if (mm->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == EFFECT_MOTION_TYPE)
    {
        mm->Clear();
        bot->StopMoving();
    }

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const floorZ = bot->GetMapHeight(x, y, bot->GetPositionZ(), true, MAX_FALL_DISTANCE);

    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ <= CYCLONE_DROP_HEIGHT)
        return false;

    if (!bot->movespline->Finalized())
        return false;

    // Exact waypoint: a pathed move searches for a route from the bot's own position first, and a
    // bot lifted above the navmesh has none. The point generator falls back to a straight spline.
    return MoveTo(
        SSC_MAP_ID, x, y, floorZ, false, false, false, true,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Morogrim Tidewalker

// Separate tanking positions are used for phase 1 and phase 2 to address the Water Globule
// mechanic in phase 2
bool MorogrimTidewalkerPositionMainTankAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (AI_VALUE(Unit*, "current target") != tidewalker)
        return Attack(tidewalker);

    if (tidewalker->GetVictim() != bot || !bot->IsWithinMeleeRange(tidewalker))
        return false;

    if (tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return MoveToPhase1TankPosition(tidewalker);

    return MoveToPhase2TankPosition(tidewalker);
}

// Phase 1: tank position is up against the Northeast pillar
bool MorogrimTidewalkerPositionMainTankAction::MoveToPhase1TankPosition(Unit* tidewalker)
{
    constexpr float arrivalDist = 1.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, TIDEWALKER_PHASE_1_TANK_POSITION, arrivalDist, tidewalker, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Phase 2: the path takes the tank around the pillar and back up into the Northeast corner. The
// step is shortened when it runs away from the boss, since that one is walked backwards.
bool MorogrimTidewalkerPositionMainTankAction::MoveToPhase2TankPosition(Unit* tidewalker)
{
    Position const& phase2 = TIDEWALKER_PHASE_2_TANK_POSITION;
    constexpr float arrivalDist = 1.0f;

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(bot, phase2, arrivalDist, PATH_STEP_DISTANCE, stepX, stepY))
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    bool const backwards =
        (stepX - botX) * (tidewalker->GetPositionX() - botX) +
        (stepY - botY) * (tidewalker->GetPositionY() - botY) < 0.0f;

    if (backwards && !GetPathStepTowardPoint(
            bot, phase2, arrivalDist, PATH_BACKWARD_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Ranged stack behind the boss in the Northeast corner in phase 2
// No corresponding method for melee since they will do so anyway
bool MorogrimTidewalkerStackRangedBehindBossAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    // The point moves with him as the tank takes him to the corner, so ranged trail him there and
    // are never between him and the tank
    Position const behind = GetTidewalkerStackPoint(tidewalker);

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, behind, TIDEWALKER_RANGED_STACK_RADIUS, PATH_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Brings back a non-tank that ended up far out, such as one carried off by Watery Grave. Inside
// the distance, everybody moves as they normally would.
bool MorogrimTidewalkerReturnToBossAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, tidewalker, TIDEWALKER_MAX_DISTANCE_FROM_BOSS, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (AI_VALUE(Unit*, "current target") != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() != bot || !bot->IsWithinMeleeRange(vashj))
        return false;

    if (GetLadyVashjPhase(vashj) == 1)
        return MoveToPhase1TankPosition(vashj);

    return MoveAwayFromEnchantedElementals(vashj);
}

// Phase 1: Position Vashj in the center of the platform
bool LadyVashjMainTankPositionBossAction::MoveToPhase1TankPosition(Unit* vashj)
{
    constexpr float arrivalDistance = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, VASHJ_PLATFORM_CENTER_POSITION, arrivalDistance,
            vashj, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Phase 3: No fixed position, but move Vashj away from Enchanted Elementals
bool LadyVashjMainTankPositionBossAction::MoveAwayFromEnchantedElementals(Unit* vashj)
{
    constexpr float searchRadius = 25.0f;
    std::list<Creature*> creatures;
    vashj->GetCreatureListWithEntryInGrid(
        creatures, Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL), searchRadius);

    // Surge lands at about 4.4y from her center, so this leaves about 2s of their walk
    constexpr float safeDistance = 10.0f;
    std::vector<Unit*> elementals;
    bool tooClose = false;
    for (Creature* creature : creatures)
    {
        if (!creature->IsAlive())
            continue;

        elementals.push_back(creature);
        if (vashj->GetExactDist2d(creature) < safeDistance)
            tooClose = true;
    }

    if (!tooClose)
        return false;

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    // Her tank's spore trigger fires at the tank radius, so steps have to stay that far out too
    if (!FindVashjDaisStepAwayFromUnits(
            bot, elementals, vashj, stepX, stepY, stepZ, backwards,
            &GetToxicSporePositions(botAI), TOXIC_SPORES_TANK_AVOID_RADIUS))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Semicircle around center of the room (to allow escape paths by Static Charged bots)
bool LadyVashjPhase1SpreadRangedInArcAction::Execute(Event /*event*/)
{
    if (_reachedRangedPosition)
        return false;

    if (!_hasRangedPosition)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        std::vector<Player*> spreadMembers;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->GetMapId() == SSC_MAP_ID && GET_PLAYERBOT_AI(member) &&
                PlayerbotAI::IsRanged(member))
            {
                spreadMembers.push_back(member);
            }
        }

        size_t count = spreadMembers.size();
        if (count == 0)
            return false;

        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        size_t botIndex =
            (it != spreadMembers.end()) ? std::distance(spreadMembers.begin(), it) : 0;

        constexpr float arcCenter = M_PI / 2.0f; // West
        constexpr float arcSpan = M_PI; // 180°
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle;
        if (count == 1)
            angle = arcCenter;
        else
            angle = arcStart + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        Position const& center = VASHJ_PLATFORM_CENTER_POSITION;
        constexpr float radius = 25.0f;
        float const targetX = center.GetPositionX() + radius * std::cos(angle);
        float const targetY = center.GetPositionY() + radius * std::sin(angle);

        // The dais slopes down from the center, so the slot takes the ground height under it
        float targetZ = bot->GetMapHeight(targetX, targetY, center.GetPositionZ());
        if (targetZ <= INVALID_HEIGHT)
            targetZ = center.GetPositionZ();

        _rangedPosition = Position(targetX, targetY, targetZ);
        _hasRangedPosition = true;
    }

    constexpr float arrivalDistance = 2.0f;
    if (bot->GetExactDist2d(_rangedPosition) <= arrivalDistance)
    {
        _reachedRangedPosition = true;
        return false;
    }

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, _rangedPosition, arrivalDistance, PATH_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Ranged dps and healers hold cluster slots around the edge of the dais: the cluster nearest a
// Tainted Elemental kills it and its healer loots it, and Enchanted Elementals are met on their way
// in. Bots go back to their slot after being sent after an elemental.
bool LadyVashjPhase2PositionInClusterAction::Execute(Event /*event*/)
{
    VashjClusterSlot const slot = GetVashjClusterSlot(bot);
    if (slot.cluster < 0)
        return false;

    Position const& clusterPosition = GetVashjClusterPosition(slot);

    // The looter stays on the elemental until the core is looted
    if (GetDesignatedCoreLooter(botAI, bot) == bot && GetVashjTaintedElemental(bot))
        return false;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && IsVashjTaintedElementalKiller(bot, tainted))
        return false;

    // Stepped in to a Strider; back once it dies or is dragged away
    if (PlayerbotAI::IsRangedDps(bot) &&
        IsVashjStriderToStepInTo(bot, AI_VALUE(Unit*, "current target")))
    {
        return false;
    }

    constexpr float arrivalDistance = 2.0f;
    if (bot->GetExactDist2d(clusterPosition) <= arrivalDistance)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, clusterPosition, arrivalDistance, PATH_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Nothing else puts ranged at range in phase 3. Out of Entangle first, then a little apart, so one
// spore catches fewer of them.
bool LadyVashjPhase3PositionRangedAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    constexpr float vashjDistance = 15.0f;
    constexpr float spreadDistance = 4.0f;

    std::vector<Unit*> avoid;
    if (bot->GetExactDist2d(vashj) < vashjDistance)
    {
        avoid.push_back(vashj);
    }
    else if (Group* group = bot->GetGroup())
    {
        bool tooClose = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != SSC_MAP_ID)
            {
                continue;
            }

            avoid.push_back(member);
            if (bot->GetExactDist2d(member) < spreadDistance)
                tooClose = true;
        }

        if (!tooClose)
            return false;
    }

    if (avoid.empty())
        return false;

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    if (!FindVashjDaisStepAwayFromUnits(
            bot, avoid, nullptr, stepX, stepY, stepZ, backwards,
            &GetToxicSporePositions(botAI)))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// For absorbing Shock Burst
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (mainTank->HasAura(Id(SscSpells::SPELL_GROUNDING_TOTEM_EFFECT)))
        return false;

    constexpr float distFromTank = 25.0f;
    if (bot->GetDistance(mainTank) > distFromTank)
    {
        // Don't walk back in while Static Charge is keeping this bot clear of somebody
        Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        if (vashj && ShouldAvoidVashjStaticCharge(bot, vashj))
            return false;

        return MoveTo(mainTank, distFromTank, MovementPriority::MOVEMENT_COMBAT);
    }

    return botAI->CanCastSpell("grounding totem", bot) &&
           botAI->CastSpell("grounding totem", bot);
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // The pulse reaches 10y from the holder's center
    constexpr float safeDistance = 11.0f;
    Player* vashjVictim = vashj->GetVictim() ? vashj->GetVictim()->ToPlayer() : nullptr;
    if (bot == vashjVictim)
        return false;

    std::vector<Unit*> avoid;
    bool tooClose = false;

    // If any other bot has Static Charge, it should move away from other group members
    if (HasStaticCharge(bot))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != SSC_MAP_ID)
            {
                continue;
            }

            avoid.push_back(member);
            if (bot->GetExactDist2d(member) < safeDistance)
                tooClose = true;
        }
    }
    // If Vashj's target has Static Charge, other group members should move away.
    else if (vashjVictim && HasStaticCharge(vashjVictim))
    {
        avoid.push_back(vashjVictim);
        tooClose = bot->GetExactDist2d(vashjVictim) < safeDistance;
    }

    if (!tooClose)
        return false;

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    if (!FindVashjDaisStepAwayFromUnits(
            bot, avoid, nullptr, stepX, stepY, stepZ, backwards, &GetToxicSporePositions(botAI)))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool LadyVashjAssignPhase2AndPhase3DpsPriorityAction::Execute(Event /*event*/)
{
    Position const& center = VASHJ_PLATFORM_CENTER_POSITION;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    // Search and attack radius are intended to keep bots from going down the stairs
    float const maxSearchRange = PlayerbotAI::IsRanged(bot) ? 60.0f : 55.0f;
    float const maxPursueRange = maxSearchRange - 5.0f;
    int8 const phase = GetLadyVashjPhase(vashj);

    // In phase 2 ranged dps hold cluster slots and shoot only what is in range of them, other than
    // the cluster sent after a Tainted Elemental
    bool const holdsClusterSlot = phase == 2 && PlayerbotAI::IsRangedDps(bot);
    float const spellRange = botAI->GetRange("spell");
    Unit* tainted = nullptr;
    if (holdsClusterSlot)
    {
        tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
        if (tainted && !IsVashjTaintedElementalKiller(bot, tainted))
            tainted = nullptr;
    }

    // Everyone but tanks leaves an Elite or Strider alone until a tank has it, so nobody pulls one
    // onto a cluster
    bool const waitForTank = phase == 2 && !PlayerbotAI::IsTank(bot);

    Unit* enchanted = nullptr;
    Unit* enchantedNearestBot = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    auto const& attackers = AI_VALUE(GuidVector, "possible targets no los");
    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (phase == 2 && distFromCenter > maxSearchRange)
            continue;

        // A tanked Strider a little out of range is stepped in to; anything else must be in range
        if (holdsClusterSlot && !bot->IsWithinCombatRange(unit, spellRange) &&
            !IsVashjStriderToStepInTo(bot, unit))
        {
            continue;
        }

        switch (unit->GetEntry())
        {
            case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;

                if (!enchantedNearestBot ||
                    bot->GetExactDist2d(unit) < bot->GetExactDist2d(enchantedNearestBot))
                {
                    enchantedNearestBot = unit;
                }
                break;

            case Id(SscNpcs::NPC_COILFANG_ELITE):
                if (waitForTank && !IsTankedByTank(unit))
                    break;

                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case Id(SscNpcs::NPC_COILFANG_STRIDER):
                if (waitForTank && !IsTankedByTank(unit))
                    break;

                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case Id(SscNpcs::NPC_TOXIC_SPOREBAT):
            {
                // Chasing a bat any higher, or off the dais, walks hunters up into the air
                constexpr float maxSporebatHeight = 40.0f;
                if (unit->GetPositionZ() - center.GetPositionZ() > maxSporebatHeight ||
                    !IsOnVashjDais(unit->GetPositionX(), unit->GetPositionY(), 0.0f))
                {
                    break;
                }

                if (!sporebat || bot->GetDistance(unit) < bot->GetDistance(sporebat))
                    sporebat = unit;
                break;
            }

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (phase == 2)
    {
        Unit* enchantedNearVashj =
            enchanted && vashj->GetExactDist2d(enchanted) <= VASHJ_ENCHANTED_NEAR_HER_DISTANCE ?
            enchanted : nullptr;

        // Striders need several ranged on them at once
        if (holdsClusterSlot)
            targets = { strider, enchanted, elite };
        // Melee stay near her and the Elites: Enchanted about to reach her first, then Elites,
        // then whichever other Enchanted is nearest
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchantedNearVashj, elite, enchantedNearestBot };
        // Tanks stay in the middle for the next Elite or Strider, wherever it comes from
        else if (PlayerbotAI::IsTank(bot))
        {
            if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
                targets = { strider, elite, enchantedNearVashj };
            else
                targets = { elite, strider, enchantedNearVashj };
        }
        else
            targets = { enchanted, elite, strider };
    }

    if (phase == 3)
    {
        if (PlayerbotAI::IsTank(bot))
        {
            if (PlayerbotAI::IsMainTank(bot))
                targets = { vashj };
            else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
                targets = { strider, elite, enchanted, vashj };
            else
                targets = { elite, strider, enchanted, vashj };
        }
        else if (PlayerbotAI::IsRanged(bot))
        {
            // Hunters are assigned to kill Sporebats in Phase 3
            if (bot->getClass() == CLASS_HUNTER)
                targets = { sporebat, enchanted, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    Unit* target = tainted;
    if (!target)
    {
        for (Unit* candidate : targets)
        {
            if (candidate && bot->GetExactDist2d(candidate) <= maxPursueRange)
            {
                target = candidate;
                break;
            }
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();

    if (currentTarget && currentTarget == vashj && phase == 2)
    {
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
    }

    // A tank with nothing in the middle drops an Enchanted farther out and waits there instead
    if (!target && phase == 2 && PlayerbotAI::IsTank(bot) && currentTarget &&
        currentTarget->GetEntry() == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL))
    {
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
    }

    if (target && currentTarget != target && AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    // If bots have wandered too far from the center, move them back
    // THIS DOESN'T WORK SINCE MOVETO A WORLD OBJECT IS LIMITED TO SPELL DIST
    // Cluster bots are taken back to their slots by their own action instead. So are idle tanks in
    // phase 2, to the middle; a tank holding a Strider past the south-east hold is about 55y out,
    // and this would drag it off the hold.
    if (holdsClusterSlot || (phase == 2 && PlayerbotAI::IsTank(bot)) ||
        bot->GetExactDist2d(vashj) <= maxPursueRange)
    {
        return false;
    }

    return MoveTo(vashj, maxPursueRange - 10.0f, MovementPriority::MOVEMENT_FORCED);
}

bool LadyVashjReturnToTheGroundAction::Execute(Event /*event*/)
{
    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();

    // Search down from the dais, not from the bot, so the floor found is the dais or the
    // stairs and never the pipes the bot may be standing on
    float const floorZ = bot->GetMapHeight(x, y, VASHJ_PLATFORM_CENTER_POSITION.GetPositionZ());
    if (floorZ <= INVALID_HEIGHT)
        return false;

    bot->AttackStop();
    bot->CastStop();
    bot->StopMoving();
    bot->GetMotionMaster()->Clear();
    bot->NearTeleportTo(x, y, floorZ, bot->GetOrientation());

    return true;
}

bool LadyVashjTankAttackAndPositionStriderAction::Execute(Event /*event*/)
{
    // Automatically apply Fear Ward to tanks to make Strider tankable. This simulates the real-life
    // strategy where the Strider can be meleed by players wearing an Ogre Suit (due to the
    // extended combat reach).
    if (!bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)))
        bot->AddAura(Id(SscSpells::SPELL_FEAR_WARD), bot);

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    if (!strider)
        return false;

    // Only the first assist tank affirmatively picks up Striders
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != strider)
    {
        return Attack(strider);
    }

    if (strider->GetVictim() != bot)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 2)
        return MoveStriderToHoldPosition(strider);

    // But all tanks move away from Vashj if they are holding a Strider, except the Main Tank in
    // phase 3, who is holding Vashj.
    if (phase == 3 && PlayerbotAI::IsMainTank(bot))
        return false;

    return MoveStriderAwayFromVashj(vashj);
}

// Phase 2: the hold point nearest the Strider.
bool LadyVashjTankAttackAndPositionStriderAction::MoveStriderToHoldPosition(Unit* strider)
{
    Position const& hold = *std::min_element(
        VASHJ_STRIDER_HOLD_POSITIONS.begin(), VASHJ_STRIDER_HOLD_POSITIONS.end(),
        [strider](Position const& a, Position const& b)
        {
            return strider->GetExactDist2d(a) < strider->GetExactDist2d(b);
        });

    constexpr float arrivalDistance = 3.0f;
    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToBringTankedUnitTo(
            bot, strider, hold, arrivalDistance, stepX, stepY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Walks a path, which goes round the generators.
bool LadyVashjTankWaitInTheMiddleAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, vashj, VASHJ_IDLE_TANK_DISTANCE, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// The tank takes the Elite to the nearer of the two Elite tank positions, where a cluster's ranged
// reach it. For a human tank: in front of the north rock, or south-east of the middle.
bool LadyVashjPositionCoilfangEliteAction::Execute(Event /*event*/)
{
    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) ||
        elite->GetVictim() != bot)
    {
        return false;
    }

    Position const& spot = *std::min_element(
        VASHJ_ELITE_TANK_POSITIONS.begin(), VASHJ_ELITE_TANK_POSITIONS.end(),
        [elite](Position const& a, Position const& b)
        {
            return elite->GetExactDist2d(a) < elite->GetExactDist2d(b);
        });

    constexpr float arrivalDistance = 3.0f;
    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToBringTankedUnitTo(
            bot, elite, spot, arrivalDistance, stepX, stepY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Phase 3: keep the Strider away from Vashj, where bots tend to congregate to take down
// elementals.
bool LadyVashjTankAttackAndPositionStriderAction::MoveStriderAwayFromVashj(Unit* vashj)
{
    float const currentDistance = bot->GetExactDist2d(vashj);
    constexpr float safeDistance = 28.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(vashj, safeDistance - currentDistance, true);
}

// Keeps the cluster slots filled: holders never move, and a spare takes the slot of one who dies.
// The table is per instance, so a new tracker picks up where a dead one left off.
bool LadyVashjAssignClusterSlotsAction::Execute(Event /*event*/)
{
    return UpdateVashjClusterHolders(bot);
}

// Chosen once per elemental, from where everyone stands when it spawns. Choosing again as bots
// move would hand the job back and forth, since the killers walk toward it too.
bool LadyVashjAssignTaintedCoreLooterAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    int8 const cluster = GetNearestVashjCluster(tainted);
    Player* looter = FindTaintedCoreLooter(bot, tainted, cluster);
    if (!looter)
        return false;

    // TEMP LOG
    auto previous = vashjTaintedCoreLooter.find(bot->GetInstanceId());
    bool const repick = previous != vashjTaintedCoreLooter.end() &&
        previous->second.tainted == tainted->GetGUID();

    vashjTaintedCoreLooter.insert_or_assign(bot->GetInstanceId(),
        TaintedCoreLooter{ tainted->GetGUID(), tainted->GetPosition(), looter->GetGUID(),
            cluster });

    // TEMP LOG
    if (!repick)
        StartTaintedLog(bot);

    char const* role = PlayerbotAI::IsHeal(looter) ? "healer" :
        PlayerbotAI::IsRangedDps(looter) ? "ranged dps" :
        PlayerbotAI::IsMelee(looter) ? "melee" : "other";
    LOG_INFO("playerbots",
        "[SSC tainted] +{}ms {} looter {} ({}) at {:.1f} yd, cluster {}, elemental at {:.1f} "
        "{:.1f} {:.1f}",
        TaintedLogElapsedMs(bot), repick ? "re-picked" : "picked", looter->GetName(), role,
        looter->GetExactDist(tainted), cluster, tainted->GetPositionX(), tainted->GetPositionY(),
        tainted->GetPositionZ());

    std::string killers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && IsVashjTaintedElementalKiller(member, tainted))
            {
                killers += std::string(member->GetName()) + " " +
                    std::to_string(static_cast<int>(member->GetExactDist(tainted))) + " yd; ";
            }
        }
    }
    LOG_INFO("playerbots", "[SSC tainted] +{}ms killers: {}", TaintedLogElapsedMs(bot),
        killers.empty() ? "none" : killers);

    return true;
}

// The killers walk until they have the elemental in attack range and in sight, then attack. From
// a cluster the edge of the dais usually blocks the view down to it, and Attack() refuses a target
// out of sight. Ranged stop at spell range, which keeps hunters out of their dead zone. The looter
// walks straight to it instead, so it is on the corpse when it dies, and attacks once there.
bool LadyVashjAttackTaintedElementalAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    bool const isLooter = GetDesignatedCoreLooter(botAI, bot) == bot;

    // TEMP LOG
    if (TaintedLogFirstTime(bot, "sent"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms {} {} sent from {:.1f} yd",
            TaintedLogElapsedMs(bot), isLooter ? "looter" : "killer", bot->GetName(),
            bot->GetExactDist(tainted));
    }

    constexpr float stopDistance = 3.0f;
    bool const inRange = isLooter ? bot->GetExactDist(tainted) <= stopDistance :
        PlayerbotAI::IsRanged(bot) ? bot->IsWithinCombatRange(tainted, botAI->GetRange("spell")) :
        bot->IsWithinMeleeRange(tainted);
    if (!inRange || !bot->IsWithinLOSInMap(tainted))
    {
        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, tainted, stopDistance, stepX, stepY))
        {
            // TEMP LOG
            if (TaintedLogThrottle(bot, "nopath"))
            {
                LOG_INFO("playerbots", "[SSC tainted] +{}ms {} has no path at {:.1f} yd",
                    TaintedLogElapsedMs(bot), bot->GetName(), bot->GetExactDist(tainted));
            }

            return false;
        }

        // Still true between steps, so nothing lower starts a cast on the way
        bool const moved = MoveTo(
            SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);

        // TEMP LOG
        if (!moved && !bot->isMoving() && TaintedLogThrottle(bot, "refused"))
        {
            LOG_INFO("playerbots",
                "[SSC tainted] +{}ms {} step refused at {:.1f} yd, bot z {:.1f}",
                TaintedLogElapsedMs(bot), bot->GetName(), bot->GetExactDist(tainted),
                bot->GetPositionZ());
        }

        return moved || bot->isMoving();
    }

    // TEMP LOG
    if (TaintedLogFirstTime(bot, "inrange"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms {} in range at {:.1f} yd, elemental at {:.0f}%",
            TaintedLogElapsedMs(bot), bot->GetName(), bot->GetExactDist(tainted),
            tainted->GetHealthPct());
    }

    if (AI_VALUE(Unit*, "current target") != tainted)
        return Attack(tainted);

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasItemCount(Id(SscItems::ITEM_TAINTED_CORE), 1, false))
            return false;
    }

    Creature* elemental = GetVashjTaintedElemental(bot);
    if (!elemental || elemental->IsAlive())
        return false;

    // TEMP LOG
    if (TaintedLogFirstTime(bot, "dead"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms dead, looter {} at {:.1f} yd",
            TaintedLogElapsedMs(bot), bot->GetName(), bot->GetExactDist(elemental));
    }

    // OpenLootAction refuses beyond this. The walk comes before IsLootPossible(), which also
    // refuses while the height gap is over this, as it is from partway up the stairs.
    constexpr float maxLootRange = INTERACTION_DISTANCE - 2.0f;
    constexpr float distFromObject = 2.0f;

    if (bot->GetDistance(elemental) > maxLootRange)
    {
        // TEMP LOG
        if (TaintedLogThrottle(bot, "tocorpse"))
        {
            LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} walking to corpse at {:.1f} yd",
                TaintedLogElapsedMs(bot), bot->GetName(), bot->GetDistance(elemental));
        }

        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, elemental, distFromObject, stepX, stepY))
            return false;

        return MoveTo(
            SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    LootObject loot(bot, elemental->GetGUID());
    if (!loot.IsLootPossible(bot))
    {
        // TEMP LOG
        if (TaintedLogThrottle(bot, "notpossible"))
        {
            LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} loot not possible",
                TaintedLogElapsedMs(bot), bot->GetName());
        }

        return false;
    }

    context->GetValue<LootObject>("loot target")->Set(loot);

    OpenLootAction open(botAI);
    if (!open.Execute(Event()))
    {
        // TEMP LOG
        if (TaintedLogThrottle(bot, "openfailed"))
        {
            LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} open loot failed at {:.1f} yd",
                TaintedLogElapsedMs(bot), bot->GetName(), bot->GetDistance(elemental));
        }

        return false;
    }

    bot->SetLootGUID(elemental->GetGUID());
    constexpr uint8 coreIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << coreIndex;
    bot->GetSession()->QueuePacket(packet);

    // TEMP LOG
    if (TaintedLogFirstTime(bot, "packet") || TaintedLogThrottle(bot, "packet"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} loot packet sent",
            TaintedLogElapsedMs(bot), bot->GetName());
    }

    uint32 const now = getMSTime();
    lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);

    return true;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(botAI, bot);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(botAI, bot);

    uint32 const instanceId = vashj->GetInstanceId();

    auto itLooter = vashjTaintedCoreLooter.find(instanceId);
    if (itLooter == vashjTaintedCoreLooter.end())
        return false;

    Unit* closestTrigger =
        GetNearestActiveShieldGeneratorTriggerByEntry(vashj, itLooter->second.taintedPosition);
    if (!closestTrigger)
        return false;

    // Not gated behind CheatMask because the auto application of Fear Ward is necessary
    // to address an issue with bot movement, which is that bots cannot be rooted and
    // therefore will move when feared while holding the Tainted Core
    if (!bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)))
        bot->AddAura(Id(SscSpells::SPELL_FEAR_WARD), bot);

    Item* item = bot->GetItemByEntry(Id(SscItems::ITEM_TAINTED_CORE));
    if (!item || !botAI->HasItemInInventory(Id(SscItems::ITEM_TAINTED_CORE)))
    {
        // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
        if (bot == firstCorePasser &&
            LineUpFirstCorePasser(designatedLooter))
        {
            return true;
        }
        else if (bot == secondCorePasser &&
                 LineUpSecondCorePasser(firstCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == thirdCorePasser && LineUpThirdCorePasser(
                 firstCorePasser, secondCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == fourthCorePasser && LineUpFourthCorePasser(
                 secondCorePasser, thirdCorePasser, closestTrigger))
        {
            return true;
        }
    }
    else if (item && botAI->HasItemInInventory(Id(SscItems::ITEM_TAINTED_CORE)))
    {
        // Designated core looter logic
        // Applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter && IsFirstCorePasserInPosition(firstCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            uint32 const now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, firstCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // First core passer: receive core from looter at the top of the stairs,
        // pass to second core passer
        else if (bot == firstCorePasser && IsSecondCorePasserInPosition(secondCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            uint32 const now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, secondCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Second core passer: if closest usable generator is within passing distance
        // of the first passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == secondCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsThirdCorePasserInPosition(thirdCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            uint32 const now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, thirdCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Third core passer: if closest usable generator is within passing distance
        // of the second passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == thirdCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsFourthCorePasserInPosition(fourthCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            uint32 const now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, fourthCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Fourth core passer: the fourth passer is rarely needed and no more than
        // four ever should be, so it should use the Core on the nearest generator
        else if (bot == fourthCorePasser && UseCoreOnNearestGenerator(instanceId))
            return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* designatedLooter)
{
    if (!designatedLooter)
        return false;

    float const centerX = VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    float const centerY = VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();
    constexpr float radius = 57.5f;

    auto it = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (it == intendedVashjCorePasserLineup.end())
    {
        float mx = designatedLooter->GetPositionX();
        float my = designatedLooter->GetPositionY();
        float angle = atan2(my - centerY, mx - centerX);

        float targetX = centerX + radius * std::cos(angle);
        float targetY = centerY + radius * std::sin(angle);
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedVashjCorePasserLineup.try_emplace(
            bot->GetGUID(), Position(targetX, targetY, targetZ));
        it = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    Position const& pos = it->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(
    Player* firstCorePasser, Unit* closestTrigger)
{
    if (!firstCorePasser || !closestTrigger)
        return false;

    auto itFirst = intendedVashjCorePasserLineup.find(firstCorePasser->GetGUID());
    if (itFirst == intendedVashjCorePasserLineup.end())
        return false;

    auto itSecond = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itSecond == intendedVashjCorePasserLineup.end())
    {
        float fx = itFirst->second.GetPositionX();
        float fy = itFirst->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - fx;
        float dy = closestTrigger->GetPositionY() - fy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX;
        float targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = fx + dx * moveDist;
            targetY = fy + dy * moveDist;
        }
        else
        {
            targetX = fx + dx * farDistance;
            targetY = fy + dy * farDistance;
        }

        intendedVashjCorePasserLineup.try_emplace(
            bot->GetGUID(), Position(targetX, targetY, targetZ));
        itSecond = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    Position const& pos = itSecond->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(
    Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    if (!secondCorePasser || !closestTrigger)
        return false;

    bool needThirdPasser =
        (IsFirstCorePasserInPosition(firstCorePasser) &&
         firstCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needThirdPasser)
        return false;

    auto itSecond = intendedVashjCorePasserLineup.find(secondCorePasser->GetGUID());
    if (itSecond == intendedVashjCorePasserLineup.end())
        return false;

    auto itThird = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itThird == intendedVashjCorePasserLineup.end())
    {
        float sx = itSecond->second.GetPositionX();
        float sy = itSecond->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - sx;
        float dy = closestTrigger->GetPositionY() - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX;
        float targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = sx + dx * moveDist;
            targetY = sy + dy * moveDist;
        }
        else
        {
            targetX = sx + dx * farDistance;
            targetY = sy + dy * farDistance;
        }

        intendedVashjCorePasserLineup.try_emplace(
            bot->GetGUID(), Position(targetX, targetY, targetZ));
        itThird = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    Position const& pos = itThird->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(
    Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger)
{
    if (!thirdCorePasser || !closestTrigger)
        return false;

    bool needFourthPasser =
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsThirdCorePasserInPosition(thirdCorePasser) &&
         thirdCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needFourthPasser)
        return false;

    auto itThird = intendedVashjCorePasserLineup.find(thirdCorePasser->GetGUID());
    if (itThird == intendedVashjCorePasserLineup.end())
        return false;

    auto itFourth = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itFourth == intendedVashjCorePasserLineup.end())
    {
        float sx = itThird->second.GetPositionX();
        float sy = itThird->second.GetPositionY();

        float tx = closestTrigger->GetPositionX();
        float ty = closestTrigger->GetPositionY();

        float dx = tx - sx;
        float dy = ty - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        constexpr float nearTriggerDist = 1.5f;
        float targetX = tx - dx * nearTriggerDist;
        float targetY = ty - dy * nearTriggerDist;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedVashjCorePasserLineup.try_emplace(
            bot->GetGUID(), Position(targetX, targetY, targetZ));
        itFourth = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    Position const& pos = itFourth->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

// The next four functions check if the respective passer is <= 2 yards of their intended
// position and are used to determine when the prior bot in the chain can pass the core
bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInPosition(Player* firstCorePasser)
{
    if (!firstCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(firstCorePasser->GetGUID());
    if (itSnap == intendedVashjCorePasserLineup.end())
        return false;

    return firstCorePasser->GetExactDist2d(
        itSnap->second.GetPositionX(), itSnap->second.GetPositionY()) <= 2.0f;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInPosition(Player* secondCorePasser)
{
    if (!secondCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(secondCorePasser->GetGUID());
    if (itSnap == intendedVashjCorePasserLineup.end())
        return false;

    return secondCorePasser->GetExactDist2d(
        itSnap->second.GetPositionX(), itSnap->second.GetPositionY()) <= 2.0f;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInPosition(Player* thirdCorePasser)
{
    if (!thirdCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(thirdCorePasser->GetGUID());
    if (itSnap == intendedVashjCorePasserLineup.end())
        return false;

    return thirdCorePasser->GetExactDist2d(
        itSnap->second.GetPositionX(), itSnap->second.GetPositionY()) <= 2.0f;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInPosition(Player* fourthCorePasser)
{
    if (!fourthCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(fourthCorePasser->GetGUID());
    if (itSnap == intendedVashjCorePasserLineup.end())
        return false;

    return fourthCorePasser->GetExactDist2d(
        itSnap->second.GetPositionX(), itSnap->second.GetPositionY()) <= 2.0f;
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator(uint32 instanceId)
{
    auto const& generators =
        GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    GeneratorInfo const* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator || bot->GetExactDist2d(generator) > 4.5f)
        return false;

    Item* core = bot->GetItemByEntry(Id(SscItems::ITEM_TAINTED_CORE));
    if (!core || bot->CanUseItem(core) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    uint8 const bagIndex = core->GetBagSlot();
    uint8 const slot = core->GetSlot();
    constexpr uint8 cast_count = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (core->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = core->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    ObjectGuid const item_guid = core->GetGUID();
    constexpr uint32 glyphIndex = 0;
    constexpr uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex;
    packet << slot;
    packet << cast_count;
    packet << spellId;
    packet << item_guid;
    packet << glyphIndex;
    packet << castFlags;
    packet << (uint32)TARGET_FLAG_GAMEOBJECT;
    packet << generator->GetGUID().WriteAsPacked();

    bot->GetSession()->HandleUseItemOpcode(packet);

    lastVashjCoreImbueAttempt.erase(instanceId);
    auto coreHandlers = GetCoreHandlers(botAI, bot);
    for (Player* handler : coreHandlers)
    {
        if (handler)
        {
            intendedVashjCorePasserLineup.erase(handler->GetGUID());
            lastVashjCoreInInventoryTime.erase(handler->GetGUID());
        }
    }

    return true;
}

// Pets never leave a living target on their own (PetAI::OwnerAttacked), so they would stay on
// Vashj through all of phase 2. This puts them on their master's target instead.
bool LadyVashjCommandPetTargetAction::Execute(Event /*event*/)
{
    Guardian* pet = bot->GetGuardianPet();
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!pet || !vashj)
        return false;

    CharmInfo* charmInfo = pet->GetCharmInfo();
    Unit* target = GetVashjPetTarget(botAI, pet, vashj);

    // Nothing worth attacking in phase 2 but an immune Vashj, so come back
    if (!target)
    {
        pet->AttackStop();
        pet->InterruptNonMeleeSpells(false);
        pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        if (charmInfo)
        {
            charmInfo->SetCommandState(COMMAND_FOLLOW);
            charmInfo->SetIsCommandAttack(false);
            charmInfo->SetIsAtStay(false);
            charmInfo->SetIsReturning(true);
            charmInfo->SetIsCommandFollow(true);
            charmInfo->SetIsFollowing(false);
            charmInfo->RemoveStayPosition();
        }

        return true;
    }

    if (!pet->IsValidAttackTarget(target))
        return false;

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());
    if (charmInfo)
    {
        charmInfo->SetIsCommandAttack(true);
        charmInfo->SetIsAtStay(false);
        charmInfo->SetIsFollowing(false);
        charmInfo->SetIsCommandFollow(false);
        charmInfo->SetIsReturning(false);
    }

    pet->AI()->AttackStart(target);
    return true;
}

// The standard "avoid aoe" strategy does work for Toxic Spores, but this method
// provides more buffer distance and limits the area in which bots can move
// so that they do not go down the stairs
bool LadyVashjAvoidToxicSporesAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    std::vector<Position> const& spores = GetToxicSporePositions(botAI);
    bool const tanking = vashj->GetVictim() == bot;

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    bool found = FindVashjDaisStepAwayFromPositions(
        bot, spores, vashj, stepX, stepY, stepZ, backwards);

    // When no step gains on every pool, still step away from the closest one if it is close enough
    // to hurt, even toward another.
    if (!found)
    {
        auto const closest = std::min_element(spores.begin(), spores.end(),
            [this](Position const& a, Position const& b)
            {
                return bot->GetExactDist2dSq(a) < bot->GetExactDist2dSq(b);
            });

        if (closest != spores.end() && bot->GetExactDist2d(*closest) < TOXIC_SPORES_AVOID_RADIUS)
        {
            std::vector<Position> const nearest = { *closest };
            found = FindVashjDaisStepAwayFromPositions(
                bot, nearest, vashj, stepX, stepY, stepZ, backwards);
        }
    }

    if (!found)
        return false;

    MovementPriority const priority = tanking ?
        MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, backwards);
}

bool LadyVashjPaladinUseHandOfFreedomAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Her target never moves for Static Charge, and phase 1 has no spores to leave
    Unit* const skip = GetLadyVashjPhase(vashj) == 1 ? vashj->GetVictim() : nullptr;

    std::vector<Position> const& spores = GetToxicSporePositions(botAI);

    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == skip || !member->IsAlive() ||
            !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)) || !PlayerbotAI::IsMelee(member))
        {
            continue;
        }

        bool nearToxicSpore = false;
        for (Position const& spore : spores)
        {
            if (member->GetExactDist2d(spore) < TOXIC_SPORES_HIT_RADIUS)
            {
                nearToxicSpore = true;
                break;
            }
        }

        if (nearToxicSpore)
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankToxic = member;

            if (!anyToxic)
                anyToxic = member;
        }

        if (HasStaticCharge(member))
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankStatic = member;

            if (!anyStatic)
                anyStatic = member;
        }
    }

    // Priority 1: Entangled in Toxic Spores (prefer main tank)
    Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
    if (toxicTarget && botAI->CanCastSpell("hand of freedom", toxicTarget))
        return botAI->CastSpell("hand of freedom", toxicTarget);

    // Priority 2: Entangled with Static Charge (prefer main tank)
    Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
    if (staticTarget && botAI->CanCastSpell("hand of freedom", staticTarget))
        return botAI->CastSpell("hand of freedom", staticTarget);

    return false;
}

// Cloak of Shadows strips Static Charge (the 35729 handler in SpellEffects.cpp)
bool LadyVashjRogueUseCloakOfShadowsAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot);
}
