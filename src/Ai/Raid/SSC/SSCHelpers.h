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
    // Trash
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
    SPELL_LEOTHERAS_WHIRLWIND    = 37640,
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
    // Trash
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
inline constexpr float PATH_STEP_DISTANCE = 3.5f;
inline constexpr float PATH_BACKWARD_STEP_DISTANCE = 2.25f;

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);
bool CastTankTaunt(PlayerbotAI* botAI, Unit* target);
bool FindHazardEscapeStep(
    Player* bot, Position const& hazard, float moveDist, float& stepX, float& stepY, float& stepZ);
bool IsDryGround(Player* bot, float x, float y);
// Follows the path corner by corner, so a bot can step around a pillar in its way.
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY);
// The group's ranged bots in the instance are spaced evenly along the arc, in group order.
bool GetRangedArcAngle(Player* bot, float arcCenter, float arcSpan, float& angle);
std::vector<Unit*> GetOtherLivingGroupMembers(Player* bot);

// Trash

// 25y radius + ~2y player CombatReach
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

// Phase changes reset threat, so DPS is held on either side of one.
enum class HydrossDpsHoldWindow : uint8
{
    None,
    // From 1s after the phase's Mark hits 100% until the phase changes
    BeforePhaseChange,
    // The first 5s of a phase
    AfterPhaseChange,
};

// Ranged spread this far apart in frost phase, to mitigate Water Tomb.
inline constexpr float HYDROSS_FROST_RANGED_SPREAD_DISTANCE = 5.0f;
// Cleansing Field (37935) is a 20 yd area aura that adds both combat reaches (3 for the helper,
// 5 for Hydross), so he flips crossing 28 yd from the helper, centre to centre.
inline constexpr float HYDROSS_CLEANSING_FIELD_RADIUS = 28.0f;
// The incoming tank waits this far short of the field's edge, on its own side.
inline constexpr float HYDROSS_HANDOFF_SHORT_DISTANCE = 8.0f;

inline Position const HYDROSS_FROST_TANK_POSITION =  { -235.653f, -354.823f, -0.828f };
inline Position const HYDROSS_NATURE_TANK_POSITION = { -224.721f, -324.755f, -3.682f };
inline Position const HYDROSS_CLEANSING_FIELD_CENTER = { -239.715f, -366.440f, -0.745f };

extern std::unordered_map<uint32, uint32> hydrossFrostPhaseStartTime;
extern std::unordered_map<uint32, uint32> hydrossNaturePhaseStartTime;
extern std::unordered_map<uint32, uint32> hydrossNatureMarkMaxedTime;
extern std::unordered_map<uint32, uint32> hydrossFrostMarkMaxedTime;

// The main tank holds Hydross in frost phase, the first assist tank in nature phase. Every other
// tank is an add tank and picks up the Elementals that spawn upon phase changes.
bool IsHydrossFrostTank(Player* bot);
bool IsHydrossNatureTank(Player* bot);
bool IsHydrossPhaseTank(Player* bot);
bool IsHydrossAddTank(Player* bot);
bool IsHydrossInFrostPhase(Unit* hydross);
bool IsHydrossInNaturePhase(Unit* hydross);
HydrossDpsHoldWindow GetHydrossDpsHoldWindow(Unit* hydross);
Position GetHydrossHandoffPosition(bool frostTank);
bool HasMarkOfHydrossAt100Percent(Player* player);
bool HasNoMarkOfHydross(Player* bot);
bool HasMarkOfCorruptionAt100Percent(Player* player);
bool HasNoMarkOfCorruption(Player* bot);

// The Lurker Below

inline constexpr float LURKER_WHIRL_RADIUS = 25.0f;
inline constexpr float LURKER_RANGED_SAFE_DISTANCE = LURKER_WHIRL_RADIUS + 2.0f;
// Melee returning to Lurker from an islet stop this far from him, on the walkway.
inline constexpr float LURKER_WALKWAY_RADIUS = 21.0f;
// A melee bot farther than this from Lurker is out on an islet. The Ambushers' islets are
// 45-55 yd from him, and the Guardians spawn on land 25-30 yd from him.
inline constexpr float LURKER_ISLET_DISTANCE = 40.0f;

