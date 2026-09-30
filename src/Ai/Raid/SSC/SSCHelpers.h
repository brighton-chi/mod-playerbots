/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCHELPERS_H
#define PLAYERBOTS_SSCHELPERS_H

#include "Common.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Creature;
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
    SPELL_METAMORPHOSIS          = 37673,
    SPELL_CHAOS_BLAST            = 37675,
    SPELL_INSIDIOUS_WHISPER      = 37676,

    // Fathom-Lord Karathress
    SPELL_CYCLONE                = 38517, // 4 yd feather fall + knockback every 1s, 5s aura

    // Lady Vashj <Coilfang Matron>
    SPELL_FEAR_WARD              =  6346,
    SPELL_MAGIC_BARRIER          = 38112,
    SPELL_TAINTED_CORE_PARALYZE  = 38132,
    SPELL_STATIC_CHARGE          = 38280,
    SPELL_ENTANGLE               = 38316,
    SPELL_TOXIC_SPORES           = 38575,

    // Druid
    SPELL_FAERIE_FIRE_FERAL      = 16857,
    SPELL_TREE_OF_LIFE           = 33891,
    SPELL_DRUID_BERSERK          = 50334,

    // Hunter
    SPELL_MISDIRECTION_CAST      = 34477,
    SPELL_MISDIRECTION           = 35079, // the aura on the hunter

    // Paladin
    SPELL_DIVINE_SHIELD          =   642,
    SPELL_AVENGING_WRATH         = 31884,

    // Priest
    SPELL_DISPERSION             = 47585,

    // Rogue
    SPELL_CLOAK_OF_SHADOWS       = 31224,

    // Shaman
    SPELL_GROUNDING_TOTEM_EFFECT =  8178,

    // Warrior
    SPELL_VIGILANCE              = 50720,
};

enum class SscNpcs : uint32
{
    // Trash Mobs
    NPC_WATER_ELEMENTAL_TOTEM    = 22236,

    // The Lurker Below
    NPC_THE_LURKER_BELOW         = 21217,
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
    NPC_ENCHANTED_ELEMENTAL      = 21958,
    NPC_COILFANG_ELITE           = 22055,
    NPC_COILFANG_STRIDER         = 22056,
    NPC_TOXIC_SPOREBAT           = 22140,

