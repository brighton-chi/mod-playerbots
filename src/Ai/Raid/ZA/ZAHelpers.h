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

class GameObject;
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

    // Akil'zon <Eagle Avatar>
    NPC_AKILZON                     = 23574,

    // Nalorakk <Bear Avatar>
    NPC_NALORAKK                    = 23576,

    // Jan'alai <Dragonhawk Avatar>
    NPC_JANALAI                     = 23578,
    NPC_AMANI_DRAGONHAWK_HATCHLING  = 23598,
    NPC_AMANISHI_HATCHER            = 23818,
    NPC_FIRE_BOMB                   = 23920,

    // Halazzi <Lynx Avatar>
    NPC_HALAZZI                     = 23577,
    NPC_SPIRIT_OF_THE_LYNX          = 24143,
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
};

enum class ZaObjects : uint32
{
    GO_FREEZING_TRAP                = 186669,
};

// General
inline constexpr uint32 ZA_MAP_ID = 568;
// For Hex Lord and Zul'jin. Whirlwind radius is 8y. Safe distance has 4y of MoveAway padding;
// hold distance is for the don't run back in multiplier and adds another 3y of padding.
inline constexpr float ZA_WHIRLWIND_SAFE_DISTANCE = 12.0f;
inline constexpr float ZA_WHIRLWIND_HOLD_DISTANCE = 15.0f;

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
inline constexpr float JANALAI_PLATFORM_Z = 19.146f;
// Measured in the room. The fifth corner cuts off the northwest, where the platform is broken and
// fenced off; the other four trace the floor's limits. Every point in it has to be walkable and it
// has to stay convex - that is what lets FindSafeStepInZone() ignore where a step lands.
//
// Not inset from the fire wall, and it does not need to be: those ten bombs are NPC_FIRE_BOMB like
// the other forty, so the safe distance already keeps candidates off them, and only over the ~10 yd
// of each edge they occupy rather than the whole edge. The east and west edges run alongside the
// wall as a result; the north and south are well back, since there it crosses the bridge mouths.
//
// A vector where the other position tables are inline std::array: those are indexed where they sit,
// this one is handed to FindSafeStepInZone().
inline std::vector<Position> const JANALAI_SAFE_ZONE = {
    Position(-12.576924f, 1131.9163f, JANALAI_PLATFORM_Z),
    Position(-52.068108f, 1132.8198f, JANALAI_PLATFORM_Z),
    Position(-51.508590f, 1158.4890f, JANALAI_PLATFORM_Z),
    Position(-44.008713f, 1168.6125f, JANALAI_PLATFORM_Z),
    Position(-12.115170f, 1167.6681f, JANALAI_PLATFORM_Z)
};
// Hatchers open eggs in a ramp - 1 on the first tick, then 2, then 3, every 5s - across 40 eggs,
// 20 per side. Bloodlust waits for this many Hatchlings to be up.
inline constexpr uint32 JANALAI_BLOODLUST_HATCHLING_COUNT = 6;
// Ranged spread evenly around this, centred on the tank position. It has to clear the melee and
// stay inside JANALAI_SAFE_ZONE on every bearing, which on the west side is the tighter of the two.
inline constexpr float JANALAI_RANGED_SPREAD_RADIUS = 15.0f;
// How far the avoidance looks for somewhere to stand. The bombs blanket the room, so a failed
// search never means there is no clear spot, only none nearby - reach is what fixes that, and it is
// nearly free because the rings return on the first hit and only crowded cases pay for the rest.
inline constexpr float JANALAI_FIRE_BOMB_MAX_SEARCH_DISTANCE = 30.0f;
// Must cover the furthest candidate plus the safe distance, so every bomb bearing on that candidate
// is in the list. Move one of the pair and move the other.
inline constexpr float JANALAI_FIRE_BOMB_SEARCH_RADIUS = 40.0f;
// Fire Bomb (42630) has a 4y radius, padded by 1.5. Measure with GetExactDist2d, never
// GetDistance2d: the damage is a DEST area effect from an NPC caster, so membership is exact centre
// to centre with no reach on either side, and the danger test and the safe-spot test only agree in
// exact terms.
//
// The pad is deliberately tight. In a field this dense it is the most expensive number here - clear
// ground goes as e^(-n*pi*r^2/A), so half a yard off it is worth more than ten yards of search
// reach, in both bots left with nowhere to stand and bots disturbed at all. It can afford to be:
// the bombs are static and the danger test uses this same constant, so a bot at rest is outside the
// blast whatever the pad is.
inline constexpr float JANALAI_FIRE_BOMB_SAFE_DISTANCE = 5.5f;
// Feeds the "jan'alai fire bombs" value. Worst-case detection latency, not staleness. A second is
// affordable - bombs spawn at their final positions and detonate on a fixed 11s timer - and it is
// about what a player takes, since reacting here means picking a gap rather than stepping aside.
inline constexpr uint32 FIRE_BOMB_CACHE_INTERVAL_MS = 1000;
// Jan'alai hatches every remaining egg at once at 35% HP so that opens the door for Bloodlust in
// any case. Using 33% to account for some delay for the event to actually complete.
inline constexpr float JANALAI_HATCH_ALL_HEALTH_PCT = 33.0f;

