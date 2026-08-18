/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagActions.h"
#include "Creature.h"
#include "MagHelpers.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include <algorithm>
#include <limits>
#include <list>
#include <vector>

using namespace MagHelpers;

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event /*event*/)
{
    Creature* channeler = GetChanneler(bot, SOUTH_CHANNELER);
    if (!channeler)
    {
        if (Creature* channelerW = GetChanneler(bot, WEST_CHANNELER))
            channeler = channelerW;

        if (Creature* channelerE = GetChanneler(bot, EAST_CHANNELER))
            channeler = channelerE;
    }

    if (channeler)
    {
        if (AI_VALUE(Unit*, "current target") != channeler)
            return Attack(channeler);
        return false;
    }

    // After first three channelers are dead, wait for Magtheridon to activate
    Position const& position = WAITING_FOR_MAGTHERIDON_POSITION;
    if (bot->GetExactDist2d(position) <= 2.0f)
    {
        bot->SetFacingTo(position.GetOrientation());
        return true;
    }

    return MoveTo(
        MAG_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonAssistTanksAttackLastTwoChannelersAction::Execute(Event /*event*/)
{
    Creature* channeler = GetChanneler(bot, NORTHWEST_CHANNELER);
    Position const* position = &NW_CHANNELER_TANK_POSITION;
    if (!channeler || !position || !PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        if (Creature* channelerNe = GetChanneler(bot, NORTHEAST_CHANNELER))
        {
            channeler = channelerNe;
            position = &NE_CHANNELER_TANK_POSITION;
        }
    }

    if (!channeler || !position)
        return false;

    if (AI_VALUE(Unit*, "current target") != channeler)
        return Attack(channeler);

    if (channeler->GetVictim() != bot)
        return false;

    float const distToPosition = bot->GetExactDist2d(*position);
    if (distToPosition <= 3.0f)
        return false;

    float const dX = position->GetPositionX() - bot->GetPositionX();
    float const dY = position->GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(distToPosition, 3.5f);
    float const moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

    return MoveTo(
        MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonMisdirectHellfireChannelersToMainTankAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    Creature* targetChanneler = nullptr;
    if (hunterIndex == 0)
        targetChanneler = GetChanneler(bot, WEST_CHANNELER);
    else if (hunterIndex == 1)
        targetChanneler = GetChanneler(bot, EAST_CHANNELER);

    if (!targetChanneler)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(MagSpells::SPELL_MISDIRECTION)))
        return false;

    if (botAI->CanCastSpell("steady shot", targetChanneler))
        return botAI->CastSpell("steady shot", targetChanneler);

    return false;
}

bool MagtheridonAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Creature* channeler = nullptr;
    if (Creature* channelerS = GetChanneler(bot, SOUTH_CHANNELER))
        channeler = channelerS;
    else if (Creature* channelerW = GetChanneler(bot, WEST_CHANNELER))
        channeler = channelerW;
    else if (Creature* channelerE = GetChanneler(bot, EAST_CHANNELER))
        channeler = channelerE;
    else if (Creature* channelerNw = GetChanneler(bot, NORTHWEST_CHANNELER))
        channeler = channelerNw;
    else if (Creature* channelerNe = GetChanneler(bot, NORTHEAST_CHANNELER))
        channeler = channelerNe;

    if (!channeler)
        return false;

    if (AI_VALUE(Unit*, "current target") != channeler)
        return Attack(channeler);

    return MarkTargetWithSkull(bot, channeler);
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonWarlockCcBurningAbyssalAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> abyssals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(MagNpcs::NPC_BURNING_ABYSSAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            abyssals.push_back(creature);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK &&
            GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex >= 0 && warlockIndex < static_cast<int>(abyssals.size()))
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!botAI->HasAura("banish", assignedAbyssal) &&
            botAI->CanCastSpell("banish", assignedAbyssal))
        {
            return botAI->CastSpell("banish", assignedAbyssal);
        }
    }

    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) &&
            !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
        {
            return botAI->CastSpell("fear", excessAbyssal);
        }
    }

    return false;
}

// Main tank will back up to the Eastern point of the room
bool MagtheridonMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    if (AI_VALUE(Unit*, "current target") != magtheridon)
        return Attack(magtheridon);

    if (magtheridon->GetVictim() != bot || !bot->IsWithinMeleeRange(magtheridon) ||
        bot->GetHealthPct() < 50.0f)
    {
        return false;
    }

    Position const& position = MAGTHERIDON_TANK_POSITION;

    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = magtheridon->GetPositionX() - botX;
    float const toBossY = magtheridon->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Ranged stay away from Magtheridon and other players
