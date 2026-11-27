/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCHELPERS_H
#define PLAYERBOTS_SSCHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

class Creature;
class Player;
class Unit;

namespace SscHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Boss ids from serpent_shrine.h, matching DungeonEncounter.dbc's encounterIndex.
enum class SscBosses : uint32
{
    DATA_HYDROSS_THE_UNSTABLE   = 0,
    DATA_THE_LURKER_BELOW       = 1,
    DATA_LEOTHERAS_THE_BLIND    = 2,
    DATA_FATHOM_LORD_KARATHRESS = 3,
    DATA_MOROGRIM_TIDEWALKER    = 4,
    DATA_LADY_VASHJ             = 5,
};

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
    SPELL_FEAR_WARD              = 6346,
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
    SPELL_MISDIRECTION           = 34477,
    SPELL_MISDIRECTION_AURA      = 35079, // the aura on the hunter

    // Paladin
    SPELL_DIVINE_SHIELD          = 642,
    SPELL_HAND_OF_FREEDOM        = 1044,
    SPELL_AVENGING_WRATH         = 31884,

    // Priest
    SPELL_DISPERSION             = 47585,

    // Rogue
    SPELL_CLOAK_OF_SHADOWS       = 31224,

    // Shaman
    SPELL_GROUNDING_TOTEM_EFFECT = 8178,

    // Warrior
    SPELL_VIGILANCE              = 50720,
};

enum class SscNpcs : uint32
{
    // Trash
    NPC_WATER_ELEMENTAL_TOTEM    = 22236,

    // Hydross the Unstable <Duke of Currents>
    NPC_HYDROSS_THE_UNSTABLE     = 21216,

    // The Lurker Below
    NPC_THE_LURKER_BELOW         = 21217,
    NPC_COILFANG_GUARDIAN        = 21873,

    // Leotheras the Blind
    NPC_LEOTHERAS_THE_BLIND      = 21215,
    NPC_INNER_DEMON              = 21857,
    NPC_SHADOW_OF_LEOTHERAS      = 21875,

    // Morogrim Tidewalker
    NPC_MOROGRIM_TIDEWALKER      = 21213,
    NPC_TIDEWALKER_LURKER        = 21920,

    // Fathom-Lord Karathress
    NPC_FATHOM_LORD_KARATHRESS   = 21214,
    NPC_FATHOM_GUARD_CARIBDIS    = 21964,
    NPC_FATHOM_GUARD_TIDALVESS   = 21965,
    NPC_FATHOM_GUARD_SHARKKIS    = 21966,
    NPC_SPITFIRE_TOTEM           = 22091,
    NPC_FATHOM_LURKER            = 22119,
    NPC_FATHOM_SPOREBAT          = 22120,

    // Lady Vashj <Coilfang Matron>
    NPC_LADY_VASHJ               = 21212,
    NPC_ENCHANTED_ELEMENTAL      = 21958,
    NPC_COILFANG_ELITE           = 22055,
    NPC_COILFANG_STRIDER         = 22056,
    NPC_TOXIC_SPOREBAT           = 22140,

    // Pets that PetAI keeps at spell range while they have mana to cast
    NPC_IMP                      = 416,
    NPC_WATER_ELEMENTAL          = 510,
    NPC_WATER_ELEMENTAL_PERM     = 37994,
};

enum class SscItems : uint32
{
    // Lady Vashj <Coilfang Matron>
    ITEM_TAINTED_CORE            = 31088,
};

// Shared

inline constexpr uint32 SSC_MAP_ID = 548;
inline constexpr uint32 HAZARD_CACHE_INTERVAL_MS = 200;
inline constexpr float PATH_STEP_DISTANCE = 3.5f;
inline constexpr float PATH_BACKWARD_STEP_DISTANCE = 2.25f;
inline constexpr float SSC_ARRIVAL_DISTANCE = 2.0f;

Unit* GetSscBoss(Player* bot, SscBosses boss);
bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);
bool CastTankTaunt(PlayerbotAI* botAI, Unit* target);
bool FindHazardEscapeStep(
    Player* bot, Position const& hazard, float moveDist, float& stepX, float& stepY, float& stepZ);
// A path step that keeps tankedUnit in front of a tank holding it, moving backwards when the step
// leads away from it. Pass the bot's Z to MoveTo().
bool GetTankPathStep(
    Player* bot, Position const& position, float arrivalDist, Unit* tankedUnit, float& stepX,
    float& stepY, bool& backwards);
bool IsDryGround(Player* bot, float x, float y);
bool GetRangedArcAngle(Player* bot, float arcCenter, float arcSpan, float& angle);
std::vector<Unit*> GetOtherLivingGroupMembers(Player* bot);

// Trash

// 25 yd radius plus a player's 1.5 yd combat reach (1.95 yd with Bloodlust), rounded up.
inline constexpr float TOXIC_POOL_HAZARD_RADIUS = 27.0f;
inline constexpr float TOXIC_POOL_HOLDING_RADIUS = TOXIC_POOL_HAZARD_RADIUS + 5.0f;
inline constexpr float TOXIC_POOL_SEARCH_RADIUS = TOXIC_POOL_HOLDING_RADIUS + 2.0f;

bool GetToxicPoolPosition(PlayerbotAI* botAI, Position& toxicPool);
bool IsNearToxicPool(PlayerbotAI* botAI, float radius);

