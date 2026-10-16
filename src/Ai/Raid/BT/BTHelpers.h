/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTHELPERS_H
#define PLAYERBOTS_BTHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Creature;
class GameObject;
class Pet;
class Player;
class PlayerbotAI;
class SpellInfo;
class Unit;

namespace BtHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class BtSpells : uint32
{
    // Trash
    SPELL_SHARED_BONDS              = 41363,
    SPELL_SPELL_ABSORPTION          = 41034,

    // High Warlord Naj'entus
    SPELL_IMPALING_SPINE            = 39837,
    SPELL_TIDAL_SHIELD              = 39872,

    // Supremus
    SPELL_SNARE_SELF                = 41922,
    SPELL_VOLCANIC_GEYSER           = 42055,
    SPELL_MOLTEN_FLAME_PATCH        = 40253, // the patches' dynamic objects

    // Teron Gorefiend
    SPELL_SHADOW_OF_DEATH           = 40251,
    SPELL_SPIRITUAL_VENGEANCE       = 40268,

    SPELL_SPIRIT_LANCE              = 40157,
    SPELL_SPIRIT_CHAINS             = 40175,
    SPELL_SPIRIT_VOLLEY             = 40314,
    SPELL_SPIRIT_STRIKE             = 40325,
    SPELL_SPIRIT_SHIELD             = 40322,

    // Gurtogg Bloodboil
    SPELL_BOSS_FEL_RAGE             = 40594, // Is dropped for the final 2s of phase 2
    SPELL_PLAYER_FEL_RAGE           = 40604, // Lasts the entirety of phase 2 (30s)
    SPELL_INSIGNIFICANCE            = 40618,
    SPELL_BLOODBOIL                 = 42005,

    // Reliquary of Souls
    SPELL_DEADEN                    = 41410,
    SPELL_RUNE_SHIELD               = 41431,

    // Mother Shahraz
    SPELL_FATAL_ATTRACTION          = 41001,

    // Gathios the Shatterer
    SPELL_BLESSING_OF_PROTECTION    = 41450,
    SPELL_BLESSING_OF_SPELL_WARDING = 41451,
    SPELL_JUDGEMENT                 = 41467,
    SPELL_SEAL_OF_COMMAND           = 41469,
    SPELL_CONSECRATION              = 41541,

    // Lady Malande
    SPELL_CIRCLE_OF_HEALING         = 41455,

    // Veras Darkshadow
    SPELL_VERAS_VANISH              = 41476,

    // High Nethermancer Zerevor
    SPELL_DAMPEN_MAGIC              = 41478,
    SPELL_FLAMESTRIKE               = 41481,
    SPELL_BLIZZARD                  = 41482,

    // Illidan Stormrage <The Betrayer>
    SPELL_DEMON_TRANSFORM_1         = 40511,
    SPELL_DEMON_TRANSFORM_2         = 40398,
    SPELL_DEMON_TRANSFORM_3         = 40510,
    SPELL_DEMON_FORM                = 40506,
    SPELL_DARK_BARRAGE              = 40585,
    SPELL_AGONIZING_FLAMES          = 40932,
    SPELL_PARASITIC_SHADOWFIEND_1   = 41917, // cast by Illidan (primary infection)
    SPELL_PARASITIC_SHADOWFIEND_2   = 41914, // cast by Shadowfiend on contact (secondary infection)
    SPELL_PARALYZE                  = 41083, // cast by a Shadow Demon on the unit it walks to

    // Druid
    SPELL_TREE_OF_LIFE              = 33891,

    // Hunter
    SPELL_FEIGN_DEATH               =  5384,
    SPELL_FROST_TRAP                = 13809,
    SPELL_FROST_TRAP_AURA           = 13810, // the slowing patch, laid once the trap is sprung
    SPELL_MISDIRECTION              = 34477,
    SPELL_MISDIRECTION_AURA         = 35079, // the aura on the hunter

    // Mage
    SPELL_COUNTERSPELL              =  2139,
    SPELL_SPELLSTEAL                = 30449,

    // Shaman
    SPELL_WIND_SHEAR                = 57994,

    // Warlock (Felhunter)
    SPELL_SPELL_LOCK_1              = 19244,
    SPELL_SPELL_LOCK_2              = 19647,

    // Warrior
    SPELL_SPELL_REFLECTION          = 23920,
    SPELL_VIGILANCE                 = 50720,
};

enum class BtNpcs : uint32
{
    // Trash
    NPC_SISTER_OF_PAIN              = 22956,
    NPC_SISTER_OF_PLEASURE          = 22964,
    NPC_SHADOWMOON_REAVER           = 22879,

    // Supremus
    NPC_SUPREMUS_VOLCANO            = 23085,

    // Shade of Akama
    NPC_ASHTONGUE_CHANNELER         = 23421,
    NPC_ASHTONGUE_SORCERER          = 23215,

    // Teron Gorefiend
    NPC_SHADOWY_CONSTRUCT           = 23111,

    // Reliquary of Souls
    NPC_ESSENCE_OF_DESIRE           = 23419,

    // Illidari Council
    NPC_GATHIOS_THE_SHATTERER       = 22949,
    NPC_HIGH_NETHERMANCER_ZEREVOR   = 22950,
    NPC_LADY_MALANDE                = 22951,

