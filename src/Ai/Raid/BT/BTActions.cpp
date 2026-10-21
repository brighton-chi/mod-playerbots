/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTActions.h"
#include "BTHelpers.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <utility>

using namespace BtHelpers;
using namespace EncounterHelpers;

// Shared

bool BlackTempleResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;

    Action* spineRemoverAction =
        context->GetAction("high warlord naj'entus remove impaling spine");
    if (spineRemoverAction &&
        static_cast<HighWarlordNajentusRemoveImpalingSpineAction*>(
            spineRemoverAction)->ResetEncounterState())
    {
        reset = true;
    }

    Action* spineThrowerAction = context->GetAction("high warlord naj'entus throw impaling spine");
    if (spineThrowerAction &&
        static_cast<HighWarlordNajentusThrowImpalingSpineAction*>(
            spineThrowerAction)->ResetEncounterState())
    {
        reset = true;
    }

    Action* kiteAction = context->GetAction("supremus kite boss");
    if (kiteAction && static_cast<SupremusKiteBossAction*>(kiteAction)->ResetEncounterState())
        reset = true;

    Action* fireAvoidAction = context->GetAction("supremus move away from fire");
    if (fireAvoidAction &&
        static_cast<SupremusMoveAwayFromFireAction*>(fireAvoidAction)->ResetEncounterState())
    {
        reset = true;
    }

    Action* fireReachAction = context->GetAction("supremus reach around fire");
    if (fireReachAction &&
        static_cast<SupremusReachAroundFireAction*>(fireReachAction)->ResetEncounterState())
    {
        reset = true;
    }

    Action* gathiosTankAction = context->GetAction("illidari council main tank position gathios");
    if (gathiosTankAction && static_cast<IllidariCouncilMainTankPositionGathiosAction*>(
            gathiosTankAction)->ResetTankStep())
    {
        reset = true;
    }

    Action* zerevorTankAction = context->GetAction("illidari council mage tank position zerevor");
    if (zerevorTankAction && static_cast<IllidariCouncilMageTankPositionZerevorAction*>(
            zerevorTankAction)->ResetTankStep())
    {
        reset = true;
    }

    Action* flameTankAction =
        context->GetAction("illidan stormrage assist tanks handle flames of azzinoth");
    if (flameTankAction &&
        static_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(
            flameTankAction)->ResetTankStep())
    {
        reset = true;
    }

    Action* illidanTankAction = context->GetAction("illidan stormrage main tank reposition boss");
    if (illidanTankAction &&
        static_cast<IllidanStormrageMainTankRepositionBossAction*>(
            illidanTankAction)->ResetEncounterState())
    {
        reset = true;
    }

    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return reset;

    reset |= BtResetInstance(bot->GetInstanceId());
    reset |= PruneShadowmoonReaverAbsorptionStarts(bot);

    reset |= ClearStrategyTargetIcon(bot, RtiTargetValue::skullIndex, {
        Id(BtNpcs::NPC_ASHTONGUE_CHANNELER),
        Id(BtNpcs::NPC_ASHTONGUE_SORCERER),
    });

    return reset;
}

bool BlackTempleMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, mainTank);
}

bool BlackTemplePositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    Unit* victim = boss->GetVictim();
    if (!victim)
        return false;

    if (victim != bot)
    {
        Player* playerVictim = victim->ToPlayer();
        if (!_allTanks || !playerVictim || !PlayerbotAI::IsTank(playerVictim))
            return false;
    }

    if (PlayerbotAI::IsTank(bot) && !bot->IsWithinMeleeRange(boss))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, _position, _arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool BlackTempleSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, _distance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), _distance);
}

bool BlackTempleCastSpellReflectionAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(BtSpells::SPELL_SPELL_REFLECTION), bot) &&
        botAI->CastSpell(Id(BtSpells::SPELL_SPELL_REFLECTION), bot);
}

// Trash

// Damage on the Sister of Pleasure is split evenly with her Sister of Pain, and they have equal
// health, so killing Pleasure kills both without anyone hitting Pain through Shell of Pain.
bool FocusSisterOfPleasureAction::Execute(Event /*event*/)
{
    if (IsMechanicTrackerBot(bot, BT_MAP_ID))
    {
        Unit* pleasure = FindLinkedSisterOfPleasure(botAI);
        return pleasure && MarkTargetWithSkull(bot, pleasure);
    }

    // Everyone else drops Sister of Pain, and the DPS target then picks the skull.
    Unit* pain = AI_VALUE(Unit*, "current target");
    if (!pain || pain->GetEntry() != Id(BtNpcs::NPC_SISTER_OF_PAIN))
        return false;

    for (CurrentSpellTypes const type :
         { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL })
    {
        Spell* spell = bot->GetCurrentSpell(type);
        if (spell && spell->m_targets.GetUnitTargetGUID() == pain->GetGUID())
            bot->InterruptSpell(type);
    }

    bot->AttackStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    return true;
}

bool ShadowmoonReaverStopWandAction::Execute(Event /*event*/)
{
    bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
    return true;
}

// Imps, water elementals, succubi and felhunters are kept off Reavers while anything else can be
// attacked. With only Reavers left, the pet waits passive by its owner while one is unsafe, and
// is sent onto them otherwise.
bool ShadowmoonReaverControlCasterPetAction::Execute(Event /*event*/)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
        return false;

    auto const& reavers = context->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    if (reavers.empty())
        return RestoreReactState(pet);

    Unit* target = FindPetTargetOtherThanReaver(botAI);
    if (!target && IsAnyShadowmoonReaverUnsafeForMagic(botAI))
    {
        if (pet->HasReactState(REACT_PASSIVE))
            return false;

        _previousReactState = pet->GetReactState();
        _passivePet = pet->GetGUID();
        pet->AttackStop();
        pet->InterruptNonMeleeSpells(false);
        pet->SetReactState(REACT_PASSIVE);
        if (CharmInfo* charmInfo = pet->GetCharmInfo())
        {
            charmInfo->SetIsCommandAttack(false);
            charmInfo->SetIsCommandFollow(true);
        }

        pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        return true;
    }

    bool const restored = RestoreReactState(pet);
    Unit* victim = pet->GetVictim();
    if ((!victim || !victim->IsAlive()) && !pet->HasReactState(REACT_PASSIVE))
    {
        // Idle, as after waiting passive: sent to the other target, else to a Reaver now that
        // all are safe, the owner's target first.
        if (target)
            return CommandPetAttack(pet, target);

        auto const canAttackReaver = [pet](Unit* unit)
        {
            return unit && unit->IsAlive() &&
                unit->GetEntry() == Id(BtNpcs::NPC_SHADOWMOON_REAVER) &&
                pet->IsValidAttackTarget(unit);
        };

        Unit* ownerTarget = AI_VALUE(Unit*, "current target");
        if (canAttackReaver(ownerTarget))
            return CommandPetAttack(pet, ownerTarget);

        for (ObjectGuid const& guid : reavers)
        {
            Unit* reaver = botAI->GetUnit(guid);
            if (canAttackReaver(reaver))
                return CommandPetAttack(pet, reaver);
        }

        return restored;
    }

    if (!target || victim->GetEntry() != Id(BtNpcs::NPC_SHADOWMOON_REAVER))
        return restored;

    return CommandPetAttack(pet, target);
}

bool ShadowmoonReaverControlCasterPetAction::RestoreReactState(Guardian* pet)
{
    if (_passivePet.IsEmpty())
        return false;

    bool const isSamePet = pet->GetGUID() == _passivePet;
    _passivePet.Clear();
    if (!isSamePet)
        return false;

    pet->SetReactState(_previousReactState);
    return true;
}

