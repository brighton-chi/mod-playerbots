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
    SPELL_SPOUT_COUNTERCLOCKWISE = 37429,
    SPELL_SPOUT_CLOCKWISE        = 37430,

    // Leotheras the Blind
    SPELL_LEOTHERAS_BANISHED     = 37546,
    SPELL_WHIRLWIND              = 37640,
    SPELL_WHIRLWIND_CHANNEL      = 37641,
    SPELL_METAMORPHOSIS          = 37673,
    SPELL_CHAOS_BLAST            = 37675,
    SPELL_INSIDIOUS_WHISPER      = 37676,

    // Fathom-Lord Karathress
    SPELL_CYCLONE                = 38517, // 4 yd feather fall + knockback every 1s, 5s aura

    // Lady Vashj <Coilfang Matron>
    SPELL_FEAR_WARD              =  6346,
    SPELL_MAGIC_BARRIER          = 38112,
    SPELL_POISON_BOLT            = 38253,
    SPELL_STATIC_CHARGE          = 38280,
    SPELL_ENTANGLE               = 38316,
    SPELL_TOXIC_SPORES           = 38575, // the pool a Spore Drop Trigger lays

    // Druid
    SPELL_FAERIE_FIRE_FERAL      = 16857,
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

    // Warrior
    SPELL_VIGILANCE              = 50720,
};

enum class SscNpcs : uint32
{
    // Trash Mobs
    NPC_WATER_ELEMENTAL_TOTEM    = 22236,

    // The Lurker Below
    NPC_COILFANG_AMBUSHER        = 21865,
    NPC_COILFANG_GUARDIAN        = 21873,

    // Leotheras the Blind
    NPC_LEOTHERAS_THE_BLIND      = 21215,
    NPC_INNER_DEMON              = 21857,
    NPC_SHADOW_OF_LEOTHERAS      = 21875,

    // Morogrim Tidewalker
    NPC_TIDEWALKER_LURKER        = 21920,

    // Fathom-Lord Karathress
    NPC_SPITFIRE_TOTEM           = 22091,
    NPC_FATHOM_LURKER            = 22119,
    NPC_FATHOM_SPOREBAT          = 22120,

    // Lady Vashj <Coilfang Matron>
    NPC_WORLD_INVISIBLE_TRIGGER  = 12999,
    NPC_LADY_VASHJ               = 21212,
    NPC_ENCHANTED_ELEMENTAL      = 21958,
    NPC_TAINTED_ELEMENTAL        = 22009,
    NPC_COILFANG_ELITE           = 22055,
    NPC_COILFANG_STRIDER         = 22056,
    NPC_TOXIC_SPOREBAT           = 22140,
};

enum class SscItems : uint32
{
    // Lady Vashj <Coilfang Matron>
    ITEM_TAINTED_CORE            = 31088,
};

// General

inline constexpr uint32 SSC_MAP_ID = 548;
inline constexpr uint32 HAZARD_CACHE_INTERVAL_MS = 200;
// Steps short enough to navigate poor terrain, matching the standard in EncounterHelpers.
inline constexpr float PATH_STEP_DISTANCE = 3.5f;
inline constexpr float PATH_BACKWARD_STEP_DISTANCE = 2.25f;

// A step out of a circular hazard.
bool FindHazardEscapeStep(
    Player* bot, Position const& hazard, float moveDist, float& stepX, float& stepY, float& stepZ);
// True where the map has ground above any liquid at x/y.
bool IsDryGround(Player* bot, float x, float y);
// One step along the bot's path to a point, stopping short of it by stopDistance. The step
// follows the path corner-by-corner, rather than aiming at the far end of it, so a bot can move
// around a pillar between it and the point.
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY);

// Trash

// 25y radius + ~2y player CombatReach; see the Hyjal D&D note on persistent ground AoE range in AC.
inline constexpr float TOXIC_POOL_HAZARD_RADIUS = 27.0f;
inline constexpr float TOXIC_POOL_HOLDING_RADIUS = TOXIC_POOL_HAZARD_RADIUS + 5.0f;
inline constexpr float TOXIC_POOL_SEARCH_RADIUS = TOXIC_POOL_HOLDING_RADIUS + 2.0f;

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

// The main tank holds Hydross in frost phase, the first assist tank in nature phase. Every other
// tank is an add tank and picks up the Elementals that spawn upon phase changes.
bool IsHydrossPhaseTank(Player* bot);
bool IsHydrossAddTank(Player* bot);
bool IsHydrossInFrostPhase(Unit* hydross);
bool IsHydrossInNaturePhase(Unit* hydross);
bool HasMarkOfHydrossAt100Percent(Player* bot);
bool HasNoMarkOfHydross(Player* bot);
bool HasMarkOfCorruptionAt100Percent(Player* bot);
bool HasNoMarkOfCorruption(Player* bot);

