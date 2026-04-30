/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <array>
#include <vector>

#include "RaidSunwellActions.h"
#include "RaidSunwellKiljaedenEncounter.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

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
    KiljaedenArmageddon armageddon;
    if (!TryGetKiljaedenNearestArmageddon(bot, armageddon))
        return false;

    constexpr uint32 minInterval = 0;
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

    if (bot->GetExactDist2d(targetPosition->GetPositionX(),
                            targetPosition->GetPositionY()) <= 2.0f)
    {
        return false;
    }

    return MoveTo(SUNWELL_MAP_ID, targetPosition->GetPositionX(),
                  targetPosition->GetPositionY(), targetPosition->GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                  true, false);
}

bool KiljaedenPositionRangedAction::Execute(Event /*event*/)
{
    Position targetPosition = KILJAEDEN_TANK_POSITION;
    if (!TryGetRangedPosition(targetPosition))
        return false;

    if (bot->GetExactDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY()) <= 2.0f)
        return false;

    return MoveTo(SUNWELL_MAP_ID, targetPosition.GetPositionX(),
                  targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                  true, false);
}

bool KiljaedenPositionRangedAction::TryGetRangedPosition(Position& position) const
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsRanged(bot))
        return false;

    EnsureKiljaedenRangedAssignments(botAI, bot);

    auto instanceItr = kiljaedenRangedAssignments.find(bot->GetInstanceId());
    if (instanceItr == kiljaedenRangedAssignments.end())
        return false;

    auto assignmentItr = instanceItr->second.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.end())
        return false;

    uint8 slotIndex = assignmentItr->second;

    EnsureKiljaedenRangedArmageddonAssignments(botAI, bot);
    auto armageddonAssignmentItr =
        kiljaedenRangedArmageddonAssignments.find(bot->GetInstanceId());
    if (armageddonAssignmentItr != kiljaedenRangedArmageddonAssignments.end())
    {
        auto tempAssignmentItr = armageddonAssignmentItr->second.find(bot->GetGUID());
        if (tempAssignmentItr != armageddonAssignmentItr->second.end())
            slotIndex = tempAssignmentItr->second;
    }

    return TryGetKiljaedenRangedSlotPosition(slotIndex, position);
}

bool KiljaedenRemoveFireBloomAction::Execute(Event /*event*/)
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
