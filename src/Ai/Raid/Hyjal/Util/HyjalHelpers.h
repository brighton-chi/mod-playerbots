/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALHELPERS_H
#define PLAYERBOTS_HYJALHELPERS_H

#include "ObjectGuid.h"
#include "Position.h"
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace HyjalHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class HyjalSpells : uint32
{
    // Rage Winterchill
    SPELL_DEATH_AND_DECAY  = 31258,

    // Anetheron
    SPELL_INFERNO          = 31299,

    // Kaz'rogal
    SPELL_MARK_OF_KAZROGAL = 31447,

    // Azgalor
    SPELL_RAIN_OF_FIRE     = 31340,
    SPELL_DOOM             = 31347,

    // Archimonde
    SPELL_DOOMFIRE_TRAIL   = 31943, // 6y persistent area aura dropped along the trail, lasts 18s
    SPELL_DOOMFIRE         = 31944, // Damaging part of trail
    SPELL_DOOMFIRE_DOT     = 31969, // DoT after exiting trail
    // SPELL_ARCHIMONDE_FEAR  = 31970,
    SPELL_AIR_BURST        = 32014,

    // Hunter
    SPELL_MISDIRECTION     = 35079,

    // Mage
    SPELL_ICE_BLOCK        = 45438,

    // Paladin
    SPELL_DIVINE_SHIELD    = 642,

    // Priest
    SPELL_FEAR_WARD        = 6346,

    // Shaman
    SPELL_TREMOR_TOTEM     = 8143,
};

enum class HyjalNpcs : uint32
{
    // Archimonde
    NPC_DOOMFIRE           = 18095,
};

enum class TankPositionState : uint8
{
    MovingToTransition = 0,
    MovingToFinal      = 1,
    Positioned         = 2,
    Unknown            = 255,
};

// General
inline constexpr uint32 HYJAL_MAP_ID = 534;
inline constexpr float HAZARD_SEARCH_RADIUS = 60.0f; // For Death & Decay and Rain of Fire
struct RangedGroups
{
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
};
RangedGroups GetRangedGroups(Player* bot);
std::pair<size_t, size_t> GetBotCircleIndexAndCount(Player* bot, RangedGroups const& groups);

// Rage Winterchill
inline constexpr float DEATH_AND_DECAY_SAFE_RADIUS = 22.0f; // 20y radius + 1.5y player hitbox + 0.5y buffer
inline Position const WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
bool GetDeathAndDecayPosition(Player* bot, Position& deathAndDecay); // at most one is ever up
bool IsNearDeathAndDecay(Player* bot, float radius); // for callers wanting a margin on the hazard
bool IsInDeathAndDecay(Player* bot);

// Anetheron
inline Position const ANETHERON_TANK_POSITION =       { 5033.177f, -1765.996f, 1324.195f };
inline Position const ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
inline Position const ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
Player* GetInfernoTarget(Unit* anetheron);
Position const& GetClosestInfernalTankPosition(Player* bot);

// Kaz'rogal
inline Position const KAZROGAL_TANK_TRANSITION_POSITION = { 5528.792f, -2636.486f, 1481.293f };
inline Position const KAZROGAL_TANK_FINAL_POSITION =      { 5511.514f, -2662.466f, 1480.288f };
extern std::unordered_map<ObjectGuid, TankPositionState> kazrogalTankStep;
extern std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

// Azgalor
inline constexpr float RAIN_OF_FIRE_RADIUS = 17.0f; // 15y radius + 1.5y player hitbox + 0.5y buffer
inline Position const AZGALOR_TANK_TRANSITION_POSITION = { 5486.787f, -2696.215f, 1482.007f };
inline Position const AZGALOR_TANK_FINAL_POSITION =      { 5496.379f, -2675.265f, 1481.053f };
inline Position const AZGALOR_DOOMGUARD_POSITION =       { 5485.555f, -2731.659f, 1485.555f };
extern std::unordered_map<ObjectGuid, TankPositionState> azgalorTankStep;
TankPositionState GetAzgalorTankPositionState(PlayerbotAI* botAI, Player* bot);
std::vector<Position> GetRainOfFirePositions(Player* bot);
bool IsNearRainOfFire(Player* bot, float radius); // for callers wanting a margin on the hazard
bool IsInRainOfFire(Player* bot);
// Cleave chains from Azgalor's victim to four more players, but only within this distance of
// that victim and inside his frontal arc. Both are padded past the 10y jump radius and the
// 180 degree filter in Spell::SearchChainTargets, since he turns with the tank
inline constexpr float CLEAVE_CHAIN_RADIUS = 12.0f;
inline constexpr float CLEAVE_DANGER_ARC = 200.0f * static_cast<float>(M_PI) / 180.0f;
bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y);
bool AnyGroupMemberHasDoom(Player* bot);

// Archimonde
struct AirBurstData
{
    ObjectGuid targetGuid;
    uint32 castTime;
};
inline constexpr float AIR_BURST_SAFE_DISTANCE = 15.0f;
inline Position const ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };
extern std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;
AirBurstData* GetRecentArchimondeAirBurst(uint32 instanceId);

}

#endif
