/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"
#include <algorithm>
#include <vector>

using namespace HyjalHelpers;

// General

bool HyjalSummitResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();

    bool erased = false;
    if (PlayerbotAI::IsTank(bot))
    {
        if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        {
            if (kazrogalTankStep.erase(guid) > 0)
                erased = true;

            if (isBelowManaThreshold.erase(guid) > 0)
                erased = true;
        }

        if (!AI_VALUE2(Unit*, "find target", "azgalor") &&
            azgalorTankStep.erase(guid) > 0)
        {
            erased = true;
        }

        return erased;
    }
    else
    {
        if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        {
            Action* action = context->GetAction("rage winterchill spread ranged in circle");
            if (action && static_cast<RageWinterchillSpreadRangedInCircleAction*>(
                    action)->ResetWinterchillPositionReached())
            {
                erased = true;
            }
        }

        if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        {
            Action* action = context->GetAction("anetheron spread ranged in circle");
            if (action && static_cast<AnetheronSpreadRangedInCircleAction*>(
                    action)->ResetAnetheronPositionReached())
            {
                erased = true;
            }
        }

        if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
            isBelowManaThreshold.erase(guid) > 0)
            erased = true;

        return erased;
    }
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

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", winterchill))
    {
        return botAI->CastSpell("steady shot", winterchill);
    }

    return false;
}

// Position is back towards the center of the base to give some more room to manuever
bool RageWinterchillMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (AI_VALUE(Unit*, "current target") != winterchill)
        return Attack(winterchill);

    if (winterchill->GetVictim() != bot || !bot->IsWithinMeleeRange(winterchill))
        return false;

    Position const& position = WINTERCHILL_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 4.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = winterchill->GetPositionX() - botX;
    float const toBossY = winterchill->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Spread ranged DPS in a circle initially--after the initial spread, movement is free
