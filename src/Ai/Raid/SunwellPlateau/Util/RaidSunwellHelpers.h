/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLHELPERS_H
#define _PLAYERBOT_RAIDSUNWELLHELPERS_H

#include <array>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Common.h"
#include "Position.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SunwellHelpers
{

enum class SunwellSpells : uint32
{
    // Kalecgos
    SPELL_SPECTRAL_EXHAUSTION          = 44867,
    SPELL_SPECTRAL_BLAST_PORTAL        = 44866,
    SPELL_ARCANE_BUFFET                = 45018,
    SPELL_CURSE_OF_BOUNDLESS_AGONY     = 45032,
    SPELL_TELEPORT_SPECTRAL            = 46019,
    SPELL_TELEPORT_NORMAL_REALM        = 46020,
    SPELL_SPECTRAL_REALM               = 46021,

    // Brutallus
    SPELL_METEOR_SLASH                 = 45150,
    SPELL_BURN                         = 46394,

    // Felmyst
    SPELL_SUMMON_DEMONIC_VAPOR         = 45391,
    SPELL_ENCAPSULATE                  = 45661,
    SPELL_GAS_NOVA                     = 45855,
    SPELL_FELMYST_SPEED_BURST          = 45495,
    SPELL_FOG_OF_CORRUPTION            = 45582,
    SPELL_FOG_OF_CORRUPTION_CHARM      = 45717,

    // Eredar Twins
    SPELL_BLAZE                        = 45235,
    SPELL_CONFLAGRATION                = 45342,
    SPELL_FLAME_TOUCHED                = 45348,
    SPELL_FLAME_SEAR                   = 46771,

    // M'uru
    SPELL_DARKNESS                     = 45996,
    SPELL_DARKNESS_PRE_EFFECT          = 45999,
    SPELL_SHADOW_BOLT_VOLLEY           = 46082,
    SPELL_FEL_FIREBALL                 = 46101,
    SPELL_SPELL_FURY                   = 46102,
    SPELL_FLURRY                       = 46160,

    // Kil'jaeden <The Deceiver>
    SPELL_FIRE_BLOOM                   = 45641,
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS = 46605,

    // Hunter
    SPELL_MISDIRECTION                 = 35079,

    // Shaman
    SPELL_GROUNDING_TOTEM_EFFECT       = 8178,

    // Apocalypse Guard
    SPELL_INFERNAL_DEFENSE             = 46287,
};

enum class SunwellNpcs : uint32
{
    // Trash
    NPC_APOCALYPSE_GUARD        = 25593,

    // Felmyst
    NPC_FELMYST                 = 25038,
    NPC_DEMONIC_VAPOR           = 25265,
    NPC_DEMONIC_VAPOR_TRAIL     = 25267,
    // NPC_UNYIELDING_DEAD      = 25268,

    // Eredar Twins
    NPC_GRAND_WARLOCK_ALYTHESS  = 25166,

    // M'uru
    NPC_MURU                    = 25741,
    NPC_VOID_SENTINEL           = 25772,
    NPC_DARK_FIEND              = 25744,
    NPC_DARKNESS                = 25879,
    NPC_SHADOWSWORD_BERSERKER   = 25798,
    NPC_SHADOWSWORD_FURY_MAGE   = 25799,
    NPC_VOID_SPAWN              = 25824,
    NPC_ENTROPIUS               = 25840,
    NPC_SINGULARITY             = 25855,

    // Kil'jaeden <The Deceiver>
    NPC_SHIELD_ORB              = 25502,
    NPC_HAND_OF_THE_DECEIVER    = 25588,
    NPC_VOLATILE_FELFIRE_FIEND  = 25598,
    // NPC_SINISTER_REFLECTION  = 25708,
    NPC_ARMAGEDDON_TARGET       = 25735,
};

enum class SunwellObjects : uint32
{
    GO_SPECTRAL_RIFT = 187055,
    GO_BLAZE         = 187366,
};

constexpr uint32 SUNWELL_MAP_ID = 580;

// Trash

Creature* GetInfernalDefenseApocalypseGuard(Player* bot);

// Kalecgos

constexpr uint8 KALECGOS_TANK_COUNT = 3;
constexpr uint8 KALECGOS_GROUP_COUNT = 4;
constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
constexpr float KALECGOS_SPECTRAL_REALM_Z = -74.5f;

struct KalecgosRealmState
{
    uint32 lastEnterMs = 0;
    uint32 lastExitMs = 0;
    bool inSpectralRealm = false;
};

struct KalecgosEncounterState
{
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

extern const Position KALECGOS_TANK_POSITION;
extern const Position KALECGOS_INITIAL_RANGED_POSITION;

extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;
extern std::unordered_set<ObjectGuid> hasReachedKalecgosInitialRangedPosition;

bool IsKalecgosDecurser(PlayerbotAI* botAI, Player* bot);
void EnsureKalecgosGroupAssignments(PlayerbotAI* botAI, Player* bot);
Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot);
bool HasReachedKalecgosInitialRangedPosition(Player* bot);
void SetKalecgosInitialRangedPositionReached(Player* bot, bool reached);
bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot);
bool IsInKalecgosSpectralRealm(Player* bot);
bool IsKalecgosRealmTransitionGraceActive(Player* bot);
void RecordKalecgosSpectralBlastTarget(PlayerbotAI* botAI, Player* bot);
void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot);
void RecordKalecgosNormalRealmEnter(Player* bot);