// Spout: each bot runs on its own radius 20-22y from Lurker (main tank 20y), close to him but
// mostly on dry land, and spread so it looks less artificial. Bots in the 120° cone behind him
// are safe and wait out the wind-up until the spin direction is known.
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MIN = 20.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MAX = 22.0f;
inline constexpr float LURKER_SPOUT_RUN_ARC_HALF_WIDTH = static_cast<float>(M_PI) / 3.0f;
inline constexpr float LURKER_SPOUT_RUN_STEP = 3.5f;
inline constexpr float LURKER_SPOUT_RUN_RADIAL_DEADZONE = 2.0f;
// A bot may run this far past directly behind Lurker, in the spin direction, before it stops.
// This is to prevent the very intelligent bots from lapping Lurker and getting blasted.
inline constexpr float LURKER_SPOUT_RUN_OVERTAKE_MARGIN = static_cast<float>(M_PI) / 6.0f;

// Submerge: the main tank and first two assist tanks each claim the lowest-GUID Guardian no
// other tank holds, and claim another the same way if theirs dies.
inline constexpr size_t LURKER_GUARDIAN_TANK_COUNT = 3;
inline constexpr uint32 LURKER_GUARDIAN_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 LURKER_GUARDIAN_TANK_CACHE_INTERVAL_MS = 1000;
inline constexpr float LURKER_GUARDIAN_SEARCH_RADIUS = 100.0f;

inline Position const LURKER_MAIN_TANK_POSITION = { 23.706f, -406.038f, -19.686f };

extern std::unordered_map<uint32, std::array<ObjectGuid, LURKER_GUARDIAN_TANK_COUNT>>
    lurkerGuardianTankAssignments;

// REACT_PASSIVE covers the whole Spout: a 3s wind-up (37431), then a 16s spin aura at
// 0.1 rad/250ms, 37429 counterclockwise or 37430 clockwise.
bool IsLurkerSpouting(Unit* lurker);
bool IsLurkerSurfacedAndCalm(Unit* lurker);
// +1 counter-clockwise, -1 clockwise, 0 during the wind-up.
int8 GetLurkerSpoutSpin(Unit* lurker);
bool DoesPathRoundLurker(Player* bot, Unit* lurker, float x, float y, float z, int8 direction);
float GetArrivingPathLength(Player* bot, float x, float y, float z, float tolerance);
GuidVector FindLurkerGuardianGuids(Player* bot);
std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI);
GuidVector FindLurkerGuardianTankGuids(Player* bot);
int8 GetLurkerGuardianTankIndex(PlayerbotAI* botAI);
// Lurker, or a Guardian seen from an islet, where a path onto it crosses the deep water.
bool ShouldGoToLurkerWalkway(Player* bot, Unit* lurker, Unit* target);

// Leotheras the Blind

inline constexpr float LEOTHERAS_SEARCH_DISTANCE = 100.0f;
inline constexpr uint32 LEOTHERAS_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 LEOTHERAS_HUMANOID_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_DEMON_DPS_WAIT_MS = 10 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_FINAL_DPS_WAIT_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_WHIRLWIND_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
inline constexpr float LEOTHERAS_WHIRLWIND_SAFE_DISTANCE = 25.0f;
// Ranged keep this far from the humanoid form, outside Whirlwind's 10 yd.
inline constexpr float LEOTHERAS_RANGED_SAFE_DISTANCE = 20.0f;
inline constexpr float LEOTHERAS_RANGED_SPREAD_DISTANCE = 4.0f;
// Chaos Blast deals splash damage within 8y of the target.
inline constexpr float LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE = 10.0f;
// In the final phase, Leotheras's tank keeps him this far from the Shadow's target.
inline constexpr float LEOTHERAS_SHADOW_SEPARATION_DISTANCE = 20.0f;

