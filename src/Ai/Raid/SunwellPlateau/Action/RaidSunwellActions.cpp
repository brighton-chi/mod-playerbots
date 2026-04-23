/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellActions.h"
#include "RaidSunwellHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TargetValue.h"
#include "Timer.h"

using namespace SunwellHelpers;

// General

bool SunwellPlateauEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    uint32 instanceId = bot->GetInstanceId();
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");

    bool erased = false;

    // Kalecgos

    if (!kalecgos && botAI->IsRanged(bot) &&
        hasReachedKalecgosInitialRangedPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!kalecgos && !sathrovarr)
    {
        bool isInKalecgosRealmTransitionGrace = false;
        if (!bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)))
        {
            auto realmStateItr = kalecgosRealmStates.find(guid);
            if (realmStateItr != kalecgosRealmStates.end())
            {
                uint32 now = getMSTime();
                constexpr uint32 realmTransitionGraceMs = 2000;
                if ((realmStateItr->second.lastEnterMs &&
                     getMSTimeDiff(realmStateItr->second.lastEnterMs, now) < realmTransitionGraceMs) ||
                    (realmStateItr->second.lastExitMs &&
                     getMSTimeDiff(realmStateItr->second.lastExitMs, now) < realmTransitionGraceMs))
                {
                    isInKalecgosRealmTransitionGrace = true;
                }
            }
        }

        if (!isInKalecgosRealmTransitionGrace)
        {
            if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
                kalecgosEncounterStates.erase(instanceId) > 0)
            {
                erased = true;
            }

            if (kalecgosRealmStates.erase(guid) > 0)
                erased = true;
        }
    }

    // Brutallus

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
    {
        if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
            bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_BURN));

        if (botAI->IsRanged(bot) &&
            brutallusRangedBurnStates.erase(guid) > 0)
        {
            erased = true;
        }

        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            brutallusRangedAssignments.erase(instanceId) > 0)
        {
            erased = true;
        }
    }

    // Felmyst

    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
    {
        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            felmystRangedAssignments.erase(instanceId) > 0)
        {
            erased = true;
        }

        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            felmystFogOfCorruptionStates.erase(instanceId) > 0)
        {
            erased = true;
        }

        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            felmystDemonicVaporPathIndices.erase(instanceId) > 0)
        {
            erased = true;
        }

        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            felmystDemonicVaporWaypointIndices.erase(instanceId) > 0)
        {
            erased = true;
        }
    }

    // Eredar Twins
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED)))
            bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED));

        if (botAI->IsTank(bot) && alythessTankStep.erase(guid) > 0)
            erased = true;
    }

    // M'uru
    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
    {
        if (muruDarknessStates.erase(instanceId) > 0)
        {
            erased = true;
        }
    }

    // Kil'jaeden <The Deceiver>
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden") &&
        !AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
    {
        if (IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID) &&
            kiljaedenArmageddons.erase(instanceId) > 0)
        {
            erased = true;
        }

        ClearKiljaedenRangedArmageddonAssignments(instanceId);
    }

    return erased;
}

// Kalecgos

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    MarkTargetWithSkull(bot, kalecgos);

    const Position& position = KALECGOS_TANK_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                               position.GetPositionY());

    if (kalecgos->GetVictim() && kalecgos->GetVictim() != bot)
    {
        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    if (bot->GetVictim() != kalecgos)
        return Attack(kalecgos);

    if (kalecgos->GetVictim() != bot)
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
    }
    else if (kalecgos->GetVictim() == bot && bot->IsWithinMeleeRange(kalecgos))
    {
        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
        kalecgos && kalecgos->GetVictim() == bot)
    {
        Player* nextTank = GetKalecgosCurrentTank(botAI, bot);
        if (!nextTank || nextTank == bot)
            return false;

        const Position& position = KALECGOS_TANK_POSITION;
        if (nextTank->GetExactDist2d(position.GetPositionX(),
            position.GetPositionY()) > 3.0f || kalecgos->GetVictim() != nextTank)
        {
            return false;
        }
    }

    GameObject* rift = bot->FindNearestGameObject(
        static_cast<uint32>(SunwellObjects::GO_SPECTRAL_RIFT), 50.0f, true);
    if (!rift)
        return false;

    if (bot->GetExactDist2d(rift) < 3.0f)
    {
        rift->Use(bot);
        return true;
    }
    else
    {
        return MoveTo(SUNWELL_MAP_ID, rift->GetPositionX(), rift->GetPositionY(),
                      rift->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

// Sathrovarr's Combat Reach is 4.0f
// Kalecgos' Combat Reach is 10.5f
bool KalecgosDisperseRangedAction::Execute(Event /*event*/)
{
    if (!HasReachedKalecgosInitialRangedPosition(bot))
    {
        const Position& initialPos = KALECGOS_INITIAL_RANGED_POSITION;
        /* float distToInitialPosition = bot->GetExactDist2d(initialPos.GetPositionX(),
                                                          initialPos.GetPositionY());
        if (distToInitialPosition > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, initialPos.GetPositionX(), initialPos.GetPositionY(),
                          initialPos.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        } */
        constexpr float initialRangedRadius = 10.0f;
        return MoveInside(SUNWELL_MAP_ID, initialPos.GetPositionX(), initialPos.GetPositionY(),
                          initialPos.GetPositionZ(), initialRangedRadius,
                          MovementPriority::MOVEMENT_COMBAT);

        SetKalecgosInitialRangedPositionReached(bot, true);
    }

    constexpr float safeDistFromPlayer = 8.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        const float safeDistFromBoss = 20.0f;
        constexpr uint32 minInterval = 0;

        if (bot->GetExactDist2d(kalecgos) < safeDistFromBoss)
            return FleePosition(kalecgos->GetPosition(), safeDistFromBoss, minInterval);
    }

    return false;
}

bool KalecgosDetermineBossToAttackAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    if (Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        target = sathrovarr;
        MarkTargetWithStar(bot, sathrovarr);
        SetRtiTarget(botAI, "star", sathrovarr);
    }
    else if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        target = kalecgos;
        MarkTargetWithDiamond(bot, kalecgos);
        SetRtiTarget(botAI, "diamond", kalecgos);
    }
    else
        return false;

    if (bot->GetTarget() != target->GetGUID())
        return Attack(target);

    return false;
}

