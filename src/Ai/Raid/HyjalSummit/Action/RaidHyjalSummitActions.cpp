/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitEraseTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();

    bool erased = false;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill") &&
        hasReachedWinterchillPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "anetheron") &&
        hasReachedAnetheronPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
        isBelowManaThreshold.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "azgalor") &&
        azgalorTankStep.erase(guid) > 0)
    {
        erased = true;
    }

    return erased;
}

// Rage Winterchill

bool RageWinterchillMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", winterchill))
        return botAI->CastSpell("steady shot", winterchill);

    return false;
}

bool RageWinterchillMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (bot->GetTarget() != winterchill->GetGUID())
        return Attack(winterchill);

    if (winterchill->GetVictim() == bot)
    {
        const Position& position = WINTERCHILL_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool RageWinterchillSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedWinterchillPosition[guid])
    {
        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        const float radius = botAI->IsHeal(bot) ? 25.0f : 35.0f;
        float angle = 0.0f;

        constexpr float arcSpan = 2.0f * M_PI;
        constexpr float arcCenter = 0.0f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        const Position& position = WINTERCHILL_TANK_POSITION;
        float targetX = position.GetPositionX() + radius * std::cos(angle);
        float targetY = position.GetPositionY() + radius * std::sin(angle);

        // START TEST: Calculate the actual ground level at the target X, Y
        float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY, position.GetPositionZ());

        if (targetZ <= INVALID_HEIGHT)
            targetZ = position.GetPositionZ();

        // Ensure the path is walkable and adjust coordinates if there is a collision
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), targetX, targetY,
                                                       targetZ, false);

        if (bot->GetExactDist(targetX, targetY, targetZ) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        } // END TEST
        else
        {
            hasReachedWinterchillPosition[guid] = true;
        }
    }

    return false;
}

// Anetheron

bool AnetheronMisdirectBossAndInfernalsToTanksAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (anetheron->GetHealthPct() > 95.0f)
    {
        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (!mainTank)
            return false;

        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", anetheron))
            return botAI->CastSpell("steady shot", anetheron);
    }

    if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
        infernal && infernal->GetHealthPct() > 50.0f)
    {
        Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
        if (!firstAssistTank)
            return false;

        if (botAI->CanCastSpell("misdirection", firstAssistTank))
            return botAI->CastSpell("misdirection", firstAssistTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", infernal))
            return botAI->CastSpell("steady shot", infernal);
    }

    return false;
}

bool AnetheronMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    MarkTargetWithSquare(bot, anetheron);
    SetRtiTarget(botAI, "square", anetheron);

    if (bot->GetTarget() != anetheron->GetGUID())
        return Attack(anetheron);

    if (anetheron->GetVictim() == bot)
    {
        const Position& position = ANETHERON_MAIN_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AnetheronSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedAnetheronPosition[guid])
    {
        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        const float radius = botAI->IsHeal(bot) ? 27.0f : 34.0f;
        float angle = 0.0f;

        constexpr float arcSpan = 3.0f * M_PI / 2.0f;
        constexpr float arcCenter = 0.165f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        const Position& position = ANETHERON_MAIN_TANK_POSITION;

        float targetX = position.GetPositionX() + radius * std::sin(angle);
        float targetY = position.GetPositionY() + radius * std::cos(angle);

        // START TEST: Calculate the actual ground level at the target X, Y
        float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY, position.GetPositionZ());
        if (targetZ <= INVALID_HEIGHT)
            targetZ = position.GetPositionZ();

        // Ensure the path is walkable and adjust coordinates if there is a collision
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), targetX, targetY,
                                                       targetZ, false);

        if (bot->GetExactDist(targetX, targetY, targetZ) > 2.0f)
        {
            // Set exact_waypoint (8th parameter) to false to ensure pathfinding is used
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        } //END TEST
        else
        {
            hasReachedAnetheronPosition[guid] = true;
        }
    }
    else
    {
        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 2000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
    }

    return false;
}

bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    const Position& position = GetClosestInfernalTankPosition(bot);
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        botAI->Reset();
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AnetheronFirstAssistTankPickUpInfernalsAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (infernoTarget && infernoTarget != bot)
    {
        float distToInfernoTarget = bot->GetExactDist2d(infernoTarget);
        if (distToInfernoTarget > 5.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(HYJAL_SUMMIT_MAP_ID, infernoTarget->GetPositionX(),
                          infernoTarget->GetPositionY(), infernoTarget->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                          true, false);
        }
    }

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
        return false;

    MarkTargetWithDiamond(bot, infernal);
    SetRtiTarget(botAI, "diamond", infernal);

    if (bot->GetTarget() != infernal->GetGUID())
        return Attack(infernal);

    if (infernal->GetVictim() == bot && bot->IsWithinMeleeRange(infernal))
    {
        const Position& position = GetClosestInfernalTankPosition(bot);
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (botAI->IsMelee(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (bot->GetTarget() != anetheron->GetGUID())
            return Attack(anetheron);

        return false;
    }
    else if (botAI->IsRanged(bot))
    {
        if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal"))
        {
            constexpr float safeDistFromInfernal = 10.0f;
            constexpr uint32 minInterval = 0;
            if (infernal->GetVictim() != bot &&
                bot->GetDistance2d(infernal) < safeDistFromInfernal)
            {
                return FleePosition(infernal->GetPosition(), safeDistFromInfernal, minInterval);
            }

            if (botAI->IsRangedDps(bot) && bot->GetDistance2d(infernal) < 50.0f)
            {
                if (Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
                    !firstAssistTank ||
                    (firstAssistTank && infernal->GetVictim() == firstAssistTank))
                {
                    SetRtiTarget(botAI, "diamond", infernal);

                    if (bot->GetTarget() != infernal->GetGUID())
                        return Attack(infernal);
                }
            }
        }
        else if (botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "square", anetheron);
            if (bot->GetTarget() != anetheron->GetGUID())
                return Attack(anetheron);
        }
    }

    return false;
}

// Kaz'rogal

bool KazrogalMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", kazrogal))
        return botAI->CastSpell("steady shot", kazrogal);

    return false;
}

bool KazrogalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->GetTarget() != kazrogal->GetGUID())
        return Attack(kazrogal);

    if (kazrogal->GetVictim() == bot && bot->IsWithinMeleeRange(kazrogal))
    {
        const Position& position = KAZROGAL_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    const Position& position = KAZROGAL_TANK_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 4.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KazrogalSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    size_t count = 0;
    size_t botIndex = 0;
    constexpr float radius = 24.0f;

    constexpr float arcSpan = 2.0f * M_PI; // 360 degrees
    float arcStart = 0.2f; // slight offset for starting point

    float angle = 0.0f;

    if (botAI->IsHeal(bot))
    {
        auto findIt = std::find(healers.begin(), healers.end(), bot);
        botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
        count = healers.size();

        angle = (count == 1) ? arcStart :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count));
    }
    else
    {
        auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
        botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
        count = rangedDps.size();

        angle = (count == 1) ? (arcStart + M_PI) : // offset opposite healers if solo
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count));
    }

    float targetX = kazrogal->GetPositionX() + radius * std::cos(angle);
    float targetY = kazrogal->GetPositionY() + radius * std::sin(angle);

    // Terrain check and collision adjustment
    float targetZ = bot->GetMapWaterOrGroundLevel(targetX, targetY, kazrogal->GetPositionZ());
    if (targetZ <= INVALID_HEIGHT)
        targetZ = kazrogal->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                   bot->GetPositionZ(), targetX, targetY,
                                                   targetZ, false);
    // End test

    if (bot->GetExactDist2d(targetX, targetY) > 0.5f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, targetZ, false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KazrogalLowManaBotMoveFromGroupAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        if (!botAI->HasAura("aspect of the viper", bot) &&
            botAI->CanCastSpell("aspect of the viper", bot))
        {
            return botAI->CastSpell("aspect of the viper", bot);
        }
    }
    else
    {
        if (bot->GetPower(POWER_MANA) <= 3000)
            isBelowManaThreshold.try_emplace(bot->GetGUID(), true);

        if (bot->HasAura(SPELL_MARK_OF_KAZROGAL) && bot->GetPower(POWER_MANA) <= 1200)
        {
            if (bot->getClass() == CLASS_MAGE &&
                botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot))
            {
                return true;
            }
            else if (bot->getClass() == CLASS_PALADIN &&
                     botAI->CanCastSpell("divine shield", bot) &&
                     botAI->CastSpell("divine shield", bot))
            {
                return true;
            }
        }

        constexpr float safeDistance = 16.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (nearestPlayer && bot->GetDistance2d(nearestPlayer) < safeDistance)
            return MoveFromGroup(safeDistance);
    }

    return false;
}

// Azgalor

bool AzgalorMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", azgalor))
        return botAI->CastSpell("steady shot", azgalor);

    return false;
}

