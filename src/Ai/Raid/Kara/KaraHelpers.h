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
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace KaraHelpers
{

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

    // The Curator
    SPELL_CURATOR_EVOCATION       = 30254,

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
    SPELL_RAIN_OF_BONES           = 37091,

    // Shaman
    SPELL_TREMOR_TOTEM            = 8143,
    SPELL_GROUNDING_TOTEM         = 8177,
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
    DATA_CHESS_EVENT                = 8,
    DATA_MALCHEZAAR                 = 9,
    DATA_NIGHTBANE                  = 10,
};

Unit* GetKaraBoss(Player* bot, KaraBosses boss);
// Barnes spawns the Opera cast non-attackable, and the play's release clears the flag, so an
// actor counts only once released, as the script itself tests.
Unit* GetOperaActor(Player* bot, std::string_view name);

bool IsSafePosition (float x, float y, std::vector<Unit*> const& hazards, float hazardRadius);

// Shamans drop Tremor Totem while a Spectral Charger, the Big Bad Wolf, Dorothee or Roar is
// fighting, or Nightbane is on the ground.
bool IsKarazhanFearCasterActive(Player* bot);

// Trash

// A stun stops Warp Breach for good if it lands during the 1 s cast; interrupts do nothing.
// Each stunner waits its own delay after the cast starts, so the first stun ends that cast and the
// others go to a warp still casting.
inline constexpr uint32 WARP_BREACH_MIN_REACTION_MS = 200;
inline constexpr uint32 WARP_BREACH_MAX_REACTION_MS = 600;
// From here a stunner on the warp casts nothing else, keeping its global cooldown free.
inline constexpr float MANA_WARP_STUNNER_HOLD_HEALTH_PCT = 15.0f;
// Stun reaches, centre to centre
inline constexpr float MELEE_STUN_REACH = 5.0f;
inline constexpr float HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float SHADOWFURY_REACH = 30.0f;
inline constexpr float WAR_STOMP_RADIUS = 8.0f;
// A stunner out of reach walks to this far inside it.
inline constexpr float MANA_WARP_STUN_APPROACH_MARGIN = 1.0f;
// The nine Mana Warps stand in four packs within 30 yd of this point.
inline Position const MANA_WARP_PACKS_CENTER = { -11326.0f, -1781.0f, 179.8f };
inline constexpr float MANA_WARP_PACKS_RADIUS = 60.0f;

bool IsNearManaWarpPacks(Player* bot);
// Stunners: Hammer of Justice, Concussion Blow or Shockwave, Bash, Kidney Shot, Maim, Shadowfury,
// War Stomp. 0 for a bot with none.
float GetManaWarpStunReach(Player* bot);
// Rogues and cat druids stun only their target, which holds their combo points.
Unit* GetManaWarpToStun(PlayerbotAI* botAI, uint32 reactionMs);
// Tanks keep tanking and stun without holding.
Unit* GetManaWarpToHoldFor(PlayerbotAI* botAI);

// Attumen the Huntsman
inline Position const ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };
// Shadow Cleave jumps from the tank to two more players within 10 yd in front of Attumen.
inline constexpr float SHADOW_CLEAVE_JUMP_RADIUS = 10.0f;
// Intangible Presence curses every player within 10 yd of Attumen.
inline constexpr float INTANGIBLE_PRESENCE_RADIUS = 10.0f;
inline constexpr float ATTUMEN_HAZARD_CLEARANCE = 1.0f;
// Behind mounted Attumen, centre to centre. Charge goes to a player more than 8 yd away edge to
// edge (11 yd centre to centre), and Auto Shot fails within 5 yd.
inline constexpr float ATTUMEN_STACK_DISTANCE = 2.0f;
inline constexpr float ATTUMEN_HUNTER_STACK_DISTANCE = 8.0f;
inline constexpr float ATTUMEN_STACK_TOLERANCE = 1.5f;

// Midnight turns invisible when mounted Attumen is summoned.
bool IsAttumenMounted(Unit const& midnight);
// Both Attumens share a name. For up to 1 s after the merge, the unmounted one can be found
// instead, and this returns null.
Unit* GetMountedAttumen(Player* bot);

// Maiden of Virtue
// Her spawn point; the ranged spots are a ring around it.
inline Position const MAIDEN_OF_VIRTUE_POSITION = { -10945.881f, -2103.782f, 92.712f };
inline constexpr float MAIDEN_OF_VIRTUE_POSITION_TOLERANCE = 1.5f;
// She stops 6.8 yd from a tank she follows, so the tank aims this far past her spot.
inline constexpr float MAIDEN_OF_VIRTUE_TANK_LEAD_DISTANCE = 7.0f;
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
// Holy Ground hits a player within 12 yd of her centre, and breaks Repentance.
inline constexpr float HOLY_GROUND_RADIUS = 12.0f;
inline constexpr float MAIDEN_REPENTANCE_BREAK_CLEARANCE = 1.0f;