bool ShadowmoonReaverControlCasterPetAction::CommandPetAttack(Guardian* pet, Unit* target)
{
    pet->AttackStop();
    pet->InterruptNonMeleeSpells(false);
    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->SetTarget(target->GetGUID());
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
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

// High Warlord Naj'entus

bool HighWarlordNajentusAssignSpineRemoverAction::Execute(Event /*event*/)
{
    Player* impaled = FindNajentusUnassignedImpaledPlayer(bot);
    if (!impaled)
        return false;

    // Drop entries for impales that have ended, and any earlier one for this impaled player.
    std::optional<std::vector<NajentusSpineAssignment>>& assignmentsField =
        BtState(bot->GetInstanceId()).najentusSpineAssignments;
    std::vector<NajentusSpineAssignment>& assignments =
        assignmentsField ? *assignmentsField : assignmentsField.emplace();
    std::erase_if(assignments,
        [this, impaled](NajentusSpineAssignment const& assignment)
        {
            return assignment.impaled == impaled->GetGUID() ||
                !IsNajentusImpaled(ObjectAccessor::GetPlayer(*bot, assignment.impaled));
        });

    Player* remover = FindNajentusSpineRemover(bot, impaled);
    if (!remover)
        return false;

    assignments.push_back({ impaled->GetGUID(), remover->GetGUID() });
    return true;
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event /*event*/)
{
    Player* impaled = GetNajentusImpaledPlayerToFree(bot);
    if (!impaled)
        return false;

    // The spine is summoned where the impaled player stands.
    constexpr float searchRadius = 5.0f;
    GameObject* spineGo = impaled->FindNearestGameObject(
        Id(BtObjects::GO_NAJENTUS_SPINE), searchRadius, true);
    if (!spineGo)
        return false;

    bool const atSpine = bot->GetExactDist2d(spineGo) <= 3.0f;

    // One reaction delay of 1 to 2 s per spine, before clicking or walking to it.
    if (spineGo->GetGUID() != _spineGuid)
    {
        _spineGuid = spineGo->GetGUID();
        _usedSpine = false;
        _reactionStartTime = getMSTime();
        _reactionDelay = urand(1000, 2000);
    }

    if (_usedSpine || GetMSTimeDiffToNow(_reactionStartTime) < _reactionDelay)
        return false;

    bot->CastStop();

    if (!atSpine)
    {
        return MoveTo(
            BT_MAP_ID, spineGo->GetPositionX(), spineGo->GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    spineGo->Use(bot);
    _usedSpine = true;
    return true;
}

bool HighWarlordNajentusAssignSpineThrowerAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    Player* thrower = FindNajentusSpineThrower(bot, najentus);
    if (!thrower)
        return false;

    BtState(bot->GetInstanceId()).najentusSpineThrower = thrower->GetGUID();
    return true;
}

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (!bot->IsWithinCombatRange(najentus, NAJENTUS_HURL_SPINE_RANGE))
    {
        float const approachDist = NAJENTUS_HURL_SPINE_RANGE - NAJENTUS_HURL_SPINE_APPROACH_MARGIN +
            bot->GetCombatReach() + najentus->GetCombatReach();
        float const angle = najentus->GetAngle(bot);
        float const targetX = najentus->GetPositionX() + approachDist * std::cos(angle);
        float const targetY = najentus->GetPositionY() + approachDist * std::sin(angle);

        bot->CastStop();
        return MoveTo(
            BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    Aura* shield = najentus->GetAura(Id(BtSpells::SPELL_TIDAL_SHIELD));
    Item* spine = bot->GetItemByEntry(Id(BtItems::ITEM_NAJENTUS_SPINE));
    if (!shield || !spine)
        return false;

    // Reaction delay of 0.5 to 1.5 s, counted from the shield going up.
    if (!_throwDelay)
        _throwDelay = urand(500, 1500);

    if (static_cast<uint32>(shield->GetMaxDuration() - shield->GetDuration()) < _throwDelay)
        return false;

    bot->CastStop();
    botAI->ImbueItem(spine, najentus);
    BtState(bot->GetInstanceId()).najentusSpineThrowTime = getMSTime();
    _throwDelay = 0;
    return true;
}

// Supremus

namespace // Supremus
{

constexpr float NEVER_CAUGHT = std::numeric_limits<float>::max();

float GetKiteHeadingAngle(uint8 index)
{
    return index * 2.0f * M_PI / SUPREMUS_KITE_HEADINGS;
}

// How far along a heading from (x, y) stays inside the open area.
float GetKiteBoundaryDistance(float x, float y, float angle)
{
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);
    float distance = std::numeric_limits<float>::max();
    if (dx > 0.0f)
        distance = std::min(distance, (SUPREMUS_BOUNDARY_MAX_X - x) / dx);
    else if (dx < 0.0f)
        distance = std::min(distance, (SUPREMUS_BOUNDARY_MIN_X - x) / dx);

    if (dy > 0.0f)
        distance = std::min(distance, (SUPREMUS_BOUNDARY_MAX_Y - y) / dy);
    else if (dy < 0.0f)
        distance = std::min(distance, (SUPREMUS_BOUNDARY_MIN_Y - y) / dy);

    // Negative only for a heading that leads further out from outside the area.
    return std::max(0.0f, distance);
}

// Along a heading from (x, y): how far to the first hazard's safe zone that (x, y) is outside of,
// and how far until clear of every one it is inside (0 if none).
void GetFireCrossing(
    std::vector<SupremusHazard> const& hazards, float x, float y, float angle,
    float& entryDistance, float& exitDistance)
{
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);

    entryDistance = std::numeric_limits<float>::max();
    exitDistance = 0.0f;
    for (SupremusHazard const& hazard : hazards)
    {
        // The heading crosses the circle where t^2 + 2 * along * t + outside = 0.
        float const offsetX = x - hazard.position.GetPositionX();
        float const offsetY = y - hazard.position.GetPositionY();
        float const along = offsetX * dx + offsetY * dy;
        float const outside =
            offsetX * offsetX + offsetY * offsetY - hazard.safeRadius * hazard.safeRadius;
        float const discriminant = along * along - outside;
        if (outside < 0.0f)
            exitDistance = std::max(exitDistance, -along + std::sqrt(discriminant));
        else if (along < 0.0f && discriminant > 0.0f)
            entryDistance = std::min(entryDistance, -along - std::sqrt(discriminant));
    }
}

} // end anonymous namespace (Supremus)

// Re-planned every step. Each heading runs until a wall, the open area's edge or fire stops it,
// and is scored by when he would reach the bot. Fire is ignored only when that is the one way to
// stay out of his reach.
bool SupremusKiteBossAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    // Only its own last step holds the kite; a fire walk begun before the fixate gives way.
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
    if (lastMove.msTime != _lastStepTime)
        lastMove.Set(nullptr);
    else if (IsWaitingForLastMove(priority))
        return false;

    float const timeRemaining = GetSupremusFixateTimeRemaining(supremus);
    float const catchDistance = GetSupremusCatchDistance(bot, supremus);
    // At least a volcano's width, so a bot standing in one sees the way out.
    float const probeDistance = std::max(
        bot->GetSpeed(MOVE_RUN) * timeRemaining, 2.0f * SUPREMUS_VOLCANO_SAFE_DISTANCE);
    std::vector<SupremusHazard> const& hazards = GetSupremusHazards(botAI);

    // Keep the way the last step went, or else start straight away from him.
    constexpr uint32 headingMemoryMs = 2000;
    float const preferredAngle = getMSTimeDiff(_lastStepTime, getMSTime()) < headingMemoryMs ?
        _lastAngle : supremus->GetAngle(bot);

    std::array<float, SUPREMUS_KITE_HEADINGS> openDistances;
    for (uint8 i = 0; i < SUPREMUS_KITE_HEADINGS; ++i)
    {
        float const angle = GetKiteHeadingAngle(i);
        openDistances[i] = std::min(
            GetCollisionFreeDistance(angle, probeDistance),
            GetKiteBoundaryDistance(bot->GetPositionX(), bot->GetPositionY(), angle));
    }

    // He one-shots what he reaches, so a heading he reaches only delays it. Headings he never
    // reaches go by the quickest way out of fire, then the smallest turn.
    auto const isBetter = [](KitePlan const& a, KitePlan const& b)
    {
        if (a.timeUntilCaught != b.timeUntilCaught)
            return a.timeUntilCaught > b.timeUntilCaught;

        if (a.fireExitDistance != b.fireExitDistance)
            return a.fireExitDistance < b.fireExitDistance;

        return a.turn < b.turn;
    };

    auto const planHeadings = [&](std::vector<SupremusHazard> const& fire)
    {
        std::vector<KitePlan> plans;
        for (uint8 i = 0; i < SUPREMUS_KITE_HEADINGS; ++i)
        {
            float const angle = GetKiteHeadingAngle(i);
            float entryDistance;
            float exitDistance;
            GetFireCrossing(
                fire, bot->GetPositionX(), bot->GetPositionY(), angle, entryDistance,
                exitDistance);
            float const runDistance = std::min(openDistances[i], entryDistance);
            // Stopping short of leaving the fire the bot stands in is no way out.
            if (runDistance < exitDistance)
                continue;

            float const turn = std::abs(std::remainder(angle - preferredAngle, 2.0f * M_PI));
            float const timeUntilCaught =
                SimulateChase(supremus, timeRemaining, catchDistance, angle, runDistance);
            plans.push_back({ angle, runDistance, exitDistance, turn, timeUntilCaught });
        }

        std::sort(plans.begin(), plans.end(), isBetter);
        return plans;
    };

    std::vector<KitePlan> plans = planHeadings(hazards);
    if (!hazards.empty() && (plans.empty() || plans.front().timeUntilCaught != NEVER_CAUGHT))
    {
        std::vector<KitePlan> ignoringFire = planHeadings({});
        if (plans.empty() || ignoringFire.front().timeUntilCaught == NEVER_CAUGHT)
            plans = std::move(ignoringFire);
    }

    for (KitePlan const& plan : plans)
    {
        float const targetX = bot->GetPositionX() + plan.runDistance * std::cos(plan.angle);
        float const targetY = bot->GetPositionY() + plan.runDistance * std::sin(plan.angle);
        float stepX;
        float stepY;
        float stepZ;
        if (!CanTakeStepTowards(
                bot, targetX, targetY, SUPREMUS_KITE_STEP_DISTANCE, stepX, stepY, stepZ))
        {
            continue;
        }

        if (!MoveTo(
                BT_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true,
                false))
        {
            return false;
        }

        _lastAngle = plan.angle;
        _lastStepTime = lastMove.msTime;
        return true;
    }

    return false;
}

float SupremusKiteBossAction::GetCollisionFreeDistance(float angle, float maxDistance)
{
    float x = bot->GetPositionX() + maxDistance * std::cos(angle);
    float y = bot->GetPositionY() + maxDistance * std::sin(angle);
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = bot->GetPositionZ();

    // With failOnCollision off, a blocked run is cut short at the contact point and still returns
    // true. A false return leaves the point uncut, so it counts as no room at all.
    if (!bot->GetMap()->CanReachPositionAndGetValidCoords(bot, x, y, z, false, false))
        return 0.0f;

    return bot->GetExactDist2d(x, y);
}

// Seconds until he reaches the bot, or NEVER_CAUGHT if not before the fixate ends. He runs straight
// at wherever the bot is; the bot runs the heading, then stands. In game he has to path around
// whatever stopped the bot, so this errs toward his catching it.
float SupremusKiteBossAction::SimulateChase(
    Unit* supremus, float timeRemaining, float catchDistance, float angle, float runDistance)
{
    constexpr float timeStep = 0.25f;
    float const botStep = bot->GetSpeed(MOVE_RUN) * timeStep;
    float const supremusStep = supremus->GetSpeed(MOVE_RUN) * timeStep;
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);
    uint32 const numSteps = static_cast<uint32>(std::ceil(timeRemaining / timeStep));

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float supremusX = supremus->GetPositionX();
    float supremusY = supremus->GetPositionY();
    float runLeft = runDistance;
    for (uint32 i = 1; i <= numSteps; ++i)
    {
        float const botMove = std::min(botStep, runLeft);
        runLeft -= botMove;
        botX += botMove * dx;
        botY += botMove * dy;

        float const gap = std::hypot(botX - supremusX, botY - supremusY);
        float const supremusMove = std::min(supremusStep, gap);
        if (gap - supremusMove < catchDistance)
            return i * timeStep;

        supremusX += (botX - supremusX) / gap * supremusMove;
        supremusY += (botY - supremusY) / gap * supremusMove;
    }

    return NEVER_CAUGHT;
}

bool SupremusMoveAwayFromFireAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    std::vector<SupremusHazard> const& hazards = GetSupremusHazards(botAI);
    if (!IsInSupremusHazard(
            hazards, bot->GetPositionX(), bot->GetPositionY(), SupremusHazardZone::Safe))
    {
        return false;
    }

    // A bot walled in holds, and the fire around it changes slowly; the search runs up to 16
    // raycasts, so don't repeat it every tick unless the bot starts burning.
    constexpr uint32 retryDelayMs = 500;
    if (getMSTimeDiff(_lastFailedSearchTime, getMSTime()) < retryDelayMs &&
        !IsInSupremusHazard(
            hazards, bot->GetPositionX(), bot->GetPositionY(), SupremusHazardZone::Damage))
    {
        return false;
    }

    Position destination;
    if (!FindSafestNearbyPosition(hazards, destination))
    {
        _lastFailedSearchTime = getMSTime();
        return false;
    }

    bot->CastStop();
    return MoveTo(
        BT_MAP_ID, destination.GetPositionX(), destination.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, priority, true, false);
}