inline constexpr float WATER_ELEMENTAL_TOTEM_SEARCH_DISTANCE = 20.0f;
inline constexpr uint32 WATER_ELEMENTAL_TOTEM_CACHE_INTERVAL_MS = 1000;

ObjectGuid FindWaterElementalTotemGuid(Player* bot);
Creature* GetWaterElementalTotem(PlayerbotAI* botAI);
bool IsSkullOnWaterElementalTotem(PlayerbotAI* botAI);

// Hydross the Unstable <Duke of Currents>

enum class HydrossDpsHoldWindow : uint8
{
    None,
    BeforePhaseChange,
    AfterPhaseChange,
};

// Ranged spread this far apart in frost phase to mitigate Water Tomb.
inline constexpr float HYDROSS_FROST_RANGED_SPREAD_DISTANCE = 5.0f;
// Cleansing Field (37935) is a 20 yd aura on the Hydross Cleansing Field Helper (21934). Both
// combat reaches count (3 for the helper, 5 for Hydross), so the phase change occurs at 28 yd.
inline constexpr float HYDROSS_CLEANSING_FIELD_RADIUS = 28.0f;
// The incoming phase's tank waits this far short of the field's edge, on its own side.
inline constexpr float HYDROSS_HANDOFF_SHORT_DISTANCE = 8.0f;

inline Position const HYDROSS_FROST_TANK_POSITION =    { -235.653f, -354.823f, -0.828f };
inline Position const HYDROSS_NATURE_TANK_POSITION =   { -224.721f, -324.755f, -3.682f };
inline Position const HYDROSS_CLEANSING_FIELD_CENTER = { -239.715f, -366.440f, -0.745f };

bool IsHydrossFrostTank(Player* bot);
bool IsHydrossNatureTank(Player* bot);
bool IsHydrossPhaseTank(Player* bot);
bool IsHydrossAddTank(Player* bot);
bool IsHydrossInFrostPhase(Unit* hydross);
bool IsHydrossInNaturePhase(Unit* hydross);
HydrossDpsHoldWindow GetHydrossDpsHoldWindow(Unit* hydross);
bool IsHydrossPetHoldActive(Unit* hydross);
Position GetHydrossHandoffPosition(bool frostTank);
bool CanHydrossMarkBeMaxed(Unit* hydross);
bool HasMarkOfHydrossAt100Percent(Player* player);
bool HasNoMarkOfHydross(Player* bot);
bool HasMarkOfCorruptionAt100Percent(Player* player);
bool HasNoMarkOfCorruption(Player* bot);

// The Lurker Below

inline constexpr float LURKER_WHIRL_RADIUS = 25.0f;
inline constexpr float LURKER_RANGED_CLEARANCE = LURKER_WHIRL_RADIUS + 2.0f;
// Below the clearance's 2 yd margin past Whirl, since a stepping bot stops up to this far short.
inline constexpr float LURKER_RANGED_ARRIVAL_DISTANCE = 0.5f;
// Melee returning to Lurker from an islet stop this far from Lurker, on the walkway.
inline constexpr float LURKER_WALKWAY_RADIUS = 21.0f;
// A melee bot farther than this from Lurker is out on an islet. The Ambushers' islets are
// 45-55 yd from Lurker, and the Guardians spawn on land 25-30 yd from Lurker.
inline constexpr float LURKER_ISLET_DISTANCE = 40.0f;

// Spout: each bot runs on its own radius 20-22 yd from Lurker (main tank 20 yd), close to Lurker
// but mostly on dry land, and spread so it looks less artificial. Bots in the 120° cone behind
// Lurker are safe and wait out the wind-up until the spin direction is known.
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MIN = 20.0f;
inline constexpr float LURKER_SPOUT_RUN_RADIUS_MAX = 22.0f;
inline constexpr float LURKER_SPOUT_RUN_ARC_HALF_WIDTH = static_cast<float>(M_PI) / 3.0f;
inline constexpr float LURKER_SPOUT_RUN_STEP = 3.5f;
inline constexpr float LURKER_SPOUT_RUN_RADIAL_DEADZONE = 2.0f;
// A bot may run this far past directly behind Lurker, in the spin direction, before it stops.
// This is to prevent the very intelligent bots from lapping Lurker and getting blasted.
inline constexpr float LURKER_SPOUT_RUN_OVERTAKE_MARGIN = static_cast<float>(M_PI) / 6.0f;

inline constexpr size_t LURKER_GUARDIAN_TANK_COUNT = 3;
using LurkerGuardianTankAssignments = std::array<ObjectGuid, LURKER_GUARDIAN_TANK_COUNT>;
inline constexpr uint32 LURKER_GUARDIAN_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 LURKER_GUARDIAN_TANK_CACHE_INTERVAL_MS = 1000;
inline constexpr float LURKER_GUARDIAN_SEARCH_RADIUS = 100.0f;

// In front of a pillar to limit the distance that the main tank gets knocked back by Whirl.
inline Position const LURKER_MAIN_TANK_POSITION = { 23.706f, -406.038f, -19.686f };
inline Position const LURKER_SPAWN_POSITION = { 38.457f, -417.324f, -18.917f };

bool IsLurkerSpouting(Unit* lurker);
bool IsLurkerSurfacedAndCalm(Unit* lurker);
int8 GetLurkerSpoutSpin(Unit* lurker);
bool DoesPathRoundLurker(Player* bot, Unit* lurker, float x, float y, float z, int8 direction);
float GetArrivingPathLength(Player* bot, float x, float y, float z, float tolerance);
GuidVector FindLurkerGuardianGuids(Player* bot);
std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI);
GuidVector FindLurkerGuardianTankGuids(Player* bot);
int8 GetLurkerGuardianTankIndex(PlayerbotAI* botAI);
bool ShouldGoToLurkerWalkway(Player* bot, Unit* lurker, Unit* target);