std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI);
// Walks outward from the bot in 1 yd rings, bounded by a convex polygon given as its corners in
// order (either winding), and hands back one step of moveDist toward the nearest spot inside it
// that is clear of every hazard. The step is validated with CanTakeStepTowards(), so a candidate
// whose step cannot actually be taken is skipped for the next one out. Pass stepZ to MoveTo(), not
// the bot's Z - the helper has already snapped it to ground.
//
// The polygon must be convex and wholly walkable; see JANALAI_SAFE_ZONE.
uint32 CountJanalaiHatchlingsByEntry(PlayerbotAI* botAI);
bool IsJanalaiBombing(Unit* janalai);
GuidVector FindNearbyFireBombGuids(Player* bot);
// GetNearbyFireBombs() resolves that value for the avoidance search, which is the only caller that
// needs the bombs themselves. Everything else asks IsJanalaiBombing() instead.
std::vector<Unit*> GetNearbyFireBombs(PlayerbotAI* botAI);
// Jan'alai carries this channel for exactly as long as the bombs are dangerous: StartBombing()
// applies it alongside the spawn and the Boom() task removes it in the same moment it detonates
// them. The bomb creatures themselves linger 4s past that on a 15s despawn, so counting them keeps
// the raid frozen after the damage has already landed.
bool FindSafeStepInZone(
    Player* bot, std::vector<Unit*> const& hazards, std::vector<Position> const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ);
// Counts living units of one entry in the group's attacker list. Reads "attackers", which the
// engine already recomputes on a 1s cache, so this is cheap enough for a multiplier - unlike
// "possible targets no los", which has no cache and re-runs a sight-range grid search per call.
// The trade is that it only sees units actually threatening the raid, so it suits adds that fight
// back and not passive ones like the Hatchers.


// Halazzi <Lynx Avatar>
inline Position const HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

// Hex Lord Malacrass
// Freezing Trap (43448) stuns for 10s across a 10y radius, and it is that radius to clear rather
// than the object's 2.5y trigger circle: whoever springs it catches everyone within 10. Measure
// with GetExactDist2d, never GetDistance2d - a SRC area effect, so exact centre to centre.
//
// Padded by 1 rather than the usual 2, because the tank flees with everyone else and Malacrass
// follows it; every extra yard walks him closer to the door. The pad can be tight because the trap
// is static and the trigger uses this same constant, so a bot at rest is outside the stun anyway.
inline constexpr float ZA_FREEZING_TRAP_SAFE_DISTANCE = 11.0f;
// Far enough to notice a trap before the bot is inside the safe distance. It doubles as how far
// HexLordMalacrassStayAwayFromFreezingTrapMultiplier holds a bot off, and that gap is what stops a
// bot from turning back the moment it clears the flee - so keep this the wider of the pair.
inline constexpr float ZA_FREEZING_TRAP_SEARCH_RADIUS = 16.0f;
// Feeds the "hex lord malacrass freezing trap" value, which the trigger and the action share rather
// than each running the grid search. 200ms because reacting here is a reflex - one object, move
// directly away - and because the trap arrives unannounced: 43447 places it by
// TARGET_DEST_CASTER_RADIUS with a random bearing, so it lands 5y from the boss on any side,
// inside the melee ring.
inline constexpr uint32 FREEZING_TRAP_CACHE_INTERVAL_MS = 200;

// Behind the "hex lord malacrass freezing trap" value.
ObjectGuid FindNearbyFreezingTrapGuid(Player* bot);
// Resolves that value back to the object. Held as a guid rather than a pointer because a trap that
// fires and despawns inside the interval then comes back as nullptr instead of dangling.
GameObject* GetNearbyFreezingTrap(PlayerbotAI* botAI);

// Zul'jin
inline Position const ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };
// Four Feather Vortexes chase random raid members at 7.0 y/s - a player's own run speed - and
// retarget whoever they reach, so there is nothing to dodge. The raid spreads instead, far enough
// apart that a vortex passing through one member does not clip anyone else with its 4 yd aura.
//
// Order is what matters to GetZuljinSpreadSlotIndex() - the healer slots must come first. The four
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

// Ranged raid members are ordered healers first, then ranged dps, each in group order, and member
// i takes slot i. That fills the healer slots at the head of a spread list before anyone else, and
// spills the remainder over in order: one healer leaves the second healer slot to a ranged dps, a
// third healer takes the first dps slot. Wraps once there are more members than slots. Dead members
// keep their slot so a death does not reshuffle the whole raid. Returns false for a bot that is not
// ranged, or is not grouped or in the instance.
bool GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex);

}

#endif