// Magtheridon's CombatReach is 12 yards and BoundingRadius is 4 yards
bool MagtheridonSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    if (IsCubeClicker(bot))
    {
        auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
        if (timerIt != blastNovaTimer.end() &&
            getMSTimeDiff(timerIt->second, getMSTime()) >= BLAST_NOVA_INTERIM_MS)
        {
            return false;
        }
    }

    constexpr float safeDistFromBoss = 10.0f;
    float const currentDistance = bot->GetDistance2d(magtheridon);
    if (currentDistance < safeDistFromBoss)
        return MoveAway(magtheridon, safeDistFromBoss - currentDistance);

    constexpr float safeDistFromPlayer = 6.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

// For bots that are assigned to click cubes
// Magtheridon casts Blast Nova every 54.35 to 55.40s, with a 2s cast time
bool MagtheridonUseManticronCubeAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    CubeInfo const* cubeInfo = GetAssignedCube();
    if (!cubeInfo)
        return false;

    GameObject* cube = botAI->GetGameObject(cubeInfo->guid);
    if (!cube)
        return false;

    // Release cubes after Blast Nova is interrupted
    if (HandleCubeRelease(magtheridon))
        return true;

    // If Blast Nova is actively casting, always try to click the cube
    if (IsBlastNovaCasting(magtheridon))
        return HandleCubeInteraction(*cubeInfo, cube);

    // Otherwise, if Blast Nova is coming soon, move to and wait near the cube
    return HandleWaitingPhase(*cubeInfo);
}

