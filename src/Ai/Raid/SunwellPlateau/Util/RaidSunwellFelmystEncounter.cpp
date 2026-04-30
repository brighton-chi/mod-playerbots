/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <cmath>
#include <list>

#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "RaidSunwellFelmystEncounter.h"

namespace SunwellHelpers
{

const Position FELMYST_TANK_POSITION = { 1470.891f, 601.720f, 23.227f };

const std::array<Position, 3> FELMYST_FOG_LEFT_LANES = {{
    { 1494.745f, 704.000f, 50.085f, 4.747f },
    { 1469.923f, 703.239f, 50.086f, 4.747f },
    { 1446.515f, 701.518f, 50.085f, 4.747f },
}};

const std::array<Position, 3> FELMYST_FOG_RIGHT_LANES = {{
    { 1492.820f, 515.668f, 50.083f, 1.449f },
    { 1466.732f, 515.595f, 50.572f, 1.449f },
    { 1441.640f, 520.520f, 50.083f, 1.449f },
}};

const std::array<std::array<Position, 3>, 3> FELMYST_FOG_SAFE_SPOTS = {{
    {{
        { 1466.414f, 598.460f, 22.691f },
        { 1468.840f, 614.437f, 22.460f },
        { 1469.725f, 628.240f, 21.588f },
    }},
    {{
        { 1500.258f, 613.369f, 26.310f },
        { 1500.335f, 596.584f, 25.872f },
        { 1501.360f, 630.656f, 25.482f },
    }},
    {{
        { 1485.415f, 602.346f, 24.075f },
        { 1486.311f, 585.250f, 23.376f },
        { 1488.347f, 619.364f, 24.587f },
    }}
}};

const Position FELMYST_FOG_LEFT_SIDE = { 1469.064f, 729.585f, 59.824f, 4.677f };
const Position FELMYST_FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

const std::array<std::array<Position, 4>, 2> FELMYST_DEMONIC_VAPOR_KITE_PATHS = {{
    {{
        { 1484.994f, 598.407f, 23.859f },
        { 1491.537f, 580.538f, 23.356f },
        { 1475.972f, 565.603f, 22.783f },
        { 1458.482f, 584.163f, 21.248f },
    }},
    {{
        { 1483.642f, 635.021f, 22.168f },
        { 1495.737f, 650.552f, 23.033f },
        { 1480.149f, 663.314f, 20.998f },
        { 1459.241f, 650.507f, 19.350f },
    }}
}};

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystRangedAssignments;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporPathIndices;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporWaypointIndices;
std::unordered_map<uint32, FelmystFogOfCorruptionState> felmystFogOfCorruptionStates;
std::unordered_map<uint32, FelmystIncomingEncapsulateState> felmystIncomingEncapsulateStates;

FelmystFogLocation GetFelmystFogLocationFromLanePointIndex(uint8 laneIndex, bool useLeftPoint)
{
    switch (laneIndex)
    {
        case 0:
            return useLeftPoint ? FelmystFogLocation::LeftTop : FelmystFogLocation::RightTop;
        case 1:
            return useLeftPoint ? FelmystFogLocation::LeftMiddle : FelmystFogLocation::RightMiddle;
        case 2:
            return useLeftPoint ? FelmystFogLocation::LeftBottom : FelmystFogLocation::RightBottom;
        default:
            return FelmystFogLocation::None;
    }
}

FelmystFogLane GetFelmystFogLaneFromLocation(FelmystFogLocation location)
{
    switch (location)
    {
        case FelmystFogLocation::LeftTop:
        case FelmystFogLocation::RightTop:
            return FelmystFogLane::Top;
        case FelmystFogLocation::LeftMiddle:
        case FelmystFogLocation::RightMiddle:
            return FelmystFogLane::Middle;
        case FelmystFogLocation::LeftBottom:
        case FelmystFogLocation::RightBottom:
            return FelmystFogLane::Bottom;
        default:
            return FelmystFogLane::None;
    }
}

bool IsFelmystFogSideLocation(FelmystFogLocation location)
{
    return location == FelmystFogLocation::LeftSide || location == FelmystFogLocation::RightSide;
}

bool IsNearFelmystFogSafeSpot(Player* bot, FelmystFogLane dangerLane, float& closestDistance)
{
    closestDistance = std::numeric_limits<float>::max();
    if (dangerLane == FelmystFogLane::None)
        return false;

    uint8 laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
        return false;

    for (Position const& safeSpot : FELMYST_FOG_SAFE_SPOTS[laneIndex])
    {
        float distance = bot->GetExactDist2d(safeSpot.GetPositionX(), safeSpot.GetPositionY());
        if (distance < closestDistance)
            closestDistance = distance;
    }

    return closestDistance <= FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE;
}

FelmystFogLocation GetFelmystFogLocationFromPosition(
    float positionX, float positionY, float matchDistance)
{
    float bestDistance = matchDistance;
    FelmystFogLocation bestLocation = FelmystFogLocation::None;

    float leftSideDistance = std::hypot(
        positionX - FELMYST_FOG_LEFT_SIDE.GetPositionX(),
        positionY - FELMYST_FOG_LEFT_SIDE.GetPositionY());
    if (leftSideDistance <= bestDistance)
    {
        bestDistance = leftSideDistance;
        bestLocation = FelmystFogLocation::LeftSide;
    }

    float rightSideDistance = std::hypot(
        positionX - FELMYST_FOG_RIGHT_SIDE.GetPositionX(),
        positionY - FELMYST_FOG_RIGHT_SIDE.GetPositionY());
    if (rightSideDistance <= bestDistance)
    {
        bestDistance = rightSideDistance;
        bestLocation = FelmystFogLocation::RightSide;
    }

    for (uint8 laneIndex = 0; laneIndex < FELMYST_FOG_LEFT_LANES.size(); ++laneIndex)
    {
        float leftDistance = std::hypot(
            positionX - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionX(),
            positionY - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionY());
        if (leftDistance <= bestDistance)
        {
            bestDistance = leftDistance;
            bestLocation = GetFelmystFogLocationFromLanePointIndex(laneIndex, true);
        }

        float rightDistance = std::hypot(
            positionX - FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionX(),
            positionY - FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionY());
        if (rightDistance <= bestDistance)
        {
            bestDistance = rightDistance;
            bestLocation = GetFelmystFogLocationFromLanePointIndex(laneIndex, false);
        }
    }

    return bestLocation;
}

bool TryGetFelmystMovementDestination(Unit* felmyst, Position& destination)
{
    if (!felmyst)
        return false;

    float destinationX = 0.0f;
    float destinationY = 0.0f;
    float destinationZ = 0.0f;
    if (!felmyst->GetMotionMaster()->GetDestination(destinationX, destinationY, destinationZ))
        return false;

    destination = Position{ destinationX, destinationY, destinationZ };
    return true;
}

FelmystFogLocation GetFelmystCurrentFogLocation(Unit* felmyst)
{
    if (!felmyst)
        return FelmystFogLocation::None;

    return GetFelmystFogLocationFromPosition(
        felmyst->GetPositionX(), felmyst->GetPositionY(), FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE);
}

FelmystFogLocation GetFelmystDestinationFogLocation(Unit* felmyst)
{
    Position destination;
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return FelmystFogLocation::None;

    return GetFelmystFogLocationFromPosition(
        destination.GetPositionX(), destination.GetPositionY(), FELMYST_FOG_DESTINATION_MATCH_DISTANCE);
}

void EnsureFelmystRangedAssignments(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    auto& assignments = felmystRangedAssignments[bot->GetInstanceId()];

    std::vector<ObjectGuid> invalidAssignments;
    for (auto const& assignment : assignments)
    {
        bool found = false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != assignment.first)
                continue;

            found = botAI->IsRanged(member);
            break;
        }

