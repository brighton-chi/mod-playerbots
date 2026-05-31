/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>

#include "SWPActions.h"
#include "SWPEncounter_Kalec.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TargetValue.h"

using namespace SunwellHelpers;

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    const Position& position = KALECGOS_TANK_POSITION;
    const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                     position.GetPositionY());
    auto const moveTowardTankPosition = [&]()
    {
        const float dX = position.GetPositionX() - bot->GetPositionX();
        const float dY = position.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, distToPosition);
        const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    };

    Player* currentVictimPlayer = kalecgos->GetVictim() ? kalecgos->GetVictim()->ToPlayer() : nullptr;
    const bool otherTankHasAggro =
        currentVictimPlayer && currentVictimPlayer != bot && botAI->IsTank(currentVictimPlayer);

    if (otherTankHasAggro)
    {
        if (distToPosition > 3.0f)
            return moveTowardTankPosition();
    }

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    if (kalecgos->GetVictim() != bot)
    {
        if (otherTankHasAggro && distToPosition > 3.0f)
            return moveTowardTankPosition();

        return botAI->DoSpecificAction("taunt spell", event, true);
    }
    else if (kalecgos->GetVictim() == bot && bot->IsWithinMeleeRange(kalecgos))
    {
        if (distToPosition > 3.0f)
            return moveTowardTankPosition();
    }

    return false;
}

bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
        kalecgos && botAI->IsTank(bot))
    {
        Player* surfaceTank = GetKalecgosCurrentTank(botAI, bot);
        if (!surfaceTank)
            return false;

        if (surfaceTank == bot)
        {
            surfaceTank = GetKalecgosReplacementTank(botAI, bot);
            if (!surfaceTank)
                return false;
        }

        const Position& position = KALECGOS_TANK_POSITION;
        if (surfaceTank->GetExactDist2d(position.GetPositionX(),
            position.GetPositionY()) > 3.0f || kalecgos->GetVictim() != surfaceTank)
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
    if (hasReachedKalecgosInitialRangedPosition.find(bot->GetGUID()) ==
        hasReachedKalecgosInitialRangedPosition.end())
    {
        const Position& initialPos = KALECGOS_INITIAL_RANGED_POSITION;
        constexpr float initialRangedRadius = 10.0f;
        if (bot->GetExactDist2d(initialPos.GetPositionX(), initialPos.GetPositionY()) <=
            initialRangedRadius)
        {
            SetKalecgosInitialRangedPositionReached(bot, true);
            return false;
        }

        return MoveInside(SUNWELL_MAP_ID, initialPos.GetPositionX(), initialPos.GetPositionY(),
                          initialPos.GetPositionZ(), initialRangedRadius,
                          MovementPriority::MOVEMENT_COMBAT);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        constexpr float safeDistFromBoss = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(kalecgos) < safeDistFromBoss)
            return FleePosition(kalecgos->GetPosition(), safeDistFromBoss, minInterval);
    }

    return false;
}

bool KalecgosRemoveArcaneBuffetAction::Execute(Event /*event*/)
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

bool KalecgosSathrovarrTankStandWithKalecAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    Unit* kalec = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_KALECGOS_HUMANOID), searchRadius);

    if (!kalec)
        return false;

    const Position& position = kalec->GetPosition();
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
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
    {
        return false;
    }

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    return false;
}

bool KalecgosReturnToSpectralRealmGroundAction::Execute(Event /*event*/)
{
    if (bot->TeleportTo(SUNWELL_MAP_ID, bot->GetPositionX(), bot->GetPositionY(),
                        KALECGOS_SPECTRAL_REALM_Z, bot->GetOrientation()))
    {
        return true;
    }

    return false;
}
