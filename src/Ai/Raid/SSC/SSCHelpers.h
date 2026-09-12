/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCHELPERS_H
#define PLAYERBOTS_SSCHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

class Creature;
class Map;
class Player;
class PlayerbotAI;
class Unit;

namespace SscHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class SscSpells : uint32
{
    // Trash Mobs
    SPELL_TOXIC_POOL             = 38718,

    // Hydross the Unstable <Duke of Currents>
    SPELL_MARK_OF_HYDROSS_10     = 38215,
    SPELL_MARK_OF_HYDROSS_25     = 38216,
    SPELL_MARK_OF_HYDROSS_50     = 38217,
    SPELL_MARK_OF_HYDROSS_100    = 38218,
    SPELL_MARK_OF_HYDROSS_250    = 38231,
    SPELL_MARK_OF_HYDROSS_500    = 40584,
    SPELL_MARK_OF_CORRUPTION_10  = 38219,
    SPELL_MARK_OF_CORRUPTION_25  = 38220,
    SPELL_MARK_OF_CORRUPTION_50  = 38221,
    SPELL_MARK_OF_CORRUPTION_100 = 38222,
    SPELL_MARK_OF_CORRUPTION_250 = 38230,
    SPELL_MARK_OF_CORRUPTION_500 = 40583,
    SPELL_HYDROSS_CORRUPTION     = 37961,

    // The Lurker Below
    SPELL_SPOUT_VISUAL           = 37431, // 3s wind-up cast
    SPELL_SPOUT_COUNTERCLOCKWISE = 37429, // 16s aura; facing +0.1 rad every 250ms, cone each tick
    SPELL_SPOUT_CLOCKWISE        = 37430, // same, -0.1 rad

    // Leotheras the Blind
    SPELL_LEOTHERAS_BANISHED     = 37546,
    SPELL_WHIRLWIND              = 37640,
    SPELL_WHIRLWIND_CHANNEL      = 37641,
    SPELL_METAMORPHOSIS          = 37673,
    SPELL_CHAOS_BLAST            = 37675,
    SPELL_INSIDIOUS_WHISPER      = 37676,

    // Lady Vashj <Coilfang Matron>
    SPELL_FEAR_WARD              =  6346,
    SPELL_MAGIC_BARRIER          = 38112,
    SPELL_POISON_BOLT            = 38253,
    SPELL_STATIC_CHARGE          = 38280,
    SPELL_ENTANGLE               = 38316,

    // Druid
    SPELL_CAT_FORM               =   768,
    SPELL_BEAR_FORM              =  5487,
    SPELL_DIRE_BEAR_FORM         =  9634,
    SPELL_TREE_OF_LIFE           = 33891,
    SPELL_DRUID_BERSERK          = 50334,

    // Hunter
    SPELL_MISDIRECTION           = 35079,

    // Mage
    SPELL_SLOW                   = 31589,

    // Paladin
    SPELL_AVENGING_WRATH         = 31884,

    // Rogue
    SPELL_CLOAK_OF_SHADOWS       = 31224,

    // Shaman
    SPELL_GROUNDING_TOTEM_EFFECT =  8178,

    // Warlock
    SPELL_CURSE_OF_EXHAUSTION    = 18223,
};

enum class SscNpcs : uint32
{
    // Trash Mobs
    NPC_WATER_ELEMENTAL_TOTEM    = 22236,

    // Hydross the Unstable <Duke of Currents>
    NPC_PURE_SPAWN_OF_HYDROSS    = 22035,
    NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

    // The Lurker Below
    NPC_COILFANG_GUARDIAN        = 21873,

    // Leotheras the Blind
    NPC_LEOTHERAS_THE_BLIND      = 21215,
    NPC_INNER_DEMON              = 21857,
    NPC_SHADOW_OF_LEOTHERAS      = 21875,

    // Fathom-Lord Karathress
    NPC_SPITFIRE_TOTEM           = 22091,

    // Lady Vashj <Coilfang Matron>
    NPC_WORLD_INVISIBLE_TRIGGER  = 12999,
    NPC_LADY_VASHJ               = 21212,
    NPC_ENCHANTED_ELEMENTAL      = 21958,
    NPC_TAINTED_ELEMENTAL        = 22009,
    NPC_COILFANG_ELITE           = 22055,
    NPC_COILFANG_STRIDER         = 22056,
    NPC_TOXIC_SPOREBAT           = 22140,
    NPC_SPORE_DROP_TRIGGER       = 22207,
};

enum class SscItems : uint32
{
    // Lady Vashj <Coilfang Matron>
    ITEM_TAINTED_CORE            = 31088,
};

inline constexpr uint32 SSC_MAP_ID = 548;
inline constexpr uint32 HAZARD_CACHE_INTERVAL = 200;

// Trash