// Brutallus
// Note: BoundingRadius of 6f, CombatReach of 18f

bool BrutallusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", brutallus))
    {
        return botAI->CastSpell("steady shot", brutallus);
    }

    return false;
}

bool BrutallusTanksHandleBossAction::Execute(Event event)
{
    Unit* brutallus = nullptr;
    Player* mainTank = nullptr;
    Player* assistTank = nullptr;
    Aura* mainTankAura = nullptr;
    Aura* assistTankAura = nullptr;

    brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (bot->GetVictim() != brutallus)
        return Attack(brutallus);

    mainTank = GetGroupMainTank(botAI, bot);
    assistTank = GetGroupAssistTank(botAI, bot, 0);

    if (mainTank && mainTank->IsAlive())
    {
        mainTankAura = mainTank->GetAura(
            static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH));
    }

    if (assistTank && assistTank->IsAlive())
    {
        assistTankAura = assistTank->GetAura(
            static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH));
    }

    bool isMainTank = botAI->IsMainTank(bot);

    if (!isMainTank)
    {
        bool shouldTaunt = (!mainTank || !mainTank->IsAlive()) ||
                           (mainTankAura && mainTankAura->GetStackAmount() >= 3);

        if (brutallus->GetVictim() == bot)
            return false;

        if (shouldTaunt)
            return botAI->DoSpecificAction("taunt spell", event, true);

        Position position = GetBrutallusTankPosition(brutallus, false, bot->GetPositionZ());
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        return false;
    }

    Position position =
        GetBrutallusTankPosition(brutallus, isMainTank, bot->GetPositionZ());
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (brutallus->GetVictim() == bot && distToPosition <= 3.0f)
        return false;

    if (distToPosition > 3.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    if (isMainTank &&
        ((!assistTank || !assistTank->IsAlive()) ||
         (assistTankAura && assistTankAura->GetStackAmount() >= 3) ||
         (!assistTankAura && !mainTankAura)))
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
    }

    return false;
}

bool BrutallusPositionMeleeAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    uint8 meleeIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, false, meleeIndex))
        return false;

    Position position;
    if (!TryGetBrutallusMeleePosition(
            bot, brutallus, meleeIndex, bot->GetPositionZ(), position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool BrutallusPositionRangedAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");
    else if (bot->getClass() == CLASS_MAGE && botAI->HasAura("ice block", bot))
             botAI->RemoveAura("ice block");

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    const ObjectGuid guid = bot->GetGUID();
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    auto burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToFrontStep)
    {
        burnState = BrutallusRangedBurnState::ReturningToNormal;
        brutallusRangedBurnStates[guid] = burnState;
    }
    else if (burnState == BrutallusRangedBurnState::MovingToMirrorStep)
    {
        burnState = BrutallusRangedBurnState::ReturningToFrontStep;
        brutallusRangedBurnStates[guid] = burnState;
    }
    else if (burnState == BrutallusRangedBurnState::MovingToRearFinal ||
             burnState == BrutallusRangedBurnState::AtRearFinal)
    {
        burnState = BrutallusRangedBurnState::ReturningToMirrorStep;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToMirrorStep)
    {
        Position position;
        if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, true, BRUTALLUS_RETURN_RANGED_RADIUS,
                bot->GetPositionZ(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::ReturningToFrontStep;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToFrontStep)
    {
        Position position;
        if (!TryGetBrutallusRangedArcPosition(
            brutallus, rangedIndex, BRUTALLUS_RETURN_RANGED_RADIUS, false,
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        Position returnStepPosition;
        if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, false, BRUTALLUS_RETURN_RANGED_RADIUS,
            bot->GetPositionZ(), returnStepPosition))
            return false;

        if (bot->GetExactDist2d(returnStepPosition.GetPositionX(),
                                returnStepPosition.GetPositionY()) <= 1.0f)
        {
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::ReturningToNormal;
        }

        return false;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToNormal)
    {
        Position position;
        if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, false, BRUTALLUS_NORMAL_RANGED_RADIUS,
                bot->GetPositionZ(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates.erase(guid);
        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, false, BRUTALLUS_NORMAL_RANGED_RADIUS,
            bot->GetPositionZ(), position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool BrutallusHandleBurnAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (RemoveBurnWithCooldown(bot))
        return true;

    if (botAI->IsMelee(bot) || !ShouldMoveForBrutallusBurn(bot))
        return false;

    const ObjectGuid guid = bot->GetGUID();
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    auto burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::ReturningToNormal)
    {
        burnState = BrutallusRangedBurnState::MovingToFrontStep;
        brutallusRangedBurnStates[guid] = burnState;
    }
    else if (burnState == BrutallusRangedBurnState::ReturningToFrontStep)
    {
        burnState = BrutallusRangedBurnState::MovingToMirrorStep;
        brutallusRangedBurnStates[guid] = burnState;
    }
    else if (burnState == BrutallusRangedBurnState::ReturningToMirrorStep)
    {
        burnState = BrutallusRangedBurnState::MovingToRearFinal;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::None)
    {
        burnState = BrutallusRangedBurnState::MovingToFrontStep;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToFrontStep)
    {
        Position stepPosition;
        if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, false, BRUTALLUS_BURN_RANGED_RADIUS,
                bot->GetPositionZ(), stepPosition))
            return false;

        if (bot->GetExactDist2d(stepPosition.GetPositionX(), stepPosition.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                          stepPosition.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToMirrorStep;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::MovingToMirrorStep)
    {
        Position position;
        if (!TryGetBrutallusRangedArcPosition(
            brutallus, rangedIndex, BRUTALLUS_BURN_RANGED_RADIUS, true,
            bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        Position mirrorStepPosition;
        if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, true, BRUTALLUS_BURN_RANGED_RADIUS,
                bot->GetPositionZ(), mirrorStepPosition))
        {
            return false;
        }

        if (bot->GetExactDist2d(mirrorStepPosition.GetPositionX(),
                                mirrorStepPosition.GetPositionY()) <= 1.0f)
        {
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToRearFinal;
        }

        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedStepPosition(
            brutallus, rangedIndex, true, BRUTALLUS_BURN_RANGED_RADIUS,
            bot->GetPositionZ(), position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::AtRearFinal;

    return false;
}

bool BrutallusHandleBurnAction::RemoveBurnWithCooldown(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

// Felmyst

bool FelmystMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", felmyst))
    {
        return botAI->CastSpell("steady shot", felmyst);
    }

    return false;
}

bool FelmystMainTankPositionBossOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (bot->GetVictim() != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() == bot && bot->GetHealthPct() > 50.0f)
    {
        const Position& position = FELMYST_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool FelmystPositionRangedOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");
    else if (bot->getClass() == CLASS_MAGE && botAI->HasAura("ice block", bot))
             botAI->RemoveAura("ice block");

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(botAI, bot, felmyst, position))
        return false;

    constexpr float rangedGroupRadius = 3.0f;
    return MoveInside(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), rangedGroupRadius,
                      MovementPriority::MOVEMENT_COMBAT);
}

bool FelmystPositionMeleeOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    float desiredDist = bot->GetMeleeRange(felmyst);
    float behindAngle = Position::NormalizeOrientation(felmyst->GetOrientation() + M_PI);
    float targetX = felmyst->GetPositionX() + desiredDist * std::cos(behindAngle);
    float targetY = felmyst->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
    {
        return MoveTo(SUNWELL_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    float distToEncapsulated = bot->GetDistance2d(encapsulateTarget);

    if (distToEncapsulated > FELMYST_ENCAPSULATE_SAFE_DISTANCE)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    EnsureFelmystRangedAssignments(botAI, bot);

    bool isEncapsulateTargetInRangedGroup = false;
    auto instanceItr = felmystRangedAssignments.find(bot->GetInstanceId());
    if (instanceItr != felmystRangedAssignments.end())
    {
        isEncapsulateTargetInRangedGroup = instanceItr->second.find(
            encapsulateTarget->GetGUID()) != instanceItr->second.end();
    }

    float targetX = 0.0f;
    float targetY = 0.0f;
    if (isEncapsulateTargetInRangedGroup)
    {
        float meleeDistance = bot->GetMeleeRange(felmyst);
        float behindAngle = Position::NormalizeOrientation(felmyst->GetOrientation() + M_PI);
        targetX = felmyst->GetPositionX() + meleeDistance * std::cos(behindAngle);
        targetY = felmyst->GetPositionY() + meleeDistance * std::sin(behindAngle);
    }
    else
    {
        constexpr float sideDistance = 22.0f;
        float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);
        float leftAngle = frontAngle + M_PI_2;
        float rightAngle = frontAngle - M_PI_2;

        Position leftPosition;
        leftPosition.Relocate(
            felmyst->GetPositionX() + std::cos(leftAngle) * sideDistance,
            felmyst->GetPositionY() + std::sin(leftAngle) * sideDistance,
            bot->GetPositionZ());

        Position rightPosition;
        rightPosition.Relocate(
            felmyst->GetPositionX() + std::cos(rightAngle) * sideDistance,
            felmyst->GetPositionY() + std::sin(rightAngle) * sideDistance,
            bot->GetPositionZ());

        float leftDistance = bot->GetExactDist2d(
            leftPosition.GetPositionX(), leftPosition.GetPositionY());
        float rightDistance = bot->GetExactDist2d(
            rightPosition.GetPositionX(), rightPosition.GetPositionY());
        Position const& destination = leftDistance <= rightDistance ? leftPosition : rightPosition;

        targetX = destination.GetPositionX();
        targetY = destination.GetPositionY();
    }

    return MoveTo(SUNWELL_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystCastMassDispelOnGasNovaAction::Execute(Event /*event*/)
{
    Player* gasNovaTarget = GetFelmystGasNovaDispelTarget(bot);
    if (!gasNovaTarget)
        return false;

    if (botAI->CanCastSpell("mass dispel", gasNovaTarget))
        return botAI->CastSpell("mass dispel", gasNovaTarget);

    return false;
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    Unit* hazard = GetNearestFelmystDemonicVaporHazard(bot);
    if (hazard)
    {
        constexpr float safeDistFromVapor = 10.0f;
        constexpr uint32 minInterval = 0;
        float currentDistance = bot->GetDistance2d(hazard);
        if (currentDistance < safeDistFromVapor)
        {
            // return FleePosition(
            //     hazard->GetPosition(), safeDistFromVapor, minInterval);
            return MoveAway(hazard, safeDistFromVapor - currentDistance);
        }
    }

    return false;
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    return MoveTo(SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                  destination.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystAvoidFogOfCorruptionAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    FelmystFogOfCorruptionState fogState;
    if (!TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState))
        return false;

    std::array<Position, 3> destinations;
    uint8 destinationCount = 0;
    if (!TryGetFelmystFogSafeDestinations(bot, fogState.lane, destinations, destinationCount))
        return false;

    for (uint8 index = 0; index < destinationCount; ++index)
    {
        Position const& destination = destinations[index];
        if (MoveTo(SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                   destination.GetPositionZ(), false, false, false, false,
                   MovementPriority::MOVEMENT_FORCED, true, false))
        {
            return true;
        }
    }

    return false;
}

// Eredar Twins

bool EredarTwinsMeleeJumpDownFromBalconyAction::Execute(Event /*event*/)
{
    const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
    const Position& landingPos = EREDAR_TWINS_P2_MELEE_STACK_POSITION;

    constexpr float arrivalDistance = 2.0f;
    float distanceToJumpPos =
        bot->GetExactDist2d(jumpPos.GetPositionX(), jumpPos.GetPositionY());

    if (distanceToJumpPos > arrivalDistance)
    {
        return MoveTo(SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
                      jumpPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        return JumpTo(SUNWELL_MAP_ID, landingPos.GetPositionX(),
                      landingPos.GetPositionY(), landingPos.GetPositionZ(),
                      MovementPriority::MOVEMENT_FORCED);
    }
}

bool EredarTwinsMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

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

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        tankTarget = GetGroupAssistTank(botAI, bot, 0);
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupMainTank(botAI, bot);
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupAssistTank(botAI, bot, 1);
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

bool EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    MarkTargetWithStar(bot, sacrolash);
    SetRtiTarget(botAI, "star", sacrolash);

    if (bot->GetVictim() != sacrolash)
        return Attack(sacrolash);

    if (sacrolash->GetVictim() == bot && bot->IsWithinMeleeRange(sacrolash))
    {
        const Position& position = SACROLASH_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool EredarTwinsFirstAssistTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    MarkTargetWithCircle(bot, alythess);
    SetRtiTarget(botAI, "circle", alythess);

    if (bot->GetVictim() != alythess)
        return Attack(alythess);

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = alythessTankStep.count(guid) ? alythessTankStep[guid] : 0;
    if (index >= ALYTHESS_TANK_POSITIONS.size())
        index = 0;

    auto findSafeAlythessTankIndex = [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        size_t const offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITIONS.size(); ++offset)
        {
            uint8 candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITIONS.size());
            if (IsAlythessTankPositionSafe(bot, ALYTHESS_TANK_POSITIONS[candidateIndex]))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    if (!IsAlythessTankPositionSafe(bot, ALYTHESS_TANK_POSITIONS[index]))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        alythessTankStep[guid] = index;
    }

    const Position& position = ALYTHESS_TANK_POSITIONS[index];

    constexpr float maxDistance = 1.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (alythess->GetVictim() == bot)
    {
        if (distToPosition <= maxDistance &&
            ShouldAdvanceAlythessTankPosition(alythess, bot))
        {
            uint8 safeIndex = index;
            if (!findSafeAlythessTankIndex(index, false, safeIndex))
                return false;

            index = safeIndex;
            alythessTankStep[guid] = index;
            const Position& newPosition = ALYTHESS_TANK_POSITIONS[index];
            float newDistToPosition = bot->GetExactDist2d(newPosition);
            if (newDistToPosition > maxDistance)
            {
                float dX = newPosition.GetPositionX() - bot->GetPositionX();
                float dY = newPosition.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, newDistToPosition);
                float moveX = bot->GetPositionX() + (dX / newDistToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / newDistToPosition) * moveDist;

                return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                              true, false);
            }
        }
        else if (distToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                          true, false);
        }
    }

    return false;
}

bool EredarTwinsPositionRangedAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");
    else if (bot->getClass() == CLASS_MAGE && botAI->HasAura("ice block", bot))
             botAI->RemoveAura("ice block");

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash)
    {
        const Position& position = EREDAR_TWINS_P1_RANGED_POSITION;

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }
    else if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
        const Position& landingPos = EREDAR_TWINS_P2_RANGED_STACK_POSITION;

        constexpr float arrivalDistance = 2.0f;
        float distanceToJumpPos =
            bot->GetExactDist2d(jumpPos.GetPositionX(), jumpPos.GetPositionY());

        if (distanceToJumpPos > arrivalDistance)
        {
            return MoveTo(SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
                          jumpPos.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            return JumpTo(SUNWELL_MAP_ID, landingPos.GetPositionX(),
                          landingPos.GetPositionY(), landingPos.GetPositionZ(),
                          MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    const Position& position = botAI->IsRanged(bot) ?
        EREDAR_TWINS_P2_RANGED_STACK_POSITION : EREDAR_TWINS_P2_MELEE_STACK_POSITION;

    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 0.5f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool EredarTwinsRemoveFlameSearAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool EredarTwinsDpsPrioritizeLadySacrolashAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PALADIN && botAI->HasAura("divine shield", bot))
        botAI->RemoveAura("divine shield");

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    if (sacrolash)
        SetRtiTarget(botAI, "star", sacrolash);

    if (sacrolash && ShouldHoldSacrolashThreat(botAI, bot, alythess, sacrolash))
    {
        if (bot->GetVictim() == sacrolash || bot->GetTarget() == sacrolash->GetGUID())
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
            return true;
        }

        return false;
    }

    if (sacrolash)
    {
        if (bot->GetTarget() != sacrolash->GetGUID())
            return Attack(sacrolash);

        return false;
    }

    if (alythess)
    {
        SetRtiTarget(botAI, "circle", alythess);

        if (bot->GetTarget() != alythess->GetGUID())
            return Attack(alythess);

        return false;
    }

    return false;
}