// Leotheras the Blind

inline constexpr EncounterHelpers::ClassRole LEOTHERAS_WARLOCK_TANK_ROLE = {
    .playerClass = CLASS_WARLOCK,
    .preferredSpec = WARLOCK_TAB_DEMONOLOGY,
    .role = { "class_role_leotheras_warlock_tank", "Leotheras warlock tank" },
    .candidate = { "class_role_class_warlock", "Warlock" },
    .preferredCandidate = { "class_role_class_demonology_warlock", "Demonology Warlock" },
};
// 100 yd takes the raid to just around the column near the entrance to Leo's chamber.
inline constexpr float LEOTHERAS_CLASS_ROLE_ANNOUNCE_DISTANCE = 100.0f;
// 150 yd is about the distance to the entrance to Leo's chamber. Beyond him, it extends past
// Karathress's spawn point.
inline constexpr float LEOTHERAS_CLASS_ROLE_REARM_DISTANCE = 150.0f;

inline constexpr float LEOTHERAS_SEARCH_DISTANCE = LEOTHERAS_CLASS_ROLE_REARM_DISTANCE;
inline constexpr uint32 LEOTHERAS_CACHE_INTERVAL_MS = 200;

inline constexpr uint32 LEOTHERAS_HUMANOID_DPS_HOLD_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_DEMON_DPS_HOLD_MS = 10 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_FINAL_DPS_HOLD_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 LEOTHERAS_WHIRLWIND_DPS_HOLD_MS = 3 * IN_MILLISECONDS;

inline constexpr float LEOTHERAS_WHIRLWIND_HAZARD_RADIUS = 10.0f;
// Ranged stay this distance from Leo during Humanoid phase until Whirlwind begins.
inline constexpr float LEOTHERAS_RANGED_CLEARANCE = LEOTHERAS_WHIRLWIND_HAZARD_RADIUS + 5.0f;
// While Whirlwind is channeling, all non-tanks move to this distance from Leo as he is very fast.
inline constexpr float LEOTHERAS_WHIRLWIND_CLEARANCE =
    LEOTHERAS_WHIRLWIND_HAZARD_RADIUS + 15.0f;
inline constexpr float LEOTHERAS_RANGED_SPREAD_DISTANCE = 4.0f;
inline constexpr float LEOTHERAS_CHAOS_BLAST_HAZARD_RADIUS = 8.0f;
inline constexpr float LEOTHERAS_CHAOS_BLAST_CLEARANCE =
    LEOTHERAS_CHAOS_BLAST_HAZARD_RADIUS + 2.0f;
inline constexpr float LEOTHERAS_SHADOW_SEPARATION_DISTANCE = 20.0f;

inline constexpr float LEOTHERAS_LIFE_TAP_MANA_PCT = 20.0f;
inline constexpr float LEOTHERAS_LIFE_TAP_HEALTH_PCT = 90.0f;

inline Position const LEOTHERAS_SPAWN_POSITION = { 376.543f, -438.631f, 29.608f };

enum class LeotherasDpsHoldWindow : uint8
{
    None,
    Humanoid,
    Demon,
    Final,
};

GuidVector FindLeotherasUnitGuids(Player* bot);
Creature* GetLeotheras(PlayerbotAI* botAI);
bool IsSpellbinderPhase(Unit* leotheras);
Creature* GetActiveLeotherasHumanoid(PlayerbotAI* botAI);
bool IsLeotherasHumanoidPhase(PlayerbotAI* botAI);
Creature* GetLeotherasDemon(PlayerbotAI* botAI);
bool IsLeotherasDemonPhase(PlayerbotAI* botAI);
Creature* GetShadowOfLeotheras(PlayerbotAI* botAI);
bool IsLeotherasFinalPhase(PlayerbotAI* botAI);
Creature* GetLeotherasDemonOrShadow(PlayerbotAI* botAI);
bool UpdateSscClassRoles(PlayerbotAI* botAI);
bool UpdateLeotherasClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetLeotherasWarlockTank(Player* bot);
bool IsLeotherasWarlockTank(Player* bot);
bool IsLeotherasChannelingWhirlwind(Unit* leotheras);
Creature* GetLeotherasHumanoidToAvoid(PlayerbotAI* botAI);
Unit* GetDemonTargetToAvoid(Player* bot, Unit* demon);
Unit* GetChaosBlastTargetToAvoid(PlayerbotAI* botAI);
Unit* GetShadowTargetToSeparateFrom(PlayerbotAI* botAI);
LeotherasDpsHoldWindow GetLeotherasDpsHoldWindow(PlayerbotAI* botAI, Unit* leotheras);
bool IsLeotherasDpsHoldActive(PlayerbotAI* botAI, Unit* leotheras);
bool IsLeotherasPetHoldActive(PlayerbotAI* botAI, Unit* leotheras);
Unit* GetLeotherasPetTarget(PlayerbotAI* botAI, Creature* pet);
bool HasTooManyChaosBlastStacks(Player* bot);
bool HasInnerDemon(Player* bot);
Creature* GetPersonalInnerDemon(PlayerbotAI* botAI);

// Fathom-Lord Karathress

