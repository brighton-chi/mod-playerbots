/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAHELPERS_H
#define PLAYERBOTS_ZAHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

class Creature;
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

// Boss ids from zulaman.h, matching DungeonEncounter.dbc's encounterIndex.
enum class ZaBosses : uint32
{
    DATA_AKILZON            = 0,
    DATA_NALORAKK           = 1,
    DATA_JANALAI            = 2,
    DATA_HALAZZI            = 3,
    DATA_HEX_LORD_MALACRASS = 4,
    DATA_ZULJIN             = 5,
};

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
    SPELL_CREEPING_PARALYSIS        = 43095,

    // Hunter
    SPELL_MISDIRECTION              = 34477,
    SPELL_MISDIRECTION_AURA         = 35079, // the aura on the hunter

    // Priest
    SPELL_MASS_DISPEL               = 32375,
};

enum class ZaNpcs : uint32
{
    // Trash
    NPC_AMANI_HEALING_WARD         = 23757,
    NPC_AMANI_PROTECTIVE_WARD      = 23822,

    // Akil'zon <Eagle Avatar>
    NPC_AKILZON                    = 23574,

    // Nalorakk <Bear Avatar>
    NPC_NALORAKK                   = 23576,

    // Jan'alai <Dragonhawk Avatar>
    NPC_JANALAI                    = 23578,
    NPC_AMANI_DRAGONHAWK_HATCHLING = 23598,
    NPC_AMANISHI_HATCHER           = 23818,
    NPC_FIRE_BOMB                  = 23920,

    // Halazzi <Lynx Avatar>
    NPC_HALAZZI                    = 23577,
    NPC_HALAZZI_TROLL              = 24144,
    NPC_SPIRIT_OF_THE_LYNX         = 24143,
    NPC_CORRUPTED_LIGHTNING_TOTEM  = 24224,

    // Hex Lord Malacrass
    NPC_HEX_LORD_MALACRASS         = 24239,
    NPC_ALYSON_ANTILLE             = 24240,
    NPC_THURG                      = 24241,
    NPC_SLITHER                    = 24242,
    NPC_LORD_RAADAN                = 24243,
    NPC_GAZAKROTH                  = 24244,
    NPC_FENSTALKER                 = 24245,
    NPC_DARKHEART                  = 24246,
    NPC_KORAGG                     = 24247,

    // Zul'jin
    NPC_ZULJIN                     = 23863,
};

enum class ZaObjects : uint32
{
    GO_FREEZING_TRAP = 186669,
};

// Shared

inline constexpr uint32 ZA_MAP_ID = 568;
// For Hex Lord and Zul'jin. Whirlwind radius is 8 yd. Clearance has 4 yd of MoveAway padding;
// hold distance is for the don't run back in multiplier and adds another 3 yd of padding.
inline constexpr float ZA_WHIRLWIND_CLEARANCE = 12.0f;
inline constexpr float ZA_WHIRLWIND_HOLD_DISTANCE = 15.0f;
// For Medicine Man totems, Halazzi totem, and Jan'alai Hatchers.
inline constexpr float ZA_CREATURE_SEARCH_RADIUS = 40.0f;

Unit* GetZaBoss(Player* bot, ZaBosses boss);

// Akil'zon <Eagle Avatar>

// Electrical Storm runs on a fixed 60s cycle once the encounter starts. Bots react from
// AKILZON_STORM_LEAD_MS before each cast until the storm itself has expired.
inline constexpr uint32 AKILZON_STORM_PERIOD_MS = 60 * IN_MILLISECONDS;
inline constexpr uint32 AKILZON_STORM_LEAD_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 AKILZON_STORM_DURATION_MS = 10 * IN_MILLISECONDS;
inline constexpr float AKILZON_RANGED_SPREAD_DISTANCE = 13.0f;

// Centerish of the platform.
inline Position const AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };

bool IsInStormWindow(uint32 startMs);
Player* GetElectricalStormTarget(Player* bot);

// Nalorakk <Bear Avatar>

inline constexpr float NALORAKK_RANGED_SPREAD_DISTANCE = 11.0f;
// Toward the back, down the stairs from his starting position.
inline Position const NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };

bool IsNalorakkInBearForm(Unit* nalorakk);

// Jan'alai <Dragonhawk Avatar>

inline constexpr float JANALAI_PLATFORM_Z = 19.146f;
inline constexpr float JANALAI_RANGED_SPREAD_RADIUS = 15.0f;
// Around JANALAI_TANK_POSITION, where Jan'alai is moved for each Fire Bomb event. All Bombs and the
// Fire Wall are within 34 yd of it.
inline constexpr float JANALAI_FIRE_BOMB_SEARCH_RADIUS = 40.0f;
inline constexpr float JANALAI_FIRE_BOMB_SAFE_SPOT_SEARCH_RADIUS = 30.0f;
// Fire Bomb (42630) has a 4 yd radius. The 1.5 yd extra padding is added for realism.
inline constexpr float JANALAI_FIRE_BOMB_HAZARD_RADIUS = 4.0f;
inline constexpr float JANALAI_FIRE_BOMB_CLEARANCE = JANALAI_FIRE_BOMB_HAZARD_RADIUS + 1.5f;
inline constexpr uint32 FIRE_BOMB_CACHE_INTERVAL_MS = 1000;
// Hatchers open eggs at increasing speed: 1 on the first tick, then 2, then 3, every 5s. There are
// 20 eggs per side. Bloodlust is held until this many Hatchlings are active.
inline constexpr uint32 JANALAI_BLOODLUST_HATCHLING_COUNT = 6;
// Jan'alai hatches every remaining egg at once at 35% HP. If somehow a group killed both sides of
// eggs incrementally and so fast that 6 Hatchlings were never up at a time, then Bloodlust goes out
// at this point. Using 33% accounts for some delay for the hatching event to occur.
inline constexpr float JANALAI_HATCH_ALL_HEALTH_PCT = 33.0f;

// Centerish of the platform
inline Position const JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };
// The safe zone is about the entire reachable platform, minus the destroyed, fenced-off corner and
// the parts that jut out on each side and connect to the bridges to the eggs. The measured area
// is right up against the Fire Wall, but that's fine because the Fire Wall's damage actually comes
// from the same Fire Bomb trigger NPCs so the Fire Bomb avoidance will keep away from the wall too.
inline std::vector<Position> const JANALAI_SAFE_ZONE = {
    Position(-12.576924f, 1131.9163f, JANALAI_PLATFORM_Z),
    Position(-52.068108f, 1132.8198f, JANALAI_PLATFORM_Z),
    Position(-51.508590f, 1158.4890f, JANALAI_PLATFORM_Z),
    Position(-44.008713f, 1168.6125f, JANALAI_PLATFORM_Z),
    Position(-12.115170f, 1167.6681f, JANALAI_PLATFORM_Z)
};

// Around JANALAI_TANK_POSITION. Every egg and both Hatcher spawns are within 70 yd of it.
inline constexpr float JANALAI_ADD_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 JANALAI_ADD_CACHE_INTERVAL_MS = 200;

// Living Hatchers and Hatchlings.
GuidVector FindJanalaiAddGuids(Player* bot);
std::vector<Unit*> GetJanalaiAdds(Player* bot, ZaNpcs npc);
std::pair<Unit*, Unit*> GetAmanishiHatcherPair(Player* bot);
uint32 CountJanalaiHatchlingsByEntry(Player* bot);
// Jan'alai casts Fire Bomb Channel at the beginning of each Fire Bomb event until bombs detonate.
bool IsJanalaiBombing(Unit* janalai);
GuidVector FindFireBombGuids(Player* bot);
std::vector<Unit*> GetFireBombs(Player* bot);
bool FindSafeStepInJanalaiZone(
    Player* bot, std::vector<Unit*> const& hazards, std::vector<Position> const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ);

// Halazzi <Lynx Avatar>

// Around HALAZZI_TANK_POSITION. He summons the totem where he stands; 100 yd leaves room for
// wherever he is tanked.
inline constexpr float HALAZZI_TOTEM_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 HALAZZI_TOTEM_CACHE_INTERVAL_MS = 200;

// Right near his starting position.
inline Position const HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

GuidVector FindCorruptedLightningTotemGuids(Player* bot);
Creature* GetNearestCorruptedLightningTotem(Player* bot);