bool AzgalorMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    MarkTargetWithStar(bot, azgalor);
    SetRtiTarget(botAI, "star", azgalor);

    if (bot->GetTarget() != azgalor->GetGUID())
        return Attack(azgalor);

    if (azgalor->GetVictim() == bot)
    {
        const ObjectGuid guid = bot->GetGUID();
        uint8 step = azgalorTankStep.count(guid) ? azgalorTankStep[guid] : 0;

        const Position positions[2] =
        {
            AZGALOR_MAIN_TANK_TRANSITION_POSITION,
            AZGALOR_MAIN_TANK_FINAL_POSITION
        };
        constexpr float maxDistance = 2.0f;
        const Position& position = positions[step];
        float distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance && bot->IsWithinMeleeRange(azgalor))
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, true);
        }

        if (step == 0 && distToPosition <= maxDistance)
        {
            azgalorTankStep[guid] = 1;
        }
        else if (step == 1 && distToPosition <= maxDistance)
        {
            float orientation = atan2(azgalor->GetPositionY() - bot->GetPositionY(),
                                      azgalor->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            azgalorTankStep[guid] = 2; // try setting a step 2 for dps/healers to engage
        }
    }

    return false;
}

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;

    if (GetAzgalorTankStep(botAI, bot) < 1 && azgalor->GetHealthPct() >= 85.0f &&
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 5.0f)
    {
        if (botAI->IsAssistHealOfIndex(bot, 0, true))
        {
            constexpr float safeDistFromBoss = 30.0f;
            constexpr uint32 minInterval = 0;
            if (bot->GetExactDist2d(azgalor) < safeDistFromBoss)
                return FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval);
        }
        else
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        constexpr uint32 minInterval = 1000;

        // Azgalor's hitbox is 8.8 yards
        constexpr float safeDistFromBoss = 29.0f;
        if (bot->GetExactDist2d(azgalor) < safeDistFromBoss)
            return FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval);

        // Lesser Doomguard's hitbox is 3.75 yards
        constexpr float safeDistFromDoomguard = 14.0f;
        if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
            doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
            return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard, minInterval);

        constexpr float safeDistFromPlayer = 6.0f;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
    }

    return false;
}

bool AzgalorMeleeWaitAtSafePositionAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_WAITING_POSITION;
    return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                  position.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AzgalorMoveToDoomguardTankAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 5.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AzgalorFirstAssistTankPositionDoomguardAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    float moveDist = 0.0f;
    bool shouldMove = false;
    bool moveBackwards = false;

    if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard"))
    {
        MarkTargetWithCircle(bot, doomguard);
        SetRtiTarget(botAI, "circle", doomguard);

        if (bot->GetTarget() != doomguard->GetGUID())
            return Attack(doomguard);

        if (doomguard->GetVictim() == bot && bot->IsWithinMeleeRange(doomguard) &&
            distToPosition > 3.0f)
        {
            moveDist = std::min(5.0f, distToPosition);
            shouldMove = true;
            moveBackwards = true;
        }
    }
    else if (distToPosition > 3.0f)
    {
        moveDist = std::min(10.0f, distToPosition);
        shouldMove = true;
        moveBackwards = false;
    }

    if (shouldMove)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, moveBackwards);
    }

    return false;
}

bool AzgalorAssignDpsPriorityAction::Execute(Event /*event*/)
{
    if (botAI->IsRanged(bot))
    {
        if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
            doomguard && bot->GetDistance2d(doomguard) < 40.0f)
        {
            SetRtiTarget(botAI, "circle", doomguard);

            if (bot->GetTarget() != doomguard->GetGUID())
                return Attack(doomguard);
        }
    }
    else if (Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor"))
    {
        SetRtiTarget(botAI, "star", azgalor);

        if (bot->GetTarget() != azgalor->GetGUID())
            return Attack(azgalor);
    }

    return false;
}

// Archimonde

bool ArchimondeMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", archimonde))
        return botAI->CastSpell("steady shot", archimonde);

    return false;
}

