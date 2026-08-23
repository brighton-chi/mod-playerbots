/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// General

bool HyjalSummitResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();

    bool erased = false;
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

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        erased |= botsBelowManaThreshold.erase(guid) > 0;

    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        erased |= archimondeAirBurstTargets.erase(bot->GetInstanceId()) > 0;

    return erased;
}

bool HyjalSummitMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", boss) && botAI->CastSpell("steady shot", boss);
}

bool HyjalSummitMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    if (boss->GetVictim() != bot || !bot->IsWithinMeleeRange(boss))
        return false;

    if (bot->GetHealthPct() < _bailBelowHealthPct)
        return false;

    float const distToPosition = bot->GetExactDist2d(_position);
    if (distToPosition <= _arrivalDistance)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = _position.GetPositionX() - botX;
    float const toPosY = _position.GetPositionY() - botY;

    float const toBossX = boss->GetPositionX() - botX;
    float const toBossY = boss->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Rage Winterchill

// This is essentially a forced "avoid aoe" due to the default AiPlayerbot.MaxAoeAvoidRadius in the
// config being 15 yards; avoidance works fine without this strategy if it is set to 20+ yards.
bool RageWinterchillRangedGetOutOfDeathAndDecayAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetDeathAndDecayPosition(botAI, pool))
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

    Position const& position = WINTERCHILL_TANK_POSITION;
    constexpr float moveDist = 3.5f;
    float moveX, moveY, moveZ, chosenX, chosenY;
    if (!FindStepToCircle(bot, position, radius, angle, moveDist, moveX, moveY, moveZ, {},
                          &chosenX, &chosenY))
    {
        _winterchillPositionReached = true;
        return false;
    }

    if (bot->GetExactDist2d(chosenX, chosenY) <= 2.0f)
    {
        _winterchillPositionReached = true;
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Melee looks for an open position within the boss's melee range. If one isn't available (likely
// the case if D&D lands on melee, with its 20y radius), then melee takes the shortest path out of
// the hazard and waits it out.
bool RageWinterchillMeleeManeuverThroughDeathAndDecayAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Position pool;
    if (!GetDeathAndDecayPosition(botAI, pool))
        return false;

    constexpr float moveDist = 10.0f;
    float moveX, moveY, moveZ;

    float const meleeRadius = bot->GetMeleeRange(winterchill) - MELEE_RANGE_INSET;

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

    if (!IsInDeathAndDecay(botAI))
        return false;

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

    Player* tank = nullptr;
    Unit* enemy = nullptr;
    if (anetheron->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
    {
        tank = GetGroupMainTank(bot);
        enemy = anetheron;
    }
    else if (Unit* infernal = GetLooseInfernal(bot))
    {
        tank = GetInfernalTank(bot);
        enemy = infernal;
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", enemy) && botAI->CastSpell("steady shot", enemy);
}

// As with Winterchill, this is just an initial spread, though in the case of Anetheron, bots still
// try to spread a bit throughout the fight because of Carrion Swarm
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

    constexpr float moveDist = 3.5f;
    float moveX, moveY, moveZ, chosenX, chosenY;
    if (!FindStepToCircle(
            bot, position, radius, angle, moveDist, moveX, moveY, moveZ, {}, &chosenX, &chosenY))
    {
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

// Everyone standing near whoever Inferno is aimed at is about to be stunned, since the Infernal
// lands on that player's feet. The 3.5s cast is the whole window, and stepping out of it also
// starts the bot clear of the immolation aura the Infernal carries afterwards
bool AnetheronMoveAwayFromInfernoTargetAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(infernoTarget->GetPosition(), INFERNAL_ESCAPE_DISTANCE, minInterval);
}

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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Note that Infernals cannot be taunted.
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

    bool backwards = false;
    if (Unit* held = GetInfernalTargetingBot(bot))
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Melee stay on Anetheron throughout. Ranged attack Infernals if they are reasonably nearby (50y).
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    if (Unit* nearest = GetNearestInfernal(bot))
    {
        constexpr uint32 minInterval = 0;
        if (nearest->GetVictim() != bot && bot->GetExactDist2d(nearest) < INFERNAL_DANGER_RADIUS)
            return FleePosition(nearest->GetPosition(), INFERNAL_DANGER_RADIUS, minInterval);
    }

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsHeal(bot))
        return AI_VALUE(Unit*, "current target") != anetheron && Attack(anetheron);

    Unit* infernal = GetFocusedInfernal(botAI);
    if (infernal && anetheron->GetHealthPct() > 10.0f && bot->GetDistance2d(infernal) < 50.0f)
    {
        // Wait for the tank to pick up the Infernal before attacking directly
        Player* infernalTank = GetInfernalTank(bot);
        if (!infernalTank || infernal->GetVictim() == infernalTank)
            return AI_VALUE(Unit*, "current target") != infernal && Attack(infernal);
    }

    return AI_VALUE(Unit*, "current target") != anetheron && Attack(anetheron);
}

// Kaz'rogal
// CombatReach is 7.875 yards

bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KazrogalSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != HYJAL_MAP_ID || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (findIt == rangedMembers.end())
        return false;

    size_t const count = rangedMembers.size();
    size_t const botIndex = std::distance(rangedMembers.begin(), findIt);

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    float const arcRadius = GetKazrogalRangedArcRadius(kazrogal);
    float const arcSpan = GetKazrogalRangedArcSpan(arcRadius);
    float const arcStart = KAZROGAL_RANGED_ARC_CENTER - arcSpan / 2.0f;

    float angle = (count == 1) ? KAZROGAL_RANGED_ARC_CENTER :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = kazrogal->GetPositionX() + arcRadius * std::cos(angle);
    float targetY = kazrogal->GetPositionY() + arcRadius * std::sin(angle);

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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KazrogalMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    if (bot->GetPower(POWER_MANA) > MARK_REJOIN_MANA)
    {
        botsBelowManaThreshold.erase(bot->GetGUID());
        return false;
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, MARK_ESCAPE_DISTANCE);
    if (!nearestPlayer)
        return false;

    float const step = MARK_ESCAPE_DISTANCE - bot->GetExactDist2d(nearestPlayer);

    // Away from whoever is nearest. This combination of away from Kaz'rogal and nearest player
    // gets the bots a distance away from the boss before spreading sideways.
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (kazrogal && nearestPlayer->GetExactDist2d(kazrogal) > bot->GetExactDist2d(kazrogal))
        return MoveAway(kazrogal, step);

    return MoveAway(nearestPlayer, step);
}

bool KazrogalActivateAspectOfTheViperAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot) &&
        botAI->CastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot);
}

bool KazrogalCancelMarkAction::Execute(Event /*event*/)
{
    uint32 const spellId = bot->getClass() == CLASS_MAGE
        ? Id(HyjalSpells::SPELL_ICE_BLOCK) : Id(HyjalSpells::SPELL_DIVINE_SHIELD);

    if (!PlayerbotAI::IsHeal(bot)) // Remove bubble/ice block to resume dps immediately
        bot->RemoveAura(spellId);

    return botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

// Life Tap first, then cast Shadow Ward if there isn't enough health to do so
bool KazrogalWarlockManageManaAction::Execute(Event /*event*/)
{
    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA &&
        bot->GetHealthPct() > sPlayerbotAIConfig.lowHealth)
    {
        return botAI->CanCastSpell("life tap", bot) && botAI->CastSpell("life tap", bot);
    }

    if (!HasMarkOfKazrogal(bot))
        return false;

    return botAI->CanCastSpell("shadow ward", bot) && botAI->CastSpell("shadow ward", bot);
}

// Azgalor
// CombatReach is 8.8 yards
// Doomguard CombatReach is 3.75 yards

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    float const safeDistFromBoss = 30.0f; // arbitrary, but ~20 yards + both CombatReaches
    constexpr uint32 minInterval = 0;

    if (bot->GetExactDist2d(azgalor) < safeDistFromBoss &&
        FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval))
    {
        return true;
    }

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    constexpr float safeDistFromDoomguard = 10.0f; // War Stomp is 10 yards center-to-center

    if (doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard);

    if (doomguard && AI_VALUE(Unit*, "current target") == doomguard)
        return false;

    // Don't spread if focused on the Doomguard. There's actually not much space due to the need
    // to maintain significant distance from Azgalor as a proxy to avoid getting in Cleave distance.
    constexpr float safeDistFromPlayer = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

