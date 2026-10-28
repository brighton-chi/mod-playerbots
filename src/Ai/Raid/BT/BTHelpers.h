/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTHELPERS_H
#define PLAYERBOTS_BTHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include "SharedDefines.h"
#include <array>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Aura;
class Creature;
class GameObject;
class Pet;
class Player;
class Spell;
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
    SPELL_VOLCANIC_ERUPTION         = 40276,
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
    SPELL_PARASITIC_SHADOWFIEND_PRI = 41917, // primary infection, cast by Illidan
    SPELL_PARASITIC_SHADOWFIEND_SEC = 41914, // secondary infection, from a Shadowfiend's hit
    SPELL_PARALYZE                  = 41083, // cast by a Shadow Demon on the unit it walks to
    SPELL_SHADOW_PRISON             = 40647, // on every player in Maiev's arrival, 30 s
    SPELL_SHADOW_BLAST              = 41078, // Demon Form, 20 yd round his victim as the cast ends

    // Death Knight
    SPELL_MIND_FREEZE               = 47528,

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

    // Paladin
    SPELL_DIVINE_SHIELD             =   642,

    // Priest
    SPELL_DISPERSION                = 47585,

    // Rogue
    SPELL_KICK                      =  1766,

    // Shaman
    SPELL_TOTEMIC_RECALL            = 36936,
    SPELL_WIND_SHEAR                = 57994,

    // Warlock (Felhunter)
    SPELL_SPELL_LOCK_1              = 19244,
    SPELL_SPELL_LOCK_2              = 19647,

    // Warrior
    SPELL_SHIELD_BASH               =    72,
    SPELL_BERSERKER_STANCE          =  2458,
    SPELL_PUMMEL                    =  6552,
    SPELL_SPELL_REFLECTION          = 23920,
    SPELL_VIGILANCE                 = 50720,
};

enum class BtNpcs : uint32
{
    // Trash
    NPC_SISTER_OF_PAIN              = 22956,
    NPC_SISTER_OF_PLEASURE          = 22964,
    NPC_SHADOWMOON_REAVER           = 22879,

    // High Warlord Naj'entus
    NPC_HIGH_WARLORD_NAJENTUS       = 22887,

    // Supremus
    NPC_SUPREMUS                    = 22898,
    NPC_SUPREMUS_VOLCANO            = 23085,

    // Shade of Akama
    NPC_SHADE_OF_AKAMA              = 22841,
    NPC_ASHTONGUE_CHANNELER         = 23421,
    NPC_ASHTONGUE_SORCERER          = 23215,

    // Teron Gorefiend
    NPC_TERON_GOREFIEND             = 22871,
    NPC_SHADOWY_CONSTRUCT           = 23111,

    // Gurtogg Bloodboil
    NPC_GURTOGG_BLOODBOIL           = 22948,

    // Reliquary of Souls
    NPC_RELIQUARY_OF_THE_LOST       = 22856,
    NPC_ESSENCE_OF_SUFFERING        = 23418,
    NPC_ESSENCE_OF_DESIRE           = 23419,
    NPC_ESSENCE_OF_ANGER            = 23420,

    // Mother Shahraz
    NPC_MOTHER_SHAHRAZ              = 22947,

    // Illidari Council
    NPC_GATHIOS_THE_SHATTERER       = 22949,
    NPC_HIGH_NETHERMANCER_ZEREVOR   = 22950,
    NPC_LADY_MALANDE                = 22951,
    NPC_VERAS_DARKSHADOW            = 22952,

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
    GO_FROST_TRAP                   = 164639, // laid by 13809, despawns once sprung
};

inline constexpr uint32 BT_MAP_ID = 564;
inline constexpr float PATH_STEP_DISTANCE = 3.5f;

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
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY);
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);
// Sends the pet onto the target as a player's attack command does.
bool CommandPetAttack(Creature* pet, Unit* target);
GuidVector FindAttackerGuidsWithEntry(PlayerbotAI* botAI, uint32 entry);
// How long the aura has been up; for an aura with a fixed duration.
uint32 GetAuraElapsedMs(Aura const* aura);
// Turns the bot to the unit unless it is already in the bot's front 90 deg.
void FaceUnit(Player* bot, Unit* unit);

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

// Searched by entry around the marking bot, so it finds her without line of sight.
inline constexpr float SISTER_OF_PLEASURE_SEARCH_RADIUS = 100.0f;

// The living Sister of Pain whose Shared Bonds a living Sister of Pleasure carries; null for any
// other unit.
Unit* GetLinkedSisterOfPain(Unit* pleasure);
Unit* FindLinkedSisterOfPleasure(PlayerbotAI* botAI);
// The Sister of Pain linked to the skulled Sister of Pleasure.
Unit* GetSkulledSisterOfPain(Player* bot);
// True for a Shadowmoon Reaver while magic would build Chaotic Charge on her: from the start of
// Spell Absorption until it ends, from the margin before her earliest recast, and before her
// first one.
bool IsShadowmoonReaverUnsafeForMagic(Unit* unit);
bool IsAnyShadowmoonReaverUnsafeForMagic(PlayerbotAI* botAI);
// Reavers are trash, so this runs on every reset and drops only a dead or vanished Reaver's
// timing; a pack still being fought keeps its own.
bool PruneShadowmoonReaverAbsorptionStarts(Player* bot);
ChaoticChargeReach GetChaoticChargeReach(SpellInfo const* spellInfo);
// Imps, water elementals, succubi and felhunters, whose attacks build Chaotic Charge.
bool IsChargeBuildingPet(Unit* unit);
// Where to send such a pet instead of a Reaver: the owner's target, else any other attacker.
Unit* FindPetTargetOtherThanReaver(PlayerbotAI* botAI);

// High Warlord Naj'entus

// Impales can overlap (every 20 s, 30 s stun), so one entry per impaled player.
struct NajentusSpineAssignment
{
    ObjectGuid impaled;
    ObjectGuid remover;
};

// Needle Spine Explosion hits allies within 6 yd of each player struck.
inline constexpr float NAJENTUS_RANGED_SPREAD_DISTANCE = 7.0f;
// Hurl Spine (39948) range, counted beyond both combat reaches as Spell::CheckRange does.
inline constexpr float NAJENTUS_HURL_SPINE_RANGE = 25.0f;
// How far inside that range a thrower walking in stops.
inline constexpr float NAJENTUS_HURL_SPINE_APPROACH_MARGIN = 2.0f;
// A spine flies up to 1.25 s and breaks the shield on landing, and the throw is a queued packet,
// so no spine is thrown this long after the last one.
inline constexpr uint32 NAJENTUS_HURL_SPINE_FLIGHT_MS = 2 * IN_MILLISECONDS;