bool ArchimondeCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (archimonde->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* spell = archimonde->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id == SPELL_AIR_BURST)
        {
            Player* mainTank = GetGroupMainTank(botAI, bot);
            if (mainTank && spell->m_targets.GetUnitTarget() == mainTank)
            {
                float currentDistance = bot->GetDistance2d(mainTank);
                constexpr float safeDistance = 14.0f;
                if (currentDistance < safeDistance)
                {
                    botAI->Reset();
                    return MoveAway(mainTank, safeDistance - currentDistance);
                }
            }
        }
    }

    constexpr float safeDistFromVictim = 16.0f;
    constexpr float safeDistFromPlayer = 8.0f;
    constexpr uint32 minInterval = 1000;

    Unit* victim = archimonde->GetVictim();
    if (victim && victim != bot && bot->GetExactDist2d(victim) < safeDistFromVictim &&
        FleePosition(victim->GetPosition(), safeDistFromVictim, minInterval))
        return true;

    if (botAI->IsRanged(bot))
    {
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
        if (nearestPlayer &&
            FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval))
            return true;
    }

    return false;
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    std::vector<DoomfireLine> hazardLines;
    std::list<Creature*> doomfires;
    constexpr float searchRadius = 40.0f;

    bot->GetCreatureListWithEntryInGrid(doomfires, NPC_DOOMFIRE, searchRadius);

    for (Creature* creature : doomfires)
    {
        if (creature)
        {
            Position start = creature->GetPosition();
            float destX, destY, destZ;
            if (creature->GetMotionMaster()->GetDestination(destX, destY, destZ))
            {
                Position end(destX, destY, destZ);
                hazardLines.push_back({start, end});
            }
        }
    }

    if (hazardLines.empty())
        return false;

    constexpr float hazardWidth = 15.0f;
    bool inDanger = false;
    for (auto const& line : hazardLines)
    {
        float dist = DistanceToDoomfireLine(bot->GetPosition(), line.start, line.end);
        if (dist < hazardWidth)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Position safePos = FindSafePositionFromDoomfires(bot, hazardLines, hazardWidth);

    float distToSafePos = bot->GetExactDist2d(safePos.GetPositionX(), safePos.GetPositionY());
    if (distToSafePos > 1.0f)
    {
        float dx = safePos.GetPositionX() - bot->GetPositionX();
        float dy = safePos.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToSafePos);
        float moveX = bot->GetPositionX() + (dx / distToSafePos) * moveDist;
        float moveY = bot->GetPositionY() + (dy / distToSafePos) * moveDist;

        Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
        bool backwards = archimonde && archimonde->GetVictim() == bot;
        // Healers need to be allowed to go through fire to reach healing targets so
        // don't set movement priority to forced for them
        MovementPriority movePriority = botAI->IsHeal(bot) ?
            MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, movePriority, true, backwards);
    }

    return false;
}

float ArchimondeAvoidDoomfireAction::DistanceToDoomfireLine(
    const Position& testPosition, const Position& lineStartPosition, const Position& lineEndPosition)
{
    float dx = lineEndPosition.GetPositionX() - lineStartPosition.GetPositionX();
    float dy = lineEndPosition.GetPositionY() - lineStartPosition.GetPositionY();
    float lengthSq = dx*dx + dy*dy;
    if (lengthSq == 0.0f)
        return testPosition.GetExactDist2d(lineStartPosition.GetPositionX(),
                                           lineStartPosition.GetPositionY());

    float projectionFactor =
        ((testPosition.GetPositionX() - lineStartPosition.GetPositionX()) * dx +
         (testPosition.GetPositionY() - lineStartPosition.GetPositionY()) * dy) / lengthSq;
    projectionFactor = std::max(0.0f, std::min(1.0f, projectionFactor));
    float projX = lineStartPosition.GetPositionX() + projectionFactor * dx;
    float projY = lineStartPosition.GetPositionY() + projectionFactor * dy;

    return testPosition.GetExactDist2d(projX, projY);
}

Position ArchimondeAvoidDoomfireAction::FindSafePositionFromDoomfires(
    Player* bot, const std::vector<DoomfireLine>& lines, float hazardWidth)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 40.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos = bot->GetPosition();
    float minMoveDistance = std::numeric_limits<float>::max();

    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::sin(angle);
            float y = bot->GetPositionY() + distance * std::cos(angle);
            float z = bot->GetPositionZ();

            bool isSafe = true;
            for (auto const& line : lines)
            {
                if (DistanceToDoomfireLine(
                    Position(x, y, z), line.start, line.end) < hazardWidth)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            float moveDistance = bot->GetExactDist2d(x, y);
            if (moveDistance < minMoveDistance)
            {
                bestPos = Position(x, y, z);
                minMoveDistance = moveDistance;
            }
        }
    }

    return bestPos;
}

bool ArchimondeRemoveDoomfireDotAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("ice block", bot))
        return botAI->CastSpell("ice block", bot);
    else if (botAI->CanCastSpell("cloak of shadows", bot))
        return botAI->CastSpell("cloak of shadows", bot);
    else if (botAI->CanCastSpell("divine shield", bot))
        return botAI->CastSpell("divine shield", bot);

    return false;
}
