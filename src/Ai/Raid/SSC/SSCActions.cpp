/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCActions.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SSCHelpers.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

using namespace SscHelpers;
using namespace EncounterHelpers;

// Shared

bool SscResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;

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

    uint32 const instanceId = bot->GetInstanceId();

    reset |= vashjClusterHolders.erase(instanceId) > 0;
    reset |= vashjTaintedCoreLooter.erase(instanceId) > 0;
    reset |= vashjCorePassingChains.erase(instanceId) > 0;
    reset |= vashjGroundingShaman.erase(instanceId) > 0;
    reset |= karathressDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasHumanoidPhaseStartTime.erase(instanceId) > 0;
    reset |= leotherasWhirlwindEndTime.erase(instanceId) > 0;
    reset |= leotherasDemonPhaseStartTime.erase(instanceId) > 0;
    reset |= leotherasFinalPhaseStartTime.erase(instanceId) > 0;
    reset |= lurkerGuardianTankAssignments.erase(instanceId) > 0;
    reset |= hydrossFrostMarkMaxedTime.erase(instanceId) > 0;
    reset |= hydrossNatureMarkMaxedTime.erase(instanceId) > 0;
    reset |= hydrossNaturePhaseStartTime.erase(instanceId) > 0;
    reset |= hydrossFrostPhaseStartTime.erase(instanceId) > 0;

    if (!AI_VALUE2(bool, "combat", "self target"))
    {
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);
        reset |= ClearTargetIcon(bot, RtiTargetValue::crossIndex);
    }

    return reset;
}

bool SscMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* tank = GetGroupMainTank(bot);
    if (!tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, tank);
}

bool SscStopAttackingAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());

    return true;
}

bool SscSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, _distance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), _distance);
}

// Trash

bool UnderbogColossusEscapeToxicPoolAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

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
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event /*event*/)
{
    return MarkTargetWithSkull(bot, GetWaterElementalTotem(botAI));
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstablePositionAndSwapTanksAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    bool const myPhase = _frostTank ?
        IsHydrossInFrostPhase(hydross) : IsHydrossInNaturePhase(hydross);
    Position const& myPosition = _frostTank ?
        HYDROSS_FROST_TANK_POSITION : HYDROSS_NATURE_TANK_POSITION;

    if (!myPhase)
    {
        // Once the other tank starts walking him over, meet him short of the field's edge
        if (GetHydrossDpsHoldWindow(hydross) == HydrossDpsHoldWindow::BeforePhaseChange)
            return StepTo(GetHydrossHandoffPosition(_frostTank), hydross);

        return StepTo(myPosition, hydross);
    }

    bool const markMaxed =
        _frostTank ? HasMarkOfHydrossAt100Percent(bot) : HasMarkOfCorruptionAt100Percent(bot);

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

    Position const& otherPosition = _frostTank ?
        HYDROSS_NATURE_TANK_POSITION : HYDROSS_FROST_TANK_POSITION;
    std::unordered_map<uint32, uint32> const& markMaxedTimes =
        _frostTank ? hydrossFrostMarkMaxedTime : hydrossNatureMarkMaxedTime;

    constexpr uint32 phaseChangeDelayMs = 1 * IN_MILLISECONDS;
    auto it = markMaxedTimes.find(hydross->GetInstanceId());
    if (it != markMaxedTimes.end() && getMSTimeDiff(it->second, getMSTime()) >= phaseChangeDelayMs)
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

bool HydrossTheUnstableMisdirectToTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    Player* tank =
        IsHydrossInFrostPhase(hydross) ? GetGroupMainTank(bot) : GetGroupAssistTank(bot, 0);
    if (!tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, hydross, tank);
}

bool HydrossTheUnstableManagePhaseTimersAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();

    // The tank holding Hydross always has the Mark. The tracker may not, if it was dead when the
    // Mark landed.
    Unit* victim = hydross->GetVictim();
    Player* marked = victim && victim->IsPlayer() ? victim->ToPlayer() : bot;

    bool updated = false;

    if (IsHydrossInFrostPhase(hydross))
    {
        updated |= hydrossFrostPhaseStartTime.try_emplace(instanceId, now).second;
        updated |= hydrossNaturePhaseStartTime.erase(instanceId) > 0;
        updated |= hydrossNatureMarkMaxedTime.erase(instanceId) > 0;

        if (!hydrossFrostMarkMaxedTime.contains(instanceId) &&
            HasMarkOfHydrossAt100Percent(marked))
        {
            updated |= hydrossFrostMarkMaxedTime.try_emplace(instanceId, now).second;
        }
    }
    else // Nature phase
    {
        updated |= hydrossNaturePhaseStartTime.try_emplace(instanceId, now).second;
        updated |= hydrossFrostPhaseStartTime.erase(instanceId) > 0;
        updated |= hydrossFrostMarkMaxedTime.erase(instanceId) > 0;

        if (!hydrossNatureMarkMaxedTime.contains(instanceId) &&
            HasMarkOfCorruptionAt100Percent(marked))
        {
            updated |= hydrossNatureMarkMaxedTime.try_emplace(instanceId, now).second;
        }
    }

    return updated;
}

// The Lurker Below

namespace
{

// Recorded as the bot's last movement so IsWaitingForLastMove covers it. Walks a spillway or the
// water at walkway height where the navmesh would go the long way round or along the bottom.
bool MoveStraightTo(PlayerbotAI* botAI, float x, float y, float z, MovementPriority priority)
{
    if (!botAI->CanMove())
        return false;

    Player* bot = botAI->GetBot();
    float const distance = bot->GetExactDist(x, y, z);
    if (distance <= 0.01f)
        return false;

    // MovePoint and a 2-point escort path both pathfind (PointMovementGenerator hands the spline
    // generatePath true). A path of three points is followed as given.
    float const startX = bot->GetPositionX();
    float const startY = bot->GetPositionY();
    float const startZ = bot->GetPositionZ();
    Movement::PointsArray path = {
        G3D::Vector3(startX, startY, startZ),
        G3D::Vector3((startX + x) / 2.0f, (startY + y) / 2.0f, (startZ + z) / 2.0f),
        G3D::Vector3(x, y, z),
    };

    MotionMaster* motionMaster = bot->GetMotionMaster();
    motionMaster->Clear();
    motionMaster->MoveSplinePath(&path);

    float const delay = std::clamp(
        IN_MILLISECONDS * distance / bot->GetSpeed(MOVE_RUN) -
            static_cast<float>(botAI->GetReactDelay()),
        0.0f, static_cast<float>(sPlayerbotAIConfig.maxWaitForMove));
    botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().Set(
        SSC_MAP_ID, x, y, z, bot->GetOrientation(), delay, priority);
    return true;
}

} // end anonymous namespace (Lurker)