bool EredarTwinsConflagratedBotMoveFromGroupAction::Execute(Event /*event*/)
{
    const Position& position = botAI->IsRanged(bot) ?
        EREDAR_TWINS_RANGED_CONFLAG_POSITION : EREDAR_TWINS_MELEE_CONFLAG_POSITION;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// M'uru

bool MuruMisdirectEnemiesToTanksAction::Execute(Event /*event*/)
{
    Unit* targetEnemy = nullptr;
    Unit* targetTank = nullptr;
    if (Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        targetEnemy = voidSentinel;
        if (Player* mainTank = GetGroupMainTank(botAI, bot))
            targetTank = mainTank;
    }
    else if (Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius"))
    {
        targetEnemy = entropius;
        if (Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0))
            targetTank = firstAssistTank;
    }

    if (!targetEnemy || !targetTank)
        return false;

    if (botAI->CanCastSpell("misdirection", targetTank))
        return botAI->CastSpell("misdirection", targetTank);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", targetEnemy))
    {
        return botAI->CastSpell("steady shot", targetEnemy);
    }

    return false;
}

bool MuruPositionRangedAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (muru && muru->GetHealth() > 1)
    {
        const Position& position = MURU_STACK_POSITION;
        constexpr float rangedGroupRadius = 3.0f;
        return MoveInside(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), rangedGroupRadius,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    else
    {
        constexpr float safeDistFromPlayer = 4.0f;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
    }

    return false;
}

bool MuruKillDarkFiendsWithDispelAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!muru && !entropius)
        return false;

    const bool isMuruPhase = muru && muru->GetHealth() > 1;

    Creature* darkFiendNearMuru = nullptr;
    constexpr float searchRadius = 50.0f;
    std::list<Creature*> darkFiends;
    bot->GetCreatureListWithEntryInGrid(
        darkFiends, static_cast<uint32>(SunwellNpcs::NPC_DARK_FIEND), searchRadius);

    if (isMuruPhase)
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && creature->IsAlive() &&
                creature->GetExactDist2d(muru) < 15.0f)
            {
                darkFiendNearMuru = creature;
                break;
            }
        }
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (isMuruPhase)
        {
            if (darkFiendNearMuru && botAI->CanCastSpell("mass dispel", muru))
                return botAI->CastSpell("mass dispel", muru);

            for (Creature* creature : darkFiends)
            {
                if (creature && botAI->CanCastSpell("mass dispel", creature) &&
                    botAI->CastSpell("mass dispel", creature))
                {
                    return true;
                }
            }
        }

        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell("dispel magic", creature))
                return botAI->CastSpell("dispel magic", creature);
        }
    }
    else
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell("purge", creature))
                return botAI->CastSpell("purge", creature);
        }
    }

    return false;
}