    // Pets that PetAI keeps at spell range while they have the mana
    NPC_IMP                      =   416,
    NPC_WATER_ELEMENTAL          =   510,
    NPC_WATER_ELEMENTAL_PERM     = 37994,
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
// The bot's angle on an arc arcSpan wide around arcCenter, with the group's ranged bots in the
// instance spaced evenly along it in group order. False if there are none.
bool GetRangedArcAngle(Player* bot, float arcCenter, float arcSpan, float& angle);
// Every other living group member in the instance.
std::vector<Unit*> GetOtherLivingGroupMembers(Player* bot);

// Trash

// 25y radius + ~2y player CombatReach; see the Hyjal D&D note on persistent ground AoE range in AC.
inline constexpr float TOXIC_POOL_HAZARD_RADIUS = 27.0f;
inline constexpr float TOXIC_POOL_HOLDING_RADIUS = TOXIC_POOL_HAZARD_RADIUS + 5.0f;
inline constexpr float TOXIC_POOL_SEARCH_RADIUS = TOXIC_POOL_HOLDING_RADIUS + 2.0f;

bool GetToxicPoolPosition(PlayerbotAI* botAI, Position& toxicPool);
bool IsNearToxicPool(PlayerbotAI* botAI, float radius);

inline constexpr float WATER_ELEMENTAL_TOTEM_SEARCH_DISTANCE = 20.0f;
inline constexpr uint32 WATER_ELEMENTAL_TOTEM_CACHE_INTERVAL_MS = 1000;

ObjectGuid FindWaterElementalTotemGuid(Player* bot);
Creature* GetWaterElementalTotem(PlayerbotAI* botAI);
// True while skull is on a living Water Elemental Totem, so a second totem doesn't take it.
bool IsSkullOnWaterElementalTotem(PlayerbotAI* botAI);

// Hydross the Unstable <Duke of Currents>

// Ranged spread this far apart in frost phase, to mitigate Water Tomb.
inline constexpr float HYDROSS_FROST_RANGED_SPREAD_DISTANCE = 6.0f;

inline Position const HYDROSS_FROST_TANK_POSITION =  { -236.669f, -358.352f, -0.828f };
inline Position const HYDROSS_NATURE_TANK_POSITION = { -225.471f, -327.790f, -3.682f };

extern std::unordered_map<uint32, uint32> hydrossFrostPhaseStartTime;
extern std::unordered_map<uint32, uint32> hydrossNaturePhaseStartTime;
extern std::unordered_map<uint32, uint32> hydrossNatureMarkMaxedTime;
extern std::unordered_map<uint32, uint32> hydrossFrostMarkMaxedTime;

// Phase changes reset threat, so DPS is held on either side of one.
enum class HydrossDpsHoldWindow : uint8
{
    None,
    // From 1s after the phase's Mark hits 100% until the phase changes
    BeforePhaseChange,
    // The first 5s of a phase
    AfterPhaseChange,
};

// The main tank holds Hydross in frost phase, the first assist tank in nature phase. Every other
// tank is an add tank and picks up the Elementals that spawn upon phase changes.
bool IsHydrossFrostTank(Player* bot);
bool IsHydrossNatureTank(Player* bot);
bool IsHydrossPhaseTank(Player* bot);
bool IsHydrossAddTank(Player* bot);
bool IsHydrossInFrostPhase(Unit* hydross);
bool IsHydrossInNaturePhase(Unit* hydross);
HydrossDpsHoldWindow GetHydrossDpsHoldWindow(Unit* hydross);
bool HasMarkOfHydrossAt100Percent(Player* player);
bool HasNoMarkOfHydross(Player* bot);
bool HasMarkOfCorruptionAt100Percent(Player* player);
bool HasNoMarkOfCorruption(Player* bot);

// The Lurker Below

inline constexpr float LURKER_WHIRL_RADIUS = 25.0f;
inline constexpr float LURKER_RANGED_SAFE_DISTANCE = LURKER_WHIRL_RADIUS + 2.0f;
// Melee returning to Lurker from an islet stop this far from him, on the walkway.
inline constexpr float LURKER_WALKWAY_RADIUS = 21.0f;

// Spout avoidance mechanics:
// Each bot is assigned a radius from Lurker from 19-21y. The range is to make things look less
// artificial, and the intent is to keep the radius close to Lurker while keeping the circle on dry
// land as much as possible (water is not completely avoidable due to a couple of spillways).
// Within the 19-21y band, a bot in the 120° cone behind Lurker (60° to either side) is considered
// safe. Any bot in that safe zone will wait during the Spout windup until the spin direction is
// determined.
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MIN = 19.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MAX = 21.0f;
inline constexpr float LURKER_SPOUT_RUN_ARC_HALF_WIDTH = static_cast<float>(M_PI) / 3.0f;
inline constexpr float LURKER_SPOUT_RUN_STEP = 7.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIAL_DEADZONE = 2.0f;
// A bot may run this far past directly behind Lurker, in the spin direction, before it stops.
// This is to prevent the very intelligent bots from lapping Lurker and getting blasted.
inline constexpr float LURKER_SPOUT_RUN_OVERTAKE_MARGIN = static_cast<float>(M_PI) / 6.0f;

// Submerge: A Coilfang Guardian is assigned to each of the main tank and first two assist tanks.
// When its action first runs, each tank claims the lowest-GUID Guardian that no other tank holds,
// and claims another the same way if its own dies.
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
// The Guardian tanks in index order; empty if there are fewer than 3 living tanks, humans included.
std::vector<Player*> GetLurkerGuardianTanks(Player* bot);

// Leotheras the Blind

inline constexpr float LEOTHERAS_SEARCH_DISTANCE = 100.0f;
inline constexpr uint32 LEOTHERAS_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 LEOTHERAS_HUMANOID_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_DEMON_DPS_WAIT_MS = 10 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_FINAL_DPS_WAIT_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_WHIRLWIND_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
inline constexpr float LEOTHERAS_WHIRLWIND_SAFE_DISTANCE = 25.0f;
// Ranged keep this far from the humanoid form, outside Whirlwind's 10 yd.
inline constexpr float LEOTHERAS_RANGED_SAFE_DISTANCE = 15.0f;
// Chaos Blast deals splash damage within 8y of the target.
inline constexpr float LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE = 10.0f;
// In the final phase, Leotheras's tank keeps him this far from the Shadow's target.
inline constexpr float LEOTHERAS_SHADOW_SEPARATION_DISTANCE = 20.0f;

extern std::unordered_map<uint32, uint32> leotherasHumanoidPhaseStartTime;
// When the current Whirlwind will end, determined by the aura's remaining duration.
extern std::unordered_map<uint32, uint32> leotherasWhirlwindEndTime;
extern std::unordered_map<uint32, uint32> leotherasDemonPhaseStartTime;
extern std::unordered_map<uint32, uint32> leotherasFinalPhaseStartTime;

ObjectGuid FindLeotherasGuid(Player* bot);
ObjectGuid FindShadowOfLeotherasGuid(Player* bot);
Creature* GetLeotheras(PlayerbotAI* botAI);
bool IsSpellbinderPhase(Unit* leotheras);
Creature* GetActiveLeotherasHumanoid(PlayerbotAI* botAI);
bool IsLeotherasHumanoidPhase(PlayerbotAI* botAI);
Creature* GetLeotherasDemon(PlayerbotAI* botAI);
bool IsLeotherasDemonPhase(PlayerbotAI* botAI);
Creature* GetShadowOfLeotheras(PlayerbotAI* botAI);
bool IsLeotherasFinalPhase(PlayerbotAI* botAI);
Creature* GetLeotherasDemonOrShadow(PlayerbotAI* botAI);
Player* GetLeotherasWarlockTank(Player* bot);
bool IsLeotherasWarlockTank(Player* bot);
bool IsLeotherasChannelingWhirlwind(Unit* leotheras);
// The humanoid form, if the bot is not his target and is closer than the ranged safe distance.
Creature* GetLeotherasHumanoidToAvoid(PlayerbotAI* botAI);
// The demon's target, if it isn't the bot and the bot is within Chaos Blast's splash of it.
Unit* GetDemonTargetToAvoid(Player* bot, Unit* demon);
// The demon form's target, or else the Warlock tank, if the bot is within Chaos Blast's splash.
Unit* GetChaosBlastTargetToAvoid(PlayerbotAI* botAI);
// The Shadow's target, if the bot is Leotheras's target and within the separation distance.
Unit* GetShadowTargetToSeparateFrom(PlayerbotAI* botAI);
// Threat resets at each phase change and on every Whirlwind tick, so damage is held at the
// start of each phase and just after each Whirlwind.
bool IsLeotherasDpsHoldActive(PlayerbotAI* botAI, Unit* leotheras);
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
inline constexpr float CARIBDIS_TIDAL_SURGE_SAFE_DISTANCE = 12.0f;
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

struct KarathressCouncilAssignment
{
    char const* name;
    int8 assistTankIndex; // -1 for the main tank
};

inline constexpr std::array KARATHRESS_COUNCIL = {
    KarathressCouncilAssignment{ "fathom-lord karathress", -1 },
    KarathressCouncilAssignment{ "fathom-guard caribdis", 0 },
    KarathressCouncilAssignment{ "fathom-guard sharkkis", 1 },
    KarathressCouncilAssignment{ "fathom-guard tidalvess", 2 },
};

ObjectGuid FindSpitfireTotemGuid(Player* bot);
Creature* GetSpitfireTotem(PlayerbotAI* botAI);
bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem);
Unit* GetSharkkisTankTarget(PlayerbotAI* botAI);
// A Fathom Sporebat before a Fathom Lurker; null once none is left.
Unit* GetSharkkisPet(Player* bot);
// For a tank to move to its designated position, it must not only acquire its own target but not
// be holding any other tank's target.
bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI, Unit* ownTarget);
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
// 5 yd behind him, on the line from his victim through him (his facing if he has none).
// Following the victim rather than his facing keeps the point steady while the tank walks him
// to the corner.
Position GetTidewalkerStackPoint(Unit const& tidewalker);