        if (!found)
            invalidAssignments.push_back(assignment.first);
    }

    for (ObjectGuid const& guid : invalidAssignments)
        assignments.erase(guid);

    uint32 leftCount = 0;
    uint32 rightCount = 0;
    for (auto const& assignment : assignments)
    {
        if (assignment.second == 0)
            ++leftCount;
        else
            ++rightCount;
    }

    std::vector<Player*> priests;
    std::vector<Player*> healers;
    std::vector<Player*> rangedDamage;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (member->getClass() == CLASS_PRIEST)
            priests.push_back(member);
        else if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    auto sortByGuid = [](std::vector<Player*>& members)
    {
        std::sort(members.begin(), members.end(),
            [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    };

    sortByGuid(priests);
    sortByGuid(healers);
    sortByGuid(rangedDamage);

    auto assignMembers = [&](std::vector<Player*> const& members)
    {
        for (Player* member : members)
        {
            uint8 sideIndex = leftCount <= rightCount ? 0 : 1;
            assignments[member->GetGUID()] = sideIndex;

            if (sideIndex == 0)
                ++leftCount;
            else
                ++rightCount;
        }
    };

    assignMembers(priests);
    assignMembers(healers);
    assignMembers(rangedDamage);
}

float GetFelmystFrontAngle(PlayerbotAI* botAI, Player* bot, Unit* felmyst)
{
    float frontX = FELMYST_TANK_POSITION.GetPositionX();
    float frontY = FELMYST_TANK_POSITION.GetPositionY();

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && mainTank->IsAlive() &&
        mainTank->GetMapId() == felmyst->GetMapId())
    {
        frontX = mainTank->GetPositionX();
        frontY = mainTank->GetPositionY();
    }
    else if (Unit* victim = felmyst->GetVictim())
    {
        frontX = victim->GetPositionX();
        frontY = victim->GetPositionY();
    }

    return std::atan2(frontY - felmyst->GetPositionY(), frontX - felmyst->GetPositionX());
}

