/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAHELPERS_H
#define PLAYERBOTS_KARAHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <cmath>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace KaraHelpers
{

struct KaraInstanceState;

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class KaraSpells : uint32
{
    // Trash
    SPELL_WARP_BREACH             = 29919,

    // Maiden of Virtue
    SPELL_REPENTANCE              = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD  = 30756,
    SPELL_GOBLIN_ROCKET_BOOTS     = 8892,  // the Little Red Riding Hood speed cheat (70% vs. 50%)
    SPELL_FEIGN_DEATH             = 5384,
    SPELL_INTERVENE               = 3411,

    // The Curator
    SPELL_CURATOR_EVOCATION       = 30254,

    // Terestian Illhoof
    SPELL_BROKEN_PACT             = 30065, // on Terestian from Kil'rek's death to his return
    SPELL_SACRIFICE               = 30115, // on the player until the Demon Chains die

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST       = 30004,
    SPELL_FLAME_WREATH_AURA       = 29946,
    SPELL_BLIZZARD                = 29951,
    SPELL_ARCANE_EXPLOSION        = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF         = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF       = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF        = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_NETHER_EXHAUSTION_RED   = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE  = 38639,
    SPELL_NETHERSPITE_BANISHED    = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH           = 30129,
    SPELL_RAIN_OF_BONES           = 37098, // the channel; its ticks are 37091

    // Shaman
    SPELL_TREMOR_TOTEM            = 8143,
    SPELL_GROUNDING_TOTEM         = 8177,

    // Warlock
    SPELL_BANISH_RANK_1           = 710,
    SPELL_BANISH_RANK_2           = 18647,
};

enum class KaraNpcs : uint32
{
    // Trash
    NPC_MANA_WARP                   = 16530,

    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Moroes
    NPC_LADY_KEIRA_BERRYBUCK        = 17007,
    NPC_LADY_CATRIONA_VON_INDI      = 19872,
    NPC_LORD_CRISPIN_FERENCE        = 19873,
    NPC_BARON_RAFE_DREUGER          = 19874,
    NPC_BARONESS_DOROTHEA_MILLSTIPE = 19875,
    NPC_LORD_ROBIN_DARIS            = 19876,

    // Romulo and Julianne
    NPC_ROMULO                      = 17533,
    NPC_JULIANNE                    = 17534,

    // The Wizard of Oz
    NPC_DOROTHEE                    = 17535,
    NPC_STRAWMAN                    = 17543,
    NPC_ROAR                        = 17546,
    NPC_TINHEAD                     = 17547,
    NPC_TITO                        = 17548,

    // The Curator
    NPC_THE_CURATOR                 = 15691,

    // Terestian Illhoof
    NPC_TERESTIAN_ILLHOOF           = 15688,
    NPC_KILREK                      = 17229,
    NPC_DEMON_CHAINS                = 17248,
    NPC_FIENDISH_IMP                = 17267,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL          = 17167,

    // Netherspite
    NPC_NETHERSPITE                 = 15689,
    NPC_VOID_ZONE                   = 16697,
    NPC_GREEN_PORTAL                = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL                 = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                  = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_PRINCE_MALCHEZAAR           = 15690,
    NPC_NETHERSPITE_INFERNAL        = 17646,

    // Nightbane
    NPC_NIGHTBANE                   = 17225,
    NPC_RESTLESS_SKELETON           = 17261,
};

// Shared
inline constexpr uint32 KARA_MAP_ID = 532;

// DungeonEncounter.dbc encounterIndex, map 532
enum class KaraBosses : uint32
{
    DATA_ATTUMEN                    = 0,
    DATA_MOROES                     = 1,
    DATA_MAIDEN                     = 2,
    DATA_OPERA_PERFORMANCE          = 3,
    DATA_CURATOR                    = 4,
    DATA_TERESTIAN                  = 5,
    DATA_ARAN                       = 6,
    DATA_NETHERSPITE                = 7,
    // DATA_CHESS_EVENT             = 8, // No strategy
    DATA_MALCHEZAAR                 = 9,
    DATA_NIGHTBANE                  = 10,
};

Unit* GetKaraBoss(Player* bot, KaraBosses boss);
// Barnes spawns the Opera cast non-attackable, and the play's release clears the flag, so an
// actor counts only once released, as the script itself tests.
Unit* GetOperaActor(Player* bot, std::string_view name);

// Shamans drop Tremor Totem while a Spectral Charger, the Big Bad Wolf, Dorothee or Roar is
// fighting, or Nightbane is on the ground.
bool IsKarazhanFearCasterActive(Player* bot);

// Trash

// A stun stops Warp Breach for good if it lands during the 1 s cast; interrupts do nothing.
// Each stunner waits its own delay after the cast starts, so the first stun ends that cast and the
// others go to a warp still casting.
inline constexpr uint32 WARP_BREACH_MIN_REACTION_MS = 200;
inline constexpr uint32 WARP_BREACH_MAX_REACTION_MS = 300; // War Stomp's 0.5 s cast still lands
// From here a stunner on the warp casts nothing else, keeping its global cooldown free.
inline constexpr float MANA_WARP_STAND_BY_HEALTH_PCT = 15.0f;
// Stun reaches, center to center
inline constexpr float MELEE_STUN_REACH = 5.0f;
inline constexpr float HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float SHADOWFURY_REACH = 30.0f;
inline constexpr float WAR_STOMP_RADIUS = 8.0f;
// A stunner out of reach walks to this far inside it.
inline constexpr float MANA_WARP_STUN_APPROACH_MARGIN = 1.0f;
inline constexpr float MANA_WARP_PACKS_RADIUS = 60.0f;
// The nine Mana Warps stand in four packs within 30 yd of this point.
inline Position const MANA_WARP_PACKS_CENTER = { -11326.0f, -1781.0f, 179.8f };

bool IsNearManaWarpPacks(Player* bot);
// Stunners: Hammer of Justice, Concussion Blow or Shockwave, Bash, Kidney Shot, Maim, Shadowfury,
// War Stomp. 0 for a bot with none.
float GetManaWarpStunReach(Player* bot);
// Rogues and cat druids stun only their target, which holds their combo points.
Unit* GetManaWarpToStun(PlayerbotAI* botAI, uint32 reactionMs);
// Tanks and healers stun without standing by.
Unit* GetManaWarpToStunNext(PlayerbotAI* botAI);

// Attumen the Huntsman
// Shadow Cleave jumps from the tank to two more players within 10 yd in front of Attumen.
inline constexpr float SHADOW_CLEAVE_JUMP_RADIUS = 10.0f;
// Intangible Presence curses every player within 10 yd of Attumen.
inline constexpr float INTANGIBLE_PRESENCE_RADIUS = 10.0f;
inline constexpr float ATTUMEN_HAZARD_CLEARANCE = 1.0f;
inline constexpr uint32 ATTUMEN_MOUNTED_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
// Behind mounted Attumen, center to center. His CombatReach is 2.25 yd; a player's is 1.5, or 1.95
// with Bloodlust. Charge skips a player within 8 yd edge to edge (both reaches added): 11.75 yd
// center to center, 12.2 with Bloodlust. Hunter shots have no stored minimum range, so a shot
// fails only within melee range: 5.08 yd, 5.53 with Bloodlust. Hunters stand between the two.
inline constexpr float ATTUMEN_STACK_DISTANCE = 3.0f;
inline constexpr float ATTUMEN_HUNTER_STACK_DISTANCE = 8.5f;
inline constexpr float ATTUMEN_STACK_TOLERANCE = 0.3f;
inline Position const ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };

// Midnight turns invisible when mounted Attumen is summoned.
bool IsAttumenMounted(Unit* midnight);
// Both Attumens share a name. For up to 1 s after the merge, the unmounted one can be found
// instead, and this returns null.
Unit* GetMountedAttumen(Player* bot);
// The bot's position behind mounted Attumen, or nothing while the bot is within tolerance of it.
std::optional<Position> GetAttumenStackPosition(Player* bot, Unit* attumen);
// For all but the main tank, for 5 s from when Attumen mounts Midnight.
bool IsAttumenDpsHeld(Player* bot);

// Moroes
// Guests can't leave his room: Moroes evades if one does.
inline constexpr float MOROES_GUEST_SEARCH_RADIUS = 100.0f;

// Maiden of Virtue
inline constexpr float MAIDEN_OF_VIRTUE_POSITION_TOLERANCE = 1.5f;
// She stops 6.8 yd from a tank she follows, so the tank aims this far past her position.
inline constexpr float MAIDEN_OF_VIRTUE_TANK_LEAD_DISTANCE = 7.0f;
// Holy Ground hits a player within 12 yd of her center, and breaks Repentance.
inline constexpr float HOLY_GROUND_RADIUS = 12.0f;
inline constexpr float MAIDEN_REPENTANCE_BREAK_CLEARANCE = 1.0f;
// Her spawn point; the ranged positions are a ring around it.
inline Position const MAIDEN_OF_VIRTUE_POSITION = { -10945.881f, -2103.782f, 92.712f };
inline std::array const MAIDEN_OF_VIRTUE_RANGED_POSITIONS = {
    Position{ -10959.242f, -2119.617f, 92.180f }, // SE (opposite entrance)
    Position{ -10944.495f, -2123.857f, 92.180f }, // E
    Position{ -10966.017f, -2105.288f, 92.175f }, // S
    Position{ -10931.178f, -2116.580f, 92.179f }, // NE
    Position{ -10960.912f, -2090.437f, 92.179f }, // SW
    Position{ -10925.828f, -2102.425f, 92.180f }, // N
    Position{ -10947.590f, -2082.815f, 92.180f }, // W
    Position{ -10933.089f, -2088.502f, 92.180f }, // NW (entrance)
};