// Lady Vashj <Coilfang Matron>

inline Position const VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.902f };

// The dais is a regular dodecagon on the platform center, corners every 30 degrees from due
// north. This is the distance from the center to the middle of each edge.
inline constexpr float VASHJ_DAIS_APOTHEM = 57.05f;
// The foot of the stairs, measured the same way.
inline constexpr float VASHJ_STAIR_BASE_DISTANCE = 90.19f;
// The rock over the north edge of the dais, from the stair base up across the dais and back down.
inline std::array const VASHJ_NORTH_ROCK = {
    Position{ 119.256f, -910.155f, 22.314f },
    Position{  85.970f, -893.277f, 38.525f },
    Position{  73.946f, -897.039f, 41.173f },
    Position{  68.584f, -917.259f, 41.333f },
    Position{  77.624f, -925.960f, 41.165f },
    Position{ 120.362f, -931.205f, 22.520f },
};
// Steps keep this far off the rock outline. Vashj is large and snags on it trailing her tank.
inline constexpr float VASHJ_NORTH_ROCK_CLEARANCE = 5.0f;
// Where bots other than her tank stand or walk to dodge pools. The larger clearance is for her
// path, not theirs, and in the notch west of the rock it rules out two thirds of the ring round
// her.
inline constexpr float VASHJ_STANDING_ROCK_CLEARANCE = 2.0f;
// Steps and spots keep this far inside the edge of the dais. Vashj trails her tank, so she
// stays on the dais as long as it does. The margin is only slack for the notch the rock cuts
// and for pathing near the edge.
inline constexpr float VASHJ_DAIS_MARGIN = 1.0f;
// A bot this far over the floor in phase 3, after a Sporebat or up on the pipes, is put back down.
inline constexpr float VASHJ_ABOVE_GROUND_HEIGHT = 1.5f;

// A pool hits anyone within 5 yd plus their own reach, about 6.5 yd for a player.
inline constexpr float TOXIC_SPORES_HIT_RADIUS = 6.5f;
// Where bots choose to stand, with room to spare past the edge of the pool.
inline constexpr float TOXIC_SPORES_AVOID_RADIUS = 7.5f;
// Her tank stands farther off, so the melee on her far side are clear too: she stops about 3.5 yd
// from the tank, and melee about 3.75 yd from her, so 7.25 plus TOXIC_SPORES_AVOID_RADIUS.
inline constexpr float TOXIC_SPORES_TANK_AVOID_RADIUS = 15.0f;
// Well past the widest avoid radius.
inline constexpr float TOXIC_SPORES_SEARCH_RADIUS = 50.0f;
// Melee dps within this of a pool are moved only by the melee spore action, so stock reach-melee
// can't walk them back through a pool on the way to their target.
inline constexpr float TOXIC_SPORES_MELEE_CONTROL_RADIUS = 10.0f;

// Phase 3 ranged stand out of Entangle's reach of her, and this far apart.
inline constexpr float VASHJ_PHASE_3_RANGED_DISTANCE = 15.0f;
inline constexpr float VASHJ_PHASE_3_RANGED_SPREAD_DISTANCE = 4.0f;

// Static Charge pulses reach 10y from the holder's center.
inline constexpr float VASHJ_STATIC_CHARGE_SAFE_DISTANCE = 11.0f;

// For the "ssc vashj adds" value. Only which adds exist is cached, not what is read from them
// (positions, health, victims).
inline constexpr uint32 VASHJ_ADDS_CACHE_INTERVAL_MS = 200;

