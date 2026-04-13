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
                kalecgosEncounterStates.erase(bot->GetInstanceId()) > 0)
            {
                erased = true;
            }

            if (kalecgosRealmStates.erase(guid) > 0)
                erased = true;
        }
    }

    // Brutallus

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");

    if (!brutallus && botAI->IsRanged(bot) &&
        hasReachedBrutallusRangedBurnStepPosition.erase(guid) > 0)
    {
        erased = true;
    }

    return erased;
}

// Kalecgos & Sathrovarr the Corruptor

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

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
    constexpr float arrivalDistance = 3.0f;
    if (!HasReachedKalecgosInitialRangedPosition(bot))
    {
        const Position& initialPosition = KALECGOS_INITIAL_RANGED_POSITION;
        float distanceToInitialPosition = bot->GetExactDist2d(initialPosition.GetPositionX(),
                                                              initialPosition.GetPositionY());
        if (distanceToInitialPosition > arrivalDistance)
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
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (bot->GetVictim() != brutallus)
        return Attack(brutallus);

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);
    Aura* mainTankAura = nullptr;
    Aura* assistTankAura = nullptr;

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
    Position position = GetBrutallusTankPosition(brutallus, isMainTank, bot->GetPositionZ());

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (brutallus->GetVictim() == bot)
        return false;

    if (isMainTank &&
        ((!assistTank || !assistTank->IsAlive()) ||
         (assistTankAura && assistTankAura->GetStackAmount() >= 3) ||
         (!assistTankAura && !mainTankAura)))
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
    }
    else if ((!mainTank || !mainTank->IsAlive()) ||
             (mainTankAura && mainTankAura->GetStackAmount() >= 3))
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

    constexpr float positionTolerance = 2.0f;

    Position position;
    if (!TryGetBrutallusMeleePosition(brutallus, meleeIndex, bot->GetPositionZ(), position))
        return false;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > positionTolerance)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
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

    uint8 rangedIndex = 0;
    if (!TryGetBrutallusPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    hasReachedBrutallusRangedBurnStepPosition.erase(bot->GetGUID());

    constexpr float positionTolerance = 2.0f;
    Position position;
    if (!TryGetBrutallusRangedPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
        return false;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > positionTolerance)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
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

    if (botAI->IsMelee(bot))
        return false;

    if (!bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN_DAMAGE)))
    {
        hasReachedBrutallusRangedBurnStepPosition.erase(bot->GetGUID());
        return false;
    }

    uint8 rangedIndex = 0;
    if (!TryGetBrutallusPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    constexpr float positionTolerance = 2.0f;
    bool hasReachedBurnStep = hasReachedBrutallusRangedBurnStepPosition[bot->GetGUID()];
    if (!hasReachedBurnStep)
    {
        Position stepPosition;
        if (!TryGetBrutallusRangedBurnStepPosition(brutallus, rangedIndex, bot->GetPositionZ(), stepPosition))
            return false;

        if (bot->GetExactDist2d(stepPosition.GetPositionX(), stepPosition.GetPositionY()) > positionTolerance)
        {
            return MoveTo(SUNWELL_MAP_ID, stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                          stepPosition.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        hasReachedBrutallusRangedBurnStepPosition[bot->GetGUID()] = true;
        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedBurnPosition(brutallus, rangedIndex, bot->GetPositionZ(), position))
        return false;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > positionTolerance)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

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

bool FelmystAction::Execute(Event /*event*/)
{
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
