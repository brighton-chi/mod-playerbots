/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "SWPEncounter_Felmyst.h"

namespace SunwellHelpers
{

const std::array<Position, 3> FELMYST_TANK_POSITIONS = {{
    { 1460.145f, 598.290f, 21.869f },
    { 1480.587f, 636.805f, 21.713f },
    { 1479.524f, 584.069f, 23.231f },
}};

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
    {{ // Top lane safe spots
        { 1466.877f, 562.297f, 22.231f },
        { 1466.718f, 602.838f, 22.834f },
        { 1466.896f, 641.309f, 20.496f },
    }},
    {{ // Middle lane safe spots
        { 1500.352f, 570.684f, 24.830f },
        { 1500.372f, 602.543f, 26.305f },
        { 1500.275f, 639.854f, 24.744f },
    }},
    {{ // Bottom lane safe spots
        { 1484.481f, 568.884f, 23.328f },
        { 1484.491f, 602.682f, 24.015f },
        { 1484.395f, 635.028f, 22.242f },
    }}
}};

const Position FELMYST_FOG_LEFT_SIDE =  { 1469.064f, 729.585f, 59.824f, 4.677f };
const Position FELMYST_FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

const Position FELMYST_HIGH_Y_LANDING_POSITION = { 1476.77f, 665.094f, 20.6423f };
const Position FELMYST_LOW_Y_LANDING_POSITION = { 1469.93f, 557.009f, 22.631699f };
const Position FELMYST_CENTER_GROUND_REFERENCE = { 1473.35f, 611.0515f, 21.637f };

struct FelmystDemonicVaporAnchor
{
    Position position;
    FelmystFogLane lane;
    uint8 sideMask;
};

constexpr uint8 FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE = 0x1;
constexpr uint8 FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE = 0x2;