extern std::unordered_map<uint32, uint32> leotherasHumanoidPhaseStartTime;
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
// (1) First priority is an assistant Warlock (real player or bot).
// (2) If no assistant Warlock, then look for any Warlock bot.
Player* GetLeotherasWarlockTank(Player* bot);
bool IsLeotherasWarlockTank(Player* bot);
bool IsLeotherasChannelingWhirlwind(Unit* leotheras);
Creature* GetLeotherasHumanoidToAvoid(PlayerbotAI* botAI);
Unit* GetDemonTargetToAvoid(Player* bot, Unit* demon);
Unit* GetChaosBlastTargetToAvoid(PlayerbotAI* botAI);
Unit* GetShadowTargetToSeparateFrom(PlayerbotAI* botAI);
// Threat resets at each phase change and on every Whirlwind tick, so damage is held at the
// start of each phase and just after each Whirlwind.
bool IsLeotherasDpsHoldActive(PlayerbotAI* botAI, Unit* leotheras);
bool HasTooManyChaosBlastStacks(Player* bot);
bool HasInnerDemon(Player* bot);
Creature* GetPersonalInnerDemon(PlayerbotAI* botAI);

// Fathom-Lord Karathress

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

// Karathress gains Blessing of the Tides if he hits 75% HP with any Fathom-Guard still alive, so if
// ranged fail to kill Caribdis before he gets to this percent health, melee needs to stop dps.
inline constexpr float KARATHRESS_BLESSING_HOLD_HEALTH_PCT = 85.0f;
// The widest tank AoE is Death and Decay at 10 yd.
inline constexpr float KARATHRESS_AOE_THREAT_CLEARANCE = 15.0f;
inline constexpr uint32 KARATHRESS_DPS_WAIT_MS = 12 * IN_MILLISECONDS;

inline constexpr float SPITFIRE_TOTEM_SEARCH_DISTANCE = 75.0f;
// Ranged attack Spitfire Totems only when this close. This will exclude some ranged bots on
// Caribdis, which is the point, as ranged needs to maintain their spread due to Cyclones.
inline constexpr float SPITFIRE_TOTEM_RANGED_ATTACK_DISTANCE = 30.0f;
inline constexpr uint32 SPITFIRE_TOTEM_CACHE_INTERVAL_MS = 200;

// The healer anchors on Caribdis, not her victim, which jumps into the room whenever her tank
// loses her. The tank stands on her, so 32 yd from her is about 35 yd from the tank.
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
// One toss leaves a bot about 1.5 yd up; navmesh Z sits well under 1 yd off the floor
inline constexpr float CARIBDIS_CYCLONE_DROP_HEIGHT = 1.0f;

inline Position const KARATHRESS_TANK_POSITION = { 474.403f, -531.118f,  -7.548f };
inline Position const CARIBDIS_TANK_POSITION =   { 464.462f, -475.820f, -13.158f };
inline Position const SHARKKIS_TANK_POSITION =   { 508.057f, -541.109f, -10.133f };
inline Position const TIDALVESS_TANK_POSITION =  { 521.833f, -503.329f, -13.158f };

extern std::unordered_map<uint32, uint32> karathressDpsWaitTimer;

ObjectGuid FindSpitfireTotemGuid(Player* bot);
Creature* GetSpitfireTotem(PlayerbotAI* botAI);
bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem);
// Sharkkis's tank holds his pets too. Sharkkis on somebody else comes first, then a pet on
// somebody else, then Sharkkis, then a pet that is already on the tank; null once none is left.
Unit* GetSharkkisTankTarget(PlayerbotAI* botAI);
// A Fathom Sporebat before a Fathom Lurker; null once none is left.
Unit* GetSharkkisPet(Player* bot);
bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI, Unit* ownTarget);
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);

// Morogrim Tidewalker

inline constexpr float TIDEWALKER_PHASE_2_HEALTH_PCT = 25.0f;
// The move to the corner starts a little early so it is done before the first Globules arrive.
inline constexpr float TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT = TIDEWALKER_PHASE_2_HEALTH_PCT + 2.0f;
// Non-tanks farther than this in phase 1, such as one sent out by Watery Grave, are brought
// back. Every grave is inside it, so healing a grave victim never crosses it.
inline constexpr float TIDEWALKER_MAX_DISTANCE_FROM_BOSS = 45.0f;
inline constexpr float TIDEWALKER_RANGED_BEHIND_DISTANCE = 5.0f;
// Hunters can't shoot inside his melee range (about 8.8 yd centre to centre), so they stack
// far enough back that the near edge of their stack stays clear of it.
inline constexpr float TIDEWALKER_HUNTER_BEHIND_DISTANCE = 13.0f;
inline constexpr float TIDEWALKER_RANGED_STACK_RADIUS = 3.0f;
inline constexpr float TIDEWALKER_MURLOC_MAX_TARGET_DISTANCE = 50.0f;