// Runnin', runnin', runnin', I'm runnin' over here, run, run, run-run, run.
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    constexpr float pi = static_cast<float>(M_PI);

    // The main tank runs the inner edge of the band. Everyone else gets a radius from their
    // GUID, so the running looks a bit more natural.
    uint32 const seed = bot->GetGUID().GetCounter();
    float const runRadius = PlayerbotAI::IsMainTank(bot) ? LURKER_SPOUT_RUN_RADIUS_MIN :
        LURKER_SPOUT_RUN_RADIUS_MIN +
            (LURKER_SPOUT_RUN_RADIUS_MAX - LURKER_SPOUT_RUN_RADIUS_MIN) * (seed % 100) / 100.0f;

    float const distance = bot->GetExactDist2d(lurker);
    float const botAngle = std::atan2(
        bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    float const relative = Position::NormalizeOrientation(botAngle - lurker->GetOrientation());
    bool const inArc = std::fabs(relative - pi) <= LURKER_SPOUT_RUN_ARC_HALF_WIDTH;

    float const lurkerX = lurker->GetPositionX();
    float const lurkerY = lurker->GetPositionY();
    float const lurkerZ = lurker->GetPositionZ();

    // Lurker is spinning. The only nuance is that bots will not advance more than 30 degrees
    // past directly behind the boss as Sprinting/Spirit Walking can otherwise lap the spin.
    if (int8 const spin = GetLurkerSpoutSpin(lurker))
    {
        float const aheadOfBeam = spin > 0 ? relative : 2.0f * pi - relative;
        float const room = pi + LURKER_SPOUT_RUN_OVERTAKE_MARGIN - aheadOfBeam;
        float const stepAngle = std::min(LURKER_SPOUT_RUN_STEP / runRadius, room);
        constexpr float minStep = 2.0f;
        if (stepAngle * runRadius < minStep)
            return false;

        float const radialStep = std::clamp(
            runRadius - distance, -LURKER_SPOUT_RUN_STEP, LURKER_SPOUT_RUN_STEP);
        float const moveRadius = distance + radialStep;
        float const moveAngle = botAngle + spin * stepAngle;

        constexpr MovementPriority spinPriority = MovementPriority::MOVEMENT_FORCED;
        bot->CastStop();
        if (IsWaitingForLastMove(spinPriority))
            return false;

        // Straight, as a player would run over a spillway rather than path round it
        return MoveStraightTo(
            botAI, lurkerX + moveRadius * std::cos(moveAngle),
            lurkerY + moveRadius * std::sin(moveAngle), lurkerZ, spinPriority);
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

    // Lurker is winding-up, bot is in front of the boss: one far move to the nearer arc edge, at a
    // lower movement priority than the run during the spin.
    int8 const direction = relative < pi ? 1 : -1;
    float const edgeAngle =
        lurker->GetOrientation() + pi - direction * LURKER_SPOUT_RUN_ARC_HALF_WIDTH;
    float const edgeX = lurkerX + runRadius * std::cos(edgeAngle);
    float const edgeY = lurkerY + runRadius * std::sin(edgeAngle);

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    bot->CastStop();
    if (DoesPathRoundLurker(bot, lurker, edgeX, edgeY, lurkerZ, direction))
    {
        return MoveTo(
            SSC_MAP_ID, edgeX, edgeY, lurkerZ, false, false, false, false, priority, true, false);
    }

    float const stepAngle = botAngle + direction * LURKER_SPOUT_RUN_STEP / runRadius;
    return MoveStraightTo(
        botAI, lurkerX + runRadius * std::cos(stepAngle),
        lurkerY + runRadius * std::sin(stepAngle), lurkerZ, priority);
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

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    constexpr float pathTolerance = 3.0f;
    float const pathLength = GetArrivingPathLength(
        bot, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        pathTolerance);

    // A spillway breaks the walkway for the pathfinder: the path stops at it, or goes all the way
    // round. A straight move keeps to walkway height over water.
    constexpr float maxDetourFactor = 1.5f;
    float const distance = bot->GetExactDist2d(position);
    if (pathLength < 0.0f || pathLength > distance * maxDetourFactor)
    {
        return MoveStraightTo(
            botAI, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            priority);
    }

    return MoveTo(
        SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, priority, true, false);
}

bool TheLurkerBelowSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (!_hasRangedPosition)
    {
        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 5.592f; // measured in game to be across from the main tank
        float angle;
        if (!GetRangedArcAngle(bot, arcCenter, arcSpan, angle))
            return false;

        float const targetX =
            lurker->GetPositionX() + LURKER_RANGED_SAFE_DISTANCE * std::cos(angle);
        float const targetY =
            lurker->GetPositionY() + LURKER_RANGED_SAFE_DISTANCE * std::sin(angle);

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

// Only with three living tanks, humans included (a human's Guardian is left to them). Otherwise
// normal tank assist picks up the Guardians.
bool TheLurkerBelowTanksPickUpGuardiansAction::Execute(Event /*event*/)
{
    std::vector<Unit*> const guardians = GetLurkerGuardians(botAI);
    if (guardians.empty())
        return false;

    int8 const myIndex = GetLurkerGuardianTankIndex(botAI);
    if (myIndex < 0)
        return false;

    Unit* guardian = botAI->GetUnit(ClaimGuardianForTank(guardians, myIndex));
    if (!guardian || !guardian->IsAlive())
        return false;

    if (AI_VALUE(Unit*, "current target") != guardian)
        return Attack(guardian);

    if (guardian->GetVictim() == bot)
        return false;

    return CastTankTaunt(botAI, guardian);
}

ObjectGuid TheLurkerBelowTanksPickUpGuardiansAction::ClaimGuardianForTank(
    std::vector<Unit*> const& guardians, int8 myIndex)
{
    auto& assignments = lurkerGuardianTankAssignments[bot->GetInstanceId()];
    ObjectGuid& assignedGuid = assignments[myIndex];

    if (std::any_of(guardians.begin(), guardians.end(),
            [&assignedGuid](Unit* guardian) { return guardian->GetGUID() == assignedGuid; }))
    {
        return assignedGuid;
    }

    // With this tank's slot cleared, a Guardian found in the table is another tank's
    assignedGuid = ObjectGuid::Empty;

    for (Unit* guardian : guardians)
    {
        if (std::find(assignments.begin(), assignments.end(), guardian->GetGUID()) ==
            assignments.end())
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

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    // The Ambushers sometimes chill at the edge of their islets and the Guardians are on land, so
    // the move stops right on top of them. Back to Lurker, or to a Guardian from an islet, it goes
    // to the walkway instead; stock reach takes it on from there.
    uint32 const entry = target->GetEntry();
    bool const toWalkway = ShouldGoToLurkerWalkway(bot, lurker, target);

    // An Ambusher sometimes steps off its islet into the water. A move onto it would take the
    // bot down to the pool floor, so wait for it to come back.
    if (!toWalkway && !IsDryGround(bot, target->GetPositionX(), target->GetPositionY()))
        return false;

    Unit* anchor = toWalkway ? lurker : target;
    float const anchorDistance = toWalkway ? LURKER_WALKWAY_RADIUS : 0.0f;
    float const angle = anchor->GetAngle(bot);
    float const destX = anchor->GetPositionX() + std::cos(angle) * anchorDistance;
    float const destY = anchor->GetPositionY() + std::sin(angle) * anchorDistance;
    float const destZ = anchor->GetPositionZ();

    // Forced toward Lurker, so it replaces a run still under way to an Ambusher or a Guardian
    MovementPriority const priority = entry == Id(SscNpcs::NPC_THE_LURKER_BELOW) ?
        MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;
    return MoveTo(
        SSC_MAP_ID, destX, destY, destZ, false, false, false, false, priority, true, false);
}

// Straight out, since a path from down here follows the water. It keeps the tick while the move
// runs, so nothing else (follow, for one) turns it round underwater.
bool TheLurkerBelowMeleeGetOutOfWaterAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return true;

    // As the direct move chooses, and to the walkway too with no target or one in the water
    Unit* target = AI_VALUE(Unit*, "current target");
    float destX;
    float destY;
    float destZ;
    if (target && !ShouldGoToLurkerWalkway(bot, lurker, target) &&
        IsDryGround(bot, target->GetPositionX(), target->GetPositionY()))
    {
        destX = target->GetPositionX();
        destY = target->GetPositionY();
        destZ = target->GetPositionZ();
    }
    else
    {
        float const angle = lurker->GetAngle(bot);
        destX = lurker->GetPositionX() + std::cos(angle) * LURKER_WALKWAY_RADIUS;
        destY = lurker->GetPositionY() + std::sin(angle) * LURKER_WALKWAY_RADIUS;
        destZ = lurker->GetPositionZ();
    }

    return MoveStraightTo(botAI, destX, destY, destZ, priority);
}

// Leotheras the Blind

bool LeotherasTheBlindWarlockTankAttackDemonFormAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
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

// The rage banked is there in full when an Inner Demon lands. Rage does not decay in combat, and
// white hits alone cannot outthreat Searing Pain.
bool LeotherasTheBlindTanksBuildRageOnDemonFormAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    return AI_VALUE(Unit*, "current target") != leotherasDemon && Attack(leotherasDemon);
}

bool LeotherasTheBlindRangedKeepDistanceAction::Execute(Event /*event*/)
{
    constexpr uint32 minInterval = 0;

    if (Creature* leotherasHumanoid = GetLeotherasHumanoidToAvoid(botAI))
    {
        if (FleePosition(
                leotherasHumanoid->GetPosition(), LEOTHERAS_RANGED_SAFE_DISTANCE, minInterval))
        {
            return true;
        }
    }

    Unit* blastTarget = GetChaosBlastTargetToAvoid(botAI);
    return blastTarget && FleePosition(
        blastTarget->GetPosition(), LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE, minInterval);
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid)
        return false;

    float const currentDistance = bot->GetExactDist2d(leotherasHumanoid);
    if (currentDistance >= LEOTHERAS_WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(leotherasHumanoid, LEOTHERAS_WHIRLWIND_SAFE_DISTANCE - currentDistance);
}

// This method is unlikely to come into play if a Warlock tank is used.
bool LeotherasTheBlindMeleeRunFromChaosBlastAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE &&
        botAI->CanCastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot))
    {
        return true;
    }

    Unit* demonVictim = GetDemonTargetToAvoid(bot, GetLeotherasDemonOrShadow(botAI));
    if (!demonVictim)
        return false;

    float const currentDistance = bot->GetExactDist2d(demonVictim);
    return MoveAway(demonVictim, LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE - currentDistance);
}

