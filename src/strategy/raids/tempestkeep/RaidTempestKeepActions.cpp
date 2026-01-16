#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "AiFactory.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using namespace TempestKeepHelpers;

// General

bool TempestKeepEraseTimersAndTrackersAction::Execute(Event event)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    bool erased = false;
    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
    {
        erased |= lastRebirthState.erase(instanceId) > 0;
        erased |= isAlarInPhase2.erase(instanceId) > 0;
    }
    if (!AI_VALUE2(Unit*, "find target", "void reaver"))
    {
        erased |= initialVoidReaverPositions.erase(guid) > 0;
        erased |= hasReachedInitialVoidReaverPosition.erase(guid) > 0;
    }
    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        erased |= advisorDpsWaitTimer.erase(instanceId) > 0;

    return erased;
}

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event event)
{
    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");
    if (!centurion)
        return false;

    if (centurion->GetHealth() == centurion->GetMaxHealth())
    {
        if (!centurion->HasAura(SPELL_POLYMORPH_SHEEP) &&
            !centurion->HasAura(SPELL_POLYMORPH_TURTLE) &&
            !centurion->HasAura(SPELL_POLYMORPH_PIG))
        {
            if (botAI->CanCastSpell("polymorph", centurion))
                return botAI->CastSpell("polymorph", centurion);
        }
    }
    else if (centurion->HasAura(SPELL_ARCANE_FLURRY))
    {
        botAI->Reset();
        return botAI->CastSpell("polymorph", centurion);
    }

    return false;
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", alar))
        return botAI->CastSpell("steady shot", alar);

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    MarkTargetWithStar(bot, alar);
    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (botAI->IsMainTank(bot) && PositionMainTank(bot, alar, locationIndex))
        return true;

    if (botAI->IsAssistTankOfIndex(bot, 0) && PositionAssistTank(bot, alar, locationIndex))
        return true;

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(
    Player* mainTank, Unit* alar, int8 locationIndex)
{
    if (!mainTank)
        return false;

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_3_IDX)
                ? PLATFORM_POSITIONS[0] : PLATFORM_POSITIONS[2];

        if (mainTank->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (mainTank->GetTarget() != alar->GetGUID())
        {
            return Attack(alar);
        }
        else if (alar->GetVictim() != mainTank)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, alar))
                    return botAI->CastSpell(spellName, alar);
            }
        }
    }

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(
    Player* assistTank, Unit* alar, int8 locationIndex)
{
    if (!assistTank)
        return false;

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_1_IDX)
                ? PLATFORM_POSITIONS[1] : PLATFORM_POSITIONS[3];

        if (assistTank->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (assistTank->GetTarget() != alar->GetGUID())
        {
            return Attack(alar);
        }
        else if (alar->GetVictim() != assistTank)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, alar))
                    return botAI->CastSpell(spellName, alar);
            }
        }
    }

    return false;
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event event)
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target = PLATFORM_POSITIONS[locationIndex];

        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    return false;
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event event)
{
    if (!botAI->IsRanged(bot) && !botAI->IsAssistTankOfIndex(bot, 1))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position groundPositions[] =
            {ALAR_GROUND_0, ALAR_GROUND_1, ALAR_GROUND_2, ALAR_GROUND_3};
        const Position& groundTarget = groundPositions[locationIndex];

        if (botAI->IsRanged(bot))
        {
            if (bot->GetExactDist2d(
                groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 8.0f)
            {
                return MoveInside(TEMPEST_KEEP_MAP_ID, groundTarget.GetPositionX(),
                                  groundTarget.GetPositionY(), groundTarget.GetPositionZ(),
                                  8.0f, MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1))
        {
            Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
            if (!ember && bot->GetExactDist2d(
                groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 20.0f)
            {
                return MoveInside(TEMPEST_KEEP_MAP_ID, groundTarget.GetPositionX(),
                                  groundTarget.GetPositionY(), groundTarget.GetPositionZ(),
                                  20.0f, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool AlarAssistTanksPickUpEmbersAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return HandlePhase1Embers(alar);
    else
        return HandlePhase2Embers(alar);

    return false;
}

// Embers will be tanked by only the second assist tank in Phase 1
bool AlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!botAI->IsAssistTankOfIndex(bot, 1))
        return false;

    Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
    if (ember)
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetTarget() != ember->GetGUID())
            return Attack(ember);

        if (ember->GetVictim() == bot)
        {
            int8 locationIndex = GetAlarCurrentLocationIndex(alar);
            if (locationIndex == LOCATION_NONE)
            {
                Position dest;
                locationIndex = GetAlarDestinationLocationIndex(alar, dest);
            }

            if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
            {
                const Position& groundTarget = GROUND_POSITIONS[locationIndex];
                const Position& center = ALAR_POINT_MIDDLE;
                float dx = center.GetPositionX() - groundTarget.GetPositionX();
                float dy = center.GetPositionY() - groundTarget.GetPositionY();
                float distToCenter = groundTarget.GetExactDist2d(center.GetPositionX(), center.GetPositionY());

                float moveDist = 25.0f;
                float targetX = groundTarget.GetPositionX() + (dx / distToCenter) * moveDist;
                float targetY = groundTarget.GetPositionY() + (dy / distToCenter) * moveDist;
                float targetZ = groundTarget.GetPositionZ();

                return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            else
            {
                const float safeDistance = 15.0f;
                Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
                if (nearestPlayer)
                    return MoveFromGroup(safeDistance + 1.0f);
            }
        }
        else if (!bot->IsWithinMeleeRange(ember))
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ember->GetPositionX(), ember->GetPositionY(),
                          ember->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar)
bool AlarAssistTanksPickUpEmbersAction::HandlePhase2Embers(Unit* alar)
{
    auto [firstEmber, secondEmber] = GetFirstTwoEmbersOfAlar(botAI);

    if (botAI->IsAssistTankOfIndex(bot, 1) && firstEmber)
    {
        MarkTargetWithSquare(bot, firstEmber);
        SetRtiTarget(botAI, "square", firstEmber);

        if (firstEmber->GetVictim() != bot)
        {
            if (bot->GetTarget() != firstEmber->GetGUID())
                return Attack(firstEmber);

            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, firstEmber))
                    return botAI->CastSpell(spellName, firstEmber);
            }
        }
        else if (bot->IsWithinMeleeRange(firstEmber))
        {
            const float safeDistance = 15.0f;
            Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, safeDistance);
            if (nearestPlayer)
                return MoveFromGroup(safeDistance + 1.0f);
        }
    }
    else if (GetSecondEmberTank(botAI, alar) == bot && secondEmber)
    {
        MarkTargetWithCircle(bot, secondEmber);
        SetRtiTarget(botAI, "circle", secondEmber);

        if (secondEmber->GetVictim() != bot)
        {
            if (bot->GetTarget() != secondEmber->GetGUID())
                return Attack(secondEmber);

            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, secondEmber))
                    return botAI->CastSpell(spellName, secondEmber);
            }
        }
        else if (bot->IsWithinMeleeRange(secondEmber))
        {
            const float safeDistance = 15.0f;
            Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, safeDistance);
            if (nearestPlayer)
                return MoveFromGroup(safeDistance + 1.0f);
        }
    }

    return false;
}