bool MuruFirstAssistTankHandleVoidSentinelAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (!voidSentinel)
        return false;

    if (botAI->IsAssistTankOfIndex(bot, 0, true) && bot->GetVictim() != voidSentinel)
        return Attack(voidSentinel);

    if (voidSentinel->GetVictim() == bot && bot->IsWithinMeleeRange(voidSentinel))
    {
        const Position* position = GetClosestVoidSentinelTankPosition(voidSentinel, bot);
        if (!position)
            return false;

        float distToPosition = bot->GetExactDist2d(position->GetPositionX(),
                                                   position->GetPositionY());
        if (distToPosition > 3.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position->GetPositionX(), position->GetPositionY(),
                          position->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool MuruSetGroundingTotemInFirstAssistTankGroupAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell("grounding totem", bot) &&
           botAI->CastSpell("grounding totem", bot);
}

bool MuruAvoidDarknessAction::Execute(Event /*event*/)
{
    constexpr float stackArrivalDistance = 4.0f;
    return MoveInside(SUNWELL_MAP_ID, MURU_STACK_POSITION.GetPositionX(),
                      MURU_STACK_POSITION.GetPositionY(), MURU_STACK_POSITION.GetPositionZ(),
                      stackArrivalDistance, MovementPriority::MOVEMENT_COMBAT);
}

bool MuruAvoidSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = GetNearestMuruSingularity(bot);
    if (!singularity)
        return false;

    float safeDistance = entropius->GetVictim() == bot ? 15.0f : 10.0f;
    float currentDistance = bot->GetExactDist2d(singularity);
    if (currentDistance >= safeDistance)
        return false;

    return FleePosition(singularity->GetPosition(), safeDistance);
}