// The nearest position a yard clear of every hazard's safe zone that the bot can run to in a
// straight line. A bot not yet burning takes only a route that stays out of every damage zone, and
// otherwise holds: walled in, running through fire is worse than waiting at its edge. A burning bot
// takes the route with the least ground under fire, then the nearest.
bool SupremusMoveAwayFromFireAction::FindSafestNearbyPosition(
    std::vector<SupremusHazard> const& hazards, Position& destination)
{
    constexpr float maxRadius = 40.0f;
    // Two yards keeps the search to 320 position against a trail of 20 or more patches.
    constexpr float distanceStep = 2.0f;
    constexpr uint8 numAngles = 16;
    constexpr float angleStep = 2.0f * M_PI / numAngles;
    constexpr float clearance = 1.0f;
    constexpr uint32 numDistances = static_cast<uint32>(maxRadius / distanceStep);
    // Positions along one heading cross the same fire, and their yards differ only by rounding,
    // which picked one at random up to 40 yd out; 0.5 yd buckets keep the nearest first among them.
    constexpr float underFireBucket = 0.5f;
    constexpr uint8 maxStraightChecks = 16;

    Position const start = bot->GetPosition();
    bool const isBurning = IsInSupremusHazard(
        hazards, start.GetPositionX(), start.GetPositionY(), SupremusHazardZone::Damage);
    std::vector<std::pair<uint32, Position>> burningRoutes;
    uint8 straightChecks = 0;
    for (uint32 i = 1; i <= numDistances; ++i)
    {
        float const distance = i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * angleStep;
            float const x = start.GetPositionX() + distance * std::cos(angle);
            float const y = start.GetPositionY() + distance * std::sin(angle);
            if (IsInSupremusHazard(hazards, x, y, SupremusHazardZone::Safe, clearance))
                continue;

            Position const candidate(x, y, start.GetPositionZ());
            float const underFire = GetLineLengthInSupremusHazards(
                hazards, start, candidate, SupremusHazardZone::Damage);
            // Rings run nearest first, so the first clear route is the nearest.
            if (underFire <= 0.0f)
            {
                if (straightChecks++ >= maxStraightChecks)
                    return false;

                if (CanRunStraightTo(candidate))
                {
                    destination = candidate;
                    return true;
                }

                continue;
            }

            if (isBurning)
            {
                burningRoutes.emplace_back(
                    static_cast<uint32>(underFire / underFireBucket), candidate);
            }
        }
    }

    if (burningRoutes.empty())
        return false;

    std::stable_sort(burningRoutes.begin(), burningRoutes.end(),
        [](auto const& a, auto const& b) { return a.first < b.first; });

    for (auto const& route : burningRoutes)
    {
        if (straightChecks++ >= maxStraightChecks)
            break;

        if (CanRunStraightTo(route.second))
        {
            destination = route.second;
            return true;
        }
    }

    // Better to run the least fiery route unchecked than to stand burning.
    destination = burningRoutes.front().second;
    return true;
}

// Without this, a position behind wreckage or a wall sends MoveTo pathing the long way round.
bool SupremusMoveAwayFromFireAction::CanRunStraightTo(Position const& destination)
{
    float x = destination.GetPositionX();
    float y = destination.GetPositionY();
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = bot->GetPositionZ();

    if (!bot->GetMap()->CanReachPositionAndGetValidCoords(bot, x, y, z, true, false))
        return false;

    // An incomplete path can cut the point short without counting as a collision.
    constexpr float truncationTolerance = 1.0f;
    return std::hypot(x - destination.GetPositionX(), y - destination.GetPositionY()) <=
        truncationTolerance;
}

// Steps toward the target, turning as little as it must to keep each step clear of fire, which
// slides the bot round a volcano or a trail in its way. With nothing clear it holds.
bool SupremusReachAroundFireAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Unit* target;
    float range;
    if (!GetSupremusReachBlockedByFire(botAI, target, range))
        return false;

    std::vector<SupremusHazard> const& hazards = GetSupremusHazards(botAI);
    float const aim = bot->GetAngle(target);
    constexpr float turnStep = static_cast<float>(M_PI) / 16.0f;

    auto const tryStep = [&](uint8 turns, int8 side)
    {
        // Between two clear ends, a 3.5 yd step cuts at most 0.25 yd into a safe zone, still well
        // short of the damage zone (0.75 yd for a patch, 2.75 for a volcano).
        float const angle = aim + side * turns * turnStep;
        float const x = bot->GetPositionX() + SUPREMUS_REACH_STEP_DISTANCE * std::cos(angle);
        float const y = bot->GetPositionY() + SUPREMUS_REACH_STEP_DISTANCE * std::sin(angle);
        if (IsInSupremusHazard(hazards, x, y, SupremusHazardZone::Safe))
            return false;

        float stepX;
        float stepY;
        float stepZ;
        if (!CanTakeStepTowards(bot, x, y, SUPREMUS_REACH_STEP_DISTANCE, stepX, stepY, stepZ))
            return false;

        // Only a turned step commits to a side; walking straight leaves the next turn free.
        if (turns > 0)
        {
            _lastSide = side;
            _lastStepTime = getMSTime();
        }

        return MoveTo(
            BT_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
    };

    constexpr uint8 turnsToSquare = 8;
    constexpr uint8 turnsToBehind = 16;

    // Once sliding one way it keeps to that side, turning back if it must, which follows a
    // volcano's edge out of the notch where two meet; trying the other side first makes it dither.
    constexpr uint32 sideMemoryMs = 2000;
    if (getMSTimeDiff(_lastStepTime, getMSTime()) < sideMemoryMs)
    {
        int8 const side = _lastSide;
        for (uint8 i = 0; i <= turnsToBehind; ++i)
        {
            if (tryStep(i, side))
                return true;
        }

        for (uint8 i = 1; i <= turnsToSquare; ++i)
        {
            if (tryStep(i, static_cast<int8>(-side)))
                return true;
        }

        return false;
    }

    if (tryStep(0, 1))
        return true;

    for (uint8 i = 1; i <= turnsToSquare; ++i)
    {
        if (tryStep(i, 1) || tryStep(i, -1))
            return true;
    }

    return false;
}

// Shade of Akama

// Channelers stand on a platform out of sight from below, and Attack refuses a target out of
// sight, so a bot walks the path toward its target until it can see it.
bool ShadeOfAkamaMeleeDpsPrioritizeChannelersAction::Execute(Event /*event*/)
{
    Unit* target = GetShadeOfAkamaKillTarget(botAI);
    if (!target)
        return false;

    if (MarkTargetWithSkull(bot, target))
        return true;

    if (!bot->IsWithinLOSInMap(target))
    {
        constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
        if (IsWaitingForLastMove(priority))
            return false;

        constexpr float stopDistance = 5.0f;
        float stepX;
        float stepY;
        if (!GetPathStepTowardUnit(bot, target, stopDistance, stepX, stepY))
            return false;

        return MoveTo(
            BT_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority,
            true, false);
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

// Teron Gorefiend

// Assume positions in arc at the edge of the balcony (farthest from Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BT_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t const count = rangedMembers.size();
    auto const findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t const botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = 2.0f * M_PI / 5.0f;
    constexpr float arcCenter = 6.279f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 12.0f;
    float const angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float const targetX = GOREFIEND_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float const targetY = GOREFIEND_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) <= GOREFIEND_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        return botAI->CanCastSpell(Id(BtSpells::SPELL_FEIGN_DEATH), bot) &&
            botAI->CastSpell(Id(BtSpells::SPELL_FEIGN_DEATH), bot);
    }

    return botAI->CanCastSpell("vanish", bot) && botAI->CastSpell("vanish", bot);
}

bool TeronGorefiendMoveToCornerToDieAction::Execute(Event /*event*/)
{
    Position const& position = GOREFIEND_DIE_POSITION;
    if (bot->GetExactDist2d(position) <= GOREFIEND_POSITION_TOLERANCE)
        return false;

    bot->CastStop();
    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Unit* spirit = bot->GetCharm();
    if (!spirit)
        return false;

    std::vector<Unit*> const constructs = GetConstructs(gorefiend);
    Unit* leadConstruct = GetLeadConstruct(gorefiend, constructs);
    if (!leadConstruct)
    {
        Unit* victim = gorefiend->GetVictim();
        if (victim && CastVengefulSpiritSpell(spirit, victim, Id(BtSpells::SPELL_SPIRIT_SHIELD)))
            return true;

        bool moving = false;
        if (!spirit->IsWithinMeleeRange(gorefiend))
        {
            float const distance = spirit->GetExactDist2d(gorefiend);
            float const moveDistance = distance - gorefiend->GetCombatReach();
            float const dX = gorefiend->GetPositionX() - spirit->GetPositionX();
            float const dY = gorefiend->GetPositionY() - spirit->GetPositionY();

            spirit->GetMotionMaster()->MovePoint(
                0, spirit->GetPositionX() + dX / distance * moveDistance,
                spirit->GetPositionY() + dY / distance * moveDistance, spirit->GetPositionZ());

            moving = true;
        }

        return CastVengefulSpiritSpell(
            spirit, gorefiend, Id(BtSpells::SPELL_SPIRIT_STRIKE)) || moving;
    }

    bool moving = false;
    float const distanceToLead = spirit->GetExactDist2d(leadConstruct);
    if (distanceToLead > GOREFIEND_SPIRIT_AOE_DISTANCE)
    {
        float const moveDistance = distanceToLead - GOREFIEND_SPIRIT_AOE_DISTANCE + 2.0f;
        float const dX = leadConstruct->GetPositionX() - spirit->GetPositionX();
        float const dY = leadConstruct->GetPositionY() - spirit->GetPositionY();

        spirit->GetMotionMaster()->MovePoint(
            0, spirit->GetPositionX() + dX / distanceToLead * moveDistance,
            spirit->GetPositionY() + dY / distanceToLead * moveDistance, spirit->GetPositionZ());

        moving = true;
    }
    else if (CastVengefulSpiritSpell(spirit, leadConstruct, Id(BtSpells::SPELL_SPIRIT_VOLLEY)) ||
        CastVengefulSpiritSpell(spirit, leadConstruct, Id(BtSpells::SPELL_SPIRIT_CHAINS)))
    {
        return true;
    }

    return CastVengefulSpiritSpell(
        spirit, GetLanceTarget(gorefiend, constructs), Id(BtSpells::SPELL_SPIRIT_LANCE)) || moving;
}

// A grid search from Teron, not the bot's own targets: the bot is the possessing body, left
// where Shadow of Death sent it, and its target lists keep only units it can see.
std::vector<Unit*> TeronGorefiendControlAndDestroyShadowyConstructsAction::GetConstructs(
    Unit* gorefiend) const
{
    std::list<Creature*> creatures;
    gorefiend->GetCreatureListWithEntryInGrid(
        creatures, Id(BtNpcs::NPC_SHADOWY_CONSTRUCT), GOREFIEND_CONSTRUCT_SEARCH_RADIUS);

    std::vector<Unit*> constructs;
    for (Creature* creature : creatures)
    {
        if (creature->IsAlive())
            constructs.push_back(creature);
    }

    return constructs;
}

// The construct nearest Teron leads the way to the raid.
Unit* TeronGorefiendControlAndDestroyShadowyConstructsAction::GetLeadConstruct(
    Unit* gorefiend, std::vector<Unit*> const& constructs) const
{
    Unit* leadConstruct = nullptr;
    float leadDistance = 0.0f;
    for (Unit* construct : constructs)
    {
        float const distance = gorefiend->GetExactDist2d(construct);
        if (!leadConstruct || distance < leadDistance)
        {
            leadConstruct = construct;
            leadDistance = distance;
        }
    }

    return leadConstruct;
}

