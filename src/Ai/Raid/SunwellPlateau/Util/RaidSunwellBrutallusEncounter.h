/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLBRUTALLUSENCOUNTER_H
#define _PLAYERBOT_RAIDSUNWELLBRUTALLUSENCOUNTER_H

#include <unordered_map>

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidSunwellData.h"

class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

struct BrutallusRangedSlotInfo
{
    bool isMainTankGroup = false;
    uint8 arcPositionIndex = 0;
};

enum class BrutallusRangedBurnState : uint8
{
    None,
    MovingToFrontStep,
    MovingToMirrorStep,
    MovingToBurnPosition,
    AtBurnPosition,
    ReturningNormalArc
};

extern const Position BRUTALLUS_MAIN_TANK_POSITION;
constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -M_PI_2;
constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 20.25f;
constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
constexpr float BRUTALLUS_RANGED_LANE_OFFSET = 5.0f;
constexpr float BRUTALLUS_NORMAL_RANGED_RADIUS =
    BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
constexpr float BRUTALLUS_BURN_TRAVEL_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS - BRUTALLUS_RANGED_LANE_OFFSET;
constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS =
    BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
extern std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

float GetBrutallusMainTankAngle(Unit* brutallus);
Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z);
Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z);
bool TryGetBrutallusMeleePosition(
    Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position);
float GetBrutallusRangedSlotAngle(
    Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo);
bool TryGetBrutallusRangedStepPosition(
    Unit* brutallus, uint8 rangedIndex, bool useMirrorAngle,
    float radius, float z, Position& position);
bool TryGetBrutallusRangedArcPosition(
    Unit* brutallus, uint8 rangedIndex, float radius, bool moveTowardMirror,
    float currentX, float currentY, float z, Position& position);
void EnsureBrutallusRangedAssignments(PlayerbotAI* botAI, Player* bot);
bool TryGetBrutallusAssignedPositionIndex(PlayerbotAI* botAI, Player* bot, bool wantRanged,
    uint8& positionIndex);

}

#endif