bool AlarRangedDpsPrioritizeEmbersAction::Execute(Event event)
{
    auto [firstEmber, secondEmber] = GetFirstTwoEmbersOfAlar(botAI);

    const float safeDistance = 15.0f;
    if (firstEmber)
    {
        if (bot->GetDistance2d(firstEmber) < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(firstEmber, safeDistance - bot->GetDistance2d(firstEmber));
        }
        SetRtiTarget(botAI, "square", firstEmber);

        if (bot->GetTarget() != firstEmber->GetGUID())
            return Attack(firstEmber);
    }
    else if (secondEmber)
    {
        if (bot->GetDistance2d(secondEmber) < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(secondEmber, safeDistance - bot->GetDistance2d(secondEmber));
        }
        SetRtiTarget(botAI, "circle", secondEmber);

        if (bot->GetTarget() != secondEmber->GetGUID())
            return Attack(secondEmber);
    }
    else if (Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar"))
    {
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    return false;
}

// Jump from platform during Flame Quills and wait at assigned position after landing
bool AlarJumpFromPlatformAction::Execute(Event event)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPEST_KEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else if (botAI->IsMainTank(bot))
    {
        if (bot->GetExactDist2d(ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
                          ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (bot->GetExactDist2d(ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
                          ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 1))
    {
        if (bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > 25.0f)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
                              ALAR_POINT_MIDDLE.GetPositionZ(), 25.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (botAI->IsMelee(bot))
    {
        if (bot->GetExactDist2d(ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY()) > 5.0f)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
                              ALAR_ROOM_S_CENTER.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (botAI->IsRanged(bot))
    {
        if (bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > 10.0f)
        {
            return MoveInside(TEMPEST_KEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
                              ALAR_POINT_MIDDLE.GetPositionZ(), 10.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPEST_KEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        float currentDistance = bot->GetDistance2d(alar);
        const float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
        {
            botAI->Reset();
            return MoveAway(alar, safeDistance - currentDistance);
        }
    }

    return false;
}

// Main tank and first assist tank will swap tanking Al'ar when Melt Armor is applied
bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (alar->GetHealth() == alar->GetMaxHealth())
    {
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetTarget() != alar->GetGUID())
            return Attack(alar);
    }

    Player* secondEmberTank = GetSecondEmberTank(botAI, alar);
    if (secondEmberTank && secondEmberTank != bot)
    {
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetTarget() != alar->GetGUID())
        {
            return Attack(alar);
        }
        else if (alar->GetVictim() != bot)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, alar))
                    return botAI->CastSpell(spellName, alar);
            }
        }
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (AvoidFlamePatch())
        return true;

    if (HandleDiveBomb(alar))
        return true;

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::AvoidFlamePatch()
{
    std::vector<Unit*> flamePatches = GetAllHazardTriggers(botAI, bot, NPC_FLAME_PATCH, 40.0f);
    const float hazardRadius = 8.0f;
    for (Unit* flamePatch : flamePatches)
    {
        if (bot->GetExactDist2d(flamePatch) < hazardRadius)
        {
            Position safestPos = FindSafestNearbyPosition(bot, flamePatches, 30.0f, hazardRadius);
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(TEMPEST_KEEP_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                          safestPos.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    if ((alar->HasUnitState(UNIT_STATE_CASTING) &&
        alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_DIVE)) ||
        !alar->IsVisible())
    {
        float currentDistance = bot->GetDistance2d(alar);
        const float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(alar, safeDistance - currentDistance);
        }
    }
    else
    {
        Position dest;
        if (GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
            GetAlarDestinationLocationIndex(alar, dest) == POINT_QUILL_OR_DIVE_IDX)
        {
            const float safeDistance = 10.0f;
            Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
            if (nearestPlayer)
            {
                const uint32 minInterval = 0;
                return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                             nearestPlayer->GetPositionZ()), safeDistance, minInterval);
            }
        }
    }

    return false;
}

// For Phase 2, ensure that bots don't get too far away and become inactive
bool AlarReturnToRoomCenterAction::Execute(Event event)
{
    const Position& center = ALAR_ROOM_CENTER;
    if (bot->GetVictim() == nullptr &&
        bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 45.0f)
    {
        return MoveInside(TEMPEST_KEEP_MAP_ID, center.GetPositionX(), center.GetPositionY(),
                          center.GetPositionZ(), 40.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AlarManagePhaseTrackerAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 instanceId = alar->GetMap()->GetInstanceId();

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
                         alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_PHASE2);
    bool lastRebirth = lastRebirthState[instanceId];

    if (lastRebirth && !rebirthActive)
        isAlarInPhase2[instanceId] = true;

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver

bool VoidReaverTanksPositionBossAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    const Position& position = VOID_REAVER_TANK_POSITION;

    float dX = position.GetPositionX() - bot->GetPositionX();
    float dY = position.GetPositionY() - bot->GetPositionY();
    float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                       position.GetPositionY());

    if (bot->IsWithinMeleeRange(voidReaver) && distanceToPosition > 2.0f)
    {
        float moveDist = std::min(5.0f, distanceToPosition);
        float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

        return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, true);
    }

    return false;
}

bool VoidReaverRangedUseAggroDumpAbilityAction::Execute(Event event)
{
    botAI->Reset();
    static const std::array<const char*, 6> spells =
    {
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot))
            return botAI->CastSpell(spell, bot);
    }

    return false;
}