    // Illidan Stormrage <The Betrayer>
    NPC_ILLIDAN_STORMRAGE           = 22917,
    NPC_FLAME_OF_AZZINOTH           = 22997,
    NPC_DEMON_FIRE                  = 23069,
    NPC_ILLIDAN_DB_TARGET           = 23070,
    NPC_BLAZE                       = 23259,
    NPC_FLAME_CRASH                 = 23336,
    NPC_SHADOW_DEMON                = 23375,
    NPC_PARASITIC_SHADOWFIEND       = 23498,
};

enum class BtItems : uint32
{
    // High Warlord Naj'entus
    ITEM_NAJENTUS_SPINE             = 32408,
};

enum class BtObjects : uint32
{
    // High Warlord Naj'entus
    GO_NAJENTUS_SPINE               = 185584,

    // Illidan Stormrage <The Betrayer>
    GO_CAGE_TRAP                    = 185916,
};

inline constexpr uint32 BT_MAP_ID = 564;

// Misdirects onto the tank, then spends it with Steady Shot on the target.
bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);
// A rectangle around centre whose depth runs toward facing and whose width runs across it.
bool IsInRectangle(
    Position const& point, Position const& centre, Position const& facing, float halfWidth,
    float halfDepth);
// This bot's own point in that rectangle, the same every call and spread between bots, at the
// bot's Z. Falls back to centre when the way from centre to the point is blocked.
Position GetBotPointInRectangle(
    Player* bot, Position const& centre, Position const& facing, float halfWidth, float halfDepth);

// Trash

// Where a spell would build Chaotic Charge on a Shadowmoon Reaver: on its target, or on anyone in
// its area.
enum class ChaoticChargeReach : uint8
{
    None,
    Target,
    Area,
};

// Spell Absorption lasts 15 s and is recast 30 to 40 s after the last one began. Magic is held
// from the margin before the earliest recast, for casts and channels still landing.
inline constexpr uint32 REAVER_ABSORPTION_DURATION_MS = 15 * IN_MILLISECONDS;
inline constexpr uint32 REAVER_ABSORPTION_MIN_RECAST_MS = 30 * IN_MILLISECONDS;
inline constexpr uint32 REAVER_MAGIC_MARGIN_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 SHADOWMOON_REAVER_CACHE_INTERVAL_MS = 1000;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>>
    shadowmoonReaverAbsorptionStart;

// Searched by entry around the marking bot, so it finds her without line of sight.
inline constexpr float SISTER_OF_PLEASURE_SEARCH_RADIUS = 100.0f;

// A living Sister of Pleasure carrying Shared Bonds from a living Sister of Pain.
bool IsLinkedSisterOfPleasure(Unit* unit);
Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI);
GuidVector FindShadowmoonReaverGuids(PlayerbotAI* botAI);
// True for a Shadowmoon Reaver while magic would build Chaotic Charge on her: from the start of
// Spell Absorption until it ends, from the margin before her earliest recast, and before her
// first one.
bool IsShadowmoonReaverUnsafeForMagic(Unit* unit);
bool IsAnyShadowmoonReaverUnsafeForMagic(PlayerbotAI* botAI);
ChaoticChargeReach GetChaoticChargeReach(SpellInfo const* spellInfo);
// Imps, water elementals, succubi and felhunters, whose attacks build Chaotic Charge.
bool IsChargeBuildingPet(Unit* unit);
// Where to send such a pet instead of a Reaver: the owner's target, else any other attacker.
Unit* FindPetTargetOtherThanReaver(PlayerbotAI* botAI);

// High Warlord Naj'entus

struct NajentusSpineAssignment
{
    ObjectGuid impaled;
    ObjectGuid remover;
};

inline constexpr float NAJENTUS_RANGED_DISTANCE_FROM_BOSS = 10.0f;
// Needle Spine Explosion hits allies within 6 yd of each player struck.
inline constexpr float NAJENTUS_RANGED_SPREAD_DISTANCE = 7.0f;
// Hurl Spine (39948) range, counted beyond both combat reaches as Spell::CheckRange does.
inline constexpr float NAJENTUS_HURL_SPINE_RANGE = 25.0f;
// How far inside that range a thrower walking in stops.
inline constexpr float NAJENTUS_HURL_SPINE_APPROACH_MARGIN = 2.0f;

inline Position const NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

// Impales can overlap (every 20 s, 30 s stun), so one entry per impaled player.
extern std::unordered_map<uint32, std::vector<NajentusSpineAssignment>> najentusSpineAssignments;
extern std::unordered_map<uint32, ObjectGuid> najentusSpineThrower;

bool IsNajentusImpaled(Player* player);
// An impaled group member with no living remover assigned, for the mechanic tracker to assign.
Player* FindNajentusUnassignedImpaledPlayer(Player* bot);
// The nearest living non-tank bot to the impaled player that isn't impaled or already a remover.
Player* FindNajentusSpineRemover(Player* bot, Player* impaled);
// The impaled player this bot was assigned to free, while still impaled.
Player* GetNajentusImpaledPlayerToFree(Player* bot);
bool IsNajentusSpineThrower(Player* bot);
// The assigned thrower while it can still throw: alive, on the map, not impaled, holding a spine.
Player* GetNajentusSpineThrower(Player* bot);
// The bot holding a spine nearest Naj'entus that can throw it.
Player* FindNajentusSpineThrower(Player* bot, Unit* najentus);

// Supremus

// Ground fire: an erupting volcano or a Molten Flame patch.
struct SupremusHazard
{
    Position position;
    float damageRadius;
    float safeRadius;
};