// What the "ssc vashj adds" value stores, by kind.
struct VashjAddGuids
{
    GuidVector enchanted;
    GuidVector elites;
    GuidVector striders;
    GuidVector sporebats;
};

// A target, for LadyVashjAssignTargetPriorityAction.
enum class VashjTarget : uint8
{
    TaintedElemental,
    CoilfangStrider,
    CoilfangElite,
    EnchantedElemental,
    ToxicSporebat,
    LadyVashj,
};

struct VashjTargetTier
{
    VashjTarget target;
    // Enchanted Elementals only: the farthest from Vashj this tier takes one
    float maxDistanceFromVashj = std::numeric_limits<float>::max();
};

// What LadyVashjAssignTargetPriorityAction checks targets against, gathered each tick.
struct VashjTargetFacts
{
    Unit* vashj = nullptr;
    // Only for one of the cluster sent after it
    Unit* tainted = nullptr;
    int8 phase = -1;
    // From the bot, and in phase 2 from the centre too; they keep bots from going down the stairs
    float maxPursueRange = 0.0f;
    float maxSearchRange = 0.0f;
    float spellRange = 0.0f;
    // Phase 2 ranged dps hold cluster slots and shoot only what is in range of them, other than
    // the cluster sent after a Tainted Elemental
    bool holdsClusterSlot = false;
    // Phase 2: everyone but tanks leaves an Elite or Strider alone until a tank has it, so nobody
    // pulls one onto a cluster
    bool waitForTank = false;
    // Tanks: one per Elite or Strider, so the others stay free for the next ones. A new one goes
    // to the nearest free tank, which in phase 2 is the one on the side it comes from while they
    // wait in the middle.
    bool oneTankEach = false;
    // Melee dps: the living Striders, whose Panic their targets must be clear of
    std::vector<Unit*> panicStriders;
};

inline constexpr size_t VASHJ_CLUSTER_RANGED_SLOTS = 3;
inline constexpr int8 VASHJ_CLUSTER_HEALER_SLOT = 3;
// A holder within this of its slot, 2D, is there.
inline constexpr float VASHJ_CLUSTER_ARRIVAL_DISTANCE = 2.0f;

struct VashjCluster
{
    std::array<Position, VASHJ_CLUSTER_RANGED_SLOTS> ranged;
    Position healer;
};

inline std::array const VASHJ_CLUSTERS = {
    // Slots at -156, -178 and -134 degrees
    VashjCluster{
        {
            Position{ -17.87f, -944.69f, 41.30f },
            Position{ -22.33f, -925.36f, 41.30f },
            Position{  -6.49f, -960.95f, 41.30f },
        },
        Position{ -6.91f, -939.81f, 41.65f },
    },
    // Slots at 116, 102 and 130 degrees
    VashjCluster{
        {
            Position{   6.84f, -876.80f, 41.30f },
            Position{  18.82f, -872.68f, 41.30f },
            Position{  -3.79f, -883.71f, 41.30f },
        },
        Position{ 12.10f, -887.59f, 41.65f },
    },
    // Slots at -68, -80 and -56 degrees
    VashjCluster{
        {
            Position{  49.11f, -971.75f, 41.30f },
            Position{  38.66f, -974.75f, 41.30f },
            Position{  58.71f, -966.65f, 41.30f },
        },
        Position{ 44.62f, -960.63f, 41.65f },
    },
    // Slots at 43, 37 and 49 degrees
    VashjCluster{
        {
            Position{  67.66f, -888.08f, 41.30f },
            Position{  71.16f, -892.25f, 41.30f },
            Position{  63.75f, -884.30f, 41.30f },
        },
        Position{ 60.35f, -894.90f, 41.65f },
    },
};

// A bot's cluster and its slot there: 0-2 for ranged dps, VASHJ_CLUSTER_HEALER_SLOT for the healer.
struct VashjClusterSlot
{
    int8 cluster = -1;
    int8 slot = -1;
};

inline constexpr size_t VASHJ_CLUSTER_COUNT = std::tuple_size_v<decltype(VASHJ_CLUSTERS)>;
// Which clusters get a slot filled first, as indexes into VASHJ_CLUSTERS. Cluster 3 comes first:
// it takes the Tainted spawn east of the rock, the farthest from any cluster, so with too few
// ranged dps to fill every cluster it is the one to keep full.
inline constexpr std::array VASHJ_CLUSTER_FILL_ORDER = {
    int8{ 2 }, int8{ 0 }, int8{ 1 }, int8{ 3 },
};
static_assert(VASHJ_CLUSTER_FILL_ORDER.size() == VASHJ_CLUSTER_COUNT);
// Per instance, the bot holding each cluster slot: [cluster][slot].
using VashjClusterHolders =
    std::array<std::array<ObjectGuid, VASHJ_CLUSTER_RANGED_SLOTS + 1>, VASHJ_CLUSTER_COUNT>;