Creature* GetFelmystDemonicVaporSummonedByBot(Player* carrier)
{
    if (!carrier)
        return nullptr;

    constexpr float searchRadius = 80.0f;
    std::list<Creature*> vapors;
    carrier->GetCreatureListWithEntryInGrid(
        vapors, static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR), searchRadius);

    for (Creature* creature : vapors)
    {
        if (creature && creature->IsAlive() &&
            creature->GetSummonerGUID() == carrier->GetGUID())
        {
            return creature;
        }
    }

    return nullptr;
}

void ClearFelmystDemonicVaporKiteState(Player* bot)
{
    const uint32 instanceId = bot->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    auto pathInstanceItr = felmystDemonicVaporPathIndices.find(instanceId);
    if (pathInstanceItr != felmystDemonicVaporPathIndices.end())
    {
        pathInstanceItr->second.erase(guid);
        if (pathInstanceItr->second.empty())
            felmystDemonicVaporPathIndices.erase(pathInstanceItr);
    }

    auto waypointInstanceItr = felmystDemonicVaporWaypointIndices.find(instanceId);
    if (waypointInstanceItr != felmystDemonicVaporWaypointIndices.end())
    {
        waypointInstanceItr->second.erase(guid);
        if (waypointInstanceItr->second.empty())
            felmystDemonicVaporWaypointIndices.erase(waypointInstanceItr);
    }
}

float GetDistanceToSegment2d(
    float pointX, float pointY, Position const& start, Position const& end)
{
    float startX = start.GetPositionX();
    float startY = start.GetPositionY();
    float endX = end.GetPositionX();
    float endY = end.GetPositionY();
    float deltaX = endX - startX;
    float deltaY = endY - startY;
    float segmentLengthSquared = deltaX * deltaX + deltaY * deltaY;

    if (segmentLengthSquared <= 0.0f)
        return std::hypot(pointX - startX, pointY - startY);

    float projection = ((pointX - startX) * deltaX + (pointY - startY) * deltaY) /
        segmentLengthSquared;
    projection = std::clamp(projection, 0.0f, 1.0f);

    float closestX = startX + projection * deltaX;
    float closestY = startY + projection * deltaY;
    return std::hypot(pointX - closestX, pointY - closestY);
}