// Brutallus

struct BrutallusRangedSlotInfo
{
    bool isMainTankGroup = false;
    uint8 arcPositionIndex = 0;
};

enum class BrutallusRangedBurnState : uint8
{
    None,
    MovingToFrontStep,
    MovingToMirrorStep,
    MovingToRearFinal,
    AtRearFinal,
    ReturningToMirrorStep,
    ReturningToFrontStep,
    ReturningToNormal
};

extern const Position BRUTALLUS_MAIN_TANK_POSITION;
constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -M_PI_2;
constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 20.25f;
constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
constexpr float BRUTALLUS_RANGED_LANE_OFFSET = 5.0f;
constexpr float BRUTALLUS_NORMAL_RANGED_RADIUS =
    BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
constexpr float BRUTALLUS_BURN_RANGED_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS - BRUTALLUS_RANGED_LANE_OFFSET;
constexpr float BRUTALLUS_RETURN_RANGED_RADIUS =
    BRUTALLUS_NORMAL_RANGED_RADIUS + BRUTALLUS_RANGED_LANE_OFFSET;
constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS =
    BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
extern std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

bool ShouldMoveForBrutallusBurn(Player* bot);
float GetBrutallusMainTankAngle(Unit* brutallus);
Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z);
float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
float NormalizeSignedAngle(float angle);
Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z);
bool TryGetBrutallusMeleePosition(
    Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position);
float GetBrutallusRangedSlotAngle(
    Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo);
bool TryGetBrutallusRangedStepPosition(
    Unit* brutallus, uint8 rangedIndex, bool useMirrorAngle,
    float radius, float z, Position& position);
bool TryGetBrutallusRangedArcPosition(
    Unit* brutallus, uint8 rangedIndex, float radius, bool moveTowardMirror,
    float currentX, float currentY, float z, Position& position);
void EnsureBrutallusRangedAssignments(PlayerbotAI* botAI, Player* bot);
bool TryGetBrutallusAssignedPositionIndex(PlayerbotAI* botAI, Player* bot, bool wantRanged,
    uint8& positionIndex);

// Felmyst

enum class FelmystFogLane : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Top = 0,
    Middle = 1,
    Bottom = 2,
};

enum class FelmystFogPhase : uint8
{
    None,
    Windup,
    Sweep,
    Recovery,
};

enum class FelmystFogLocation : uint8
{
    None,
    LeftSide,
    RightSide,
    LeftTop,
    LeftMiddle,
    LeftBottom,
    RightTop,
    RightMiddle,
    RightBottom,
};

struct FelmystFogOfCorruptionState
{
    FelmystFogLane lane = FelmystFogLane::None;
    FelmystFogPhase phase = FelmystFogPhase::None;
    uint32 expireMs = 0;
};

constexpr float FELMYST_ENCAPSULATE_SAFE_DISTANCE = 21.0f;
constexpr float FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE = 8.0f;
constexpr float FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE = 3.0f;
constexpr float FELMYST_FOG_DESTINATION_MATCH_DISTANCE = 1.0f;

extern const Position FELMYST_TANK_POSITION;

extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporPathIndices;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporWaypointIndices;
extern std::unordered_map<uint32, FelmystFogOfCorruptionState> felmystFogOfCorruptionStates;