enum class SupremusHazardZone
{
    Damage,
    Safe,
};

// Avoidance picks spots up to 40 yd out, and a volcano's safe zone reaches 18 yd from its centre.
inline constexpr float SUPREMUS_HAZARD_SEARCH_RADIUS = 60.0f;
inline constexpr uint32 SUPREMUS_HAZARD_CACHE_INTERVAL_MS = 200;
// Tank phase only, so a Molten Flame trail chasing one ranged bot doesn't run through the rest.
inline constexpr float SUPREMUS_RANGED_SPREAD_DISTANCE = 6.0f;
// Volcanic Geyser hits within 15 yd of the volcano's centre. Reach gives way to fire, so the 3 yd
// buffer is for movers that don't check it: a tick or two of running before avoidance turns back.
inline constexpr float SUPREMUS_VOLCANO_HAZARD_RADIUS = 15.0f;
inline constexpr float SUPREMUS_VOLCANO_SAFE_DISTANCE = SUPREMUS_VOLCANO_HAZARD_RADIUS + 3.0f;
// A Molten Flame patch hits within 5 yd plus the victim's combat reach, 6.5 yd for a player.
inline constexpr float SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS = 6.5f;
inline constexpr float SUPREMUS_MOLTEN_FLAME_SAFE_DISTANCE =
    SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS + 1.0f;
inline constexpr int64 SUPREMUS_FIXATE_INTERVAL_SECONDS = 10;
// Kept beyond his melee range, which is about 27 yd from his centre: his combat reach is 24.
inline constexpr float SUPREMUS_KITE_SAFETY_MARGIN = 3.0f;
inline constexpr float SUPREMUS_KITE_STEP_DISTANCE = 3.5f;
inline constexpr uint8 SUPREMUS_KITE_HEADINGS = 16;
inline constexpr float SUPREMUS_REACH_STEP_DISTANCE = 3.5f;
// The Y boundaries run from the top of the ramp leading up into the temple to just before reaching
// the vestibule inside the courtyard gate. The X boundaries are pretty close to spanning the entire
// width of the courtyard
inline constexpr float SUPREMUS_BOUNDARY_MIN_X = 587.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_X = 820.0f;
inline constexpr float SUPREMUS_BOUNDARY_MIN_Y = 590.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_Y = 970.0f;

bool IsSupremusKitePhase(Unit* supremus);
// Fixates fall every 10s from the start of the kite phase. Snare Self's apply time is in whole
// seconds, so this can run up to 1s long.
float GetSupremusFixateTimeRemaining(Unit* supremus);
float GetSupremusCatchDistance(Player* bot, Unit* supremus);
// Whether he would reach the bot before the fixate ends if it stood still.
bool CanSupremusCatchStandingBot(Player* bot, Unit* supremus);
// Erupting from its 1s cast until Volcanic Geyser ends, about 19s of its full 30s duration.
bool IsSupremusVolcanoErupting(Unit* volcano);
// Molten Flame only in the kite phase; in the tank phase stock avoid aoe handles it.
std::vector<SupremusHazard> FindSupremusHazards(PlayerbotAI* botAI);
std::vector<SupremusHazard> const& GetSupremusHazards(PlayerbotAI* botAI);
bool IsInSupremusHazard(
    std::vector<SupremusHazard> const& hazards, float x, float y, SupremusHazardZone zone,
    float margin = 0.0f);
// Yards of the straight line inside the hazards' zones, summed over hazards.
float GetLineLengthInSupremusHazards(
    std::vector<SupremusHazard> const& hazards, Position const& from, Position const& to,
    SupremusHazardZone zone);
// The target and range stock reach would use; true when its straight walk into range passes
// through a hazard's safe zone.
bool GetSupremusReachBlockedByFire(PlayerbotAI* botAI, Unit*& target, float& range);

// Shade of Akama

// His instance boundary, so the add search runs only in his room.
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_X = 406.8f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_X = 564.0f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_Y = 327.9f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_Y = 473.5f;
inline constexpr float SHADE_OF_AKAMA_ADD_SEARCH_RADIUS = 80.0f;
inline constexpr uint32 SHADE_OF_AKAMA_ADD_CACHE_INTERVAL_MS = 1000;
inline constexpr float PATH_STEP_DISTANCE = 3.5f;

// Living, attackable channelers by GUID, then sorcerers by GUID: the kill order.
GuidVector FindShadeOfAkamaAddGuids(PlayerbotAI* botAI);
Unit* GetShadeOfAkamaKillTarget(PlayerbotAI* botAI);
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY);
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);

// Teron Gorefiend

// The run from the balcony to the corner takes about 12.5 s.
inline constexpr int32 GOREFIEND_SHADOW_OF_DEATH_MOVE_MS = 15 * IN_MILLISECONDS;
inline constexpr float GOREFIEND_POSITION_TOLERANCE = 2.0f;
// Spirit Chains and Spirit Volley hit within 12 yd of the spirit.
inline constexpr float GOREFIEND_SPIRIT_AOE_DISTANCE = 10.0f;
// Spirit Lance's lowest damage. Constructs within one Lance of the highest health count as even.
inline constexpr uint32 GOREFIEND_SPIRIT_LANCE_MIN_DAMAGE = 6175;
inline constexpr uint32 GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS = 1000;
// From his tank spot, about 90 yd reaches the far corners of his room.
inline constexpr float GOREFIEND_CONSTRUCT_SEARCH_RADIUS = 100.0f;