bool MuruCastStunOnShadowswordBerserkerAction::Execute(Event /*event*/)
{
    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    if (!berserker)
        return false;

    auto castStop = [&](const char* spell)
    {
        return botAI->CanCastSpell(spell, berserker) &&
               botAI->CastSpell(spell, berserker);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return castStop("bash") || castStop("maim");

        case CLASS_PALADIN:
            return castStop("hammer of justice");

        case CLASS_ROGUE:
            return castStop("kidney shot");

        case CLASS_WARLOCK:
            return castStop("shadowfury");

        case CLASS_WARRIOR:
            return castStop("concussion blow") || castStop("revenge stun") ||
                   castStop("shockwave");

        default:
            return bot->getRace() == RACE_TAUREN && castStop("war stomp");
    }
}

bool MuruInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    if (!furyMage)
        return false;

    auto castStop = [&](const char* spell)
    {
        return botAI->CanCastSpell(spell, furyMage) &&
               botAI->CastSpell(spell, furyMage);
    };

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return castStop("mind freeze") || castStop("strangulate");

        case CLASS_HUNTER:
            return castStop("silencing shot");

        case CLASS_MAGE:
            return castStop("counterspell");

        case CLASS_ROGUE:
            return castStop("kick");

        case CLASS_SHAMAN:
            return castStop("wind shear");

        case CLASS_WARRIOR:
            return castStop("pummel") || castStop("shield bash");

        default:
            return bot->getRace() == RACE_BLOODELF && castStop("arcane torrent");
    }
}

bool MuruCastSpellStealOnSpellFuryAction::Execute(Event /*event*/)
{
    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage && botAI->CanCastSpell("spellsteal", furyMage) &&
           botAI->CastSpell("spellsteal", furyMage);
}

bool MuruWarlockEnslaveVoidSpawnAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_WARLOCK || bot->GetCharm())
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");

    Creature* voidSpawn = FindAvailableVoidSpawnForEnslave(botAI, bot, muru, entropius);
    if (!voidSpawn)
        return false;

    if (botAI->CanCastSpell("enslave demon", voidSpawn))
        return botAI->CastSpell("enslave demon", voidSpawn);

    return false;
}

bool MuruEnslavedVoidSpawnCastShadowBoltVolleyAction::Execute(Event /*event*/)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");

    Unit* voidSpawn = bot->GetCharm();
    if (!voidSpawn || !voidSpawn->IsAlive() ||
        voidSpawn->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN))
    {
        return false;
    }

    Unit* target = GetVoidSpawnVolleyPriorityTarget(botAI, bot, muru, entropius);
    if (!target)
        return false;

    bool commandedAttack = CommandControlledCreatureToAttack(voidSpawn, target);

    if (voidSpawn->GetExactDist2d(target) > sPlayerbotAIConfig.spellDistance)
        return commandedAttack;

    if (voidSpawn->HasSpellCooldown(
            static_cast<uint32>(SunwellSpells::SPELL_SHADOW_BOLT_VOLLEY)))
    {
        return commandedAttack;
    }

    constexpr uint32 globalCooldown = 1000;
    voidSpawn->CastSpell(
        target, static_cast<uint32>(SunwellSpells::SPELL_SHADOW_BOLT_VOLLEY), true);
    voidSpawn->AddSpellCooldown(
        static_cast<uint32>(SunwellSpells::SPELL_SHADOW_BOLT_VOLLEY), 0, globalCooldown);

    return true;
}