// 25y radius + ~2y player CombatReach; see the Hyjal D&D note on persistent ground AoE range in AC.
inline constexpr float TOXIC_POOL_HAZARD_RADIUS = 27.0f;
inline constexpr float TOXIC_POOL_HOLDING_RADIUS = TOXIC_POOL_HAZARD_RADIUS + 5.0f; // For multiplier
inline constexpr float TOXIC_POOL_SEARCH_RADIUS = TOXIC_POOL_HOLDING_RADIUS + 2.0f; // 2y margin for hazard search

std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, std::string const& value);
// A step straight out of a circular hazard, fanning around the escape heading on collision. Copied
// from HyjalHelpers pending promotion to EncounterHelpers.
bool GetHazardEscapeStep(
    Player* bot, Position const& hazard, float escapeRadius, float moveDist, float& stepX,
    float& stepY, float& stepZ);
bool GetToxicPoolPosition(PlayerbotAI* botAI, Position& toxicPool);
bool IsNearToxicPool(PlayerbotAI* botAI, float radius);
bool IsInToxicPool(PlayerbotAI* botAI);

// Hydross the Unstable <Duke of Currents>

inline Position const HYDROSS_FROST_TANK_POSITION =  { -236.669f, -358.352f, -0.828f };
inline Position const HYDROSS_NATURE_TANK_POSITION = { -225.471f, -327.790f, -3.682f };

extern std::unordered_map<uint32, uint32> hydrossFrostDpsWaitTimer;
extern std::unordered_map<uint32, uint32> hydrossNatureDpsWaitTimer;
extern std::unordered_map<uint32, uint32> hydrossChangeToFrostPhaseTimer;
extern std::unordered_map<uint32, uint32> hydrossChangeToNaturePhaseTimer;

// The main tank holds Hydross in frost, the first assist tank in nature. Every other tank is an
// add tank and never touches Hydross.
bool IsHydrossPhaseTank(Player* bot);
bool IsHydrossAddTank(Player* bot);
bool IsHydrossInFrostPhase(Unit* hydross);
bool IsHydrossInNaturePhase(Unit* hydross);
bool HasMarkOfHydrossAt100Percent(Player* bot);
bool HasNoMarkOfHydross(Player* bot);
bool HasMarkOfCorruptionAt100Percent(Player* bot);
bool HasNoMarkOfCorruption(Player* bot);

// The Lurker Below

inline Position const LURKER_MAIN_TANK_POSITION = { 23.706f, -406.038f, -19.686f };

extern std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;

// The script sets REACT_PASSIVE on the first tick of the Spout wind-up and REACT_AGGRESSIVE when
// the rotation aura drops 19s later, and at no other point while in combat; Submerge uses the
// stand state instead.
bool IsLurkerSpouting(Unit* lurker);
// +1 counter-clockwise, -1 clockwise, 0 during the 3s wind-up before the spin starts.
int8 GetLurkerSpoutSpin(Unit* lurker);

// Spout sweeps at 0.4 rad/s. A bot running at 7 yd/s manages 7 / r rad/s, so the ring radius is
// the speed: 17 yd keeps pace with the beam, 21 yd falls behind at 0.07 rad/s. Each bot gets a
// fixed radius in this band and a fixed offset around "behind" from its GUID so the raid looks
// spread rather than stacked, without the destination moving from tick to tick.
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MIN = 17.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MAX = 21.0f;
inline constexpr float LURKER_SPOUT_RUN_ARC_HALF_WIDTH = static_cast<float>(M_PI) / 3.0f;
inline constexpr float LURKER_SPOUT_RUN_STEP = 3.5f;
inline constexpr float LURKER_SPOUT_RUN_ANGULAR_DEADZONE = 0.105f; // ~6 degrees

// Submerge: three Coilfang Guardians, one each for the main tank and the first two assist tanks.
// The guardians are found by a sorted, cached grid search so every tank sees the same list in the
// same order (summon GUIDs are sequential, so sorted is spawn order).
inline constexpr uint32 LURKER_GUARDIAN_CACHE_INTERVAL = 200;
inline constexpr float LURKER_GUARDIAN_SEARCH_RADIUS = 100.0f;
inline constexpr size_t LURKER_GUARDIAN_TANK_COUNT = 3;
// How far apart the tanks hold their guardians, and the step used to get there.
inline constexpr float LURKER_GUARDIAN_TANK_SEPARATION = 20.0f;
inline constexpr float LURKER_GUARDIAN_TANK_MOVE_STEP = 2.25f;
inline constexpr float LURKER_GUARDIAN_TANK_MOVE_DEADZONE = 1.5f;

extern std::unordered_map<uint32, std::array<ObjectGuid, LURKER_GUARDIAN_TANK_COUNT>>
    lurkerGuardianTankAssignments;