inline Position const NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

bool IsNajentusImpaled(Player* player);
// An impaled group member with no living remover assigned, for the mechanic tracker to assign.
Player* FindNajentusUnassignedImpaledPlayer(Player* bot);
// The nearest living non-tank bot to the impaled player that isn't impaled or already a remover.
Player* FindNajentusSpineRemover(Player* bot, Player* impaled);
Player* GetNajentusImpaledPlayerToFree(Player* bot);
bool IsNajentusSpineThrower(Player* bot);
// The assigned thrower while it can still throw: alive, on the map, not impaled, holding a spine.
Player* GetNajentusSpineThrower(Player* bot);
// The bot holding a spine nearest Naj'entus that can throw it.
Player* FindNajentusSpineThrower(Player* bot, Unit* najentus);
bool IsNajentusSpineInFlight(Player* bot);

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

// Avoidance picks positions up to 40 yd out, and a volcano's safe zone reaches 18 yd from its
// center.
inline constexpr float SUPREMUS_HAZARD_SEARCH_RADIUS = 60.0f;
inline constexpr uint32 SUPREMUS_HAZARD_CACHE_INTERVAL_MS = 200;
// Tank phase only, so a Molten Flame trail chasing one ranged bot doesn't run through the rest.
inline constexpr float SUPREMUS_RANGED_SPREAD_DISTANCE = 6.0f;
// Volcanic Geyser hits within 15 yd of the volcano's centre. The extra 3 yd covers movers that
// don't check for fire, for the tick or two before avoidance turns them back.
inline constexpr float SUPREMUS_VOLCANO_HAZARD_RADIUS = 15.0f;
inline constexpr float SUPREMUS_VOLCANO_SAFE_DISTANCE = SUPREMUS_VOLCANO_HAZARD_RADIUS + 3.0f;
// A Molten Flame patch hits within 5 yd plus the victim's combat reach, 6.5 yd for a player.
inline constexpr float SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS = 6.5f;
inline constexpr float SUPREMUS_MOLTEN_FLAME_SAFE_DISTANCE =
    SUPREMUS_MOLTEN_FLAME_HAZARD_RADIUS + 1.0f;
// He fixates a new target every 10 s from the start of the kite phase.
inline constexpr int64 SUPREMUS_FIXATE_INTERVAL_SECONDS = 10;
// Kept beyond his melee range, which is about 27 yd from his centre: his combat reach is 24.
inline constexpr float SUPREMUS_KITE_SAFETY_MARGIN = 3.0f;
inline constexpr float SUPREMUS_KITE_STEP_DISTANCE = 3.5f;
inline constexpr uint8 SUPREMUS_KITE_HEADINGS = 16;
// A step between two points outside a hazard's safe zone cuts at most 0.25 yd into it, well short
// of the damage zone.
inline constexpr float SUPREMUS_REACH_STEP_DISTANCE = 3.5f;
// The Y boundaries run from the top of the ramp leading up into the temple to just before reaching
// the vestibule inside the courtyard gate. The X boundaries are pretty close to spanning the entire
// width of the courtyard
inline constexpr float SUPREMUS_BOUNDARY_MIN_X = 587.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_X = 820.0f;
inline constexpr float SUPREMUS_BOUNDARY_MIN_Y = 590.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_Y = 970.0f;

bool IsSupremusKitePhase(Unit* supremus);
float GetSupremusFixateTimeRemaining(Unit* supremus);
float GetSupremusCatchDistance(Player* bot, Unit* supremus);
bool CanSupremusCatchStandingBot(Player* bot, Unit* supremus);
// Erupting while Volcanic Geyser lasts: the first 18s of its 30s, from the moment it appears.
bool IsSupremusVolcanoErupting(Unit* volcano);
// Molten Flame only in the kite phase; in the tank phase stock avoid aoe handles it.
std::vector<SupremusHazard> FindSupremusHazards(PlayerbotAI* botAI);
std::vector<SupremusHazard> const& GetSupremusHazards(PlayerbotAI* botAI);
bool IsInSupremusHazard(
    std::vector<SupremusHazard> const& hazards, float x, float y, SupremusHazardZone zone,
    float margin = 0.0f);
float GetLineLengthInSupremusHazards(
    std::vector<SupremusHazard> const& hazards, Position const& from, Position const& to,
    SupremusHazardZone zone);
// With Divine Shield or Dispersion a bot may walk through fire, but not stop in it.
bool CanWalkThroughSupremusFire(Player* bot);
// Whether the straight walk to reach target, stopping at range, crosses a hazard's safe zone.
bool IsSupremusReachBlockedByFire(PlayerbotAI* botAI, Unit const* target, float range);
// The target of the reach fire blocks, for the bot to walk around to: the party member to heal for a
// healer, else its current target. A healer routes no attack reach.
Unit* GetSupremusFireBlockedReachTarget(PlayerbotAI* botAI);

// Shade of Akama

// His instance boundary, so the add search runs only in his room.
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_X = 406.8f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_X = 564.0f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MIN_Y = 327.9f;
inline constexpr float SHADE_OF_AKAMA_BOUNDARY_MAX_Y = 473.5f;
// Reaches every channeler from anywhere in his room, so every bot gets the same kill order.
inline constexpr float SHADE_OF_AKAMA_ADD_SEARCH_RADIUS = 150.0f;
inline constexpr uint32 SHADE_OF_AKAMA_ADD_CACHE_INTERVAL_MS = 1000;

// Living, attackable channelers by GUID, then sorcerers by GUID: the kill order.
GuidVector FindShadeOfAkamaAddGuids(PlayerbotAI* botAI);
Unit* GetShadeOfAkamaKillTarget(PlayerbotAI* botAI);

// Teron Gorefiend

// The run from the balcony to the corner takes about 12.5 s.
inline constexpr int32 GOREFIEND_SHADOW_OF_DEATH_MOVE_MS = 15 * IN_MILLISECONDS;
inline constexpr float GOREFIEND_POSITION_TOLERANCE = 2.0f;
// Spirit Chains and Spirit Volley hit within 12 yd of the spirit.
inline constexpr float GOREFIEND_SPIRIT_AOE_DISTANCE = 10.0f;
// Spirit Lance's lowest damage. Constructs within one Lance of the highest health count as even.
inline constexpr uint32 GOREFIEND_SPIRIT_LANCE_MIN_DAMAGE = 6175;
inline constexpr uint32 GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS = 1000;
// From his tank position, about 90 yd reaches the far corners of his room.
inline constexpr float GOREFIEND_CONSTRUCT_SEARCH_RADIUS = 100.0f;