inline Position const GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f };
inline Position const GOREFIEND_DIE_POSITION  = { 525.709f, 377.177f, 193.203f };

GuidVector FindShadowyConstructGuids(PlayerbotAI* botAI);
// A triggered cast records neither its cooldown nor the global cooldown, so both come from the
// spell data. False if the cast fails.
bool CastVengefulSpiritSpell(Unit* spirit, Unit* target, uint32 spellId);

// Gurtogg Bloodboil

// Bloodboil hits the 5 farthest players, so 3 groups of 5 ranged rotate to soak it.
inline constexpr size_t GURTOGG_ROTATION_GROUP_COUNT = 3;
inline constexpr size_t GURTOGG_ROTATION_GROUP_SIZE = 5;
inline constexpr float GURTOGG_POSITION_TOLERANCE = 2.0f;
// Each ranged group spreads over a rectangle facing the tank position.
inline constexpr float GURTOGG_RANGED_HALF_WIDTH = 5.0f;
inline constexpr float GURTOGG_RANGED_HALF_DEPTH = 2.0f;
// Groups swap this long after a Bloodboil lands, not as it lands.
inline constexpr int32 GURTOGG_ROTATION_DELAY_MS = 2 * IN_MILLISECONDS;
// Arcing Smash during Fel Rage reaches about 11.5 yd from him, and he stays within about 8 yd of
// the Fel Rage target, so the cleave can't reach past about 10 yd from that player. Everyone else
// in front of him steps off his line to that player to keep this far away.
inline constexpr float GURTOGG_FEL_RAGE_AVOID_DISTANCE = 15.0f;
inline constexpr float GURTOGG_FEL_RAGE_AVOID_MARGIN = 1.0f;
// Melee without Fel Rage stand this far behind him during it, opposite the Fel Rage target. A bot
// in front first goes to a point this far around to his side, so it doesn't cut across his front.
inline constexpr float GURTOGG_FEL_RAGE_MELEE_DISTANCE = 5.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_TOLERANCE = 2.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_SIDE_ANGLE = 2.0f * static_cast<float>(M_PI) / 3.0f;
// For this long into Fel Rage, bots behind him keep away too.
inline constexpr int32 GURTOGG_FEL_RAGE_EARLY_AVOID_MS = 5 * IN_MILLISECONDS;
// Bewildering Strike hands him to the second tank, so everyone else stays below it.
inline constexpr float GURTOGG_THREAT_HOLD_RATIO = 0.8f;
inline constexpr uint32 GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS = 1000;

inline Position const GURTOGG_TANK_POSITION   = { 735.987f, 272.451f, 63.554f };
inline Position const GURTOGG_RANGED_POSITION = { 762.265f, 277.183f, 63.781f };
inline Position const GURTOGG_SOAKER_POSITION = { 769.348f, 280.116f, 63.780f };

Position const& GetGurtoggBloodboilPosition(Player* bot);
// The player he is fixed on during Fel Rage, or nullptr outside it.
Unit* GetGurtoggFelRageTarget(Unit* gurtogg);
Position GetGurtoggFelRageMeleePosition(Player* bot, Unit* gurtogg, Unit* felRageTarget);
// 0 with fewer than two tanks on his threat list.
float FindGurtoggSecondTankThreat(PlayerbotAI* botAI);

// Reliquary of Souls

// Essence of Suffering fixates the nearest enemy every 5 s, so tanks above this health stand
// nearer to her than other melee and pets (about 10 yd). She backs away from a victim within
// about 1.4 yd and chases one beyond about 10 yd, so the tank distance stays between the two.
inline constexpr float SUFFERING_TANK_MIN_HEALTH_PCT = 25.0f;
inline constexpr float SUFFERING_TANK_DISTANCE = 6.0f;
inline constexpr float SUFFERING_TANK_MAX_DISTANCE = 7.0f;
inline constexpr float SUFFERING_MELEE_DISTANCE = 10.0f;
inline constexpr float SUFFERING_MELEE_MIN_DISTANCE = 9.0f;
inline constexpr float SUFFERING_RANGED_DISTANCE = 15.0f;
// Other dispellers wait this long into each Rune Shield, so a mage steals it when one can.
inline constexpr uint32 RUNE_SHIELD_MAGE_PRIORITY_MS = 2000;

bool IsSufferingFixateTank(Player* bot);
bool IsOutOfSufferingPosition(Player* bot, Unit* suffering);

// Mother Shahraz

inline constexpr float SHAHRAZ_TANK_POSITION_TOLERANCE = 0.5f;
// Wide enough for an off-tank on her victim to take over without melee backing off.
inline constexpr float SHAHRAZ_POSITIONED_DISTANCE = 3.0f;
inline constexpr float SHAHRAZ_OFF_TANK_DISTANCE = 2.0f;
inline constexpr float SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE = 5.0f;

inline Position const SHAHRAZ_TANK_POSITION       = { 960.438f, 178.989f, 192.826f };
inline Position const SHAHRAZ_TRANSITION_POSITION = { 951.327f, 179.550f, 192.550f };
inline Position const SHAHRAZ_RANGED_POSITION     = { 935.267f, 175.459f, 192.821f };

// Illidari Council

inline constexpr float COUNCIL_FLOOR_Z_THRESHOLD = 270.000f;

