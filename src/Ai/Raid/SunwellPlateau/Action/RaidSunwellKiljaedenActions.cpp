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

    for (const ObjectGuid& guid : attackers)
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

    TryAnnounceKiljaedenDragonOrbUser(botAI, bot);

    if (volatileFelfireFiend)
        MarkTargetWithSkull(bot, volatileFelfireFiend);

    std::array<Player*, 3> tanks = { mainTank, firstAssistTank, secondAssistTank };
    const size_t assignedCount = hands.size() < tanks.size() ? hands.size() : tanks.size();

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

        if (AI_VALUE(Unit*, "current target") != hand)
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
        auto const isSafePosition = [&](Position const& position)
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

        const bool assignedSafe = isSafePosition(assignedPosition);
        const bool swapSafe = isSafePosition(swapPosition);
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

    auto const instanceItr = kiljaedenRangedAssignments.find(bot->GetInstanceId());
    if (instanceItr == kiljaedenRangedAssignments.end())
        return false;

    auto const assignmentItr = instanceItr->second.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.end())
        return false;

    uint8 slotIndex = assignmentItr->second;

    EnsureKiljaedenRangedArmageddonAssignments(botAI, bot);
    auto const armageddonAssignmentItr =
        kiljaedenRangedArmageddonAssignments.find(bot->GetInstanceId());
    if (armageddonAssignmentItr != kiljaedenRangedArmageddonAssignments.end())
    {
        auto const tempAssignmentItr = armageddonAssignmentItr->second.find(bot->GetGUID());
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

bool KiljaedenUseDragonOrbAction::Execute(Event /*event*/)
{
    GameObject* closestOrb = nullptr;
    float closestDistance = 0.0f;

    for (const uint32 orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        GameObject* orb = bot->FindNearestGameObject(orbEntry, 200.0f, true);
        if (!orb || orb->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE) ||
            orb->HasGameObjectFlag(GO_FLAG_IN_USE))
        {
            continue;
        }

        const float distance = bot->GetExactDist2d(orb);
        if (!closestOrb || distance < closestDistance)
        {
            closestOrb = orb;
            closestDistance = distance;
        }
    }

    if (!closestOrb)
        return false;

    if (closestDistance < 3.0f)
    {
        closestOrb->Use(bot);
        return true;
    }

    return MoveTo(SUNWELL_MAP_ID, closestOrb->GetPositionX(), closestOrb->GetPositionY(),
                  closestOrb->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KiljaedenControlDragonAction::Execute(Event /*event*/)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return ExecuteDuringDarknessOfAThousandSouls(kiljaeden);

    return ExecuteOutsideDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenControlDragonAction::ExecuteDuringDarknessOfAThousandSouls(Unit* kiljaeden)
{
    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

    Spell* darknessSpell = kiljaeden->FindCurrentSpellBySpellId(
        static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    if (!darknessSpell)
        return false;

    constexpr float desiredDistanceFromStack = 2.0f;
    constexpr float castReadyDistanceFromStack = 3.0f;
    const Position& stackPosition = KILJAEDEN_STACK_POSITION;
    const float distanceToStack =
        dragon->GetExactDist2d(stackPosition.GetPositionX(), stackPosition.GetPositionY());
    if (distanceToStack > castReadyDistanceFromStack)
    {
        bool const pointMovementInProgress =
            dragon->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE &&
            dragon->isMoving();
        if (pointMovementInProgress)
            return true;

        const float deltaX = stackPosition.GetPositionX() - dragon->GetPositionX();
        const float deltaY = stackPosition.GetPositionY() - dragon->GetPositionY();
        const float moveRatio = (distanceToStack - desiredDistanceFromStack) / distanceToStack;
        const float moveX = dragon->GetPositionX() + deltaX * moveRatio;
        const float moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, stackPosition.GetPositionZ());
        return true;
    }

    if (dragon->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        dragon->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        return false;
    }

    const uint32 darknessCastTimeLeft = darknessSpell->GetCastTimeRemaining();
    if (darknessCastTimeLeft > 3000)
    {
        if (CastKiljaedenDragonSpell(
                dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE)))
        {
            return true;
        }

        if (CastKiljaedenDragonSpell(
                dragon, static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE)))
        {
            return true;
        }
    }

    if (darknessCastTimeLeft < 5000)
    {
        return CastKiljaedenDragonSpell(
            dragon, static_cast<uint32>(SunwellSpells::SPELL_SHIELD_OF_THE_BLUE));
    }

    return false;
}

bool KiljaedenControlDragonAction::ExecuteOutsideDarknessOfAThousandSouls(Unit* /*kiljaeden*/)
{
    Unit* dragon = GetKiljaedenControlledDragon(bot);
    if (!dragon)
        return false;

    if (dragon->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        dragon->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        return false;
    }

    uint32 spellId = 0;
    Player* target = nullptr;

    uint32 const hasteSpellId =
        static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_HASTE);
    uint32 const revitalizeSpellId =
        static_cast<uint32>(SunwellSpells::SPELL_DRAGON_BREATH_REVITALIZE);

    if (!dragon->HasSpellCooldown(hasteSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(botAI, bot, dragon, hasteSpellId);
        if (!target)
            target = FindClosestKiljaedenDragonTarget(bot, dragon, hasteSpellId);
        if (target)
            spellId = hasteSpellId;
    }

    if (!target && !dragon->HasSpellCooldown(revitalizeSpellId))
    {
        target = FindBestKiljaedenDragonClusterTarget(botAI, bot, dragon, revitalizeSpellId);
        if (!target)
            target = FindClosestKiljaedenDragonTarget(bot, dragon, revitalizeSpellId);
        if (target)
            spellId = revitalizeSpellId;
    }

    if (!target)
    {
        target = FindClosestKiljaedenDragonTarget(bot, dragon);
        if (!target)
            return false;

        if (!dragon->HasSpellCooldown(hasteSpellId))
            spellId = hasteSpellId;
        else if (!dragon->HasSpellCooldown(revitalizeSpellId))
            spellId = revitalizeSpellId;
        else
            return false;
    }

    if (!spellId)
        return false;

    constexpr float desiredDistance = 6.0f;
    constexpr float distanceTolerance = 1.0f;
    const float distanceToTarget = dragon->GetExactDist2d(target);
    if (distanceToTarget > desiredDistance + distanceTolerance ||
        (distanceToTarget > 0.1f && distanceToTarget < desiredDistance - distanceTolerance))
    {
        const float deltaX = target->GetPositionX() - dragon->GetPositionX();
        const float deltaY = target->GetPositionY() - dragon->GetPositionY();
        const float moveRatio = (distanceToTarget - desiredDistance) / distanceToTarget;
        const float moveX = dragon->GetPositionX() + deltaX * moveRatio;
        const float moveY = dragon->GetPositionY() + deltaY * moveRatio;

        dragon->GetMotionMaster()->MovePoint(0, moveX, moveY, target->GetPositionZ());
        return true;
    }

    dragon->SetFacingToObject(target);

    return CastKiljaedenDragonSpell(dragon, spellId);
}