inline Position const GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f };
inline Position const GOREFIEND_DIE_POSITION  = { 525.709f, 377.177f, 193.203f };

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
inline constexpr uint32 GURTOGG_ROTATION_DELAY_MS = 2 * IN_MILLISECONDS;
// Arcing Smash during Fel Rage reaches 11.5 yd from him, and he stays within about 14 yd of the
// Fel Rage target, so the cleave can't reach past about 14 yd from that player. Everyone else in
// front of him steps off his line to that player to keep this far away.
inline constexpr float GURTOGG_FEL_RAGE_AVOID_DISTANCE = 15.0f;
inline constexpr float GURTOGG_FEL_RAGE_AVOID_MARGIN = 1.0f;
// Melee without Fel Rage stand this far behind him during it, opposite the Fel Rage target. A bot
// in front first goes to a point this far around to his side, so it doesn't cut across his front.
inline constexpr float GURTOGG_FEL_RAGE_MELEE_DISTANCE = 5.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_TOLERANCE = 2.0f;
inline constexpr float GURTOGG_FEL_RAGE_MELEE_SIDE_ANGLE = 2.0f * static_cast<float>(M_PI) / 3.0f;
// For this long into Fel Rage, bots behind him keep away too.
inline constexpr uint32 GURTOGG_FEL_RAGE_EARLY_AVOID_MS = 5 * IN_MILLISECONDS;
// Bewildering Strike hands him to the second tank, so everyone else stays below it.
inline constexpr float GURTOGG_THREAT_HOLD_RATIO = 0.8f;
inline constexpr uint32 GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS = 1000;

inline Position const GURTOGG_TANK_POSITION   = { 735.987f, 272.451f, 63.554f };
inline Position const GURTOGG_RANGED_POSITION = { 762.265f, 277.183f, 63.781f };
inline Position const GURTOGG_SOAKER_POSITION = { 769.348f, 280.116f, 63.780f };

Position const& GetGurtoggBloodboilPosition(Player* bot);
// The Fel Rage target taunts him onto itself, so it is his victim; null outside Fel Rage.
Unit* GetGurtoggFelRageTarget(Unit* gurtogg);
Position GetGurtoggFelRageMeleePosition(
    Player* bot, Unit const& gurtogg, Unit const& felRageTarget);
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
inline constexpr uint32 RUNE_SHIELD_MAGE_PRIORITY_MS = 2 * IN_MILLISECONDS;
inline constexpr float SPELLSTEAL_RANGE = 30.0f;

bool IsSufferingFixateTank(Player* bot);
bool IsOutOfSufferingPosition(Player* bot, Unit* suffering);

// Mother Shahraz

inline constexpr float SHAHRAZ_TANK_POSITION_TOLERANCE = 0.5f;
// Wide enough for an off-tank on her victim to take over without melee backing off.
inline constexpr float SHAHRAZ_POSITIONED_DISTANCE = 3.0f;
inline constexpr float SHAHRAZ_OFF_TANK_DISTANCE = 2.0f;
inline constexpr float SHAHRAZ_RANGED_POSITION_TOLERANCE = 1.0f;
inline constexpr float SHAHRAZ_MELEE_ENGAGE_HEALTH_PCT = 90.0f;
inline constexpr float SHAHRAZ_FATAL_ATTRACTION_STEP_DISTANCE = 5.0f;

inline Position const SHAHRAZ_TANK_POSITION       = { 960.438f, 178.989f, 192.826f };
inline Position const SHAHRAZ_TRANSITION_POSITION = { 951.327f, 179.550f, 192.550f };
inline Position const SHAHRAZ_RANGED_POSITION     = { 935.267f, 175.459f, 192.821f };

// Melee dps wait at the ranged position while this is true.
bool IsShahrazBeingPositioned(Unit* shahraz);

// Illidari Council

inline constexpr EncounterHelpers::ClassRole ZEREVOR_MAGE_TANK_ROLE = {
    .playerClass = CLASS_MAGE,
    .role = { "class_role_zerevor_mage_tank", "Zerevor mage tank" },
    .candidate = { "class_role_class_mage", "Mage" },
};
// 125 yards extends to the outer entrance to the Council's room.
inline constexpr float COUNCIL_CLASS_ROLE_ANNOUNCE_DISTANCE = 125.0f;
// 175 yards extends to the top of the steps leading to the courtyard outside the Council's room.
inline constexpr float COUNCIL_CLASS_ROLE_REARM_DISTANCE = 175.0f;

inline constexpr uint32 COUNCIL_DPS_WAIT_MS = 5 * IN_MILLISECONDS;
inline constexpr float COUNCIL_AOE_THREAT_CLEARANCE = 15.0f;
inline constexpr float COUNCIL_RANGED_SPREAD_DISTANCE = 4.0f;
inline constexpr float COUNCIL_MEMBER_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 COUNCIL_MEMBER_CACHE_INTERVAL_MS = 1000;
inline constexpr float COUNCIL_TANK_ARRIVAL_DISTANCE = 2.0f;
// Bots heal out to HealDistance, 38.5 yd by default.
inline constexpr float MAGE_TANK_HEALER_MAX_DISTANCE = 38.0f;
// A patch on the mage tank reaches 11.5 yd from where it stood.
inline constexpr float MAGE_TANK_HEALER_MIN_DISTANCE = 12.5f;
// Zerevor casts Arcane Explosion when anyone is within 10 yd plus both combat reaches (14.65 yd).
inline constexpr float ZEREVOR_ARCANE_EXPLOSION_SAFE_DISTANCE = 15.0f;
// A patch reaches a player whose centre is within its radius plus the player's reach (1.5 yd).
inline constexpr float COUNCIL_PATCH_MARGIN = 2.5f;
inline constexpr int32 CIRCLE_OF_HEALING_REACTION_MS = 200;
// A ranged bot whose own cast would end with less than this left on Circle of Healing drops it.
inline constexpr int32 CIRCLE_OF_HEALING_CASTER_MARGIN_MS = 500;
inline constexpr float COUNTERSPELL_RANGE = 30.0f;
inline constexpr float WIND_SHEAR_RANGE = 25.0f;
inline constexpr float SPELL_LOCK_RANGE = 30.0f;