// Repentance stuns every healer at once; the tank takes her to the nearest one.
Player* GetNearestRepentantHealer(Player* bot, Unit const& maiden);

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
// A settled chase stays further off than this, so a run going well doesn't use the long cooldown.
inline constexpr float BIG_BAD_WOLF_PROTECT_DISTANCE = 8.5f;
inline constexpr int32 BIG_BAD_WOLF_PROTECT_REACTION_MS = 500;
inline constexpr float HAND_OF_PROTECTION_RANGE = 30.0f;
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

uint8 GetNextBigBadWolfRunIndex(uint8 index, int32 direction);
// He runs straight at the bot, cutting inside any route, so a poor start runs into him. This
// forecasts the chase for every point and direction, and returns the one that keeps him furthest
// away at his closest.
BigBadWolfRunPlan GetBigBadWolfRunPlan(Player* bot, Unit const& wolf, float seconds);
// His victim, if it is a non-tank on Hood that he has caught up with.
Player* GetBigBadWolfTargetToProtect(Unit const& wolf);

// Romulo and Julianne
// Once one is down, the other must die within 10 s. The skull moves to the healthier one only
// once it is this far ahead.
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
// Arcing Sear jumps 13 yd centre to centre between players, so this spread only cuts its jumps.
inline constexpr float THE_CURATOR_RANGED_SPREAD_DISTANCE = 7.0f;
inline Position const THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };

// Shade of Aran
// He blinks here before each super spell. Blizzard patches land in a ring around it.
inline Position const ARAN_ROOM_CENTER = { -11164.8f, -1912.0f, 232.1f };
// Hits within 21 yd of his centre, no reach added.
inline constexpr float ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS = 21.0f;
inline constexpr float ARAN_ARCANE_EXPLOSION_CLEARANCE = ARAN_ARCANE_EXPLOSION_HAZARD_RADIUS + 1.0f;
// Hits within 9 yd of his centre, no reach added.
inline constexpr float ARAN_COUNTERSPELL_HAZARD_RADIUS = 9.0f;
// Ranged stand here, centre to centre: outside Counterspell, and inside most Blizzard patches.
inline constexpr float ARAN_RANGED_MIN_DISTANCE = ARAN_COUNTERSPELL_HAZARD_RADIUS + 1.0f;
inline constexpr float ARAN_RANGED_MAX_DISTANCE = 13.0f;
// A patch reaches 13 yd plus the player's reach.
inline constexpr float ARAN_BLIZZARD_HAZARD_RADIUS = 13.0f;
inline constexpr float ARAN_ROOM_SEARCH_RADIUS = 60.0f;
// Ranged run here from a patch, measured from the room centre. The train goes clockwise and stops
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

bool IsAranCastingArcaneExplosion(Unit* aran);
std::optional<Position> GetBlizzardEscapeSpot(Player* bot);
bool IsFlameWreathInProgress(Player* bot, Unit* aran);
bool CanReachTargetDuringFlameWreath(PlayerbotAI* botAI, Unit* aran);

// Netherspite
bool IsBanishPhase(Unit* netherspite);
std::vector<Player*> GetRedBlockers(Player* bot);
std::vector<Player*> GetBlueBlockers(Player* bot);
std::vector<Player*> GetGreenBlockers(Player* bot);
std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot);
std::vector<Unit*> GetAllVoidZones(Player* bot);
bool FindBeamPosition(
    Unit* netherspite, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos);

// Prince Malchezaar
std::vector<Unit*> GetSpawnedInfernals(Player* bot);
bool IsStraightPathSafe(
    float startX, float startY, float targetX, float targetY,
    std::vector<Unit*> const& hazards, float hazardRadius);
bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY);

// Nightbane
inline constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
inline constexpr float NIGHTBANE_GROUND_Z = 91.473f;
inline Position const TERRACE_DOME_CENTER = { -11126.015f, -1925.271f, 91.473f };
inline Position const TERRACE_EAST_END    = { -11115.958f, -1972.058f, 91.457f };
inline Position const TERRACE_WEST_END    = { -11077.521f, -1913.315f, 91.471f };
inline std::array const NIGHTBANE_FLIGHT_STACK_POSITIONS = {
    Position{ -11156.233f, -1888.353f, 91.473f },  // primary
    Position{ -11149.115f, -1897.154f, 91.473f },  // backup in case of charred earth
};
inline std::array const NIGHTBANE_RAIN_OF_BONES_POSITIONS = {
    Position{ -11166.516f, -1901.405f, 91.473f },  // primary
    Position{ -11158.752f, -1909.394f, 91.473f },  // backup in case of charred earth
};
inline Position const NIGHTBANE_TELEPORT_POSITION = { -11159.555f, -1893.526f, 91.473f };

// Shared encounter state

struct KaraInstanceState
{
    std::optional<uint32> attumenDpsWaitTimer;
    std::optional<uint32> netherspiteDpsWaitTimer;
    std::optional<ObjectGuid> currentRedBlocker;
    std::optional<ObjectGuid> currentGreenBlocker;
    std::optional<ObjectGuid> currentBlueBlocker;
    std::optional<uint32> nightbaneDpsWaitTimer;
    std::optional<uint32> nightbaneFlightPhaseStartTimer;
};

KaraInstanceState& KaraState(uint32 instanceId);
KaraInstanceState const* FindKaraState(uint32 instanceId);
bool KaraResetInstance(uint32 instanceId);

}

#endif
