/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKHELPERS_H
#define PLAYERBOTS_TKHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include <array>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

class Creature;
class Item;
class Player;
class Unit;

namespace TkHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Boss ids from the_eye.h, matching DungeonEncounter.dbc's encounterIndex.
enum class TkBosses : uint32
{
    DATA_ALAR                       = 0,
    DATA_VOID_REAVER                = 1,
    DATA_HIGH_ASTROMANCER_SOLARIAN  = 2,
    DATA_KAELTHAS_SUNSTRIDER        = 3,
};

enum class TkSpells : uint32
{
    // Crimson Hand Centurion
    SPELL_ARCANE_FLURRY             = 37268,

    // Al'ar
    SPELL_MODEL_INVISIBILITY        = 24401, // "Test Pet Passive" (AC hack? used for Al'ar)
    SPELL_REBIRTH_PHASE2            = 34342,
    SPELL_REBIRTH_DIVE              = 35369,
    SPELL_MELT_ARMOR                = 35410,

    // Void Reaver
    SPELL_ARCANE_ORB                = 34172,

    // High Astromancer Solarian
    SPELL_SOLARIAN_TRANSFORM        = 39117,
    SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

    // Kael'thas Sunstrider
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_EMBER_BLAST               = 34341, // For Phoenixes. Al'ar has this same aura.
    SPELL_MENTAL_PROTECTION_FIELD   = 36480, // Staff of Disintegration
    SPELL_ARCANE_BARRIER            = 36481, // Phaseshift Bulwark
    SPELL_KAELTHAS_MIND_CONTROL     = 36797,
    SPELL_SHOCK_BARRIER             = 36815,
    SPELL_GRAVITY_LAPSE_AURA        = 39432, // AC uses this name (SPELL_GRAVITY_LAPSE is 35941)

    // Hunter
    SPELL_MISDIRECTION              = 34477,
    SPELL_MISDIRECTION_AURA         = 35079, // the aura on the hunter

    // Shaman
    SPELL_TREMOR_TOTEM              = 8143,
};

enum class TkNpcs : uint32
{
    // Trash
    NPC_CRIMSON_HAND_CENTURION      = 20048,

    // Al'ar
    NPC_EMBER_OF_ALAR               = 19551,
    NPC_FLAME_PATCH                 = 20602,

    // High Astromancer Solarian
    NPC_SOLARIUM_PRIEST             = 18806,
    NPC_SOLARIUM_AGENT              = 18925,

    // Kael'thas Sunstrider
    NPC_KAELTHAS_SUNSTRIDER         = 19622,
    NPC_LORD_SANGUINAR              = 20060,
    NPC_GRAND_ASTROMANCER_CAPERNIAN = 20062,
    NPC_MASTER_ENGINEER_TELONICUS   = 20063,
    NPC_THALADRED_THE_DARKENER      = 20064,
    NPC_NETHERSTRAND_LONGBOW        = 21268,
    NPC_DEVASTATION                 = 21269,
    NPC_COSMIC_INFUSER              = 21270,
    NPC_INFINITY_BLADES             = 21271,
    NPC_WARP_SLICER                 = 21272,
    NPC_PHASESHIFT_BULWARK          = 21273,
    NPC_STAFF_OF_DISINTEGRATION     = 21274,
    // NPC_NETHER_VAPOR             = 21002, // Unimplemented in AC; method needed if fixed
    NPC_PHOENIX                     = 21362,
    NPC_PHOENIX_EGG                 = 21364,
    NPC_FLAME_STRIKE_TRIGGER        = 21369,
};

enum class TkItems : uint32
{
    // Kael'thas Sunstrider
    ITEM_WARP_SLICER                = 30311,
    ITEM_INFINITY_BLADE             = 30312,
    ITEM_STAFF_OF_DISINTEGRATION    = 30313,
    ITEM_PHASESHIFT_BULWARK         = 30314,
    // ITEM_DRAENETHYST_MINE_CRYSTAL= 30315, // Noting that the ids are not all contiguous
    ITEM_DEVASTATION                = 30316,
    ITEM_COSMIC_INFUSER             = 30317,
    ITEM_NETHERSTRAND_LONGBOW       = 30318,
    ITEM_NETHER_SPIKES              = 30319,
};