// Panic (38258) fears every player within this of a Strider, centre to centre: an area spell
// round an NPC caster adds neither reach.
inline constexpr float VASHJ_STRIDER_PANIC_RADIUS = 11.0f;
// Where Striders are tanked in phase 2, one in each gap between two clusters: 16y+ from every
// cluster slot and healer post (Panic fears within 11y), 18y+ from the generators and 30y+ from
// the centre. A Strider's combat reach is 9, so a 30y spell reaches it from about 40y; with the
// step-in below, all six ranged of both clusters reach it.
inline std::array const VASHJ_STRIDER_HOLD_POSITIONS = {
    Position{ -6.0f, -913.5f, 41.9f },
    Position{  9.5f, -963.5f, 41.5f },
    Position{ 33.5f, -889.5f, 41.9f },
};
// Cluster ranged within this of a tanked Strider, centre to centre, step in to cast range of it.
// They stop about 40y from it, well clear of Panic and of adds walking in.
inline constexpr float VASHJ_STRIDER_STEP_IN_DISTANCE = 50.0f;
// Where Elites are tanked in phase 2, each in range of all three ranged of one cluster (4, then 1):
// 12y+ from every cluster slot and healer post, 18y+ from every Strider hold so the melee behind
// them are clear of Panic, and 22y+ from the walk-in lines out past 30y from the centre, where an
// add is usually not yet tanked. Elites spawn 42-46y from the nearer one. The first is the nearest
// spot to Vashj meeting all that (29y from her, the second 27y), with each ranged's line to it 7y+
// from the generator centres.
inline std::array const VASHJ_ELITE_TANK_POSITIONS = {
    Position{ 57.0f, -913.0f, 42.0f },
    Position{  5.5f, -934.0f, 42.1f },
};
// Melee take Enchanted Elementals within this of Vashj before other targets, and tanks not
// holding an Elite or Strider in phase 2 take no others.
inline constexpr float VASHJ_ENCHANTED_NEAR_HER_DISTANCE = 20.0f;
// Tanks with nothing to tank in phase 2 wait within this of Vashj, to reach adds on any side.
inline constexpr float VASHJ_IDLE_TANK_DISTANCE = 10.0f;
// An Elite or Strider within this of its tank position, centre to spot, is there.
inline constexpr float VASHJ_ADD_TANK_ARRIVAL_DISTANCE = 3.0f;
// Phase 3: a tank takes its Strider this far from Vashj, where bots gather to kill elementals.
inline constexpr float VASHJ_PHASE_3_STRIDER_DISTANCE_FROM_VASHJ = 28.0f;

// Target tiers by phase and role, best first (GetVashjTargetTiers)
// Striders need several ranged on them at once
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_CLUSTER_RANGED_TIERS = {
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::CoilfangElite },
};
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_TAINTED_KILLER_TIERS = {
    VashjTargetTier{ VashjTarget::TaintedElemental },
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::CoilfangElite },
};
// Melee stay near her and the Elites: Enchanted about to reach her, then Elites
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_MELEE_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental, VASHJ_ENCHANTED_NEAR_HER_DISTANCE },
    VashjTargetTier{ VashjTarget::CoilfangElite },
};
// Tanks stay in the middle for the next Elite or Strider, wherever it comes from
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_TANK_TIERS = {
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::EnchantedElemental, VASHJ_ENCHANTED_NEAR_HER_DISTANCE },
};
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_HEALER_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::CoilfangStrider },
};
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_MAIN_TANK_TIERS = {
    VashjTargetTier{ VashjTarget::LadyVashj },
};
// Every tank but hers, one Elite or Strider each as in phase 2
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_TANK_TIERS = {
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::LadyVashj },
};
// Hunters are assigned to kill Sporebats in phase 3
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_HUNTER_TIERS = {
    VashjTargetTier{ VashjTarget::ToxicSporebat },
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::LadyVashj },
};
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_RANGED_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::LadyVashj },
};
// Melee stay on her in the dps race, but for Enchanted about to reach her and Elites
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_MELEE_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental, VASHJ_ENCHANTED_NEAR_HER_DISTANCE },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::LadyVashj },
};

// The Tainted Elemental a looter was chosen for, per instance
struct TaintedCoreLooter
{
    ObjectGuid tainted;
    ObjectGuid looter;
    // The cluster nearest the elemental, whose ranged dps kill it
    int8 cluster = -1;
};

// The four rim triggers Elites and Striders spawn at, 54-56.5y out
inline std::array const VASHJ_ADD_SPAWN_POSITIONS = {
    Position{  43.329f, -869.731f, 41.2f },
    Position{ -22.597f, -900.382f, 41.2f },
    Position{  13.781f, -975.633f, 41.2f },
    Position{  78.381f, -950.659f, 41.2f },
};
// The rock on the stairs between corners 1 and 2, base -> stairs -> base
inline std::array const VASHJ_SOUTH_WEST_ROCK = {
    Position{ -16.473f, -843.635f, 22.78f },
    Position{ -10.493f, -849.837f, 27.23f },
    Position{ -10.034f, -860.397f, 32.37f },
    Position{ -14.241f, -865.373f, 32.69f },
    Position{ -46.103f, -872.655f, 22.53f },
};
// Every Shield Generator, used or not. A used one still blocks movement and line of sight.
inline std::array const VASHJ_SHIELD_GENERATOR_POSITIONS = {
    Position{ 52.048f, -901.236f, 44.0f },
    Position{ 52.448f, -944.825f, 44.0f },
    Position{  7.810f, -945.244f, 44.0f },
    Position{  7.417f, -901.109f, 44.0f },
};
// The Shield Generators' spawn ids
inline constexpr std::array VASHJ_SHIELD_GENERATOR_SPAWN_IDS = {
    uint32{ 47482 }, // NW
    uint32{ 47483 }, // NE
    uint32{ 47484 }, // SE
    uint32{ 47485 }, // SW
};

