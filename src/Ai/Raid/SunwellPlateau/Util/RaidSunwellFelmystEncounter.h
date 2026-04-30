/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLFELMYSTENCOUNTER_H
#define _PLAYERBOT_RAIDSUNWELLFELMYSTENCOUNTER_H

#include <array>
#include <limits>
#include <unordered_map>

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidSunwellData.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

enum class FelmystFogLane : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Top = 0,
    Middle = 1,
    Bottom = 2,
};

enum class FelmystFogPhase : uint8
{
    None,
    Windup,
    Sweep,
    Recovery,
};

enum class FelmystFogLocation : uint8
{
    None,
    LeftSide,
    RightSide,
    LeftTop,
    LeftMiddle,
    LeftBottom,
    RightTop,
    RightMiddle,
    RightBottom,
};

struct FelmystFogOfCorruptionState
{
    FelmystFogLane lane = FelmystFogLane::None;
    FelmystFogPhase phase = FelmystFogPhase::None;
    uint32 expireMs = 0;
};

struct FelmystIncomingEncapsulateState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 expireMs = 0;
    bool auraObserved = false;
};

constexpr float FELMYST_ENCAPSULATE_SAFE_DISTANCE = 21.0f;
constexpr float FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE = 8.0f;
constexpr float FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE = 3.0f;
constexpr float FELMYST_FOG_DESTINATION_MATCH_DISTANCE = 1.0f;

extern const Position FELMYST_TANK_POSITION;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporPathIndices;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporWaypointIndices;
extern std::unordered_map<uint32, FelmystFogOfCorruptionState> felmystFogOfCorruptionStates;
extern std::unordered_map<uint32, FelmystIncomingEncapsulateState> felmystIncomingEncapsulateStates;

void EnsureFelmystRangedAssignments(PlayerbotAI* botAI, Player* bot);
void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs = 3000);
float GetFelmystFrontAngle(PlayerbotAI* botAI, Player* bot, Unit* felmyst);
Creature* GetFelmystDemonicVaporSummonedByBot(Player* carrier);
void ClearFelmystDemonicVaporKiteState(Player* bot);
bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination);
bool TryGetFelmystFogSafeDestinations(
    Player* bot, FelmystFogLane dangerLane, std::array<Position, 3>& destinations,
    uint8& destinationCount);
bool TryGetFelmystFogOfCorruptionStageState(
    Unit* felmyst, FelmystFogOfCorruptionState& state);
bool TryGetActiveFelmystFogOfCorruptionState(
    Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state);
Unit* GetNearestFelmystFogOfCorruptionCharmedTarget(Player* bot);
Unit* GetNearestFelmystDemonicVaporHazard(Player* bot);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool TryGetFelmystRangedPosition(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position);
Player* GetFelmystGasNovaDispelTarget(Player* bot);

}

#endif