// Shared

inline constexpr uint32 TK_MAP_ID = 550;
inline constexpr uint32 TK_CACHE_INTERVAL_MS = 200;
inline constexpr float TK_ARRIVAL_DISTANCE = 2.0f;

Unit* GetTkBoss(Player* bot, TkBosses boss);
std::vector<Unit*> GetTkAdds(PlayerbotAI* botAI, TkNpcs npc);
std::pair<Unit*, Unit*> GetTargetUnitPair(PlayerbotAI* botAI, TkNpcs npc);
Player* GetNearestNonTankPlayerInRadius(Player* bot, float radius);

// Trash

Unit* GetCenturionCastingArcaneFlurry(PlayerbotAI* botAI);

// Al'ar <Phoenix God>
// Al'ar's combat reach is 15 yd.

enum AlarLocationIndex
{
    PLATFORM_0_IDX, // West
    PLATFORM_1_IDX, // Northwest
    PLATFORM_2_IDX, // Northeast
    PLATFORM_3_IDX, // East
    POINT_QUILL_OR_DIVE_IDX,
    POINT_MIDDLE_IDX,
    LOCATION_NONE = -1
};

// 17.0f is intentionally a little lower than the actual balcony positions to capture melee that
// may be slightly down the ramps from Platform 0 or 3.
inline constexpr float ALAR_BALCONY_Z = 17.0f;
inline constexpr uint8 TOTAL_ALAR_LOCATIONS = 6;

// Radius of Flame Patch (35383). No combat reaches added.
inline constexpr float FLAME_PATCH_HAZARD_RADIUS = 6.0f;
inline constexpr float FLAME_PATCH_CLEARANCE = FLAME_PATCH_HAZARD_RADIUS + 2.0f;
// Radius of Ember Blast (34133), cast when an Ember of Al'ar dies. No combat reaches added.
inline constexpr float EMBER_OF_ALAR_HAZARD_RADIUS = 15.0f;
inline constexpr float EMBER_OF_ALAR_CLEARANCE = EMBER_OF_ALAR_HAZARD_RADIUS + 5.0f;
// Radius of Dive Bomb (35181) around its target. No combat reaches added.
inline constexpr float DIVE_BOMB_HAZARD_RADIUS = 10.0f;
inline constexpr float DIVE_BOMB_CLEARANCE = DIVE_BOMB_HAZARD_RADIUS + 5.0f;
// Radius of Rebirth (34342 at the phase change, 35369 after Dive Bomb). No combat reaches added.
inline constexpr float ALAR_REBIRTH_HAZARD_RADIUS = 15.0f;
inline constexpr float ALAR_REBIRTH_CLEARANCE = ALAR_REBIRTH_HAZARD_RADIUS + 10.0f;

inline Position const ALAR_LANDING_PLATFORM_0 = { 335.638f,  59.4879f, 17.9319f }; // West
inline Position const ALAR_LANDING_PLATFORM_1 = { 388.751f,  31.7312f, 20.2636f }; // Northwest
inline Position const ALAR_LANDING_PLATFORM_2 = { 388.791f, -33.1059f, 20.2636f }; // Northeast
inline Position const ALAR_LANDING_PLATFORM_3 = { 332.723f, -61.1590f, 17.9791f }; // East
inline std::array const ALAR_LANDING_PLATFORM_POSITIONS = {
    ALAR_LANDING_PLATFORM_0,
    ALAR_LANDING_PLATFORM_1,
    ALAR_LANDING_PLATFORM_2,
    ALAR_LANDING_PLATFORM_3,
};

