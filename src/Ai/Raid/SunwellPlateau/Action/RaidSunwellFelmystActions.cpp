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

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() == bot && bot->GetHealthPct() > 50.0f)
    {
        Position const& position = GetFelmystMainTankGroundPosition(bot);
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

    Position position;
    if (!TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Melee, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(
            position.GetPositionX(), position.GetPositionY()) > 0.25f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
    {
        return botAI->CanCastSpell("ice block", bot) &&
               botAI->CastSpell("ice block", bot);
    }
    else
    {
        return botAI->CanCastSpell("divine shield", bot) &&
               botAI->CastSpell("divine shield", bot);
    }
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;


    const FelmystGroundStack botStack =
        GetClosestFelmystGroundStack(botAI, bot, felmyst, bot);
    const FelmystGroundStack targetStack =
        GetClosestFelmystGroundStack(botAI, bot, felmyst, encapsulateTarget);
    if (botStack == FelmystGroundStack::None || targetStack == FelmystGroundStack::None ||
        botStack != targetStack)
    {
        return false;
    }

    auto const tryMoveToStack = [&](FelmystGroundStack stack)
    {
        Position position;
        if (!TryGetFelmystGroundStackPosition(botAI, bot, felmyst, stack, position))
            return false;

        return MoveInside(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(),
                          FELMYST_RANGED_GROUP_RADIUS, MovementPriority::MOVEMENT_FORCED);
    };

    if (targetStack == FelmystGroundStack::Left ||
        targetStack == FelmystGroundStack::Right)
    {
        if (tryMoveToStack(FelmystGroundStack::Melee))
            return true;

        return tryMoveToStack(
            targetStack == FelmystGroundStack::Left ?
                FelmystGroundStack::Right : FelmystGroundStack::Left);
    }

    Position leftPosition;
    Position rightPosition;
    if (!TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Left, leftPosition) ||
        !TryGetFelmystGroundStackPosition(
            botAI, bot, felmyst, FelmystGroundStack::Right, rightPosition))
    {
        return false;
    }

    if (bot->GetExactDist2d(leftPosition.GetPositionX(), leftPosition.GetPositionY()) <=
        bot->GetExactDist2d(rightPosition.GetPositionX(), rightPosition.GetPositionY()))
    {
        if (tryMoveToStack(FelmystGroundStack::Left))
            return true;

        return tryMoveToStack(FelmystGroundStack::Right);
    }

    if (tryMoveToStack(FelmystGroundStack::Right))
        return true;

    return tryMoveToStack(FelmystGroundStack::Left);
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