// Repentance stuns every healer at once; the tank takes her to the nearest one.
Player* GetNearestRepentantHealer(Player* bot, Unit* maiden);

// The Big Bad Wolf
struct BigBadWolfRunPlan
{
    uint8 index = 0;
    int32 direction = 1;
};

// The run turns for the next point when this close to the current one: one step.
inline constexpr float BIG_BAD_WOLF_RUN_TURN_DISTANCE = 3.5f;
// No step for this long means a new Hood or the end of a fear or stun, so the run is replanned.
inline constexpr uint32 BIG_BAD_WOLF_RUN_RESTART_MS = 1000;
inline constexpr int32 BIG_BAD_WOLF_PROTECT_REACTION_MS = 500;
inline constexpr float HAND_OF_PROTECTION_RANGE = 30.0f;
inline constexpr float POWER_WORD_SHIELD_RANGE = 40.0f;
inline constexpr float INTERVENE_MIN_RANGE = 8.0f;
inline constexpr float INTERVENE_MAX_RANGE = 25.0f;
// Hand of Protection (rank 3) lasts 10 s, so it waits for the end of Hood or a target in trouble:
// one hit does 6.8-9.6k with Hood's armour at 0.
inline constexpr int32 HAND_OF_PROTECTION_DURATION_MS = 10 * IN_MILLISECONDS;
inline constexpr float BIG_BAD_WOLF_PROTECT_HEALTH_PCT = 50.0f;
// He stays at the stage edge between Hoods; from mid-stage he would catch any run.
inline Position const BIG_BAD_WOLF_TANK_POSITION = { -10913.391f, -1773.508f, 90.477f };
// Round the stage edge, on the stage's own axes, clear of the trees and the backdrop.
inline std::array const BIG_BAD_WOLF_RUN_POSITIONS = {
    Position{ -10873.346f, -1767.691f, 90.477f },
    Position{ -10873.569f, -1760.625f, 90.477f },
    Position{ -10876.827f, -1753.725f, 90.477f },
    Position{ -10883.397f, -1749.476f, 90.477f },
    Position{ -10893.279f, -1747.877f, 90.477f },
    Position{ -10901.284f, -1747.492f, 90.477f },
    Position{ -10908.019f, -1749.260f, 90.477f },
    Position{ -10912.599f, -1754.285f, 90.477f },
    Position{ -10913.570f, -1761.219f, 90.477f },
    Position{ -10912.652f, -1768.362f, 90.477f },
    Position{ -10908.455f, -1775.869f, 90.477f },
    Position{ -10899.897f, -1780.339f, 90.477f },
    Position{ -10888.966f, -1781.551f, 90.477f },
    Position{ -10879.527f, -1778.071f, 90.477f },
};

bool IsLittleRedRidingHood(Player* bot);
uint8 GetNextBigBadWolfRunIndex(uint8 index, int32 direction);
// He runs straight at the bot, cutting inside any route, so a poor start runs into him. This
// forecasts the chase for every point and direction, and returns the one that keeps him furthest
// away at his closest.
BigBadWolfRunPlan GetBigBadWolfRunPlan(Player* bot, Unit const& wolf, float seconds);
// His victim, if it is on Hood and his swings can still hurt it.
Player* GetBigBadWolfRedHood(Unit* wolf);
// A human Hood target that isn't a tank, within his melee range, near the end of Hood or below
// half health. Bots have the speed cheat.
Player* GetBigBadWolfTargetToProtect(Unit* wolf);

// Romulo and Julianne
// Once one is down, the other must die within 10 s. The skull moves to the one with higher health
// only once it is this far ahead.
inline constexpr float ROMULO_AND_JULIANNE_HEALTH_GAP_PCT = 10.0f;

