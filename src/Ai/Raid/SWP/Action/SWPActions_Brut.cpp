/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <cmath>

#include "SWPActions.h"
#include "SWPEncounter_Brut.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

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
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (AI_VALUE(Unit*, "current target") != brutallus)
        return Attack(brutallus);

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    if (!mainTank || !assistTank)
        return false;

    Aura* mainTankAura =
        mainTank->GetAura(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH));

    Aura* assistTankAura =
        assistTank->GetAura(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH));

    constexpr float tankPositionTolerance = 2.0f;
    const bool hasReachedInitialMainTankPosition = mainTank == bot &&
        brutallusMainTankInitialPositionReached.find(bot->GetGUID()) !=
            brutallusMainTankInitialPositionReached.end();

    if (mainTank == bot)
    {
        const Position position = {
            BRUTALLUS_MAIN_TANK_POSITION.GetPositionX(),
            BRUTALLUS_MAIN_TANK_POSITION.GetPositionY(),
            bot->GetPositionZ()
        };

        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (!hasReachedInitialMainTankPosition &&
            distToPosition <= tankPositionTolerance)
        {
            brutallusMainTankInitialPositionReached.insert(bot->GetGUID());
        }

        if (brutallus->GetVictim() == bot &&
            (hasReachedInitialMainTankPosition ||
             distToPosition <= tankPositionTolerance))
        {
            return false;
        }

        if (!hasReachedInitialMainTankPosition &&
            distToPosition > tankPositionTolerance)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        if (!mainTankAura &&
            ((assistTankAura && assistTankAura->GetStackAmount() >= 3) ||
             !assistTankAura))
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }
    }
    else
    {
        if (brutallus->GetVictim() == bot)
            return false;

        const float mainTankAngle = Position::NormalizeOrientation(
            std::atan2(mainTank->GetPositionY() - brutallus->GetPositionY(),
                       mainTank->GetPositionX() - brutallus->GetPositionX()));

        const float assistTankAngle = Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        const Position position = GetBrutallusPositionAtAngle(
            brutallus, assistTankAngle, BRUTALLUS_TANK_POSITION_RADIUS, bot->GetPositionZ());

        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > tankPositionTolerance)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        if ((!assistTankAura &&
            ((mainTankAura && mainTankAura->GetStackAmount() >= 3) ||
             !mainTankAura)))
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }
    }

    return false;
}

bool BrutallusPositionMeleeAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    uint8 meleeIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, false, meleeIndex))
        return false;

    Position position;
    if (!TryGetBrutallusMeleePosition(
            bot, brutallus, mainTank, assistTank, meleeIndex, bot->GetPositionZ(), position))
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
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    const ObjectGuid guid = bot->GetGUID();
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    auto const burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        ReleaseBrutallusBurnPad(bot);
        brutallusRangedBurnStates.erase(guid);
        burnState = BrutallusRangedBurnState::None;
    }
    else if (burnState == BrutallusRangedBurnState::TraversingInnerLane ||
             burnState == BrutallusRangedBurnState::MovingToBurnPosition ||
             burnState == BrutallusRangedBurnState::AtBurnPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToOuterLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane)
    {
        const float currentAngle = Position::NormalizeOrientation(
            std::atan2(bot->GetPositionY() - brutallus->GetPositionY(),
                       bot->GetPositionX() - brutallus->GetPositionX()));
        const Position position = GetBrutallusPositionAtAngle(
            brutallus, currentAngle, BRUTALLUS_OUTER_LANE_RADIUS, bot->GetPositionZ());

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingOuterLane;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingOuterLane)
    {
        Position returnTargetPosition;
        if (!TryGetBrutallusRangedPosition(
                brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_OUTER_LANE_RADIUS, bot->GetPositionZ(), returnTargetPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                brutallus, returnTargetPosition.GetPositionX(),
                returnTargetPosition.GetPositionY(), BRUTALLUS_OUTER_LANE_RADIUS,
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

        if (bot->GetExactDist2d(returnTargetPosition.GetPositionX(),
                                returnTargetPosition.GetPositionY()) <= 1.0f)
        {
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::ReturningToNormalPosition;
        }

        return false;
    }

    if (burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        Position position;
        if (!TryGetBrutallusRangedPosition(
            brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_NORMAL_RANGED_RADIUS, bot->GetPositionZ(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        ReleaseBrutallusBurnPad(bot);
        brutallusRangedBurnStates.erase(guid);
        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedPosition(
            brutallus, mainTank, assistTank, rangedIndex,
            BRUTALLUS_NORMAL_RANGED_RADIUS, bot->GetPositionZ(), position))
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

    if (botAI->IsMelee(bot) || !bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
        return false;

    const ObjectGuid guid = bot->GetGUID();
    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(botAI, bot, true, rangedIndex))
        return false;

    auto const burnStateItr = brutallusRangedBurnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != brutallusRangedBurnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane ||
        burnState == BrutallusRangedBurnState::TraversingOuterLane ||
        burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::None)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        Position stepPosition;
        if (!TryGetBrutallusRangedPosition(
                brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_INNER_LANE_RADIUS, bot->GetPositionZ(), stepPosition))
        {
            return false;
        }

        if (bot->GetExactDist2d(stepPosition.GetPositionX(), stepPosition.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                          stepPosition.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingInnerLane;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingInnerLane)
    {
        Position padIngressPosition;
        if (!TryGetBrutallusBurnPadPosition(
                bot, brutallus, mainTank, rangedIndex, BRUTALLUS_INNER_LANE_RADIUS,
                bot->GetPositionZ(), padIngressPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                brutallus, padIngressPosition.GetPositionX(), padIngressPosition.GetPositionY(),
                BRUTALLUS_INNER_LANE_RADIUS, bot->GetPositionX(), bot->GetPositionY(),
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

        if (bot->GetExactDist2d(padIngressPosition.GetPositionX(),
                                padIngressPosition.GetPositionY()) <= 1.0f)
        {
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToBurnPosition;
        }

        return false;
    }

    Position position;
    if (!TryGetBrutallusBurnPadPosition(
            bot, brutallus, mainTank, rangedIndex, BRUTALLUS_BURN_PAD_RADIUS,
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

    brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::AtBurnPosition;

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