float GetDistanceToFelmystDemonicVaporPath(float pointX, float pointY, uint8 pathIndex)
{
    if (pathIndex >= FELMYST_DEMONIC_VAPOR_KITE_PATHS.size())
        return std::numeric_limits<float>::max();

    float bestDistance = std::numeric_limits<float>::max();
    auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
    for (uint8 waypointIndex = 0; waypointIndex < path.size(); ++waypointIndex)
    {
        Position const& start = path[waypointIndex];
        Position const& end = path[(waypointIndex + 1) % path.size()];
        float segmentDistance = GetDistanceToSegment2d(pointX, pointY, start, end);
        if (segmentDistance < bestDistance)
            bestDistance = segmentDistance;
    }

    return bestDistance;
}

uint8 GetNearestFelmystDemonicVaporPathIndex(float pointX, float pointY)
{
    uint8 bestPathIndex = 0;
    float bestDistance = std::numeric_limits<float>::max();

    for (uint8 pathIndex = 0; pathIndex < FELMYST_DEMONIC_VAPOR_KITE_PATHS.size(); ++pathIndex)
    {
        float pathDistance = GetDistanceToFelmystDemonicVaporPath(pointX, pointY, pathIndex);
        if (pathDistance < bestDistance)
        {
            bestDistance = pathDistance;
            bestPathIndex = pathIndex;
        }
    }

    return bestPathIndex;
}

uint8 GetNearestFelmystDemonicVaporWaypointIndex(Player* bot, uint8 pathIndex)
{
    uint8 bestIndex = 0;
    float bestDistance = std::numeric_limits<float>::max();
    auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];

    for (uint8 waypointIndex = 0; waypointIndex < path.size(); ++waypointIndex)
    {
        Position const& waypoint = path[waypointIndex];
        float distance = bot->GetExactDist2d(waypoint.GetPositionX(), waypoint.GetPositionY());
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = waypointIndex;
        }
    }

    return bestIndex;
}

uint8 GetNextFelmystDemonicVaporWaypointIndex(
    Player* bot, uint8 pathIndex, uint8 currentWaypointIndex)
{
    auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
    uint8 waypointIndex = currentWaypointIndex % path.size();
    uint8 nextWaypointIndex = (waypointIndex + 1) % path.size();

    constexpr float waypointReachedDistance = 4.0f;
    Position const& currentWaypoint = path[waypointIndex];
    if (bot->GetExactDist2d(currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY()) <=
        waypointReachedDistance)
    {
        return nextWaypointIndex;
    }

    float currentDistance = bot->GetExactDist2d(
        currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY());
    Position const& nextWaypoint = path[nextWaypointIndex];
    float nextDistance = bot->GetExactDist2d(
        nextWaypoint.GetPositionX(), nextWaypoint.GetPositionY());
    if (nextDistance + 1.0f < currentDistance)
        return nextWaypointIndex;

    return waypointIndex;
}