inline Position const TIDEWALKER_PHASE_1_TANK_POSITION = { 410.925f, -741.916f, -7.146f };
inline Position const TIDEWALKER_PHASE_2_TANK_POSITION = { 446.571f, -767.155f, -7.144f };
// Behind him on the line from his victim through him, not his facing, so the point holds steady
// while the tank walks him to the corner.
Position GetTidewalkerStackPoint(Player const& bot, Unit const& tidewalker);

// Lady Vashj <Coilfang Matron>

// Vashj: General

// The dais is a regular dodecagon on the platform center, corners every 30 degrees from due
// north. This is the distance from the center to the middle of each edge.
inline constexpr float VASHJ_DAIS_APOTHEM = 57.05f;
// The foot of the stairs, measured the same way.
inline constexpr float VASHJ_STAIR_BASE_DISTANCE = 90.19f;
// Steps keep this far off the rock outline. Vashj is large and snags on it trailing her tank.
inline constexpr float VASHJ_NORTH_ROCK_CLEARANCE = 5.0f;
// For bots other than her tank. The tank's larger clearance would rule out two thirds of the
// ring round her in the notch west of the rock.
inline constexpr float VASHJ_STANDING_ROCK_CLEARANCE = 2.0f;
// Vashj trails her tank, so she stays on the dais as long as it does. The margin is only slack
// for the notch the rock cuts and for pathing near the edge.
inline constexpr float VASHJ_DAIS_MARGIN = 1.0f;
// A bot this far over the floor in phase 3, after a Sporebat or up on the pipes, is put back down.
inline constexpr float VASHJ_ABOVE_GROUND_HEIGHT = 1.5f;

inline Position const VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.902f };
// The rock over the north edge of the dais, from the stair base up across the dais and back down.
inline std::array const VASHJ_NORTH_ROCK = {
    Position{ 119.256f, -910.155f, 22.314f },
    Position{  85.970f, -893.277f, 38.525f },
    Position{  73.946f, -897.039f, 41.173f },
    Position{  68.584f, -917.259f, 41.333f },
    Position{  77.624f, -925.960f, 41.165f },
    Position{ 120.362f, -931.205f, 22.520f },
};

int8 GetLadyVashjPhase(Unit* vashj);
bool IsOnVashjDais(float x, float y, float margin, float rockClearance);

// Vashj: Static Charge, Entangle and Shock Blast

// Static Charge pulses reach 10y from the holder's center.
inline constexpr float VASHJ_STATIC_CHARGE_SAFE_DISTANCE = 11.0f;
// Phase 3 ranged stand out of Entangle's reach of her, and this far apart.
inline constexpr float VASHJ_PHASE_3_RANGED_DISTANCE = 15.0f;
inline constexpr float VASHJ_PHASE_3_RANGED_SPREAD_DISTANCE = 4.0f;

extern std::unordered_map<uint32, ObjectGuid> vashjGroundingShaman;

bool HasVashjStaticCharge(Player* player);
bool IsVashjPhase3RangedTooClose(Player* bot, Unit* vashj);
bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj);
bool IsInVashjStaticChargeReach(Player* bot, Unit* vashj);
// The Entangled melee a Paladin frees, or nullptr. In phase 3, one in a pool, else one holding
// Static Charge, the main tank first each time. In phase 1, only a Static Charge holder.
Player* GetVashjHandOfFreedomTarget(PlayerbotAI* botAI, Unit* vashj);
Player* GetVashjGroundingShaman(Player* bot);
// Grounding Totem Effect is a party aura, so only a Shaman in the main tank's subgroup covers it.
Player* FindVashjGroundingShaman(Player* bot);

// Vashj: Toxic Spores