inline std::array const GATHIOS_TANK_POSITIONS = {
    Position{ 662.977f, 296.246f, 271.688f },
    Position{ 636.238f, 283.719f, 271.629f },
    Position{ 655.571f, 261.377f, 271.687f },
    Position{ 673.789f, 274.139f, 271.689f },
};
inline std::array const ZEREVOR_TANK_POSITIONS = {
    Position{ 654.854f, 352.502f, 271.689f },
    Position{ 644.597f, 338.875f, 271.688f },
    // Position{ 660.325f, 357.568f, 271.688f },
    // Position{ 679.657f, 356.482f, 271.699f },
};

extern std::unordered_map<uint32, uint32> councilDpsWaitTimer;
inline constexpr uint32 ZEREVOR_MAGE_TANK_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 COUNCIL_DPS_WAIT_MS = 5 * IN_MILLISECONDS;
inline constexpr float COUNCIL_AOE_THREAT_CLEARANCE = 15.0f;
inline constexpr float COUNCIL_RANGED_SPREAD_DISTANCE = 4.0f;
inline constexpr float COUNCIL_MEMBER_SEARCH_RADIUS = 100.0f;
// Bots heal out to HealDistance, 38.5 yd by default.
inline constexpr float MAGE_TANK_HEALER_MAX_DISTANCE = 38.0f;
// A patch on the mage tank reaches 11.9 yd from where it stood.
inline constexpr float MAGE_TANK_HEALER_MIN_DISTANCE = 12.5f;
// Zerevor casts Arcane Explosion when anyone is within 10 yd plus both combat reaches (14.65 yd).
inline constexpr float ZEREVOR_ARCANE_EXPLOSION_SAFE_DISTANCE = 15.0f;
// A patch reaches a player whose centre is within its radius plus both object sizes (1.9 yd).
inline constexpr float COUNCIL_PATCH_MARGIN = 2.5f;
inline constexpr int32 CIRCLE_OF_HEALING_REACTION_MS = 200;
// A ranged bot whose own cast would end with less than this left on Circle of Healing drops it.
inline constexpr int32 CIRCLE_OF_HEALING_CASTER_MARGIN_MS = 500;

ObjectGuid FindZerevorMageTankGuid(Player* bot);
Player* GetZerevorMageTank(PlayerbotAI* botAI);
bool IsZerevorMageTank(PlayerbotAI* botAI);
bool HasDangerousCouncilAura(Player* bot);
bool IsVerasVanished(Unit* veras);
bool CanInterruptCircleOfHealing(Unit* malande);
bool IsInZerevorPatch(Unit* zerevor, Position const& point, float margin = 0.0f);
bool IsInCouncilPatch(Position const& point, Unit* gathios, Unit* zerevor, float margin = 0.0f);
bool IsMalandeInCouncilPatch(Unit* malande, Unit* gathios, Unit* zerevor);
bool IsZerevorOnMageTank(PlayerbotAI* botAI, Unit* zerevor);
bool IsMageTankHealerPositionSafe(Position const& point, Player* mageTank, Unit* zerevor);
bool FindMageTankHealerPosition(
    Player* bot, Player* mageTank, Unit* zerevor, Position& destination);
bool FindMalandeTankPosition(
    Player* bot, Unit* malande, Unit* gathios, Unit* zerevor, Position& destination);
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);
Unit* GetCouncilPetTarget(PlayerbotAI* botAI, Creature* pet);
uint32 GetReadySpellLock(Creature* pet, Unit* target);

// Illidan Stormrage <The Betrayer>

// Fight phases 1 to 5 are Initial, Flying, Grounded, Demon and Maiev; Landing runs from the last
// Flame's death until he is back on the ground. Dying is during the 1 HP death scene.
enum class IllidanPhase : uint8
{
    None, // also Maiev's arrival
    Initial,
    Flying,
    Landing,
    Grounded,
    Demon,
    Maiev,
    Dying,
};

// Damage on him is held after each threat reset, so the tank that should have him acts first.
enum class IllidanDpsHold : uint8
{
    None,
    AfterReset,      // Phase 1 and Phase 3 starts, and Phase 5 straight from Demon Form;
                     // everyone but the Main Tank
    DemonFormStart,  // everyone but the warlock tank
    DemonFormMiddle, // from the start hold to the end hold; the Main Tank only
    DemonFormEnd,    // the last 3 s before the reset; everyone
    DemonFormNoPush, // 33% or below in the form, so he leaves it above 30%; everyone
};

struct EyeBlastDangerArea
{
    Position start;
    Position end;
    float width;
};

// Phase 4 places, from where Demon Form roots him and the line from him through the grate
// centre. Ranged DPS slots are in spell reach of him and healer slots behind them, each filled
// from the warlock's side.
struct DemonFormLayout
{
    Position warlockSpot;
    Position mainTankWaitSpot;
    Position pickupSpot;
    std::vector<Position> rangedSlots;
    std::vector<Position> healerSlots;
    std::vector<Position> meleeSlots;
};

// A Shadow Demon and the unit carrying its Paralyze, the one it walks to; target is empty once
// that unit is dead, which in AC leaves the Demon idle.
struct ShadowDemon
{
    ObjectGuid demon = ObjectGuid::Empty;
    ObjectGuid target = ObjectGuid::Empty;
};