// Wizard of Oz
inline constexpr float SCORCH_RANGE = 30.0f;
// A mage out of range walks to this far inside it; Strawman moves fast.
inline constexpr float SCORCH_APPROACH_MARGIN = 2.0f;
inline std::array const OZ_TARGETS = {
    "dorothee",
    "tito",
    "roar",
    "strawman",
    "tinhead",
};

// The Curator
// Arcing Sear jumps 13 yd center to center between players, so this spread only cuts its jumps.
inline constexpr float THE_CURATOR_RANGED_SPREAD_DISTANCE = 7.0f;
inline Position const THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };

// Terestian Illhoof
// Demon Chains exist only while a group member is sacrificed.
bool IsTerestianSacrificeActive(Player* bot);

// Shade of Aran
// Hits within 21 yd of his center, no reach added.
inline constexpr float ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS = 21.0f;
inline constexpr float ARAN_ARCANE_EXPLOSION_CLEARANCE = ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS + 1.0f;
// The pull lands everyone on his spot, so each bot takes its own way out.
inline constexpr float ARAN_PULLED_ONTO_BOSS_DISTANCE = 1.0f;
inline constexpr uint32 ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS = 16;
// Hits within 9 yd of his center, no reach added.
inline constexpr float ARAN_COUNTERSPELL_HAZARD_RADIUS = 9.0f;
// Ranged stand here, center to center: outside Counterspell, and inside most Blizzard patches.
inline constexpr float ARAN_RANGED_MIN_DISTANCE = ARAN_COUNTERSPELL_HAZARD_RADIUS + 1.0f;
inline constexpr float ARAN_RANGED_MAX_DISTANCE = 13.0f;
inline constexpr float ARAN_RANGED_BAND_TOLERANCE = 0.5f;
inline constexpr uint32 ARAN_ELEMENTALS_CACHE_INTERVAL_MS = 1000;
// A patch reaches 13 yd plus the player's reach.
inline constexpr float ARAN_BLIZZARD_HAZARD_RADIUS = 13.0f;
inline constexpr float ARAN_ROOM_SEARCH_RADIUS = 60.0f;
// Ranged run here from a patch, measured from the room center. The train goes clockwise and stops
// short of a full lap, so it doesn't come back to ground behind its oldest patch.
inline constexpr float ARAN_BLIZZARD_ESCAPE_DISTANCE = ARAN_RANGED_MIN_DISTANCE + 0.5f;
inline constexpr float ARAN_BLIZZARD_ESCAPE_ANGLE = static_cast<float>(M_PI) * 40.0f / 180.0f;
// A ring reaches 3 yd plus the player's reach. Leaving it explodes, and so does crossing it.
inline constexpr float ARAN_FLAME_WREATH_RING_HAZARD_RADIUS = 3.0f;
inline constexpr float ARAN_FLAME_WREATH_REACH_CLEARANCE = 5.0f;
// Targets are picked when the cast ends, so everyone has to be standing still by then.
inline constexpr int32 ARAN_FLAME_WREATH_STOP_LEAD_MS = 1500;
inline constexpr int32 ARAN_FLAME_WREATH_RING_DURATION_MS = 20000;
// No one moves this soon after the rings land, which also outlasts the ring value's cache.
inline constexpr int32 ARAN_FLAME_WREATH_SETTLE_MS = 1000;
inline constexpr uint32 ARAN_FLAME_WREATH_RING_CACHE_INTERVAL_MS = 500;
// He blinks here before each super spell. Blizzard patches land in a ring around it.
inline Position const ARAN_ROOM_CENTER = { -11164.8f, -1912.0f, 232.1f };

bool IsAranCastingArcaneExplosion(Unit* aran);
std::optional<Position> GetBlizzardEscapePosition(Player* bot);
bool IsFlameWreathInProgress(PlayerbotAI* botAI, Unit* aran);
bool CanReachTargetDuringFlameWreath(PlayerbotAI* botAI, Unit* aran);
GuidVector FindUnbanishedConjuredElementals(Player* bot);

// Netherspite

enum class NetherspiteBeam : uint8
{
    Red,
    Green,
    Blue,
};

struct NetherspiteBeamData
{
    NetherspiteBeam beam;
    KaraNpcs portal;
    KaraSpells buff;
    KaraSpells exhaustion;
};

struct NetherspiteBeamState
{
    std::optional<Position> portal;
    ObjectGuid blocker;
    ObjectGuid next;
    Position blockerPosition;
    Position nextPosition;
};

struct NetherspiteBeamOffset
{
    float along;
    float off;
};

struct NetherspiteBanishSlot
{
    ObjectGuid guid;
    Position position;
};