// Lance breaks Chains, so it frees the chained construct farthest from Teron first, keeping the
// rest together. With none chained, Lance goes round them, keeping its 9 s slow on each and
// wearing them down evenly: of those within one Lance of the highest health, the nearest Teron.
Unit* TeronGorefiendControlAndDestroyShadowyConstructsAction::GetLanceTarget(
    Unit* gorefiend, std::vector<Unit*> const& constructs) const
{
    Unit* lanceTarget = nullptr;
    float lanceDistance = 0.0f;
    uint32 highestHealth = 0;
    for (Unit* construct : constructs)
    {
        highestHealth = std::max(highestHealth, construct->GetHealth());
        if (!construct->HasAura(Id(BtSpells::SPELL_SPIRIT_CHAINS)))
            continue;

        float const distance = gorefiend->GetExactDist2d(construct);
        if (!lanceTarget || distance > lanceDistance)
        {
            lanceTarget = construct;
            lanceDistance = distance;
        }
    }

    if (lanceTarget)
        return lanceTarget;

    for (Unit* construct : constructs)
    {
        if (construct->GetHealth() + GOREFIEND_SPIRIT_LANCE_MIN_DAMAGE <= highestHealth)
            continue;

        float const distance = gorefiend->GetExactDist2d(construct);
        if (!lanceTarget || distance < lanceDistance)
        {
            lanceTarget = construct;
            lanceDistance = distance;
        }
    }

    return lanceTarget;
}

// Gurtogg Bloodboil

bool GurtoggBloodboilRotateRangedGroupsAction::Execute(Event /*event*/)
{
    Position const& centre = GetGurtoggBloodboilPosition(bot);
    if (IsInRectangle(
            bot->GetPosition(), centre, GURTOGG_TANK_POSITION, GURTOGG_RANGED_HALF_WIDTH,
            GURTOGG_RANGED_HALF_DEPTH))
    {
        return false;
    }

    Position const point = GetBotPointInRectangle(
        bot, centre, GURTOGG_TANK_POSITION, GURTOGG_RANGED_HALF_WIDTH, GURTOGG_RANGED_HALF_DEPTH);

    bot->CastStop();
    return MoveTo(
        BT_MAP_ID, point.GetPositionX(), point.GetPositionY(), point.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED);
}

bool GurtoggBloodboilMeleeMoveBehindAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    Position const position = GetGurtoggFelRageMeleePosition(bot, *gurtogg, *felRageTarget);
    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED);
}

// Steps sideways off his line to the Fel Rage target, the axis of Arcing Smash, so a bot ahead
// of a walking target isn't pushed along in front of it.
bool GurtoggBloodboilRangedMoveOutOfFrontalConeAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    float const axisAngle = gurtogg->GetAngle(felRageTarget);
    float const dX = bot->GetPositionX() - felRageTarget->GetPositionX();
    float const dY = bot->GetPositionY() - felRageTarget->GetPositionY();
    float const along = dX * std::cos(axisAngle) + dY * std::sin(axisAngle);
    float const across = -dX * std::sin(axisAngle) + dY * std::cos(axisAngle);

    float const distance = GURTOGG_FEL_RAGE_AVOID_DISTANCE + GURTOGG_FEL_RAGE_AVOID_MARGIN;
    float const side = across >= 0.0f ? 1.0f : -1.0f;
    float const targetAcross =
        side * std::sqrt(std::max(distance * distance - along * along, 0.0f));
    float const step = targetAcross - across;

    bot->CastStop();
    return MoveTo(
        BT_MAP_ID, bot->GetPositionX() - step * std::sin(axisAngle),
        bot->GetPositionY() + step * std::cos(axisAngle), bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED);
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (!desire && !anger)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    Unit* target = desire ? desire : anger;

    return target->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT &&
        MisdirectTargetToTank(botAI, target, mainTank);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    float const distance = bot->GetExactDist2d(suffering);
    if (IsSufferingFixateTank(bot))
    {
        return distance > SUFFERING_TANK_MAX_DISTANCE &&
            MoveToDistanceFromSuffering(suffering, SUFFERING_TANK_DISTANCE);
    }

    if (PlayerbotAI::IsMelee(bot))
    {
        return distance < SUFFERING_MELEE_MIN_DISTANCE &&
            MoveToDistanceFromSuffering(suffering, SUFFERING_MELEE_DISTANCE);
    }

    return RangedMoveAwayFromBoss(suffering);
}

// Straight in or out along her line to the bot, so nobody crosses nearer to her than the tanks.
bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::MoveToDistanceFromSuffering(
    Unit* suffering, float distance)
{
    float const angle = suffering->GetAngle(bot);
    float const x = suffering->GetPositionX() + distance * std::cos(angle);
    float const y = suffering->GetPositionY() + distance * std::sin(angle);

    return MoveTo(
        BT_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::RangedMoveAwayFromBoss(Unit* suffering)
{
    if (bot->GetExactDist2d(suffering) >= SUFFERING_RANGED_DISTANCE)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(suffering->GetPosition(), SUFFERING_RANGED_DISTANCE, minInterval);
}

bool ReliquaryOfSoulsHealersDpsSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            if (bot->HasAura(Id(BtSpells::SPELL_TREE_OF_LIFE)))
            {
                bot->RemoveOwnedAura(
                    Id(BtSpells::SPELL_TREE_OF_LIFE), ObjectGuid::Empty, 0,
                    AURA_REMOVE_BY_CANCEL);
            }

            if (botAI->CanCastSpell("barkskin", bot) && botAI->CastSpell("barkskin", bot))
                return true;

            return botAI->CanCastSpell("wrath", suffering) && botAI->CastSpell("wrath", suffering);
        }
        case CLASS_PALADIN:
        {
            if (botAI->CanCastSpell("avenging wrath", bot) &&
                botAI->CastSpell("avenging wrath", bot))
            {
                return true;
            }

            for (char const* spell :
                 { "exorcism", "hammer of wrath", "holy shock", "judgement of light" })
            {
                if (botAI->CanCastSpell(spell, suffering) && botAI->CastSpell(spell, suffering))
                    return true;
            }

            return false;
        }
        case CLASS_PRIEST:
            return botAI->CanCastSpell("smite", suffering) && botAI->CastSpell("smite", suffering);
        case CLASS_SHAMAN:
        {
            for (char const* spell : { "earth shock", "chain lightning", "lightning bolt" })
            {
                if (botAI->CanCastSpell(spell, suffering) && botAI->CastSpell(spell, suffering))
                    return true;
            }

            return false;
        }
        default:
            return false;
    }
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire)
        return false;

    if (!botAI->CanCastSpell(Id(BtSpells::SPELL_SPELLSTEAL), desire))
        return false;

    return botAI->CastSpell(Id(BtSpells::SPELL_SPELLSTEAL), desire);
}

// Mother Shahraz

bool MotherShahrazTanksPositionBossUnderPillarAction::Execute(Event /*event*/)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    if (AI_VALUE(Unit*, "current target") != shahraz)
        return Attack(shahraz);

    Unit* victim = shahraz->GetVictim();
    if (!victim)
        return false;

    // Saber Lash is split between her victim and the two closest to it.
    if (victim != bot)
    {
        if (bot->GetExactDist2d(victim) <= SHAHRAZ_OFF_TANK_DISTANCE)
            return false;

        return MoveTo(
            BT_MAP_ID, victim->GetPositionX(), victim->GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    Position const& tankPosition = SHAHRAZ_TANK_POSITION;
    Position const& transitionPosition = SHAHRAZ_TRANSITION_POSITION;

    float const distToTankPosition = bot->GetExactDist2d(tankPosition);
    if (distToTankPosition <= SHAHRAZ_TANK_POSITION_TOLERANCE)
    {
        if (!bot->HasInArc(static_cast<float>(M_PI) / 2.0f, shahraz))
            bot->SetFacingTo(bot->GetAngle(shahraz));

        return false;
    }

    if (!bot->IsWithinMeleeRange(shahraz))
        return false;

    // Within this distance of the tank position, the bot is past the statue.
    float const finalLegDistance =
        transitionPosition.GetExactDist2d(tankPosition) + SHAHRAZ_TANK_POSITION_TOLERANCE;
    Position const& position = distToTankPosition <= finalLegDistance ?
        tankPosition : transitionPosition;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool MotherShahrazMeleeDpsWaitAtSafePositionAction::Execute(Event /*event*/)
{
    Position const& position = SHAHRAZ_RANGED_POSITION;
    if (bot->GetExactDist2d(position) <= SHAHRAZ_RANGED_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// This doesn't matter for bots since they don't take fall damage, and it's actually easier
// to tank her closer to her starting position, but I want to simulate a player strategy
bool MotherShahrazPositionRangedUnderPillarAction::Execute(Event /*event*/)
{
    Position const& position = SHAHRAZ_RANGED_POSITION;
    if (bot->GetExactDist2d(position) <= SHAHRAZ_RANGED_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MotherShahrazBreakFatalAttractionAction::Execute(Event /*event*/)
{
    std::vector<Player*> const attractedPlayers = GetAttractedPlayers();
    if (attractedPlayers.size() < 2)
        return false;

    auto const botIt = std::find(attractedPlayers.begin(), attractedPlayers.end(), bot);
    if (botIt == attractedPlayers.end())
        return false;

    // Holders run away from the ranged position, in GUID order: with two, one to each side; with
    // three, the first straight away and the other two to each side.
    auto const index = std::distance(attractedPlayers.begin(), botIt);
    constexpr float quarterTurn = static_cast<float>(M_PI) / 2.0f;
    float spreadAngle = SHAHRAZ_RANGED_POSITION.GetAngle(bot);
    if (attractedPlayers.size() == 2)
        spreadAngle += index ? -quarterTurn : quarterTurn;
    else if (index)
        spreadAngle += index == 1 ? quarterTurn : -quarterTurn;

    // Straight on first, then alternately left and right out to 90 degrees, so a bot against a
    // wall slides along it.
    constexpr uint8 numCandidates = 9;
    constexpr float angleStep = static_cast<float>(M_PI) / 8.0f;
    for (uint8 i = 0; i < numCandidates; ++i)
    {
        int8 const side = (i % 2) ? 1 : -1;
        float const angle = spreadAngle + side * ((i + 1) / 2) * angleStep;
        float const x = bot->GetPositionX() +
            std::cos(angle) * SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE;
        float const y = bot->GetPositionY() +
            std::sin(angle) * SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE;

        float stepX;
        float stepY;
        float stepZ;
        if (CanTakeStepTowards(
                bot, x, y, SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE, stepX, stepY, stepZ))
        {
            return MoveTo(
                BT_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

std::vector<Player*> MotherShahrazBreakFatalAttractionAction::GetAttractedPlayers()
{
    std::vector<Player*> attractedPlayers;
    Group* group = bot->GetGroup();
    if (!group)
        return attractedPlayers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION)))
            attractedPlayers.push_back(member);
    }

    std::sort(attractedPlayers.begin(), attractedPlayers.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    return attractedPlayers;
}

// Illidari Council

bool IllidariCouncilMisdirectToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* councilTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0 || hunterIndex >= 4)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
        tankTarget = GetZerevorMageTank(botAI);
    }
    else if (hunterIndex == 1)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "lady malande");
        tankTarget = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 2)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
        tankTarget = GetGroupMainTank(bot);
    }
    else if (hunterIndex == 3)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "veras darkshadow");
        tankTarget = GetGroupAssistTank(bot, 1);
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, councilTarget, tankTarget);
}

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    if (AI_VALUE(Unit*, "current target") != gathios)
        return Attack(gathios);

    if (gathios->GetVictim() != bot || !bot->IsWithinMeleeRange(gathios))
        return false;

    constexpr float arrivalDist = 2.0f;
    if (bot->GetExactDist2d(GATHIOS_TANK_POSITIONS[_tankStep]) <= arrivalDist &&
        HasDangerousCouncilAura(bot))
    {
        _tankStep = (_tankStep + 1) % GATHIOS_TANK_POSITIONS.size();
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GATHIOS_TANK_POSITIONS[_tankStep], arrivalDist, gathios, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool IllidariCouncilInterruptCircleOfHealingAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    // CanCastSpell passes a target out of range, and the failed cast still turns the bot to her.
    auto const castInterrupt = [&](char const* spell, bool inRange)
    {
        return inRange && botAI->CanCastSpell(spell, malande) && botAI->CastSpell(spell, malande);
    };

    bool const inMeleeRange = bot->IsWithinMeleeRange(malande);
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return castInterrupt("mind freeze", inMeleeRange);
        case CLASS_MAGE:
            return castInterrupt(
                "counterspell", bot->IsWithinCombatRange(malande, COUNTERSPELL_RANGE));
        case CLASS_ROGUE:
            return castInterrupt("kick", inMeleeRange);
        case CLASS_SHAMAN:
            return castInterrupt(
                "wind shear", bot->IsWithinCombatRange(malande, WIND_SHEAR_RANGE));
        case CLASS_WARLOCK:
        {
            Pet* pet = bot->GetPet();
            uint32 const spellLock = GetReadySpellLock(pet, malande);
            return spellLock && pet->CastSpell(malande, spellLock, false) == SPELL_CAST_OK;
        }
        case CLASS_WARRIOR:
            return castInterrupt("pummel", inMeleeRange) ||
                castInterrupt("shield bash", inMeleeRange);
        default:
            return false;
    }
}