// One Shadowfiend infection, recorded by the mechanic tracker. The run spot is fixed for the phase
// it was recorded in. Once the trap is assigned, hunter is empty if a Frost Trap patch already
// covered the spot.
struct IllidanInfection
{
    Position runSpot;
    IllidanPhase phase = IllidanPhase::None;
    ObjectGuid hunter = ObjectGuid::Empty;
    bool isTrapAssigned = false;
};

inline constexpr uint32 ILLIDAN_WARLOCK_TANK_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 ILLIDAN_TRAP_HUNTER_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 ILLIDAN_SPEECH_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 SHADOW_DEMON_CACHE_INTERVAL_MS = 200;
// From any bot on the summit, about 90 yd reaches the far edge.
inline constexpr float ILLIDAN_ADD_SEARCH_RADIUS = 100.0f;
// From his spot this takes in Akama's, 81 yd away, where the speech is started.
inline constexpr float ILLIDAN_SPEECH_SEARCH_RADIUS = 100.0f;
// A bot this close to its place is in it.
inline constexpr float ILLIDAN_SPOT_TOLERANCE = 1.0f;
// A bot without the Shadowfiend debuff this close to a Shadowfiend is where a debuff just ended.
inline constexpr float ILLIDAN_SHADOWFIEND_BIRTHER_DISTANCE = 8.0f;
// Below this a Shadowfiend is finished off: melee take one in their melee range, and its birther
// acts again.
inline constexpr float ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT = 50.0f;
// After landing, everyone else leaves the landing stack once he is led this far from the grate
// centre, or 3 s after the threat reset, whichever comes first.
inline constexpr float ILLIDAN_LANDING_RELEASE_DISTANCE = 15.0f;
// At or below this he is burnt down: no hold and no stop.
inline constexpr float ILLIDAN_BURN_HEALTH_PCT = 5.0f;
// At or below this in Demon Form nobody damages him, so he changes back above 30%.
inline constexpr float ILLIDAN_DEMON_FORM_NO_PUSH_HEALTH_PCT = 33.0f;
inline constexpr uint32 ILLIDAN_RESET_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
// The warlock tank has him to itself for the 12.23 s transform and about 2 s more.
inline constexpr uint32 ILLIDAN_DEMON_FORM_DPS_WAIT_MS = 14 * IN_MILLISECONDS;
// Demon Form ends, with a threat reset, this long after the transform starts.
inline constexpr uint32 ILLIDAN_DEMON_FORM_DURATION_MS = 72230;
// Everyone stops damaging him this long before the end.
inline constexpr uint32 ILLIDAN_DEMON_FORM_END_HOLD_MS = 3 * IN_MILLISECONDS;
// The Main Tank's walk-in, about 44 yd, starts this long before the end. No Flame Burst comes in
// the last 14 s.
inline constexpr uint32 ILLIDAN_MAIN_TANK_WALK_IN_MS = 10 * IN_MILLISECONDS;
// Shadow Demons appear this long after the transform starts, 30 s after he turns aggressive.
inline constexpr uint32 ILLIDAN_SHADOW_DEMON_SPAWN_MS = 42230;
// Warlocks start Seed of Corruption (2 s cast) from this long before the Demons appear until this
// long after, so its 15 yd blast catches them in the 2.6 s they spend near him.
inline constexpr uint32 ILLIDAN_SEED_LEAD_MS = 1 * IN_MILLISECONDS;
inline constexpr uint32 ILLIDAN_SEED_LATE_MS = 500;
inline constexpr uint32 ILLIDAN_LANDING_WAIT_MS = 3 * IN_MILLISECONDS;
// Hunters misdirect onto the Main Tank this long from each Phase 1, 3 and 5 start.
inline constexpr uint32 ILLIDAN_PHASE_START_MISDIRECT_MS = 3 * IN_MILLISECONDS;
// A Frost Trap patch already on the run spot does for a new infection if it outlasts the debuff
// by this much, the time ranged get on the slowed Shadowfiends.
inline constexpr int32 ILLIDAN_FROST_TRAP_SLOW_MS = 5 * IN_MILLISECONDS;

// Tank ring: he is held 25 yd from the grate centre with the Main Tank 6 yd outward of him, so he
// faces out and the raid stands inside.
inline constexpr float ILLIDAN_RING_RADIUS = 25.0f;
inline constexpr float ILLIDAN_TANK_OFFSET = 6.0f;
// On each Flame Crash the Main Tank walks to the ring point this far counterclockwise of him. He
// runs up close behind a walking tank and stops about 30 deg on, back on the ring.
inline constexpr float ILLIDAN_RING_LEAD_ANGLE = 35.0f * static_cast<float>(M_PI) / 180.0f;
// While he is this close to the grate centre, as after landing, the Main Tank leads him straight
// out along its own bearing to the lead radius; he stops 22 to 28 yd out.
inline constexpr float ILLIDAN_PICKUP_DISTANCE = 15.0f;
inline constexpr float ILLIDAN_PICKUP_LEAD_RADIUS = 28.0f;
// A Flame Crash this close to the Main Tank was cast on it.
inline constexpr float ILLIDAN_FLAME_CRASH_ON_TANK_DISTANCE = 9.0f;

// Phases 3 and 5: the centre, six at 6 yd and twelve at 12 yd, in that fill order.
inline constexpr size_t ILLIDAN_RANGED_SLOT_COUNT = 19;
inline constexpr float ILLIDAN_GRATE_INNER_RADIUS = 6.0f;
inline constexpr float ILLIDAN_GRATE_OUTER_RADIUS = 12.0f;
inline constexpr size_t ILLIDAN_HEALER_SLOT_COUNT = 8;
inline constexpr size_t ILLIDAN_MELEE_SLOT_COUNT = 15;