bool LeotherasTheBlindDestroyInnerDemonAction::Execute(Event /*event*/)
{
    Creature* innerDemon = GetPersonalInnerDemon(botAI);
    if (!innerDemon)
        return false;

    if (AI_VALUE(Unit*, "current target") != innerDemon)
    {
        bot->CastStop();
        return Attack(innerDemon);
    }

    if (bot->getClass() == CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return HandleFeralTankStrategy(innerDemon);

    if (bot->getClass() == CLASS_HUNTER)
        return HandleHunterStrategy(innerDemon);

    if (PlayerbotAI::IsHeal(bot))
        return HandleHealerStrategy(innerDemon);

    return false;
}

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
// Traps are their main source of damage in this situation.
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
        {
            if (bot->HasAura(Id(SscSpells::SPELL_TREE_OF_LIFE)))
            {
                bot->RemoveOwnedAura(
                    Id(SscSpells::SPELL_TREE_OF_LIFE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
            }

            if (botAI->CanCastSpell("barkskin", bot) && botAI->CastSpell("barkskin", bot))
                return true;

            return botAI->CanCastSpell("wrath", innerDemon) &&
                botAI->CastSpell("wrath", innerDemon);
        }
        case CLASS_PALADIN:
        {
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
        }
        case CLASS_PRIEST:
        {
            return botAI->CanCastSpell("smite", innerDemon) &&
                botAI->CastSpell("smite", innerDemon);
        }
        case CLASS_SHAMAN:
        {
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
        }
        default:
            return false;
    }
}

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
    Unit* shadowVictim = GetShadowTargetToSeparateFrom(botAI);
    if (!shadowVictim)
        return false;

    float const currentDistance = bot->GetExactDist2d(shadowVictim);
    return MoveAway(
        shadowVictim, LEOTHERAS_SHADOW_SEPARATION_DISTANCE - currentDistance, true);
}

bool LeotherasTheBlindMisdirectDemonFormToTankAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
    if (!leotherasDemon)
        return false;

    Player* tank = GetLeotherasWarlockTank(bot);
    if (!tank)
        tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    return MisdirectTargetToTank(botAI, leotherasDemon, tank);
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
        changed |= leotherasHumanoidPhaseStartTime.try_emplace(instanceId, now).second;
        changed |= TrackWhirlwindEnd(leotheras, instanceId, now);
        changed |= leotherasDemonPhaseStartTime.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseStartTime.erase(instanceId) > 0;
    }
    else if (IsLeotherasDemonPhase(botAI))
    {
        changed |= leotherasDemonPhaseStartTime.try_emplace(instanceId, now).second;
        changed |= leotherasHumanoidPhaseStartTime.erase(instanceId) > 0;
        changed |= leotherasWhirlwindEndTime.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseStartTime.erase(instanceId) > 0;
    }
    else if (IsLeotherasFinalPhase(botAI))
    {
        changed |= leotherasFinalPhaseStartTime.try_emplace(instanceId, now).second;
        changed |= TrackWhirlwindEnd(leotheras, instanceId, now);
        changed |= leotherasHumanoidPhaseStartTime.erase(instanceId) > 0;
        changed |= leotherasDemonPhaseStartTime.erase(instanceId) > 0;
    }

    return changed;
}

// Whirlwind resets threat on every tick. Hold dps for a moment after it ends. Do not hold
// dps while Whirlwind is active.
bool LeotherasTheBlindManageDpsWaitTimersAction::TrackWhirlwindEnd(
    Unit* leotheras, uint32 instanceId, uint32 now)
{
    if (Aura const* whirlwind = leotheras->GetAura(Id(SscSpells::SPELL_LEOTHERAS_WHIRLWIND)))
    {
        return leotherasWhirlwindEndTime.try_emplace(
            instanceId, now + whirlwind->GetDuration()).second;
    }

    auto it = leotherasWhirlwindEndTime.find(instanceId);
    if (it == leotherasWhirlwindEndTime.end())
        return false;

    // This addresses the situation in which Whirlwind ends early due to the transition into
    // the final phase being triggered.
    if (now < it->second)
    {
        it->second = now;
        return true;
    }

    if (now - it->second >= LEOTHERAS_WHIRLWIND_DPS_WAIT_MS)
    {
        leotherasWhirlwindEndTime.erase(it);
        return true;
    }

    return false;
}