// As far as I can tell, it is not possible for bots to detect Arcane Orbs
// Therefore, this spreads out the ranged bots so as few of them as possible get hit
bool VoidReaverSpreadRangedAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    if (initialVoidReaverPositions.empty())
    {
        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !botAI->IsRanged(member))
                    continue;
                if (botAI->IsHeal(member))
                    healers.push_back(member);
                else
                    rangedDps.push_back(member);
            }
        }

        const Position& position = VOID_REAVER_TANK_POSITION;
        const float radius = 30.0f;
        const float offsetArc = 1.0f;
        const uint8 botsPerRing = 8;

        std::vector<Player*> rangedBots = healers;
        rangedBots.insert(rangedBots.end(), rangedDps.begin(), rangedDps.end());

        for (size_t i = 0; i < rangedBots.size(); ++i)
        {
            Player* ranged = rangedBots[i];
            uint8 ringIndex = i / botsPerRing;
            uint8 posInRing = i % botsPerRing;
            float ringRadius = radius + (ringIndex * offsetArc);
            float angle = 2 * M_PI * posInRing / botsPerRing;

            float targetX = position.GetPositionX() + ringRadius * std::cos(angle);
            float targetY = position.GetPositionY() + ringRadius * std::sin(angle);

            Position pos(targetX, targetY, ranged->GetPositionZ());
            initialVoidReaverPositions[ranged->GetGUID()] = pos;
            hasReachedInitialVoidReaverPosition[ranged->GetGUID()] = false;
        }
    }

    Position targetPosition = initialVoidReaverPositions[bot->GetGUID()];
    float destX = targetPosition.GetPositionX();
    float destY = targetPosition.GetPositionY();

    if (bot->GetExactDist2d(destX, destY) > 1.0f)
    {
        return MoveTo(TEMPEST_KEEP_MAP_ID, destX, destY, targetPosition.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

Position VoidReaverSpreadRangedAction::GetRangedBotPosition(const Position& center,
    float radius, uint8 botsPerRing, float offsetArc, uint8 botIndex, float botZ)
{
    float angleOffset = (offsetArc / radius);
    uint8 ringIndex = botIndex / botsPerRing;
    uint8 posInRing = botIndex % botsPerRing;
    float baseAngle = 2 * M_PI * posInRing / botsPerRing;
    float angle = baseAngle + (ringIndex == 1 ? angleOffset : 0);

    if (ringIndex > 1)
    {
        angle = 2 * M_PI * (botIndex % botsPerRing) / botsPerRing;
        ringIndex = 0;
    }

    float targetX = center.GetPositionX() + radius * cos(angle);
    float targetY = center.GetPositionY() + radius * sin(angle);

    return Position(targetX, targetY, botZ);
}

// High Astromancer Solarian

bool HighAstromancerSolarianRangedLeaveSpaceForMeleeAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    float currentDistance = bot->GetExactDist2d(astromancer);
    const float minDistance = 20.0f;
    if (currentDistance < minDistance)
        return MoveAway(astromancer, minDistance - currentDistance + 2.0f);

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event event)
{
    const float safeDistance = 15.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (nearestPlayer)
    {
        botAI->Reset();
        return MoveFromGroup(safeDistance + 1.0f);
    }

    return false;
}

bool HighAstromancerSolarianStackForAoeAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* stackTarget = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsRanged(member))
        {
            stackTarget = member;
            break;
        }
    }

    if (stackTarget && bot != stackTarget && bot->GetExactDist2d(stackTarget) >= 5.0f)
    {
        return MoveTo(TEMPEST_KEEP_MAP_ID, stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                      stackTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Split melee into two groups, one on each Solarium Priest
bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event event)
{
    auto priestsPair = GetSolariumPriests(botAI);
    if (!priestsPair.first || !priestsPair.second)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto meleeMembers = GetMeleeBots(group);
    if (meleeMembers.empty())
        return false;

    Unit* targetPriest = AssignSolariumPriestsToBots(priestsPair, meleeMembers);
    if (!targetPriest)
        return false;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return false;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (targetPriest == priestsPair.first)
    {
        MarkTargetWithSquare(bot, targetPriest);
        SetRtiTarget(botAI, "square", targetPriest);
    }
    else
    {
        MarkTargetWithStar(bot, targetPriest);
        SetRtiTarget(botAI, "star", targetPriest);
    }

    if (bot->GetTarget() != targetPriest->GetGUID())
        return Attack(targetPriest);

    return false;
}

std::pair<Unit*, Unit*> HighAstromancerSolarianTargetSolariumPriestsAction::GetSolariumPriests(PlayerbotAI* botAI)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    for (auto const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_SOLARIUM_PRIEST)
        {
            if (!lowest || unit->GetGUID().GetRawValue() < lowest->GetGUID().GetRawValue())
                lowest = unit;

            if (!highest || unit->GetGUID().GetRawValue() > highest->GetGUID().GetRawValue())
                highest = unit;
        }
    }

    return {lowest, highest};
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots(Group* group)
{
    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && GET_PLAYERBOT_AI(member))
            meleeMembers.push_back(member);
    }

    std::sort(meleeMembers.begin(), meleeMembers.end(),
              [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    const std::pair<Unit*, Unit*>& priestsPair, const std::vector<Player*>& meleeMembers)
{
    if (!priestsPair.first || !priestsPair.second || meleeMembers.empty())
        return nullptr;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (totalMelee == 1)
        return priestsPair.first;

    size_t split = totalMelee / 2;
    if (botIndex < split)
        return priestsPair.first;
    else
        return priestsPair.second;
}

bool HighAstromancerSolarianTankVoidwalkerAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    if (bot->GetTarget() != astromancer->GetGUID())
        return Attack(astromancer);

    if (astromancer->GetVictim() != bot)
    {
        const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
        for (const char* spellName : taunts)
        {
            if (botAI->CanCastSpell(spellName, astromancer))
                return botAI->CastSpell(spellName, astromancer);
        }
    }

    return false;
}

