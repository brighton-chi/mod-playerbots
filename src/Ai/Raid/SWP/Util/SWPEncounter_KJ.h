/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKJ_H
#define PLAYERBOTS_SWPENCOUNTERKJ_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPSharedConstants.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
class Unit;

namespace SwpHelpers
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

struct KiljaedenEncounterState
{
    std::vector<KiljaedenArmageddon> armageddons;
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    std::unordered_map<ObjectGuid, uint8> rangedArmageddonAssignments;
    uint32 dragonOrbAnnouncementMs = 0;
    uint32 rangedAssignmentRebuildMs = 0;
    uint32 rangedArmageddonRebuildMs = 0;
};

inline std::array const KILJAEDEN_DRAGON_ORB_ENTRIES = {
    Id(SwpObjects::GO_DRAGON_ORB_1),
    Id(SwpObjects::GO_DRAGON_ORB_2),
    Id(SwpObjects::GO_DRAGON_ORB_3),
    Id(SwpObjects::GO_DRAGON_ORB_4),
};

inline constexpr float KILJAEDEN_PHASE3_HP_THRESHOLD = 85.0f;
inline constexpr float KILJAEDEN_PHASE4_HP_THRESHOLD = 55.0f;
inline constexpr float KILJAEDEN_PHASE5_HP_THRESHOLD = 25.0f;

// Shield of the Blue (45848) lasts 5s and Darkness of a Thousand Souls (46605) is an 8s channel,
// so the dragon casts once less than this remains and the shield still covers the hit. Bots with
// Fire Bloom hold clear of the stack until the same point.
inline constexpr int32 KILJAEDEN_SHIELD_OF_THE_BLUE_CAST_WINDOW_MS = 4500;

inline constexpr uint32 KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS = 10000;

// Both assignment passes rebuild shared per-instance state from scratch, and every ranged bot runs
// them every tick, so a timestamp throttle is the tool rather than a per-bot value. The canonical
// slots only move when raid membership does; the armageddon reshuffle is on a shorter leash
// because the missile lands six seconds after its target creature spawns.
inline constexpr uint32 KILJAEDEN_RANGED_ASSIGNMENT_REBUILD_INTERVAL_MS = 1000;
inline constexpr uint32 KILJAEDEN_ARMAGEDDON_ASSIGNMENT_REBUILD_INTERVAL_MS = 250;

inline constexpr float KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE = 11.0f;

// Reaches for the sinister reflection pickup ladder: Avenger's Shield and Death and Decay reach
// 30, the charges 25, Icy Touch 20, and the two self-centred taunts land inside their own radius
// (10 for Challenging Shout and Roar, 8 for Consecration). Checked explicitly because
// PlayerbotAI::CanCastSpell whitelists SPELL_FAILED_OUT_OF_RANGE and returns true regardless.
inline constexpr float KILJAEDEN_REFLECTION_RANGED_REACH = 30.0f;
inline constexpr float KILJAEDEN_REFLECTION_CHARGE_REACH = 25.0f;
inline constexpr float KILJAEDEN_REFLECTION_ICY_TOUCH_REACH = 20.0f;
inline constexpr float KILJAEDEN_REFLECTION_SHOUT_REACH = 10.0f;
inline constexpr float KILJAEDEN_REFLECTION_CONSECRATION_REACH = 8.0f;
inline constexpr float KILJAEDEN_REFLECTION_SEARCH_RADIUS = 100.0f;

// Radii for the hand pickup abilities that are anchored on the caster rather than on the hand.
// CanCastSpell never range-checks those against the target and whitelists SPELL_FAILED_OUT_OF_RANGE
// besides, so a cast from across the room reports success and spends the cooldown on nothing.
// 8 covers War Stomp 20549 and all three Arcane Torrent ranks; 10 is Shockwave 46968's cone.
inline constexpr float KILJAEDEN_SELF_AOE_RACIAL_RADIUS = 8.0f;
inline constexpr float KILJAEDEN_SHOCKWAVE_RADIUS = 10.0f;