// Fathom-Lord Karathress
// Note: The strategy uses 4 tanks, and having at least 2 is crucial to separate Caribdis.

bool FathomLordKarathressTanksPositionTargetsAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position position;

    if (PlayerbotAI::IsMainTank(bot))
    {
        // Karathress is tanked by the main tank near his starting position.
        target = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
        position = KARATHRESS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        // Caribdis is pulled far to the West in the corner by the first assist tank.
        target = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        position = CARIBDIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        // Sharkkis is pulled North to the other side of the ramp by the second assist tank.
        target = GetSharkkisTankTarget(botAI);
        position = SHARKKIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, false))
    {
        // Tidalvess is pulled Northwest near the pillar by the third assist tank.
        target = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        position = TIDALVESS_TANK_POSITION;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    if (IsHoldingAnotherTanksCouncilMember(botAI, target))
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

// Caribdis's tank spot is far away so a dedicated healer is needed. Her tank stands on her, so
// seeing her is seeing the tank.
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

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
        priority, true, false);
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank.
bool FathomLordKarathressMisdirectToTanksAction::Execute(Event /*event*/)
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

    return MisdirectTargetToTank(botAI, enemy, tank);
}

// Priority: (1) Spitfire Totem, (2) Tidalvess, (3) Caribdis (ranged only), (4) Sharkkis,
// (5) Sharkkis pet, (6) Karathress.
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
    else if (Unit* pet = GetSharkkisPet(bot))
    {
        target = pet;
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

    // Caribdis is tanked out of sight of the room, and Attack refuses a target out of sight. A bot
    // that loses sight of her is still in range, so nothing else walks it back.
    if (target == caribdis && !bot->IsWithinLOSInMap(caribdis))
        return ApproachCaribdis(caribdis);

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target == caribdis)
    {
        if (MarkTargetWithCross(bot, caribdis))
            return true;

        // Flee is held for the whole fight, so ranged keep out of Tidal Surge themselves
        if (bot->GetExactDist(caribdis) >= CARIBDIS_TIDAL_SURGE_SAFE_DISTANCE)
            return false;

        constexpr uint32 minInterval = 0;
        return FleePosition(
            caribdis->GetPosition(), CARIBDIS_TIDAL_SURGE_SAFE_DISTANCE, minInterval);
    }
    // While a totem stands, skull stays on it: ranged out of its reach are on something else, and
    // marking that would flip the skull against the bots on the totem
    return (!totem || target == totem) && MarkTargetWithSkull(bot, target);
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

// From the second toss on, each knockback starts mid-air: its spline never finishes, or ends at
// the bot's own height. Nothing brings the bot down, so it's sent to the ground below.
bool FathomLordKarathressDropToGroundAfterCycloneAction::Execute(Event /*event*/)
{
    // A spline that never finished still holds the controlled slot, and the bot refuses every
    // move while it does, so it is cleared by hand.
    MotionMaster* mm = bot->GetMotionMaster();
    if (mm->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == EFFECT_MOTION_TYPE)
    {
        mm->Clear();
        bot->StopMoving();
    }

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const floorZ = bot->GetMapHeight(x, y, bot->GetPositionZ(), true, MAX_FALL_DISTANCE);

    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ <= CARIBDIS_CYCLONE_DROP_HEIGHT)
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
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    // The point moves with him as the tank takes him to the corner, so ranged trail him there and
    // are never between him and the tank
    Position const behind = GetTidewalkerStackPoint(*bot, *tidewalker);

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, behind, TIDEWALKER_RANGED_STACK_RADIUS, PATH_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool MorogrimTidewalkerReturnToBossAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, tidewalker, TIDEWALKER_MAX_DISTANCE_FROM_BOSS, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
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

    return MoveAwayFromElementalsAndStriders(vashj);
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

// Phase 3: No fixed position, but move Vashj away from Enchanted Elementals and from Striders
// that another tank has, or nobody does
bool LadyVashjMainTankPositionBossAction::MoveAwayFromElementalsAndStriders(Unit* vashj)
{
    constexpr float searchRadius = 25.0f;
    VashjAddGuids const& adds = context->GetValue<VashjAddGuids>("ssc vashj adds")->RefGet();

    // Surge lands at about 5.5y from her center, so this leaves about 2s of their walk
    constexpr float safeDistance = 10.0f;
    std::vector<Unit*> units;
    bool tooClose = false;
    for (ObjectGuid const& guid : adds.enchanted)
    {
        Unit* enchanted = botAI->GetUnit(guid);
        if (!enchanted || !enchanted->IsAlive())
            continue;

        float const distance = vashj->GetExactDist2d(enchanted);
        if (distance > searchRadius)
            continue;

        units.push_back(enchanted);
        if (distance < safeDistance)
            tooClose = true;
    }

    // Panic fears within 11y, so this keeps it off the melee on her far side. One on her tank
    // follows it anyway.
    constexpr float striderSafeDistance = 18.0f;
    for (ObjectGuid const& guid : adds.striders)
    {
        Unit* strider = botAI->GetUnit(guid);
        if (!strider || !strider->IsAlive() || strider->GetVictim() == bot)
            continue;

        float const distance = vashj->GetExactDist2d(strider);
        if (distance > searchRadius)
            continue;

        units.push_back(strider);
        if (distance < striderSafeDistance)
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
            bot, units, vashj, VASHJ_NORTH_ROCK_CLEARANCE, stepX, stepY, stepZ, backwards,
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
    if (!_hasRangedPosition)
    {
        constexpr float arcCenter = M_PI / 2.0f; // West
        constexpr float arcSpan = M_PI; // 180°
        float angle;
        if (!GetRangedArcAngle(bot, arcCenter, arcSpan, angle))
            return false;

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

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, _rangedPosition, arrivalDistance, PATH_STEP_DISTANCE, stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

// Mechanic tracker only. Fills the slots in group order the first time, then puts the first
// living spare of the right role into each vacated slot. Holders never move.
bool LadyVashjAssignClusterSlotsAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    VashjClusterHolders& holders = vashjClusterHolders[bot->GetInstanceId()];
    auto holdsSlot = [&holders](ObjectGuid guid)
    {
        return std::any_of(holders.begin(), holders.end(), [guid](auto const& cluster)
        {
            return std::find(cluster.begin(), cluster.end(), guid) != cluster.end();
        });
    };

    std::vector<Player*> rangedSpares;
    std::vector<Player*> healerSpares;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SSC_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || holdsSlot(member->GetGUID()))
        {
            continue;
        }

        if (PlayerbotAI::IsRangedDps(member))
            rangedSpares.push_back(member);
        else if (PlayerbotAI::IsHeal(member))
            healerSpares.push_back(member);
    }

    bool changed = false;
    size_t nextRanged = 0;
    size_t nextHealer = 0;
    for (VashjClusterSlot const& slot : GetVashjClusterFillOrder())
    {
        ObjectGuid& holder = holders[slot.cluster][slot.slot];
        if (IsLiveVashjClusterHolder(bot, holder))
            continue;

        bool const isHealerSlot = slot.slot == VASHJ_CLUSTER_HEALER_SLOT;
        std::vector<Player*> const& spares = isHealerSlot ? healerSpares : rangedSpares;
        size_t& next = isHealerSlot ? nextHealer : nextRanged;
        if (next >= spares.size())
            continue;

        holder = spares[next++]->GetGUID();
        changed = true;
    }

    return changed;
}