// In the order the tracker picks: an empty green beam heals him 4000 a second per stack, so green
// picks before blue.
inline constexpr std::array NETHERSPITE_BEAMS = {
    NetherspiteBeamData{
        NetherspiteBeam::Red, KaraNpcs::NPC_RED_PORTAL, KaraSpells::SPELL_RED_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_RED,
    },
    NetherspiteBeamData{
        NetherspiteBeam::Green, KaraNpcs::NPC_GREEN_PORTAL, KaraSpells::SPELL_GREEN_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN,
    },
    NetherspiteBeamData{
        NetherspiteBeam::Blue, KaraNpcs::NPC_BLUE_PORTAL, KaraSpells::SPELL_BLUE_BEAM_DEBUFF,
        KaraSpells::SPELL_NETHER_EXHAUSTION_BLUE,
    },
};

// The script takes a player within 1.5 yd of the line only if their 2D distance to the portal, less
// both reaches, beats Netherspite's. His reach is 18, so that starts 18 - 1.5 yd from his center.
inline constexpr float NETHERSPITE_BEAM_WIDTH = 1.5f;
inline constexpr float NETHERSPITE_BEAM_START = 16.5f;
inline constexpr float NETHERSPITE_BEAM_CLEARANCE = NETHERSPITE_BEAM_WIDTH + 1.5f;
// Consumption is NPC AoE on target 15: exact distance, no reach.
inline constexpr float VOID_ZONE_RADIUS = 2.0f;
inline constexpr float VOID_ZONE_CLEARANCE = VOID_ZONE_RADIUS + 1.0f;
inline constexpr float VOID_ZONE_BEAM_POSITION_CLEARANCE = VOID_ZONE_RADIUS + 0.5f;
inline constexpr float VOID_ZONE_STEP_AROUND_DISTANCE = VOID_ZONE_CLEARANCE + 0.5f;
// From one step, the way to the next 3.5 yd position can cut inside the same zone's clearance.
inline constexpr float VOID_ZONE_WIDE_STEP_AROUND_DISTANCE = VOID_ZONE_STEP_AROUND_DISTANCE + 2.0f;
inline constexpr float NETHERSPITE_BEAM_POSITION_TOLERANCE = 0.5f;
inline constexpr float NETHERSPITE_BANISH_SLOT_TOLERANCE =
    static_cast<float>(M_PI) * 6.0f / 180.0f;
// Netherbreath throws a bot about 25 yd out along its bearing, past the healers' range.
inline constexpr float NETHERSPITE_BANISH_SLOT_RADIUS_TOLERANCE = 5.0f;
// Beams start 10s into a portal phase. Between 0 and 5s, all non-tanks hold DPS. Between 5s and
// 10s, non-tanks hold DPS unless a tank is the victim. After beams spawn, nobody holds DPS.
inline constexpr uint32 NETHERSPITE_DPS_HOLD_MS = 5 * IN_MILLISECONDS;
inline constexpr uint32 NETHERSPITE_BEAMS_START_MS = 10 * IN_MILLISECONDS;
inline constexpr uint8 NETHERSPITE_BEAM_STACK_LIMIT = 25;
// The next blocker needs a moment, and a cast it can't stop, to get on the beam.
inline constexpr uint8 NETHERSPITE_NEXT_STEPS_ON_AT_STACKS = 24;
inline constexpr uint8 NETHERSPITE_POSITION_TRIES = 12;
inline constexpr float NETHERSPITE_BLOCKER_DISTANCE = 18.0f;
inline constexpr float NETHERSPITE_BLOCKER_MIN_DISTANCE = 17.0f;
inline constexpr float NETHERSPITE_HUNTER_BLOCKER_DISTANCE = 25.0f;
inline constexpr float NETHERSPITE_BEAM_SEARCH_MAX_DISTANCE = 34.0f;
inline constexpr float NETHERSPITE_NEXT_MIN_OFF_LINE = 3.5f;
inline constexpr float NETHERSPITE_NEXT_MAX_OFF_LINE = 6.0f;
inline constexpr float NETHERSPITE_NEXT_ALONG_RANGE = 2.5f;
inline constexpr float NETHERSPITE_MAX_OWN_LINE_ARC = static_cast<float>(M_PI) * 60.0f / 180.0f;
inline constexpr float NETHERSPITE_MAX_HEALER_ARC = static_cast<float>(M_PI) * 170.0f / 180.0f;
inline constexpr float NETHERSPITE_HEALER_TANK_RANGE = 38.0f;
inline constexpr float NETHERSPITE_BOT_SPEED = 7.0f;