inline std::array const GATHIOS_TANK_POSITIONS = {
    Position{ 662.977f, 296.246f, 271.688f },
    Position{ 636.238f, 283.719f, 271.629f },
    Position{ 655.571f, 261.377f, 271.687f },
    Position{ 673.789f, 274.139f, 271.689f },
};
inline Position const ZEREVOR_SPAWN_POSITION = { 700.792f, 307.192f, 277.526f };
inline std::array const ZEREVOR_TANK_POSITIONS = {
    Position{ 654.854f, 352.502f, 271.689f },
    Position{ 644.597f, 338.875f, 271.688f },
};

struct IllidariCouncilMemberGuids
{
    ObjectGuid gathios;
    ObjectGuid malande;
    ObjectGuid zerevor;
    ObjectGuid veras;
};

struct IllidariCouncilMembers
{
    Unit* gathios = nullptr;
    Unit* malande = nullptr;
    Unit* zerevor = nullptr;
    Unit* veras = nullptr;
};

// An entry search, not find target, since a bot back from the dead isn't yet on the members'
// threat lists. Only members in combat count.
IllidariCouncilMemberGuids FindIllidariCouncilMemberGuids(Player* bot);
IllidariCouncilMembers GetIllidariCouncilMembers(PlayerbotAI* botAI);
bool UpdateIllidariCouncilClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetZerevorMageTank(PlayerbotAI* botAI);
bool IsZerevorMageTank(PlayerbotAI* botAI);
bool HasDangerousCouncilAura(Player* bot);
bool IsVerasVanished(Unit* veras);
bool CanInterruptCircleOfHealing(Unit* malande);
// Zerevor has at most one Blizzard and one Flamestrike down at a time.
bool IsInZerevorPatch(Unit* zerevor, Position const& point, float margin = 0.0f);
bool IsInCouncilPatch(Position const& point, Unit* gathios, Unit* zerevor, float margin = 0.0f);
// Gathios's Consecration reaches Malande only at the pull, while he is beside her on the dais.
bool IsMalandeInCouncilPatch(Unit* malande, Unit* gathios, Unit* zerevor);
// Ranged interrupt only while her melee can't: under Blessing of Protection, or in a council
// patch. Their interrupts are all magic, so never under Blessing of Spell Warding.
bool ShouldRangedInterruptCircleOfHealing(Unit* malande, Unit* gathios, Unit* zerevor);
bool IsZerevorOnMageTank(PlayerbotAI* botAI, Unit* zerevor);
// Within heal range of the mage tank, clear of a patch on it, of Arcane Explosion, and of any
// patch down.
bool IsMageTankHealerPositionSafe(Position const& point, Player* mageTank, Unit* zerevor);
// The nearest safe point on two rings round the mage tank that the bot can walk to in a straight
// line and that has line of sight to the mage tank.
bool FindMageTankHealerPosition(
    Player* bot, Player* mageTank, Unit* zerevor, Position& destination);
// The nearest point within melee range of Malande, clear of every council patch, that the bot can
// walk to in a straight line.
bool FindMalandeTankPosition(
    Player* bot, Unit* malande, Unit* gathios, Unit* zerevor, Position& destination);
// Any council member but the bot's own target.
bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range);
// Felhunters stay on Malande for Spell Lock; other pets follow their master's target, or go to
// Gathios while the master has none.
Unit* GetCouncilPetTarget(PlayerbotAI* botAI, Creature* pet);
// PlayerbotAI::CanCastSpell passes any spell the pet knows without checking range or cooldown.
uint32 GetReadySpellLock(Creature* pet, Unit* target);

// Illidan Stormrage <The Betrayer>

enum class IllidanPhase : uint8
{
    None, // also Maiev's arrival scene
    Initial, // "phase 1"
    Flying, // "phase 2"
    Landing, // transition from Illidan flying to his landing point until he is again selectable
    Grounded, // "phase 3"
    Demon, // "phase 4"
    Maiev, // "phase 5"
    Dying, // the 1 HP death scene
};

// Damage on him is held after each threat reset, so the tank that should have him acts first.
enum class IllidanDpsHold : uint8
{
    None,
    AfterReset,      // Phase 1, 3, and 5 starts; everyone but the Main Tank
    DemonFormStart,  // everyone but the warlock tank
    DemonFormMiddle, // from the start hold to the end hold; the Main Tank only
    DemonFormEnd,    // the last 3s before the reset; everyone
    DemonFormNoPush, // 30 to 33% in the form, so he leaves it above 30%; everyone but the
                     // warlock tank
};