// Within the server's INTERACTION_DISTANCE, with a margin. Edge to edge in 3D, as the server
// measures it, so the height gap needs no check of its own.
inline constexpr float VASHJ_CORE_LOOT_RANGE = INTERACTION_DISTANCE - 2.0f;
// Throw Key reaches 40y edge to edge, about 43y centre to centre. Spots are planned this far
// apart centre to centre, in 3D.
inline constexpr float VASHJ_CORE_THROW_PLAN_DISTANCE = 40.0f;
// The looter stands up to about 6y from the corpse, and the first throw is planned from the corpse.
inline constexpr float VASHJ_CORE_LOOTER_OFFSET = 6.0f;
// Line of sight is measured from each player's collision height, which the DBC puts between 1.21
// (gnome) and 2.64 (tauren female). Spots are planned from the lowest, so any race sees along
// them; with 2.0 an undead's throw from the stairs ran into the rim of the dais.
inline constexpr float VASHJ_CORE_PLAN_EYE_HEIGHT = 1.2f;
// Elites and Striders attack anyone within 20y of them.
inline constexpr float VASHJ_CORE_SPOT_SPAWN_CLEARANCE = 22.0f;
// Other catchers stand this far from every generator's centre, off its base.
inline constexpr float VASHJ_CORE_SPOT_GENERATOR_CLEARANCE = 5.0f;
// Five covers every spawn and generator pair in an offline model of the dais; four missed the
// farthest generator from two spawns.
inline constexpr size_t VASHJ_CORE_MAX_CATCHERS = 5;
// A catcher within this of its spot, 2D, is on it. The last one closer in, so it is sure to be
// within use range of the generator.
inline constexpr float VASHJ_CORE_SPOT_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float VASHJ_CORE_USE_SPOT_ARRIVAL_DISTANCE = 0.5f;

struct VashjCoreCatcher
{
    Position spot;
    ObjectGuid bot;
    // The first catcher is released when the chain is planned, the second when the first sets out,
    // each later one when the catcher before it stands on its spot. It sets out readyDelay after
    // its release, a player's reaction time.
    bool released = false;
    uint32 releaseTime = 0;
    uint32 readyDelay = 0;
    // False for a killer, who sets out only once the elemental is dead
    bool prepositions = false;
    bool arrived = false;
};

// One core's way to a generator, per instance. Planned by the mechanic tracker bot when the looter
// is picked; the holder plans it again from where it is rooted if the next throw can't be made.
struct VashjCoreChain
{
    ObjectGuid tainted;
    ObjectGuid generator;
    // Who throws to the first catcher: the looter, or the holder a new plan started from
    ObjectGuid start;
    // In throw order; the last one uses the core on the generator. The first spot's bot is picked
    // with the plan, every later one's when the spot is released.
    std::vector<VashjCoreCatcher> catchers;
    // Left out of every later plan of this chain once its throws failed
    ObjectGuid excluded;
    // The highest catcher index that has held the core; those before it are done
    int8 reached = -1;
    // No way to a generator was found; the holder destroys the core
    bool failed = false;
    uint8 replans = 0;
    // The last throw, kept through a new plan so the next still waits its turn
    ObjectGuid throwTarget;
    uint32 throwTime = 0;
    uint8 failedThrows = 0;
    // The catcher the holder is waiting on and since when, and since when that catcher has stood
    // on its spot out of reach
    ObjectGuid waitTarget;
    uint32 waitStart = 0;
    uint32 blockedStart = 0;
};

extern std::unordered_map<uint32, VashjClusterHolders> vashjClusterHolders;
extern std::unordered_map<uint32, TaintedCoreLooter> vashjTaintedCoreLooter;
extern std::unordered_map<uint32, VashjCoreChain> vashjCoreChains;
extern std::unordered_map<uint32, ObjectGuid> vashjGroundingShaman;

int8 GetLadyVashjPhase(Unit* vashj);
std::vector<Position> const& GetToxicSporePositions(PlayerbotAI* botAI);
VashjAddGuids FindVashjAddGuids(PlayerbotAI* botAI);
// True if x/y is on the dais, at least margin inside its edge, and clear of the north rock by
// rockClearance.
bool IsOnVashjDais(float x, float y, float margin, float rockClearance);
// A step that leads away from every position given while staying on the dais and rockClearance
// off the north rock: VASHJ_NORTH_ROCK_CLEARANCE for her tank, VASHJ_STANDING_ROCK_CLEARANCE for
// anyone else. facing is optional, for a tank: when the bot is its victim, a step leading away
// from it is walked backwards. spores is optional too: when given, no step ends within
// sporeRadius of one.
bool FindVashjDaisStepAwayFromPositions(
    Player* bot, std::vector<Position> const& positions, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores = nullptr, float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
// The same, away from where each unit given stands now.
bool FindVashjDaisStepAwayFromUnits(
    Player* bot, std::vector<Unit*> const& units, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores = nullptr, float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
// For her tank pinned by pools, where no single step gains on them: the spot up to 35y away,
// TOXIC_SPORES_TANK_AVOID_RADIUS or more from every pool, cheapest to reach in a straight line
// that stays on the dais, counting yards walked through a pool several times over.
bool FindVashjTankBreakoutSpot(
    Player* bot, std::vector<Position> const& spores, Position& spot);
bool HasVashjStaticCharge(Player* player);
// Melee dps who dodge pools around their target in phase 3. Tanks and Static Charge holders have
// their own movement, and while her target holds it, melee step away from it like everyone else.
bool IsVashjRingMelee(Player* bot, Unit* vashj);
// True if any pool is within radius of the bot.
bool IsNearToxicSpores(PlayerbotAI* botAI, float radius);
// True if the bot is in melee range of target, on the dais, and radius or more from every pool.
bool IsInMeleeRangeClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius);
// A Paladin under Divine Shield or a Priest under Dispersion, who may walk straight through pools
// on the way somewhere, though not stop in one.
bool CanWalkThroughToxicSpores(Player* bot);
// A step toward the nearest point, on a ring just inside the bot's melee range of target, that is
// radius or more from every pool and on the dais. False if the bot already stands clear in melee
// range, or if no point of the ring is clear.
bool GetMeleeRingStepClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius, float& stepX,
    float& stepY, float& stepZ);