bool IllidariCouncilFirstAssistTankFocusMalandeAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    if (AI_VALUE(Unit*, "current target") != malande)
        return Attack(malande);

    // Leaves a council patch only for a position from which it can still hit her; otherwise stay.
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (!HasDangerousCouncilAura(bot) || !bot->IsWithinMeleeRange(malande) ||
        IsWaitingForLastMove(priority))
    {
        return false;
    }

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!gathios || !zerevor)
        return false;

    if (!IsInCouncilPatch(bot->GetPosition(), gathios, zerevor, COUNCIL_PATCH_MARGIN))
        return false;

    Position destination;
    if (!FindMalandeTankPosition(bot, malande, gathios, zerevor, destination))
        return false;

    return MoveTo(
        BT_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
        destination.GetPositionZ(), false, false, false, false, priority, true, false);
}

bool IllidariCouncilSecondAssistTankPositionVerasAction::Execute(Event /*event*/)
{
    Unit* veras = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (!veras)
        return false;

    if (AI_VALUE(Unit*, "current target") != veras)
        return Attack(veras);

    if (veras->GetVictim() != bot)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, mainTank->GetPosition(), arrivalDist, veras, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event /*event*/)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return false;

    if (bot->HasAura(Id(BtSpells::SPELL_VIGILANCE)))
    {
        bot->RemoveOwnedAura(
            Id(BtSpells::SPELL_VIGILANCE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    if (zerevor->HasAura(Id(BtSpells::SPELL_DAMPEN_MAGIC)) &&
        botAI->CanCastSpell("spellsteal", zerevor))
    {
        return botAI->CastSpell("spellsteal", zerevor);
    }

    if (AI_VALUE(Unit*, "current target") != zerevor)
        return Attack(zerevor);

    if (zerevor->GetVictim() != bot)
        return false;

    constexpr float arrivalDist = 2.0f;
    if (bot->GetExactDist2d(ZEREVOR_TANK_POSITIONS[_tankStep]) <= arrivalDist &&
        HasDangerousCouncilAura(bot))
    {
        _tankStep = (_tankStep + 1) % ZEREVOR_TANK_POSITIONS.size();
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, ZEREVOR_TANK_POSITIONS[_tankStep], arrivalDist, zerevor, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidariCouncilPositionMageTankHealerAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    Player* mageTank = GetZerevorMageTank(botAI);
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!mageTank || !zerevor)
        return false;

    Position destination;
    if (FindMageTankHealerPosition(bot, mageTank, zerevor, destination))
    {
        return MoveTo(
            BT_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
            destination.GetPositionZ(), false, false, false, false, priority, true, false);
    }

    float stepX;
    float stepY;
    if (!GetPathStepTowardUnit(bot, mageTank, MAGE_TANK_HEALER_MIN_DISTANCE, stepX, stepY))
        return false;

    return MoveTo(
        BT_MAP_ID, stepX, stepY, bot->GetPositionZ(), false, false, false, false, priority, true,
        false);
}

bool IllidariCouncilCommandPetTargetAction::Execute(Event /*event*/)
{
    Guardian* pet = bot->GetGuardianPet();
    Unit* target = GetCouncilPetTarget(botAI, pet);
    if (!target || !pet->IsValidAttackTarget(target))
        return false;

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
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

// Rogues and warriors take Malande for interrupts, leaving her under Blessing of Protection; death
// knights and Enhancement shamans stay, since Mind Freeze and Wind Shear still land. All of them
// leave her while she stands in a Zerevor patch or Gathios's Consecration. Casters take Gathios,
// and the mage tank's healer Zerevor. Everyone skips a member immune to their damage.
bool IllidariCouncilAssignDpsTargetsAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    Unit* veras = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (IsVerasVanished(veras))
        veras = nullptr;

    bool const isCaster = PlayerbotAI::IsCaster(bot);
    uint32 const immunity = isCaster ? Id(BtSpells::SPELL_BLESSING_OF_SPELL_WARDING) :
        Id(BtSpells::SPELL_BLESSING_OF_PROTECTION);

    Unit* target = nullptr;
    if (zerevor && PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
    {
        target = zerevor;
    }
    else if (isCaster)
    {
        if (gathios && !gathios->HasAura(immunity))
            target = gathios;
        else if (veras && !veras->HasAura(immunity))
            target = veras;
        else if (malande && !malande->HasAura(immunity))
            target = malande;
    }
    else if (ShouldAttackMalande(malande, gathios, zerevor))
    {
        target = malande;
    }
    else if (veras && !veras->HasAura(immunity))
    {
        target = veras;
    }
    else if (gathios && !gathios->HasAura(immunity))
    {
        target = gathios;
    }

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool IllidariCouncilAssignDpsTargetsAction::ShouldAttackMalande(
    Unit* malande, Unit* gathios, Unit* zerevor) const
{
    constexpr float zerevorClearance = 15.0f;
    if (!malande || (zerevor && zerevor->GetExactDist2d(malande) < zerevorClearance) ||
        IsMalandeInCouncilPatch(malande, gathios, zerevor))
    {
        return false;
    }

    switch (bot->getClass())
    {
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return !malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION));
        case CLASS_DEATH_KNIGHT:
        case CLASS_SHAMAN:
            return true;
        default:
            return false;
    }
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    return EmplaceIfUnset(
        BtState(gathios->GetMap()->GetInstanceId()).councilDpsWaitTimer, getMSTime());
}

// Illidan Stormrage <The Betrayer>

// The Main Tank waits 15.5 yd north of the grate centre, facing him, so he stops on the ring when
// he runs in. Everyone else waits on the grate.
bool IllidanStormragePositionForPullAction::Execute(Event /*event*/)
{
    bool const isMainTank = PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
    Position position = ILLIDAN_GRATE_CENTRE;
    if (isMainTank)
        position = ILLIDAN_PULL_TANK_POSITION;

    if (bot->GetExactDist2d(position) <= ILLIDAN_POSITION_TOLERANCE)
    {
        Creature* illidan = isMainTank ? bot->FindNearestCreature(
            Id(BtNpcs::NPC_ILLIDAN_STORMRAGE), ILLIDAN_SPEECH_SEARCH_RADIUS) : nullptr;
        if (illidan && !bot->HasInArc(static_cast<float>(M_PI) / 2.0f, illidan))
            bot->SetFacingTo(bot->GetAngle(illidan));

        return false;
    }

    return MoveInside(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        ILLIDAN_POSITION_TOLERANCE, MovementPriority::MOVEMENT_COMBAT);
}

bool IllidanStormrageMisdirectToTanksAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);

    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev)
    {
        Player* mainTank = GetGroupMainTank(bot);
        return mainTank && mainTank->IsAlive() &&
            MisdirectTargetToTank(botAI, illidan, mainTank);
    }

    if (phase == IllidanPhase::Flying && TryMisdirectToFlameTanks(group))
        return true;

    return phase == IllidanPhase::Demon && TryMisdirectToWarlockTank(illidan);
}

bool IllidanStormrageMisdirectToTanksAction::TryMisdirectToFlameTanks(Group* group)
{
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    // The first hunter takes the East Flame to its tank, the others the West Flame, each as soon
    // as its own Flame is up, since the West one comes about 0.7 s first.
    bool const isEast = hunterIndex == 0;
    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    Unit* flame = isEast ? eastFlame : westFlame;
    if (!flame || flame->GetHealthPct() <= BOSS_ENGAGED_HEALTH_PCT)
        return false;

    return MisdirectTargetToTank(botAI, flame, GetGroupAssistTank(bot, isEast ? 1 : 0));
}

// Its Steady Shot would be damage on him while nobody may damage him.
bool IllidanStormrageMisdirectToTanksAction::TryMisdirectToWarlockTank(Unit* illidan)
{
    if (!illidan)
        return false;

    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    if (window == IllidanDpsHold::DemonFormEnd || window == IllidanDpsHold::DemonFormNoPush)
        return false;

    return MisdirectTargetToTank(botAI, illidan, GetIllidanWarlockTank(botAI));
}

