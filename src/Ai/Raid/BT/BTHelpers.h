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

class GameObject;
class Player;
class PlayerbotAI;
class SpellInfo;
class Unit;

namespace BlackTempleHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class BlackTempleSpells : uint32
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

    // Teron Gorefiend
    SPELL_SHADOW_OF_DEATH           = 40251,
    SPELL_SPIRITUAL_VENGEANCE       = 40268,

    SPELL_SPIRIT_LANCE              = 40157,
    SPELL_SPIRIT_CHAINS             = 40175,
    SPELL_SPIRIT_VOLLEY             = 40314,
    SPELL_SPIRIT_STRIKE             = 40325,

    // Gurtogg Bloodboil
    SPELL_BOSS_FEL_RAGE             = 40594,
    SPELL_PLAYER_FEL_RAGE           = 40604,
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

    // Veras Darkshadow
    SPELL_VANISH                    = 41476,

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
    SPELL_SHADOW_PRISON             = 40647,
    SPELL_CAGED                     = 40695,
    SPELL_PARASITIC_SHADOWFIEND_1   = 41917, // cast by Illidan (primary infection)
    SPELL_PARASITIC_SHADOWFIEND_2   = 41914, // cast by Shadowfiend on contact (secondary infection)

    // Druid
    SPELL_TREE_OF_LIFE              = 33891,

    // Hunter
    SPELL_FROST_TRAP                = 13809,
    SPELL_MISDIRECTION              = 35079,

    // Mage
    SPELL_SPELLSTEAL                = 30449,

    // Shaman
    SPELL_EARTHBIND_TOTEM           =  2484,
};

enum class BlackTempleNpcs : uint32
{
    // Trash
    NPC_SISTER_OF_PAIN        = 22956,
    NPC_SISTER_OF_PLEASURE    = 22964,
    NPC_SHADOWMOON_REAVER     = 22879,

    // Supremus
    NPC_SUPREMUS_VOLCANO      = 23085,

    // Shade of Akama
    NPC_ASHTONGUE_CHANNELER   = 23421,
    NPC_ASHTONGUE_SORCERER    = 23215,

    // Teron Gorefiend
    NPC_SHADOWY_CONSTRUCT     = 23111,

    // Reliquary of Souls
    NPC_ESSENCE_OF_DESIRE     = 23419,

    // Illidan Stormrage <The Betrayer>
    NPC_FLAME_OF_AZZINOTH     = 22997,
    NPC_DEMON_FIRE            = 23069,
    NPC_ILLIDAN_DB_TARGET     = 23070,
    NPC_BLAZE                 = 23259,
    NPC_FLAME_CRASH           = 23336,
    NPC_SHADOW_DEMON          = 23375,
    NPC_PARASITIC_SHADOWFIEND = 23498,
};

enum class BlackTempleItems : uint32
{
    // High Warlord Naj'entus
    ITEM_NAJENTUS_SPINE = 32408,
};

enum class BlackTempleObjects : uint32
{
    // High Warlord Naj'entus
    GO_NAJENTUS_SPINE = 185584,

    // Illidan Stormrage <The Betrayer>
    GO_SHADOW_TRAP    = 185916,
};

enum class TankPositionState : uint8
{
    MovingToTransition = 0,
    MovingToFinal      = 1,
    Positioned         = 2,
    Unknown            = 255,
};

inline constexpr uint32 BT_MAP_ID = 564;

// Misdirects onto the tank, then spends it with Steady Shot on the target.
bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);

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
inline constexpr uint32 REAVER_ABSORPTION_DURATION_MS = 15000;
inline constexpr uint32 REAVER_ABSORPTION_MIN_RECAST_MS = 30000;
inline constexpr uint32 REAVER_MAGIC_MARGIN_MS = 5000;
inline constexpr uint32 SHADOWMOON_REAVER_CACHE_INTERVAL_MS = 1000;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint32>>
    shadowmoonReaverAbsorptionStart;

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
// Casters and healers leave a Reaver while magic would build Chaotic Charge on her; everyone else
// stays, with the spells that would build it held.
void AppendShadowmoonReaverExclusions(PlayerbotAI* botAI, GuidSet& exclusions);
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