struct KarathressCouncilAssignment
{
    SscNpcs npc;
    int8 assistTankIndex; // -1 for the main tank
};

inline constexpr std::array KARATHRESS_COUNCIL = {
    KarathressCouncilAssignment{ SscNpcs::NPC_FATHOM_LORD_KARATHRESS, -1 },
    KarathressCouncilAssignment{ SscNpcs::NPC_FATHOM_GUARD_CARIBDIS, 0 },
    KarathressCouncilAssignment{ SscNpcs::NPC_FATHOM_GUARD_SHARKKIS, 1 },
    KarathressCouncilAssignment{ SscNpcs::NPC_FATHOM_GUARD_TIDALVESS, 2 },
};

// Measured from Karathress's tank position. The farthest tank position, Caribdis's, is 56 yd away.
inline constexpr float KARATHRESS_UNIT_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 KARATHRESS_UNIT_CACHE_INTERVAL_MS = 200;

// Karathress gains Blessing of the Tides if he hits 75% HP with any Fathom-Guard still alive, so if
// ranged fail to kill Caribdis before he gets to this percent health, melee needs to stop DPS.
inline constexpr float KARATHRESS_BLESSING_HOLD_HEALTH_PCT = 85.0f;
// The widest tank AoE is Death and Decay at 10 yd.
inline constexpr float KARATHRESS_AOE_THREAT_CLEARANCE = 15.0f;
inline constexpr uint32 KARATHRESS_DPS_HOLD_MS = 12 * IN_MILLISECONDS;

inline constexpr float SPITFIRE_TOTEM_SEARCH_DISTANCE = 75.0f;
// Ranged attack Spitfire Totems only when this close. This will exclude some ranged bots on
// Caribdis, which is the point, as ranged needs to maintain their spread due to Cyclones.
inline constexpr float SPITFIRE_TOTEM_RANGED_ATTACK_DISTANCE = 30.0f;

inline constexpr float CARIBDIS_HEALER_DISTANCE = 32.0f;
inline constexpr float CARIBDIS_HEALER_MAX_DISTANCE = 35.0f;
inline constexpr float CARIBDIS_TIDAL_SURGE_HAZARD_RADIUS = 10.0f;
inline constexpr float CARIBDIS_TIDAL_SURGE_CLEARANCE =
    CARIBDIS_TIDAL_SURGE_HAZARD_RADIUS + 2.0f;
// Bots move toward Caribdis until she's in sight, but never closer than this.
inline constexpr float CARIBDIS_APPROACH_STOP_DISTANCE = 5.0f;
// Cyclones spawn on random players within 45 yd of Caribdis and pick up all players within 4 yd.
inline constexpr float CARIBDIS_CYCLONE_SUMMON_RANGE = 45.0f;
inline constexpr float CARIBDIS_RANGED_SPREAD_DISTANCE = 4.0f;
// A bot more than 1 yd above the ground is treated as being in a Cyclone. One toss lifts players
// about 1.5 yd, and a bot standing on the ground is well under 1 yd from the navmesh.
inline constexpr float CARIBDIS_CYCLONE_DROP_HEIGHT = 1.0f;

// A Cyclone can leave a bot on the ledge of either lower-floor pillar's base. The navmesh doesn't
// connect the ledge to the floor, so no pathed move can take the bot off it.
inline std::array const KARATHRESS_LEDGE_PILLARS = {
    Position{ 478.9f, -487.4f, -13.0f },
    Position{ 521.6f, -487.4f, -13.0f },
};
// The floor's navmesh starts within 8.3 yd of a pillar's centre on every bearing.
inline constexpr float KARATHRESS_PILLAR_BASE_RADIUS = 8.5f;
inline constexpr float KARATHRESS_PILLAR_STEP_OFF_DISTANCE = KARATHRESS_PILLAR_BASE_RADIUS + 1.0f;
// A bot this far above the floor near a pillar is on the ledge, 3 to 4 yd up, or in the air.
inline constexpr float KARATHRESS_PILLAR_ABOVE_FLOOR_HEIGHT = 2.0f;

inline Position const KARATHRESS_TANK_POSITION = { 474.403f, -531.118f,  -7.548f };
inline Position const CARIBDIS_TANK_POSITION =   { 464.462f, -475.820f, -13.158f };
inline Position const SHARKKIS_TANK_POSITION =   { 508.057f, -541.109f, -10.133f };
inline Position const TIDALVESS_TANK_POSITION =  { 521.833f, -503.329f, -13.158f };

GuidVector FindKarathressUnitGuids(Player* bot);
Unit* GetKarathressCouncilMember(Player* bot, SscNpcs npc);
Creature* GetSpitfireTotem(PlayerbotAI* botAI);
bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem);
Unit* GetSharkkisTankTarget(PlayerbotAI* botAI);
Unit* GetSharkkisPet(Player* bot);
bool IsKarathressBlessingHoldWindowActive(Player* bot);
bool IsKarathressBlessingHoldActive(Player* bot);
bool IsKarathressStartHoldWindowActive(Player* bot);
bool IsKarathressStartDpsHoldActive(Player* bot);
bool IsKarathressPetHoldActive(Player* bot);
bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI, Unit* ownTarget);
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);
Unit* GetKarathressPetTarget(PlayerbotAI* botAI, Creature* pet);
Position const* GetKarathressLedgePillar(Player* bot);

// Morogrim Tidewalker