// The cluster nearest a Tainted Elemental kills it and its healer loots it, and Enchanted
// Elementals are met on their way in.
bool LadyVashjPhase2PositionInClusterAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Position const* clusterPosition =
        GetVashjClusterPositionToReturnTo(bot, AI_VALUE(Unit*, "current target"));
    if (!clusterPosition)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(
            bot, *clusterPosition, VASHJ_CLUSTER_ARRIVAL_DISTANCE, PATH_STEP_DISTANCE,
            stepX, stepY))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

// Nothing else puts ranged at range in phase 3. Out of Entangle first, then a little apart, so one
// spore catches fewer of them.
bool LadyVashjPhase3PositionRangedAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!IsVashjPhase3RangedTooClose(bot, vashj))
        return false;

    std::vector<Unit*> avoid;
    if (bot->GetExactDist2d(vashj) < VASHJ_PHASE_3_RANGED_DISTANCE)
        avoid.push_back(vashj);
    else
        avoid = GetOtherLivingGroupMembers(bot);

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    if (!FindVashjDaisStepAwayFromUnits(
            bot, avoid, nullptr, VASHJ_STANDING_ROCK_CLEARANCE, stepX, stepY, stepZ,
            backwards, &GetToxicSporePositions(botAI)))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, backwards);
}

// Mechanic tracker only. The pick is kept until that Shaman dies or leaves the instance.
bool LadyVashjAssignGroundingShamanAction::Execute(Event /*event*/)
{
    Player* shaman = FindVashjGroundingShaman(bot);
    if (!shaman)
        return false;

    vashjGroundingShaman[bot->GetInstanceId()] = shaman->GetGUID();
    return true;
}

// For absorbing Shock Blast
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (mainTank->HasAura(Id(SscSpells::SPELL_GROUNDING_TOTEM_EFFECT)))
        return false;

    // Grounding Totem Effect reaches 30y from the totem, which lands 3y from the Shaman
    constexpr float distFromTank = 27.0f;
    if (bot->GetDistance(mainTank) > distFromTank)
    {
        // Don't walk back in while Static Charge is keeping this bot clear of somebody
        Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        if (vashj && ShouldAvoidVashjStaticCharge(bot, vashj))
            return false;

        return MoveTo(mainTank, distFromTank, MovementPriority::MOVEMENT_COMBAT);
    }

    return botAI->CanCastSpell("grounding totem", bot) && botAI->CastSpell("grounding totem", bot);
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!IsInVashjStaticChargeReach(bot, vashj))
        return false;

    std::vector<Unit*> avoid;

    if (HasVashjStaticCharge(bot))
    {
        avoid = GetOtherLivingGroupMembers(bot);
    }
    else
    {
        avoid.push_back(vashj->GetVictim());
    }

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    if (!FindVashjDaisStepAwayFromUnits(
            bot, avoid, nullptr, VASHJ_STANDING_ROCK_CLEARANCE, stepX, stepY, stepZ,
            backwards, &GetToxicSporePositions(botAI)))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, backwards);
}

namespace
{

bool IsVashjTargetAllowed(
    Player* bot, VashjTargetFacts const& facts, VashjTargetTier const& tier, Unit* unit)
{
    if (!unit || !unit->IsAlive())
        return false;

    // Melee dps anywhere in reach of it could be standing in a Strider's Panic, and be feared away
    // again and again. Vashj too.
    for (Unit* strider : facts.panicStriders)
    {
        if (unit->GetExactDist(strider) <= VASHJ_STRIDER_PANIC_RADIUS + bot->GetMeleeRange(unit))
            return false;
    }

    if (tier.target == VashjTarget::TaintedElemental)
        return unit == facts.tainted;

    // With no move back any more, a bot far out keeps her and stock reach brings it in
    if (tier.target == VashjTarget::LadyVashj)
        return unit == facts.vashj;

    if (bot->GetExactDist2d(unit) > facts.maxPursueRange)
        return false;

    Position const& center = VASHJ_PLATFORM_CENTER_POSITION;
    if (facts.phase == 2 && unit->GetExactDist2d(center) > facts.maxSearchRange)
        return false;

    // A tanked Strider a little out of range is stepped in to; anything else must be in range
    if (facts.holdsClusterSlot && !bot->IsWithinCombatRange(unit, facts.spellRange) &&
        !IsTankedStriderInStepInReach(bot, unit))
    {
        return false;
    }

    switch (tier.target)
    {
        case VashjTarget::EnchantedElemental:
        {
            return unit->GetEntry() == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL) &&
                facts.vashj->GetExactDist2d(unit) <= tier.maxDistanceFromVashj;
        }
        case VashjTarget::CoilfangStrider:
        case VashjTarget::CoilfangElite:
        {
            uint32 const entry = tier.target == VashjTarget::CoilfangStrider ?
                Id(SscNpcs::NPC_COILFANG_STRIDER) : Id(SscNpcs::NPC_COILFANG_ELITE);

            if (unit->GetEntry() != entry)
                return false;

            if (facts.waitForTank)
                return IsVashjAddHeldByTank(unit);

            if (!facts.oneTankEach)
                return true;

            Player* owner = GetVashjAddOwningTank(bot, unit);
            return owner ? owner == bot :
                IsNearestFreeVashjTank(bot, unit, facts.vashj, facts.phase);
        }
        case VashjTarget::ToxicSporebat:
        {
            // Chasing a bat any higher, or off the dais, walks bots up into the air
            constexpr float maxSporebatHeight = 40.0f;
            return unit->GetEntry() == Id(SscNpcs::NPC_TOXIC_SPOREBAT) &&
                unit->GetPositionZ() - center.GetPositionZ() <= maxSporebatHeight &&
                IsOnVashjDais(
                    unit->GetPositionX(), unit->GetPositionY(), 0.0f,
                    VASHJ_STANDING_ROCK_CLEARANCE);
        }
        default:
            return false;
    }
}

// Enchanted nearest her, Elites and Striders lowest in health, Sporebats nearest the bot
bool IsBetterVashjTarget(Player* bot, Unit* vashj, VashjTarget target, Unit* a, Unit* b)
{
    if (!vashj || !a || !b)
        return false;

    switch (target)
    {
        case VashjTarget::EnchantedElemental:
            return vashj->GetExactDist2d(a) < vashj->GetExactDist2d(b);
        case VashjTarget::CoilfangStrider:
        case VashjTarget::CoilfangElite:
            return a->GetHealthPct() < b->GetHealthPct();
        case VashjTarget::ToxicSporebat:
            return bot->GetDistance(a) < bot->GetDistance(b);
        default:
            return false;
    }
}