// Use the fog-lane X bands projected onto each grounded side landing.
const std::array<FelmystDemonicVaporAnchor, 6> FELMYST_DEMONIC_VAPOR_KITE_ANCHORS = {{
    {
        { 1492.820f, FELMYST_LOW_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_LOW_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Top,
                FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE,
    },
    {
        { 1494.745f, FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_HIGH_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Top,
                FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE,
    },
    {
        { 1466.732f, FELMYST_LOW_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_LOW_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Middle,
                FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE,
    },
    {
        { 1469.923f, FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_HIGH_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Middle,
                FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE,
    },
    {
        { 1441.640f, FELMYST_LOW_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_LOW_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Bottom,
                FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE,
    },
    {
        { 1446.515f, FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY(),
          FELMYST_HIGH_Y_LANDING_POSITION.GetPositionZ() },
        FelmystFogLane::Bottom,
                FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE,
    }
}};

const std::array<Position, 3> FELMYST_DEMONIC_VAPOR_LANE_REFERENCES = {{
    { 1493.783f, 609.834f, 50.084f },
    { 1468.328f, 609.417f, 50.329f },
    { 1444.078f, 611.019f, 50.084f },
}};

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    felmystRangedAssignments;

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    felmystDemonicVaporRegionIndices;

std::unordered_map<uint32, uint8>
    felmystDemonicVaporUsedRegionMasks;

std::unordered_map<uint32, uint8>
    felmystDemonicVaporFirstRegionIndices;

std::unordered_map<uint32, FelmystFogOfCorruptionState>
    felmystFogOfCorruptionStates;

std::unordered_map<uint32, FelmystIncomingEncapsulateState>
    felmystIncomingEncapsulateStates;

std::unordered_map<ObjectGuid, FelmystFogCrateStuckState>
    felmystFogCrateStuckStates;

void ResetFelmystDemonicVaporFlightState(uint32 instanceId)
{
    felmystDemonicVaporRegionIndices.erase(instanceId);
    felmystDemonicVaporUsedRegionMasks.erase(instanceId);
    felmystDemonicVaporFirstRegionIndices.erase(instanceId);
}

Creature* GetTrackedFelmyst(Player* bot)
{
    constexpr float searchRadius = 250.0f;
    std::list<Creature*> felmysts;
    bot->GetCreatureListWithEntryInGrid(
        felmysts, static_cast<uint32>(SunwellNpcs::NPC_FELMYST), searchRadius);

    Creature* nearestFelmyst = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (Creature* felmyst : felmysts)
    {
        if (!felmyst || !felmyst->IsAlive())
            continue;

        const float distance = bot->GetDistance2d(felmyst);
        if (distance < nearestDistance)
        {
            nearestFelmyst = felmyst;
            nearestDistance = distance;
        }
    }

    return nearestFelmyst;
}

void ResetFelmystDemonicVaporFlightStateIfGrounded(Player* bot)
{
    Creature* felmyst = GetTrackedFelmyst(bot);
    if (!felmyst || !felmyst->IsFlying())
        ResetFelmystDemonicVaporFlightState(bot->GetInstanceId());
}

bool TryGetFelmystGroundStackCenter(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, FelmystGroundStack stack,
    float& positionX, float& positionY)
{
    if (!felmyst)
        return false;

    switch (stack)
    {
        case FelmystGroundStack::Melee:
        {
            const float behindAngle = Position::NormalizeOrientation(
                GetFelmystFrontAngle(botAI, bot, felmyst) + M_PI);
            positionX = felmyst->GetPositionX() +
                FELMYST_MELEE_DISTANCE * std::cos(behindAngle);
            positionY = felmyst->GetPositionY() +
                FELMYST_MELEE_DISTANCE * std::sin(behindAngle);
            return true;
        }

        case FelmystGroundStack::Left:
        case FelmystGroundStack::Right:
        {
            const float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);
            const float sideAngle = frontAngle +
                (stack == FelmystGroundStack::Left ? M_PI_2 : -M_PI_2);
            positionX = felmyst->GetPositionX() +
                std::cos(sideAngle) * FELMYST_RANGED_SIDE_DISTANCE;
            positionY = felmyst->GetPositionY() +
                std::sin(sideAngle) * FELMYST_RANGED_SIDE_DISTANCE;
            return true;
        }

        default:
            return false;
    }
}

FelmystFogLocation GetFelmystFogLocationFromLanePointIndex(
    uint8 laneIndex, bool useLeftPoint)
{
    switch (laneIndex)
    {
        case 0:
            return useLeftPoint ?
                FelmystFogLocation::LeftTop : FelmystFogLocation::RightTop;
        case 1:
            return useLeftPoint ?
                FelmystFogLocation::LeftMiddle : FelmystFogLocation::RightMiddle;
        case 2:
            return useLeftPoint ?
                FelmystFogLocation::LeftBottom : FelmystFogLocation::RightBottom;
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
    return location == FelmystFogLocation::LeftSide ||
           location == FelmystFogLocation::RightSide;
}

bool IsNearFelmystFogSafeSpot(
    Player* bot, FelmystFogLane dangerLane, float& closestDistance)
{
    closestDistance = std::numeric_limits<float>::max();
    if (dangerLane == FelmystFogLane::None)
        return false;

    const uint8 laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
        return false;

    for (Position const& safeSpot : FELMYST_FOG_SAFE_SPOTS[laneIndex])
    {
        const float distance =
            bot->GetExactDist2d(safeSpot.GetPositionX(), safeSpot.GetPositionY());
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

    const float leftSideDistance = std::hypot(
        positionX - FELMYST_FOG_LEFT_SIDE.GetPositionX(),
        positionY - FELMYST_FOG_LEFT_SIDE.GetPositionY());
    if (leftSideDistance <= bestDistance)
    {
        bestDistance = leftSideDistance;
        bestLocation = FelmystFogLocation::LeftSide;
    }

    const float rightSideDistance = std::hypot(
        positionX - FELMYST_FOG_RIGHT_SIDE.GetPositionX(),
        positionY - FELMYST_FOG_RIGHT_SIDE.GetPositionY());
    if (rightSideDistance <= bestDistance)
    {
        bestDistance = rightSideDistance;
        bestLocation = FelmystFogLocation::RightSide;
    }

    for (uint8 laneIndex = 0; laneIndex < FELMYST_FOG_LEFT_LANES.size(); ++laneIndex)
    {
        const float leftDistance = std::hypot(
            positionX - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionX(),
            positionY - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionY());
        if (leftDistance <= bestDistance)
        {
            bestDistance = leftDistance;
            bestLocation = GetFelmystFogLocationFromLanePointIndex(laneIndex, true);
        }

        const float rightDistance = std::hypot(
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
    if (!felmyst->GetMotionMaster()->GetDestination(
            destinationX, destinationY, destinationZ))
    {
        return false;
    }

    destination = Position{ destinationX, destinationY, destinationZ };
    return true;
}

bool TryGetFelmystLandingDestination(Unit* felmyst, Position& destination)
{
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return false;

    constexpr float landingMatchDistance = 3.0f;
    return destination.GetExactDist2d(
               FELMYST_HIGH_Y_LANDING_POSITION.GetPositionX(),
               FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY()) <= landingMatchDistance ||
           destination.GetExactDist2d(
               FELMYST_LOW_Y_LANDING_POSITION.GetPositionX(),
               FELMYST_LOW_Y_LANDING_POSITION.GetPositionY()) <= landingMatchDistance;
}

FelmystFogLocation GetFelmystCurrentFogLocation(Unit* felmyst)
{
    if (!felmyst)
        return FelmystFogLocation::None;

    return GetFelmystFogLocationFromPosition(
        felmyst->GetPositionX(), felmyst->GetPositionY(),
        FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE);
}

FelmystFogLocation GetFelmystDestinationFogLocation(Unit* felmyst)
{
    Position destination;
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return FelmystFogLocation::None;

    return GetFelmystFogLocationFromPosition(
        destination.GetPositionX(), destination.GetPositionY(),
        FELMYST_FOG_DESTINATION_MATCH_DISTANCE);
}

void EnsureFelmystRangedAssignments(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    auto& assignments = felmystRangedAssignments[bot->GetInstanceId()];
    std::vector<Player*> healers;
    std::vector<Player*> rangedDamage;

    assignments.clear();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    auto const sortByGuid = [](std::vector<Player*>& members)
    {
        std::sort(members.begin(), members.end(),
            [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    };

    sortByGuid(healers);
    sortByGuid(rangedDamage);

    for (uint32 index = 0; index < healers.size(); ++index)
    {
        const FelmystGroundStack stack =
            static_cast<FelmystGroundStack>(index % 3);
        assignments[healers[index]->GetGUID()] = static_cast<uint8>(stack);
    }

    for (uint32 index = 0; index < rangedDamage.size(); ++index)
    {
        const FelmystGroundStack stack =
            index % 2 == 0 ? FelmystGroundStack::Left : FelmystGroundStack::Right;
        assignments[rangedDamage[index]->GetGUID()] = static_cast<uint8>(stack);
    }
}

bool TryGetFelmystGroundStackPosition(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, FelmystGroundStack stack,
    Position& position)
{
    float destinationX = 0.0f;
    float destinationY = 0.0f;
    if (!TryGetFelmystGroundStackCenter(
            botAI, bot, felmyst, stack, destinationX, destinationY))
    {
        return false;
    }

    float destinationZ =
        bot->GetMapWaterOrGroundLevel(destinationX, destinationY, bot->GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        destinationX, destinationY, destinationZ, false);

    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

FelmystGroundStack GetClosestFelmystGroundStack(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, Unit* unit)
{
    if (!unit || !felmyst)
        return FelmystGroundStack::None;

    FelmystGroundStack bestStack = FelmystGroundStack::None;
    float bestDistance = std::numeric_limits<float>::max();
    for (FelmystGroundStack stack : { FelmystGroundStack::Melee,
                                      FelmystGroundStack::Left,
                                      FelmystGroundStack::Right })
    {
        float stackX = 0.0f;
        float stackY = 0.0f;
        if (!TryGetFelmystGroundStackCenter(
                botAI, bot, felmyst, stack, stackX, stackY))
        {
            continue;
        }

        const float stackDistance = unit->GetExactDist2d(stackX, stackY);
        if (stackDistance < bestDistance)
        {
            bestDistance = stackDistance;
            bestStack = stack;
        }
    }

    return bestStack;
}

float GetFelmystFrontAngle(PlayerbotAI* botAI, Player* bot, Unit* felmyst)
{
    Position const& defaultTankPosition = GetFelmystMainTankGroundPosition(bot);
    float frontX = defaultTankPosition.GetPositionX();
    float frontY = defaultTankPosition.GetPositionY();

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

    return std::atan2(
        frontY - felmyst->GetPositionY(), frontX - felmyst->GetPositionX());
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

bool IsFelmystDemonicVaporHeadNearBot(Player* bot)
{
    constexpr float kiteDistanceThreshold = 15.0f;
    Creature* vapor = GetFelmystDemonicVaporSummonedByBot(bot);
    return vapor && bot->GetDistance2d(vapor) <= kiteDistanceThreshold;
}

bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return false;

    // Preserve initial airborne pull targeting until Felmyst drops below the opener threshold.
    if (felmyst->GetHealthPct() > 90.0f)
        return false;

    Position destination;
    if (!TryGetFelmystMovementDestination(felmyst, destination))
        return true;

    constexpr float landingMatchDistance = 3.0f;
    return destination.GetExactDist2d(
               FELMYST_HIGH_Y_LANDING_POSITION.GetPositionX(),
               FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY()) > landingMatchDistance &&
           destination.GetExactDist2d(
               FELMYST_LOW_Y_LANDING_POSITION.GetPositionX(),
               FELMYST_LOW_Y_LANDING_POSITION.GetPositionY()) > landingMatchDistance;
}

void ClearFelmystDemonicVaporKiteState(Player* bot)
{
    const uint32 instanceId = bot->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    auto const regionInstanceItr =
        felmystDemonicVaporRegionIndices.find(instanceId);
    if (regionInstanceItr != felmystDemonicVaporRegionIndices.end())
    {
        regionInstanceItr->second.erase(guid);
        if (regionInstanceItr->second.empty())
            felmystDemonicVaporRegionIndices.erase(regionInstanceItr);
    }

    ResetFelmystDemonicVaporFlightStateIfGrounded(bot);
}

Position const& GetFelmystMainTankGroundPosition(Player* player)
{
    Position const* bestPosition = &FELMYST_TANK_POSITIONS[0];
    float bestDistance = std::numeric_limits<float>::max();

    if (!player)
        return *bestPosition;

    for (Position const& position : FELMYST_TANK_POSITIONS)
    {
        const float distance = player->GetExactDist2d(
            position.GetPositionX(), position.GetPositionY());
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = &position;
        }
    }

    return *bestPosition;
}

FelmystFogLane GetNearestFelmystDemonicVaporLane(Player* bot)
{
    FelmystFogLane bestLane = FelmystFogLane::Middle;
    float bestDistance = std::numeric_limits<float>::max();

    for (uint8 laneIndex = 0; laneIndex < FELMYST_DEMONIC_VAPOR_LANE_REFERENCES.size(); ++laneIndex)
    {
        Position const& reference = FELMYST_DEMONIC_VAPOR_LANE_REFERENCES[laneIndex];
        const float distance = bot->GetExactDist2d(
            reference.GetPositionX(), reference.GetPositionY());
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestLane = static_cast<FelmystFogLane>(laneIndex);
        }
    }

    return bestLane;
}

uint8 GetFelmystDemonicVaporAllowedSides(Player* bot)
{
    const float centerDistance = bot->GetExactDist2d(
        FELMYST_CENTER_GROUND_REFERENCE.GetPositionX(),
        FELMYST_CENTER_GROUND_REFERENCE.GetPositionY());
    const float highYDistance = bot->GetExactDist2d(
        FELMYST_HIGH_Y_LANDING_POSITION.GetPositionX(),
        FELMYST_HIGH_Y_LANDING_POSITION.GetPositionY());
    const float lowYDistance = bot->GetExactDist2d(
        FELMYST_LOW_Y_LANDING_POSITION.GetPositionX(),
        FELMYST_LOW_Y_LANDING_POSITION.GetPositionY());

    if (centerDistance <= highYDistance && centerDistance <= lowYDistance)
        return FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE | FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE;

    return highYDistance <= lowYDistance ?
        FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE : FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE;
}

std::array<FelmystFogLane, 3> GetFelmystDemonicVaporLanePriority(FelmystFogLane lane)
{
    switch (lane)
    {
        case FelmystFogLane::Top:
            return {{ FelmystFogLane::Top, FelmystFogLane::Middle, FelmystFogLane::Bottom }};
        case FelmystFogLane::Bottom:
            return {{ FelmystFogLane::Bottom, FelmystFogLane::Middle, FelmystFogLane::Top }};
        case FelmystFogLane::Middle:
        default:
            return {{ FelmystFogLane::Middle, FelmystFogLane::Top, FelmystFogLane::Bottom }};
    }
}

uint8 GetFelmystDemonicVaporAnchorSide(uint8 anchorIndex)
{
    if (anchorIndex >= FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
        return 0;

    return FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].sideMask;
}

uint8 GetFelmystDemonicVaporAnchorMask(uint8 anchorIndex)
{
    if (anchorIndex >= FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
        return 0;

    return static_cast<uint8>(1u << anchorIndex);
}

float GetDistanceToFelmystDemonicVaporAnchor(Player* bot, uint8 anchorIndex)
{
    if (anchorIndex >= FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
        return std::numeric_limits<float>::max();

    Position const& anchor = FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position;
    return bot->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY());
}

uint8 SelectPreferredFelmystDemonicVaporSide(Player* bot, FelmystFogLane lane, uint8 allowedSides)
{
    if (allowedSides == FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE ||
        allowedSides == FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE)
    {
        return allowedSides;
    }

    float bestLowYDistance = std::numeric_limits<float>::max();
    float bestHighYDistance = std::numeric_limits<float>::max();

    for (uint8 anchorIndex = 0;
         anchorIndex < FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size();
         ++anchorIndex)
    {
        FelmystDemonicVaporAnchor const& anchor =
            FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
        if (anchor.lane != lane)
            continue;

        const float distance = GetDistanceToFelmystDemonicVaporAnchor(bot, anchorIndex);
        if (anchor.sideMask == FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE)
            bestLowYDistance = distance;
        else if (anchor.sideMask == FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE)
            bestHighYDistance = distance;
    }

    return bestLowYDistance <= bestHighYDistance ?
        FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE : FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE;
}

void PushUniqueFelmystDemonicVaporAnchor(
    std::vector<uint8>& anchorIndices, uint8 anchorIndex)
{
    if (std::find(anchorIndices.begin(), anchorIndices.end(), anchorIndex) == anchorIndices.end())
        anchorIndices.push_back(anchorIndex);
}

void AppendFelmystDemonicVaporAnchorsForSide(
    std::vector<uint8>& anchorIndices, FelmystFogLane preferredLane, uint8 sideMask)
{
    auto const lanePriority = GetFelmystDemonicVaporLanePriority(preferredLane);
    for (FelmystFogLane lane : lanePriority)
    {
        for (uint8 anchorIndex = 0;
             anchorIndex < FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size();
             ++anchorIndex)
        {
            FelmystDemonicVaporAnchor const& anchor =
                FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
            if (anchor.sideMask == sideMask && anchor.lane == lane)
                PushUniqueFelmystDemonicVaporAnchor(anchorIndices, anchorIndex);
        }
    }
}

std::vector<Unit*> GetFelmystDemonicVaporHazards(Player* bot)
{
    std::vector<Unit*> hazards;
    constexpr float searchRadius = 100.0f;

    auto const addHazards = [&](uint32 entry)
    {
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

            if (creature->IsAlive())
                hazards.push_back(creature);
        }
    };

    addHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR));
    addHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL));
    return hazards;
}

float GetFelmystMinDistanceToOtherPlayers(Player* bot, float x, float y)
{
    float minDistance = std::numeric_limits<float>::max();
    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* member = it->GetSource();
        if (!member || member == bot || !member->IsAlive() || !member->IsInWorld() ||
            member->GetMapId() != bot->GetMapId())
        {
            continue;
        }

        const float distance = std::hypot(
            x - member->GetPositionX(), y - member->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

float GetFelmystMinDistanceToHazards(
    float x, float y, std::vector<Unit*> const& hazards)
{
    float minDistance = std::numeric_limits<float>::max();
    for (Unit* hazard : hazards)
    {
        if (!hazard)
            continue;

        const float distance = std::hypot(
            x - hazard->GetPositionX(), y - hazard->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

bool IsFelmystDemonicVaporPathSafe(
    Player* bot, Position const& start, Position const& target,
    std::vector<Unit*> const& hazards)
{
    constexpr float pathStepSize = 2.0f;
    constexpr float playerPathClearance = 7.0f;
    constexpr float hazardPathClearance = 10.0f;
    const float totalDistance = start.GetExactDist2d(
        target.GetPositionX(), target.GetPositionY());
    if (totalDistance <= 0.0f)
        return true;

    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (float checkDistance = 0.0f;
         checkDistance <= totalDistance;
         checkDistance += pathStepSize)
    {
        const float t = checkDistance / totalDistance;
        const float checkX = start.GetPositionX() +
            (target.GetPositionX() - start.GetPositionX()) * t;
        const float checkY = start.GetPositionY() +
            (target.GetPositionY() - start.GetPositionY()) * t;

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* member = it->GetSource();
            if (!member || member == bot || !member->IsAlive() || !member->IsInWorld() ||
                member->GetMapId() != bot->GetMapId())
            {
                continue;
            }

            if (std::hypot(checkX - member->GetPositionX(), checkY - member->GetPositionY()) <
                playerPathClearance)
            {
                return false;
            }
        }

        for (Unit* hazard : hazards)
        {
            if (!hazard)
                continue;

            if (std::hypot(checkX - hazard->GetPositionX(), checkY - hazard->GetPositionY()) <
                hazardPathClearance)
            {
                return false;
            }
        }
    }

    return true;
}

bool TryGetFelmystDemonicVaporAnchorDestination(
    Player* bot, uint8 anchorIndex, std::vector<Unit*> const& hazards,
    bool requireSafePath, bool requireSafeEndpoint, Position& destination)
{
    if (anchorIndex >= FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
        return false;

    constexpr float minPlayerEndpointClearance = 8.0f;
    constexpr float minHazardEndpointClearance = 12.0f;

    float destinationX = FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionX();
    float destinationY = FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionY();
    float destinationZ = FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    const float minPlayerDistance =
        GetFelmystMinDistanceToOtherPlayers(bot, destinationX, destinationY);
    const float minHazardDistance =
        GetFelmystMinDistanceToHazards(destinationX, destinationY, hazards);
    if (requireSafeEndpoint &&
        (minPlayerDistance < minPlayerEndpointClearance ||
         minHazardDistance < minHazardEndpointClearance))
    {
        return false;
    }

    Position const candidate(destinationX, destinationY, destinationZ, bot->GetOrientation());
    Position const origin(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        bot->GetOrientation());
    if (requireSafePath &&
        !IsFelmystDemonicVaporPathSafe(bot, origin, candidate, hazards))
    {
        return false;
    }

    destination = candidate;
    return true;
}

bool TryGetFelmystDemonicVaporStepDestination(
    Player* bot, Position const& anchorDestination, Position& destination)
{
    constexpr float stepDistance = 10.0f;
    const float distanceToAnchor = bot->GetExactDist2d(
        anchorDestination.GetPositionX(), anchorDestination.GetPositionY());
    if (distanceToAnchor <= stepDistance)
    {
        destination = anchorDestination;
        return true;
    }

    const float directionX =
        (anchorDestination.GetPositionX() - bot->GetPositionX()) / distanceToAnchor;
    const float directionY =
        (anchorDestination.GetPositionY() - bot->GetPositionY()) / distanceToAnchor;

    float destinationX = bot->GetPositionX() + directionX * stepDistance;
    float destinationY = bot->GetPositionY() + directionY * stepDistance;
    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, anchorDestination.GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    if (bot->GetExactDist2d(destinationX, destinationY) <= 0.01f)
        return false;

    destination = Position(
        destinationX, destinationY, destinationZ, bot->GetOrientation());
    return true;
}

bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination)
{
    const uint32 instanceId = bot->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    ResetFelmystDemonicVaporFlightStateIfGrounded(bot);

    if (!IsFelmystDemonicVaporHeadNearBot(bot))
    {
        ClearFelmystDemonicVaporKiteState(bot);
        return false;
    }

    auto& regionIndices = felmystDemonicVaporRegionIndices[instanceId];
    uint8& usedRegionMask = felmystDemonicVaporUsedRegionMasks[instanceId];
    auto const regionItr = regionIndices.find(guid);
    std::vector<uint8> preferredAnchors;

    if (regionItr != regionIndices.end() &&
        regionItr->second < FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
    {
        PushUniqueFelmystDemonicVaporAnchor(preferredAnchors, regionItr->second);
        AppendFelmystDemonicVaporAnchorsForSide(
            preferredAnchors,
            FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].lane,
            FELMYST_DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].sideMask);
    }
    else
    {
        const FelmystFogLane preferredLane = GetNearestFelmystDemonicVaporLane(bot);
        const uint8 allowedSides = GetFelmystDemonicVaporAllowedSides(bot);
        uint8 preferredSide = SelectPreferredFelmystDemonicVaporSide(
            bot, preferredLane, allowedSides);

        auto const firstRegionItr = felmystDemonicVaporFirstRegionIndices.find(instanceId);
        if (allowedSides == (FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE | FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE) &&
            firstRegionItr != felmystDemonicVaporFirstRegionIndices.end() &&
            firstRegionItr->second < FELMYST_DEMONIC_VAPOR_KITE_ANCHORS.size())
        {
            const uint8 firstSide = GetFelmystDemonicVaporAnchorSide(firstRegionItr->second);
            const uint8 oppositeSide =
                firstSide == FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE ?
                FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE : FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE;
            preferredSide = oppositeSide;
        }

        AppendFelmystDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, preferredSide);

        const uint8 alternateSide =
            preferredSide == FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE ?
            FELMYST_DEMONIC_VAPOR_HIGH_Y_SIDE : FELMYST_DEMONIC_VAPOR_LOW_Y_SIDE;
        if (allowedSides & alternateSide)
            AppendFelmystDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, alternateSide);
    }

    std::vector<Unit*> const hazards = GetFelmystDemonicVaporHazards(bot);
    auto const tryAnchors = [&](bool requireSafePath, bool requireSafeEndpoint)
    {
        for (uint8 anchorIndex : preferredAnchors)
        {
            if (regionItr == regionIndices.end() &&
                (usedRegionMask & GetFelmystDemonicVaporAnchorMask(anchorIndex)) != 0)
            {
                continue;
            }

            Position anchorDestination;
            if (!TryGetFelmystDemonicVaporAnchorDestination(
                    bot, anchorIndex, hazards,
                    requireSafePath, requireSafeEndpoint, anchorDestination))
            {
                continue;
            }

            if (!TryGetFelmystDemonicVaporStepDestination(
                    bot, anchorDestination, destination))
            {
                continue;
            }

            regionIndices[guid] = anchorIndex;
            usedRegionMask |= GetFelmystDemonicVaporAnchorMask(anchorIndex);
            felmystDemonicVaporFirstRegionIndices.try_emplace(instanceId, anchorIndex);
            return true;
        }

        return false;
    };

    if (tryAnchors(true, true) ||
        tryAnchors(false, true) ||
        tryAnchors(false, false))
    {
        return true;
    }

    return false;
}

bool TryGetFelmystFogOfCorruptionStageState(
    Unit* felmyst, FelmystFogOfCorruptionState& state)
{
    state = FelmystFogOfCorruptionState();
    const uint32 now = getMSTime();
    constexpr uint32 fogRecoveryGraceMs = 2500;
    constexpr uint32 fogThirdPassSidePauseGraceMs = 10000;

    if (!felmyst)
        return false;

    const uint32 instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying())
    {
        ResetFelmystDemonicVaporFlightState(instanceId);
        felmystFogOfCorruptionStates.erase(instanceId);
        return false;
    }

    FelmystFogOfCorruptionState& tracker = felmystFogOfCorruptionStates[instanceId];
    const bool hasTracker = tracker.phase != FelmystFogPhase::None;
    const FelmystFogLocation currentLocation = GetFelmystCurrentFogLocation(felmyst);
    const FelmystFogLocation destinationLocation = GetFelmystDestinationFogLocation(felmyst);
    const FelmystFogLane currentLane = GetFelmystFogLaneFromLocation(currentLocation);
    const FelmystFogLane destinationLane = GetFelmystFogLaneFromLocation(destinationLocation);
    const bool isSweeping = felmyst->HasAura(
        static_cast<uint32>(SunwellSpells::SPELL_FELMYST_SPEED_BURST));

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
        FelmystFogLane selectedLane = currentLane !=
            FelmystFogLane::None ? currentLane : tracker.lane;
        if (selectedLane == FelmystFogLane::None)
            return false;

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
        if (tracker.phase == FelmystFogPhase::Sweep)
        {
            ++tracker.completedSweepCount;
            tracker.expireMs = now +
                (tracker.completedSweepCount >= 3 ?
                     fogThirdPassSidePauseGraceMs : fogRecoveryGraceMs);
        }

        tracker.phase = FelmystFogPhase::Recovery;
        state = tracker;
        return true;
    }

    tracker.lane = FelmystFogLane::None;
    tracker.phase = FelmystFogPhase::None;
    tracker.expireMs = 0;
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

    const uint8 laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
        return false;

    auto const& safeSpots = FELMYST_FOG_SAFE_SPOTS[laneIndex];
    std::array<uint8, 3> candidateOrder = { 0, 1, 2 };
    std::list<Creature*> vaporHazards;
    auto const addVaporHazards = [&](uint32 entry)
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

    auto const isSafeSpotBlockedByVapor = [&](Position const& safeSpot)
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

bool TryGetFelmystRangedPosition(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position)
{
    if (!felmyst)
        return false;

    EnsureFelmystRangedAssignments(botAI, bot);

    auto const instanceItr = felmystRangedAssignments.find(bot->GetInstanceId());
    if (instanceItr == felmystRangedAssignments.end())
        return false;

    auto const assignmentItr = instanceItr->second.find(bot->GetGUID());
    if (assignmentItr == instanceItr->second.end())
        return false;

    return TryGetFelmystGroundStackPosition(
        botAI, bot, felmyst,
        static_cast<FelmystGroundStack>(assignmentItr->second), position);
}

void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs)
{
    if (!target)
        return;

    const uint32 now = getMSTime();
    FelmystIncomingEncapsulateState& state =
        felmystIncomingEncapsulateStates[target->GetInstanceId()];

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + FELMYST_INCOMING_ENCAPSULATE_DELAY_MS;

    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
    state.auraObserved = false;
}

Player* GetFelmystEncapsulateTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    const uint32 now = getMSTime();
    auto const incomingItr = felmystIncomingEncapsulateStates.find(bot->GetInstanceId());
    if (incomingItr != felmystIncomingEncapsulateStates.end())
    {
        auto& incomingState = incomingItr->second;
        Player* incomingTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != incomingState.targetGuid)
                continue;

            incomingTarget = member;
            break;
        }

        if (incomingTarget &&
            incomingTarget->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE)))
        {
            incomingState.auraObserved = true;
            return incomingTarget;
        }

        if (!incomingTarget || incomingState.auraObserved || incomingState.expireMs <= now)
            felmystIncomingEncapsulateStates.erase(incomingItr);
        else
            return incomingState.delayMs <= now ? incomingTarget : nullptr;
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
        if (!member || !member->HasAura(
                static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
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