// Shadowfiend run spots. Phases 1, 3 and 5: from the grate centre, counterclockwise of his
// bearing. Phase 4: from him, behind the ranged DPS, beside the healers and in front of the melee.
inline constexpr float ILLIDAN_RUN_SPOT_DISTANCE = 27.0f;
inline constexpr float ILLIDAN_RUN_SPOT_ANGLE = 90.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ILLIDAN_DEMON_RUN_SPOT_DISTANCE = 39.6f;
inline constexpr float ILLIDAN_DEMON_RUN_SPOT_ANGLE = -16.0f * static_cast<float>(M_PI) / 180.0f;

// Phase 4, from him and off his inward line, positive toward the warlock tank's side. His reach is
// 1.5 in Demon Form, so a 30 yd spell reaches 33 yd on him: the warlock tank stands 2 yd inside.
inline constexpr float ILLIDAN_WARLOCK_SPOT_DISTANCE = 31.0f;
inline constexpr float ILLIDAN_WARLOCK_SPOT_ANGLE = 75.0f * static_cast<float>(M_PI) / 180.0f;
// The Main Tank waits beside the melee, on the first melee row one slot past a group of seven
// (5 melee DPS and 2 tanks) on the side away from the warlock tank, so the raid can kill a Shadow
// Demon walking to it.
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_DISTANCE = 48.4f;
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_ANGLE = -35.0f * static_cast<float>(M_PI) / 180.0f;
// At the end it walks straight to the pickup spot in front of the raid, just outside Aura of Dread
// and 22.5 yd from the warlock tank (Shadow Blast 20). He keeps the warlock tank as his victim
// through the reset and runs at it, so the Main Tank takes him with a ranged opener.
inline constexpr float ILLIDAN_PICKUP_SPOT_DISTANCE = 17.0f;
inline constexpr float ILLIDAN_PICKUP_SPOT_ANGLE = 30.0f * static_cast<float>(M_PI) / 180.0f;

// World trigger 22515.
inline Position const ILLIDAN_GRATE_CENTRE = { 676.021f, 305.455f, 353.582f };
// He kneels here and stands here for his speech.
inline Position const ILLIDAN_SPEECH_POSITION = { 705.732f, 304.988f, 353.925f };
// The Main Tank waits this far north of the centre at the pull, so he stops on the ring.
inline Position const ILLIDAN_PULL_TANK_POSITION = { 691.521f, 305.455f, 353.790f };
// While he lands, the Main Tank waits 6 yd north of the landing point and everyone else 22 yd
// south of the centre.
inline Position const ILLIDAN_LANDING_TANK_POSITION = { 682.648f, 304.761f, 353.920f };
inline Position const ILLIDAN_LANDING_STACK_POSITION = { 654.021f, 305.455f, 353.790f };
inline Position const ILLIDAN_LANDING_POSITION = { 676.648f, 304.761f, 354.189f };
inline Position const ILLIDAN_N_GRATE_POSITION = { 682.100f, 306.000f, 353.192f };
inline Position const ILLIDAN_E_GRATE_POSITION = { 673.500f, 298.500f, 353.192f };
inline Position const ILLIDAN_W_GRATE_POSITION = { 672.400f, 312.500f, 353.192f };
inline std::array const GRATE_POSITIONS = {
    ILLIDAN_N_GRATE_POSITION,
    ILLIDAN_E_GRATE_POSITION,
    ILLIDAN_W_GRATE_POSITION,
};

inline Position const ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
inline std::array const E_GLAIVE_TANK_POSITIONS = {
    Position{ 683.000f, 295.000f, 354.000f },
    Position{ 696.969f, 300.982f, 354.302f },
    Position{ 691.112f, 287.461f, 354.363f },
    Position{ 676.674f, 280.797f, 354.268f },
    Position{ 664.414f, 284.834f, 354.271f },
    Position{ 656.826f, 295.113f, 354.165f },
    Position{ 665.000f, 304.000f, 354.000f },
};

inline Position const ILLIDAN_W_GLAIVE_WAITING_POSITION = { 676.102f, 316.305f, 353.192f };
inline std::array const W_GLAIVE_TANK_POSITIONS = {
    Position{ 697.208f, 313.475f, 354.234f },
    Position{ 681.000f, 318.000f, 354.000f },
    Position{ 664.000f, 307.000f, 354.000f },
    Position{ 656.161f, 314.132f, 354.092f },
    Position{ 665.080f, 326.905f, 354.128f },
    Position{ 678.809f, 329.968f, 354.387f },
    Position{ 690.889f, 324.277f, 354.204f },
};

extern std::unordered_map<uint32, IllidanPhase> illidanLastPhase;
// The phase before the current one, so Phase 5 straight from Demon Form shows as a reset.
extern std::unordered_map<uint32, IllidanPhase> illidanPreviousPhase;
extern std::unordered_map<uint32, uint32> illidanPhaseStartTime;
extern std::unordered_map<uint32, uint32> illidanFlamePhaseStartTime;
extern std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
extern std::unordered_map<uint32, ObjectGuid> westFlameGuid;
// Each infected player's infection, kept until the debuff is gone or the phase changes.
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, IllidanInfection>>
    illidanInfections;