bool HighAstromancerSolarianCastFearWardOnMainTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event event)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    float currentDistance = bot->GetExactDist2d(thaladred);
    const float safeDistance = 22.0f;
    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        return MoveAway(thaladred, safeDistance - currentDistance);
    }

    return false;
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again for good measure)
bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
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

    Unit* advisorTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(botAI, bot);
    }
    else if (hunterIndex == 1)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(botAI, bot);
    }

    if (!advisorTarget ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        advisorTarget->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return false;

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", advisorTarget))
        return botAI->CastSpell("steady shot", advisorTarget);

    return false;
}

bool KaelthasSunstriderMainTankPositionSanguinarAction::Execute(Event event)
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    MarkTargetWithStar(bot, sanguinar);
    SetRtiTarget(botAI, "star", sanguinar);

    if (bot->GetTarget() != sanguinar->GetGUID())
        return Attack(sanguinar);

    if (sanguinar->GetVictim() == bot && bot->IsWithinMeleeRange(sanguinar))
    {
        const Position& position = SANGUINAR_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Use tank strategy only when necessary to tank Capernian; otherwise, use DPS strategies
bool KaelthasSunstriderManageWarlockTankStrategyAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    bool currentlyTank = botAI->HasStrategy("tank", BotState::BOT_STATE_COMBAT);

    // Phase 1: Switch to tank after Sanguinar is "dead" (feign death)
    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        if (!currentlyTank)
        {
            Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
            if (sanguinar && sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        }
    }
    // Phase 2: Switch to DPS
    else if (kaelAI->GetPhase() == PHASE_WEAPONS)
    {
        if (currentlyTank)
            botAI->ChangeStrategy("-tank", BotState::BOT_STATE_COMBAT);
    }
    // Phase 2→3 Transition: Weapons dead, waiting for advisors - switch to tank
    else if (kaelAI->GetPhase() == PHASE_TRANSITION)
    {
        if (!currentlyTank)
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
    }
    // Phase 3
    else if (kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        // If Capernian is alive, switch to tank strategy
        // (fallback in case not all weapons were down before Phase 3)
        if (capernian && !currentlyTank)
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        // If Capernian is dead, reset to DPS for remainder of encounter
        else if (!capernian && currentlyTank)
            botAI->ChangeStrategy("-tank", BotState::BOT_STATE_COMBAT);
    }

    return false;
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return false;

    MarkTargetWithCircle(bot, capernian);
    SetRtiTarget(botAI, "circle", capernian);

    if (bot->GetTarget() != capernian->GetGUID())
        return Attack(capernian);

    if (capernian->GetVictim() == bot && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        float currentDist = bot->GetDistance2d(capernian);
        if (currentDist == 0.0f)
            return false;

        const float minDistance = 20.0f;
        if (currentDist < minDistance)
            return MoveAway(capernian, minDistance - currentDist + 1.0f);
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (botAI->IsRanged(bot))
    {
        if (RangedBotsDisperse(kaelAI))
            return true;
    }

    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        if (StayBackFromCapernian())
            return true;
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(boss_kaelthas* kaelAI)
{
    if (kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;
    }

    const float safeDistance = 6.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (nearestPlayer)
    {
        const uint32 minInterval = 1000;
        return FleePosition(Position(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(),
                                     nearestPlayer->GetPositionZ()), safeDistance, minInterval);
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::StayBackFromCapernian()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    // Main tank purposely stays in range to bait Conflagration in Phase 1
    if (botAI->IsMainTank(bot))
    {
        const float desiredDist = 15.0f;
        if (fabs(bot->GetExactDist2d(capernian) - desiredDist))
        {
            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();
            float distanceToCapernian = bot->GetExactDist2d(capernian);
            if (distanceToCapernian == 0.0f)
                return false;

            float nx = dx / distanceToCapernian;
            float ny = dy / distanceToCapernian;

            float targetX = capernian->GetPositionX() + nx * desiredDist;
            float targetY = capernian->GetPositionY() + ny * desiredDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, capernian->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return true;
    }

    float safeDistance = 0.0f;
    if (botAI->IsMelee(bot))
        safeDistance = 45.0f;
    else if (botAI->IsRangedDps(bot))
        safeDistance = 25.0f;
    else if (botAI->IsHeal(bot))
        safeDistance = 40.0f;

    float currentDistance = bot->GetExactDist2d(capernian);
    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        return MoveAway(capernian, safeDistance - currentDistance);
    }

    if (botAI->IsMelee(bot))
    {
        botAI->Reset();
        return true;
    }

    return false;
}

bool KaelthasSunstriderFirstAssistTankPositionTelonicusAction::Execute(Event event)
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    MarkTargetWithTriangle(bot, telonicus);
    SetRtiTarget(botAI, "triangle", telonicus);

    if (bot->GetTarget() != telonicus->GetGUID())
        return Attack(telonicus);

    if (telonicus->GetVictim() == bot && bot->IsWithinMeleeRange(telonicus))
    {
        const Position& position = TELONICUS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event event)
{
    const Position* movePosition = nullptr;

    if (botAI->IsHealAssistantOfIndex(bot, 0))
        movePosition = &ADVISOR_HEAL_POSITION;
    else if (botAI->IsMainTank(bot))
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        if (sanguinar && sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &SANGUINAR_WAITING_POSITION;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        if (telonicus && telonicus->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &TELONICUS_WAITING_POSITION;
    }
    else if (GetCapernianTank(botAI, bot) == bot)
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        if (capernian && capernian->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &CAPERNIAN_WAITING_POSITION;
    }

    if (movePosition)
    {
        if (bot->GetExactDist2d(movePosition->GetPositionX(), movePosition->GetPositionY()) > 2.0f)
        {
            return MoveTo(TEMPEST_KEEP_MAP_ID, movePosition->GetPositionX(), movePosition->GetPositionY(),
                          movePosition->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderReequipGearAction::Execute(Event event)
{
    return botAI->DoSpecificAction("equip upgrades", Event(), true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event event)
{
    // Target priority 1: Thaladred, except Capernian tank
    Player* capernianTank = GetCapernianTank(botAI, bot);
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if ((!capernianTank || bot != capernianTank) &&
        thaladred && !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        MarkTargetWithSquare(bot, thaladred);
        SetRtiTarget(botAI, "square", thaladred);

        if (bot->GetTarget() != thaladred->GetGUID())
            return Attack(thaladred);

        return false;
    }

    // Target priority 2: Capernian for ranged only (excluding longbow tank)
    Player* longBowTank = GetNetherstrandLongbowTank(botAI, bot);

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (botAI->IsRangedDps(bot) && (!longBowTank || bot != longBowTank) &&
        capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "circle", capernian);

        if (bot->GetTarget() != capernian->GetGUID())
            return Attack(capernian);

        return false;
    }

    // Target priority 3: Sanguinar (longbow tank and melee move here after Thaladred)
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "star", sanguinar);

        if (bot->GetTarget() != sanguinar->GetGUID())
            return Attack(sanguinar);

        return false;
    }

    // Target priority 4: Telonicus
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "triangle", telonicus);

        if (bot->GetTarget() != telonicus->GetGUID())
            return Attack(telonicus);

        // Melee DPS need to stay at max-ish melee range behind Telonicus (god damn bombs)
        if (botAI->IsMelee(bot) && botAI->IsDps(bot) && telonicus->GetVictim() != bot)
        {
            float maxMeleeRange = bot->GetMeleeRange(telonicus);
            const float meleeRangeBuffer = 0.5f;
            const float tolerance = 0.75f;

            float desiredDist = std::max(2.0f, maxMeleeRange - meleeRangeBuffer);
            float currentDist = bot->GetExactDist2d(telonicus);

            if (fabs(currentDist - desiredDist) > tolerance)
            {
                float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
                float targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
                float targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

                if (bot->GetExactDist2d(targetX, targetY) > tolerance)
                {
                    return MoveTo(TEMPEST_KEEP_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false,
                                  false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
                }
            }
        }
    }

    return false;
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    const char* advisorNames[] =
    {
        "grand astromancer capernian",
        "master engineer telonicus",
        "lord sanguinar"
    };

    for (const char* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            const time_t now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(kaelthas->GetMap()->GetInstanceId(), now);
            return true;
        }
    }

    return false;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event event)
{
    if (botAI->IsAssistTank(bot))
        SetRtiTarget(botAI, "moon", nullptr);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe
    // But this applies to assist tanks only after they get aggro on the mace, dagger, or sword
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");
    if (axe)
    {
        if (botAI->IsDps(bot) || botAI->IsHeal(bot) ||
            (botAI->IsAssistTank(bot) && (mace && mace->GetVictim() == bot ||
             dagger && dagger->GetVictim() == bot || sword && sword->GetVictim() == bot)))
        {
            const float safeDistance = botAI->IsAssistTank(bot) ? 15.0f : 10.0f;
            float currentDistance = bot->GetExactDist2d(axe);
            if (currentDistance < safeDistance)
                return MoveAway(axe, safeDistance - currentDistance + 1.0f);
        }
    }

    if (botAI->IsDps(bot))
    {
        // Priority 1: Staff of Disintegration (Skull)
        if (Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration"))
        {
            MarkTargetWithSkull(bot, staff);
            SetRtiTarget(botAI, "skull", staff);

            if (bot->GetTarget() != staff->GetGUID())
                return Attack(staff);
        }
        // Priority 2: Cosmic Infuser (Skull)
        else if (mace)
        {
            MarkTargetWithSkull(bot, mace);
            SetRtiTarget(botAI, "skull", mace);

            if (bot->GetTarget() != mace->GetGUID())
                return Attack(mace);
        }
        // Priority 3: Warp Slicer (Skull)
        else if (sword)
        {
            MarkTargetWithSkull(bot, sword);
            SetRtiTarget(botAI, "skull", sword);

            if (bot->GetTarget() != sword->GetGUID())
                return Attack(sword);
        }
        // Priority 4: Infinity Blades (Skull)
        else if (dagger)
        {
            MarkTargetWithSkull(bot, dagger);
            SetRtiTarget(botAI, "skull", dagger);

            if (bot->GetTarget() != dagger->GetGUID())
                return Attack(dagger);
        }
        // Priority 5: Devastation - ranged only (Diamond--marked in other method by main tank)
        else if (axe && botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "diamond", axe);

            if (bot->GetTarget() != axe->GetGUID())
                return Attack(axe);
        }
        // Priority 6: Netherstrand Longbow (Cross--marked in other method by longbow tank)
        else if (Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow"))
        {
            SetRtiTarget(botAI, "cross", longbow);

            if (bot->GetTarget() != longbow->GetGUID())
                return Attack(longbow);
        }
        // Priority 7: Phaseshift Bulwark (Skull)
        else if (Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark"))
        {
            MarkTargetWithSkull(bot, shield);
            SetRtiTarget(botAI, "skull", shield);

            if (bot->GetTarget() != shield->GetGUID())
                return Attack(shield);
        }
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event event)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (!axe)
        return false;

    MarkTargetWithDiamond(bot, axe);
    SetRtiTarget(botAI, "diamond", axe);

    if (bot->GetTarget() != axe->GetGUID())
        return Attack(axe);

    if (axe->GetVictim() == bot)
    {
        const float safeDistance = 12.0f;
        Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, safeDistance);
        if (nearestPlayer)
        {
            float currentDistance = bot->GetExactDist2d(nearestPlayer);
            return MoveFromGroup(safeDistance + 1.0f);
        }
    }

    return false;
}

bool KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction::Execute(Event event)
{
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    if (!longbow)
        return false;

    MarkTargetWithCross(bot, longbow);
    SetRtiTarget(botAI, "cross", longbow);

    if (bot->GetTarget() != longbow->GetGUID())
        return Attack(longbow);

    if (longbow->GetVictim() == bot)
    {
        const float dangerZone = 15.0f;
        Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, dangerZone);
        if (nearestPlayer)
        {
            float currentDistance = bot->GetExactDist2d(nearestPlayer);
            if (currentDistance < dangerZone)
                return MoveFromGroup(dangerZone);
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event event)
{
    struct WeaponInfo
    {
        uint32 npcEntry;
        uint32 itemId;
        const char* name;
    };

    const WeaponInfo weapons[] =
    {
        { NPC_NETHERSTRAND_LONGBOW, ITEM_NETHERSTRAND_LONGBOW, "netherstrand longbow" },
        { NPC_COSMIC_INFUSER, ITEM_COSMIC_INFUSER, "cosmic infuser" },
        { NPC_DEVASTATION, ITEM_DEVASTATION, "devastation" },
        { NPC_INFINITY_BLADES, ITEM_INFINITY_BLADE, "infinity blade" },
        { NPC_WARP_SLICER, ITEM_WARP_SLICER, "warp slicer" },
        { NPC_STAFF_OF_DISINTEGRATION, ITEM_STAFF_OF_DISINTEGRATION, "staff of disintegration" },
        { NPC_PHASESHIFT_BULWARK, ITEM_PHASESHIFT_BULWARK, "phaseshift bulwark" }
    };

    for (auto const& weapon : weapons)
    {
        if (ShouldBotLootWeapon(weapon.npcEntry))
        {
            if (bot->HasItemCount(weapon.itemId, 1, false))
                continue;

            return LootWeapon(weapon.npcEntry, weapon.itemId, weapon.name);
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(uint32 weaponEntry)
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    switch (weaponEntry)
    {
        case NPC_NETHERSTRAND_LONGBOW:
            return bot->getClass() == CLASS_HUNTER;

        case NPC_COSMIC_INFUSER:
            return botAI->IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at appropriate gear levels
        // So they're better off looting only the dagger to MH it and break MCs
        case NPC_DEVASTATION:
            return (bot->getClass() == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) ||
                   (bot->getClass() == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
                   (botAI->IsDps(bot) && bot->getClass() == CLASS_DEATH_KNIGHT);

        case NPC_INFINITY_BLADES:
            return bot->getClass() == CLASS_ROGUE ||
                   bot->getClass() == CLASS_HUNTER ||
                   (bot->getClass() == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT) ||
                   (bot->getClass() == CLASS_WARRIOR && tab != WARRIOR_TAB_ARMS);

        case NPC_WARP_SLICER:
            return bot->getClass() == CLASS_ROGUE && tab != ROGUE_TAB_ASSASSINATION ||
                   (botAI->IsTank(bot) &&
                    (bot->getClass() == CLASS_DEATH_KNIGHT ||
                     bot->getClass() == CLASS_PALADIN));

        case NPC_STAFF_OF_DISINTEGRATION:
            return (botAI->IsRangedDps(bot) && bot->getClass() != CLASS_HUNTER) ||
                   (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL);

        case NPC_PHASESHIFT_BULWARK:
            return botAI->IsTank(bot) &&
                   (bot->getClass() == CLASS_PALADIN ||
                    bot->getClass() == CLASS_WARRIOR ||
                    bot->getClass() == CLASS_DEATH_KNIGHT);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(
    uint32 weaponEntry, uint32 itemId, const char* weaponName)
{
    auto const& corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig->lootDistance;

    for (auto const& guid : corpses)
    {
        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
            continue;

        WorldObject* object = loot.GetWorldObject(bot);
        if (!object)
            continue;

        Creature* creature = object->ToCreature();
        if (!creature)
            continue;

        if (creature->GetEntry() != weaponEntry || creature->IsAlive())
            continue;

        context->GetValue<LootObject>("loot target")->Set(loot);

        float distToObject = bot->GetDistance(object);

        if (distToObject > maxLootRange)
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);

        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());

        if (!opened)
            return opened;

        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        const uint8 weaponIndex = 0;

        botAI->AddTimedEvent([this, botGuid, corpseGuid, weaponIndex, itemId, weaponName]()
        {
            Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver || !receiver->IsInWorld())
                return;

            if (receiver->HasItemCount(itemId, 1, false))
                return;

            receiver->SetLootGUID(corpseGuid);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << weaponIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        botAI->DoSpecificAction("equip upgrades", Event(), true);
        return true;
    }

    return false;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event event)
{
    return UsePhaseshiftBulwark() ||
           UseStaffOfDisintegration() ||
           UseNetherstrandLongbow();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != ITEM_PHASESHIFT_BULWARK)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(SPELL_SHOCK_BARRIER))
        return false;

    if (bot->HasAura(SPELL_ARCANE_BARRIER))
        return false;

    if (bot->CanUseItem(offHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != ITEM_STAFF_OF_DISINTEGRATION)
        return false;

    if (bot->HasAura(SPELL_MENTAL_PROTECTION_FIELD))
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != ITEM_NETHERSTRAND_LONGBOW)
        return false;

    if (bot->HasItemCount(ITEM_NETHER_SPIKES, 401, false))
        return false;

    return UseEquippedItemWithPacket(ranged);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseEquippedItemWithPacket(Item* item)
{
    if (!item)
        return false;

    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    uint32 targetFlag = TARGET_FLAG_UNIT;
    packet << targetFlag << bot->GetPackGUID();

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

bool KaelthasSunstriderMainTankPositionBossAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    MarkTargetWithStar(bot, kaelthas);
    SetRtiTarget(botAI, "star", kaelthas);

    if (bot->GetTarget() != kaelthas->GetGUID())
        return Attack(kaelthas);

    if (kaelthas->GetVictim() == bot && bot->IsWithinMeleeRange(kaelthas))
    {
        const Position& position = KAELTHAS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TEMPEST_KEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event event)
{
    std::vector<Unit*> flameStrikes =
        GetAllHazardTriggers(botAI, bot, NPC_FLAME_STRIKE_TRIGGER, 40.0f);
    if (flameStrikes.empty())
        return false;

    const float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameStrike : flameStrikes)
    {
        if (bot->GetExactDist2d(flameStrike) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Position safestPos = FindSafestNearbyPosition(bot, flameStrikes, 30.0f, hazardRadius);

    botAI->Reset();
    return MoveTo(TEMPEST_KEEP_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::Execute(Event event)
{
    if (botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1))
    {
        std::vector<Unit*> phoenixes;
        auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_PHOENIX && unit->IsAlive())
                phoenixes.push_back(unit);
        }

        if (phoenixes.empty())
            return false;

        std::sort(phoenixes.begin(), phoenixes.end(),
                  [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

        Unit* targetPhoenix = nullptr;
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            targetPhoenix = phoenixes[0];
            MarkTargetWithSquare(bot, targetPhoenix);
            SetRtiTarget(botAI, "square", targetPhoenix);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1) && phoenixes.size() >= 2)
        {
            targetPhoenix = phoenixes[1];
            MarkTargetWithCircle(bot, targetPhoenix);
            SetRtiTarget(botAI, "circle", targetPhoenix);
        }

        if (!targetPhoenix)
            return false;

        if (bot->GetTarget() != targetPhoenix->GetGUID())
            return Attack(targetPhoenix);

        if (targetPhoenix->GetVictim() == bot)
        {
            const float safeDistance = 10.0f;

            Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
            if (Group* group = bot->GetGroup())
            {
                if (nearestPlayer && group)
                {
                    PlayerbotAI* nearestAI = GET_PLAYERBOT_AI(nearestPlayer->ToPlayer());
                    if (nearestAI && (nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 0) ||
                                       nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 1)))
                    {
                        nearestPlayer = nullptr;
                    }
                }
            }

            if (nearestPlayer)
            {
                float closestDist = bot->GetExactDist2d(nearestPlayer);
                if (closestDist < safeDistance)
                    return MoveFromGroup(safeDistance + 2.0f);
            }
        }

        return false;
    }
    else if (!botAI->IsTank(bot))
    {
        Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix");
        if (!phoenix)
            return false;

        float currentDistance = bot->GetExactDist2d(phoenix);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance + 2.0f);
    }
    else if (botAI->IsRangedDps(bot))
    {
        Unit* phoenixEgg = AI_VALUE2(Unit*, "find target", "phoenix egg");
        if (!phoenixEgg)
            return false;

        MarkTargetWithDiamond(bot, phoenixEgg);
        SetRtiTarget(botAI, "diamond", phoenixEgg);

        if (bot->GetTarget() != phoenixEgg->GetGUID())
            return Attack (phoenixEgg);
    }

    return false;
}

bool KaelthasSunstriderBreakMindControlAction::Execute(Event event)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot)
                continue;

            if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
            {
                float dist = bot->GetExactDist2d(member);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    mcTarget = member;
                }
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        uint32 delay = urand(1500, 2500); // 1.5 to 2.5 seconds
        float x = mcTarget->GetPositionX();
        float y = mcTarget->GetPositionY();
        float z = mcTarget->GetPositionZ();
        botAI->AddTimedEvent(
            [this, x, y, z]() {
                MoveTo(TEMPEST_KEEP_MAP_ID, x, y, z, false, false, false, false,
                       MovementPriority::MOVEMENT_COMBAT, true, false);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    static const std::array<const char*, 4> spells =
    {
        "hamstring",
        "wing clip",
        "shiv",
        "stormstrike"
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, mcTarget))
            return botAI->CastSpell(spell, mcTarget);
    }

    return false;
}

// Shock Barrier needs to be #1 focus, even if there is a Phoenix Egg up
bool KaelthasSunstriderBreakThroughShockBarrierAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(SPELL_SHOCK_BARRIER))
    {
        static const std::array<const char*, 8> spells =
        {
            "bash",
            "counterspell",
            "kick",
            "mind freeze",
            "pummel",
            "shield bash",
            "silencing shot",
            "wind shear",
        };

        for (const char* spell : spells)
        {
            if (botAI->CanCastSpell(spell, kaelthas))
                return botAI->CastSpell(spell, kaelthas);
        }
    }
    else if (bot->GetTarget() != kaelthas->GetGUID())
    {
        SetRtiTarget(botAI, "star", kaelthas);
        return Attack(kaelthas);
    }

    return false;
}

// Bots immediately fall to the ground after Gravity Lapse, so this action
// name is kind of a misnomer (though bots are still in a flying state)
bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const float minSpreadDistance = 16.0f;

    std::vector<Player*> nearbyPlayers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (bot->IsWithinDist3d(member, minSpreadDistance * 1.0f))
            nearbyPlayers.push_back(member);
    }

    if (nearbyPlayers.empty())
        return false;

    Player* closestPlayer = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (Player* player : nearbyPlayers)
    {
        float distToPlayer = bot->GetExactDist(player);
        if (distToPlayer < closestDist)
        {
            closestDist = distToPlayer;
            closestPlayer = player;
        }
    }

    if (closestPlayer && closestDist < minSpreadDistance)
    {
        float angle = bot->GetAngle(closestPlayer) + M_PI;
        float distance = minSpreadDistance - closestDist;

        float x = bot->GetPositionX() + std::cos(angle) * distance;
        float y = bot->GetPositionY() + std::sin(angle) * distance;

        return MoveTo(TEMPEST_KEEP_MAP_ID, x, y, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