CubeInfo const* MagtheridonUseManticronCubeAction::GetAssignedCube()
{
    auto mapIt = botToCubeAssignments.find(bot->GetMap()->GetInstanceId());
    if (mapIt == botToCubeAssignments.end())
        return nullptr;

    auto it = mapIt->second.find(bot->GetGUID());
    return it != mapIt->second.end() ? &it->second : nullptr;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (!bot->HasAura(Id(MagSpells::SPELL_SHADOW_GRASP)) ||
        IsBlastNovaCasting(magtheridon))
    {
        return false;
    }

    uint32 delay = urand(200, 3000);
    botAI->AddTimedEvent(
        [this]
        {
            bot->CastStop();
        },
        delay);
    botAI->SetNextCheckDelay(delay + 50);
    return true;
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    auto timerIt = blastNovaTimer.find(bot->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end() ||
        getMSTimeDiff(timerIt->second, getMSTime()) < BLAST_NOVA_INTERIM_MS)
    {
        return false;
    }

    constexpr float safeWaitDistance = 8.0f;

    if (fabs(bot->GetDistance2d(cubeInfo.x, cubeInfo.y) - safeWaitDistance) <= 1.0f)
        return true;

    Position safePos;
    if (!FindSafePositionNearCube(cubeInfo, safeWaitDistance, safePos))
        return false;

    bot->CastStop();
    return MoveTo(
        MAG_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonUseManticronCubeAction::FindSafePositionNearCube(
    CubeInfo const& cubeInfo, float preferredDistance, Position& outPos)
{
    constexpr float angleStep = M_PI / 8.0f;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
    {
        float const x = cubeInfo.x + std::cos(angle) * preferredDistance;
        float const y = cubeInfo.y + std::sin(angle) * preferredDistance;

        if (IsPositionInActiveDebris(bot->GetMap()->GetInstanceId(), x, y))
            continue;

        if (IsPositionInActiveConflagration(botAI, x, y))
            continue;

        float const moveDistance = bot->GetExactDist2d(x, y);
        if (moveDistance < minMoveDistance)
        {
            outPos = Position(x, y, bot->GetPositionZ());
            minMoveDistance = moveDistance;
            foundSafe = true;
        }
    }

    return foundSafe;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(
    CubeInfo const& cubeInfo, GameObject* cube)
{
    constexpr float interactDistance = 1.0f;
    float const cubeDist = bot->GetDistance2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist < interactDistance + 1.0f)
    {
        uint32 delay = urand(200, 1500);
        botAI->AddTimedEvent(
            [this, cube]
            {
                bot->StopMoving();
                cube->Use(bot);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    bot->CastStop();
    return MoveTo(cube, interactDistance, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonMoveOutOfDebrisAction::Execute(Event /*event*/)
{
    Position safePos;
    if (!FindSafePosition(safePos))
        return false;

    bot->CastStop();
    return MoveTo(
        MAG_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(),
        bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MagtheridonMoveOutOfDebrisAction::FindSafePosition(Position& outPos)
{
    constexpr float maxSearchRadius = 20.0f;
    constexpr float distanceStep = 1.0f;
    constexpr float angleStep = M_PI / 12.0f;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;
    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    // Need to remove float loop
    for (float distance = 2.0f; distance <= maxSearchRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
        {
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (IsPositionInActiveDebris(instanceId, x, y))
                continue;

            if (IsPositionInActiveConflagration(botAI, x, y))
                continue;

            float const moveDistance = bot->GetExactDist2d(x, y);

            if (!foundSafe || moveDistance < minMoveDistance)
            {
                outPos = Position(x, y, bot->GetPositionZ());
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }
    }

    return foundSafe;
}

bool MagtheridonManageTimersAndAssignmentsAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    uint32 const instanceId = magtheridon->GetMap()->GetInstanceId();
    uint32 const now = getMSTime();

    if (!lastBlastNovaState[instanceId] && IsBlastNovaCasting(magtheridon))
        blastNovaTimer[instanceId] = now;

    lastBlastNovaState[instanceId] = IsBlastNovaCasting(magtheridon);

    bool updated = false;

    if (IsMagtheridonActive(magtheridon))
    {
        if (blastNovaTimer.try_emplace(instanceId, now).second)
            updated = true;

        if (dpsWaitTimer.try_emplace(instanceId, now).second)
            updated = true;

        if (magtheridon->GetHealthPct() < 30.0f && !ceilingCollapseApplied[instanceId])
        {
            blastNovaTimer[instanceId] += 18 * IN_MILLISECONDS;
            ceilingCollapseApplied[instanceId] = true;
            updated = true;
        }

        if (NeedsCubeReassignment(instanceId) && AssignCubeClickers())
            updated = true;
    }
    else
    {
        if (blastNovaTimer.erase(instanceId) > 0)
            updated = true;

        if (dpsWaitTimer.erase(instanceId) > 0)
            updated = true;

        if (botToCubeAssignments.erase(instanceId) > 0)
            updated = true;

        if (ceilingCollapseApplied.erase(instanceId) > 0)
            updated = true;

        if (lastBlastNovaState.erase(instanceId) > 0)
            updated = true;
    }

    return updated;
}

bool MagtheridonManageTimersAndAssignmentsAction::AssignCubeClickers()
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);

    auto& assignment = botToCubeAssignments[instanceId];
    Group* group = bot->GetGroup();
    if (!group || cubes.empty())
    {
        assignment.clear();
        return true;
    }

    // Prune dead or absent players from the existing assignment
    for (auto it = assignment.begin(); it != assignment.end(); )
    {
        Player* player = ObjectAccessor::FindPlayer(it->first);
        if (!player || !player->IsAlive() || player->GetMapId() != MAG_MAP_ID)
            it = assignment.erase(it);
        else
            ++it;
    }

    // Fill unassigned cubes
    for (CubeInfo const& cube : cubes)
    {
        bool alreadyAssigned = false;
        for (auto const& pair : assignment)
        {
            if (pair.second.guid == cube.guid)
            {
                alreadyAssigned = true;
                break;
            }
        }
        if (alreadyAssigned)
            continue;

        Player* candidate = nullptr;

        // Pass 1: ranged DPS excluding warlocks
        for (GroupReference* ref = group->GetFirstMember();
             ref && !candidate; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !PlayerbotAI::IsRangedDps(member) ||
                !GET_PLAYERBOT_AI(member) || member->getClass() == CLASS_WARLOCK)
            {
                continue;
            }

            if (assignment.find(member->GetGUID()) != assignment.end())
                continue;

            candidate = member;
        }

        // Pass 2: any non-tank bot
        if (!candidate)
        {
            for (GroupReference* ref = group->GetFirstMember();
                 ref && !candidate; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || PlayerbotAI::IsTank(member) ||
                    !GET_PLAYERBOT_AI(member))
                {
                    continue;
                }

                if (assignment.find(member->GetGUID()) != assignment.end())
                    continue;

                candidate = member;
            }
        }

        if (candidate)
            assignment[candidate->GetGUID()] = cube;
    }

    return true;
}

bool MagtheridonManageTimersAndAssignmentsAction::NeedsCubeReassignment(const uint32 instanceId)
{
    auto mapIt = botToCubeAssignments.find(instanceId);
    if (mapIt == botToCubeAssignments.end() || mapIt->second.empty())
        return true;

    for (auto const& pair : mapIt->second)
    {
        Player* assigned = ObjectAccessor::FindPlayer(pair.first);
        if (!assigned || !assigned->IsAlive())
            return true;
    }

    return false;
}

bool MagtheridonEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    bool erased = false;

    if (blastNovaTimer.erase(instanceId) > 0)
        erased = true;
    if (dpsWaitTimer.erase(instanceId) > 0)
        erased = true;
    if (ceilingCollapseApplied.erase(instanceId) > 0)
        erased = true;
    if (lastBlastNovaState.erase(instanceId) > 0)
        erased = true;
    if (botToCubeAssignments.erase(instanceId) > 0)
        erased = true;
    if (activeDebrisPositions.erase(instanceId) > 0)
        erased = true;

    return erased;
}