// The Main Tank holds him about 25 yd from the grate centre, 6 yd outward of him so he faces out.
// On each Flame Crash on the tank it leads him to the ring point 35 deg counterclockwise of him,
// once more if he stops under 25 deg on; he stops about 30 deg on, and the tank settles outward of
// wherever he stopped. Near the centre, as after landing, it leads him straight out instead.
// While he lands the tank waits between the landing point and the raid's stack, then leads him
// north, so Phase 3 starts due north. In Demon Form it waits beside the melee, then 10 s before
// the end walks to the pickup position in front of the raid to take him at the reset. In Maiev's
// arrival it stands just inward of him once out of Shadow Prison; he wakes with no threat reset,
// and the settle then takes it round him to outward.
bool IllidanStormrageMainTankRepositionBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase != IllidanPhase::Landing && phase != IllidanPhase::Demon &&
        phase != IllidanPhase::None && AI_VALUE(Unit*, "current target") != illidan)
    {
        return Attack(illidan);
    }

    BtInstanceState const* state = FindBtState(illidan->GetInstanceId());
    uint32 const phaseStartTime = state ? state->illidanPhaseStartTime.value_or(0) : 0;
    if (phaseStartTime != _phaseStartTime)
    {
        _phaseStartTime = phaseStartTime;
        _lead = IllidanTankLead::None;
    }

    Position position;
    if (phase == IllidanPhase::Landing)
    {
        position = ILLIDAN_LANDING_TANK_POSITION;
    }
    else if (phase == IllidanPhase::None)
    {
        float const radius = std::max(
            0.0f,
            illidan->GetExactDist2d(ILLIDAN_GRATE_CENTRE) - ILLIDAN_MAIEV_SCENE_TANK_OFFSET);
        position = GetRingPoint(GetGrateAngle(*illidan), radius);
    }
    else if (phase == IllidanPhase::Demon)
    {
        if (!FindDemonFormPosition(*illidan, position))
            return false;
    }
    // The tank's own logic takes him back; the ring movement is only for when he is on it.
    else if (illidan->GetVictim() != bot || !FindRingPosition(*illidan, phase, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position) <= ILLIDAN_POSITION_TOLERANCE)
    {
        if (!bot->HasInArc(static_cast<float>(M_PI) / 2.0f, illidan))
            bot->SetFacingTo(bot->GetAngle(illidan));

        return false;
    }

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// It joins the melee on the adds until the walk-in, then drops its target, so its rotation keeps
// its cooldowns for the reset.
bool IllidanStormrageMainTankRepositionBossAction::FindDemonFormPosition(
    Unit const& illidan, Position& position)
{
    bool const isWalkIn = IsIllidanDemonFormEnding(illidan, ILLIDAN_MAIN_TANK_WALK_IN_MS);
    Unit* target = AI_VALUE(Unit*, "current target");
    uint32 const targetEntry = target && target->IsAlive() ? target->GetEntry() : 0;
    bool const isOnAdd = targetEntry == Id(BtNpcs::NPC_SHADOW_DEMON) ||
        targetEntry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND);
    if (isWalkIn && (target || bot->GetVictim()))
    {
        bot->AttackStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
    }
    else if (!isWalkIn && isOnAdd)
    {
        return false;
    }

    DemonFormLayout const layout = GetDemonFormLayout(illidan);
    if (IsIllidanDemonFormEnding(illidan, ILLIDAN_MAIN_TANK_STEP_IN_MS))
        position = layout.mainTankStepInPosition;
    else if (isWalkIn)
        position = layout.mainTankPickupPosition;
    else
        position = layout.mainTankWaitPosition;

    return true;
}

bool IllidanStormrageMainTankRepositionBossAction::FindRingPosition(
    Unit const& illidan, IllidanPhase phase, Position& position)
{
    StartRingLead(illidan, phase);

    // Where he stops depends on when the tank stops, so it isn't redirected while he moves.
    if (illidan.isMoving())
        return false;

    bool const isLeading = _lead != IllidanTankLead::None;
    // How far round the ring he stopped short of the lead position; an overshoot reads ~360.
    float const shortfall = _lead == IllidanTankLead::Step ?
        Position::NormalizeOrientation(GetGrateAngle(_leadPosition) - GetGrateAngle(illidan)) :
        0.0f;
    if (isLeading && bot->GetExactDist2d(_leadPosition) > ILLIDAN_POSITION_TOLERANCE)
    {
        position = _leadPosition;
    }
    else if (isLeading && illidan.IsNonMeleeSpellCast(false))
    {
        // A cast (Draw Soul often follows a Flame Crash) holds him in place; he follows the tank
        // to the lead point once it ends, so the tank waits there for him.
        return false;
    }
    else if (shortfall > ILLIDAN_RING_LEAD_ANGLE - ILLIDAN_RING_SHORT_STEP_ANGLE &&
        shortfall < static_cast<float>(M_PI))
    {
        _leadPosition = GetRingPoint(
            GetGrateAngle(illidan) + ILLIDAN_RING_LEAD_ANGLE, ILLIDAN_RING_RADIUS);
        _lead = IllidanTankLead::StepAgain;
        position = _leadPosition;
    }
    else
    {
        _lead = IllidanTankLead::None;
        position = GetIllidanSettlePosition(illidan);
    }

    return true;
}

void IllidanStormrageMainTankRepositionBossAction::StartRingLead(
    Unit const& illidan, IllidanPhase phase)
{
    // The newest one (GUIDs only grow), so a Flame Crash landing on an older one still counts.
    std::list<Creature*> flameCrashes;
    bot->GetCreatureListWithEntryInGrid(
        flameCrashes, Id(BtNpcs::NPC_FLAME_CRASH), ILLIDAN_FLAME_CRASH_ON_TANK_DISTANCE);
    Creature* flameCrash = nullptr;
    for (Creature* creature : flameCrashes)
    {
        if (creature->IsAlive() &&
            bot->GetExactDist2d(creature) <= ILLIDAN_FLAME_CRASH_ON_TANK_DISTANCE &&
            (!flameCrash || flameCrash->GetGUID() < creature->GetGUID()))
        {
            flameCrash = creature;
        }
    }

    if (flameCrash && _lastFlameCrash < flameCrash->GetGUID())
    {
        _lastFlameCrash = flameCrash->GetGUID();
        _leadPosition = GetRingPoint(
            GetGrateAngle(illidan) + ILLIDAN_RING_LEAD_ANGLE, ILLIDAN_RING_RADIUS);
        _lead = IllidanTankLead::Step;
    }
    else if (_lead == IllidanTankLead::None &&
        illidan.GetExactDist2d(ILLIDAN_GRATE_CENTRE) < ILLIDAN_PICKUP_DISTANCE)
    {
        // After landing the tank is on the raid's side of him, so it leads him north, away from
        // them.
        bool const isAfterLanding = GetIllidanPhaseBefore(illidan, phase) == IllidanPhase::Landing;
        float const leadAngle = isAfterLanding ? ILLIDAN_LANDING_LEAD_ANGLE : GetGrateAngle(*bot);
        _leadPosition = GetRingPoint(leadAngle, ILLIDAN_PICKUP_LEAD_RADIUS);
        _lead = IllidanTankLead::Pickup;
    }
}

// A trap hunter on duty drops Frost Trap on arrival. An infected hunter is never on duty.
bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Position const position =
        GetIllidanRecordedInfectionPosition(botAI, *illidan, GetIllidanPhase(illidan));
    if (bot->GetExactDist2d(position) > ILLIDAN_POSITION_TOLERANCE)
    {
        return MoveTo(
            BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (!IsIllidanTrapHunterOnDuty(botAI))
        return false;

    uint32 const frostTrap = Id(BtSpells::SPELL_FROST_TRAP);
    return botAI->CanCastSpell(frostTrap, bot) && botAI->CastSpell(frostTrap, bot);
}

// The run position is fixed when an infection is recorded, so it doesn't move as he does, and the
// trap hunter once picked, so a Frost Trap coming off cooldown mid-run can't pass the job on. A
// change of run-position phase records both again.
bool IllidanStormrageTrackInfectionsAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    auto const& infected = context->GetValue<GuidVector>(
        "illidan stormrage players with parasitic shadowfiend")->RefGet();
    IllidanPhase const phase = GetIllidanPhase(illidan);
    IllidanPhase const infectionPhase = GetIllidanInfectionPositionPhase(*illidan, phase);
    bool const hasInfectionPosition = phase != IllidanPhase::None && phase != IllidanPhase::Flying;
    std::optional<std::unordered_map<ObjectGuid, IllidanInfection>>& infectionsField =
        BtState(bot->GetInstanceId()).illidanInfections;

    auto& infections = infectionsField ? *infectionsField : infectionsField.emplace();
    std::erase_if(
        infections,
        [&infected, infectionPhase, hasInfectionPosition](auto const& entry)
        {
            return std::find(infected.begin(), infected.end(), entry.first) == infected.end() ||
                (hasInfectionPosition && entry.second.phase != infectionPhase);
        });

    if (hasInfectionPosition)
    {
        for (ObjectGuid const& guid : infected)
        {
            if (infections.contains(guid))
                continue;

            // A new infection shares the position already in use, so one trap does for both.
            IllidanInfection infection;
            infection.phase = infectionPhase;
            infection.infectionPosition = infections.empty() ?
                GetIllidanInfectionPosition(*illidan, infectionPhase) :
                infections.begin()->second.infectionPosition;
            infections.emplace(guid, infection);
        }

        for (auto& [guid, infection] : infections)
        {
            Player* player = ObjectAccessor::FindPlayer(guid);
            if (!infection.isTrapAssigned && player &&
                FindTrapHunter(*player, infection.infectionPosition, infections, infection.hunter))
            {
                infection.isTrapAssigned = true;
            }
        }
    }

    if (infections.empty())
        infectionsField.reset();

    return false;
}

// A hunter already sent for another infection, its trap not yet down, lays one trap for both. Else
// the first trap hunter in order with Frost Trap ready. Never an infected hunter: its own
// Shadowfiends come out on the trap and springing it engages them onto the hunter who laid it.
// False while none is ready, so the infection is tried again next tick.
bool IllidanStormrageTrackInfectionsAction::FindTrapHunter(
    Player const& infected, Position const& infectionPosition,
    std::unordered_map<ObjectGuid, IllidanInfection> const& infections, ObjectGuid& hunter)
{
    uint32 const frostTrap = Id(BtSpells::SPELL_FROST_TRAP);
    for (auto const& [player, infection] : infections)
    {
        Player* other = ObjectAccessor::FindPlayer(infection.hunter);
        if (other && other->IsAlive() && !other->HasSpellCooldown(frostTrap) &&
            !HasParasiticShadowfiend(other))
        {
            hunter = infection.hunter;
            return true;
        }
    }

    auto const& trapHunters =
        context->GetValue<GuidVector>("illidan stormrage trap hunters")->RefGet();
    ObjectGuid candidate;
    for (ObjectGuid const& guid : trapHunters)
    {
        Player* trapHunter = ObjectAccessor::FindPlayer(guid);
        if (trapHunter && trapHunter->IsAlive() && !trapHunter->HasSpellCooldown(frostTrap) &&
            !HasParasiticShadowfiend(trapHunter))
        {
            candidate = guid;
            break;
        }
    }

    if (candidate.IsEmpty())
        return false;

    // Ice already round the infection position that outlasts the debuff needs no new trap.
    Aura const* parasite = infected.GetAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_1));
    if (!parasite)
        parasite = infected.GetAura(Id(BtSpells::SPELL_PARASITIC_SHADOWFIEND_2));

    int32 const parasiteMs = parasite ? parasite->GetDuration() : 0;
    bool const isCovered =
        IsFrostTrapPatchAt(bot, infectionPosition, parasiteMs + ILLIDAN_FROST_TRAP_SLOW_MS);
    hunter = isCovered ? ObjectGuid::Empty : candidate;
    return true;
}