void EnsureFelmystRangedAssignments(PlayerbotAI* botAI, Player* bot);
float GetFelmystFrontAngle(PlayerbotAI* botAI, Player* bot, Unit* felmyst);
Creature* GetFelmystDemonicVaporSummonedByBot(Player* carrier);
void ClearFelmystDemonicVaporKiteState(Player* bot);
bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination);
bool TryGetFelmystFogSafeDestinations(
    Player* bot, FelmystFogLane dangerLane, std::array<Position, 3>& destinations,
    uint8& destinationCount);
bool TryGetFelmystFogOfCorruptionStageState(
    Unit* felmyst, FelmystFogOfCorruptionState& state);
bool TryGetActiveFelmystFogOfCorruptionState(
    Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state);
Unit* GetNearestFelmystFogOfCorruptionCharmedTarget(Player* bot);
Unit* GetNearestFelmystDemonicVaporHazard(Player* bot);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool TryGetFelmystRangedPosition(
    PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position);
Player* GetFelmystGasNovaDispelTarget(Player* bot);

// Eredar Twins

constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;

extern const Position SACROLASH_TANK_POSITION;
extern const std::array<Position, 5> ALYTHESS_TANK_POSITIONS;
extern const Position EREDAR_TWINS_P1_RANGED_POSITION;
extern const Position EREDAR_TWINS_P2_MELEE_STACK_POSITION;
extern const Position EREDAR_TWINS_P2_RANGED_STACK_POSITION;
extern const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION;
extern const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION;

extern std::unordered_map<ObjectGuid, uint8> alythessTankStep;

bool IsSacrolashTank(PlayerbotAI* botAI, Player* bot);
bool IsAlythessTank(PlayerbotAI* botAI, Player* bot);
bool ShouldHoldSacrolashThreat(PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash);
bool IsAlythessTankPositionSafe(Player* bot, Position const& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
bool IsEredarTwinsConflagrationTarget(Unit* alythess, Player* bot);

// M'uru

struct MuruEncounterTargets
{
    Unit* muru = nullptr;
    Unit* entropius = nullptr;
    std::vector<Unit*> voidSentinels;
    std::vector<Unit*> voidSpawns;
    std::vector<Unit*> furyMages;
    std::vector<Unit*> berserkers;
};
struct MuruDarknessState
{
    uint32 startMs = 0;
    uint32 expireMs = 0;
};

extern const Position MURU_STACK_POSITION;
extern const Position MURU_VOID_SENTINEL_N_TANK_POSITION;
extern const Position MURU_VOID_SENTINEL_E_TANK_POSITION;
extern const Position MURU_CENTER_POSITION;
extern const Position MURU_ENTRANCE_POSITION;

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_set<ObjectGuid>> muruEntropiusInitialRangedPositionsReached;

Creature* GetNearestMuruSingularity(Player* bot, float searchRadius = 30.0f);
bool IsFirstAssistTankInSameGroup(PlayerbotAI* botAI, Player* bot);
bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs = 10000);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Creature* FindAvailableVoidSpawnForEnslave(
    PlayerbotAI* botAI, Player* bot, Unit* muru, Unit* entropius);

// Kil'jaeden <The Deceiver>

struct KiljaedenRangedBotAssignment
{
    ObjectGuid guid;
    uint8 slotIndex = 0;
};
struct KiljaedenArmageddon
{
    Position destination;
    uint32 expireMs = 0;
    float safeDistance = 0.0f;
};

constexpr uint32 KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS = 10000;
constexpr float KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE = 11.0f; // Radius of hazard is 9 yards
constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

extern const Position KILJAEDEN_CENTER_POSITION;
extern const Position KILJAEDEN_TANK_POSITION;
extern const Position KILJAEDEN_S_MELEE_POSITION;
extern const Position KILJAEDEN_E_MELEE_POSITION;
extern const Position KILJAEDEN_STACK_POSITION;

extern std::unordered_map<uint32, std::vector<KiljaedenArmageddon>> kiljaedenArmageddons;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedAssignments;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedArmageddonAssignments;

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool HasActiveKiljaedenArmageddon(uint32 instanceId);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
float GetKiljaedenRangedSlotAngle(uint8 slotIndex);
bool IsKiljaedenRangedSlotSafe(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons);
float GetKiljaedenNearestArmageddonDistance(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons);
void EnsureKiljaedenRangedAssignments(PlayerbotAI* botAI, Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(PlayerbotAI* botAI, Player* bot);

}

#endif