bool IsBanishPhase(Unit* netherspite);
// Cat Form turns a druid's power to energy, so a druid goes by its strategies: cat and bear are not
// mana users, balance and resto are.
bool IsManaUser(Player* player);
// Holds for 5 s from each portal phase, then until a tank is his victim. At 10 s the red beam's
// threat settles it. The threat reset leaves him on the first player to make any threat.
bool IsNetherspiteDpsHeld(Player* bot, Unit* netherspite);
bool HasNetherspiteBeamStackLimit(Player* player, NetherspiteBeam beam);
uint8 GetNetherspiteBeamStacks(Player* player, NetherspiteBeamData const& data);
bool AreNetherspitePortalsKnown(KaraInstanceState const& state);
// Includes the red blocker.
bool IsNetherspiteBlockerOrNext(KaraInstanceState const& state, ObjectGuid guid);
Position const* FindNetherspiteBlockerPosition(KaraInstanceState const& state, ObjectGuid guid);
Position const* FindNetherspiteNextPosition(KaraInstanceState const& state, ObjectGuid guid);
NetherspiteBanishSlot const* FindNetherspiteBanishSlot(
    KaraInstanceState const& state, ObjectGuid guid);
float GetNetherspiteBanishRadius(Player* player);
void YellNetherspiteBeamText(Player* player, std::string const& key, std::string const& text);
void YellNetherspiteBeamHandover(Player* leaving, Player* blocking, NetherspiteBeam beam);
// Along the line from his center, and to the side, positive counterclockwise.
NetherspiteBeamOffset GetNetherspiteBeamOffset(
    Unit const& netherspite, Position const& portal, float x, float y);
bool IsOnNetherspiteBeam(Unit* netherspite, Position const& portal, Unit* unit);
Position GetPositionOnNetherspiteBeam(
    Unit const& netherspite, Position const& portal, float distance);
bool IsClearOfVoidZones(KaraInstanceState const& state, float x, float y, float clearance);
// Within the clearance of a void zone on the way. A bot already in one can leave it any way, and a
// bot inside the clearance can pass it but not enter it.
bool CrossesVoidZone(KaraInstanceState const& state, Position const& from, Position const& to);
// The blocker's position on the line: the clear point nearest where it wants to stand.
float FindNetherspiteBeamPositionDistance(
    Unit const& netherspite, Position const& portal, KaraInstanceState const& state,
    Player* blocker);
// The side of the green or blue beam that faces the other one, away from red: +1 counterclockwise.
float GetNetherspiteInnerSide(
    Unit const& netherspite, KaraInstanceState const& state, NetherspiteBeam beam);
// A tank just stays off the beams and out of void zones. Everyone else stands between green and
// blue, on the side without red, at attack range. Healers keep the red tank in range.
bool IsBetweenNetherspiteBeams(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state, float x, float y);
// In the area, and the way there crosses no void zone, and no beam that will be up by the time the
// bot arrives.
bool CanMoveBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state, float x, float y);
// A random position in the area, then the nearest safe position if there is none. Neither crosses a
// void zone, or a beam that will be up by the time the bot arrives.
std::optional<Position> FindPositionBetweenNetherspiteBeams(
    Player* bot, Unit* netherspite, KaraInstanceState const& state);
// The destination, or a step around the first void zone on the way there. None if neither side of
// that zone is clear.
std::optional<Position> GetStepAroundVoidZones(
    Player* bot, Unit* netherspite, KaraInstanceState const& state,
    Position const& destination);
// Where a reach toward the target stops: range plus both reaches from it, as ReachCombatTo has it.
Position GetReachEndPosition(Unit const& bot, Unit const& target, float range);

// Prince Malchezaar
// Hellfire and Shadow Nova are NPC AoE on target 16 and 15: exact distance, no reach.
inline constexpr float HELLFIRE_RADIUS = 20.0f;
inline constexpr float HELLFIRE_CLEARANCE = HELLFIRE_RADIUS + 1.0f;
inline constexpr float SHADOW_NOVA_RADIUS = 30.0f;
inline constexpr float SHADOW_NOVA_MOVE_DISTANCE = SHADOW_NOVA_RADIUS + 1.0f;
inline constexpr float SHADOW_NOVA_CLEARANCE = SHADOW_NOVA_RADIUS + 2.0f;
// His melee range for a player: his reach 4 + 1.5 + 4/3.
inline constexpr float MALCHEZAAR_MELEE_RANGE = 6.83f;
// With him this clear of a landed infernal, all of his melee ring is clear of it.
inline constexpr float MALCHEZAAR_RING_CLEARANCE = HELLFIRE_CLEARANCE + MALCHEZAAR_MELEE_RANGE;
// He ends about this far behind a tank walking him away.
inline constexpr float MALCHEZAAR_TRAIL_DISTANCE = 6.0f;
// The model the infernal has until Infernal Rain lands on it.
inline constexpr uint32 INFERNAL_MODEL_INVISIBLE = 11686;
inline constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
// The movers' arrival distance, and how far short of a spot a straight walk may stop.
inline constexpr float MALCHEZAAR_ARRIVAL_DISTANCE = 0.5f;
inline constexpr uint32 LANDED_INFERNALS_CACHE_INTERVAL_MS = 200;