GuidVector FindLurkerGuardianGuids(Player* bot);
std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI);
// The guardian tanks in index order; empty unless all three exist.
std::vector<Player*> GetLurkerGuardianTanks(Player* bot);
bool CastTauntOn(PlayerbotAI* botAI, Unit* target);

// Leotheras the Blind

inline constexpr float LEOTHERAS_SEARCH_DISTANCE = 100.0f;

extern std::unordered_map<uint32, uint32> leotherasHumanoidPhaseDpsWaitTimer;
extern std::unordered_map<uint32, uint32> leotherasDemonPhaseDpsWaitTimer;
extern std::unordered_map<uint32, uint32> leotherasFinalPhaseDpsWaitTimer;

bool IsSpellbinderPhase(Creature* leotheras);
Creature* GetActiveLeotherasHumanoid(Player* bot);
bool IsLeotherasHumanoidPhase(Player* bot);
Creature* GetPhase2LeotherasDemon(Player* bot);
bool IsLeotherasDemonPhase(Player* bot);
Creature* GetPhase3LeotherasDemon(Player* bot);
bool IsLeotherasFinalPhase(Player* bot);
Creature* GetActiveLeotherasDemon(Player* bot);
Player* GetLeotherasWarlockTank(Player* bot);
bool IsLeotherasWarlockTank(Player* bot);
bool IsLeotherasChannelingWhirlwind(Unit* leotheras);
bool HasTooManyChaosBlastStacks(Player* bot);
bool HasInnerDemon(Player* bot);
Creature* GetPersonalInnerDemon(PlayerbotAI* botAI);

// Fathom-Lord Karathress

inline Position const KARATHRESS_TANK_POSITION = { 474.403f, -531.118f, -7.548f };
inline Position const TIDALVESS_TANK_POSITION = { 511.282f, -501.162f, -13.158f };
inline Position const SHARKKIS_TANK_POSITION = { 508.057f, -541.109f, -10.133f };
inline Position const CARIBDIS_TANK_POSITION = { 464.462f, -475.820f, -13.158f };
inline Position const CARIBDIS_HEALER_POSITION = { 466.203f, -503.201f, -13.158f };
inline Position const CARIBDIS_RANGED_DPS_POSITION = { 463.197f, -501.190f, -13.158f };

extern std::unordered_map<uint32, uint32> karathressDpsWaitTimer;

// Morogrim Tidewalker

inline constexpr float TIDEWALKER_PHASE_2_HEALTH_PCT = 25.0f;

inline Position const TIDEWALKER_PHASE_1_TANK_POSITION = { 410.925f, -741.916f, -7.146f };
inline Position const TIDEWALKER_PHASE_TRANSITION_WAYPOINT = { 407.035f, -759.479f, -7.168f };
inline Position const TIDEWALKER_PHASE_2_TANK_POSITION = { 446.571f, -767.155f, -7.144f };
inline Position const TIDEWALKER_PHASE_2_RANGED_POSITION = { 432.595f, -766.288f, -7.145f };

extern std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
extern std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

// Lady Vashj <Coilfang Matron>

struct GeneratorInfo
{
    ObjectGuid guid;
    float x;
    float y;
    float z;
};

inline constexpr float VASHJ_PLATFORM_CENTER_Z = 42.902f;
inline constexpr float VASHJ_PLATFORM_EDGE_Z = 41.097f;

inline Position const VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.902f };

extern std::unordered_map<ObjectGuid, bool> hasReachedVashjRangedPosition;
extern std::unordered_map<uint32, ObjectGuid> nearestVashjGeneratorTriggerGuid;
extern std::unordered_map<ObjectGuid, Position> intendedVashjCorePasserLineup;
extern std::unordered_map<uint32, uint32> lastVashjCoreImbueAttempt;
extern std::unordered_map<ObjectGuid, uint32> lastVashjCoreInInventoryTime;

bool IsMainTankInSameSubgroup(Player* bot);
int8 GetLadyVashjPhase(Unit* vashj);
bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* botAI, Unit* vashj);
Player* GetDesignatedCoreLooter(PlayerbotAI* botAI, Player* bot);
Player* GetFirstTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
Player* GetSecondTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
Player* GetThirdTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
Player* GetFourthTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
std::array<Player*, 5> GetCoreHandlers(PlayerbotAI* botAI, Player* bot);
bool AnyRecentCoreInInventory(PlayerbotAI* botAI, Player* bot);
std::vector<uint32> const SHIELD_GENERATOR_DB_GUIDS =
{
    47482, // NW
    47483, // NE
    47484, // SE
    47485  // SW
};
std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(
    Map* map, std::vector<uint32> const& generatorDbGuids);
Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Unit* reference);
GeneratorInfo const* GetNearestGeneratorToBot(
    Player* bot, std::vector<GeneratorInfo> const& generators);

}

#endif