// Ranged kill the East Flame first. A tank whose Flame is dead waits on the grate, unless the
// other Flame's tank is dead too, when it takes that Flame.
bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event /*event*/)
{
    Unit* flame = GetAssignedFlameOfAzzinoth(bot);
    if (!flame)
    {
        // An unrecorded Flame hasn't spawned yet, so the tank waits by its glaive.
        bool const isEast = IsEastFlameOfAzzinothTank(bot);
        BtInstanceState const* state = FindBtState(bot->GetInstanceId());
        bool const isRecorded =
            state && (isEast ? state->eastFlameGuid : state->westFlameGuid).has_value();
        Position position;
        if (isEast && isRecorded)
            position = ILLIDAN_E_GRATE_POSITION;
        else if (isEast)
            position = ILLIDAN_E_GLAIVE_WAITING_POSITION;
        else if (isRecorded)
            position = ILLIDAN_W_GRATE_POSITION;
        else
            position = ILLIDAN_W_GLAIVE_WAITING_POSITION;

        if (bot->GetExactDist2d(position) <= 0.5f)
            return false;

        return MoveTo(
            BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (AI_VALUE(Unit*, "current target") != flame)
        return Attack(flame);

    // Until it has its Flame, the tank's own logic takes it (taunt, reach).
    if (flame->GetVictim() != bot)
        return false;

    EyeBlastDangerArea const dangerArea = GetEyeBlastDangerArea(bot);

    // Only consider the eye blast if its trigger NPC is within 30 yards of the tank
    constexpr float eyeBlastTriggerRadius = 30.0f;
    if (dangerArea.width > 0.0f && bot->GetExactDist2d(dangerArea.start) <= eyeBlastTriggerRadius)
        return RepositionToAvoidEyeBlast(dangerArea);

    return RepositionToAvoidBlaze(flame);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(
    EyeBlastDangerArea const& dangerArea)
{
    if (!IsPositionInEyeBlastDangerArea(bot->GetPosition(), dangerArea))
        return false;

    float const dx = dangerArea.end.GetPositionX() - dangerArea.start.GetPositionX();
    float const dy = dangerArea.end.GetPositionY() - dangerArea.start.GetPositionY();
    float const length = dangerArea.start.GetExactDist2d(dangerArea.end);

    float const px = bot->GetPositionX();
    float const py = bot->GetPositionY();
    float const sx = dangerArea.start.GetPositionX();
    float const sy = dangerArea.start.GetPositionY();

    float const projection = std::clamp(
        ((px - sx) * dx + (py - sy) * dy) / (length * length), 0.0f, 1.0f);
    float const closestX = sx + projection * dx;
    float const closestY = sy + projection * dy;

    float const distToLine = bot->GetExactDist2d(closestX, closestY);
    float const moveDist = (dangerArea.width - distToLine) + 0.5f;
    if (moveDist <= 0.0f)
        return false;

    float const rawDirX = px - closestX;
    float const rawDirY = py - closestY;
    float const rawDirLength = std::sqrt(rawDirX * rawDirX + rawDirY * rawDirY);
    float const dirX = rawDirLength == 0.0f ? -(dy / length) : rawDirX / rawDirLength;
    float const dirY = rawDirLength == 0.0f ? dx / length : rawDirY / rawDirLength;

    float const safeX = px + dirX * moveDist;
    float const safeY = py + dirY * moveDist;
    float const safeZ = bot->GetPositionZ();
    Position const safePosition(safeX, safeY, safeZ);

    constexpr float minGrateDistance = 10.0f;
    bool const tooCloseToNorthGrate =
        safePosition.GetExactDist2d(ILLIDAN_N_GRATE_POSITION) < minGrateDistance;
    bool const tooCloseToEastGrate =
        safePosition.GetExactDist2d(ILLIDAN_E_GRATE_POSITION) < minGrateDistance;
    bool const tooCloseToWestGrate =
        safePosition.GetExactDist2d(ILLIDAN_W_GRATE_POSITION) < minGrateDistance;

    if (tooCloseToNorthGrate || tooCloseToEastGrate || tooCloseToWestGrate)
        return false;

    return MoveTo(
        BT_MAP_ID, safeX, safeY, safeZ, false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(Unit* flame)
{
    if (flame->GetVictim() != bot || !bot->IsWithinMeleeRange(flame))
        return false;

    // The waypoints go with the Flame, which must stay near its blade.
    auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    auto const& waypoints = flame == eastFlame ? E_GLAIVE_TANK_POSITIONS : W_GLAIVE_TANK_POSITIONS;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible triggers")->Get();

    bool blazeNearby = false;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == Id(BtNpcs::NPC_BLAZE) &&
            bot->GetDistance2d(unit) <= 8.0f)
        {
            blazeNearby = true;
            break;
        }
    }

    constexpr float arrivalDist = 0.2f;
    if (blazeNearby && bot->GetExactDist2d(waypoints[_tankStep]) <= arrivalDist)
        _tankStep = (_tankStep + 1) % waypoints.size();

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, waypoints[_tankStep], arrivalDist, flame, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive. And just like
// players, most pets cannot melee Illidan during Phase 4. Also run outside the encounter so a pet
// set passive gets its stance back after a wipe.
bool IllidanStormrageControlPetAggressionAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    IllidanPhase const phase =
        GetIllidanPhase(AI_VALUE2(Unit*, "find target", "illidan stormrage"));
    if (phase == IllidanPhase::Flying || phase == IllidanPhase::Demon)
    {
        if (!pet->HasReactState(REACT_PASSIVE))
        {
            _previousReactState = pet->GetReactState();
            _setPassive = true;
            pet->AttackStop();
            pet->SetReactState(REACT_PASSIVE);
        }
    }
    else if (_setPassive)
    {
        pet->SetReactState(_previousReactState);
        _setPassive = false;
    }

    return false;
}

// Shadow Demons first in Demon Form, then Shadowfiends. A commanded attack, which a passive pet
// carries out too. While its owner has just birthed Shadowfiends, the pet is called off them.
bool IllidanStormrageCommandPetAttackAddAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    bool const isNearNewShadowfiend = IsNearNewParasiticShadowfiend(botAI);
    Unit* victim = pet->GetVictim();
    if (isNearNewShadowfiend && victim &&
        victim->GetEntry() == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
    {
        botAI->PetFollow();
        return true;
    }

    IllidanPhase const phase =
        GetIllidanPhase(AI_VALUE2(Unit*, "find target", "illidan stormrage"));
    Unit* target =
        phase == IllidanPhase::Demon ? GetShadowDemonForPetToAttack(botAI, *pet) : nullptr;
    if (!target && !isNearNewShadowfiend)
        target = GetNearestParasiticShadowfiend(botAI, *pet);

    if (!target || !pet->IsValidAttackTarget(target))
        return false;

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());
    if (CharmInfo* charmInfo = pet->GetCharmInfo())
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

bool IllidanStormragePositionAboveGrateAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> bots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BT_MAP_ID && GET_PLAYERBOT_AI(member) &&
            !IsFlameOfAzzinothTank(member))
        {
            bots.push_back(member);
        }
    }

    if (bots.empty())
        return false;

    std::sort(bots.begin(), bots.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto const it = std::find(bots.begin(), bots.end(), bot);
    if (it == bots.end())
        return false;

    auto const& gratePositions = GRATE_POSITIONS;
    size_t const botIndex = std::distance(bots.begin(), it);
    uint8 const index = botIndex % gratePositions.size();

    Position const& position = gratePositions[index];
    if (bot->GetExactDist2d(position) <= 0.2f)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageClearDebuffWithImmunityAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool IllidanStormrageCancelImmunityAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    bot->RemoveOwnedAura(spellId, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    return true;
}

// Everyone but the Main Tank waits stacked 22 yd south of the centre, out of his way to the
// ring.
bool IllidanStormragePositionForLandingAction::Execute(Event /*event*/)
{
    Position const& position = ILLIDAN_LANDING_STACK_POSITION;
    if (bot->GetExactDist2d(position) <= ILLIDAN_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageMeleeStopAttackingAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());
    return true;
}

// The target is kept: the hold keeps the shot or channel from starting again.
bool IllidanStormrageStopShootingAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    bool isStopped = false;
    for (CurrentSpellTypes const type : { CURRENT_AUTOREPEAT_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = bot->GetCurrentSpell(type);
        if (spell && spell->m_targets.GetUnitTargetGUID() == illidan->GetGUID())
        {
            bot->InterruptSpell(type);
            isStopped = true;
        }
    }

    return isStopped;
}

bool IllidanStormrageSpreadRangedAction::Execute(Event /*event*/)
{
    if (HasParasiticShadowfiend(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    // Phase 1: stacked on the grate centre. Phases 3 and 5: in the grate slots by roster, healers
    // first, the warlock tank in the one nearest its Demon Form position.
    IllidanPhase const phase = GetIllidanPhase(illidan);
    Position position;
    if (phase == IllidanPhase::Demon)
    {
        if (!FindDemonFormSlot(*illidan, position))
            return false;
    }
    else if (phase == IllidanPhase::Initial)
    {
        position = ILLIDAN_GRATE_CENTRE;
    }
    else
    {
        int32 const slot = GetIllidanGrateIndex(botAI, *illidan);
        if (slot < 0)
            return false;

        position = GetGrateSlot(static_cast<size_t>(slot));
    }

    if (bot->GetExactDist2d(position) <= ILLIDAN_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Phase 4: healers in the healer slots and ranged DPS in theirs, each by roster.
bool IllidanStormrageSpreadRangedAction::FindDemonFormSlot(
    Unit const& illidan, Position& position)
{
    DemonFormLayout const layout = GetDemonFormLayout(illidan);
    if (PlayerbotAI::IsHeal(bot))
    {
        int32 const slot = GetIllidanHealerIndex(botAI, layout.healerSlots.size());
        if (slot < 0)
            return false;

        position = layout.healerSlots[static_cast<size_t>(slot)];
        return true;
    }

    constexpr bool inDemonForm = true;
    int32 const slot = GetIllidanRangedIndex(botAI, layout.rangedSlots.size(), inDemonForm);
    if (slot < 0)
        return false;

    position = layout.rangedSlots[static_cast<size_t>(slot)];
    return true;
}

// Short steps toward the Demon, so the bot stops once in reach, never into Aura of Dread or near
// Shadow Blast round his victim. Spread ranged brings it back once the Demon is dead.
bool IllidanStormrageReachShadowDemonAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Unit* demon = GetShadowDemonWalkingToWarlockTank(botAI);
    if (!demon)
        return false;

    constexpr float maxStep = 3.5f;
    constexpr float reachMargin = 0.5f;
    constexpr float auraOfDreadClearance = 17.0f;
    constexpr float shadowBlastClearance = 22.0f;

    float const range = botAI->GetRange("spell");
    if (bot->IsWithinCombatRange(demon, range))
        return false;

    float const reach = range + bot->GetCombatReach() + demon->GetCombatReach();
    float const distance = bot->GetExactDist2d(demon);

    float const step = std::min(maxStep, std::max(distance - reach, 0.0f) + reachMargin);
    float const x =
        bot->GetPositionX() + (demon->GetPositionX() - bot->GetPositionX()) / distance * step;
    float const y =
        bot->GetPositionY() + (demon->GetPositionY() - bot->GetPositionY()) / distance * step;

    Unit* victim = illidan->GetVictim();
    if (illidan->GetExactDist2d(x, y) < auraOfDreadClearance ||
        (victim && victim->GetExactDist2d(x, y) < shadowBlastClearance))
    {
        return false;
    }

    return MoveTo(
        BT_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Melee and tanks wait in their Demon Form slots, facing him, and leave them only for a Shadow
// Demon.
bool IllidanStormrageMeleeStayAwayFromDemonFormAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    DemonFormLayout const layout = GetDemonFormLayout(*illidan);
    int32 const slot = GetIllidanMeleeIndex(botAI, layout.meleeSlots.size());
    if (slot < 0)
        return false;

    Position const& position = layout.meleeSlots[static_cast<size_t>(slot)];
    if (bot->GetExactDist2d(position) <= ILLIDAN_POSITION_TOLERANCE)
    {
        if (!bot->HasInArc(static_cast<float>(M_PI) / 2.0f, illidan))
            bot->SetFacingTo(bot->GetAngle(illidan));

        return false;
    }

    return MoveTo(
        BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// The warlock tank holds him from its position with Shadow Ward and Searing Pain, Life Tapping
// between Searing Pains.
bool IllidanStormrageWarlockTankHandleDemonFormAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (bot->HasAura(Id(BtSpells::SPELL_VIGILANCE)))
    {
        bot->RemoveOwnedAura(
            Id(BtSpells::SPELL_VIGILANCE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    if (botAI->CanCastSpell("shadow ward", bot) && botAI->CastSpell("shadow ward", bot))
        return true;

    if (CastIllidanLifeTapIfLow(botAI))
        return true;

    Position const position = GetDemonFormLayout(*illidan).warlockTankPosition;
    if (bot->GetExactDist2d(position) > ILLIDAN_POSITION_TOLERANCE)
    {
        return MoveTo(
            BT_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None)
        return false;

    return botAI->CanCastSpell("searing pain", illidan) &&
        botAI->CastSpell("searing pain", illidan);
}

// A channel would hold the first Seed back; Life Tap comes between Seeds.
bool IllidanStormrageSeedShadowDemonsAction::Execute(Event /*event*/)
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        bot->CastStop();
        return true;
    }

    if (CastIllidanLifeTapIfLow(botAI))
        return true;

    Unit* target = FindSeedTarget();
    return target && botAI->CanCastSpell("seed of corruption", target) &&
        botAI->CastSpell("seed of corruption", target);
}

// The bot's usual Demon, else any Demon, each only without the bot's own Seed on it.
Unit* IllidanStormrageSeedShadowDemonsAction::FindSeedTarget()
{
    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("shadow demons")->RefGet();
    uint32 const seed = AI_VALUE2(uint32, "spell id", "seed of corruption");
    Unit* usualDemon = GetShadowDemonForRangedToAttack(botAI, *bot);
    if (usualDemon && !usualDemon->HasAura(seed, bot->GetGUID()))
        return usualDemon;

    for (ShadowDemon const& shadowDemon : shadowDemons)
    {
        Unit* demon = botAI->GetUnit(shadowDemon.demon);
        if (demon && demon->IsAlive() && !demon->HasAura(seed, bot->GetGUID()))
            return demon;
    }

    return nullptr;
}

bool IllidanStormrageAssignTargetPriorityAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Heals outrank this, so a healer keeps its Demon until it dies rather than losing heal ticks
    // to target switches.
    if (phase == IllidanPhase::Demon && PlayerbotAI::IsHeal(bot) && currentTarget &&
        currentTarget->IsAlive() && currentTarget->GetEntry() == Id(BtNpcs::NPC_SHADOW_DEMON))
    {
        return false;
    }

    Unit* target = nullptr;
    for (Unit* candidate : GetTargetsInPriorityOrder(illidan, phase))
    {
        if (candidate && candidate->IsAlive())
        {
            target = candidate;
            break;
        }
    }

    if (!target)
    {
        // Melee in Demon Form with nothing to hit drop their target, so they stay off him and off
        // the Demon walking to the warlock tank.
        if (phase != IllidanPhase::Demon || PlayerbotAI::IsHeal(bot) ||
            !PlayerbotAI::IsMelee(bot) || (!currentTarget && !bot->GetVictim()))
        {
            return false;
        }

        bot->AttackStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
        return true;
    }

    if (target == currentTarget)
        return false;

    // Attack() starts the swing, which a hold can't stop, so in a hold melee only take him as
    // target and walk in; the melee action starts the swing once the hold ends.
    if (target == illidan && !PlayerbotAI::IsRanged(bot) &&
        GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None)
    {
        context->GetValue<Unit*>("current target")->Set(illidan);
        bot->SetSelection(illidan->GetGUID());
        botAI->ChangeEngine(BOT_STATE_COMBAT);
        return true;
    }

    return Attack(target);
}

// Demon Form: Shadow Demons first for everyone, healers included; then for melee a nearly dead
// Shadowfiend in their reach, for ranged any Shadowfiend in their band; then him, except for melee
// and tanks, who never take him in the form.
std::vector<Unit*> IllidanStormrageAssignTargetPriorityAction::GetTargetsInPriorityOrder(
    Unit* illidan, IllidanPhase phase)
{
    bool const isGrounded = phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;

    if (phase == IllidanPhase::Demon)
    {
        if (PlayerbotAI::IsHeal(bot))
        {
            return { GetShadowDemonForRangedToAttack(botAI, *bot) };
        }
        else if (IsIllidanWarlockTank(botAI))
        {
            return { GetShadowDemonForRangedToAttack(botAI, *bot), illidan };
        }
        else if (PlayerbotAI::IsMelee(bot))
        {
            return { GetShadowDemonForMeleeToAttack(botAI, *bot), FindShadowfiendForMelee() };
        }
        else
        {
            return {
                GetShadowDemonForRangedToAttack(botAI, *bot), FindShadowfiendForRanged(), illidan,
            };
        }
    }
    else if (PlayerbotAI::IsRanged(bot))
    {
        if (isGrounded)
        {
            return { FindShadowfiendForRanged(), illidan };
        }
        else if (phase == IllidanPhase::Flying)
        {
            auto const [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
            return { FindShadowfiendForRanged(), eastFlame, westFlame };
        }
    }
    // Melee, assist tanks included, are on him, but first finish a nearly dead Shadowfiend in
    // their reach.
    else if (isGrounded)
    {
        return { FindShadowfiendForMelee(), illidan };
    }

    return {};
}

// Strict, since hitting a Shadowfiend draws it and its first hit infects: only the nearest one,
// only already in melee range and only nearly dead.
Unit* IllidanStormrageAssignTargetPriorityAction::FindShadowfiendForMelee()
{
    Unit* shadowfiend = GetNearestParasiticShadowfiend(botAI, *bot);
    if (!shadowfiend || shadowfiend->GetHealthPct() >= ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT)
        return nullptr;

    return bot->IsWithinMeleeRange(shadowfiend) ? shadowfiend : nullptr;
}

Unit* IllidanStormrageAssignTargetPriorityAction::FindShadowfiendForRanged()
{
    Unit* shadowfiend = GetNearestParasiticShadowfiend(botAI, *bot);
    if (!shadowfiend)
        return nullptr;

    constexpr float minDistance = 10.0f;
    constexpr float maxDistance = 28.5f;
    float const distance = bot->GetDistance(shadowfiend);
    return distance > minDistance && distance <= maxDistance ? shadowfiend : nullptr;
}

bool IllidanStormrageUseCageTrapAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    GameObject* trap = FindNearestCageTrap(botAI);
    if (!trap || illidan->GetExactDist2d(trap) >= 4.0f)
        return false;

    if (bot->GetExactDist2d(trap) < 3.0f)
    {
        trap->Use(bot);
        return true;
    }

    return MoveTo(
        BT_MAP_ID, trap->GetPositionX(), trap->GetPositionY(), trap->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageManageDpsTimersAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    uint32 const now = getMSTime();
    uint32 const instanceId = illidan->GetMap()->GetInstanceId();

    IllidanPhase const phase = GetIllidanPhase(illidan);
    bool updated = RecordIllidanPhase(instanceId, phase);
    BtInstanceState& state = BtState(instanceId);

    if (phase == IllidanPhase::Flying)
    {
        // Each Flame is recorded as it spawns, so its tank can take it at once; the West one comes
        // about 0.7 s before the East one. The head-start hold counts from the first.
        if (!state.eastFlameGuid || !state.westFlameGuid)
        {
            std::list<Creature*> flames;
            bot->GetCreatureListWithEntryInGrid(
                flames, Id(BtNpcs::NPC_FLAME_OF_AZZINOTH), ILLIDAN_ADD_SEARCH_RADIUS);

            for (Creature* flame : flames)
            {
                if (!flame || !flame->IsAlive())
                    continue;

                bool const isEast = flame->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION) <
                    flame->GetExactDist2d(ILLIDAN_W_GLAIVE_WAITING_POSITION);
                auto& flameGuid = isEast ? state.eastFlameGuid : state.westFlameGuid;
                if (EmplaceIfUnset(flameGuid, flame->GetGUID()))
                {
                    EmplaceIfUnset(state.illidanFlamePhaseStartTime, now);
                    updated = true;
                }
            }
        }
    }
    else
    {
        if (ResetIfSet(state.eastFlameGuid))
            updated = true;
        if (ResetIfSet(state.westFlameGuid))
            updated = true;
    }

    return updated;
}

bool IllidanStormrageClearHazardsAction::Execute(Event /*event*/)
{
    constexpr float hazardSearchRadius = 50.0f;
    std::vector<uint32> const entries =
        { Id(BtNpcs::NPC_FLAME_CRASH), Id(BtNpcs::NPC_DEMON_FIRE), Id(BtNpcs::NPC_BLAZE) };
    std::list<Creature*> hazards;
    bot->GetCreatureListWithEntryInGrid(hazards, entries, hazardSearchRadius);

    bool isCleared = false;
    for (Creature* creature : hazards)
    {
        if (creature && creature->IsAlive())
        {
            creature->Kill(bot, creature);
            isCleared = true;
        }
    }

    return isCleared;
}
