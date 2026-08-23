/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACActions.h"
#include "ACTriggers.h"
#include "Playerbots.h"
#include <algorithm>
#include <iterator>

// Shirrak the Dead Watcher

// Tank will position Shirrak at the specified coordinates, up the stairs
bool ShirrakTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* shirrak = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!shirrak)
        return false;

    if (bot->GetVictim() != shirrak)
        return Attack(shirrak);

    if (shirrak->GetVictim() != bot || !bot->IsWithinMeleeRange(shirrak) ||
        bot->GetHealthPct() < 30.0f)
    {
        return false;
    }

    Position const tankPos = { -65.171f, -162.920f, 26.504f };
    float const distToPosition = bot->GetExactDist2d(tankPos);
    if (distToPosition <= 6.0f)
        return false;

    float const dX = tankPos.GetPositionX() - bot->GetPositionX();
    float const dY = tankPos.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.0f, distToPosition);
    float const moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

    return MoveTo(
        bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool ShirrakFleeFocusFireAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 20.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), searchRadius);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
        {
            float currentDistance = bot->GetDistance2d(flare);
            constexpr float safeDistance = 12.0f;
            constexpr float buffer = 5.0f;

            if (currentDistance < safeDistance)
            {
                bot->CastStop();
                float const distanceToMove = safeDistance - currentDistance + buffer;
                return MoveAway(flare, distanceToMove);
            }
        }
    }

    return false;
}

// Ranged should keep distance from Shirrak, staying at the edge of the stairs
bool ShirrakRangedKeepDistanceAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsRanged(member))
            rangedBots.push_back(member);
    }

    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    size_t botIndex = (findIt != rangedBots.end()) ? std::distance(rangedBots.begin(), findIt) : 0;
    size_t count = rangedBots.size();

    constexpr float arcSpan = M_PI / 2.0f;
    float arcCenter = M_PI;
    float arcStart = arcCenter - (arcSpan / 2.0f);

    float angle = (count <= 1) ?
        arcCenter : (arcStart + (arcSpan * (float)botIndex / (float)(count - 1)));

    constexpr float spreadRadius = 3.0f;

    Position const rangedPos = { -21.777f, -162.700f, 26.062f };
    float targetX = rangedPos.GetPositionX() + cos(angle) * spreadRadius;
    float targetY = rangedPos.GetPositionY() + sin(angle) * spreadRadius;
    float distToSpot = bot->GetExactDist2d(targetX, targetY);

    if (distToSpot <= 4.0f)
        return false;

    float dX = targetX - bot->GetPositionX();
    float dY = targetY - bot->GetPositionY();
    float moveDist = std::min(3.5f, distToSpot);
    float moveX = bot->GetPositionX() + (dX / distToSpot) * moveDist;
    float moveY = bot->GetPositionY() + (dY / distToSpot) * moveDist;

    return MoveTo(
        bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}