// Similar to D&D avoidance, but there are two notable differences to account for: two RoFs can be
// active at a time, and escape cannot take the bot iinto Azgalor's frontal arc due to the Cleave.
bool AzgalorMeleeManeuverThroughFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    std::vector<Position> const pools = GetRainOfFirePositions(botAI);
    if (pools.empty())
        return false;

    constexpr float moveDist = 10.0f;
    float moveX;
    float moveY;
    float moveZ;
    float const meleeRadius = bot->GetMeleeRange(azgalor) - MELEE_RANGE_INSET;

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

    blocked.push_back({ azgalor->GetOrientation(), CLEAVE_DANGER_ARC / 2.0f });

    float const bossX = azgalor->GetPositionX();
    float const bossY = azgalor->GetPositionY();
    float const botHeading =
        std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX);

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

    if (!IsInRainOfFire(botAI))
        return false;

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
    {
        return IsSafeFromAzgalorCleave(azgalor, x, y);
    };

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

// Like with Winterchill, this is pretty close to a hardcoded AvoidAoeAction.
bool AzgalorRangedGetOutOfRainOfFireAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetNearestRainOfFirePosition(botAI, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, RAIN_OF_FIRE_RADIUS, minInterval);
}

// The spot is about right on top of Thrall's starting position, in order to get Thrall to aggro
// as soon as he is hit.
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
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
        // If no Doomguard is spawned, preemptively move to the tanking position.
        shouldMove = true;
    }
    else
    {
        // If at position and still no Doomguard, just wait.
        return true;
    }

    if (!shouldMove)
        return false;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Only ranged DPS within 70y should attack Doomguards. This distance seems to reach enough bots to
// get decent DPS on the adds but not reach too far (which risks ranged running in front of Azgalor
// to get to the Doomguard or just wasting time).
bool AzgalorDetermineDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (PlayerbotAI::IsMelee(bot))
        return AI_VALUE(Unit*, "current target") != azgalor && Attack (azgalor);

    Unit* target = nullptr;
    if (azgalor->GetHealthPct() < 10.0f)
    {
        target = azgalor;
    }
    else
    {
        constexpr float doomguardEngageDist = 70.0f;
        Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
        if (doomguard && bot->GetExactDist2d(doomguard) < doomguardEngageDist)
            target = doomguard;
        else
            target = azgalor;
    }

    if (!target)
        return false;

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

// Archimonde

bool ArchimondeCastFearImmunitySpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PRIEST)
        return CastFearWardOnMainTank();

    return SetTremorTotem();
}

bool ArchimondeCastFearImmunitySpellAction::CastFearWardOnMainTank()
{
    constexpr uint32 fearWard = Id(HyjalSpells::SPELL_FEAR_WARD);
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || mainTank->HasAura(fearWard))
        return false;

    return botAI->CanCastSpell(fearWard, mainTank) && botAI->CastSpell(fearWard, mainTank);
}

bool ArchimondeCastFearImmunitySpellAction::SetTremorTotem()
{
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    constexpr uint32 tremorTotem = Id(HyjalSpells::SPELL_TREMOR_TOTEM);
    return botAI->CanCastSpell(tremorTotem, bot) && botAI->CastSpell(tremorTotem, bot);
}

// Air Burst knocks everyone around its target into the air. Losing the whole melee group at once
// is what has to be avoided, since Archimonde turns to a ranged one-shot when nobody is left in
// melee range. Thus, the avoidance is to get away from the tank.
bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Unit* activeTank = archimonde->GetVictim();
    if (!activeTank)
        return false;

    AirBurstData airBurst;
    if (!GetPendingAirBurstCast(bot->GetInstanceId(), airBurst))
        return false;

    if (airBurst.targetGuid != activeTank->GetGUID() && airBurst.targetGuid != bot->GetGUID())
        return false;

    float const distanceToActiveTank = bot->GetExactDist2d(activeTank);
    if (distanceToActiveTank >= AIR_BURST_SAFE_DISTANCE)
        return false;

    return MoveAway(activeTank, AIR_BURST_SAFE_DISTANCE - distanceToActiveTank);
}