inline Position const ALAR_TANK_PLATFORM_0 = { 329.433f,  59.914f, 17.548f };
inline Position const ALAR_TANK_PLATFORM_1 = { 395.772f,  26.779f, 20.181f };
inline Position const ALAR_TANK_PLATFORM_2 = { 387.498f, -41.909f, 20.182f };
inline Position const ALAR_TANK_PLATFORM_3 = { 330.342f, -61.902f, 17.719f };
inline std::array const ALAR_TANK_PLATFORM_POSITIONS = {
    ALAR_TANK_PLATFORM_0,
    ALAR_TANK_PLATFORM_1,
    ALAR_TANK_PLATFORM_2,
    ALAR_TANK_PLATFORM_3,
};

inline Position const ALAR_MELEE_DPS_PLATFORM_0 = { 344.037f,  64.339f, 18.350f };
inline Position const ALAR_MELEE_DPS_PLATFORM_1 = { 387.558f,  40.300f, 20.182f };
inline Position const ALAR_MELEE_DPS_PLATFORM_2 = { 395.275f, -28.573f, 20.182f };
inline Position const ALAR_MELEE_DPS_PLATFORM_3 = { 346.427f, -69.361f, 18.750f };
inline std::array const ALAR_MELEE_DPS_PLATFORM_POSITIONS = {
    ALAR_MELEE_DPS_PLATFORM_0,
    ALAR_MELEE_DPS_PLATFORM_1,
    ALAR_MELEE_DPS_PLATFORM_2,
    ALAR_MELEE_DPS_PLATFORM_3,
};

inline std::array const ALAR_GROUND_POSITIONS = {
    Position{ 336.439f,  48.181f, -2.389f }, // Underish West platform
    Position{ 379.122f,  25.146f, -2.385f }, // Underish Northwest platform
    Position{ 378.583f, -27.481f, -2.385f }, // Underish Northeast platform
    Position{ 331.631f, -49.716f, -2.389f }, // Underish East platform
};

inline Position const ALAR_ROOM_CENTER         = { 330.611f,  -2.540f, -2.389f };
inline Position const ALAR_POINT_QUILL_OR_DIVE = { 332.000f,   0.010f, 43.000f };
inline Position const ALAR_POINT_MIDDLE        = { 331.000f,   0.010f, -2.380f };
inline Position const ALAR_SE_RAMP_BASE        = { 281.064f, -36.590f, -2.389f };
inline Position const ALAR_SW_RAMP_BASE        = { 281.064f,  36.590f, -2.389f };
inline Position const ALAR_ROOM_S_CENTER       = { 281.064f,   0.000f, -2.389f };

// Measured from the center of Al'ar's room. Every platform, ramp, and ground position is within
// 70 yd of it, so 100 yd covers them.
inline constexpr float ALAR_ROOM_SEARCH_RADIUS = 100.0f;

bool IsAlarInPhase2(uint32 instanceId);
int8 GetAlarDestinationLocationIndex(Unit* alar);
int8 GetAlarCurrentLocationIndex(Unit* alar);
int8 GetAlarPlatformIndex(Unit* alar);
Position const& GetClosestGroundPosition(Position const& botPos);
bool IsPrimaryEmberTank(Player* bot);
bool IsFirstAlarTank(Player* bot);
bool IsSecondAlarTank(Player* bot);
Player* GetSecondaryEmberTank(Player* bot);
GuidVector FindAlarEmberGuids(Player* bot);
GuidVector FindFlamePatchGuids(Player* bot);
std::vector<Unit*> GetFlamePatches(PlayerbotAI* botAI);

// Void Reaver
// Void Reaver's combat reach is 15 yd.

struct ArcaneOrbData
{
    Position destination;
    uint32 castTime;
};