std::array<uint32, 2> GetFelmystDemonicVaporPathOccupancyCounts(Player* bot)
{
    std::array<uint32, 2> occupancyCounts = { 0, 0 };
    auto pathInstanceItr = felmystDemonicVaporPathIndices.find(bot->GetInstanceId());
    if (pathInstanceItr != felmystDemonicVaporPathIndices.end())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member == bot)
                    continue;

                auto memberPathItr = pathInstanceItr->second.find(member->GetGUID());
                if (memberPathItr == pathInstanceItr->second.end())
                    continue;

                if (!GetFelmystDemonicVaporSummonedByBot(member))
                    continue;

                if (memberPathItr->second < occupancyCounts.size())
                    ++occupancyCounts[memberPathItr->second];
            }
        }
    }

    auto addHazardsOnPath = [&](uint32 entry)
    {
        constexpr float searchRadius = 150.0f;
        constexpr float pathOccupationDistance = 18.0f;
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
        for (Creature* creature : creatures)
        {
            if (!creature || !creature->IsAlive())
                continue;

            if (entry == static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR) &&
                creature->GetSummonerGUID() == bot->GetGUID())
            {
                continue;
            }

            uint8 pathIndex = GetNearestFelmystDemonicVaporPathIndex(
                creature->GetPositionX(), creature->GetPositionY());
            float pathDistance = GetDistanceToFelmystDemonicVaporPath(
                creature->GetPositionX(), creature->GetPositionY(), pathIndex);
            if (pathDistance <= pathOccupationDistance)
                ++occupancyCounts[pathIndex];
        }
    };

    addHazardsOnPath(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR));
    addHazardsOnPath(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL));

    return occupancyCounts;
}

uint8 SelectFelmystDemonicVaporPath(Player* bot)
{
    std::array<uint32, 2> occupancyCounts = GetFelmystDemonicVaporPathOccupancyCounts(bot);
    uint8 bestPathIndex = 0;
    uint32 bestOccupancy = std::numeric_limits<uint32>::max();
    float bestDistance = std::numeric_limits<float>::max();

    for (uint8 pathIndex = 0; pathIndex < FELMYST_DEMONIC_VAPOR_KITE_PATHS.size(); ++pathIndex)
    {
        float pathDistance = GetDistanceToFelmystDemonicVaporPath(
            bot->GetPositionX(), bot->GetPositionY(), pathIndex);
        if (occupancyCounts[pathIndex] < bestOccupancy ||
            (occupancyCounts[pathIndex] == bestOccupancy && pathDistance < bestDistance))
        {
            bestPathIndex = pathIndex;
            bestOccupancy = occupancyCounts[pathIndex];
            bestDistance = pathDistance;
        }
    }

    return bestPathIndex;
}

bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination)
{
    const uint32 instanceId = bot->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    if (!GetFelmystDemonicVaporSummonedByBot(bot))
    {
        ClearFelmystDemonicVaporKiteState(bot);
        return false;
    }

    auto& pathIndices = felmystDemonicVaporPathIndices[instanceId];
    auto& waypointIndices = felmystDemonicVaporWaypointIndices[instanceId];
    auto pathItr = pathIndices.find(guid);
    auto waypointItr = waypointIndices.find(guid);
    uint8 pathIndex = 0;
    uint8 waypointIndex = 0;

    if (pathItr == pathIndices.end())
    {
        pathIndex = SelectFelmystDemonicVaporPath(bot);
        pathIndices[guid] = pathIndex;
    }
    else
    {
        pathIndex = pathItr->second;
    }

    auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
    if (waypointItr == waypointIndices.end())
    {
        waypointIndex = 0;
        waypointIndices[guid] = waypointIndex;
    }
    else
    {
        waypointIndex = GetNextFelmystDemonicVaporWaypointIndex(bot, pathIndex, waypointItr->second);
        waypointIndices[guid] = waypointIndex;
    }

    Position const& waypoint = path[waypointIndex % path.size()];
    destination = Position{ waypoint.GetPositionX(), waypoint.GetPositionY(),
                            waypoint.GetPositionZ(), bot->GetOrientation() };
    return true;
}