std::vector<Position> FindLandedInfernals(Player* bot);
std::vector<Position> GetLandedInfernals(PlayerbotAI* botAI);
bool IsClearOfInfernals(
    Position const& position, std::vector<Position> const& infernals, float clearance);
// An infernal the line starts inside doesn't count, since the bot is walking out of it.
bool IsLineClearOfInfernals(
    Position const& from, Position const& to, std::vector<Position> const& infernals);
bool CanWalkStraightTo(Player* bot, Position const& destination);
// The nearest position on rings around origin that passes isGood and can be walked to in a straight
// line. A line clear of landed infernals comes first; with mayCross, any line after that.
template <typename IsGood>
std::optional<Position> FindMalchezaarPosition(
    Player* bot, Position const& origin, std::vector<Position> const& infernals,
    float maxDistance, bool mayCross, IsGood const& isGood)
{
    constexpr float ringStep = 2.0f;
    constexpr uint8 angleCount = 32;
    constexpr uint8 maxWalkChecks = 64;

    int const ringCount = static_cast<int>(maxDistance / ringStep);
    uint8 walkChecks = 0;
    for (bool const clearLine : { true, false })
    {
        if (!clearLine && !mayCross)
            break;

        for (int ring = 1; ring <= ringCount; ++ring)
        {
            float const distance = ring * ringStep;
            for (uint8 i = 0; i < angleCount; ++i)
            {
                float const angle = 2.0f * static_cast<float>(M_PI) * i / angleCount;
                Position const candidate(
                    origin.GetPositionX() + distance * std::cos(angle),
                    origin.GetPositionY() + distance * std::sin(angle), origin.GetPositionZ());

                if (!isGood(candidate) ||
                    (clearLine && !IsLineClearOfInfernals(origin, candidate, infernals)))
                {
                    continue;
                }

                if (walkChecks >= maxWalkChecks)
                    return std::nullopt;

                ++walkChecks;
                if (CanWalkStraightTo(bot, candidate))
                    return candidate;
            }
        }
    }

    return std::nullopt;
}
// The nearest clear position in range of the bot's target: for a ranged bot on a ring just out of
// Shadow Nova, for melee on the target's melee ring. Failing that, or with him not found (a rezzed
// bot isn't on his threat list yet), the nearest clear position anywhere.
std::optional<Position> FindMalchezaarClearPosition(
    PlayerbotAI* botAI, Unit const* malchezaar, std::vector<Position> const& infernals);
// Without him, only the infernals are tested.
bool IsClearOfMalchezaarHazards(
    Player* bot, Unit const* malchezaar, std::vector<Position> const& infernals,
    Position const& position);
// Sets clearance to what the position is clear by: enough for his melee ring and his trail if it
// can, then less.
std::optional<Position> FindMalchezaarTankPosition(
    Player* bot, std::vector<Position> const& infernals, float& clearance);

// Nightbane
enum class NightbaneFlightStage : uint8
{
    TakeOff,
    Hover,   // At the flight position, where every air-phase spell is cast
    Landing, // From leaving the flight position, unless he comes back for another air phase
};