IllidanPhase GetIllidanPhase(Unit* illidan);
// He is up and talking after Akama's gossip, still immune to players and out of combat.
bool IsIllidanSpeechInProgress(Player* bot);
// The phase started less than durationMs ago; until the mechanic tracker stamps it, it has just
// started.
bool IsIllidanPhaseStartWithin(Unit const& illidan, uint32 durationMs);
// Everyone but the Main Tank waits at the landing stack while he lands, and then until the Main
// Tank has led him out or 3 s have passed since the threat reset.
bool IsIllidanLandingWait(Unit const& illidan, IllidanPhase phase);
// Demon Form ends, with a threat reset, less than durationMs from now.
bool IsIllidanDemonFormEnding(Unit const& illidan, uint32 durationMs);
// The hold window that is running, before who it spares is decided.
IllidanDpsHold GetIllidanHoldWindow(Unit* illidan);
// The hold this bot is in, for the hold multiplier and any stop block.
IllidanDpsHold GetIllidanDpsHold(PlayerbotAI* botAI, Unit* illidan);
// The game angle of a point seen from the grate centre: 0 north, rising counterclockwise.
float GetGrateAngle(Position const& position);
Position GetRingPoint(float angle, float radius);
// 6 yd outward of him. A tank walking there from within 9.5 yd only turns him to face out.
Position GetIllidanSettleSpot(Unit const& illidan);
Position GetGrateSlot(size_t index);
DemonFormLayout GetDemonFormLayout(Unit const& illidan);
// Where an infection's run spot is recorded: from the grate centre in Phases 1, 3 and 5, from him
// in Demon Form.
Position GetIllidanShadowfiendRunSpot(Unit const& illidan, IllidanPhase phase);
// The run spot recorded for the bot's own infection, else for the infection it traps for, else
// where one would be recorded now.
Position GetIllidanRecordedRunSpot(PlayerbotAI* botAI, Unit const& illidan, IllidanPhase phase);
std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot);
bool IsWestFlameOfAzzinothTank(Player* bot);
bool IsEastFlameOfAzzinothTank(Player* bot);
bool IsFlameOfAzzinothTank(Player* bot);
// The warlock tank is an assistant warlock (player or bot), else the warlock bot with the lowest
// GUID; dead or alive, for the whole fight, since losing it wipes the raid anyway.
ObjectGuid FindIllidanWarlockTankGuid(Player* bot);
// Null while the warlock tank is dead.
Player* GetIllidanWarlockTank(PlayerbotAI* botAI);
bool IsIllidanWarlockTank(PlayerbotAI* botAI);
// The trap hunters in the order they take infections: hunter bots by GUID, dead or alive.
GuidVector FindIllidanTrapHunterGuids(Player* bot);
// Assigned to an infection whose debuff is still on, with Frost Trap ready.
bool IsIllidanTrapHunterOnDuty(PlayerbotAI* botAI);
// A Frost Trap patch round runSpot lasting at least minDurationMs more.
bool IsFrostTrapPatchAt(Player* bot, Position const& runSpot, int32 minDurationMs);
// Slot lists are bots on the map, the dead kept, each part by GUID: ranged are healers, then
// ranged DPS; melee are melee DPS, then tanks. The Main Tank is left out, and in Demon Form the
// warlock tank and the healers too, who have their own list. The index wraps round slotCount; -1
// if the bot isn't in the list.
int32 GetIllidanRangedIndex(PlayerbotAI* botAI, size_t slotCount, bool inDemonForm);
int32 GetIllidanHealerIndex(PlayerbotAI* botAI, size_t slotCount);
int32 GetIllidanMeleeIndex(PlayerbotAI* botAI, size_t slotCount);
bool HasParasiticShadowfiend(Player* player);
// An entry search, since a Shadowfiend is out of combat, and so not in attackers, until it turns
// aggressive 2.4 s after spawning.
GuidVector FindParasiticShadowfiendGuids(PlayerbotAI* botAI);
Unit* GetNearestParasiticShadowfiend(PlayerbotAI* botAI, Unit const& unit);
// No Shadowfiend debuff and a living Shadowfiend close by, as where a debuff just ended, while
// that Shadowfiend isn't yet nearly dead.
bool IsNearNewParasiticShadowfiend(PlayerbotAI* botAI);
// Living group members with either Shadowfiend debuff, humans included, so trap hunters trap for
// them too.
GuidVector FindPlayersWithParasiticShadowfiendGuids(Player* bot);
// An entry search, so it doesn't rely on the Demons being in attackers (unchecked); each Demon's
// target is the group member or pet carrying its Paralyze.
std::vector<ShadowDemon> FindShadowDemons(PlayerbotAI* botAI);
// For ranged and healers: a Demon walking to the warlock tank first, then the one nearest its
// target, ties going to the one nearest `from`; idle Demons last.
Unit* GetShadowDemonToAttack(PlayerbotAI* botAI, Unit const& from);
// For melee: the Demon nearest `from` that has walked out from him and is clear of Shadow Blast
// round his victim, one still walking to a target before an idle one.
Unit* GetShadowDemonForMelee(PlayerbotAI* botAI, Unit const& illidan, Unit const& from);
// Imps and water elementals take Demons as ranged do, other pets as melee do.
Unit* GetShadowDemonForPet(PlayerbotAI* botAI, Pet const& pet);
EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area);
GameObject* FindNearestCageTrap(PlayerbotAI* botAI);

}

#endif