Unit* GetBestVashjTarget(
    PlayerbotAI* botAI, VashjTargetFacts const& facts, VashjAddGuids const& adds,
    VashjTargetTier const& tier)
{
    Player* bot = botAI->GetBot();
    GuidVector const* guids = nullptr;
    switch (tier.target)
    {
        case VashjTarget::TaintedElemental:
            return IsVashjTargetAllowed(bot, facts, tier, facts.tainted) ? facts.tainted : nullptr;
        case VashjTarget::LadyVashj:
            return IsVashjTargetAllowed(bot, facts, tier, facts.vashj) ? facts.vashj : nullptr;
        case VashjTarget::EnchantedElemental:
            guids = &adds.enchanted;
            break;
        case VashjTarget::CoilfangElite:
            guids = &adds.elites;
            break;
        case VashjTarget::CoilfangStrider:
            guids = &adds.striders;
            break;
        case VashjTarget::ToxicSporebat:
            guids = &adds.sporebats;
            break;
    }

    Unit* best = nullptr;
    for (ObjectGuid const& guid : *guids)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (IsVashjTargetAllowed(bot, facts, tier, unit) &&
            (!best || IsBetterVashjTarget(bot, facts.vashj, tier.target, unit, best)))
        {
            best = unit;
        }
    }

    return best;
}

// A tank keeps the Elite or Strider it has rather than switching to a free one
bool IsOwnVashjAdd(Player* bot, Unit* target)
{
    if (!target || !target->IsAlive())
        return false;

    if (target->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) &&
        target->GetEntry() != Id(SscNpcs::NPC_COILFANG_STRIDER))
    {
        return false;
    }

    return GetVashjAddOwningTank(bot, target) == bot;
}

} // end anonymous namespace (Vashj targeting)

// A bot keeps its target until a higher tier has one, so it doesn't flip between two of a kind
// as they move. A target out of sight, which Attack() refuses, gives way to the next tier.
bool LadyVashjAssignTargetPriorityAction::Execute(Event /*event*/)
{
    VashjTargetFacts facts;
    facts.vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!facts.vashj)
        return false;

    facts.phase = GetLadyVashjPhase(facts.vashj);
    if (facts.phase != 2 && facts.phase != 3)
        return false;

    bool const isTank = PlayerbotAI::IsTank(bot);
    facts.maxSearchRange = PlayerbotAI::IsRanged(bot) ? 60.0f : 55.0f;
    facts.maxPursueRange = facts.maxSearchRange - 5.0f;
    facts.spellRange = botAI->GetRange("spell");
    facts.holdsClusterSlot = facts.phase == 2 && PlayerbotAI::IsRangedDps(bot);
    facts.waitForTank = facts.phase == 2 && !isTank;
    facts.oneTankEach = isTank;

    if (facts.holdsClusterSlot)
        facts.tainted = GetTaintedElementalToKill(bot);

    VashjAddGuids const& adds = context->GetValue<VashjAddGuids>("ssc vashj adds")->RefGet();
    if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
    {
        for (ObjectGuid const& guid : adds.striders)
        {
            Unit* strider = botAI->GetUnit(guid);
            if (strider && strider->IsAlive())
                facts.panicStriders.push_back(strider);
        }
    }

    std::vector<VashjTargetTier> const& tiers =
        GetVashjTargetTiers(bot, facts.phase, facts.tainted);

    // In phase 2 a tank keeps its own Elite even over a Strider, which the next free tank takes.
    // In phase 3 a Strider comes first.
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (facts.phase == 2 && facts.oneTankEach && IsOwnVashjAdd(bot, currentTarget))
        return false;

    size_t currentTier = tiers.size();
    for (size_t i = 0; currentTarget && i < tiers.size(); ++i)
    {
        if (IsVashjTargetAllowed(bot, facts, tiers[i], currentTarget))
        {
            currentTier = i;
            break;
        }
    }

    for (size_t i = 0; i < currentTier; ++i)
    {
        Unit* candidate = GetBestVashjTarget(botAI, facts, adds, tiers[i]);
        if (candidate && Attack(candidate))
            return true;
    }

    if (currentTier < tiers.size() || !currentTarget)
        return false;

    // Nothing allowed to attack, so drop what it has. A healer's cast is left alone: it is almost
    // always a heal.
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    if (!PlayerbotAI::IsHeal(bot))
        bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());

    return false;
}

// Fear Ward makes Striders tankable. This simulates the real-life strategy of meleeing a Strider
// in an Ogre Suit (due to the extended combat reach).
bool LadyVashjTankApplyFearWardAction::Execute(Event /*event*/)
{
    return bot->AddAura(Id(SscSpells::SPELL_FEAR_WARD), bot);
}

// Each tank works on the Strider it is targeting, so any number can be up at once.
bool LadyVashjPositionCoilfangStriderAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    Unit* strider = AI_VALUE(Unit*, "current target");
    if (!ShouldTankVashjStrider(bot, strider, vashj, phase))
        return false;

    // A Strider stays on whoever it was on, including a main tank who held it into phase 3
    // and has gone back to Vashj, until it is taunted off
    if (strider->GetVictim() != bot)
        return CastTankTaunt(botAI, strider);

    if (phase == 2)
        return MoveStriderToHoldPosition(strider);

    return MoveStriderAwayFromVashj(strider, vashj);
}

// Phase 2: the hold point nearest the Strider.
bool LadyVashjPositionCoilfangStriderAction::MoveStriderToHoldPosition(Unit* strider)
{
    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToBringTankedUnitTo(
            bot, strider, GetVashjStriderHoldPosition(*strider),
            VASHJ_ADD_TANK_ARRIVAL_DISTANCE, stepX, stepY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Phase 3: keep the Strider away from Vashj, where bots tend to congregate to take down
// elementals.
bool LadyVashjPositionCoilfangStriderAction::MoveStriderAwayFromVashj(
    Unit* strider, Unit* vashj)
{
    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    if (!FindVashjDaisStepAwayFromUnits(
            bot, { vashj }, strider, VASHJ_STANDING_ROCK_CLEARANCE, stepX, stepY, stepZ,
            backwards, &GetToxicSporePositions(botAI)))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// For a human tank: about 12y in front of the north rock's tip, or south-east of the middle.
bool LadyVashjPositionCoilfangEliteAction::Execute(Event /*event*/)
{
    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) ||
        elite->GetVictim() != bot)
    {
        return false;
    }

    float stepX;
    float stepY;
    bool backwards;
    if (!GetStepToBringTankedUnitTo(
            bot, elite, GetVashjEliteTankPosition(*elite), VASHJ_ADD_TANK_ARRIVAL_DISTANCE,
            stepX, stepY, backwards))
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
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, vashj, VASHJ_IDLE_TANK_DISTANCE, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

// Chosen when the elemental spawns, and again if the looter dies before it does.
bool LadyVashjAssignTaintedCoreLooterAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    int8 const cluster = GetNearestVashjCluster(tainted);
    Player* looter = FindTaintedCoreLooter(bot, tainted, cluster);
    if (!looter)
        return false;

    vashjTaintedCoreLooter.insert_or_assign(bot->GetInstanceId(),
        TaintedCoreLooter{ tainted->GetGUID(), looter->GetGUID(), cluster });

    // A new elemental gets a new chain. A re-picked looter becomes the old one's origin bot,
    // and gives up any catcher's spot it had.
    VashjCorePassingChain* chain = GetVashjCorePassingChain(bot);
    if (!chain || chain->tainted != tainted->GetGUID())
    {
        PlanVashjCorePassingChain(bot, tainted, looter);
    }
    else
    {
        chain->originBot = looter->GetGUID();
        if (int8 const index = GetVashjCoreCatcherIndex(*chain, looter); index >= 0)
            ReassignVashjCoreCatcher(bot, *chain, static_cast<size_t>(index));
    }

    return true;
}

// From a cluster the edge of the dais usually blocks the view down to it, and Attack() refuses a
// target out of sight. Spell range keeps hunters out of their dead zone.
bool LadyVashjAttackTaintedElementalAction::Execute(Event /*event*/)
{
    Unit* tainted = GetTaintedElementalToKill(bot);
    if (!tainted)
        return false;

    // Killers are ranged dps
    constexpr float stopDistance = 3.0f;
    if (!bot->IsWithinCombatRange(tainted, botAI->GetRange("spell")) ||
        !bot->IsWithinLOSInMap(tainted))
    {
        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, tainted, stopDistance, stepX, stepY))
            return false;

        // Still true between steps, so nothing lower starts a cast on the way
        bool const moved = MoveTo(
            SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);

        return moved || bot->isMoving();
    }

    if (AI_VALUE(Unit*, "current target") != tainted)
        return Attack(tainted);

    return false;
}