inline constexpr float TIDEWALKER_PHASE_2_HEALTH_PCT = 25.0f;
// The move to the corner starts a little early so it is done before the first Globules arrive.
inline constexpr float TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT = TIDEWALKER_PHASE_2_HEALTH_PCT + 2.0f;

// Non-tanks farther than this range in phase 1, such as one sent out by Watery Grave, are brought
// back. Every grave is inside this range, so a healer going to a grave victim isn't pulled back.
inline constexpr float TIDEWALKER_MAX_DISTANCE_FROM_BOSS = 45.0f;
inline constexpr float TIDEWALKER_RANGED_BEHIND_DISTANCE = 5.0f;
// Hunters can't shoot inside Tidewalker's melee range (about 8.8 yd center to center), so they get
// a farther stack distance than everybody else.
inline constexpr float TIDEWALKER_HUNTER_BEHIND_DISTANCE = 13.0f;
// The stack point follows him, so the radius absorbs his drift.
inline constexpr float TIDEWALKER_RANGED_STACK_RADIUS = 3.0f;
inline constexpr float TIDEWALKER_MURLOC_MAX_TARGET_DISTANCE = 50.0f;

inline Position const TIDEWALKER_PHASE_1_TANK_POSITION = { 410.925f, -741.916f, -7.146f };
inline Position const TIDEWALKER_PHASE_2_TANK_POSITION = { 446.571f, -767.155f, -7.144f };

Position GetTidewalkerStackPoint(Player& bot, Unit& tidewalker);

// Lady Vashj <Coilfang Matron>

// Vashj: General

// The dais is a regular dodecagon, so the apothem for each side is identical.
inline constexpr float VASHJ_DAIS_APOTHEM = 57.05f;
inline constexpr float VASHJ_STAIR_BASE_APOTHEM = 90.19f;
// This is the closest that Vashj's tank will come to the north rock (to keep Vashj from evading).
inline constexpr float VASHJ_NORTH_ROCK_CLEARANCE = 5.0f;
// The minimum distance from the north rock for bots other than Vashj's tank.
inline constexpr float VASHJ_STANDING_ROCK_CLEARANCE = 2.0f;
// Vashj follows her tank, so keeping the tank on the dais keeps her on it, but this is some slack.
inline constexpr float VASHJ_DAIS_MARGIN = 1.0f;
// Bots this far above ground in P3 are teleported down to prevent airwalking after Sporebats.
inline constexpr float VASHJ_ABOVE_GROUND_HEIGHT = 1.5f;

inline Position const VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.902f };
// The large rock that cuts into the North edge of the dais, measured from the stair base, up
// across the dais, and back down.
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

inline constexpr float VASHJ_STATIC_CHARGE_HAZARD_RADIUS = 10.0f;
inline constexpr float VASHJ_STATIC_CHARGE_CLEARANCE = VASHJ_STATIC_CHARGE_HAZARD_RADIUS + 1.0f;
// Vashj's Entangle has a 15 yd range, so ranged bots stay at least this far back from her.
inline constexpr float VASHJ_PHASE_3_RANGED_DISTANCE = 15.0f;
// A step toward Vashj to get her in sight stops here, not at RANGED_DISTANCE: path steps stop a
// little inside their stop distance, which would put the bot just inside Entangle's range. The
// extra half yard also covers path steps measured in 3D against this 2D minimum.
inline constexpr float VASHJ_PHASE_3_RANGED_STEP_STOP_DISTANCE =
    VASHJ_PHASE_3_RANGED_DISTANCE + 0.5f;
inline constexpr float VASHJ_PHASE_3_RANGED_SPREAD_DISTANCE = 4.0f;

bool HasVashjStaticCharge(Player* player);
bool IsVashjPhase3RangedTooClose(Player* bot, Unit* vashj);
bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj);
bool IsInVashjStaticChargeReach(Player* bot, Unit* vashj);
Player* GetVashjHandOfFreedomTarget(PlayerbotAI* botAI, Unit* vashj);
Player* GetVashjGroundingShaman(Player* bot);
Player* FindVashjGroundingShaman(Player* bot);

// Vashj: Toxic Spores

// 5 yd plus a player's 1.5 yd combat reach (1.95 yd with Bloodlust).
inline constexpr float TOXIC_SPORES_HAZARD_RADIUS = 6.5f;
inline constexpr float TOXIC_SPORES_AVOID_RADIUS = TOXIC_SPORES_HAZARD_RADIUS + 1.0f;
// Vashj's tank has a greater avoidance radius to keep melee DPS on the far side of Vashj clear
// as well. She is ~3.5 yd from the tank and melee ~3.75 yd past her (7.25 total + 0.25 slack).
inline constexpr float TOXIC_SPORES_TANK_AVOID_RADIUS = TOXIC_SPORES_AVOID_RADIUS + 7.5f;
inline constexpr float TOXIC_SPORES_SEARCH_RADIUS = 50.0f;
// Melee DPS that are this close to a toxic pool have their standard movement overridden by the
// melee spore action to keep them from running through pools to reach Vashj.
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
bool FindVashjTankBreakoutPosition(
    Player* bot, std::vector<Position> const& spores, Position& position);
bool IsVashjRingMelee(Player* bot, Unit* vashj);
bool IsNearToxicSpores(PlayerbotAI* botAI, float radius);
bool IsInMeleeRangeClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius);
bool CanWalkThroughToxicSpores(Player* bot);
bool GetMeleeRingStepClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius, float& stepX,
    float& stepY, float& stepZ);