inline constexpr uint32 ARCANE_ORB_DURATION_MS = 7000;
// Radius of Arcane Orb (34190). No combat reaches added.
inline constexpr float ARCANE_ORB_HAZARD_RADIUS = 20.0f;
inline constexpr float ARCANE_ORB_CLEARANCE = ARCANE_ORB_HAZARD_RADIUS + 2.0f;
inline constexpr float ARCANE_ORB_BUFFER_DISTANCE = ARCANE_ORB_HAZARD_RADIUS + 10.0f;
// Arcane Orb targets players who are beyond this range, edge to edge. Ranged stay past it so the
// orbs go to them and land away from melee.
inline constexpr float ARCANE_ORB_MIN_TARGET_RANGE = 20.0f;
inline constexpr float VOID_REAVER_RANGED_DISTANCE = ARCANE_ORB_MIN_TARGET_RANGE + 2.0f;
// Based on generic ranged spell range of 30 yd, edge to edge. Ranged stay a little inside so they
// can always hit Void Reaver.
inline constexpr float VOID_REAVER_RANGED_MAX_DISTANCE = 28.0f;
// While the tank is moving and still within Void Reaver's melee range, Void Reaver chases to its
// reach minus half the tank's speed, not to the tank's lead distance, so it can stop up to 4 yd
// past the center.
inline constexpr float VOID_REAVER_BOSS_ARRIVAL_DISTANCE = 5.0f;

// Center of the room: the point with the most open floor (57 yd to the nearest navmesh edge).
inline Position const VOID_REAVER_TANKING_POSITION = { 433.545f, 370.833f, 14.975f };

std::vector<Position> GetActiveArcaneOrbs(uint32 instanceId);
bool IsNearArcaneOrb(Player* bot, std::vector<Position> const& orbs, float radius);
bool IsNearActiveArcaneOrb(Player* bot, float radius);

// High Astromancer Solarian

// Measured from Solarian's spawn position. 100 yd covers her room.
inline constexpr float SOLARIAN_ROOM_SEARCH_RADIUS = 100.0f;
// Radius of the explosion (42787) around the afflicted player. No combat reaches added.
inline constexpr float WRATH_OF_THE_ASTROMANCER_HAZARD_RADIUS = 10.0f;
inline constexpr float WRATH_OF_THE_ASTROMANCER_CLEARANCE =
    WRATH_OF_THE_ASTROMANCER_HAZARD_RADIUS + 5.0f;

inline Position const SOLARIAN_SPAWN_POSITION = { 432.740f, -373.645f, 18.014f };

bool HasWrathOfTheAstromancer(Player* bot);
GuidVector FindSolarianAddGuids(Player* bot);
bool IsInVoidwalkerForm(Unit* astromancer);

// Kael'thas Sunstrider <Lord of the Blood Elves>

// Matches the phase enum of the core's boss_kaelthas.cpp. Reading the phase means casting the
// boss AI to a mirror of that class (TKKaelthasBossAI.h).
enum KTPhases
{
    PHASE_NONE           = 0,
    PHASE_SINGLE_ADVISOR = 1,
    PHASE_WEAPONS        = 2,
    PHASE_TRANSITION     = 3,
    PHASE_ALL_ADVISORS   = 4,
    PHASE_FINAL          = 5
};

// 125 yd is about the exact distance from Kael to the entrances to his room. Room searches from his
// spawn position use it too.
inline constexpr float KAELTHAS_ROOM_SEARCH_DISTANCE = 125.0f;

inline constexpr uint32 ADVISOR_DPS_HOLD_NOT_STARTED = 0;
inline constexpr uint32 ADVISOR_DPS_HOLD_MS = 10 * IN_MILLISECONDS;
// The active warlock tank uses Life Tap when below this percent mana and above this percent health.
inline constexpr float CAPERNIAN_LIFE_TAP_MANA_PCT = 20.0f;
inline constexpr float CAPERNIAN_LIFE_TAP_HEALTH_PCT = 75.0f;