// A pool hits anyone within 5 yd plus their own reach, about 6.5 yd for a player.
inline constexpr float TOXIC_SPORES_HIT_RADIUS = 6.5f;
inline constexpr float TOXIC_SPORES_AVOID_RADIUS = 7.5f;
// Her tank stands farther off, so the melee on her far side are clear too: she stops about 3.5 yd
// from the tank, and melee about 3.75 yd from her, so 7.25 plus TOXIC_SPORES_AVOID_RADIUS.
inline constexpr float TOXIC_SPORES_TANK_AVOID_RADIUS = 15.0f;
inline constexpr float TOXIC_SPORES_SEARCH_RADIUS = 50.0f;
// Melee dps within this of a pool are moved only by the melee spore action, so stock reach-melee
// can't walk them back through a pool on the way to their target.
inline constexpr float TOXIC_SPORES_MELEE_CONTROL_RADIUS = 10.0f;

std::vector<Position> const& GetToxicSporePositions(PlayerbotAI* botAI);
bool FindVashjDaisStepAwayFromPositions(
    Player* bot, std::vector<Position> const& positions, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores = nullptr, float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
bool FindVashjDaisStepAwayFromUnits(
    Player* bot, std::vector<Unit*> const& units, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores = nullptr, float sporeRadius = TOXIC_SPORES_AVOID_RADIUS);
// For her tank pinned by pools, where no single step gains on them.
bool FindVashjTankBreakoutSpot(
    Player* bot, std::vector<Position> const& spores, Position& spot);
bool IsVashjRingMelee(Player* bot, Unit* vashj);
bool IsNearToxicSpores(PlayerbotAI* botAI, float radius);
bool IsInMeleeRangeClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius);
// Divine Shield or Dispersion: may walk straight through pools, though not stop in one.
bool CanWalkThroughToxicSpores(Player* bot);
// Like Azgalor's Rain of Fire maneuver, but sampled every 5 degrees so each point can be checked
// against the dais and the rock too.
bool GetMeleeRingStepClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius, float& stepX,
    float& stepY, float& stepZ);
// The last way out of a pool for a boxed-in melee.
bool GetStepOutOfNearestSpore(
    Player* bot, std::vector<Position> const& spores, float radius, float& stepX, float& stepY,
    float& stepZ);
// Stock reach would walk them into a pool and the spore action straight back out, over and
// over, so a pool on the straight walk to reach range blocks it.
bool GetVashjReachBlockedBySpores(PlayerbotAI* botAI, Unit*& target, float& range);
bool GetStepToCastRangeAroundSpores(
    Player* bot, Unit* target, float castRange, std::vector<Position> const& spores, float& stepX,
    float& stepY, float& stepZ);

// Vashj: Phase 2 Ranged Clusters

inline constexpr size_t VASHJ_CLUSTER_RANGED_SLOTS = 3;
inline constexpr int8 VASHJ_CLUSTER_HEALER_SLOT = 3;
inline constexpr float VASHJ_CLUSTER_ARRIVAL_DISTANCE = 2.0f;

struct VashjCluster
{
    std::array<Position, VASHJ_CLUSTER_RANGED_SLOTS> ranged;
    Position healer;
};

// A bot's cluster and its slot there: 0-2 for ranged dps, VASHJ_CLUSTER_HEALER_SLOT for the healer.
struct VashjClusterSlot
{
    int8 cluster = -1;
    int8 slot = -1;
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

inline constexpr size_t VASHJ_CLUSTER_COUNT = std::tuple_size_v<decltype(VASHJ_CLUSTERS)>;
// The cluster that takes the Tainted spawn east of the rock, the farthest from any cluster,
// fills first, so with too few ranged dps it is the one kept full.
inline constexpr std::array VASHJ_CLUSTER_FILL_ORDER = {
    int8{ 2 }, int8{ 0 }, int8{ 1 }, int8{ 3 },
};
static_assert(VASHJ_CLUSTER_FILL_ORDER.size() == VASHJ_CLUSTER_COUNT);
// Per instance, the bot holding each cluster slot: [cluster][slot].
using VashjClusterHolders =
    std::array<std::array<ObjectGuid, VASHJ_CLUSTER_RANGED_SLOTS + 1>, VASHJ_CLUSTER_COUNT>;

extern std::unordered_map<uint32, VashjClusterHolders> vashjClusterHolders;

std::vector<VashjClusterSlot> GetVashjClusterFillOrder();
bool IsLiveVashjClusterHolder(Player* bot, ObjectGuid guid);
bool HasVashjClusterVacancy(Player* bot);
VashjClusterSlot GetVashjClusterSlot(Player* bot);
Position const* GetVashjClusterPositionToReturnTo(Player* bot, Unit* currentTarget);
// From the Tainted spawn just east of the rock, the nearest cluster would have to walk round it,
// so the next nearest takes it. No other spawn changes.
int8 GetNearestVashjCluster(Unit* unit);

// Vashj: Adds and Target Priority

struct VashjAddGuids
{
    GuidVector enchanted;
    GuidVector elites;
    GuidVector striders;
    GuidVector sporebats;
};

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
    // to the nearest free tank.
    bool oneTankEach = false;
    // Melee dps: the living Striders, whose Panic their targets must be clear of
    std::vector<Unit*> panicStriders;
};

