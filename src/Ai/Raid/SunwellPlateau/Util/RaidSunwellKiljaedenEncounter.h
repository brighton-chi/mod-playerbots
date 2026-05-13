/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLKILJAEDENENCOUNTER_H
#define _PLAYERBOT_RAIDSUNWELLKILJAEDENENCOUNTER_H

#include <unordered_map>
#include <vector>

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidSunwellData.h"

class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

struct KiljaedenRangedBotAssignment
{
    ObjectGuid guid;
    uint8 slotIndex = 0;
};

struct KiljaedenArmageddon
{
    Position destination;
    uint32 expireMs = 0;
    float safeDistance = 0.0f;
};

constexpr uint32 KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS = 10000;
constexpr float KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE = 11.0f;
constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;
extern uint32 const KILJAEDEN_DRAGON_ORB_ENTRIES[4];

extern const Position KILJAEDEN_CENTER_POSITION;
extern const Position KILJAEDEN_TANK_POSITION;
extern const Position KILJAEDEN_S_MELEE_POSITION;
extern const Position KILJAEDEN_E_MELEE_POSITION;
extern const Position KILJAEDEN_STACK_POSITION;

extern std::unordered_map<uint32, std::vector<KiljaedenArmageddon>> kiljaedenArmageddons;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedArmageddonAssignments;

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool HasActiveKiljaedenArmageddon(uint32 instanceId);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
float GetKiljaedenRangedSlotAngle(uint8 slotIndex);
bool IsKiljaedenRangedSlotSafe(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons);
float GetKiljaedenNearestArmageddonDistance(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons);
void EnsureKiljaedenRangedAssignments(PlayerbotAI* botAI, Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(PlayerbotAI* botAI, Player* bot);
bool IsKiljaedenDragonOrb(uint32 entry);
Player* GetKiljaedenDragonOrbUser(Player* bot);
bool TryAnnounceKiljaedenDragonOrbUser(PlayerbotAI* botAI, Player* bot);
void ResetKiljaedenDragonOrbUserAnnouncement(uint32 instanceId);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(PlayerbotAI* botAI, Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);

}

#endif