bool GetStepOutOfNearestSpore(
    Player* bot, std::vector<Position> const& spores, float radius, float& stepX, float& stepY,
    float& stepZ);
bool GetVashjReachBlockedBySpores(PlayerbotAI* botAI, Unit*& target, float& range);
bool GetStepToCastRangeAroundSpores(
    Player* bot, Unit* target, float castRange, std::vector<Position> const& spores, float& stepX,
    float& stepY, float& stepZ);

// Vashj: Phase 2 Ranged Stations

inline constexpr size_t VASHJ_STATION_RANGED_SLOTS = 3;
inline constexpr int8 VASHJ_STATION_HEALER_SLOT = 3;

struct VashjStation
{
    std::array<Position, VASHJ_STATION_RANGED_SLOTS> ranged;
    Position healer;
};

// A bot's station and its slot there: 0-2 for ranged DPS, VASHJ_STATION_HEALER_SLOT for the healer.
struct VashjStationSlot
{
    int8 station = -1;
    int8 slot = -1;
};

inline std::array const VASHJ_STATIONS = {
    // Slots at -156°, -178°, and -134°
    VashjStation{
        {
            Position{ -17.87f, -944.69f, 41.30f },
            Position{ -22.33f, -925.36f, 41.30f },
            Position{  -6.49f, -960.95f, 41.30f },
        },
        Position{ -6.91f, -939.81f, 41.65f },
    },
    // Slots at 116°, 102°, and 130°
    VashjStation{
        {
            Position{   6.84f, -876.80f, 41.30f },
            Position{  18.82f, -872.68f, 41.30f },
            Position{  -3.79f, -883.71f, 41.30f },
        },
        Position{ 12.10f, -887.59f, 41.65f },
    },
    // Slots at -68°, -80°, and -56°
    VashjStation{
        {
            Position{  49.11f, -971.75f, 41.30f },
            Position{  38.66f, -974.75f, 41.30f },
            Position{  58.71f, -966.65f, 41.30f },
        },
        Position{ 44.62f, -960.63f, 41.65f },
    },
    // Slots at 43°, 37°, and 49°
    VashjStation{
        {
            Position{  67.66f, -888.08f, 41.30f },
            Position{  71.16f, -892.25f, 41.30f },
            Position{  63.75f, -884.30f, 41.30f },
        },
        Position{ 60.35f, -894.90f, 41.65f },
    },
};

inline constexpr size_t VASHJ_STATION_COUNT = VASHJ_STATIONS.size();
// The northeast station is filled first because it has the farthest potential run to a Tainted
// Elemental (the spawn point just east of the north rock), so it is the first to get a 3rd DPS.
inline constexpr std::array VASHJ_STATION_FILL_ORDER = {
    int8{ 2 }, int8{ 0 }, int8{ 1 }, int8{ 3 },
};
static_assert(VASHJ_STATION_FILL_ORDER.size() == VASHJ_STATION_COUNT);
using VashjStationHolders =
    std::array<std::array<ObjectGuid, VASHJ_STATION_RANGED_SLOTS + 1>, VASHJ_STATION_COUNT>;

std::vector<VashjStationSlot> GetVashjStationFillOrder();
bool IsLiveVashjStationHolder(Player* bot, ObjectGuid guid);
bool HasVashjStationVacancy(Player* bot);
VashjStationSlot GetVashjStationSlot(Player* bot);
Position const* GetVashjStationPositionToReturnTo(Player* bot, Unit* currentTarget);
int8 GetNearestVashjStation(Unit* unit);

// Vashj: Adds and Target Priority

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
    float maxDistanceFromVashj = std::numeric_limits<float>::max();
};

struct VashjTargetFacts
{
    Unit* vashj = nullptr;
    Unit* tainted = nullptr;
    int8 phase = -1;
    // Measured from the bot, and in phase 2 also from the platform center, so bots don't chase
    // adds down the stairs.
    float maxPursueRange = 0.0f;
    float maxSearchRange = 0.0f;
    float spellRange = 0.0f;
    // Phase 2 ranged DPS at stations shoot only what's in range of their slot, except the station
    // sent after a Tainted Elemental.
    bool holdsStationSlot = false;
    // Phase 2: everyone but tanks leaves an Elite or Strider alone until a tank has it, so nobody
    // pulls one onto a station.
    bool waitForTank = false;
    // Tanks: one per Elite or Strider, so the others stay free for the next ones. A new one goes
    // to the nearest free tank.
    bool oneTankEach = false;
    // The living Striders. Melee DPS attack targets only outside their Panic radius.
    std::vector<Unit*> panicStriders;
};

inline constexpr uint32 VASHJ_ADDS_CACHE_INTERVAL_MS = 200;
// Measured from the platform center. Enchanted Elementals spawn up to 93 yd from it.
inline constexpr float VASHJ_ADD_SEARCH_RADIUS = 100.0f;
// Pets are sent only to Enchanted Elementals within this distance of their owner.
inline constexpr float VASHJ_PET_ENCHANTED_MAX_DISTANCE = 100.0f;
// Panic (38258) fears every player within 11 yd of a Strider, center to center.
inline constexpr float VASHJ_STRIDER_PANIC_RADIUS = 11.0f;
inline constexpr float VASHJ_STRIDER_STEP_IN_DISTANCE = 50.0f;
inline constexpr float VASHJ_ENCHANTED_NEAR_HER_DISTANCE = 20.0f;
inline constexpr float VASHJ_IDLE_TANK_DISTANCE = 10.0f;
inline constexpr float VASHJ_ADD_TANK_ARRIVAL_DISTANCE = 3.0f;
inline constexpr float VASHJ_PHASE_3_STRIDER_DISTANCE_FROM_VASHJ = 28.0f;