bool RageWinterchillSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    RangedGroups groups = GetRangedGroups(bot);
    if (groups.healers.empty() && groups.rangedDps.empty())
        return false;

    if (_winterchillPositionReached)
        return false;

    auto [botIndex, count] = GetBotCircleIndexAndCount(bot, groups);
    float const radius = PlayerbotAI::IsHeal(bot) ? 25.0f : 35.0f;
    float angle = 0.0f;

    constexpr float arcSpan = 2.0f * M_PI;
    constexpr float arcCenter = 0.0f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    Position const& position = WINTERCHILL_TANK_POSITION;
    float targetX = position.GetPositionX() + radius * std::cos(angle);
    float targetY = position.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
    {
        constexpr float moveDist = 10.0f;
        float moveX;
        float moveY;
        float moveZ;

        if (GetGroundedStepPosition(
            bot, targetX, targetY, moveDist, moveX, moveY, moveZ))
        {
            return MoveTo(
                HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        _winterchillPositionReached = true;
    }

    return false;
}

// Melee prefer a spot that still reaches Winterchill, since only one pool is ever up and it
// rarely covers his whole melee ring. When it does, they give up attacking and head straight out
bool RageWinterchillMeleeGetOutOfDeathAndDecayAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Position pool;
    if (!GetDeathAndDecayPosition(bot, pool))
        return false;

    constexpr float moveDist = 10.0f;
    float moveX, moveY, moveZ;

    struct MeleeSpot
    {
        float x;
        float y;
        float distanceToBot;
    };

    constexpr uint8 angleCount = 16;
    constexpr float angleStep = 2.0f * M_PI / angleCount;
    float const meleeRadius = bot->GetMeleeRange(winterchill) * 0.8f;

    std::vector<MeleeSpot> meleeSpots;
    meleeSpots.reserve(angleCount);

    for (uint8 i = 0; i < angleCount; ++i)
    {
        float const angle = angleStep * i;
        float const x = winterchill->GetPositionX() + std::cos(angle) * meleeRadius;
        float const y = winterchill->GetPositionY() + std::sin(angle) * meleeRadius;

        if (pool.GetExactDist2d(x, y) >= DEATH_AND_DECAY_SAFE_RADIUS)
            meleeSpots.push_back({ x, y, bot->GetExactDist2d(x, y) });
    }

    // Least travel first, so the bot slides around him rather than crossing the pool
    std::sort(meleeSpots.begin(), meleeSpots.end(),
        [](MeleeSpot const& a, MeleeSpot const& b) { return a.distanceToBot < b.distanceToBot; });

    for (MeleeSpot const& spot : meleeSpots)
    {
        if (GetGroundedStepPosition(bot, spot.x, spot.y, moveDist, moveX, moveY, moveZ))
        {
            return MoveTo(HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    // Nothing within reach of him is clear, so give up attacking and leave by the shortest line
    // out of the pool, widening the heading only where that line is blocked
    float const centerX = pool.GetPositionX();
    float const centerY = pool.GetPositionY();
    float escapeAngle =
        std::atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);

    // Dead centre gives no heading of its own, so leave by the side away from Winterchill
    if (bot->GetExactDist2d(centerX, centerY) <= 0.1f)
    {
        escapeAngle = std::atan2(
            centerY - winterchill->GetPositionY(), centerX - winterchill->GetPositionX());
    }

    // Counted rather than accumulated, so the sweep always reaches exactly 180 degrees instead of
    // depending on where eight roundings of an inexact step happen to land
    constexpr uint8 fanSteps = 8;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        float const delta = fanStep * step;
        // Both offsets are the same heading at zero, so only try it once
        uint8 const headings = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < headings; ++i)
        {
            float const angle = escapeAngle + (i == 0 ? delta : -delta);
            float const targetX = centerX + std::cos(angle) * DEATH_AND_DECAY_SAFE_RADIUS;
            float const targetY = centerY + std::sin(angle) * DEATH_AND_DECAY_SAFE_RADIUS;

            if (GetGroundedStepPosition(bot, targetX, targetY, moveDist, moveX, moveY, moveZ))
            {
                return MoveTo(
                    HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
                    MovementPriority::MOVEMENT_COMBAT, true, false);
            }
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

        if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", anetheron))
        {
            return botAI->CastSpell("steady shot", anetheron);
        }
    }

    if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
        infernal && infernal->GetHealthPct() > 50.0f)
    {
        Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
        if (!firstAssistTank)
            return false;

        if (botAI->CanCastSpell("misdirection", firstAssistTank))
            return botAI->CastSpell("misdirection", firstAssistTank);

        if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", infernal))
        {
            return botAI->CastSpell("steady shot", infernal);
        }
    }

    return false;
}