// The looter sends the loot packets itself: OpenLootAction holds the bot's next tick back by
// lootDelay (1s by default), which would hold back the first throw.
bool LadyVashjLootTaintedCoreAction::Execute(Event /*event*/)
{
    Creature* tainted = GetAssignedTaintedElemental(bot);
    if (!tainted)
        return false;

    if (bot->GetDistance(tainted) > VASHJ_CORE_LOOT_RANGE)
    {
        // Steps stop just inside loot range, centre to centre
        constexpr float rangeMargin = 0.5f;
        float const stopDistance =
            VASHJ_CORE_LOOT_RANGE + bot->GetCombatReach() + tainted->GetCombatReach() - rangeMargin;

        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, tainted, stopDistance, stepX, stepY))
            return false;

        return MoveTo(
            SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (tainted->IsAlive())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && HasTaintedCore(member))
            return false;
    }

    int8 const coreSlot = GetTaintedCoreLootSlot(tainted);
    if (coreSlot < 0)
        return false;

    // As LootObject::IsLootPossible() checks a corpse, less its height limit
    if (!bot->isAllowedToLoot(tainted))
        return false;

    // Open, take the core, close, as a player's client does. Handled in this order on the bot's
    // next session update. Closing clears the corpse's lootable flag once it is empty.
    WorldPacket* openPacket = new WorldPacket(CMSG_LOOT, 8);
    *openPacket << tainted->GetGUID();
    bot->GetSession()->QueuePacket(openPacket);

    WorldPacket* storePacket = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *storePacket << static_cast<uint8>(coreSlot);
    bot->GetSession()->QueuePacket(storePacket);

    WorldPacket* releasePacket = new WorldPacket(CMSG_LOOT_RELEASE, 8);
    *releasePacket << tainted->GetGUID();
    bot->GetSession()->QueuePacket(releasePacket);

    return true;
}

// Every member of the chain runs this: catchers walk to their spots, and whoever holds the core
// throws it on or, in reach of the generator, uses it.
bool LadyVashjPassTheTaintedCoreAction::Execute(Event /*event*/)
{
    VashjCorePassingChain* chain = GetVashjCorePassingChain(bot);
    if (!chain || chain->failed)
        return false;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    Item* core =
        HasTaintedCore(bot) ? bot->GetItemByEntry(Id(SscItems::ITEM_TAINTED_CORE)) : nullptr;
    if (!core)
        return index >= 0 && MoveToCoreSpot(*chain, index);

    GameObject* generator = botAI->GetGameObject(chain->generator);

    if (index > chain->reached)
        chain->reached = index;

    if (!generator || generator->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
    {
        ReplanVashjCorePassingChain(bot, *chain, ObjectGuid::Empty);
        return true;
    }

    if (generator->IsAtInteractDistance(*bot, generator->GetInteractionDistance()))
        return UseCoreOnGenerator(core, generator);

    // The last catcher, rooted out of reach of the generator
    size_t const next = static_cast<size_t>(index + 1);
    if (next >= chain->catchers.size())
    {
        ReplanVashjCorePassingChain(bot, *chain, ObjectGuid::Empty);
        return true;
    }

    return ThrowCore(*chain, next, core, generator);
}

// False once there, so the catcher can fight or heal from its spot while it waits.
bool LadyVashjPassTheTaintedCoreAction::MoveToCoreSpot(VashjCorePassingChain& chain, int8 index)
{
    size_t const next = static_cast<size_t>(index + 1);
    bool const last = next == chain.catchers.size();
    float const arrival = GetVashjCoreSpotArrivalDistance(chain, index);
    VashjCoreCatcher& catcher = chain.catchers[index];
    // The second catcher is picked and sets out with the first
    if (index == 0 && !last)
        ReleaseVashjCoreCatcher(bot, chain, next);

    if (bot->GetExactDist2d(catcher.spot) <= arrival)
    {
        // Each later one once the catcher before it stands on its spot
        if (!catcher.arrived)
        {
            catcher.arrived = true;
            if (!last)
                ReleaseVashjCoreCatcher(bot, chain, next);
        }

        return false;
    }

    Position const& spot = catcher.spot;

    float stepX;
    float stepY;
    if (!GetPathStepTowardPoint(bot, spot, arrival / 2.0f, PATH_STEP_DISTANCE, stepX, stepY))
        return false;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// For the last catcher, on its spot means in use range of the generator, since the core roots
// it. A throw that doesn't land is tried once more, then replanned without that catcher.
bool LadyVashjPassTheTaintedCoreAction::ThrowCore(
    VashjCorePassingChain& chain, size_t next, Item* core, GameObject* generator)
{
    uint32 const now = getMSTime();
    VashjCoreCatcher const& catcher = chain.catchers[next];
    Player* player = ObjectAccessor::GetPlayer(*bot, catcher.bot);
    if (!player || !player->IsAlive() || !player->IsInMap(bot))
    {
        ReassignVashjCoreCatcher(bot, chain, next);
        return false;
    }

    if (chain.waitTarget != player->GetGUID())
    {
        chain.waitTarget = player->GetGUID();
        chain.waitStart = now;
        chain.blockedStart = 0;
    }

    bool const last = next + 1 == chain.catchers.size();
    constexpr float onSpotDistance = 1.5f;
    bool const onSpot = last ?
        generator->IsAtInteractDistance(*player, generator->GetInteractionDistance()) :
        player->GetExactDist2d(catcher.spot) <= onSpotDistance;
    if (!onSpot)
    {
        constexpr uint32 lateMs = 10 * IN_MILLISECONDS;
        if (getMSTimeDiff(chain.waitStart, now) > lateMs)
            ReassignVashjCoreCatcher(bot, chain, next);

        return false;
    }

    // Throw Key's range, edge to edge in 3D as the spell measures it
    constexpr float throwKeyRange = 40.0f;
    if (bot->GetDistance(player) > throwKeyRange || !bot->IsWithinLOSInMap(player))
    {
        constexpr uint32 blockedMs = 3 * IN_MILLISECONDS;
        if (!chain.blockedStart)
            chain.blockedStart = now;
        else if (getMSTimeDiff(chain.blockedStart, now) > blockedMs)
        {
            ReplanVashjCorePassingChain(bot, chain, ObjectGuid::Empty);
            return true;
        }

        return false;
    }

    chain.blockedStart = 0;

    // Passes one right after another look rushed, and a bot could throw twice in a row. Also the
    // wait before a throw that didn't land is tried again.
    constexpr uint32 throwIntervalMs = 2 * IN_MILLISECONDS;
    if (chain.throwTime && getMSTimeDiff(chain.throwTime, now) < throwIntervalMs)
        return false;

    constexpr uint8 maxThrows = 2;
    if (chain.throwTarget == player->GetGUID())
    {
        if (++chain.failedThrows >= maxThrows)
        {
            ReplanVashjCorePassingChain(bot, chain, player->GetGUID());
            return true;
        }
    }
    else
        chain.failedThrows = 0;

    chain.throwTarget = player->GetGUID();
    chain.throwTime = now;
    botAI->ImbueItem(core, player);
    return true;
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnGenerator(Item* core, GameObject* generator)
{
    if (bot->CanUseItem(core) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    // Opening on the generator, as a player using the core on it; GameObject::Use() does nothing.
    // The bot's next tick skips while Opening is cast, and a second use would restart it.
    botAI->ImbueItem(core, TARGET_FLAG_GAMEOBJECT, generator->GetGUID());
    return true;
}

// As a player would delete it from their bags. Removing it takes off its Paralyze.
bool LadyVashjDestroyTaintedCoreAction::Execute(Event /*event*/)
{
    if (!HasTaintedCore(bot))
        return false;

    bot->DestroyItemCount(Id(SscItems::ITEM_TAINTED_CORE), -1, true);
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

bool LadyVashjAvoidToxicSporesAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    std::vector<Position> const& spores = GetToxicSporePositions(botAI);
    bool const tanking = vashj->GetVictim() == bot;

    // A breakout walk ends at the spot, after a while, or once a pool lands near the spot
    if (_hasBreakoutSpot)
    {
        constexpr uint32 maxBreakoutMs = 12 * IN_MILLISECONDS;
        _hasBreakoutSpot = tanking &&
            getMSTimeDiff(_breakoutStartTime, getMSTime()) < maxBreakoutMs &&
            std::none_of(spores.begin(), spores.end(), [this](Position const& spore)
            {
                return spore.GetExactDist2d(_breakoutSpot) < TOXIC_SPORES_TANK_AVOID_RADIUS;
            });

        if (_hasBreakoutSpot && StepTowardBreakoutSpot(vashj))
            return true;
    }

    float stepX;
    float stepY;
    float stepZ;
    bool backwards;
    float const rockClearance =
        tanking ? VASHJ_NORTH_ROCK_CLEARANCE : VASHJ_STANDING_ROCK_CLEARANCE;
    bool found = FindVashjDaisStepAwayFromPositions(
        bot, spores, vashj, rockClearance, stepX, stepY, stepZ, backwards);

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
                bot, nearest, vashj, rockClearance, stepX, stepY, stepZ, backwards);
        }
    }

    // Her tank pinned where no step gains on the pools crosses the edge of one if it has to
    if (!found && tanking && FindVashjTankBreakoutSpot(bot, spores, _breakoutSpot))
    {
        _hasBreakoutSpot = true;
        _breakoutStartTime = getMSTime();
        return StepTowardBreakoutSpot(vashj);
    }

    if (!found)
        return false;

    MovementPriority const priority = tanking ?
        MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, backwards);
}