// A step toward the nearest point on the dais radius from the pool nearest the bot, whatever
// other pools are there. The last way out of a pool for a boxed-in melee.
bool GetStepOutOfNearestSpore(
    Player* bot, std::vector<Position> const& spores, float radius, float& stepX, float& stepY,
    float& stepZ);
// Phase 3 ranged dps (not Hunters) and healers, not holding Static Charge: true if a pool blocks
// the reach they would make, ranged dps to cast range of their current target and healers to heal
// range of the member they need to heal. Blocked means that target is out of range and the
// straight walk to where it would be in range passes within TOXIC_SPORES_AVOID_RADIUS of a pool.
// Stock reach would walk them into the pool and the spore action straight back out, over and
// over. target and range are set to the reach's either way.
bool GetVashjReachBlockedBySpores(PlayerbotAI* botAI, Unit*& target, float& range);
// A step toward the point, 2y inside cast range of target, that is cheapest to reach in a straight
// line: the distance plus several times the yards of the line within TOXIC_SPORES_AVOID_RADIUS of
// a pool. The point is on the dais and clear of pools itself.
bool GetStepToCastRangeAroundSpores(
    Player* bot, Unit* target, float castRange, std::vector<Position> const& spores, float& stepX,
    float& stepY, float& stepZ);
// True if a phase 3 ranged bot is within VASHJ_PHASE_3_RANGED_DISTANCE of her, or within
// VASHJ_PHASE_3_RANGED_SPREAD_DISTANCE of another member.
bool IsVashjPhase3RangedTooClose(Player* bot, Unit* vashj);
// True for any bot but Vashj's target that holds Static Charge, or while her target holds it.
bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj);
// True if the bot should step away from a Static Charge pulse: a holder other than her target
// with anyone in reach of it, or anyone in reach of her charged target.
bool IsInVashjStaticChargeReach(Player* bot, Unit* vashj);
// The Entangled melee a Paladin frees, or nullptr. In phase 3, one in a pool, else one holding
// Static Charge, the main tank first each time. In phase 1, only a Static Charge holder.
Player* GetVashjHandOfFreedomTarget(PlayerbotAI* botAI, Unit* vashj);
// The one Shaman bot that keeps Grounding Totem up for the main tank while alive and in the
// instance, or nullptr. The mechanic tracker picks it with FindVashjGroundingShaman.
Player* GetVashjGroundingShaman(Player* bot);
// The first alive Shaman bot in the main tank's subgroup. Grounding Totem Effect is a party
// aura, so no Shaman outside it can cover the tank.
Player* FindVashjGroundingShaman(Player* bot);
// Each cluster's first ranged slot, then each one's second and third, then the healers, the
// clusters in VASHJ_CLUSTER_FILL_ORDER each time
std::vector<VashjClusterSlot> GetVashjClusterFillOrder();
// True if the holder is alive and in the instance.
bool IsLiveVashjClusterHolder(Player* bot, ObjectGuid guid);
// True if there is no holder table yet, or a holder is dead or gone from the instance.
bool HasVashjClusterVacancy(Player* bot);
// From the holder table; cluster -1 if the bot holds no slot.
VashjClusterSlot GetVashjClusterSlot(Player* bot);
// The cluster slot to walk back to, or nullptr: none held, already there, or away on purpose
// (the looter before the core is looted, a killer, a ranged stepped in to a Strider).
Position const* GetVashjClusterPositionToReturnTo(Player* bot, Unit* currentTarget);
// Skips a cluster whose straight line to the unit crosses the north rock.
int8 GetNearestVashjCluster(Unit* unit);
// The cluster's healer; else the spare healer (no cluster slot) nearest the elemental, who rarely
// gets there in time, so that core is given up; else the cluster's ranged dps nearest it.
Player* FindTaintedCoreLooter(Player* bot, Unit* tainted, int8 cluster);
// The Tainted Elemental the current looter was chosen for, alive or a corpse.
Creature* GetAssignedTaintedElemental(Player* bot);
// The core's slot in the elemental's loot; -1 while it is alive (loot is filled on death) and once
// the core is taken. The corpse stays flagged lootable until its looter releases the loot.
int8 GetTaintedCoreLootSlot(Creature* tainted);
// From the elemental's spawn until its core is taken from the corpse.
bool IsTaintedCoreStillToLoot(Creature* tainted);
// The living Tainted Elemental the bot is assigned to attack, else nullptr: the ranged dps of
// the cluster nearest it, its looter included.
Creature* GetTaintedElementalToKill(Player* bot);
// The looter the mechanic tracker bot chose for the current Tainted Elemental, via
// LadyVashjAssignTaintedCoreLooterAction.
bool IsDesignatedCoreLooter(Player* bot);
// The master's current target, unless a pet would be useless on it; then the Enchanted Elemental
// nearest Vashj, or Vashj herself in phase 3. Nullptr when there is nothing worth attacking.
Unit* GetVashjPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* vashj);
// True if a tank is the unit's victim.
bool IsVashjAddHeldByTank(Unit* unit);
// The bot's class taunt on target. False if it has none, or can't cast it now.
bool CastTankTaunt(PlayerbotAI* botAI, Unit* target);
// The tank an add belongs to, so each has only one: of the living tanks attacking it, the one it
// is attacking, else the first in group order. Nullptr if no tank is attacking it.
Player* GetVashjAddOwningTank(Player* bot, Unit* add);
// True if no other living bot tank is nearer the add among those free to take it: not holding
// an Elite or Strider of their own, and in phase 3 not Vashj's tank. In phase 3 a tank holding
// only an Elite is free for a Strider.
bool IsNearestFreeVashjTank(Player* bot, Unit* add, Unit* vashj, int8 phase);
// The bot's target tiers for the phase, best first. killsTainted: one of the cluster sent after a
// Tainted Elemental.
std::vector<VashjTargetTier> const& GetVashjTargetTiers(Player* bot, int8 phase, bool killsTainted);
// A step for a tank that brings the mob it is tanking onto spot. The mob trails its tank by about
// its combat reach, so the tank walks on past the spot until the mob itself stands on it. False
// once the mob is within arrivalDistance of the spot.
bool GetStepToBringTankedUnitTo(
    Player* bot, Unit* mob, Position const& spot, float arrivalDistance, float& stepX,
    float& stepY, bool& backwards);
