/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SWPENCOUNTER_KJ_H
#define _PLAYERBOT_SWPENCOUNTER_KJ_H

#include <unordered_map>
#include <vector>

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPData.h"

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

struct KiljaedenDarknessShieldState
{
    bool inDarkness = false;
    bool shieldCastThisDarkness = false;
    uint32 darknessStartMs = 0;
    uint32 lastDarknessCastMsLeft = 0;
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
extern const Position KILJAEDEN_DARKNESS_POSITION;

extern std::unordered_set<ObjectGuid> kiljaedenTrackedArmageddonTargets;
extern std::unordered_map<uint32, std::vector<KiljaedenArmageddon>> kiljaedenArmageddons;
extern std::unordered_map<uint32, uint32> kiljaedenDragonOrbAnnouncementTimes;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedArmageddonAssignments;

std::unordered_map<ObjectGuid::LowType, KiljaedenDarknessShieldState>&
    GetKiljaedenDarknessShieldStates();
void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
void EnsureKiljaedenRangedAssignments(PlayerbotAI* botAI, Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(PlayerbotAI* botAI, Player* bot);
Player* GetKiljaedenDragonOrbUser(Player* bot);
void RecordKiljaedenDragonOrbUse(Player* bot);
bool HasRecentKiljaedenDragonOrbUse(Player* bot, uint32 recentMs);
void ResetKiljaedenDragonOrbUserAnnouncement(uint32 instanceId);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(PlayerbotAI* botAI, Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);

}

#endif