// Only which adds exist is cached, not what is read from them (positions, health, victims).
inline constexpr uint32 VASHJ_ADDS_CACHE_INTERVAL_MS = 200;
// Panic (38258) fears every player within this of a Strider, centre to centre: an area spell
// round an NPC caster adds neither reach.
inline constexpr float VASHJ_STRIDER_PANIC_RADIUS = 11.0f;
// Cluster ranged within this of a tanked Strider, centre to centre, step in to cast range of it.
// They stop about 40y from it, well clear of Panic and of adds walking in.
inline constexpr float VASHJ_STRIDER_STEP_IN_DISTANCE = 50.0f;
// Melee take Enchanted Elementals within this of Vashj before other targets, and tanks not
// holding an Elite or Strider in phase 2 take no others.
inline constexpr float VASHJ_ENCHANTED_NEAR_HER_DISTANCE = 20.0f;
// Tanks with nothing to tank in phase 2 wait within this of Vashj, to reach adds on any side.
inline constexpr float VASHJ_IDLE_TANK_DISTANCE = 10.0f;
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

// One in each gap between two clusters: 16y+ from every cluster slot and healer post (Panic
// fears within 11y), 18y+ from the generators, and in reach of both clusters' ranged.
inline std::array const VASHJ_STRIDER_HOLD_POSITIONS = {
    Position{ -6.0f, -913.5f, 41.9f },
    Position{  9.5f, -963.5f, 41.5f },
    Position{ 33.5f, -889.5f, 41.9f },
};
// Each in range of all three ranged of one cluster, and 18y+ from every Strider hold so the
// melee behind them are clear of Panic.
inline std::array const VASHJ_ELITE_TANK_POSITIONS = {
    Position{ 57.0f, -913.0f, 42.0f },
    Position{  5.5f, -934.0f, 42.1f },
};

VashjAddGuids FindVashjAddGuids(PlayerbotAI* botAI);
std::vector<VashjTargetTier> const& GetVashjTargetTiers(Player* bot, int8 phase, bool killsTainted);
bool IsVashjAddHeldByTank(Unit* unit);
Player* GetVashjAddOwningTank(Player* bot, Unit* add);
bool IsNearestFreeVashjTank(Player* bot, Unit* add, Unit* vashj, int8 phase);
// The mob trails its tank by about its combat reach, so the tank walks on past the spot until
// the mob itself stands on it.
bool GetStepToBringTankedUnitTo(
    Player* bot, Unit* mob, Position const& spot, float arrivalDistance, float& stepX,
    float& stepY, bool& backwards);
Position const& GetVashjStriderHoldPosition(Unit const& strider);
Position const& GetVashjEliteTankPosition(Unit const& elite);
bool ShouldTankVashjStrider(Player* bot, Unit* strider, Unit* vashj, int8 phase);
bool IsTankedStriderInStepInReach(Player* bot, Unit* unit);
// Useless: Vashj while immune; a Strider, whose Panic fears any pet that closes to melee (the Imp
// and Water Elemental cast from range); and a Sporebat, which a pet can't reach.
Unit* GetVashjPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* vashj);

// Vashj: Tainted Elemental

struct TaintedCoreLooter
{
    ObjectGuid tainted;
    ObjectGuid looter;
    // The cluster nearest the elemental, whose ranged dps kill it
    int8 cluster = -1;
};

// Within the server's INTERACTION_DISTANCE, with a margin. Edge to edge in 3D, as the server
// measures it, so the height gap needs no check of its own.
inline constexpr float VASHJ_CORE_LOOT_RANGE = INTERACTION_DISTANCE - 2.0f;