bool TryGetFelmystFogOfCorruptionStageState(
    Unit* felmyst, FelmystFogOfCorruptionState& state)
{
    state = FelmystFogOfCorruptionState();
    const uint32 now = getMSTime();

    if (!felmyst)
        return false;

    const uint32 instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying())
    {
        felmystFogOfCorruptionStates.erase(instanceId);
        return false;
    }

    FelmystFogOfCorruptionState& tracker = felmystFogOfCorruptionStates[instanceId];
    bool hasTracker = tracker.phase != FelmystFogPhase::None;
    FelmystFogLocation currentLocation = GetFelmystCurrentFogLocation(felmyst);
    FelmystFogLocation destinationLocation = GetFelmystDestinationFogLocation(felmyst);
    FelmystFogLane currentLane = GetFelmystFogLaneFromLocation(currentLocation);
    FelmystFogLane destinationLane = GetFelmystFogLaneFromLocation(destinationLocation);
    bool isSweeping = felmyst->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FELMYST_SPEED_BURST));

    if (currentLane != FelmystFogLane::None)
    {
        constexpr uint32 fogWindupGraceMs = 7000;
        tracker.lane = currentLane;
        tracker.phase = FelmystFogPhase::Windup;
        tracker.expireMs = now + fogWindupGraceMs;
        state = tracker;
        return true;
    }

    if (isSweeping)
    {
        FelmystFogLane selectedLane = currentLane != FelmystFogLane::None ? currentLane : tracker.lane;
        if (selectedLane == FelmystFogLane::None)
            return false;

        constexpr uint32 fogRecoveryGraceMs = 2500;
        tracker.lane = selectedLane;
        tracker.phase = FelmystFogPhase::Sweep;
        tracker.expireMs = now + fogRecoveryGraceMs;
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FelmystFogLane::None &&
        tracker.phase == FelmystFogPhase::Windup &&
        !IsFelmystFogSideLocation(currentLocation) &&
        !IsFelmystFogSideLocation(destinationLocation))
    {
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FelmystFogLane::None &&
        (tracker.phase == FelmystFogPhase::Sweep ||
            tracker.phase == FelmystFogPhase::Recovery ||
            IsFelmystFogSideLocation(currentLocation) ||
            IsFelmystFogSideLocation(destinationLocation)))
    {
        tracker.phase = FelmystFogPhase::Recovery;
        state = tracker;
        return true;
    }

    felmystFogOfCorruptionStates.erase(instanceId);
    return false;
}

bool TryGetActiveFelmystFogOfCorruptionState(
    Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state)
{
    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, state))
        return false;

    if (state.phase == FelmystFogPhase::Recovery)
        return false;

    float safeSpotDistance = std::numeric_limits<float>::max();
    if (IsNearFelmystFogSafeSpot(bot, state.lane, safeSpotDistance))
        return false;

    return state.lane != FelmystFogLane::None;
}

bool TryGetFelmystFogSafeDestinations(
    Player* bot, FelmystFogLane dangerLane, std::array<Position, 3>& destinations,
    uint8& destinationCount)
{
    destinationCount = 0;
    if (dangerLane == FelmystFogLane::None)
        return false;

    uint8 laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
        return false;

    auto const& safeSpots = FELMYST_FOG_SAFE_SPOTS[laneIndex];
    std::array<uint8, 3> candidateOrder = { 0, 1, 2 };
    std::list<Creature*> vaporHazards;
    auto addVaporHazards = [&](uint32 entry)
    {
        constexpr float searchRadius = 150.0f;
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive())
                vaporHazards.push_back(creature);
        }
    };

    addVaporHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR));
    addVaporHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL));

    auto isSafeSpotBlockedByVapor = [&](Position const& safeSpot)
    {
        constexpr float safeDistanceFromVapor = 10.0f;
        for (Creature* hazard : vaporHazards)
        {
            if (!hazard)
                continue;

            if (hazard->GetExactDist2d(
                    safeSpot.GetPositionX(), safeSpot.GetPositionY()) < safeDistanceFromVapor)
            {
                return true;
            }
        }

        return false;
    };

    std::sort(candidateOrder.begin(), candidateOrder.end(),
        [&](uint8 leftIndex, uint8 rightIndex)
        {
            Position const& left = safeSpots[leftIndex];
            Position const& right = safeSpots[rightIndex];
            return bot->GetExactDist2d(left.GetPositionX(), left.GetPositionY()) <
                bot->GetExactDist2d(right.GetPositionX(), right.GetPositionY());
        });

    for (uint8 candidateIndex : candidateOrder)
    {
        Position const& safeSpot = safeSpots[candidateIndex];
        if (isSafeSpotBlockedByVapor(safeSpot))
            continue;

        float destinationX = safeSpot.GetPositionX();
        float destinationY = safeSpot.GetPositionY();
        float destinationZ = safeSpot.GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                destinationX, destinationY, destinationZ, false))
        {
            continue;
        }

        destinations[destinationCount++] = Position{ destinationX, destinationY, destinationZ };
    }

    return destinationCount > 0;
}