// Position is back towards the center of the base, near the crossroads
bool AnetheronMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (AI_VALUE(Unit*, "current target") != anetheron)
        return Attack(anetheron);

    if (anetheron->GetVictim() != bot || !bot->IsWithinMeleeRange(anetheron))
        return false;

    Position const& position = ANETHERON_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 4.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = anetheron->GetPositionX() - botX;
    float const toBossY = anetheron->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool AnetheronSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    RangedGroups groups = GetRangedGroups(bot);
    if (groups.healers.empty() && groups.rangedDps.empty())
        return false;

    if (_anetheronPositionReached)
    {
        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 2000;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

        return false;
    }

    auto [botIndex, count] = GetBotCircleIndexAndCount(bot, groups);
    float const radius = PlayerbotAI::IsHeal(bot) ? 27.0f : 34.0f;
    float angle = 0.0f;

    constexpr float arcSpan = M_PI * 2.0f;
    constexpr float arcCenter = 0.0f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    Position const& position = ANETHERON_TANK_POSITION;

    float targetX = position.GetPositionX() + radius * std::sin(angle);
    float targetY = position.GetPositionY() + radius * std::cos(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
    {
        constexpr float moveDist = 10.0f;
        float moveX;
        float moveY;
        float moveZ;

        if (GetGroundedStepPosition(
            bot, targetX, targetY, moveDist, moveX, moveY, moveZ))
        {
            return MoveTo(
                HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        _anetheronPositionReached = true;
    }

    return false;
}

// Run to the nearest of two Infernal tanking spots, East and West of Anetheron
bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    Position const& position = GetClosestInfernalTankPosition(bot);
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 2.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    constexpr float maxMoveDist = 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + ((position.GetPositionX() - botX) / distToPosition) * moveDist;
    float const moveY = botY + ((position.GetPositionY() - botY) / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Pick up the Infernal and bring it to the closest Infernal tanking position
bool AnetheronFirstAssistTankPickUpInfernalsAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (infernoTarget && infernoTarget != bot)
    {
        float const distToInfernoTarget = bot->GetExactDist2d(infernoTarget);
        if (distToInfernoTarget > 5.0f)
        {
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            constexpr float maxMoveDist = 3.5f;
            float const moveDist = std::min(maxMoveDist, distToInfernoTarget);
            float const moveX =
                botX + ((infernoTarget->GetPositionX() - botX) / distToInfernoTarget) * moveDist;
            float const moveY =
                botY + ((infernoTarget->GetPositionY() - botY) / distToInfernoTarget) * moveDist;

            return MoveTo(HYJAL_MAP_ID, moveX, moveY, infernoTarget->GetPositionZ(),
                          false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
        return false;

    if (MarkTargetWithDiamond(bot, infernal))
        return true;

    SetRtiTarget(botAI, "diamond", infernal);

    if (AI_VALUE(Unit*, "current target") != infernal)
        return Attack(infernal);

    if ((infernoTarget && infernoTarget == bot) ||
        (infernal->GetVictim() == bot && bot->IsWithinMeleeRange(infernal)))
    {
        Position const& position = GetClosestInfernalTankPosition(bot);
        float const distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > 3.0f)
        {
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            float const toPosX = position.GetPositionX() - botX;
            float const toPosY = position.GetPositionY() - botY;

            float const toInfernalX = infernal->GetPositionX() - botX;
            float const toInfernalY = infernal->GetPositionY() - botY;
            bool const backwards = (toPosX * toInfernalX + toPosY * toInfernalY) < 0.0f;

            float const maxMoveDist = backwards ? 2.25f : 3.5f;
            float const moveDist = std::min(maxMoveDist, distToPosition);
            float const moveX = botX + (toPosX / distToPosition) * moveDist;
            float const moveY = botY + (toPosY / distToPosition) * moveDist;

            return MoveTo(HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }
    }

    return false;
}

// Only nearbyish ranged DPS should attack Infernals
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (PlayerbotAI::IsMelee(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (AI_VALUE(Unit*, "current target") != anetheron)
            return Attack(anetheron);

        return false;
    }
    if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal"))
    {
        constexpr float safeDistFromInfernal = 10.0f;
        constexpr uint32 minInterval = 0;
        if (infernal->GetVictim() != bot &&
            bot->GetDistance2d(infernal) < safeDistFromInfernal)
        {
            return FleePosition(infernal->GetPosition(), safeDistFromInfernal, minInterval);
        }

        if (anetheron->GetHealthPct() > 10.0f && PlayerbotAI::IsRangedDps(bot) &&
            bot->GetDistance2d(infernal) < 50.0f)
        {
            if (Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
                !firstAssistTank || infernal->GetVictim() == firstAssistTank)
            {
                SetRtiTarget(botAI, "diamond", infernal);

                if (AI_VALUE(Unit*, "current target") != infernal)
                    return Attack(infernal);
            }
        }
    }
    else if (PlayerbotAI::IsRangedDps(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (AI_VALUE(Unit*, "current target") != anetheron)
            return Attack(anetheron);
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

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", kazrogal))
    {
        return botAI->CastSpell("steady shot", kazrogal);
    }

    return false;
}

// Position is near the gate so the raid can get start on DPS ASAP
bool KazrogalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (AI_VALUE(Unit*, "current target") != kazrogal)
        return Attack(kazrogal);

    if (kazrogal->GetVictim() == bot && bot->IsWithinMeleeRange(kazrogal))
    {
        ObjectGuid const guid = bot->GetGUID();
        TankPositionState state = kazrogalTankStep.count(guid) ?
            kazrogalTankStep[guid] : TankPositionState::MovingToTransition;

        constexpr float maxDistance = 2.0f;
        Position const& position = state == TankPositionState::MovingToTransition ?
            KAZROGAL_TANK_TRANSITION_POSITION : KAZROGAL_TANK_FINAL_POSITION;
        float const distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance)
        {
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            float const toPosX = position.GetPositionX() - botX;
            float const toPosY = position.GetPositionY() - botY;

            float const toBossX = kazrogal->GetPositionX() - botX;
            float const toBossY = kazrogal->GetPositionY() - botY;
            bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

            float const maxMoveDist = backwards ? 2.25f : 3.5f;
            float const moveDist = std::min(maxMoveDist, distToPosition);
            float const moveX = botX + (toPosX / distToPosition) * moveDist;
            float const moveY = botY + (toPosY / distToPosition) * moveDist;

            return MoveTo(HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }

        if (state == TankPositionState::MovingToTransition && distToPosition <= maxDistance)
        {
            kazrogalTankStep[guid] = TankPositionState::MovingToFinal;
        }
        else if (state != TankPositionState::MovingToTransition &&
                 distToPosition <= maxDistance)
        {
            float const orientation = atan2(kazrogal->GetPositionY() - bot->GetPositionY(),
                                            kazrogal->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            kazrogalTankStep[guid] = TankPositionState::Positioned;
        }
    }

    return false;
}

// To spread cleave damage
bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    float const distToMainTank = bot->GetExactDist2d(mainTank);
    if (distToMainTank > 4.0f)
    {
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();
        constexpr float maxMoveDist = 3.5f;
        float const moveDist = std::min(maxMoveDist, distToMainTank);
        float const moveX =
            botX + ((mainTank->GetPositionX() - botX) / distToMainTank) * moveDist;
        float const moveY =
            botY + ((mainTank->GetPositionY() - botY) / distToMainTank) * moveDist;

        return MoveTo(HYJAL_MAP_ID, moveX, moveY, mainTank->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t count = rangedMembers.size();
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = M_PI / 3.0f;
    constexpr float arcCenter = 4.225f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 20.0f;
    float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = kazrogal->GetPositionX() + radius * std::cos(angle);
    float targetY = kazrogal->GetPositionY() + radius * std::sin(angle);

    // Deadzone has to clear one AI tick of travel, or the final approach commands steps that
    // finish before the bot next thinks and it stutters in
    if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
    {
        constexpr float moveDist = 10.0f;
        float moveX, moveY, moveZ;
        if (GetGroundedStepPosition(bot, targetX, targetY, moveDist,
                                    moveX, moveY, moveZ))
        {
            return MoveTo(HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KazrogalLowManaBotTakeDefensiveMeasuresAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            if (!botAI->HasAura("aspect of the viper", bot) &&
                botAI->CanCastSpell("aspect of the viper", bot))
            {
                return botAI->CastSpell("aspect of the viper", bot);
            }
            return false;

        case CLASS_WARLOCK:
            if (botAI->CanCastSpell("life tap", bot) &&
                botAI->CastSpell("life tap", bot))
            {
                return true;
            }
            break;

        case CLASS_MAGE:
            if (bot->HasAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL)) &&
                bot->GetPower(POWER_MANA) <= 1200 && botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot))
            {
                return true;
            }
            break;

        case CLASS_PALADIN:
            if (bot->HasAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL)) &&
                bot->GetPower(POWER_MANA) <= 1200 && botAI->CanCastSpell("divine shield", bot) &&
                botAI->CastSpell("divine shield", bot))
            {
                return true;
            }
            break;

        default:
            break;
    }

    if (bot->GetPower(POWER_MANA) <= 3200)
        isBelowManaThreshold.try_emplace(bot->GetGUID(), true);

    constexpr float safeDistance = 16.0f;

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    float const currentDistance = bot->GetDistance2d(nearestPlayer);
    if (currentDistance < safeDistance)
    {
        Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
        if (!kazrogal)
            return false;

        if (bot->GetExactDist2d(kazrogal) > 36.0f)
            return MoveAway(nearestPlayer, safeDistance - currentDistance);
        else
            return MoveFromGroup(safeDistance);
    }

    return false;
}

// Warlocks: Use Shadow Ward if Mark is applied and mana is <= 3000
// Paladins: Use Shadow Resistance Aura if Priest Shadow Protection is not up
bool KazrogalCastShadowProtectionSpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_WARLOCK && bot->GetPower(POWER_MANA) <= 3000 &&
        botAI->CanCastSpell("shadow ward", bot))
    {
        return botAI->CastSpell("shadow ward", bot);
    }

    if (bot->getClass() == CLASS_PALADIN &&
        botAI->CanCastSpell("shadow resistance aura", bot))
    {
        return botAI->CastSpell("shadow resistance aura", bot);
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

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", azgalor))
    {
        return botAI->CastSpell("steady shot", azgalor);
    }

    return false;
}

// Two-step move: back up toward the base, then move back toward the base entrance
// to turn Azgalor away from the raid
bool AzgalorMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (MarkTargetWithStar(bot, azgalor))
        return true;

    SetRtiTarget(botAI, "star", azgalor);

    if (AI_VALUE(Unit*, "current target") != azgalor)
        return Attack(azgalor);

    if (azgalor->GetVictim() == bot && bot->IsWithinMeleeRange(azgalor))
    {
        ObjectGuid const guid = bot->GetGUID();
        auto it = azgalorTankStep.try_emplace(
            guid, TankPositionState::MovingToTransition).first;
        TankPositionState state = it->second;

        constexpr float maxDistance = 2.0f;
        Position const& position = state == TankPositionState::MovingToTransition ?
            AZGALOR_TANK_TRANSITION_POSITION : AZGALOR_TANK_FINAL_POSITION;
        float const distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance)
        {
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            float const toPosX = position.GetPositionX() - botX;
            float const toPosY = position.GetPositionY() - botY;

            float const toBossX = azgalor->GetPositionX() - botX;
            float const toBossY = azgalor->GetPositionY() - botY;
            bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

            float const maxMoveDist = backwards ? 2.25f : 3.5f;
            float const moveDist = std::min(maxMoveDist, distToPosition);
            float const moveX = botX + (toPosX / distToPosition) * moveDist;
            float const moveY = botY + (toPosY / distToPosition) * moveDist;

            return MoveTo(HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }

        if (state == TankPositionState::MovingToTransition && distToPosition <= maxDistance)
        {
            azgalorTankStep[guid] = TankPositionState::MovingToFinal;
        }
        else if (state != TankPositionState::MovingToTransition &&
                 distToPosition <= maxDistance)
        {
            float const orientation = atan2(azgalor->GetPositionY() - bot->GetPositionY(),
                                            azgalor->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            azgalorTankStep[guid] = TankPositionState::Positioned;
        }
    }

    return false;
}

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    TankPositionState tankState = GetAzgalorTankPositionState(botAI, bot);
    float const safeDistFromBoss =
        (tankState == TankPositionState::MovingToTransition ? 35.0f : 29.0f);
    constexpr uint32 minInterval = 0;

    if (bot->GetExactDist2d(azgalor) < safeDistFromBoss &&
        FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval))
        return true;

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    constexpr float safeDistFromDoomguard = 14.0f;
    constexpr float safeDistFromPlayer = 5.0f;

    if (doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
    {
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard);
    }
    else if (!doomguard || AI_VALUE(Unit*, "current target") != doomguard)
    {
        Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
        if (nearestPlayer)
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
    }

    return false;
}

// Melee stay on Azgalor whenever his back arc has ground clear of fire, because being behind him
// is immune to the cleave chain at any range. Only when nothing back there is clear do they give
// up attacking. Cleave safety is never traded against standing in fire--fire ticks, cleave kills
bool AzgalorMeleeGetOutOfFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    SetRtiTarget(botAI, "star", azgalor);

    std::vector<Position> const pools = GetRainOfFirePositions(bot);
    if (pools.empty())
        return false;

    constexpr uint8 angleCount = 16;
    constexpr float angleStep = 2.0f * M_PI / angleCount;

    // Tier 0 keeps Azgalor in reach, the outer tiers give that up for clear ground
    constexpr uint8 ringCount = 4;
    constexpr float ringStep = 8.0f;

    struct EscapeCandidate
    {
        float x;
        float y;
        float clearance;  // distance to the nearest pool
        float rank;       // preference within a tier, lower first
        uint8 tier;
    };

    std::vector<EscapeCandidate> candidates;
    candidates.reserve(angleCount * (1 + ringCount));

    float const meleeRadius = bot->GetMeleeRange(azgalor) * 0.8f;

    for (uint8 i = 0; i < angleCount; ++i)
    {
        float const angle = angleStep * i;
        float const x = azgalor->GetPositionX() + std::cos(angle) * meleeRadius;
        float const y = azgalor->GetPositionY() + std::sin(angle) * meleeRadius;
        // Prefer the least travel among the spots that still allow attacking
        candidates.push_back({ x, y, 0.0f, bot->GetExactDist2d(x, y), 0 });
    }

    for (uint8 ring = 1; ring <= ringCount; ++ring)
    {
        for (uint8 i = 0; i < angleCount; ++i)
        {
            float const angle = angleStep * i;
            float const x = bot->GetPositionX() + std::cos(angle) * (ringStep * ring);
            float const y = bot->GetPositionY() + std::sin(angle) * (ringStep * ring);
            // Having given up on attacking, stay as close to him as the ring allows
            candidates.push_back({ x, y, 0.0f, azgalor->GetExactDist2d(x, y), ring });
        }
    }

    // A cleave-unsafe candidate is never an option, so drop them before anything is ranked
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
        [azgalor](EscapeCandidate const& candidate)
        { return !IsSafeFromAzgalorCleave(azgalor, candidate.x, candidate.y); }),
        candidates.end());

    for (EscapeCandidate& candidate : candidates)
    {
        candidate.clearance = HAZARD_SEARCH_RADIUS;
        for (Position const& pool : pools)
        {
            float const distance = pool.GetExactDist2d(candidate.x, candidate.y);
            candidate.clearance = std::min(candidate.clearance, distance);
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](EscapeCandidate const& a, EscapeCandidate const& b)
        { return a.tier != b.tier ? a.tier < b.tier : a.rank < b.rank; });

    constexpr float moveDist = 10.0f;
    float moveX, moveY, moveZ;

    for (EscapeCandidate const& candidate : candidates)
    {
        if (candidate.clearance < RAIN_OF_FIRE_RADIUS)
            continue;

        if (GetGroundedStepPosition(bot, candidate.x, candidate.y, moveDist,
                                    moveX, moveY, moveZ))
        {
            return MoveTo(HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    // The pools cover every safe spot, so head for whatever has the most room. Only worth moving
    // to somewhere with more room than the bot already has, or it shuffles between equally bad
    // ground
    float botClearance = HAZARD_SEARCH_RADIUS;
    for (Position const& pool : pools)
        botClearance = std::min(botClearance, bot->GetExactDist2d(pool));

    std::sort(candidates.begin(), candidates.end(),
        [](EscapeCandidate const& a, EscapeCandidate const& b)
        { return a.clearance > b.clearance; });

    for (EscapeCandidate const& candidate : candidates)
    {
        if (candidate.clearance <= botClearance)
            break;

        if (GetGroundedStepPosition(bot, candidate.x, candidate.y, moveDist,
                                    moveX, moveY, moveZ))
        {
            return MoveTo(HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false,
                          false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Wait for the tank to get to the transition position (i.e., move in to attack as
// Azgalor turns away from the raid)
bool AzgalorWaitAtSafePositionAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    SetRtiTarget(botAI, "star", azgalor);

    bot->AttackStop();
    botAI->InterruptSpell();

    Position const& position = AZGALOR_DOOMGUARD_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition > 2.0f)
    {
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();
        constexpr float maxMoveDist = 3.5f;
        float const moveDist = std::min(maxMoveDist, distToPosition);
        float const moveX = botX + ((position.GetPositionX() - botX) / distToPosition) * moveDist;
        float const moveY = botY + ((position.GetPositionY() - botY) / distToPosition) * moveDist;

        return MoveTo(HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// The spot is between the paths leading from Thrall's keep
bool AzgalorMoveToDoomguardTankAction::Execute(Event /*event*/)
{
    Position const& position = AZGALOR_DOOMGUARD_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 5.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    constexpr float maxMoveDist = 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + ((position.GetPositionX() - botX) / distToPosition) * moveDist;
    float const moveY = botY + ((position.GetPositionY() - botY) / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AzgalorFirstAssistTankPositionDoomguardAction::Execute(Event /*event*/)
{
    Position const& position = AZGALOR_DOOMGUARD_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    bool shouldMove = false;
    bool backwards = false;

    if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard"))
    {
        if (MarkTargetWithCircle(bot, doomguard))
            return true;

        SetRtiTarget(botAI, "circle", doomguard);

        if (AI_VALUE(Unit*, "current target") != doomguard)
            return Attack(doomguard);

        if (doomguard->GetVictim() == bot && bot->IsWithinMeleeRange(doomguard) &&
            distToPosition > 3.0f)
        {
            float const toDoomguardX = doomguard->GetPositionX() - botX;
            float const toDoomguardY = doomguard->GetPositionY() - botY;
            backwards = (toPosX * toDoomguardX + toPosY * toDoomguardY) < 0.0f;
            shouldMove = true;
        }
    }
    else if (distToPosition > 3.0f)
    {
        shouldMove = true;
    }
    else
    {
        return true;
    }

    if (shouldMove)
    {
        float const maxMoveDist = backwards ? 2.25f : 3.5f;
        float const moveDist = std::min(maxMoveDist, distToPosition);
        float const moveX = botX + (toPosX / distToPosition) * moveDist;
        float const moveY = botY + (toPosY / distToPosition) * moveDist;

        return MoveTo(HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    return false;
}

// Only nearbyish ranged DPS should attack Doomguards; 65 yards should get to the
// side of Azgalor but not bring in any ranged standing in front
bool AzgalorRangedDpsPrioritizeDoomguardsAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (azgalor->GetHealthPct() > 10.0f)
    {
        if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
            doomguard && bot->GetDistance2d(doomguard) < 65.0f)
        {
            SetRtiTarget(botAI, "circle", doomguard);

            if (AI_VALUE(Unit*, "current target") != doomguard)
                return Attack(doomguard);
        }
    }
    else
    {
        SetRtiTarget(botAI, "star", azgalor);

        if (AI_VALUE(Unit*, "current target") != azgalor)
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

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", archimonde))
    {
        return botAI->CastSpell("steady shot", archimonde);
    }

    return false;
}

// Initially move Archimonde up the hill a bit to get space from the World Tree
bool ArchimondeMoveBossToInitialPositionAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (AI_VALUE(Unit*, "current target") != archimonde)
        return Attack(archimonde);

    if (archimonde->GetVictim() != bot || !bot->IsWithinMeleeRange(archimonde) ||
        bot->GetHealthPct() < 50.0f)
    {
        return false;
    }

    Position const& position = ARCHIMONDE_INITIAL_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = archimonde->GetPositionX() - botX;
    float const toBossY = archimonde->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool ArchimondeCastFearImmunitySpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PRIEST)
        return CastFearWardOnMainTank();

    return SetTremorTotem();
}

bool ArchimondeCastFearImmunitySpellAction::CastFearWardOnMainTank()
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || mainTank->HasAura(Id(HyjalSpells::SPELL_FEAR_WARD)))
        return false;

    if (!botAI->CanCastSpell(Id(HyjalSpells::SPELL_FEAR_WARD), mainTank))
        return false;

    return botAI->CastSpell(Id(HyjalSpells::SPELL_FEAR_WARD), mainTank);
}

bool ArchimondeCastFearImmunitySpellAction::SetTremorTotem()
{
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    if (!botAI->CanCastSpell(Id(HyjalSpells::SPELL_TREMOR_TOTEM), bot))
        return false;

    return botAI->CastSpell(Id(HyjalSpells::SPELL_TREMOR_TOTEM), bot);
}

// (1) Try to run away from the Air Burst target
// (2) At the beginning of the fight, spread ranged in anticipation of Air Burst
bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && bot != mainTank)
    {
        float const distanceToMainTank = bot->GetDistance2d(mainTank);
        bool shouldMoveFromMainTank = false;
        if (AirBurstData* data = GetRecentArchimondeAirBurst(bot->GetMap()->GetInstanceId()))
        {
            bool isRelevantAirBurstTarget =
                data->targetGuid == mainTank->GetGUID() || data->targetGuid == bot->GetGUID();
            shouldMoveFromMainTank =
                isRelevantAirBurstTarget && distanceToMainTank < AIR_BURST_SAFE_DISTANCE;
        }

        if (!shouldMoveFromMainTank && archimonde->HasUnitState(UNIT_STATE_CASTING))
        {
            Spell* spell = archimonde->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (spell && spell->m_spellInfo->Id ==
                static_cast<uint32>(HyjalSpells::SPELL_AIR_BURST))
            {
                Unit* spellTarget = spell->m_targets.GetUnitTarget();
                if ((spellTarget == mainTank || spellTarget == bot) &&
                    distanceToMainTank < AIR_BURST_SAFE_DISTANCE)
                {
                    shouldMoveFromMainTank = true;
                }
            }
        }

        if (shouldMoveFromMainTank)
            return MoveAway(mainTank, AIR_BURST_SAFE_DISTANCE - distanceToMainTank);
    }

    if (archimonde->GetHealthPct() < 90.0f)
        return false;

    constexpr float safeDistFromPlayer = 10.0f;

    if (PlayerbotAI::IsRanged(bot))
    {
        Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
        if (nearestPlayer &&
            FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer))
        {
            return true;
        }
    }

    return false;
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    constexpr float dangerDist = 10.0f;

    // Each trail patch is its own dynamic object that expires on its own after 18s, so the live
    // set of them is the trail
    std::vector<Position> const trail = GetDynamicObjectPositions(
        bot, dangerDist, static_cast<uint32>(HyjalSpells::SPELL_DOOMFIRE_TRAIL));
    if (trail.empty())
        return false;

    float totalDx = 0.0f, totalDy = 0.0f;
    for (Position const& position : trail)
    {
        float const d = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (d < dangerDist && d > 0.0f)
        {
            float const weight = (dangerDist - d) / dangerDist;
            totalDx += (bot->GetPositionX() - position.GetPositionX()) / d * weight;
            totalDy += (bot->GetPositionY() - position.GetPositionY()) / d * weight;
        }
    }

    if (totalDx != 0.0f || totalDy != 0.0f)
    {
        float const norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
        float const moveDist = std::min(norm * dangerDist, dangerDist);
        if (moveDist < 0.5f)
            return false;

        float const targetX = bot->GetPositionX() + (totalDx / norm) * moveDist;
        float const targetY = bot->GetPositionY() + (totalDy / norm) * moveDist;

        const MovementPriority priority = PlayerbotAI::IsHeal(bot) ?
            MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

        bool const backwards = archimonde->GetVictim() == bot;

        return MoveTo(HYJAL_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, false, priority, true, backwards);
    }

    return false;
}

bool ArchimondeRemoveDoomfireDotAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell(Id(HyjalSpells::SPELL_ICE_BLOCK), bot) &&
                   botAI->CastSpell(Id(HyjalSpells::SPELL_ICE_BLOCK), bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell(Id(HyjalSpells::SPELL_DIVINE_SHIELD), bot) &&
                   botAI->CastSpell(Id(HyjalSpells::SPELL_DIVINE_SHIELD), bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}