bool MuruSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!muru && !entropius)
        return false;

    Unit* target = nullptr;

    const bool isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    const bool isOtherRanged = botAI->IsRanged(bot) && !isShadowPriest;
    const bool isMeleeDps = botAI->IsMelee(bot) && !isShadowPriest && !isOtherRanged;
    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    Unit* currentVictim = bot->GetVictim();

    MuruEncounterTargets targets;
    targets.muru = muru;
    targets.entropius = entropius;
    GatherMuruEncounterTargets(botAI, targets);
    muru = targets.muru;
    entropius = targets.entropius;

    constexpr float targetSwitchDistance = 10.0f;
    auto chooseNearestTarget = [&](Unit*& current, Unit* candidate)
    {
        if (!candidate)
            return;

        if (!current)
        {
            current = candidate;
            return;
        }

        if (current == candidate)
            return;

        float currentDistance = bot->GetExactDist2d(current);
        float candidateDistance = bot->GetExactDist2d(candidate);
        if (candidateDistance + targetSwitchDistance < currentDistance)
            current = candidate;
    };

    auto selectEncounterTarget = [&](uint32 entry, std::vector<Unit*> const& candidates)
    {
        Unit* selected = nullptr;
        if (isMeleeDps && currentVictim && currentVictim->IsAlive() && currentVictim->GetEntry() == entry)
            selected = currentVictim;
        else if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
            selected = currentTarget;

        for (Unit* candidate : candidates)
            chooseNearestTarget(selected, candidate);

        return selected;
    };

    Unit* voidSentinel = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);
    Unit* voidSpawn = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), targets.voidSpawns);
    Unit* furyMage = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
    Unit* berserker = selectEncounterTarget(
        static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);
    bool voidSentinelHasTankAggro = DoesMuruUnitHaveTankAggro(botAI, voidSentinel);

    struct PriorityTarget
    {
        uint32 entry;
        Unit* unit;
    };

    std::vector<PriorityTarget> priorityTargets;
    if (isShadowPriest)
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else if (isOtherRanged)
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), voidSpawn },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius }
        };
    }
    else
    {
        priorityTargets = {
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { static_cast<uint32>(SunwellNpcs::NPC_MURU), muru },
            { static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS), entropius }
        };
    }

    auto isAllowedPriorityTarget = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;

        switch (unit->GetEntry())
        {
            case static_cast<uint32>(SunwellNpcs::NPC_MURU):
                return unit->GetHealth() > 1 &&
                       !botAI->IsTargetValueExcluded(TargetValueExclusionType::Dps, unit->GetGUID());

            case static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS):
                return true;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL):
                return (isShadowPriest || isOtherRanged) && voidSentinelHasTankAggro;

            case static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN):
                return isOtherRanged;

            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE):
            case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER):
                return !isShadowPriest;

            default:
                return false;
        }
    };

    auto getPriorityIndex = [&](Unit* unit) -> size_t
    {
        if (!isAllowedPriorityTarget(unit))
            return priorityTargets.size();

        for (size_t index = 0; index < priorityTargets.size(); ++index)
        {
            if (priorityTargets[index].entry == unit->GetEntry())
                return index;
        }

        return priorityTargets.size();
    };

    if (currentTarget && !isAllowedPriorityTarget(currentTarget))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
        currentTarget = nullptr;
    }

    for (PriorityTarget const& candidate : priorityTargets)
    {
        if (isAllowedPriorityTarget(candidate.unit))
        {
            target = candidate.unit;
            break;
        }
    }

    Unit* stickyTarget = currentTarget;
    if (isMeleeDps && currentVictim && isAllowedPriorityTarget(currentVictim))
        stickyTarget = currentVictim;

    if (stickyTarget)
    {
        size_t currentPriority = getPriorityIndex(stickyTarget);
        size_t desiredPriority = getPriorityIndex(target);
        if (currentPriority <= desiredPriority)
            target = stickyTarget;
    }

    if (target && target->GetEntry() ==
            static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER))
    {
        if (bot->getClass() == CLASS_ROGUE && !botAI->GetAura("dismantle", target) &&
            botAI->CanCastSpell("dismantle", target))
        {
            return botAI->CastSpell("dismantle", target);
        }

        if (bot->getClass() == CLASS_WARRIOR && !botAI->GetAura("disarm", target) &&
            botAI->CanCastSpell("disarm", target))
        {
            return botAI->CastSpell("disarm", target);
        }
    }

    if (target)
    {
        bool needsAttack = false;
        if (isMeleeDps)
            needsAttack = bot->GetVictim() != target;
        else
            needsAttack = currentTarget != target && bot->GetTarget() != target->GetGUID();

        if (needsAttack)
            return Attack(target);
    }

    return false;
}

// Kil'jaeden <The Deceiver>

bool KiljaedenTanksHandleHandsOfTheDeceiverAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(botAI, bot, 1);
    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    std::vector<Unit*> hands;
    Unit* volatileFelfireFiend = nullptr;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_HAND_OF_THE_DECEIVER))
        {
            hands.push_back(unit);
            continue;
        }

        if (!volatileFelfireFiend &&
            unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_VOLATILE_FELFIRE_FIEND))
        {
            volatileFelfireFiend = unit;
        }
    }

    if (hands.empty())
        return false;

    if (volatileFelfireFiend)
        MarkTargetWithSkull(bot, volatileFelfireFiend);

    std::array<Player*, 3> tanks = { mainTank, firstAssistTank, secondAssistTank };
    size_t assignedCount = hands.size() < tanks.size() ? hands.size() : tanks.size();

    for (size_t index = 0; index < assignedCount; ++index)
    {
        if (bot != tanks[index])
            continue;

        Unit* hand = hands[index];
        switch (index)
        {
            case 0:
                MarkTargetWithStar(bot, hand);
                break;
            case 1:
                MarkTargetWithCircle(bot, hand);
                break;
            case 2:
                MarkTargetWithDiamond(bot, hand);
                break;
            default:
                break;
        }

        if (bot->GetVictim() != hand)
            return Attack(hand);

        return false;
    }

    return false;
}