inline constexpr float SUPREMUS_VOLCANO_SEARCH_RADIUS = 40.0f;
inline constexpr uint32 SUPREMUS_VOLCANO_CACHE_INTERVAL_MS = 200;
// Tank phase only, so a Molten Flame trail chasing one ranged bot doesn't run through the rest.
inline constexpr float SUPREMUS_RANGED_SPREAD_DISTANCE = 5.0f;
// Volcanic Geyser hits within 15 yd of the volcano's centre. The 3 yd is one AI tick of running
// in, so avoidance stops a bot that reach is carrying in before the damage radius.
inline constexpr float SUPREMUS_VOLCANO_SAFE_DISTANCE = 15.0f + 3.0f;
inline constexpr float SUPREMUS_KITE_DISTANCE = 25.0f;
inline constexpr float SUPREMUS_KITE_STEP_DISTANCE = 5.0f;
// He evades when his own position leaves this box (instance_black_temple.cpp boundaries). He
// follows the kiter in a straight line and the box is convex, so a kiter inside it keeps him in.
inline constexpr float SUPREMUS_BOUNDARY_MIN_X = 556.1f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_X = 850.2f;
inline constexpr float SUPREMUS_BOUNDARY_MIN_Y = 542.0f;
inline constexpr float SUPREMUS_BOUNDARY_MAX_Y = 1001.0f;
inline constexpr float SUPREMUS_KITE_BOUNDARY_MARGIN = 10.0f;

bool IsSupremusKitePhase(Unit* supremus);
GuidVector FindSupremusVolcanoGuids(Player* bot);
std::vector<Unit*> GetSupremusVolcanoes(PlayerbotAI* botAI);
// Erupting from its 1 s cast until Volcanic Geyser ends, about 19 s of its 30.
bool IsSupremusVolcanoErupting(Unit* volcano);
bool IsInEruptingSupremusVolcano(
    std::vector<Unit*> const& volcanoes, float x, float y,
    float radius = SUPREMUS_VOLCANO_SAFE_DISTANCE);
bool IsInsideSupremusKiteBoundary(float x, float y);

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
// Channelers and sorcerers can't be tanked.
void AppendShadeOfAkamaTankExclusions(PlayerbotAI* botAI, GuidSet& exclusions);
bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY);
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);

// Teron Gorefiend

// The run from the balcony to the corner takes about 12.5 s.
inline constexpr int32 GOREFIEND_SHADOW_OF_DEATH_MOVE_MS = 15000;
inline constexpr float GOREFIEND_POSITION_TOLERANCE = 2.0f;

inline Position const GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f };
inline Position const GOREFIEND_DIE_POSITION  = { 525.709f, 377.177f, 193.203f };

// Gurtogg Bloodboil

// Bloodboil hits the 5 farthest, so three groups of five ranged take it in turn.
inline constexpr size_t GURTOGG_ROTATION_GROUP_COUNT = 3;
inline constexpr size_t GURTOGG_ROTATION_GROUP_SIZE = 5;
inline constexpr float GURTOGG_POSITION_TOLERANCE = 2.0f;
// Bewildering Strike hands him to the second tank, so everyone else stays below it.
inline constexpr float GURTOGG_THREAT_HOLD_RATIO = 0.8f;
inline constexpr uint32 GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS = 1000;

inline Position const GURTOGG_TANK_POSITION   = { 735.987f, 272.451f, 63.554f };
inline Position const GURTOGG_RANGED_POSITION = { 762.265f, 277.183f, 63.781f };
inline Position const GURTOGG_SOAKER_POSITION = { 769.348f, 280.116f, 63.780f };

Position const& GetGurtoggBloodboilPosition(Player* bot);
// 0 with fewer than two tanks on his threat list.
float FindGurtoggSecondTankThreat(PlayerbotAI* botAI);

// Reliquary of Souls

