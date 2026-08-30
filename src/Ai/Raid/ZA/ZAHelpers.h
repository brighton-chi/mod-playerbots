/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAHELPERS_H
#define PLAYERBOTS_ZAHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "Unit.h"
#include <array>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace ZaHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class ZaSpells : uint32
{
    // Akil'zon <Eagle Avatar>
    SPELL_ELECTRICAL_STORM          = 43648,

    // Nalorakk <Bear Avatar>
    SPELL_BEARFORM                  = 42377,

    // Jan'alai <Dragonhawk Avatar>
    SPELL_FIRE_BOMB_CHANNEL         = 42621,

    // Hex Lord Malacrass
    SPELL_HEX_LORD_WHIRLWIND        = 43442,
    SPELL_HEX_LORD_SPELL_REFLECTION = 43443,
    SPELL_UNSTABLE_AFFLICTION       = 43522,

    // Zul'jin
    SPELL_ZULJIN_WHIRLWIND          = 17207,
    SPELL_SHAPE_OF_THE_BEAR         = 42594,
    SPELL_SHAPE_OF_THE_EAGLE        = 42606,
    SPELL_SHAPE_OF_THE_LYNX         = 42607,
    SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,
    // SPELL_CLAW_RAGE              = 43149, // Would require getting Zul'jin's bossai or a hook

    // Hunter
    SPELL_MISDIRECTION              = 35079,
};

enum class ZaNpcs : uint32
{
    // Trash
    NPC_AMANI_HEALING_WARD          = 23757,
    NPC_AMANI_PROTECTIVE_WARD       = 23822,

    // Jan'alai <Dragonhawk Avatar>
    NPC_JANALAI                     = 23578,
    NPC_AMANI_DRAGONHAWK_HATCHLING  = 23598,
    NPC_AMANISHI_HATCHER            = 23818,
    NPC_FIRE_BOMB                   = 23920,

    // Halazzi <Lynx Avatar>
    NPC_CORRUPTED_LIGHTNING_TOTEM   = 24224,

    // Hex Lord Malacrass
    NPC_HEX_LORD_MALACRASS          = 24239,
    NPC_ALYSON_ANTILLE              = 24240,
    NPC_THURG                       = 24241,
    NPC_SLITHER                     = 24242,
    NPC_LORD_RAADAN                 = 24243,
    NPC_GAZAKROTH                   = 24244,
    NPC_FENSTALKER                  = 24245,
    NPC_DARKHEART                   = 24246,
    NPC_KORAGG                      = 24247,

    // Zul'jin
    NPC_ZULJIN                      = 23863,
    NPC_FEATHER_VORTEX              = 24136,
};

enum class ZaObjects : uint32
{
    GO_FREEZING_TRAP                = 186669,
};

// General
inline constexpr uint32 ZA_MAP_ID = 568;
// For Hex Lord and Zul'jin. Radius is 8y with 2y of MoveAway padding.
inline constexpr float ZA_WHIRLWIND_SAFE_DISTANCE = 10.0f;

// A flat, convex quad given by its four corners in order (either winding). Contains() is a 2D
// point-in-convex-polygon test, so rectangles that are not axis aligned work without extra math.
//
// Real platforms are rarely a clean rectangle: they jut out, and broken terrain can take a bite
// out of a corner. Rather than shrinking the corners until they fit inside every irregularity and
// giving up the floor that costs, the quad is only a cheap outer bound and floorZ is the precise
// one. A candidate is kept only when the ground under it probes within floorTolerance of floorZ,
// which rejects holes and overhangs wherever they happen to be.
struct SafeZoneQuad
{
    std::array<Position, 4> corners;
    float floorZ;
    float floorTolerance;

    bool Contains(float x, float y) const;
};

// True when the ground under (x, y) is within floorTolerance of floorZ. A spot off the platform
// probes the floor far below (or nothing at all) and fails by a wide margin.
bool IsOnFlatFloor(Player* bot, float x, float y, float floorZ, float floorTolerance);

// Walks outward from the bot in 1 yd rings, bounded by a quad, and hands back one step of moveDist
// toward the nearest spot that is clear of every hazard. The step is validated with
// CanTakeStepTowards(), so a candidate whose step cannot actually be taken is skipped for the next
// one out. Pass stepZ to MoveTo(), not the bot's Z - the helper has already snapped it to ground.
bool FindSafeStepInZone(
    Player* bot, std::vector<Unit*> const& hazards, SafeZoneQuad const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ);
bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius);
// Counts living units of one entry in the group's attacker list. Reads "attackers", which the
// engine already recomputes on a 1s cache, so this is cheap enough for a multiplier - unlike
// "possible targets no los", which has no cache and re-runs a sight-range grid search per call.
// The trade is that it only sees units actually threatening the raid, so it suits adds that fight
// back and not passive ones like the Hatchers.
uint32 CountAttackersByEntry(PlayerbotAI* botAI, uint32 entry);

// Ranged raid members are ordered healers first, then ranged dps, each in group order, and member
// i takes slot i. That fills the healer slots at the head of a spread list before anyone else, and
// spills the remainder over in order: one healer leaves the second healer slot to a ranged dps, a
// third healer takes the first dps slot. Wraps once there are more members than slots. Dead members
// keep their slot so a death does not reshuffle the whole raid. Returns false for a bot that is not
// ranged, or is not grouped or in the instance.
bool GetSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex);