// Target tiers by phase and role, best first (GetVashjTargetTiers)
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_STATION_RANGED_TIERS = {
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
inline std::vector<VashjTargetTier> const VASHJ_PHASE_2_MELEE_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental, VASHJ_ENCHANTED_NEAR_HER_DISTANCE },
    VashjTargetTier{ VashjTarget::CoilfangElite },
};
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
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_TANK_TIERS = {
    VashjTargetTier{ VashjTarget::CoilfangStrider },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::EnchantedElemental },
    VashjTargetTier{ VashjTarget::LadyVashj },
};
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
inline std::vector<VashjTargetTier> const VASHJ_PHASE_3_MELEE_TIERS = {
    VashjTargetTier{ VashjTarget::EnchantedElemental, VASHJ_ENCHANTED_NEAR_HER_DISTANCE },
    VashjTargetTier{ VashjTarget::CoilfangElite },
    VashjTargetTier{ VashjTarget::LadyVashj },
};

// Each position is between two ranged stations, at least 16 yd from every station slot and healer
// post and at least 18 yd from each generator.
inline std::array const VASHJ_STRIDER_HOLD_POSITIONS = {
    Position{ -6.0f, -913.5f, 41.9f },
    Position{  9.5f, -963.5f, 41.5f },
    Position{ 33.5f, -889.5f, 41.9f },
};
// Each position is in range of all three ranged DPS slots of one ranged station and at least 18 yd
// from every Strider holding position.
inline std::array const VASHJ_ELITE_TANK_POSITIONS = {
    Position{ 57.0f, -913.0f, 42.0f },
    Position{  5.5f, -934.0f, 42.1f },
};

GuidVector FindVashjAddGuids(Player* bot);
GuidVector const& GetVashjAdds(Player* bot);
std::vector<VashjTargetTier> const& GetVashjTargetTiers(Player* bot, int8 phase, bool killsTainted);
bool IsVashjAddHeldByTank(Unit* unit);
Player* GetVashjAddOwningTank(Player* bot, Unit* add);
bool IsNearestFreeVashjTank(Player* bot, Unit* add, Unit* vashj, int8 phase);
bool GetStepToBringTankedUnitTo(
    Player* bot, Unit* add, Position const& position, float arrivalDistance, float& stepX,
    float& stepY, bool& backwards);
Position const& GetVashjStriderHoldPosition(Unit& strider);
Position const& GetVashjEliteTankPosition(Unit& elite);
bool ShouldPositionVashjStrider(Player* bot, Unit* strider, Unit* vashj, int8 phase);
bool IsTankedStriderInStepInReach(Player* bot, Unit* unit);
Unit* GetVashjPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* vashj);
bool ShouldVashjPetsBePassive(PlayerbotAI* botAI, Unit* vashj);

// Vashj: Tainted Elemental

struct TaintedCoreLooter
{
    ObjectGuid tainted;
    ObjectGuid looter;
    int8 station = -1;
};

inline constexpr float VASHJ_CORE_LOOT_RANGE = INTERACTION_DISTANCE - 2.0f;

Player* FindTaintedCoreLooter(Player* bot, Unit* tainted, int8 station);
Creature* GetAssignedTaintedElemental(Player* bot);
int8 GetTaintedCoreLootSlot(Creature* tainted);
bool IsTaintedCoreStillToLoot(Creature* tainted);
Creature* GetTaintedElementalToKill(Player* bot);
bool IsDesignatedCoreLooter(Player* bot);
bool HasTaintedCore(Player* player);

// Vashj: Core Passing Chain

struct VashjCoreCatcher
{
    Position position;
    ObjectGuid bot = ObjectGuid::Empty;
    // The first catcher is released when the plan is made, the second when the first starts moving,
    // each later one when the one before reaches its position, delayed by readyDelay for realism.
    bool released = false;
    uint32 releaseTime = 0;
    uint32 readyDelay = 0;
    bool prepositions = false;
    bool arrived = false;
};

// The route one core takes to a generator, per instance. The mechanic tracker bot plans it when
// the looter is picked. If the next throw can't be made, the holder replans from where it stands.
struct VashjCorePassingChain
{
    ObjectGuid tainted;
    ObjectGuid generator;
    // Who throws to the first catcher: the looter, or the core holder if the chain was replanned.
    ObjectGuid originBot;
    std::vector<VashjCoreCatcher> catchers;
    // A catcher whose throws failed is left out of every later replan (the most likely reason for
    // repeated failure is a full inventory).
    ObjectGuid excluded;
    int8 reached = -1;
    bool failed = false;
    uint8 replans = 0;
    // The last throw, kept through a replan so the next throw still waits for it.
    ObjectGuid throwTarget;
    uint32 throwTime = 0;
    uint8 failedThrows = 0;
    // The catcher the holder is waiting on, when that wait started, and when the catcher started
    // standing in its position but out of throw range.
    ObjectGuid waitTarget;
    uint32 waitStart = 0;
    uint32 blockedStart = 0;
};