// Covers the 5 yd splash (37019) around each burning player, not Conflagration's 8 yd initial hit.
// No combat reaches added.
inline constexpr float CONFLAGRATION_SPLASH_RADIUS = 5.0f;
inline constexpr float CONFLAGRATION_SPLASH_CLEARANCE = CONFLAGRATION_SPLASH_RADIUS + 1.0f;
// Radius of Whirlwind (36982). No combat reaches added.
inline constexpr float DEVASTATION_HAZARD_RADIUS = 8.0f;
inline constexpr float DEVASTATION_CLEARANCE = DEVASTATION_HAZARD_RADIUS + 4.0f;
// Radius of Flame Strike (36731): the blast (115,625 damage) is 6 yd, no combat reaches added. The
// persistent area AoE (3000/s) is 10 yd + player's 1.5 yd combat reach (1.95 yd with Bloodlust).
inline constexpr float FLAME_STRIKE_HAZARD_RADIUS = 11.5f;
inline constexpr float FLAME_STRIKE_CLEARANCE = FLAME_STRIKE_HAZARD_RADIUS + 2.0f;
// Do not attempt to avoid Phoenixes if within this distance of a Flame Strike.
inline constexpr float FLAME_STRIKE_BUFFER_DISTANCE = FLAME_STRIKE_HAZARD_RADIUS + 7.0f;
// Radius of Burn (36721). No combat reaches added.
inline constexpr float PHOENIX_HAZARD_RADIUS = 8.0f;
inline constexpr float PHOENIX_CLEARANCE = PHOENIX_HAZARD_RADIUS + 4.0f;

inline constexpr EncounterHelpers::ClassRole CAPERNIAN_TANK_ROLE = {
    .playerClass = CLASS_WARLOCK,
    .preferredSpec = WARLOCK_TAB_DEMONOLOGY,
    .role = { "class_role_capernian_warlock_tank", "Capernian warlock tank" },
    .candidate = { "class_role_class_warlock", "Warlock" },
    .preferredCandidate = { "class_role_class_demonology_warlock", "Demonology Warlock" },
};
inline constexpr EncounterHelpers::ClassRole SANGUINAR_DEBUFF_HUNTER_ROLE = {
    .playerClass = CLASS_HUNTER,
    .role = { "class_role_sanguinar_debuff_hunter", "Sanguinar debuff hunter" },
    .candidate = { "class_role_class_hunter", "Hunter" },
};
inline constexpr float KAELTHAS_CLASS_ROLE_ANNOUNCE_DISTANCE = KAELTHAS_ROOM_SEARCH_DISTANCE;
// 200 yards extends to the tops of the ramps leading to Kael's room's entrances.
inline constexpr float KAELTHAS_CLASS_ROLE_REARM_DISTANCE = 200.0f;

inline Position const KAELTHAS_SPAWN_POSITION =    { 795.408f, -1.605f, 48.812f };
// To the left of the advisors' starting position (when facing them), up against the wall.
inline Position const SANGUINAR_TANK_POSITION    = { 775.478f,  39.888f, 46.780f };
inline Position const SANGUINAR_WAITING_POSITION = { 761.850f,  27.459f, 46.779f };
// Next to Sanguinar.
inline Position const TELONICUS_TANK_POSITION    = { 773.717f,  44.091f, 46.780f };
inline Position const TELONICUS_WAITING_POSITION = { 754.347f,  31.739f, 46.796f };
inline Position const CAPERNIAN_WAITING_POSITION = { 743.897f, -11.575f, 46.779f };
// A bit aways from the Sanguinar and Telonicus tanking positions.
inline Position const ADVISOR_HEAL_POSITION      = { 752.171f,  19.494f, 46.779f };
// Towards the front-center of the platform to ensure an open area for Gravity Lapse.
inline Position const KAELTHAS_TANK_POSITION     = { 774.008f,  -0.631f, 48.729f };

enum class KaelthasAdvisorDpsHold : uint8
{
    None,
    Sanguinar,
    Capernian,
    Telonicus,
};