// The Lurker Below

inline constexpr float LURKER_WHIRL_RADIUS = 25.0f;
inline constexpr float LURKER_RANGED_SAFE_DISTANCE = LURKER_WHIRL_RADIUS + 2.0f;

// Spout avoidance mechanics:
// Each bot is assigned a radius from Lurker from 19-21y. The range is to make things look more
// artificial, and the intent is to keep the radius close to Lurker while keeping the circle on dry
// land as much as possible (water is not completely avoidable due to a couple of spillways).
// Within the 19-21y band that is within a 60° cone behind Lurker, a bot is considered safe. Any
// bot in that safe zone will wait during the Spout windup until the spin direction is determined.
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MIN = 19.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MAX = 21.0f;
inline constexpr float LURKER_SPOUT_RUN_ARC_HALF_WIDTH = static_cast<float>(M_PI) / 3.0f;
inline constexpr float LURKER_SPOUT_RUN_STEP = 7.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIAL_DEADZONE = 2.0f;
// A bot may run this far past directly behind Lurker, in the spin direction, before it stops.
// This is to prevent the very intelligent bots from lapping Lurker and getting blasted.
inline constexpr float LURKER_SPOUT_RUN_OVERTAKE_MARGIN = static_cast<float>(M_PI) / 6.0f;

// Submerge: A Coilfang Guardian is assigned to each of the main tank and first two assist tanks.
// Assignment is by summon GUID (so spawn order) and persists after a Guardian is killed.
inline constexpr size_t LURKER_GUARDIAN_TANK_COUNT = 3;
inline constexpr uint32 LURKER_GUARDIAN_CACHE_INTERVAL_MS = 200;
inline constexpr float LURKER_GUARDIAN_SEARCH_RADIUS = 100.0f;

inline Position const LURKER_MAIN_TANK_POSITION = { 23.706f, -406.038f, -19.686f };

extern std::unordered_map<uint32, std::array<ObjectGuid, LURKER_GUARDIAN_TANK_COUNT>>
    lurkerGuardianTankAssignments;

// Reading REACT_PASSIVE is the easiest way to capture the entire Spout sequence.
// The actual spell mechanics are a 3s wind-up (37431), followed by a 16s aura for the spin at a
// speed of 0.1 rad/250ms. The spin aura differs for counterclockwise (37429) and clockwise (37430).
bool IsLurkerSpouting(Unit* lurker);
// Captures when Lurker is neither Spouting nor submerged.
bool IsLurkerSurfacedAndCalm(Unit* lurker);
// +1 counter-clockwise, -1 clockwise, 0 during the wind-up.
int8 GetLurkerSpoutSpin(Unit* lurker);
// True if a navmesh path from the bot to x/y sets off around Lurker in the given angular
// direction (+1 counter-clockwise, -1 clockwise).
bool DoesPathRoundLurker(Player* bot, Unit* lurker, float x, float y, float z, int8 direction);
// True if a navmesh path from the bot ends within tolerance of x/y.
bool DoesPathArrive(Player* bot, float x, float y, float z, float tolerance);
GuidVector FindLurkerGuardianGuids(Player* bot);
std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI);
// The Guardian tanks in index order; empty if there are fewer than 3 bot tanks.
std::vector<Player*> GetLurkerGuardianTanks(Player* bot);

// Leotheras the Blind

inline constexpr float LEOTHERAS_SEARCH_DISTANCE = 100.0f;
inline constexpr uint32 LEOTHERAS_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 LEOTHERAS_HUMANOID_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_DEMON_DPS_WAIT_MS = 10 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_FINAL_DPS_WAIT_MS = 5 * IN_MILLISECONDS;

extern std::unordered_map<uint32, uint32> leotherasHumanoidPhaseDpsWaitTimer;
// When the current Whirlwind will end, determined by the aura's remaining duration.
extern std::unordered_map<uint32, uint32> leotherasWhirlwindEndTime;
extern std::unordered_map<uint32, uint32> leotherasDemonPhaseDpsWaitTimer;
extern std::unordered_map<uint32, uint32> leotherasFinalPhaseDpsWaitTimer;