// Throw Key's range is 40 yd + both combat reaches (so 43 yd minimum). To allow a bit of slack
// and realism, positions in the passing chain are planned only 40 yd apart, center to center.
inline constexpr float VASHJ_CORE_THROW_PLAN_DISTANCE = 40.0f;
// The looter stands up to ~6 yd from the corpse, and the first throw is planned from the corpse.
inline constexpr float VASHJ_CORE_LOOTER_OFFSET = 6.0f;
// Line of sight is checked from each player's collision height, 1.21 (Gnome) to 2.64 (female
// Tauren). Plans assume everybody is a Gnome.
inline constexpr float VASHJ_CORE_PLAN_EYE_HEIGHT = 1.2f;
// Upon spawn, Elites and Striders move toward Vashj and attack the first player found within 20 yd
// of them. To keep bots from being donked, the goal is to keep non-tanks more than 20 yd away from
// the lanes from spawn to Vashj.
inline constexpr float VASHJ_CORE_POSITION_SPAWN_CLEARANCE = 22.0f;
// The generators have display id 7265 at size 2.1. The widest point of the generator's base,
// measured from its bounding box, is about 3.2 yd. I don't know the exact collision radius.
inline constexpr float VASHJ_CORE_POSITION_GENERATOR_CLEARANCE = 5.0f;
// Five players should be able to get from every Tainted Elemental spawn position to every
// generator while following the passing rules (e.g., collision, distance from add lanes).
inline constexpr size_t VASHJ_CORE_MAX_CATCHERS = 5;
// A chain member needs to be within this distance of its calculated position to be considered in
// position. The use position has less tolerance to ensure the bot is truly within use distance.
inline constexpr float VASHJ_CORE_POSITION_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float VASHJ_CORE_USE_POSITION_ARRIVAL_DISTANCE = 0.5f;
inline constexpr std::array VASHJ_SHIELD_GENERATOR_SPAWN_IDS = {
    uint32{ 47482 }, // NW
    uint32{ 47483 }, // NE
    uint32{ 47484 }, // SE
    uint32{ 47485 }, // SW
};

// The locations of the four triggers that spawn Elites and Striders. Each is just inside the dais
// edge (they range from 54-56.5 yd from the center).
inline std::array const VASHJ_ADD_SPAWN_POSITIONS = {
    Position{  43.329f, -869.731f, 41.2f },
    Position{ -22.597f, -900.382f, 41.2f },
    Position{  13.781f, -975.633f, 41.2f },
    Position{  78.381f, -950.659f, 41.2f },
};
// The small rock that cuts into the Southwestern stairs, measured from stair base to the highest
// point of the rock and back down the other side.
inline std::array const VASHJ_SOUTH_WEST_ROCK = {
    Position{ -16.473f, -843.635f, 22.78f },
    Position{ -10.493f, -849.837f, 27.23f },
    Position{ -10.034f, -860.397f, 32.37f },
    Position{ -14.241f, -865.373f, 32.69f },
    Position{ -46.103f, -872.655f, 22.53f },
};
inline std::array const VASHJ_SHIELD_GENERATOR_POSITIONS = {
    Position{ 52.048f, -901.236f, 44.0f },
    Position{ 52.448f, -944.825f, 44.0f },
    Position{  7.810f, -945.244f, 44.0f },
    Position{  7.417f, -901.109f, 44.0f },
};

void PlanVashjCorePassingChain(Player* bot, Unit* tainted, Player* looter);
bool ReplanVashjCorePassingChain(Player* holder, VashjCorePassingChain& chain, ObjectGuid excluded);
bool ReassignVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index);
void ReleaseVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index);
VashjCorePassingChain* GetVashjCorePassingChain(Player* bot);
int8 GetVashjCoreCatcherIndex(VashjCorePassingChain const& chain, Player* bot);
bool IsVashjCoreCatcherActive(Player* bot, VashjCorePassingChain const& chain, int8 index);
float GetVashjCorePositionArrivalDistance(VashjCorePassingChain const& chain, int8 index);

// Shared encounter state
struct SscInstanceState
{
    std::optional<uint32> hydrossFrostPhaseStartTime;
    std::optional<uint32> hydrossNaturePhaseStartTime;
    std::optional<uint32> hydrossFrostMarkMaxedTime;
    std::optional<uint32> hydrossNatureMarkMaxedTime;
    EncounterHelpers::InstanceCreatureList lurkerGuardians;
    std::optional<LurkerGuardianTankAssignments> lurkerGuardianTankAssignments;
    EncounterHelpers::InstanceCreatureList leotherasUnits;
    std::optional<uint32> leotherasHumanoidPhaseStartTime;
    std::optional<uint32> leotherasWhirlwindEndTime;
    std::optional<uint32> leotherasDemonPhaseStartTime;
    std::optional<uint32> leotherasFinalPhaseStartTime;
    EncounterHelpers::InstanceCreatureList karathressUnits;
    std::optional<uint32> karathressDpsHoldTimer;
    EncounterHelpers::InstanceCreatureList vashjAdds;
    std::optional<ObjectGuid> vashjGroundingShaman;
    std::optional<VashjStationHolders> vashjStationHolders;
    std::optional<TaintedCoreLooter> vashjTaintedCoreLooter;
    std::optional<VashjCorePassingChain> vashjCorePassingChain;
    EncounterHelpers::ClassRoleAssignment leotherasWarlockTank;
};

SscInstanceState& SscState(uint32 instanceId);
SscInstanceState* FindSscState(uint32 instanceId);
bool SscResetInstance(uint32 instanceId);

}

#endif