// Hex Lord Malacrass

// Freezing Trap (43448) stuns for 10s across a 10 yd radius, even though its trigger radius is much
// shorter at 2.5 yd.
inline constexpr float ZA_FREEZING_TRAP_CLEARANCE = 11.0f;
inline constexpr float ZA_FREEZING_TRAP_SEARCH_RADIUS = 16.0f;
inline constexpr uint32 FREEZING_TRAP_CACHE_INTERVAL_MS = 200;
// Around Malacrass's spawn. The adds spawn in a row within 12 yd of it; 100 yd leaves room for
// wherever they are tanked.
inline constexpr float HEX_LORD_MALACRASS_DPS_TARGET_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 HEX_LORD_MALACRASS_DPS_TARGET_CACHE_INTERVAL_MS = 200;

// Highest priority first.
inline constexpr std::array HEX_LORD_MALACRASS_DPS_PRIORITY = {
    ZaNpcs::NPC_LORD_RAADAN,
    ZaNpcs::NPC_ALYSON_ANTILLE,
    ZaNpcs::NPC_KORAGG,
    ZaNpcs::NPC_DARKHEART,
    ZaNpcs::NPC_FENSTALKER,
    ZaNpcs::NPC_GAZAKROTH,
    ZaNpcs::NPC_THURG,
    ZaNpcs::NPC_SLITHER,
    ZaNpcs::NPC_HEX_LORD_MALACRASS,
};

inline Position const HEX_LORD_MALACRASS_SPAWN_POSITION = { 117.363f, 923.569f, 33.973f };

ObjectGuid FindNearbyFreezingTrapGuid(Player* bot);
GameObject* GetNearbyFreezingTrap(PlayerbotAI* botAI);
GuidVector FindHexLordMalacrassDpsTargetGuids(Player* bot);

// Zul'jin

// 4 Cyclones (NPC_FEATHER_VORTEX) chase random raid members at 7 yd/s (player run speed) and have
// a 4 yd radius. They're not really dodgeable as a result, so the approach is just to spread ranged
// out for the Cyclones to have to travel farther and not be able to hit as many targets at once.
inline constexpr float ZULJIN_SPREAD_Z = 45.111f;
inline constexpr float ZULJIN_DRAGONHAWK_RANGED_SPREAD_DISTANCE = 6.0f;

// Centerish of the platform
inline Position const ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };

// Ranged are ordered by healers first. The healer positions are centered at (1) the bottom of the
// stairs leading to Zul'jin's starting spot and (2) the top of the stairs leading to his battle
// area. The intent is to allow each healer to reach every other bot's position (accordingly, every
// position is within 39 yd of both healer positions).
inline std::array<Position, 8> const ZULJIN_SPREAD_POSITIONS = {{
    // Healer positions.
    Position(120.462f, 728.502f, ZULJIN_SPREAD_Z),
    Position(119.984f, 693.188f, ZULJIN_SPREAD_Z),
    // Ranged dps positions. Three positions each at opposite ends of the platform.
    Position(94.939f, 713.698f, ZULJIN_SPREAD_Z),
    Position(145.286f, 713.034f, ZULJIN_SPREAD_Z),
    Position(103.869f, 728.703f, ZULJIN_SPREAD_Z),
    Position(135.847f, 728.816f, ZULJIN_SPREAD_Z),
    Position(143.862f, 697.302f, ZULJIN_SPREAD_Z),
    Position(95.618f, 698.439f, ZULJIN_SPREAD_Z)
}};

bool GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex);
Player* GetZuljinCreepingParalysisDispelTarget(Player* bot);

// Shared encounter state

struct ZaInstanceState
{
    std::optional<uint32> akilzonStormTimer;
    EncounterHelpers::InstanceCreatureList janalaiAdds;
    EncounterHelpers::InstanceCreatureList janalaiFireBombs;
    EncounterHelpers::InstanceCreatureList halazziTotems;
    EncounterHelpers::InstanceCreatureList hexLordMalacrassDpsTargets;
};

ZaInstanceState& ZaState(uint32 instanceId);
bool ZaResetInstance(uint32 instanceId);

}

#endif