// The Strider hold and Elite tank position nearest the add.
Position const& GetVashjStriderHoldPosition(Unit const& strider);
Position const& GetVashjEliteTankPosition(Unit const& elite);
// True if strider, the tank's current target, is a Strider it has work on: in phase 2, one on
// it not yet at its hold; in phase 3, one to taunt off whoever has it, or one on it that is too
// close to Vashj. Never for her tank in phase 3, who is on her.
bool ShouldTankVashjStrider(Player* bot, Unit* strider, Unit* vashj, int8 phase);
// True for a tanked Strider within VASHJ_STRIDER_STEP_IN_DISTANCE of the bot.
bool IsTankedStriderInStepInReach(Player* bot, Unit* unit);
// TEMP LOG (Tainted Elemental timing), remove after testing. Elapsed is from the looter pick.
void StartTaintedLog(Player* bot);
uint32 TaintedLogElapsedMs(Player* bot);
// Once per bot, key and elemental.
bool TaintedLogFirstTime(Player* bot, char const* key);
bool TaintedLogSeen(Player* bot, char const* key);
// At most once a second per bot and key.
bool TaintedLogThrottle(Player* bot, char const* key);
void TaintedLogThrow(Player* bot, Player* receiver, int catcher);
void TaintedLogChain(Player* bot, VashjCoreChain const& chain, char const* what);
// Logs when the number of usable generators changes.
void TaintedLogGenerators(Player* bot);
// By the core's Paralyze, which comes and goes with the core in the bags. Nothing else takes it
// off: no dispel type or mechanic, it pierces immunities, and it can't be cancelled.
bool HasTaintedCore(Player* player);
// A new chain for the elemental, from where it stands to the nearest usable generator a route
// reaches, with looter as its start. With no route yet, the looter plans again once it holds the
// core.
void PlanVashjCoreChain(Player* bot, Unit* tainted, Player* looter);
// Plans the chain again from the holder, rooted where it stands, to the same generator if a route
// still reaches it, else the nearest usable one that does. excluded, when given, gets no spot in
// this plan or any later one. Marks the chain failed if there is no way, or after a few tries.
bool ReplanVashjCoreChain(Player* holder, VashjCoreChain& chain, ObjectGuid excluded);
// Gives a catcher's spot to the nearest other bot that can take it.
bool ReassignVashjCoreCatcher(Player* bot, VashjCoreChain& chain, size_t index);
// Picks the nearest bot for a catcher's spot, from where the raid stands now, and lets it set out.
void ReleaseVashjCoreCatcher(Player* bot, VashjCoreChain& chain, size_t index);
VashjCoreChain* GetVashjCoreChain(Player* bot);
// The bot's place among the chain's catchers, or -1.
int8 GetVashjCoreCatcherIndex(VashjCoreChain const& chain, Player* bot);
// True while the catcher should be walking to or standing on its spot.
bool IsVashjCoreCatcherActive(Player* bot, VashjCoreChain const& chain, int8 index);
float GetVashjCoreSpotArrivalDistance(VashjCoreChain const& chain, int8 index);

}

#endif