bool LadyVashjAvoidToxicSporesAction::StepTowardBreakoutSpot(Unit* vashj)
{
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const distance = bot->GetExactDist2d(_breakoutSpot);

    constexpr float arrivalDistance = 1.5f;
    if (distance <= arrivalDistance)
    {
        _hasBreakoutSpot = false;
        return false;
    }

    // Backwards when the way leads away from her, as with the other tank steps
    float const dirX = (_breakoutSpot.GetPositionX() - botX) / distance;
    float const dirY = (_breakoutSpot.GetPositionY() - botY) / distance;
    bool const backwards = dirX * (vashj->GetPositionX() - botX) +
        dirY * (vashj->GetPositionY() - botY) < 0.0f;
    float const moveDist = backwards ? PATH_BACKWARD_STEP_DISTANCE : PATH_STEP_DISTANCE;

    float stepX;
    float stepY;
    float stepZ;
    if (!CanTakeStepTowards(
            bot, _breakoutSpot.GetPositionX(), _breakoutSpot.GetPositionY(), moveDist, stepX,
            stepY, stepZ))
    {
        _hasBreakoutSpot = false;
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// Melee stay in reach at the nearest angle no pool covers. With the whole ring covered they step
// out as other bots do, and a boxed-in melee walks straight out of the nearest pool.
bool LadyVashjMeleeMoveAroundToxicSporesAction::Execute(Event event)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;

    // Every move here is a combat one, except Vashj's own target's, which the avoid action forces
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if ((!vashj || vashj->GetVictim() != bot) && IsWaitingForLastMove(priority))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    std::vector<Position> const& spores = GetToxicSporePositions(botAI);

    float stepX;
    float stepY;
    float stepZ;
    if (target && target->IsAlive() && GetMeleeRingStepClearOfSpores(
            bot, target, spores, TOXIC_SPORES_AVOID_RADIUS, stepX, stepY, stepZ))
    {
        return MoveTo(
            SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
    }

    if (!IsNearToxicSpores(botAI, TOXIC_SPORES_AVOID_RADIUS))
        return false;

    if (LadyVashjAvoidToxicSporesAction::Execute(event))
        return true;

    if (!IsNearToxicSpores(botAI, TOXIC_SPORES_HIT_RADIUS) || !GetStepOutOfNearestSpore(
            bot, spores, TOXIC_SPORES_AVOID_RADIUS, stepX, stepY, stepZ))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
}

// Recomputed every tick; walking a clear line keeps the same point best, and once the straight
// line to cast range is clear, stock reach-spell takes over again.
bool LadyVashjRangedReachAroundToxicSporesAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* target;
    float range;
    if (!GetVashjReachBlockedBySpores(botAI, target, range))
        return false;

    float stepX;
    float stepY;
    float stepZ;
    if (!GetStepToCastRangeAroundSpores(
            bot, target, range, GetToxicSporePositions(botAI), stepX, stepY, stepZ))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
}

bool LadyVashjPaladinUseHandOfFreedomAction::Execute(Event /*event*/)
{
    Player* target =
        GetVashjHandOfFreedomTarget(botAI, AI_VALUE2(Unit*, "find target", "lady vashj"));
    return target && botAI->CanCastSpell("hand of freedom", target) &&
        botAI->CastSpell("hand of freedom", target);
}

bool LadyVashjRogueUseCloakOfShadowsAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot);
}