bool KiljaedenAvoidArmageddonsAction::Execute(Event /*event*/)
{
    if (botAI->IsRanged(bot))
        return false;

    KiljaedenArmageddon armageddon;
    if (!TryGetKiljaedenNearestArmageddon(bot, armageddon))
        return false;

    constexpr uint32 minInterval = 0;
    // botAI->InterruptSpell();

    if (FleePosition(armageddon.destination, armageddon.safeDistance, minInterval))
        return true;

    constexpr float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
    if (nearestPlayer)
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

bool KiljaedenStackForShieldOfTheBlueAction::Execute(Event /*event*/)
{
    const Position& position = KILJAEDEN_STACK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(),
                            position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool KiljaedenPositionTanksAction::Execute(Event /*event*/)
{
    const Position& position = KILJAEDEN_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(),
                            position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KiljaedenPositionMeleeAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    size_t meleeIndex = 0;
    bool foundAssignment = false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsMelee(member) || member->GetMapId() != SUNWELL_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
        {
            continue;
        }

        if (member == bot)
        {
            foundAssignment = true;
            break;
        }

        ++meleeIndex;
    }

    if (!foundAssignment)
        return false;

    Position const& assignedPosition =
        meleeIndex % 2 == 0 ? KILJAEDEN_S_MELEE_POSITION : KILJAEDEN_E_MELEE_POSITION;
    Position const& swapPosition =
        meleeIndex % 2 == 0 ? KILJAEDEN_E_MELEE_POSITION : KILJAEDEN_S_MELEE_POSITION;

    Position const* targetPosition = &assignedPosition;

    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
    auto armageddonItr = kiljaedenArmageddons.find(bot->GetInstanceId());
    if (armageddonItr != kiljaedenArmageddons.end() && !armageddonItr->second.empty())
    {
        auto isSafePosition = [&](Position const& position)
        {
            for (KiljaedenArmageddon const& armageddon : armageddonItr->second)
            {
                if (position.GetExactDist2d(armageddon.destination.GetPositionX(),
                                            armageddon.destination.GetPositionY()) <
                    armageddon.safeDistance)
                {
                    return false;
                }
            }

            return true;
        };

        bool assignedSafe = isSafePosition(assignedPosition);
        bool swapSafe = isSafePosition(swapPosition);
        if (!assignedSafe)
        {
            if (swapSafe)
                targetPosition = &swapPosition;
            else
                return false;
        }
    }

    if (bot->GetExactDist2d(targetPosition->GetPositionX(), targetPosition->GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(SUNWELL_MAP_ID, targetPosition->GetPositionX(),
                  targetPosition->GetPositionY(), targetPosition->GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                  true, false);
}

bool KiljaedenPositionRangedAction::Execute(Event /*event*/)
{
    Position targetPosition = KILJAEDEN_TANK_POSITION;
    if (!TryGetKiljaedenRangedPosition(botAI, bot, targetPosition))
        return false;

    if (bot->GetExactDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(SUNWELL_MAP_ID, targetPosition.GetPositionX(),
                  targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                  true, false);
}

/* bool KiljaedenSetDpsPriorityAction::Execute(Event /*event)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    bool isMeleeDps = botAI->IsMelee(bot) && botAI->IsDps(bot);
    bool isRangedDps = botAI->IsRangedDps(bot);
    if (!isMeleeDps && !isRangedDps)
        return false;

    bool canHaveSinisterReflections = kiljaeden->GetHealthPct() < 55.0f;
    Unit* closestShieldOrbInRange = nullptr;
    Unit* closestRangedReflectionInRange = nullptr;
    Unit* closestMeleeReflectionInRange = nullptr;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    auto chooseNearestTarget = [&](Unit*& current, Unit* candidate)
    {
        if (!candidate)
            return;

        if (!current)
        {
            current = candidate;
            return;
        }

        float currentDistance = bot->GetExactDist2d(current);
        float candidateDistance = bot->GetExactDist2d(candidate);
        constexpr float targetSwitchDistance = 5.0f;
        if (candidateDistance + targetSwitchDistance < currentDistance)
            current = candidate;
    };

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_SHIELD_ORB))
        {
            if (botAI->IsRanged(bot) && botAI->IsDps(bot) && bot->GetExactDist2d(unit) <= 30.0f)
                chooseNearestTarget(closestShieldOrbInRange, unit);

            continue;
        }

        if (canHaveSinisterReflections &&
            unit->GetEntry() == static_cast<uint32>(SunwellNpcs::NPC_SINISTER_REFLECTION))
        {
            if (botAI->IsRanged(bot) && botAI->IsDps(bot) && bot->GetExactDist2d(unit) <= 30.0f)
                chooseNearestTarget(closestRangedReflectionInRange, unit);

            if (botAI->IsMelee(bot) && botAI->IsDps(bot) && !botAI->IsTank(bot) &&
                bot->IsWithinMeleeRange(unit))
            {
                chooseNearestTarget(closestMeleeReflectionInRange, unit);
            }
        }
    }
    Unit* priorityTarget = kiljaeden;

    if (isRangedDps)
    {
        if (closestShieldOrbInRange)
            priorityTarget = closestShieldOrbInRange;
        else if (closestRangedReflectionInRange)
            priorityTarget = closestRangedReflectionInRange;

        if (bot->GetTarget() != priorityTarget->GetGUID())
            return Attack(priorityTarget);
    }
    else if (isMeleeDps)
    {
        if (closestMeleeReflectionInRange)
            priorityTarget = closestMeleeReflectionInRange;

        if (bot->GetVictim() != priorityTarget)
            return Attack(priorityTarget);
    }


    return false;
} */
