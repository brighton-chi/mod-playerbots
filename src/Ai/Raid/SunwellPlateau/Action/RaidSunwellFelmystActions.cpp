/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <array>
#include <cmath>

#include "RaidSunwellActions.h"
#include "RaidSunwellFelmystEncounter.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

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
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Bounding Radius and Combat Reach of 10.0f
bool FelmystPositionRangedOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(botAI, bot, felmyst, position))
        return false;

    return MoveInside(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), FELMYST_RANGED_GROUP_RADIUS,
                      MovementPriority::MOVEMENT_COMBAT);
}

bool FelmystPositionMeleeOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    const float behindAngle = Position::NormalizeOrientation(
        GetFelmystFrontAngle(botAI, bot, felmyst) + M_PI);
    float targetX = felmyst->GetPositionX() +
        FELMYST_MELEE_DISTANCE * std::cos(behindAngle);
    float targetY = felmyst->GetPositionY() +
        FELMYST_MELEE_DISTANCE * std::sin(behindAngle);
    float targetZ = bot->GetMapWaterOrGroundLevel(
        targetX, targetY, bot->GetPositionZ());

    if (targetZ <= INVALID_HEIGHT)
        targetZ = bot->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        targetX, targetY, targetZ, false);

    if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
    {
        return MoveTo(SUNWELL_MAP_ID, targetX, targetY, targetZ, false, false, false,
                      false, MovementPriority::MOVEMENT_COMBAT, true, false);
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

    const float distToEncapsulated = bot->GetDistance2d(encapsulateTarget);

    if (distToEncapsulated > FELMYST_ENCAPSULATE_SAFE_DISTANCE)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    EnsureFelmystRangedAssignments(botAI, bot);

    auto const instanceItr = felmystRangedAssignments.find(bot->GetInstanceId());
    const bool isEncapsulateTargetInRangedGroup =
        instanceItr != felmystRangedAssignments.end() &&
        instanceItr->second.find(encapsulateTarget->GetGUID()) != instanceItr->second.end();

    const float behindAngle = Position::NormalizeOrientation(
        felmyst->GetOrientation() + M_PI);

    const float meleeX = felmyst->GetPositionX() +
        FELMYST_MELEE_DISTANCE * std::cos(behindAngle);
    const float meleeY = felmyst->GetPositionY() +
        FELMYST_MELEE_DISTANCE * std::sin(behindAngle);

    const float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);

    const float leftX = felmyst->GetPositionX() +
        std::cos(frontAngle + M_PI_2) * FELMYST_RANGED_SIDE_DISTANCE;
    const float leftY = felmyst->GetPositionY() +
        std::sin(frontAngle + M_PI_2) * FELMYST_RANGED_SIDE_DISTANCE;
    const float rightX = felmyst->GetPositionX() +
        std::cos(frontAngle - M_PI_2) * FELMYST_RANGED_SIDE_DISTANCE;
    const float rightY = felmyst->GetPositionY() +
        std::sin(frontAngle - M_PI_2) * FELMYST_RANGED_SIDE_DISTANCE;

    auto const tryMoveToStack = [&](float x, float y)
    {
        return MoveInside(SUNWELL_MAP_ID, x, y, bot->GetPositionZ(),
                          FELMYST_RANGED_GROUP_RADIUS, MovementPriority::MOVEMENT_FORCED);
    };

    if (isEncapsulateTargetInRangedGroup)
    {
        if (tryMoveToStack(meleeX, meleeY))
            return true;

        const float leftTargetDistance =
            encapsulateTarget->GetExactDist2d(leftX, leftY);
        const float rightTargetDistance =
            encapsulateTarget->GetExactDist2d(rightX, rightY);

        if (leftTargetDistance >= rightTargetDistance)
        {
            if (leftTargetDistance > FELMYST_ENCAPSULATE_SAFE_DISTANCE &&
                tryMoveToStack(leftX, leftY))
            {
                return true;
            }

            if (rightTargetDistance > FELMYST_ENCAPSULATE_SAFE_DISTANCE &&
                tryMoveToStack(rightX, rightY))
            {
                return true;
            }
        }
        else
        {
            if (rightTargetDistance > FELMYST_ENCAPSULATE_SAFE_DISTANCE &&
                tryMoveToStack(rightX, rightY))
            {
                return true;
            }

            if (leftTargetDistance > FELMYST_ENCAPSULATE_SAFE_DISTANCE &&
                tryMoveToStack(leftX, leftY))
            {
                return true;
            }
        }
    }
    else
    {
        if (bot->GetExactDist2d(leftX, leftY) <= bot->GetExactDist2d(rightX, rightY))
        {
            if (tryMoveToStack(leftX, leftY))
                return true;

            if (tryMoveToStack(rightX, rightY))
                return true;
        }
        else
        {
            if (tryMoveToStack(rightX, rightY))
                return true;

            if (tryMoveToStack(leftX, leftY))
                return true;
        }
    }

    return MoveAway(
        encapsulateTarget, FELMYST_ENCAPSULATE_SAFE_DISTANCE - distToEncapsulated + 2.0f);
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
        const float currentDistance = bot->GetDistance2d(hazard);
        if (currentDistance < safeDistFromVapor)
            return MoveAway(hazard, safeDistFromVapor - currentDistance);
    }

    return false;
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    return MoveTo(SUNWELL_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
                  destination.GetPositionZ(), false, false, false, false,
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