// The Main Tank's lead in Phases 1, 3, and 5.
enum class IllidanTankLead : uint8
{
    None,
    Pickup,    // straight out from near the grate centre
    Opening,   // once at the pull, clockwise
    Step,      // on a Flame Crash
    StepAgain, // once, when he stopped short of the step
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
    Position warlockTankPosition;
    Position mainTankWaitPosition;
    Position mainTankPickupPosition;
    Position mainTankStepInPosition;
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

// One Shadowfiend infection, recorded by the mechanic tracker. Once the trap is assigned, hunter is
// empty if a Frost Trap already covered the position, or once the hunter has laid its trap.
struct IllidanInfection
{
    Position infectionPosition;
    ObjectGuid hunter = ObjectGuid::Empty;
    bool isTrapAssigned = false;
};

inline constexpr EncounterHelpers::ClassRole ILLIDAN_WARLOCK_TANK_ROLE = {
    .playerClass = CLASS_WARLOCK,
    .preferredSpec = WARLOCK_TAB_DEMONOLOGY,
    .role = { "class_role_illidan_warlock_tank", "Illidan warlock tank" },
    .candidate = { "class_role_class_warlock", "Warlock" },
    .preferredCandidate = { "class_role_class_demonology_warlock", "Demonology Warlock" },
};
// The raid has to get closer than 100 yards to Illidan to talk to Akama.
inline constexpr float ILLIDAN_CLASS_ROLE_ANNOUNCE_DISTANCE = 100.0f;
// The height of the second landings of the double stairs leading to the Temple summit.
inline constexpr float ILLIDAN_STAIRWELL_Z = 341.0f;
// 175 yards rearm distance matches the Council's because Illidan's XY position is nearly identical.
inline constexpr float ILLIDAN_CLASS_ROLE_REARM_DISTANCE = 175.0f;

inline constexpr uint32 ILLIDAN_PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 ILLIDAN_TRAP_HUNTER_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 ILLIDAN_SPEECH_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 ILLIDAN_SHADOW_DEMON_CACHE_INTERVAL_MS = 200;
// From any bot on the summit, about 90 yd reaches the far edge.
inline constexpr float ILLIDAN_ADD_SEARCH_RADIUS = 100.0f;
// From his position, this takes in Akama's, 81 yd away, where the speech is started.
inline constexpr float ILLIDAN_SPEECH_SEARCH_RADIUS = 100.0f;
inline constexpr float ILLIDAN_POSITION_TOLERANCE = 1.0f;
inline constexpr float ILLIDAN_MIN_SEGMENT_LENGTH = 0.1f;
// A bot without the Shadowfiend debuff this close to a Shadowfiend is where a debuff just ended.
inline constexpr float ILLIDAN_SHADOWFIEND_BIRTHER_DISTANCE = 8.0f;
// Below this a Shadowfiend is finished off: melee take one in their melee range, and its birther
// acts again.
inline constexpr float ILLIDAN_SHADOWFIEND_FINISH_HEALTH_PCT = 50.0f;
// He lifts off at 65% and Maiev arrives at 30%.
inline constexpr float ILLIDAN_PHASE_2_HEALTH_PCT = 65.0f;
inline constexpr float ILLIDAN_PHASE_5_HEALTH_PCT = 30.0f;
// DPS cooldowns are held from 95% down to this, and Bloodlust and Heroism from the pull.
inline constexpr float ILLIDAN_DPS_COOLDOWN_RELEASE_HEALTH_PCT = 62.0f;
// At or below this, and above 30%, only the warlock tank damages him in Demon Form, so he changes
// back above 30%.
inline constexpr float ILLIDAN_DEMON_FORM_NO_PUSH_HEALTH_PCT = 33.0f;
inline constexpr uint32 ILLIDAN_RESET_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
// A tank has aggro once it is the victim with this much threat: a threat reset leaves the old
// victim in place at 0. One heal's threat is a few hundred.
inline constexpr float ILLIDAN_TANK_AGGRO_THREAT = 2000.0f;
// After the landing's or Demon Form's threat reset the rest of the raid waits for the Main Tank's
// aggro at most this long.
inline constexpr uint32 ILLIDAN_PICKUP_MAX_MS = 3 * IN_MILLISECONDS;
// Damage on the Flames of Azzinoth waits this long from the first Flame's record, for their tanks:
// a Flame is in combat with nobody for its first 2 s.
inline constexpr uint32 ILLIDAN_FLAME_DPS_WAIT_MS = 6 * IN_MILLISECONDS;
// An immunity clears Dark Barrage only this long after it lands, so its beam shows first; it costs
// one 3000 tick, the first coming at 1 s.
inline constexpr uint32 ILLIDAN_DARK_BARRAGE_REACTION_MS = 1500;
// The warlock tank has him to itself for the 12.23 s transform and about 2 s more.
inline constexpr uint32 ILLIDAN_DEMON_FORM_DPS_WAIT_MS = 14 * IN_MILLISECONDS;
// Demon Form ends, with a threat reset, this long after the transform starts.
inline constexpr uint32 ILLIDAN_DEMON_FORM_DURATION_MS = 72230;
// Everyone stops damaging him this long before the end.
inline constexpr uint32 ILLIDAN_DEMON_FORM_END_HOLD_MS = 3 * IN_MILLISECONDS;
// The Main Tank's walk-in, about 44 yd, starts this long before the end. No Flame Burst comes in
// the last 14 s.
inline constexpr uint32 ILLIDAN_MAIN_TANK_WALK_IN_MS = 10 * IN_MILLISECONDS;
// For the last 1.3 s it steps in about 8 yd, so at the reset it is in his melee range (his reach
// is 7.5 again) while he finishes his last Shadow Blast, and can hit before his first victim
// check; at most one Aura of Dread tick.
inline constexpr uint32 ILLIDAN_MAIN_TANK_STEP_IN_MS = 1300;
// Shadow Demons appear this long after the transform starts, 30 s after he turns aggressive.
inline constexpr uint32 ILLIDAN_SHADOW_DEMON_SPAWN_MS = 42230;
// Between Seeds (about 890 mana each at 70) and between the warlock tank's Searing Pains (about
// 210), a warlock Life Taps below this mana, if above this health, which the warlock tank needs
// against Shadow Blast.
inline constexpr float ILLIDAN_LIFE_TAP_MANA_PCT = 20.0f;
inline constexpr float ILLIDAN_LIFE_TAP_HEALTH_PCT = 50.0f;
// Hunters misdirect onto the Main Tank this long from each Phase 1, 3 and 5 start.
inline constexpr uint32 ILLIDAN_PHASE_START_MISDIRECT_MS = 3 * IN_MILLISECONDS;
// A Frost Trap patch already on the infection position does for a new infection if it outlasts the
// debuff by this much, the time ranged get on the slowed Shadowfiends.
inline constexpr int32 ILLIDAN_FROST_TRAP_SLOW_MS = 5 * IN_MILLISECONDS;
// A Frost Trap not yet sprung this close to the infection position does too: Shadowfiends spawning
// there spring it from 3.9 yd.
inline constexpr float ILLIDAN_FROST_TRAP_DISTANCE = 3.0f;

// Tank ring: he is held 25 yd from the grate centre with the Main Tank 6 yd outward of him, so he
// faces out and the raid stands inside.
inline constexpr float ILLIDAN_RING_RADIUS = 25.0f;
inline constexpr float ILLIDAN_TANK_OFFSET = 6.0f;
// Once free of Shadow Prison, for the 2.83 s before he wakes, the Main Tank stands this far
// inward of him: between him and the raid, and clear of his backing off.
inline constexpr float ILLIDAN_MAIEV_SCENE_TANK_OFFSET = 6.0f;
// He walks back from a victim this close to his centre (both collision radii), checked every 2 to
// 6 s. A Main Tank whose straight walk would pass this close goes round him instead, stepping
// toward the settle-circle point 60 deg on, on the side nearer its position, about 5 yd from his
// centre.
inline constexpr float ILLIDAN_BACK_OFF_DISTANCE = 4.5f;
inline constexpr float ILLIDAN_ROUND_STEP_ANGLE = 60.0f * static_cast<float>(M_PI) / 180.0f;
// On each Flame Crash the Main Tank walks to the ring point this far counterclockwise of him,
// backward once he is behind it. He starts once it is 9.5 yd away, catches it and stops about 25
// deg on, back on the ring.
inline constexpr float ILLIDAN_RING_LEAD_ANGLE = 35.0f * static_cast<float>(M_PI) / 180.0f;
// A cast (Draw Soul) can hold him until the tank stands at its lead position; against a standing
// tank he stops 9.5 yd short of it, about 13 deg on. A step under this is led once more.
inline constexpr float ILLIDAN_RING_SHORT_STEP_ANGLE = 20.0f * static_cast<float>(M_PI) / 180.0f;
// While he is this close to the grate centre, as after landing, the Main Tank leads him straight
// out along its own bearing (due north after landing) to the lead radius; he stops 22 to 28 yd
// out.
inline constexpr float ILLIDAN_PICKUP_DISTANCE = 15.0f;
inline constexpr float ILLIDAN_PICKUP_LEAD_RADIUS = 28.0f;
inline constexpr float ILLIDAN_LANDING_LEAD_ANGLE = 0.0f;
// Once at the pull, before the first Flame Crash, the Main Tank leads him from due north to the
// point 62 deg clockwise of north, 27 yd out; he stops about 302 deg. Phase 1's Flame Crashes then
// land clear of the Flame tanks' steps, which they last into Phase 2.
inline constexpr float ILLIDAN_OPENING_LEAD_ANGLE = -62.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ILLIDAN_OPENING_LEAD_RADIUS = 27.0f;
// The lead starts only this soon after the pull; the first Flame Crash comes at 25 to 30 s.
inline constexpr uint32 ILLIDAN_OPENING_LEAD_MS = 15 * IN_MILLISECONDS;
// A Flame Crash this close to the Main Tank was cast on it.
inline constexpr float ILLIDAN_FLAME_CRASH_ON_TANK_DISTANCE = 9.0f;

// Phases 3 and 5: the centre, six at 6 yd and twelve at 12 yd, in that fill order.
inline constexpr size_t ILLIDAN_GRATE_INNER_SLOT_COUNT = 6;
inline constexpr size_t ILLIDAN_GRATE_OUTER_SLOT_COUNT = 12;
inline constexpr size_t ILLIDAN_RANGED_SLOT_COUNT =
    1 + ILLIDAN_GRATE_INNER_SLOT_COUNT + ILLIDAN_GRATE_OUTER_SLOT_COUNT;
inline constexpr float ILLIDAN_GRATE_INNER_RADIUS = 6.0f;
inline constexpr float ILLIDAN_GRATE_OUTER_RADIUS = 12.0f;
inline constexpr size_t ILLIDAN_HEALER_SLOT_COUNT = 8;
inline constexpr size_t ILLIDAN_MELEE_SLOT_COUNT = 15;

// Shadowfiend infection position, every phase: on the Main Tank's circle, counterclockwise of him,
// ahead of his steps, so no Flame Crash zone lies there and Draw Soul's cone in a step stops short
// (110 deg); far enough round to stay out of Shadow Blast at the transform after a late step.
inline constexpr float ILLIDAN_INFECTION_POSITION_DISTANCE = 31.0f;
inline constexpr float ILLIDAN_INFECTION_POSITION_ANGLE =
    135.0f * static_cast<float>(M_PI) / 180.0f;

// Phase 2: Eye Blast's head hits within this of its centre; its path ahead is kept clear by 3 yd.
inline constexpr float ILLIDAN_EYE_BLAST_HAZARD_RADIUS = 6.0f;
inline constexpr float ILLIDAN_EYE_BLAST_CLEARANCE = ILLIDAN_EYE_BLAST_HAZARD_RADIUS + 3.0f;

// Phase 4 hazards: Aura of Dread round him and Shadow Blast round his victim, each kept clear by
// 2 yd.
inline constexpr float ILLIDAN_AURA_OF_DREAD_HAZARD_RADIUS = 15.0f;
inline constexpr float ILLIDAN_AURA_OF_DREAD_CLEARANCE =
    ILLIDAN_AURA_OF_DREAD_HAZARD_RADIUS + 2.0f;
inline constexpr float ILLIDAN_SHADOW_BLAST_HAZARD_RADIUS = 20.0f;
inline constexpr float ILLIDAN_SHADOW_BLAST_CLEARANCE = ILLIDAN_SHADOW_BLAST_HAZARD_RADIUS + 2.0f;
// Phase 4, from him and off his inward line, positive toward the warlock tank's side. His reach is
// 1.5 in Demon Form, so a 30 yd spell reaches 33 yd on him: the warlock tank stands 2 yd inside.
inline constexpr float ILLIDAN_WARLOCK_POSITION_DISTANCE = 31.0f;
inline constexpr float ILLIDAN_WARLOCK_POSITION_ANGLE = 75.0f * static_cast<float>(M_PI) / 180.0f;
// The Main Tank waits beside the melee, on the first melee row one slot past a group of seven
// (5 melee DPS and 2 tanks) on the side away from the warlock tank, so the raid can kill a Shadow
// Demon walking to it.
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_DISTANCE = 48.4f;
inline constexpr float ILLIDAN_MAIN_TANK_WAIT_ANGLE = -35.0f * static_cast<float>(M_PI) / 180.0f;
// At the end, walk straight to the pickup position in front of the raid, just outside Aura of Dread
// and 22.5 yd from the warlock tank (Shadow Blast 20), then step in on the same bearing to 9 yd,
// 25.4 yd from the warlock tank. At the reset any threat beats his old victim's 0, Replenishment
// and HoT ticks included, so the tank must hit first.
inline constexpr float ILLIDAN_PICKUP_POSITION_DISTANCE = 17.0f;
inline constexpr float ILLIDAN_PICKUP_POSITION_ANGLE = 30.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ILLIDAN_STEP_IN_POSITION_DISTANCE = 9.0f;

// World trigger 22515.
inline Position const ILLIDAN_GRATE_CENTRE =           { 676.021f, 305.455f, 353.582f };
// He kneels here and stands here for his speech.
inline Position const ILLIDAN_SPEECH_POSITION =        { 705.732f, 304.988f, 353.925f };
// The Main Tank waits here at the pull, 15.5 yd north of the centre, so he stops on the ring when
// he runs in.
inline Position const ILLIDAN_PULL_TANK_POSITION =     { 691.521f, 305.455f, 353.790f };
// While he lands, everyone else waits 22 yd south of the centre and the Main Tank 6 yd south of
// the landing point, in his way if he runs at them after the threat reset.
inline Position const ILLIDAN_LANDING_TANK_POSITION =  { 670.648f, 304.761f, 353.920f };
inline Position const ILLIDAN_LANDING_STACK_POSITION = { 654.021f, 305.455f, 353.790f };
inline Position const ILLIDAN_LANDING_POSITION =       { 676.648f, 304.761f, 354.189f };
inline Position const ILLIDAN_N_GRATE_POSITION =       { 682.100f, 306.000f, 353.192f };
inline Position const ILLIDAN_E_GRATE_POSITION =       { 673.500f, 298.500f, 353.192f };
inline Position const ILLIDAN_W_GRATE_POSITION =       { 672.400f, 312.500f, 353.192f };
inline std::array const ILLIDAN_GRATE_POSITIONS = {
    ILLIDAN_N_GRATE_POSITION,
    ILLIDAN_E_GRATE_POSITION,
    ILLIDAN_W_GRATE_POSITION,
};

// Assist tank indices for the Flames of Azzinoth, as GetGroupAssistTank and IsAssistTankOfIndex
// number them.
inline constexpr uint8 ILLIDAN_WEST_FLAME_TANK_INDEX = 0;
inline constexpr uint8 ILLIDAN_EAST_FLAME_TANK_INDEX = 1;

// The Flame tanks' Blaze steps, one per Flame Blast, in order. The Flame follows its tank in, so
// each step is walked into on a line that turns the Flame Blast cone away from the grate stacks.
// The West tank starts on its inner steps, where Phase 1's Flame Crashes can't reach, and in melee
// reach of the grate.
inline Position const ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
inline std::array const ILLIDAN_E_GLAIVE_TANK_POSITIONS = {
    Position{ 657.100f, 291.200f, 354.000f },
    Position{ 658.600f, 280.700f, 354.000f },
    Position{ 672.100f, 277.700f, 354.000f },
    Position{ 682.600f, 279.200f, 354.000f },
    Position{ 685.600f, 289.700f, 354.000f },
    Position{ 669.100f, 288.200f, 354.000f },
};

inline Position const ILLIDAN_W_GLAIVE_WAITING_POSITION = { 676.102f, 316.305f, 353.192f };
inline std::array const ILLIDAN_W_GLAIVE_TANK_POSITIONS = {
    Position{ 695.700f, 311.700f, 354.000f },
    Position{ 685.200f, 320.700f, 354.000f },
    Position{ 668.700f, 323.700f, 354.000f },
    Position{ 674.700f, 332.700f, 354.000f },
    Position{ 685.200f, 331.200f, 354.000f },
    Position{ 695.700f, 325.200f, 354.000f },
};

// The recorded infections, read-only; null before the tracker records any.
std::unordered_map<ObjectGuid, IllidanInfection> const* FindIllidanInfections(uint32 instanceId);
IllidanPhase GetIllidanPhase(Unit* illidan);
// He is up and talking after Akama's gossip, still immune to players and out of combat.
bool IsIllidanSpeechInProgress(Player* bot);
// Written by the mechanic tracker, and at the Demon Form transform by its listener; true if the
// phase changed.
bool RecordIllidanPhase(uint32 instanceId, IllidanPhase phase);
// A phase change the mechanic tracker hasn't recorded yet counts as just started.
bool IsIllidanPhaseStartWithin(Unit const& illidan, IllidanPhase phase, uint32 durationMs);
// Until the tracker records the change, the last phase it recorded.
IllidanPhase GetIllidanPhaseBefore(Unit const& illidan, IllidanPhase phase);
// Never true before the tracker has recorded the form.
bool IsIllidanDemonFormEnding(Unit const& illidan, uint32 durationMs);
// The hold window that is running, before who it spares is decided.
IllidanDpsHold GetIllidanHoldWindow(Unit* illidan);
IllidanDpsHold GetIllidanDpsHold(PlayerbotAI* botAI, Unit* illidan);
bool IsFlameOfAzzinothDpsHeld(Unit const& illidan);
// The bot's entry on his threat list is online, not suppressed: after Shadow Prison only threat
// the bot causes brings it back.
bool IsOnlineOnIllidanThreatList(Unit const& illidan, Player* bot);
// From each phase change until the tracker sees the tanks have aggro, and all through his landing
// and Maiev's arrival; never at the pull or in the death scene.
bool IsIllidanTankAggroPending(Unit const& illidan, IllidanPhase phase);
// Each tank that should have a creature is its victim with ILLIDAN_TANK_AGGRO_THREAT: him the Main
// Tank, or in Demon Form the warlock tank; while he flies each living Flame its own tank, or the
// other one while its own is dead. A dead tank counts as having it, since nothing waits for it.
bool DoIllidanTanksHaveAggro(PlayerbotAI* botAI, Unit const& illidan, IllidanPhase phase);
// From his touchdown on landing, and after the landing's or Demon Form's threat reset until the
// Main Tank has aggro: the tank stands and hits him, and the rest of the raid causes him no
// threat, since at 0 anything else's first threat takes him from the tank.
bool IsIllidanPickupPending(PlayerbotAI* botAI, Unit* illidan);
// Single-target damage on a Shadowfiend or a Shadow Demon, which the pickup lets through.
bool IsIllidanPickupExemptTarget(Unit const* target, bool isAoe);
// While he flies and in Demon Form, and in the pickup, where a pet's attack would take him from
// the Main Tank.
bool ShouldIllidanPetsBePassive(PlayerbotAI* botAI, Unit* illidan);
// In the pickup a channel begun before the hold still gives him threat after the reset
// (Evocation's and Hymn of Hope's mana, Tranquility's heals, a Blizzard), so everyone but the Main
// Tank stops theirs; single-target damage on an add aside.
bool IsIllidanPickupChannelHeld(PlayerbotAI* botAI, Unit* illidan, Spell const& channel);
// The game angle of a point seen from the grate centre: 0 north, rising counterclockwise.
float GetGrateAngle(Position const& position);
Position GetRingPoint(float angle, float radius);
// 6 yd outward of him. A tank walking there from within 9.5 yd only turns him to face out.
Position GetIllidanSettlePosition(Unit const& illidan);
Position GetGrateSlot(size_t index);
DemonFormLayout GetDemonFormLayout(Unit const& illidan);
// Where an infection position is recorded, from where he stands now.
Position GetIllidanInfectionPosition(Unit const& illidan);
// The infection position recorded for the bot's own infection, else for the infection it traps for,
// else the one a new infection would share, else where one would be recorded now.
Position GetIllidanRecordedInfectionPosition(PlayerbotAI* botAI, Unit const& illidan);
std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot);
bool IsWestFlameOfAzzinothTank(Player* bot);
bool IsEastFlameOfAzzinothTank(Player* bot);
bool IsFlameOfAzzinothTank(Player* bot);
// The living Flame this bot tanks: its own, else the other one once its own is dead and that
// Flame's tank is dead too.
Unit* GetAssignedFlameOfAzzinoth(Player* bot);
bool UpdateIllidanClassRoleAnnouncement(PlayerbotAI* botAI);
ObjectGuid GetIllidanWarlockTankGuid(Player* bot);
// Null while the warlock tank is dead.
Player* GetIllidanWarlockTank(PlayerbotAI* botAI);
bool IsIllidanWarlockTank(PlayerbotAI* botAI);
// For the Illidan actions that outrank the stock Life Tap, which waits until a spell is
// unaffordable.
bool CastIllidanLifeTapIfLow(PlayerbotAI* botAI);
// The trap hunters in the order they take infections: hunter bots by GUID, dead or alive.
GuidVector FindIllidanTrapHunterGuids(Player* bot);
// Assigned to an infection whose debuff is still on, with Frost Trap ready, and not infected.
bool IsIllidanTrapHunterOnDuty(PlayerbotAI* botAI);
// Never runs to the infection position, so its Shadowfiends come out where it stands: his victim,
// the Main Tank, or the warlock tank in Demon Form.
bool IsExemptFromIllidanInfectionRun(PlayerbotAI* botAI, Unit* illidan, Player* player);
// While he flies, a bot infected before he took off waits at the infection position until its
// Shadowfiends are out, tanks included (the raid can't move away from them in Phase 2), and its
// trap hunter still lays the trap. The Main Tank, which never runs, waits where it stands.
bool IsIllidanInfectionPositionKeptInFlight(PlayerbotAI* botAI);
// A Frost Trap not yet sprung at infectionPosition, or a patch round it lasting at least
// minDurationMs more.
bool IsFrostTrapAt(Player* bot, Position const& infectionPosition, int32 minDurationMs);
// Slot lists are bots on the map, the dead kept, each part by GUID: ranged are healers, then
// ranged DPS; melee are melee DPS, then tanks, the Main Tank left out. In Demon Form the warlock
// tank and the healers leave the ranged list; healers have their own. The index wraps round
// slotCount; -1 if the bot isn't in the list.
int32 GetIllidanRangedIndex(PlayerbotAI* botAI, size_t slotCount, bool inDemonForm);
int32 GetIllidanHealerIndex(PlayerbotAI* botAI, size_t slotCount);
int32 GetIllidanMeleeIndex(PlayerbotAI* botAI, size_t slotCount);
// Phases 3 and 5: the grate slot by roster, except that the warlock tank (a living bot) takes the
// slot nearest its Demon Form position, so its walk at the transform is short, and whoever had that
// slot takes the warlock tank's.
int32 GetIllidanGrateIndex(PlayerbotAI* botAI, Unit const& illidan);
// While he flies: the grate stack by roster, every bot but the Flame tanks, the dead kept.
int32 GetIllidanGrateStackIndex(PlayerbotAI* botAI);
// Either Shadowfiend debuff, Illidan's own (41917) first.
Aura* GetParasiticShadowfiendAura(Player* player);
bool HasParasiticShadowfiend(Player* player);
// An entry search, since a Shadowfiend is out of combat, and so not in attackers, until it turns
// aggressive 2.4 s after spawning.
GuidVector FindParasiticShadowfiendGuids(PlayerbotAI* botAI);
// Shadowfiends have a combat reach of 1.0y.
Unit* GetNearestParasiticShadowfiend(PlayerbotAI* botAI, Unit const& unit);
// No Shadowfiend debuff and a living Shadowfiend close by that isn't yet nearly dead, as where a
// debuff just ended; any of them, since the pair spawn together and ranged focus one.
bool IsNearNewParasiticShadowfiend(PlayerbotAI* botAI);
// Living group members with either Shadowfiend debuff, humans included, so trap hunters trap for
// them too.
GuidVector FindPlayersWithParasiticShadowfiendGuids(Player* bot);
// An entry search, so it doesn't rely on the Demons being in attackers (unchecked); each Demon's
// target is the group member or pet carrying its Paralyze.
std::vector<ShadowDemon> FindShadowDemons(PlayerbotAI* botAI);
// For ranged and healers: a Demon moving to the warlock tank first, then the one nearest its
// target, ties going to the one nearest `from`; idle Demons last.
Unit* GetShadowDemonForRangedToAttack(PlayerbotAI* botAI, Unit const& from);
Unit* GetShadowDemonMovingToWarlockTank(PlayerbotAI* botAI);
// For melee: the Demon nearest `from`, one still moving to a target before an idle one. Never one
// moving to the warlock tank, which stays inside Aura of Dread or Shadow Blast all the way.
Unit* GetShadowDemonForMeleeToAttack(PlayerbotAI* botAI, Unit const& from);
// Imps and water elementals take Demons as ranged do, other pets as melee do.
Unit* GetShadowDemonForPetToAttack(PlayerbotAI* botAI, Pet const& pet);
// The bot's current target is a Shadow Demon: in Demon Form, the one reason melee leave their
// slots.
bool IsTargetingShadowDemon(PlayerbotAI* botAI);
// The point of the segment from start to end nearest to point, in 2D, at start's Z.
Position GetClosestPointOnSegment(
    Position const& start, Position const& end, Position const& point);
EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area);
bool IsPathInEyeBlastDangerArea(
    Position const& from, Position const& to, EyeBlastDangerArea const& area);

// Shared encounter state

struct BtInstanceState
{
    std::optional<std::vector<NajentusSpineAssignment>> najentusSpineAssignments;
    std::optional<ObjectGuid> najentusSpineThrower;
    std::optional<uint32> najentusSpineThrowTime;
    std::optional<uint32> councilDpsWaitTimer;
    std::optional<IllidanPhase> illidanLastPhase;
    std::optional<IllidanPhase> illidanPreviousPhase;
    std::optional<uint32> illidanPhaseStartTime;
    std::optional<uint32> illidanFlamePhaseStartTime;
    std::optional<ObjectGuid> eastFlameGuid;
    std::optional<ObjectGuid> westFlameGuid;
    // The phase in which the tracker has seen the tanks take aggro, cleared at each phase change.
    std::optional<IllidanPhase> illidanTankAggroPhase;
    // Each infected player's infection, kept until the debuff is gone.
    std::optional<std::unordered_map<ObjectGuid, IllidanInfection>> illidanInfections;
    EncounterHelpers::ClassRoleAssignment zerevorMageTank;
    EncounterHelpers::ClassRoleAssignment illidanWarlockTank;
};

bool UpdateBtClassRoles(PlayerbotAI* botAI);
BtInstanceState& BtState(uint32 instanceId);
BtInstanceState* FindBtState(uint32 instanceId);
bool BtResetInstance(uint32 instanceId);

}

#endif
