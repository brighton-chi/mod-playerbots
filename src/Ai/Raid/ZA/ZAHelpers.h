/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAHELPERS_H
#define PLAYERBOTS_ZAHELPERS_H

#include "Common.h"
#include "Position.h"
#include "Unit.h"
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
    NPC_FEATHER_VORTEX              = 24136,
};

enum class ZaObjects : uint32
{
    GO_FREEZING_TRAP                = 186669,
};

// General
inline constexpr uint32 ZA_MAP_ID = 568;
inline constexpr float ZA_PULL_COMPLETE_HP_PERCENT = 95.0f;
Position FindSafestNearbyPosition(
    Player* bot, std::vector<Unit*> const& hazards, Position const& center,
    float safeZoneRadius, float hazardRadius, bool requireSafePath);
bool IsPathSafeFromHazards(
    Position const& start, Position const& end,
    std::vector<Unit*> const& hazards, float hazardRadius);
bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius);
std::vector<Unit*> GetAllHazardTriggers(Player* bot, uint32 entry, float searchRadius);

// Akil'zon <Eagle Avatar>
inline Position const AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };
extern std::unordered_map<uint32, time_t> akilzonStormTimer;
bool IsInStormWindow(time_t start, time_t now);
Player* GetElectricalStormTarget(Player* bot);

// Nalorakk <Bear Avatar>
inline Position const NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };
bool IsNalorakkInBearForm(Unit* nalorakk);

// Jan'alai <Dragonhawk Avatar>
inline Position const JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };
bool HasFireBombNearby(Player* bot);
std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI);

// Halazzi <Lynx Avatar>
inline Position const HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

// Hex Lord Malacrass
inline constexpr float ZA_FREEZING_TRAP_SEARCH_RADIUS = 20.0f;

// Zul'jin
inline Position const ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };

}

#endif
