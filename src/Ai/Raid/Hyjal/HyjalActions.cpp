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

// Every mover here walks in short steps rather than handing MoveTo a far destination, and the Z it
// seeds MoveTo with is always the bot's own. That is not a detail: MoveTo seeds SearchForBestPath
// with the Z it is given, and the search resolves a point only a few yards ahead. Passing the
// destination's Z instead asks it to reconcile ground fifty yards away with ground under the bot's
// feet, which on Hyjal's terrain fails outright--MoveTo returns false and the bot simply stands
// there, with no error and nothing suppressing it. The destination's Z belongs in the arrival test,
// never in the step
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

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
        botsBelowManaThreshold.erase(guid))
    {
        erased = true;
    }

    return erased;
}

bool HyjalMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", boss))
    {
        return botAI->CastSpell("steady shot", boss);
    }

    return false;
}

bool HyjalMainTankPositionBossAction::Execute(Event /*event*/)
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

    // Backpedal when the spot lies behind the bot as the boss sees it, so the tank never turns
    // its back on him and swings his frontal arc across the raid. Slower, hence not unconditional
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

// Melee looks for an open position within the boss's melee range. If one isn't available (likely
// the case if D&D lands on melee, with its 20y radius), then melee takes the shortest path out of
// the hazard and waits it out.
//
// Two jobs, since the suppression that comes with this action reaches past the pool itself: inside
// the pool it is an escape, and from there out to DEATH_AND_DECAY_MELEE_CONTROL_RADIUS it is the
// only thing that can walk the bot back onto Winterchill's ring
bool RageWinterchillMeleeManeuverThroughDeathAndDecayAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Position pool;
    if (!GetDeathAndDecayPosition(bot, pool))
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

    // No heading on the ring is open. Fleeing is the answer only while the bot is actually in the
    // pool: the escape aims at a point on a circle drawn round it, so a bot that has already
    // cleared that circle would be walked back inward toward it. Standing still is better--the
    // ring reopens on its own as Winterchill is dragged or the pool expires
    if (!IsInDeathAndDecay(bot))
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

    Player* tankTarget = nullptr;
    Unit* enemyTarget = nullptr;
    if (anetheron->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Melee stay on Anetheron throughout. Ranged attack Infernals if they are reasonably nearby.
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    // Centre to centre, as the Immolation itself measures: it is cast by the creature, which is the
    // case the post-#26967 area check adds no combat reach for. GetDistance would subtract both
    // object sizes, and a Towering Infernal's is not small--the bot would be fleeing from well
    // outside the aura, and since FleePosition reports success on any tick it moves, this would
    // return before reaching the targeting below and leave the bot without a target while it ran
    if (Unit* nearest = GetNearestInfernal(botAI, bot))
    {
        constexpr uint32 minInterval = 0;
        if (nearest->GetVictim() != bot &&
            bot->GetExactDist2d(nearest) < INFERNAL_DANGER_RADIUS)
        {
            return FleePosition(nearest->GetPosition(), INFERNAL_DANGER_RADIUS, minInterval);
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
// CombatReach is 7.875 yards

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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
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
        if (!member || member->GetMapId() != HYJAL_MAP_ID || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    // A bot that is not in the list has no slot, and falling back on index 0 would send it to the
    // first bot's, not to none. Refuse instead: whatever put it here was wrong about it
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (findIt == rangedMembers.end())
        return false;

    size_t const count = rangedMembers.size();
    size_t const botIndex = std::distance(rangedMembers.begin(), findIt);

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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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

    // Away from whoever is nearest. For two bots side by side on the arc that is very nearly
    // tangential, which is the direction that separates a pair fastest. Running straight out from
    // Kaz'rogal instead would barely separate them at all: radial escape scales the gap by the
    // radius, so neighbours a yard and a half apart would have to reach the far side of the base
    // before they made 16.
    //
    // Except where that player is further out than the bot, since away-from-them then points back
    // into the raid. There Kaz'rogal is the reference instead: radially out is at least progress,
    // and it can never be inward. Both branches are MoveAway, which fans nine headings against
    // collision before giving up--worth having where fences and war machines flank every approach
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

    if (!PlayerbotAI::IsHeal(bot)) // Remove to resume dps
        bot->RemoveAura(spellId);

    return botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

// Life Tap first, because it is the only one of the two that removes the problem rather than
// softening it: mana bought back above the danger line keeps the warlock out of the escape
// entirely. Shadow Ward is what is left once health is too low to trade
bool KazrogalWarlockManageManaAction::Execute(Event /*event*/)
{
    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA &&
        bot->GetHealthPct() > sPlayerbotAIConfig.lowHealth &&
        botAI->CanCastSpell("life tap", bot))
    {
        return botAI->CastSpell("life tap", bot);
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

    float const safeDistFromBoss = 30.0f; // ~20 yards + boss and bot CombatReaches
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

    constexpr float safeDistFromPlayer = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

// The same question as at Winterchill, with two differences: Azgalor can have more than one pool
// up at a time, and his frontal arc is taken away by the cleave chain whatever the fire is doing.
// Both are just further blocked arcs on the same ring. Cleave safety is never traded against
// standing in fire--fire ticks, cleave kills
bool AzgalorMeleeManeuverThroughFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    std::vector<Position> const pools = GetRainOfFirePositions(bot);
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

    // No heading on the ring is open. Fleeing is the answer only while the bot is actually in fire:
    // the escape aims at a point on a circle drawn round the pool, so a bot that has already
    // cleared that circle would be walked back inward toward it. Standing still is better--the ring
    // reopens on its own as Azgalor is dragged or the pool expires
    if (!IsInRainOfFire(bot))
        return false;

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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
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
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
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
        if (doomguard && bot->GetExactDist2d(doomguard) < 70.0f)
            target = doomguard;
        else
            target = azgalor;
    }

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
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

    float const distanceToMainTank = bot->GetExactDist2d(mainTank);
    if (distanceToMainTank >= AIR_BURST_SAFE_DISTANCE)
        return false;

    return MoveAway(mainTank, AIR_BURST_SAFE_DISTANCE - distanceToMainTank);
}

// Runs all fight, not just off the pull. Ranged start stacked from the run in and drift back
// together afterwards, and a clump is what turns one Air Burst into a raid-wide one. The interval
// keeps it a periodic nudge rather than something contending for every tick, and the Doomfire
// multiplier takes it out entirely near a trail so it never argues with the avoidance
bool ArchimondeSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, ARCHIMONDE_RANGED_SPREAD_DISTANCE);
    if (!nearestPlayer)
        return false;

    return FleePosition(
        nearestPlayer->GetPosition(), ARCHIMONDE_RANGED_SPREAD_DISTANCE,
        ARCHIMONDE_RANGED_SPREAD_INTERVAL);
}

// Two jobs, because the suppression that comes with this action reaches further than its push does.
// Inside DOOMFIRE_DANGER_RADIUS it shoves the bot clear; from there out to DOOMFIRE_CONTROL_RADIUS
// the push has faded to nothing but no other movement is allowed yet, so this has to be what walks
// the bot back to Archimonde. Without that second half a bot that dodged a trail stands exactly
// where it was pushed while the tank drags him out of reach, and only starts chasing once the trail
// burns out
bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    // Each trail patch is its own dynamic object that expires on its own after 18s, so the live set
    // of them is the trail. The cached set spans DOOMFIRE_SEARCH_RADIUS; the field loop below
    // narrows it to DOOMFIRE_FIELD_RADIUS so patches the bot has not reached yet still get a say in
    // which way it goes, while the trapped sweep reads it whole--a bearing is only worth taking if
    // nothing sits near where it lands, and that includes patches beyond the field
    std::vector<Position> const trail = GetDoomfirePositions(bot);

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

    // Direction comes from the whole field, distance only from what is actually dangerous. Keeping
    // them apart is what holds the two properties the suppression band rests on: the push still
    // fades to exactly nothing at the danger radius, and a dense cluster can no longer sum to a
    // shove longer than the bot needs and carry it clean past the band
    float norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    float moveDist = (nearest && nearestDistance < DOOMFIRE_DANGER_RADIUS) ?
        DOOMFIRE_DANGER_RADIUS - nearestDistance : 0.0f;

    // Boxed in: patches on opposite sides cancel and the field has no direction left to give. That
    // is precisely when holding is worst, because the bot is standing in fire taking damage. Sweep
    // for the bearing whose landing point sits furthest from anything and take it, crossing a patch
    // on the way if that is what it costs. Picking "away from the nearest" instead would only trade
    // one patch for its neighbour and walk back again next tick
    constexpr float minFieldStrength = 0.05f;
    if (nearest && nearestDistance < DOOMFIRE_BURN_RADIUS && norm < minFieldStrength)
    {
        // Two passes, as FindStepToCircle does. Archimonde is fought on a wooded hill, and trees
        // and fallen logs sit in the navmesh--a bearing that is open on the fire alone may not be
        // walkable at all, and MoveTo does not say so: an incomplete path is accepted with the
        // destination quietly replaced by wherever the ray stopped, which can be back in the fire.
        // So prefer a bearing the bot can actually walk, and only when none can be walked take the
        // best of the rest, because moving badly still beats standing here burning
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

                // Ranked rather than vetoed. Boxed in is exactly the case where every bearing fails
                // an outright test, so the question has to be which is least bad, not which passes
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

    // Nothing is pushing the bot, so take over the chase. That this only happens where the action's
    // own suppression would otherwise hold the bot is the trigger's doing--it fires on the same
    // DOOMFIRE_CONTROL_RADIUS the multiplier suppresses at, so past that ordinary movement is free
    // again and there is nothing here to take over from
    //
    // Each side has to be asked in its own units. GetRange hands back the edge-to-edge figure
    // ReachSpellAction is built with, and IsWithinCombatRange is what consumes it--adding both
    // combat reaches. Measured instead against a raw centre-to-centre distance it would walk ranged
    // in by the whole of Archimonde's hitbox, toward the very trail they just dodged. GetMeleeRange
    // already carries both reaches, so the melee side compares centre to centre directly
    bool const inPosition = PlayerbotAI::IsRanged(bot) ?
        bot->IsWithinCombatRange(archimonde, botAI->GetRange("spell")) :
        bot->GetExactDist2d(archimonde) <= bot->GetMeleeRange(archimonde) - MELEE_RANGE_INSET;

    if (inPosition)
        return false;

    float const distToBoss = bot->GetExactDist2d(archimonde);
    if (distToBoss < 0.5f)
        return false;

    // A whole step every time, letting the test above stop it. Trimming the last step to land
    // exactly on the range would need that range back in centre-to-centre terms, which is the
    // conversion this is avoiding
    constexpr float maxMoveDist = 3.5f;
    float const moveX = botX + ((archimonde->GetPositionX() - botX) / distToBoss) * maxMoveDist;
    float const moveY = botY + ((archimonde->GetPositionY() - botY) / distToBoss) * maxMoveDist;

    // A trail lying between the bot and Archimonde is the ordinary case for ranged, not a corner
    // one: it walks the floor they stand off. Stepping into it only to be shoved straight back out
    // is the bounce this whole arrangement exists to prevent, so a step that would land inside the
    // danger radius is simply not taken. Asked of the destination rather than of the direction,
    // which is what makes it exact--the trail blocks the path or it does not
    if (IsPositionNearDoomfire(bot, moveX, moveY, DOOMFIRE_DANGER_RADIUS))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