extern std::unordered_map<uint32, TaintedCoreLooter> vashjTaintedCoreLooter;

Player* FindTaintedCoreLooter(Player* bot, Unit* tainted, int8 cluster);
Creature* GetAssignedTaintedElemental(Player* bot);
// The core's slot in the elemental's loot; -1 while it is alive (loot is filled on death) and once
// the core is taken. The corpse stays flagged lootable until its looter releases the loot.
int8 GetTaintedCoreLootSlot(Creature* tainted);
bool IsTaintedCoreStillToLoot(Creature* tainted);
Creature* GetTaintedElementalToKill(Player* bot);
bool IsDesignatedCoreLooter(Player* bot);
// By the core's Paralyze, which comes and goes with the core in the bags. Nothing else takes it
// off: no dispel type or mechanic, it pierces immunities, and it can't be cancelled.
bool HasTaintedCore(Player* player);

// Vashj: Core Passing Chain

struct VashjCoreCatcher
{
    Position spot;
    ObjectGuid bot;
    // The first is released with the plan, the second when the first sets out, each later one
    // when the one before is on its spot. It sets out readyDelay later, a player's reaction time.
    bool released = false;
    uint32 releaseTime = 0;
    uint32 readyDelay = 0;
    // False for a killer, who sets out only once the elemental is dead
    bool prepositions = false;
    bool arrived = false;
};

// One core's way to a generator, per instance. Planned by the mechanic tracker bot when the looter
// is picked; the holder plans it again from where it is rooted if the next throw can't be made.
struct VashjCorePassingChain
{
    ObjectGuid tainted;
    ObjectGuid generator;
    // Who throws to the first catcher: the looter, or the holder a new plan started from
    ObjectGuid originBot;
    // In throw order; the last one uses the core on the generator
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

// Throw Key reaches 40y edge to edge, about 43y centre to centre. Spots are planned this far
// apart centre to centre, in 3D.
inline constexpr float VASHJ_CORE_THROW_PLAN_DISTANCE = 40.0f;
// The looter stands up to about 6y from the corpse, and the first throw is planned from the corpse.
inline constexpr float VASHJ_CORE_LOOTER_OFFSET = 6.0f;
// Line of sight is from each player's collision height, 1.21 (gnome) to 2.64 (tauren female).
// Planned from the lowest; with 2.0 an undead's throw from the stairs hit the rim of the dais.
inline constexpr float VASHJ_CORE_PLAN_EYE_HEIGHT = 1.2f;
// Elites and Striders attack anyone within 20y of them.
inline constexpr float VASHJ_CORE_SPOT_SPAWN_CLEARANCE = 22.0f;
// Other catchers stand this far from every generator's centre, off its base.
inline constexpr float VASHJ_CORE_SPOT_GENERATOR_CLEARANCE = 5.0f;
// Five covers every spawn and generator pair; four missed the farthest generator from two spawns.
inline constexpr size_t VASHJ_CORE_MAX_CATCHERS = 5;
// The last catcher stands closer in, to be sure it's in use range of the generator.
inline constexpr float VASHJ_CORE_SPOT_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float VASHJ_CORE_USE_SPOT_ARRIVAL_DISTANCE = 0.5f;
inline constexpr std::array VASHJ_SHIELD_GENERATOR_SPAWN_IDS = {
    uint32{ 47482 }, // NW
    uint32{ 47483 }, // NE
    uint32{ 47484 }, // SE
    uint32{ 47485 }, // SW
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

extern std::unordered_map<uint32, VashjCorePassingChain> vashjCorePassingChains;

void PlanVashjCorePassingChain(Player* bot, Unit* tainted, Player* looter);
bool ReplanVashjCorePassingChain(Player* holder, VashjCorePassingChain& chain, ObjectGuid excluded);
bool ReassignVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index);
void ReleaseVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index);
VashjCorePassingChain* GetVashjCorePassingChain(Player* bot);
int8 GetVashjCoreCatcherIndex(VashjCorePassingChain const& chain, Player* bot);
bool IsVashjCoreCatcherActive(Player* bot, VashjCorePassingChain const& chain, int8 index);
float GetVashjCoreSpotArrivalDistance(VashjCorePassingChain const& chain, int8 index);

}

#endif