bool TryGetFelmystRangedPosition(PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position)
{
    if (!felmyst)
        return false;

    EnsureFelmystRangedAssignments(botAI, bot);

    auto instanceItr = felmystRangedAssignments.find(bot->GetInstanceId());
    if (instanceItr == felmystRangedAssignments.end())
        return false;

    auto assignmentItr = instanceItr->second.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.end())
        return false;

    constexpr float sideDistance = 22.0f;
    float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);
    float sideAngle = frontAngle + (assignmentItr->second == 0 ? M_PI_2 : -M_PI_2);
    float destinationX = felmyst->GetPositionX() + std::cos(sideAngle) * sideDistance;
    float destinationY = felmyst->GetPositionY() + std::sin(sideAngle) * sideDistance;
    float destinationZ = bot->GetMapWaterOrGroundLevel(destinationX, destinationY, bot->GetPositionZ());
    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        destinationX, destinationY, destinationZ, false);

    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs)
{
    if (!target)
        return;

    FelmystIncomingEncapsulateState& state =
        felmystIncomingEncapsulateStates[target->GetInstanceId()];
    state.targetGuid = target->GetGUID();
    state.expireMs = getMSTime() + durationMs;
    state.auraObserved = false;
}

Player* GetFelmystEncapsulateTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    const uint32 now = getMSTime();
    auto incomingItr = felmystIncomingEncapsulateStates.find(bot->GetInstanceId());
    if (incomingItr != felmystIncomingEncapsulateStates.end())
    {
        Player* incomingTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != incomingItr->second.targetGuid)
                continue;

            incomingTarget = member;
            break;
        }

        if (!incomingTarget)
        {
            felmystIncomingEncapsulateStates.erase(incomingItr);
        }
        else if (incomingTarget->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE)))
        {
            incomingItr->second.auraObserved = true;
            return incomingTarget;
        }
        else if (incomingItr->second.auraObserved || incomingItr->second.expireMs <= now)
        {
            felmystIncomingEncapsulateStates.erase(incomingItr);
        }
        else
        {
            return incomingTarget;
        }
    }

    Player* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() ||
            !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE)))
        {
            continue;
        }

        float distance = bot->GetDistance2d(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

Player* GetFelmystGasNovaDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member ||
            !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_GAS_NOVA)))
        {
            continue;
        }

        float distance = bot->GetDistance(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

Unit* GetNearestFelmystFogOfCorruptionCharmedTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* closestTarget = nullptr;
    float closestDistance = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member ||
            !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
        {
            continue;
        }

        float distance = bot->GetDistance(member);
        if (!closestTarget || distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

Unit* GetNearestFelmystDemonicVaporHazard(Player* bot)
{
    constexpr float searchRadius = 20.0f;
    Unit* nearestTrail = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL), searchRadius, true);
    Unit* nearestVapor = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR), searchRadius, true);

    if (!nearestTrail)
        return nearestVapor;

    if (!nearestVapor)
        return nearestTrail;

    return bot->GetDistance2d(nearestTrail) <= bot->GetDistance2d(nearestVapor) ?
        nearestTrail : nearestVapor;
}

}
