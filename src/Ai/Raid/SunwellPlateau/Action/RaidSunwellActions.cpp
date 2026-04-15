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
                if ((realmStateItr->second.lastEnterMs &&
                     getMSTimeDiff(realmStateItr->second.lastEnterMs, now) < KALECGOS_REALM_TRANSITION_GRACE_MS) ||
                    (realmStateItr->second.lastExitMs &&
                     getMSTimeDiff(realmStateItr->second.lastExitMs, now) < KALECGOS_REALM_TRANSITION_GRACE_MS))
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

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");

    if (!felmyst)
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
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

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
    if (!TryGetBrutallusPositionIndex(botAI, bot, false, meleeIndex))
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
    if (!TryGetBrutallusPositionIndex(botAI, bot, true, rangedIndex))
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
    if (!TryGetBrutallusPositionIndex(botAI, bot, true, rangedIndex))
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
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (bot->GetVictim() != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() == bot)
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

    constexpr float encapsulateRetreatDistance = 25.0f;
    float behindAngle =
        Position::NormalizeOrientation(felmyst->GetOrientation() + M_PI);
    float targetX =
        felmyst->GetPositionX() + encapsulateRetreatDistance * std::cos(behindAngle);
    float targetY =
        felmyst->GetPositionY() + encapsulateRetreatDistance * std::sin(behindAngle);

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

bool FelmystSpreadAndAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    Unit* hazard = GetNearestFelmystDemonicVaporHazard(
        bot, FELMYST_DEMONIC_VAPOR_SAFE_DISTANCE);
    if (hazard)
    {
        float currentDistance = bot->GetDistance2d(hazard);
        if (currentDistance >= FELMYST_DEMONIC_VAPOR_SAFE_DISTANCE)
            return false;

        return MoveAway(hazard, FELMYST_DEMONIC_VAPOR_SAFE_DISTANCE - currentDistance);
    }
    else if (!botAI->IsTank(bot))
    {
        constexpr float safeDistance = 5.0f;
        constexpr uint32 minInterval = 1000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
            return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(botAI, bot, destination))
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
    if (!GetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState))
        return false;

    Position destination;
    if (!TryGetFelmystFogSidewaysShiftDestination(bot, fogState.lane, destination))
        return false;

    return MoveTo(SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                  destination.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystAssignAirPhaseTargetPriorityAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    MarkTargetWithMoon(bot, felmyst);

    /* Unit* fogCharmedTarget = GetNearestFelmystFogOfCorruptionCharmedTarget(bot);
    Unit* unyieldingDead = GetFirstAliveUnitByEntry(
        botAI, static_cast<uint32>(SunwellNPCs::NPC_UNYIELDING_DEAD));
    Unit* target = nullptr;

    if (fogCharmedTarget)
    {
        target = fogCharmedTarget;
    }
    else if (botAI->IsMelee(bot))
    {
        target = unyieldingDead;
    }
    else
    {
        constexpr float felmystAirPriorityDistance = 35.0f;
        if (bot->GetDistance(felmyst) <= felmystAirPriorityDistance)
            target = felmyst;
        else if (unyieldingDead)
            target = unyieldingDead;
        else
            target = felmyst;
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    if (botAI->IsMelee(bot) && currentTarget &&
        currentTarget->GetEntry() == static_cast<uint32>(SunwellNPCs::NPC_FELMYST))
    {
        bot->AttackStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    if (!target)
        return false;

    if (currentTarget != target && bot->GetTarget() != target->GetGUID())
        return Attack(target); */

    return false;
}

// Eredar Twins (Alythess & Sacrolash)

bool EredarTwinsAction::Execute(Event /*event*/)
{
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
