/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellActions.h"
#include "RaidSunwellHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
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

    // Kalecgos & Sathrovarr the Corruptor

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
                constexpr uint32 realmTransitionGraceMs = 2000; // Tested with 3000
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
        if (botAI->IsTank(bot) && alythessTankStep.erase(guid) > 0)
        {
            erased = true;
        }
    }

    return erased;
}

// Kalecgos & Sathrovarr the Corruptor

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    MarkTargetWithSkull(bot, kalecgos);

    if (bot->GetVictim() != kalecgos)
        return Attack(kalecgos);

    if (kalecgos->GetVictim() != bot)
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
    }
    else if (kalecgos->GetVictim() == bot && bot->IsWithinMeleeRange(kalecgos))
    {
        const Position& position = KALECGOS_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                   position.GetPositionY());
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
        if (nextTank->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            return false;
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
        const Position& initialPosition = KALECGOS_INITIAL_RANGED_POSITION;
        float distanceToInitialPosition = bot->GetExactDist2d(initialPosition.GetPositionX(),
                                                              initialPosition.GetPositionY());
        if (distanceToInitialPosition > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, initialPosition.GetPositionX(), initialPosition.GetPositionY(),
                          initialPosition.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        SetKalecgosInitialRangedPositionReached(bot, true);
    }

    constexpr float safeDistFromPlayer = 8.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        const float safeDistFromKalecgos = 20.0f;
        constexpr uint32 minInterval = 0;

        if (bot->GetExactDist2d(kalecgos) < safeDistFromKalecgos)
            return FleePosition(kalecgos->GetPosition(), safeDistFromKalecgos, minInterval);
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
        return botAI->CastSpell("steady shot", brutallus);

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

    Position position = GetBrutallusTankPosition(brutallus, isMainTank, bot->GetPositionZ());
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

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
    if (bot->getClass() == CLASS_PALADIN &&
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD));
    }

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    uint8 meleeIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, false, meleeIndex))
        return false;

    Position position;
    if (!TryGetBrutallusMeleePosition(bot, brutallus, meleeIndex, bot->GetPositionZ(), position))
        return false;

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
    if (bot->getClass() == CLASS_PALADIN &&
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD));
    }
    else if (bot->getClass() == CLASS_MAGE &&
             bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK));
    }

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
        if (!TryGetBrutallusRangedBurnMirrorStepPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
            return false;

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
        if (!TryGetBrutallusRangedBurnArcPosition(
            brutallus, rangedIndex, false, bot->GetPositionX(), bot->GetPositionY(),
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

        Position frontStepPosition;
        if (!TryGetBrutallusRangedBurnStepPosition(brutallus, rangedIndex, bot->GetPositionZ(), frontStepPosition))
            return false;

        if (bot->GetExactDist2d(frontStepPosition.GetPositionX(), frontStepPosition.GetPositionY()) <= 1.0f)
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::ReturningToNormal;

        return false;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToNormal)
    {
        Position position;
        if (!TryGetBrutallusRangedPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
            return false;

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
    if (!TryGetBrutallusRangedPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
        return false;

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
        if (!TryGetBrutallusRangedBurnStepPosition(brutallus, rangedIndex, bot->GetPositionZ(), stepPosition))
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
        if (!TryGetBrutallusRangedBurnArcPosition(
                brutallus, rangedIndex, true, bot->GetPositionX(), bot->GetPositionY(),
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
        if (!TryGetBrutallusRangedBurnMirrorStepPosition(brutallus, rangedIndex, bot->GetPositionZ(), mirrorStepPosition))
            return false;

        if (bot->GetExactDist2d(mirrorStepPosition.GetPositionX(), mirrorStepPosition.GetPositionY()) <= 1.0f)
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToRearFinal;

        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedBurnPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
        return false;

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
    if (bot->getClass() == CLASS_ROGUE &&
        botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_CLOAK_OF_SHADOWS), bot))
    {
        return true;
    }
    else if (bot->getClass() == CLASS_MAGE &&
             botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK), bot) &&
             botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK), bot))
    {
        return true;
    }
    else if (bot->getClass() == CLASS_PALADIN &&
             botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD), bot) &&
             botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD), bot))
    {
        return true;
    }

    return false;
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

    if (bot->getClass() == CLASS_PALADIN &&
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD));
    }
    else if (bot->getClass() == CLASS_MAGE &&
             bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK));
    }

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

    if (bot->getClass() == CLASS_PALADIN &&
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD)))
    {
        bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD));
    }

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
    if (bot->getClass() == CLASS_ROGUE &&
        botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_CLOAK_OF_SHADOWS), bot))
    {
        return true;
    }
    else if (bot->getClass() == CLASS_MAGE &&
             botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK), bot) &&
             botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_ICE_BLOCK), bot))
    {
        return true;
    }
    else if (bot->getClass() == CLASS_PALADIN &&
             botAI->CanCastSpell(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD), bot) &&
             botAI->CastSpell(static_cast<uint32>(SunwellSpells::SPELL_DIVINE_SHIELD), bot))
    {
        return true;
    }

    return false;
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
        isEncapsulateTargetInRangedGroup =
            instanceItr->second.find(encapsulateTarget->GetGUID()) != instanceItr->second.end();
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

        float leftDistance = bot->GetExactDist2d(leftPosition.GetPositionX(), leftPosition.GetPositionY());
        float rightDistance = bot->GetExactDist2d(rightPosition.GetPositionX(), rightPosition.GetPositionY());
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

// Eredar Twins (Alythess & Sacrolash)

bool EredarTwinsMeleeJumpDownFromBalconyAction::Execute(Event /*event*/)
{
    const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
    const Position& landingPos = EREDAR_TWINS_P2_STACK_POSITION;

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

bool EredarTwinsPositionRangedAction::Execute(Event /*event*/)
{
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
        const Position& landingPos = EREDAR_TWINS_P2_STACK_POSITION;

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
            uint8 candidateIndex = static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITIONS.size());
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

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    const Position& position = EREDAR_TWINS_P2_STACK_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool EredarTwinsDpsPrioritizeLadySacrolashAction::Execute(Event /*event*/)
{
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

// M'uru & Entropius

bool MuruAction::Execute(Event /*event*/)
{
    return false;
}

// Kil'jaeden <The Deceiver>

bool KiljaedenAction::Execute(Event /*event*/)
{
    return false;
}