ObjectGuid FindLeotherasGuid(Player* bot);
ObjectGuid FindShadowOfLeotherasGuid(Player* bot);
Creature* GetLeotheras(PlayerbotAI* botAI);
bool IsSpellbinderPhase(Unit* leotheras);
Creature* GetActiveLeotherasHumanoid(PlayerbotAI* botAI);
bool IsLeotherasHumanoidPhase(PlayerbotAI* botAI);
Creature* GetPhase2LeotherasDemon(PlayerbotAI* botAI);
bool IsLeotherasDemonPhase(PlayerbotAI* botAI);
Creature* GetPhase3LeotherasDemon(PlayerbotAI* botAI);
bool IsLeotherasFinalPhase(PlayerbotAI* botAI);
Creature* GetActiveLeotherasDemon(PlayerbotAI* botAI);
Player* GetLeotherasWarlockTank(Player* bot);
bool IsLeotherasWarlockTank(Player* bot);
bool IsLeotherasChannelingWhirlwind(Unit* leotheras);
bool HasTooManyChaosBlastStacks(Player* bot);
bool HasInnerDemon(Player* bot);
Creature* GetPersonalInnerDemon(PlayerbotAI* botAI);

// Fathom-Lord Karathress

// The healer keeps to Caribdis herself, so she is covered wherever any tank puts her, and her
// victim is not used as the anchor because it jumps into the room whenever the tank loses her.
// The tank stands on her, so 32 yd from her is about 35 yd from the tank against a 40 yd heal.
inline constexpr float CARIBDIS_HEALER_DISTANCE = 32.0f;
inline constexpr float CARIBDIS_HEALER_MAX_DISTANCE = 35.0f;
// Tidal Surge's range is 10 yards.
inline constexpr float CARIBDIS_RANGED_MIN_DISTANCE = 12.0f;
// Out of sight, range means nothing: a bot within spell range behind the pillar still cannot shoot,
// so the walk goes on until she is in sight. This only stops it running into her.
inline constexpr float CARIBDIS_APPROACH_STOP_DISTANCE = 5.0f;
// A Cyclone spawns on a random player within casting range of Caribdis and catches everything
// within 4 yd of itself, so spread keeps its arrival to the one bot it was summoned on
inline constexpr float CARIBDIS_CYCLONE_SUMMON_RANGE = 45.0f;
inline constexpr float CARIBDIS_RANGED_SPREAD_DISTANCE = 4.0f;
// Karathress gains Blessing of the Tides if he hits 75% HP with any Fathom-Guard still alive, so if
// ranged fail to kill Caribdis before he gets to this percent health, melee needs to stop dps.
inline constexpr float KARATHRESS_BLESSING_HOLD_HEALTH_PCT = 85.0f;
// The widest tank AoE is Death and Decay at 10 yd.
inline constexpr float KARATHRESS_AOE_THREAT_CLEARANCE = 15.0f;
// One toss leaves a bot about 1.5 yd up; navmesh Z sits well under 1 yd off the floor
inline constexpr float CYCLONE_DROP_HEIGHT = 1.0f;
inline constexpr float SPITFIRE_TOTEM_SEARCH_DISTANCE = 75.0f;
// Ranged attack Spitfire Totems only when this close. This will exclude some ranged bots on
// Caribdis, which is the point, as ranged needs to maintain their spread due to Cyclones.
inline constexpr float SPITFIRE_TOTEM_RANGED_ATTACK_DISTANCE = 30.0f;
inline constexpr uint32 SPITFIRE_TOTEM_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 KARATHRESS_DPS_WAIT_MS = 12 * IN_MILLISECONDS;

inline Position const KARATHRESS_TANK_POSITION = { 474.403f, -531.118f,  -7.548f };
inline Position const TIDALVESS_TANK_POSITION =  { 511.282f, -501.162f, -13.158f };
inline Position const SHARKKIS_TANK_POSITION =   { 508.057f, -541.109f, -10.133f };
inline Position const CARIBDIS_TANK_POSITION =   { 464.462f, -475.820f, -13.158f };

extern std::unordered_map<uint32, uint32> karathressDpsWaitTimer;

ObjectGuid FindSpitfireTotemGuid(Player* bot);
Creature* GetSpitfireTotem(PlayerbotAI* botAI);
bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem);
Unit* GetSharkkisTankTarget(PlayerbotAI* botAI);
Unit* GetAssignedCouncilMember(PlayerbotAI* botAI);
// For a tank to move to its designated position, it must not only acquire its own target but not
// be holding any other tank's target.
bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI);
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);

// Morogrim Tidewalker