// Akil'zon <Eagle Avatar>
inline Position const AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };
// Electrical Storm runs on a fixed 60s cycle once the encounter starts. Bots react from
// AKILZON_STORM_LEAD_MS before each cast until the storm itself has expired.
inline constexpr uint32 AKILZON_STORM_PERIOD_MS = 60000;
inline constexpr uint32 AKILZON_STORM_LEAD_MS = 5000;
inline constexpr uint32 AKILZON_STORM_DURATION_MS = 10000;
// Instance id -> getMSTime() stamp of the first storm cycle.
extern std::unordered_map<uint32, uint32> akilzonStormTimer;
bool IsInStormWindow(uint32 startMs);
Player* GetElectricalStormTarget(Player* bot);

// Nalorakk <Bear Avatar>
inline Position const NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };
bool IsNalorakkInBearForm(Unit* nalorakk);

// Jan'alai <Dragonhawk Avatar>
inline Position const JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };
// Jan'alai's platform is a broadly rectangular flat floor, not a circle, and his bomb field covers
// all of it: 44 x 51 yd centered on him. The corners below trace the fire wall he summons (x -54.80
// / -10.13, y 1123.90 / 1175.68 in fireWallCoords, boss_janalai.cpp), inset 1 yd. That line is the
// outer bound and is deliberately generous - the walkable floor is not exactly rectangular, so the
// floor probe, not the corners, is what keeps bots off the broken parts.
inline constexpr float JANALAI_PLATFORM_Z = 19.146f;
// The floor below the platform sits around z 6, so anything short of that drop is surface noise.
inline constexpr float JANALAI_FLOOR_TOLERANCE = 2.0f;
inline SafeZoneQuad const JANALAI_SAFE_ZONE = {
    {{
        Position(-53.80f, 1124.90f, JANALAI_PLATFORM_Z),
        Position(-11.13f, 1124.90f, JANALAI_PLATFORM_Z),
        Position(-11.13f, 1174.68f, JANALAI_PLATFORM_Z),
        Position(-53.80f, 1174.68f, JANALAI_PLATFORM_Z)
    }},
    JANALAI_PLATFORM_Z,
    JANALAI_FLOOR_TOLERANCE
};
// Hatchers open eggs in a ramp - 1 on the first tick, then 2, then 3, every 5s - across 40 eggs,
// 20 per side. Bloodlust waits for this many Hatchlings to be up.
inline constexpr uint32 JANALAI_BLOODLUST_HATCHLING_COUNT = 6;
// The bombs blanket the whole platform, so a search only has to reach as far as the avoidance can
// move (20 yd) plus the blast radius (4, padded to 5). 30 covers that with room to spare.
inline constexpr float JANALAI_FIRE_BOMB_SEARCH_RADIUS = 30.0f;
// Feeds the "jan'alai fire bombs" value. Longer than the 200ms the other raids use for hazards,
// because bombs are unusually well behaved: they all spawn at once at their final positions, never
// move, and detonate on a fixed 11s timer (StartBombing() in boss_janalai.cpp). Staleness costs
// only detection latency against that 11s, and the value holds GUIDs rather than pointers, so a
// despawned bomb resolves to nullptr instead of dangling however far behind the cache runs.
inline constexpr uint32 FIRE_BOMB_CACHE_INTERVAL_MS = 1000;
// Jan'alai hatches every remaining egg at once at 35% HP so that opens the door for Bloodlust in
// any case. Using 33% to account for some delay for the event to actually complete.
inline constexpr float JANALAI_HATCH_ALL_HEALTH_PCT = 33.0f;
// The grid search, run once per cache interval behind the "jan'alai fire bombs" value.
GuidVector FindNearbyFireBombGuids(Player* bot);
// GetNearbyFireBombs() resolves that value for the avoidance search, which is the only caller that
// needs the bombs themselves. Everything else asks IsJanalaiBombing() instead.
std::vector<Unit*> GetNearbyFireBombs(PlayerbotAI* botAI);
// Jan'alai carries this channel for exactly as long as the bombs are dangerous: StartBombing()
// applies it alongside the spawn and the Boom() task removes it in the same moment it detonates
// them. The bomb creatures themselves linger 4s past that on a 15s despawn, so counting them keeps
// the raid frozen after the damage has already landed.
bool IsJanalaiBombing(Unit* janalai);
std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI);

// Halazzi <Lynx Avatar>
inline Position const HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

// Hex Lord Malacrass
inline constexpr float ZA_FREEZING_TRAP_SEARCH_RADIUS = 20.0f;

// Zul'jin
inline Position const ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };
// Four Feather Vortexes chase random raid members at 7.0 y/s - a player's own run speed - and
// retarget whoever they reach, so there is nothing to dodge. The raid spreads instead, far enough
// apart that a vortex passing through one member does not clip anyone else with its 4 yd aura.
//
// Order is what matters to GetSpreadSlotIndex() - the healer slots must come first. The four
// corner slots are pulled in along X until they sit exactly 39 yd from the farther healer, leaving
// a yard of slack against a 40 yd heal. The closest pair of spots is still 15.3 yd apart, so a
// vortex parked on one member never clips a second.
inline constexpr float ZULJIN_SPREAD_Z = 45.111f;
inline std::array<Position, 8> const ZULJIN_SPREAD_POSITIONS = {{
    // Healer slots.
    Position(120.462f, 728.502f, ZULJIN_SPREAD_Z),
    Position(119.984f, 693.188f, ZULJIN_SPREAD_Z),
    // Ranged dps slots.
    Position(94.939f, 713.698f, ZULJIN_SPREAD_Z),
    Position(145.286f, 713.034f, ZULJIN_SPREAD_Z),
    Position(103.869f, 728.703f, ZULJIN_SPREAD_Z),
    Position(135.847f, 728.816f, ZULJIN_SPREAD_Z),
    Position(143.862f, 697.302f, ZULJIN_SPREAD_Z),
    Position(95.618f, 698.439f, ZULJIN_SPREAD_Z)
}};

}

#endif