// Essence of Suffering fixates the nearest enemy every 5 s, so a tank above this health stays
// closest; other melee hold her back at melee range and ranged keep their distance.
inline constexpr float SUFFERING_TANK_MIN_HEALTH_PCT = 25.0f;
inline constexpr float SUFFERING_TANK_DISTANCE = 2.0f;
inline constexpr float SUFFERING_MELEE_POSITION_TOLERANCE = 0.25f;
inline constexpr float SUFFERING_RANGED_DISTANCE = 15.0f;
// Other dispellers wait this long into each Rune Shield, so a mage steals it when one can.
inline constexpr uint32 RUNE_SHIELD_MAGE_PRIORITY_MS = 2000;

bool IsSufferingFixateTank(Player* bot);
Position GetSufferingMeleePosition(Player* bot, Unit* suffering);
bool IsOutOfSufferingPosition(Player* bot, Unit* suffering);

// Mother Shahraz

inline Position const SHAHRAZ_TANK_POSITION       = { 960.438f, 178.989f, 192.826f };
inline Position const SHAHRAZ_TRANSITION_POSITION = { 951.327f, 179.550f, 192.550f };
inline Position const SHAHRAZ_RANGED_POSITION     = { 935.267f, 175.459f, 192.821f };

extern std::unordered_map<ObjectGuid, TankPositionState> shahrazTankStep;
TankPositionState GetShahrazTankPositionState(Player* bot);

// Illidari Council

inline constexpr float COUNCIL_FLOOR_Z_THRESHOLD = 270.000f;

inline std::array const GATHIOS_TANK_POSITIONS = {
    Position{ 662.977f, 296.246f, 271.688f },
    Position{ 636.238f, 283.719f, 271.629f },
    Position{ 655.571f, 261.377f, 271.687f },
    Position{ 673.789f, 274.139f, 271.689f },
};
inline Position const MALANDE_TANK_POSITION = { 690.590f, 299.790f, 277.443f };
inline Position const ZEREVOR_TANK_POSITION = { 686.219f, 377.644f, 271.689f };
inline std::array const ZEREVOR_HEALER_POSITIONS = {
    Position{ 661.385f, 351.219f, 271.690f },
    Position{ 667.003f, 363.768f, 271.690f },
};

extern std::unordered_map<uint32, uint32> councilDpsWaitTimer;
extern std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
extern std::unordered_map<ObjectGuid, uint8> zerevorHealStep;
inline constexpr uint32 ZEREVOR_MAGE_TANK_CACHE_INTERVAL_MS = 1000;

ObjectGuid FindZerevorMageTankGuid(Player* bot);
Player* GetZerevorMageTank(PlayerbotAI* botAI);
bool HasDangerousCouncilAura(Unit* unit);

// Illidan Stormrage <The Betrayer>

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

extern std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
extern std::unordered_map<ObjectGuid, ObjectGuid> illidanShadowTrapGuid;
extern std::unordered_map<ObjectGuid, Position> illidanShadowTrapDestination;
extern std::unordered_map<uint32, int> illidanLastPhase;
extern std::unordered_map<uint32, uint32> illidanBossDpsWaitTimer;
extern std::unordered_map<uint32, uint32> illidanFlameDpsWaitTimer;
extern std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
extern std::unordered_map<uint32, ObjectGuid> westFlameGuid;

inline constexpr uint32 ILLIDAN_WARLOCK_TANK_CACHE_INTERVAL_MS = 1000;
inline constexpr uint32 PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS = 200;

int GetIllidanPhase(Unit* illidan);
std::vector<Unit*> GetAllFlameCrashes(Player* bot);
std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot);
ObjectGuid FindIllidanWarlockTankGuid(Player* bot);
Player* GetIllidanWarlockTank(PlayerbotAI* botAI);
bool HasParasiticShadowfiend(Player* player);
Player* GetIllidanTrapperHunter(Player* bot);
ObjectGuid FindBotWithParasiticShadowfiendGuid(Player* bot);
Player* GetBotWithParasiticShadowfiend(PlayerbotAI* botAI);
struct EyeBlastDangerArea
{
    Position start;
    Position end;
    float width;
};
EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area);
GameObject* FindNearestTrap(PlayerbotAI* botAI);

}

#endif