inline constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
inline constexpr float NIGHTBANE_GROUND_Z = 91.473f;
inline constexpr uint32 NIGHTBANE_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
inline constexpr float CHARRED_EARTH_REACH = 11.5f; // 10y patch + 1.5y player reach
inline constexpr float CHARRED_EARTH_CLEARANCE = 1.5f;
// Rain of Bones ticks land up to 8y from its center and hit 6y around that, with no player reach.
inline constexpr float RAIN_OF_BONES_REACH = 14.0f;
inline constexpr float RAIN_OF_BONES_CLEARANCE = 1.0f;
// Blizzard, Hurricane, Rain of Fire and Volley
inline constexpr float PLAYER_GROUND_AOE_RADIUS = 8.0f;
// Below this on the ground, he has landed for the last time: crossing 25% sends him up at once.
inline constexpr float NIGHTBANE_FINAL_PHASE_HEALTH_PCT = 20.0f;
// The terrace is a ring around the dome. Terrace angles are measured around the dome in world
// coordinates, running from the west end through the south side to the east end.
inline constexpr float TERRACE_INNER_RADIUS = 36.0f; // the dome's wall is at 33.5-35.8y
inline constexpr float TERRACE_OUTER_RADIUS = 51.0f;
inline constexpr float TERRACE_ARC_RADIUS = 47.856f;
inline constexpr float TERRACE_ARC_START = 0.2418f;
inline constexpr float TERRACE_ARC_END = 4.9241f;
// All floor, and 10.5y from the outer wall, so skeletons appearing up to 10y from a Rain of Bones
// on the stack can't reach it.
inline constexpr float NIGHTBANE_STACK_RADIUS = 39.0f;
// Within 50y of the flight position. Fireball Barrage hits players more than 52y from his center.
inline constexpr float NIGHTBANE_FLIGHT_STACK_MIN_ANGLE = 1.2566f;
inline constexpr float NIGHTBANE_FLIGHT_STACK_MAX_ANGLE = 3.0020f;
inline constexpr float NIGHTBANE_FLIGHT_STACK_HOME_ANGLE = 2.0944f;

inline Position const NIGHTBANE_FLIGHT_POSITION = { -11160.125f, -1870.683f, 97.739f };
inline Position const TERRACE_DOME_CENTER = { -11126.015f, -1925.271f, 91.473f };

// Normalized to the range starting at TERRACE_ARC_START.
float GetTerraceAngle(Position const& position);
// The destination, unless the straight line to it cuts through the dome; then a step round the
// ring toward it.
Position GetTerraceWalkPosition(Position const& from, Position const& destination);
// Searches the stack circle in both directions from startAngle, away from his landing position
// first, and returns the nearest position within the limits for which isClear is true.
template <typename IsClear>
std::optional<Position> FindNightbaneStackPosition(
    float startAngle, float minAngle, float maxAngle, IsClear const& isClear)
{
    constexpr float angleStep = 0.0436f; // 2.5 degrees, or 1.7y

    for (uint8 i = 0; i * angleStep <= maxAngle - minAngle; ++i)
    {
        for (float const direction : { -1.0f, 1.0f })
        {
            float const angle = startAngle + direction * i * angleStep;
            if (angle < minAngle || angle > maxAngle)
                continue;

            Position const position(
                TERRACE_DOME_CENTER.GetPositionX() + NIGHTBANE_STACK_RADIUS * std::cos(angle),
                TERRACE_DOME_CENTER.GetPositionY() + NIGHTBANE_STACK_RADIUS * std::sin(angle),
                NIGHTBANE_GROUND_Z);
            if (isClear(position))
                return position;
        }
    }

    return std::nullopt;
}
bool IsClearOfCharredEarth(Position const& position, std::vector<Position> const& charredEarths);
// True for a unit off the ring (the terrace is only a plain ring within the arc) or below it.
bool IsOutsideNightbaneTerrace(Unit* unit);
bool IsNightbaneFlying(Unit* nightbane);
// True in flight until he leaves the flight position. The landing that follows has no air-phase
// spells, and bots are free to follow their master then.
bool IsNightbaneFlightPhaseActive(Unit* nightbane);
// In flight, and for all but the main tank until 8s after he is on the ground.
bool IsNightbaneDpsHeld(Player* bot, Unit* nightbane);
// Where the current Rain of Bones channel is aimed, or nullptr. A rain doesn't follow its target.
Position const* GetRainOfBonesCenter(Unit* nightbane);

// Shared encounter state

struct KaraInstanceState
{
    std::optional<uint32> attumenDpsHoldTimer;
    std::optional<uint32> netherspiteDpsHoldTimer;
    std::array<NetherspiteBeamState, NETHERSPITE_BEAMS.size()> netherspiteBeams;
    std::vector<Position> netherspiteVoidZones;
    std::vector<NetherspiteBanishSlot> netherspiteBanishSlots;
    std::optional<uint32> nightbaneDpsHoldTimer;
    std::optional<NightbaneFlightStage> nightbaneFlightStage;
    std::optional<Position> nightbaneFlightStackPosition;
    std::optional<Position> nightbaneRainOfBonesCenter;
};

KaraInstanceState& KaraState(uint32 instanceId);
KaraInstanceState const* FindKaraState(uint32 instanceId);
bool KaraResetInstance(uint32 instanceId);

}

#endif