// Hands cast Shadow Infusion (45772) at or below 20% health, which makes them permanently immune
// to both stun and silence, so there is nothing left to try under that. The upper gate is a
// deliberate fudge to let the tanks spread the pack before anyone locks one down.
inline constexpr float KILJAEDEN_HAND_STUN_IMMUNE_HP_PERCENT = 20.0f;
inline constexpr float KILJAEDEN_HAND_STUN_MAX_HP_PERCENT = 80.0f;

// How far apart the hand tanks keep their own pulls
inline constexpr float KILJAEDEN_HAND_TANK_SEPARATION = 15.0f;

// The orbs sit around the edge of the platform, so the search has to cover the room
inline constexpr float KILJAEDEN_DRAGON_ORB_SEARCH_RADIUS = 200.0f;

// The designated user waits near an orb already in use rather than wandering off to another
inline constexpr float KILJAEDEN_ORB_IN_USE_HOLD_DISTANCE = 15.0f;

// Grace after taking an orb before a lingering root counts as the stale one worth clearing
inline constexpr uint32 KILJAEDEN_ORB_USE_GRACE_MS = 2000;

// Debounce on re-announcing the designated orb user
inline constexpr uint32 KILJAEDEN_ORB_ANNOUNCEMENT_RESET_MS = 10000;

// Bots carrying Fire Bloom hold this far off the darkness stack until the shield is nearly up
inline constexpr float KILJAEDEN_FIRE_BLOOM_STANDOFF = 15.0f;

// Breath: Haste and Breath: Revitalize are 13 yard cones on allies, so the dragon parks a little
// under half that from its target and looks for a cluster of roughly the same span to cover at
// once. The standoff and the cluster radius share a value by derivation, not by coincidence.
inline constexpr float KILJAEDEN_DRAGON_BREATH_STANDOFF = 6.0f;
inline constexpr float KILJAEDEN_DRAGON_STANDOFF_TOLERANCE = 1.0f;
inline constexpr float KILJAEDEN_DRAGON_CLUSTER_RADIUS = 6.0f;
inline constexpr uint8 KILJAEDEN_DRAGON_MIN_CLUSTER_SIZE = 3;

inline constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
inline constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
inline constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
inline constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
inline constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
inline constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

// Two bots may share a slot when an armageddon forces a reshuffle; a third would be on top of them
inline constexpr uint8 KILJAEDEN_MAX_BOTS_PER_RANGED_SLOT = 2;

inline Position const KILJAEDEN_CENTER_POSITION =   { 1698.450f, 628.030f, 28.199f };
inline Position const KILJAEDEN_TANK_POSITION =     { 1704.729f, 634.891f, 27.787f };
inline Position const KILJAEDEN_S_MELEE_POSITION =  { 1689.487f, 632.119f, 27.823f };
inline Position const KILJAEDEN_E_MELEE_POSITION =  { 1700.542f, 619.589f, 27.786f };
inline Position const KILJAEDEN_DARKNESS_POSITION = { 1709.768f, 642.241f, 27.706f };

extern std::unordered_set<ObjectGuid> kiljaedenTrackedArmageddonTargets;
extern std::unordered_map<uint32, KiljaedenEncounterState> kiljaedenEncounterStates;
extern std::unordered_map<uint32, std::array<ObjectGuid, 3>> kiljaedenHandTankAssignments;
extern std::unordered_map<ObjectGuid::LowType, uint32> kiljaedenDragonOrbUseTimes;

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
void EnsureKiljaedenRangedAssignments(Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(Player* bot);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
Player* GetKiljaedenDragonOrbUser(Player* bot);
bool ResetKiljaedenDragonOrbUserAnnouncement(uint32 instanceId);
bool HasRecentKiljaedenDragonOrbUse(Player* bot, uint32 recentMs);
bool HasKiljaedenDragonAura(Player* bot);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);
bool HasAtLeastThreeBotTanks(
    Player* bot, Player** outMainTank = nullptr, Player** outFirstAssist = nullptr,
    Player** outSecondAssist = nullptr);

}

#endif
