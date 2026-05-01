/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellActions.h"
#include "RaidSunwellBrutallusEncounter.h"
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

    if (bot->GetVictim() != brutallus)
        return Attack(brutallus);

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* assistTank = GetGroupAssistTank(botAI, bot, 0);

    Aura* mainTankAura = mainTank && mainTank->IsAlive() ?
        mainTank->GetAura(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH)) : nullptr;

    Aura* assistTankAura = assistTank && assistTank->IsAlive() ?
        assistTank->GetAura(static_cast<uint32>(SunwellSpells::SPELL_METEOR_SLASH)) : nullptr;

    constexpr float tankPositionTolerance = 2.0f;
    const bool isMainTank = botAI->IsMainTank(bot);
    const bool assistTankMissing = !assistTank || !assistTank->IsAlive();
    const bool hasReachedInitialMainTankPosition =
        isMainTank &&
        brutallusMainTankInitialPositionsReached.find(bot->GetGUID()) !=
            brutallusMainTankInitialPositionsReached.end();

    if (!isMainTank)
    {
        const bool shouldTaunt = assistTankMissing ||
                                 (!assistTankAura &&
                                  ((mainTankAura && mainTankAura->GetStackAmount() >= 3) ||
                                   !mainTankAura));

        if (brutallus->GetVictim() == bot)
            return false;

        if (shouldTaunt)
            return botAI->DoSpecificAction("taunt spell", event, true);

        const Position position = GetBrutallusTankPosition(brutallus, false, bot->GetPositionZ());
        const float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > tankPositionTolerance)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }

        return false;
    }

    const Position position =
        GetBrutallusTankPosition(brutallus, isMainTank, bot->GetPositionZ());
    const float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (!hasReachedInitialMainTankPosition &&
        distToPosition <= tankPositionTolerance)
    {
        brutallusMainTankInitialPositionsReached.insert(bot->GetGUID());
    }

    if (brutallus->GetVictim() == bot &&
        (hasReachedInitialMainTankPosition ||
         distToPosition <= tankPositionTolerance))
        return false;

    if (!hasReachedInitialMainTankPosition &&
        distToPosition > tankPositionTolerance)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    if (isMainTank &&
        (assistTankMissing ||
         (!mainTankAura &&
          ((assistTankAura && assistTankAura->GetStackAmount() >= 3) ||
           !assistTankAura))))
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
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

    if (burnState == BrutallusRangedBurnState::MovingToInnerArcStart ||
        burnState == BrutallusRangedBurnState::TraversingInnerArc)
    {
        brutallusRangedBurnStates.erase(guid);
        burnState = BrutallusRangedBurnState::None;
    }
    else if (burnState == BrutallusRangedBurnState::MovingToBurnPosition ||
             burnState == BrutallusRangedBurnState::AtBurnPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToOuterArcStart;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToOuterArcStart)
    {
        Position position;
        if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, true, BRUTALLUS_OUTER_LANE_RADIUS,
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

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingOuterArc;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingOuterArc)
    {
        Position position;
        if (!TryGetBrutallusRangedLaneTraversalPosition(
            brutallus, mainTank, assistTank, rangedIndex, BRUTALLUS_OUTER_LANE_RADIUS, false,
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

        Position returnTargetPosition;
        if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, false, BRUTALLUS_OUTER_LANE_RADIUS,
                bot->GetPositionZ(), returnTargetPosition))
        {
            return false;
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
        if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, false,
            BRUTALLUS_NORMAL_RANGED_RADIUS,
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
    if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, false,
            BRUTALLUS_NORMAL_RANGED_RADIUS,
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

    if (burnState == BrutallusRangedBurnState::MovingToOuterArcStart ||
        burnState == BrutallusRangedBurnState::TraversingOuterArc ||
        burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToBurnPosition;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::None)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerArcStart;
        brutallusRangedBurnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToInnerArcStart)
    {
        Position stepPosition;
        if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, false,
            BRUTALLUS_INNER_LANE_RADIUS,
                bot->GetPositionZ(), stepPosition))
            return false;

        if (bot->GetExactDist2d(stepPosition.GetPositionX(), stepPosition.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, stepPosition.GetPositionX(), stepPosition.GetPositionY(),
                          stepPosition.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::TraversingInnerArc;
        return false;
    }

    if (burnState == BrutallusRangedBurnState::TraversingInnerArc)
    {
        Position position;
        if (!TryGetBrutallusRangedLaneTraversalPosition(
            brutallus, mainTank, assistTank, rangedIndex, BRUTALLUS_INNER_LANE_RADIUS, true,
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
        if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, true,
            BRUTALLUS_INNER_LANE_RADIUS,
                bot->GetPositionZ(), mirrorStepPosition))
        {
            return false;
        }

        if (bot->GetExactDist2d(mirrorStepPosition.GetPositionX(),
                                mirrorStepPosition.GetPositionY()) <= 1.0f)
        {
            brutallusRangedBurnStates[guid] = BrutallusRangedBurnState::MovingToBurnPosition;
        }

        return false;
    }

    Position position;
    if (!TryGetBrutallusRangedLanePosition(
            brutallus, mainTank, assistTank, rangedIndex, true,
            BRUTALLUS_NORMAL_RANGED_RADIUS,
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
