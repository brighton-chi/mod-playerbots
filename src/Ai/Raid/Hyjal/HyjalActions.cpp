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
#include <cmath>
#include <iterator>
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// This is essentially a forced "avoid aoe" due to the default AiPlayerbot.MaxAoeAvoidRadius in the
// config being 15 yards; avoidance works fine without this strategy if it is set to 20+ yards.
bool RageWinterchillRangedGetOutOfDeathAndDecayAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetDeathAndDecayPosition(bot, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, DEATH_AND_DECAY_RADIUS, minInterval);
}

// Spread ranged DPS in a circle initially. After the initial spread, movement is free.
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

    // The assigned angle only has to be roughly right--all this is doing is keeping ranged apart--
    // so a point that cannot be reached is worth abandoning for its neighbour rather than walking
    // at forever. Ranged are close enough together that swapping arcs with someone costs nothing
    Position const& position = WINTERCHILL_TANK_POSITION;
    constexpr float moveDist = 3.5f;
    float moveX, moveY, moveZ, chosenX, chosenY;
    if (!FindStepToCircle(bot, position, radius, angle, moveDist, moveX, moveY, moveZ, {},
                          &chosenX, &chosenY))
    {
        // Nowhere on the ring can be reached at all, so settle for where the bot stands rather
        // than spend the fight asking again and contending with everything else that wants to
        // move it. Being spread is a preference here, not a requirement
        _winterchillPositionReached = true;
        return false;
    }

    // Measured against the point actually being walked to. A bot that had to settle for a
    // neighbouring angle is finished when it gets there, not left asking forever for one it
    // cannot reach
    if (bot->GetExactDist2d(chosenX, chosenY) <= 2.0f)
    {
        _winterchillPositionReached = true;
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// When near Death & Decay, melee looks for an open position within the boss's melee range. If one
// isn't available (likely the case if D&D lands on melee, with its 20y radius), then melee just
// takes the shortest path out of the hazard and waits it out.
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

    float const meleeRadius = bot->GetMeleeRange(winterchill) - MELEE_RING_BUFFER;

    std::vector<BlockedArc> blocked;
    BlockedArc poolArc;
    if (GetHazardBlockedArc(
            winterchill->GetPosition(), meleeRadius, pool, DEATH_AND_DECAY_RADIUS, poolArc))
    {
        blocked.push_back(poolArc);
    }

    float const bossX = winterchill->GetPositionX();
    float const bossY = winterchill->GetPositionY();
    float const botHeading = std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX);

    float standAngle;
    if (FindNearestUnblockedAngle(blocked, botHeading, standAngle))
    {
        float const targetX = bossX + std::cos(standAngle) * meleeRadius;
        float const targetY = bossY + std::sin(standAngle) * meleeRadius;
        float const distToTarget = bot->GetExactDist2d(targetX, targetY);

        constexpr float minStepDistance = 0.5f;
        if (distToTarget < minStepDistance)
            return false;

        float const stepDist = std::min(moveDist, distToTarget);
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();

        return MoveTo(
            HYJAL_MAP_ID, botX + ((targetX - botX) / distToTarget) * stepDist,
            botY + ((targetY - botY) / distToTarget) * stepDist, bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float escapeMargin = 2.0f;
    if (!GetHazardEscapeStep(
            bot, pool, DEATH_AND_DECAY_RADIUS + escapeMargin, moveDist, moveX, moveY, moveZ))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Anetheron

bool AnetheronMisdirectBossAndInfernalsToTanksAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* tankTarget = nullptr;
    Unit* enemyTarget = nullptr;
    if (anetheron->GetHealthPct() > 95.0f)
    {
        tankTarget = GetGroupMainTank(botAI, bot);
        enemyTarget = anetheron;
    }
    else if (Unit* infernal = GetLooseInfernal(botAI, bot))
    {
        tankTarget = GetInfernalTank(bot);
        enemyTarget = infernal;
    }

    if (!tankTarget || !enemyTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", enemyTarget))
    {
        return botAI->CastSpell("steady shot", enemyTarget);
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
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

    // The circle was laid out with sin for X and cos for Y here, mirroring Winterchill's
    // convention. Over a full circle of evenly spaced points that maps the set onto itself, so the
    // ring is unchanged and only which bot stands where differs
    constexpr float moveDist = 3.5f;
    float moveX, moveY, moveZ, chosenX, chosenY;
    if (!FindStepToCircle(bot, position, radius, angle, moveDist, moveX, moveY, moveZ, {},
                          &chosenX, &chosenY))
    {
        // As at Winterchill: no reachable angle at all means settle for where the bot stands
        _anetheronPositionReached = true;
        return false;
    }

    if (bot->GetExactDist2d(chosenX, chosenY) <= 2.0f)
    {
        _anetheronPositionReached = true;
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Infernals cannot be taunted, so nothing the tank does will pull one off its victim. What moves
// an Infernal is its victim walking, and the summon itself lands wherever its target stands when
// the 3.5s cast ends. Both cases are the same job: carry it to the gathering spot
bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    Position const& position = GetInfernalTankPosition(bot);
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

// Stand where the Infernals are being gathered and let stock tank assist work through whatever has
// arrived. There is no pick-up to perform: Infernals are immune to taunt, so holding them is plain
// threat work, and the exclusions in the strategy are what keep this bot on them and off Anetheron
bool AnetheronInfernalTankTakePositionAction::Execute(Event /*event*/)
{
    Position const& position = GetInfernalTankPosition(bot);
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    // Backing away from the Infernal being held keeps it in front, so it follows rather than
    // wandering off to the next player it can see. Measured against the one that has the bot, not
    // whatever the bot happens to be swinging at, since tank assist can have it hitting a second
    bool backwards = false;
    if (Unit* held = GetInfernalTargetingBot(botAI, bot))
    {
        float const toHeldX = held->GetPositionX() - botX;
        float const toHeldY = held->GetPositionY() - botY;
        backwards = (toPosX * toHeldX + toPosY * toHeldY) < 0.0f;
    }

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Melee stay on Anetheron throughout. Ranged attack Infernals if they are reasonably nearby.
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    constexpr float safeDistFromInfernal = 10.0f; // Immolation (31303) radius is 10y
    if (Unit* nearest = GetNearestInfernal(botAI, bot))
    {
        constexpr uint32 minInterval = 0;
        if (nearest->GetVictim() != bot &&
            bot->GetDistance2d(nearest) < safeDistFromInfernal)
        {
            return FleePosition(nearest->GetPosition(), safeDistFromInfernal, minInterval);
        }
    }

    if (PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsHeal(bot))
    {
        if (AI_VALUE(Unit*, "current target") != anetheron)
            return Attack(anetheron);

        return false;
    }

    Unit* infernal = GetFocusedInfernal(botAI);
    if (infernal && anetheron->GetHealthPct() > 10.0f &&
        bot->GetDistance2d(infernal) < 50.0f)
    {
        // Wait for the tank to pick up the Infernal before attacking directly
        Player* infernalTank = GetInfernalTank(bot);
        if (!infernalTank || infernal->GetVictim() == infernalTank)
        {
            if (AI_VALUE(Unit*, "current target") != infernal)
                return Attack(infernal);

            return false;
        }
    }

    if (AI_VALUE(Unit*, "current target") != anetheron)
        return Attack(anetheron);

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

// Position is near the gate so the raid can get started on DPS ASAP
bool KazrogalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (AI_VALUE(Unit*, "current target") != kazrogal)
        return Attack(kazrogal);

    if (kazrogal->GetVictim() != bot || !bot->IsWithinMeleeRange(kazrogal))
        return false;

    ObjectGuid const guid = bot->GetGUID();
    TankPositionState state = kazrogalTankStep.count(guid) ?
        kazrogalTankStep[guid] : TankPositionState::MovingToTransition;
    Position const& position = state == TankPositionState::MovingToTransition ?
        KAZROGAL_TANK_TRANSITION_POSITION : KAZROGAL_TANK_FINAL_POSITION;

    constexpr float maxDistance = 2.0f;
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

        return MoveTo(
            HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    if (state == TankPositionState::MovingToTransition)
        kazrogalTankStep[guid] = TankPositionState::MovingToFinal;
    else if (state != TankPositionState::MovingToTransition)
        kazrogalTankStep[guid] = TankPositionState::Positioned;

    return false;
}

// To spread cleave damage
bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    float const distToMainTank = bot->GetExactDist2d(mainTank);
    if (distToMainTank <= 4.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const mtX = mainTank->GetPositionX();
    float const mtY = mainTank->GetPositionY();
    constexpr float maxMoveDist = 3.5f;
    float const moveDist = std::min(maxMoveDist, distToMainTank);
    float const moveX = botX + ((mtX - botX) / distToMainTank) * moveDist;
    float const moveY = botY + ((mtY - botY) / distToMainTank) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, mainTank->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
    constexpr float arcCenter = 4.225f; // measured ingame
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 20.0f;
    float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = kazrogal->GetPositionX() + radius * std::cos(angle);
    float targetY = kazrogal->GetPositionY() + radius * std::sin(angle);

    float const distToTarget = bot->GetExactDist2d(targetX, targetY);
    if (distToTarget <= 0.5f)
        return false;

    constexpr float maxMoveDist = 3.5f;
    float const moveDist = std::min(maxMoveDist, distToTarget);
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const moveX = botX + ((targetX - botX) / distToTarget) * moveDist;
    float const moveY = botY + ((targetY - botY) / distToTarget) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KazrogalLowManaBotTakeDefensiveMeasuresAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            if (!bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER)) &&
                botAI->CanCastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot))
            {
                return botAI->CastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot);
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
                bot->GetPower(POWER_MANA) <= 1200 &&
                botAI->CanCastSpell(Id(HyjalSpells::SPELL_ICE_BLOCK), bot) &&
                botAI->CastSpell(Id(HyjalSpells::SPELL_ICE_BLOCK), bot))
            {
                return true;
            }
            break;

        case CLASS_PALADIN:
            if (bot->HasAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL)) &&
                bot->GetPower(POWER_MANA) <= 1200 &&
                botAI->CanCastSpell(Id(HyjalSpells::SPELL_DIVINE_SHIELD), bot) &&
                botAI->CastSpell(Id(HyjalSpells::SPELL_DIVINE_SHIELD), bot))
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

bool KazrogalWarlockCastShadowWardAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell("shadow ward", bot) && botAI->CastSpell("shadow ward", bot);
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

    if (AI_VALUE(Unit*, "current target") != azgalor)
        return Attack(azgalor);

    if (azgalor->GetVictim() != bot || !bot->IsWithinMeleeRange(azgalor))
        return false;

    ObjectGuid const guid = bot->GetGUID();
    auto it = azgalorTankStep.try_emplace(guid, TankPositionState::MovingToTransition).first;
    TankPositionState state = it->second;
    Position const& position = state == TankPositionState::MovingToTransition ?
        AZGALOR_TANK_TRANSITION_POSITION : AZGALOR_TANK_FINAL_POSITION;

    constexpr float maxDistance = 2.0f;
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

        return MoveTo(
            HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    if (state == TankPositionState::MovingToTransition)
        azgalorTankStep[guid] = TankPositionState::MovingToFinal;
    else if (state != TankPositionState::MovingToTransition)
        azgalorTankStep[guid] = TankPositionState::Positioned;

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
    {
        return true;
    }

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    constexpr float safeDistFromDoomguard = 14.0f;

    if (doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard);

    if (doomguard && AI_VALUE(Unit*, "current target") == doomguard)
        return false;

    constexpr float safeDistFromPlayer = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

// The same question as at Winterchill, with two differences: Azgalor can have more than one pool
// up at a time, and his frontal arc is taken away by the cleave chain whatever the fire is doing.
// Both are just further blocked arcs on the same ring. Cleave safety is never traded against
// standing in fire--fire ticks, cleave kills
bool AzgalorMeleeManueverThroughFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (AI_VALUE(Unit*, "current target") != azgalor)
        return Attack(azgalor);

    std::vector<Position> const pools = GetRainOfFirePositions(bot);
    if (pools.empty())
        return false;

    constexpr float moveDist = 10.0f;
    float moveX;
    float moveY;
    float moveZ;
    float const meleeRadius = bot->GetMeleeRange(azgalor) - MELEE_RING_BUFFER;

    std::vector<BlockedArc> blocked;
    blocked.reserve(pools.size() + 1);

    for (Position const& pool : pools)
    {
        BlockedArc poolArc;
        if (GetHazardBlockedArc(
                azgalor->GetPosition(), meleeRadius, pool, RAIN_OF_FIRE_RADIUS, poolArc))
        {
            blocked.push_back(poolArc);
        }
    }

    // Every ring point sits inside the chain radius of whoever he is hitting, so on this ring the
    // range half of the cleave rule never saves anyone and his frontal arc is simply unavailable
    blocked.push_back({ azgalor->GetOrientation(), CLEAVE_DANGER_ARC / 2.0f });

    float const bossX = azgalor->GetPositionX();
    float const bossY = azgalor->GetPositionY();
    float const botHeading =
        std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX);

    float standAngle;
    if (FindNearestUnblockedAngle(blocked, botHeading, standAngle))
    {
        // Level ground, as at Winterchill, so the step needs no validating--only the unblocked
        // angle decides whether the ring is worth standing on
        float const targetX = bossX + std::cos(standAngle) * meleeRadius;
        float const targetY = bossY + std::sin(standAngle) * meleeRadius;
        float const distToTarget = bot->GetExactDist2d(targetX, targetY);

        // Already standing on the open heading, so hold rather than divide by nothing below
        constexpr float minStepDistance = 0.5f;
        if (distToTarget < minStepDistance)
            return false;

        float const stepDist = std::min(moveDist, distToTarget);
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();

        return MoveTo(
            HYJAL_MAP_ID, botX + ((targetX - botX) / distToTarget) * stepDist,
            botY + ((targetY - botY) / distToTarget) * stepDist, bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Leave the nearest pool, still refusing any heading that would cross into the cleave
    Position const* nearest = nullptr;
    float nearestDistance = 0.0f;
    for (Position const& pool : pools)
    {
        float const distance = bot->GetExactDist2d(pool);
        if (!nearest || distance < nearestDistance)
        {
            nearest = &pool;
            nearestDistance = distance;
        }
    }

    constexpr float escapeMargin = 2.0f;
    auto cleaveSafe = [azgalor](float x, float y)
    { return IsSafeFromAzgalorCleave(azgalor, x, y); };

    if (!GetHazardEscapeStep(
            bot, *nearest, RAIN_OF_FIRE_RADIUS + escapeMargin, moveDist,
            moveX, moveY, moveZ, cleaveSafe))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Wait for the tank to get to the transition position (i.e., move in to attack as
// Azgalor turns away from the raid)
bool AzgalorWaitAtSafePositionAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    bot->AttackStop();
    botAI->InterruptSpell();

    Position const& position = AZGALOR_DOOMGUARD_POSITION;
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

// As at Winterchill, except Azgalor can have more than one pool up, so the nearest is the one to
// leave. Stepping out of it and into another is handled by simply doing this again next tick
bool AzgalorRangedGetOutOfRainOfFireAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetNearestRainOfFirePosition(bot, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, RAIN_OF_FIRE_RADIUS, minInterval);
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
        if (AI_VALUE(Unit*, "current target") != doomguard)
            return Attack(doomguard);

        if (doomguard->GetVictim() != bot || !bot->IsWithinMeleeRange(doomguard))
            return false;

        if (distToPosition <= 3.0f)
            return false;

        float const toDoomguardX = doomguard->GetPositionX() - botX;
        float const toDoomguardY = doomguard->GetPositionY() - botY;
        backwards = (toPosX * toDoomguardX + toPosY * toDoomguardY) < 0.0f;
        shouldMove = true;
    }
    else if (distToPosition > 3.0f)
    {
        // If no Doomguard yet, move to position to wait for it to spawn
        shouldMove = true;
    }
    else
    {
        // If at position and no Doomguard, just wait
        return true;
    }

    if (!shouldMove)
        return false;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Only nearbyish ranged DPS should attack Doomguards; 65 yards should get to the
// side of Azgalor but not bring in any ranged standing in front
bool AzgalorDetermineDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (PlayerbotAI::IsMelee(bot))
    {
        if (AI_VALUE(Unit*, "current target") != azgalor)
            return Attack (azgalor);
        return false;
    }

    Unit* target = nullptr;
    if (azgalor->GetHealthPct() < 10.0f)
    {
        target = azgalor;
    }
    else
    {
        Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
        if (doomguard && bot->GetDistance2d(doomguard) < 60.0f)
            target = doomguard;
        else
            target = azgalor;
    }

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
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

// Air Burst knocks everyone around its target into the air. Losing the whole melee group at once
// is what has to be avoided, since Archimonde turns to a ranged one-shot when nobody is left in
// melee, so a bot standing near the main tank clears out while the cast is still up
bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || bot == mainTank)
        return false;

    // Recorded when the cast begins, so this is only ever answered during the cast
    AirBurstData* data = GetPendingAirBurstCast(bot->GetMap()->GetInstanceId());
    if (!data)
        return false;

    // Only a burst centred on the main tank or on this bot can catch the two of them together
    if (data->targetGuid != mainTank->GetGUID() && data->targetGuid != bot->GetGUID())
        return false;

    float const distanceToMainTank = bot->GetDistance2d(mainTank);
    if (distanceToMainTank >= AIR_BURST_SAFE_DISTANCE)
        return false;

    return MoveAway(mainTank, AIR_BURST_SAFE_DISTANCE - distanceToMainTank);
}

// Opening spread. Ranged start the fight stacked from the run in, and the first Air Burst lands
// 25-35s later, so they are pushed apart while there is still time to do it calmly
bool ArchimondeSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistFromPlayer = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;

    return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    constexpr float dangerDist = 10.0f;
    // Each trail patch is its own dynamic object that expires on its own after 18s, so the live
    // set of them is the trail
    std::vector<Position> const trail = GetDynamicObjectPositions(
        bot, dangerDist, Id(HyjalSpells::SPELL_DOOMFIRE_TRAIL));
    if (trail.empty())
        return false;

    float totalDx = 0.0f;
    float totalDy = 0.0f;
    for (Position const& position : trail)
    {
        float const d = bot->GetExactDist2d(position);

        if (d < dangerDist && d > 0.0f)
        {
            float const weight = (dangerDist - d) / dangerDist;
            totalDx += (bot->GetPositionX() - position.GetPositionX()) / d * weight;
            totalDy += (bot->GetPositionY() - position.GetPositionY()) / d * weight;
        }
    }

    if (totalDx == 0.0f && totalDy == 0.0f)
        return false;

    float const norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    float const moveDist = std::min(norm * dangerDist, dangerDist);
    if (moveDist < 0.5f)
        return false;

    float const targetX = bot->GetPositionX() + (totalDx / norm) * moveDist;
    float const targetY = bot->GetPositionY() + (totalDy / norm) * moveDist;

    MovementPriority const priority = PlayerbotAI::IsHeal(bot) ?
        MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    bool const backwards = archimonde && archimonde->GetVictim() == bot;

    return MoveTo(
        HYJAL_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
        false, false, priority, true, backwards);
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
            return botAI->CanCastSpell(Id(HyjalSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
                botAI->CastSpell(Id(HyjalSpells::SPELL_CLOAK_OF_SHADOWS), bot);

        default:
            return false;
    }
}