bool ArchimondeSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, ARCHIMONDE_RANGED_SPREAD_DISTANCE);
    if (!nearestPlayer)
        return false;

    constexpr uint32 minInterval = 3000;
    return FleePosition(
        nearestPlayer->GetPosition(), ARCHIMONDE_RANGED_SPREAD_DISTANCE, minInterval);
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    std::vector<Position> const trail = GetDoomfirePositions(botAI);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    Position const* nearest = nullptr;
    float nearestDistance = 0.0f;
    float totalDx = 0.0f;
    float totalDy = 0.0f;

    for (Position const& patch : trail)
    {
        float const d = bot->GetExactDist2d(patch);
        if (d >= DOOMFIRE_FIELD_RADIUS)
            continue;

        if (!nearest || d < nearestDistance)
        {
            nearest = &patch;
            nearestDistance = d;
        }

        if (d > 0.0f)
        {
            float const weight = (DOOMFIRE_FIELD_RADIUS - d) / DOOMFIRE_FIELD_RADIUS;
            totalDx += (botX - patch.GetPositionX()) / d * weight;
            totalDy += (botY - patch.GetPositionY()) / d * weight;
        }
    }

    float norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    float moveDist = (nearest && nearestDistance < DOOMFIRE_DANGER_RADIUS) ?
        DOOMFIRE_DANGER_RADIUS - nearestDistance : 0.0f;

    constexpr float minFieldStrength = 0.05f;
    if (nearest && nearestDistance < DOOMFIRE_BURN_RADIUS && norm < minFieldStrength)
    {
        constexpr uint8 fanSteps = 12;
        constexpr float escapeStep = 10.0f;
        bool found = false;

        for (uint8 pass = 0; pass < 2 && !found; ++pass)
        {
            bool const validate = (pass == 0);
            float bestClearance = -1.0f;

            for (uint8 i = 0; i < fanSteps; ++i)
            {
                float const angle = 2.0f * static_cast<float>(M_PI) * i / fanSteps;
                float const testX = botX + std::cos(angle) * DOOMFIRE_DANGER_RADIUS;
                float const testY = botY + std::sin(angle) * DOOMFIRE_DANGER_RADIUS;

                float clearance = DOOMFIRE_FIELD_RADIUS;
                for (Position const& patch : trail)
                    clearance = std::min(clearance, patch.GetExactDist2d(testX, testY));

                if (clearance <= bestClearance)
                    continue;

                float stepX = testX;
                float stepY = testY;
                float stepZ = bot->GetPositionZ();
                if (validate &&
                    !CanTakeStepTowards(bot, testX, testY, escapeStep, stepX, stepY, stepZ))
                {
                    continue;
                }

                bestClearance = clearance;
                totalDx = stepX - botX;
                totalDy = stepY - botY;
                found = true;
            }
        }

        norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
        moveDist = norm;
    }

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (norm > 0.0f && moveDist >= 0.5f)
    {
        float const targetX = botX + (totalDx / norm) * moveDist;
        float const targetY = botY + (totalDy / norm) * moveDist;

        MovementPriority const priority = PlayerbotAI::IsHeal(bot) ?
            MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

        bool const backwards = archimonde->GetVictim() == bot;

        return MoveTo(
            HYJAL_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, priority, true, backwards);
    }

    bool const inPosition = PlayerbotAI::IsRanged(bot) ?
        bot->IsWithinCombatRange(archimonde, botAI->GetRange("spell")) :
        bot->GetExactDist2d(archimonde) <= bot->GetMeleeRange(archimonde) - MELEE_RANGE_INSET;

    if (inPosition)
        return false;

    float const distToBoss = bot->GetExactDist2d(archimonde);
    if (distToBoss < 0.5f)
        return false;

    constexpr float maxMoveDist = 3.5f;
    float const moveX = botX + ((archimonde->GetPositionX() - botX) / distToBoss) * maxMoveDist;
    float const moveY = botY + ((archimonde->GetPositionY() - botY) / distToBoss) * maxMoveDist;

    if (IsPositionNearDoomfire(botAI, moveX, moveY, DOOMFIRE_DANGER_RADIUS))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
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