struct LegendaryWeaponPriority
{
    TkNpcs entry;
    // The axe is marked with a cross instead of skull as it is outside the full priority chain
    // (only ranged DPS attacks it).
    bool markWithCross;
    bool rangedDpsOnly;
};
inline constexpr std::array LEGENDARY_WEAPON_PRIORITIES = {
    LegendaryWeaponPriority{ TkNpcs::NPC_STAFF_OF_DISINTEGRATION, false, false },
    LegendaryWeaponPriority{ TkNpcs::NPC_COSMIC_INFUSER,          false, false },
    LegendaryWeaponPriority{ TkNpcs::NPC_NETHERSTRAND_LONGBOW,    false, false },
    LegendaryWeaponPriority{ TkNpcs::NPC_DEVASTATION,             true,  true  },
    LegendaryWeaponPriority{ TkNpcs::NPC_INFINITY_BLADES,         false, false },
    LegendaryWeaponPriority{ TkNpcs::NPC_WARP_SLICER,             false, false },
    LegendaryWeaponPriority{ TkNpcs::NPC_PHASESHIFT_BULWARK,      false, false },
};

uint32 GetKaelthasTkPhase(Unit* kaelthas);
bool IsAdvisorActive(Unit* advisor);
KaelthasAdvisorDpsHold GetKaelthasAdvisorDpsHold(PlayerbotAI* botAI, Unit* kaelthas);
bool IsKaelthasAdvisorDpsHoldActive(PlayerbotAI* botAI, Unit* kaelthas);
bool IsKaelthasAdvisorPetHoldActive(PlayerbotAI* botAI, Unit* kaelthas);
Unit* GetKaelthasWeaponPetTarget(PlayerbotAI* botAI, Creature* pet);
bool UpdateTkClassRoles(PlayerbotAI* botAI);
bool UpdateKaelthasClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetCapernianTank(Player* bot);
bool IsCapernianTank(Player* bot);
bool IsSanguinarDebuffHunter(Player* bot);
GuidVector FindPhoenixGuids(Player* bot);
GuidVector FindLegendaryWeaponGuids(Player* bot);
Unit* GetLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry);
Creature* GetDeadLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry);
bool HasDeadLegendaryWeapon(PlayerbotAI* botAI);
bool IsLegendaryWeaponItem(uint32 itemId);
bool HasEquippableItemForSlot(Player* bot, uint8 slot);
Item* GetEquippedItemInSlot(Player* bot, uint8 slot, uint32 itemId);
GuidVector FindFlameStrikeGuids(Player* bot);
Creature* GetNearestFlameStrikeInRadius(Player* bot, float radius);
GuidVector FindPhoenixEggGuids(Player* bot);
Creature* GetPhoenixEgg(Player* bot);
std::vector<Unit*> GetActivePhoenixes(PlayerbotAI* botAI);

// Shared encounter state

struct TkInstanceState
{
    std::optional<bool> lastRebirthState;
    std::optional<bool> isAlarInPhase2;
    EncounterHelpers::InstanceCreatureList alarEmbers;
    EncounterHelpers::InstanceCreatureList alarFlamePatches;
    std::vector<ArcaneOrbData> voidReaverArcaneOrbs;
    EncounterHelpers::InstanceCreatureList solarianAdds;
    std::optional<uint32> advisorDpsHoldTimer;
    EncounterHelpers::InstanceCreatureList kaelthasPhoenixes;
    EncounterHelpers::InstanceCreatureList kaelthasLegendaryWeapons;
    EncounterHelpers::InstanceCreatureList kaelthasFlameStrikes;
    EncounterHelpers::InstanceCreatureList kaelthasPhoenixEggs;
    EncounterHelpers::ClassRoleAssignment capernianTank;
    EncounterHelpers::ClassRoleAssignment sanguinarDebuffHunter;
};

TkInstanceState& TkState(uint32 instanceId);
TkInstanceState* FindTkState(uint32 instanceId);
bool TkResetInstance(uint32 instanceId);

}

#endif