inline constexpr float TIDEWALKER_PHASE_2_HEALTH_PCT = 25.0f;
// The move to the corner starts a little early so it is done before the first Globules arrive.
inline constexpr float TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT = TIDEWALKER_PHASE_2_HEALTH_PCT + 2.0f;
// Any non-tank farther than this from him in phase 1 is brought back, such as one sent out by
// Watery Grave, rather than staying to fight murlocs where it landed. Healing a grave victim only
// takes a healer to within heal range of it, which stays inside this for every grave.
inline constexpr float TIDEWALKER_MAX_DISTANCE_FROM_BOSS = 45.0f;
inline constexpr float TIDEWALKER_RANGED_BEHIND_DISTANCE = 5.0f;
inline constexpr float TIDEWALKER_RANGED_STACK_RADIUS = 3.0f;
// Murlocs farther than this distance from Tidewalker are excluded by AppendTargetExclusions.
inline constexpr float TIDEWALKER_MURLOC_MAX_TARGET_DISTANCE = 50.0f;

inline Position const TIDEWALKER_PHASE_1_TANK_POSITION = { 410.925f, -741.916f, -7.146f };
inline Position const TIDEWALKER_PHASE_2_TANK_POSITION = { 446.571f, -767.155f, -7.144f };
// The stack point is measured not based on Tidewalker but instead is computed by drawing a line
// between the main tank and Tidewalker. This approach is cleaner during the phase transition.
Position GetTidewalkerStackPoint(Unit* tidewalker);

// Lady Vashj <Coilfang Matron>

struct GeneratorInfo
{
    ObjectGuid guid;
    float x;
    float y;
    float z;
};

inline constexpr float VASHJ_PLATFORM_CENTER_Z = 42.902f;

inline Position const VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.902f };

// The dais is a regular dodecagon on the platform center, corners every 30 degrees from due
// north. This is the distance from the center to the middle of each edge.
inline constexpr float VASHJ_DAIS_APOTHEM = 57.05f;
// The rock over the north edge of the dais, from the stair base up across the dais and back down.
inline std::array const VASHJ_NORTH_ROCK = {
    Position{ 119.256f, -910.155f, 22.314f },
    Position{  85.970f, -893.277f, 38.525f },
    Position{  73.946f, -897.039f, 41.173f },
    Position{  68.584f, -917.259f, 41.333f },
    Position{  77.624f, -925.960f, 41.165f },
    Position{ 120.362f, -931.205f, 22.520f },
};

// A pool hits anyone within 5 yd plus their own reach, about 6.5 yd for a player.
inline constexpr float TOXIC_SPORES_HIT_RADIUS = 6.5f;
// Where bots choose to stand, with room to spare past the edge of the pool.
inline constexpr float TOXIC_SPORES_AVOID_RADIUS = 7.5f;
// Her tank stands farther off, so the melee on her far side are clear too: she stops about 3.5 yd
// from the tank, and melee about 3.75 yd from her, so 7.25 plus TOXIC_SPORES_AVOID_RADIUS.
inline constexpr float TOXIC_SPORES_TANK_AVOID_RADIUS = 15.0f;
// Well past the widest avoid radius.
inline constexpr float TOXIC_SPORES_SEARCH_RADIUS = 50.0f;

extern std::unordered_map<uint32, ObjectGuid> nearestVashjGeneratorTriggerGuid;
extern std::unordered_map<ObjectGuid, Position> intendedVashjCorePasserLineup;
extern std::unordered_map<uint32, uint32> lastVashjCoreImbueAttempt;
extern std::unordered_map<ObjectGuid, uint32> lastVashjCoreInInventoryTime;

int8 GetLadyVashjPhase(Unit* vashj);
std::vector<Position> const& GetToxicSporePositions(PlayerbotAI* botAI);
// True if x/y is on the dais, at least margin inside its edge, and not under the north rock.
bool IsOnVashjDais(float x, float y, float margin);
// A step that leads away from every position given while staying on the dais. facing is optional,
// for a tank: when the bot is its victim, a step leading away from it is walked backwards. spores
// is optional too: when given, no step ends within sporeRadius of one.
bool FindVashjDaisStepAwayFromPositions(
    Player* bot, std::vector<Position> const& positions, Unit* facing, float& stepX, float& stepY,
    float& stepZ, bool& backwards, std::vector<Position> const* spores = nullptr,
    float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
// The same, away from where each unit given stands now.
bool FindVashjDaisStepAwayFromUnits(
    Player* bot, std::vector<Unit*> const& units, Unit* facing, float& stepX, float& stepY,
    float& stepZ, bool& backwards, std::vector<Position> const* spores = nullptr,
    float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
bool HasStaticCharge(Player* player);
// True for any bot but Vashj's target that holds Static Charge, or while her target holds it.
bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj);
// The one Shaman bot that keeps Grounding Totem up for the main tank: the first alive in the
// tank's subgroup. Grounding Totem Effect is a party aura, so no Shaman outside it can cover it.
Player* GetVashjGroundingShaman(Player* bot);
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
