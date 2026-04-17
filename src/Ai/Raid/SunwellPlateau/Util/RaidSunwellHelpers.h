/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLHELPERS_H
#define _PLAYERBOT_RAIDSUNWELLHELPERS_H

#include <array>
#include <unordered_map>
#include <vector>

#include "AiObject.h"
#include "Playerbots.h"
#include "Position.h"
#include "Unit.h"

namespace SunwellHelpers
{
    enum class SunwellSpells : uint32
    {
        // Kalecgos & Sathrovarr the Corruptor
        SPELL_SPECTRAL_EXHAUSTION      = 44867,
        SPELL_SPECTRAL_BLAST_PORTAL    = 44866,
        SPELL_CURSE_OF_BOUNDLESS_AGONY = 45032,
        SPELL_TELEPORT_SPECTRAL        = 46019,
        SPELL_TELEPORT_NORMAL_REALM    = 46020,
        SPELL_SPECTRAL_REALM           = 46021,

        // Brutallus
        SPELL_METEOR_SLASH             = 45150,
        SPELL_BURN                     = 46394,

        // Felmyst
        SPELL_SUMMON_DEMONIC_VAPOR = 45391,
        SPELL_ENCAPSULATE = 45661,
        SPELL_GAS_NOVA = 45855,
        SPELL_FELMYST_SPEED_BURST = 45495,
        SPELL_FOG_OF_CORRUPTION = 45582,
        SPELL_FOG_OF_CORRUPTION_CHARM = 45717,

        // Eredar Twins (Grand Warlock Alythess and Lady Sacrolash)
        SPELL_BLAZE                 = 45235,
        SPELL_CONFLAGRATION         = 45342,

        // Hunter
        SPELL_MISDIRECTION              = 35079,

        // Mage
        SPELL_ICE_BLOCK                 = 11958,

        // Paladin
        SPELL_DIVINE_SHIELD             = 642,

        // Rogue
        SPELL_CLOAK_OF_SHADOWS          = 31224,
    };

    enum class SunwellNPCs : uint32
    {
        // Felmyst
        NPC_FELMYST             = 25038,
        NPC_DEMONIC_VAPOR       = 25265,
        NPC_DEMONIC_VAPOR_TRAIL = 25267,
        NPC_WORLD_INVISIBLE_TRIGGER = 12999,
        // NPC_UNYIELDING_DEAD  = 25268,

        // Eredar Twins
        NPC_GRAND_WARLOCK_ALYTHESS = 25166,
    };

    enum class SunwellObjects : uint32
    {
        GO_SPECTRAL_RIFT = 187055,
        GO_BLAZE         = 187366,
    };

    // General

    constexpr uint32 SUNWELL_MAP_ID = 580;

    // Kalecgos & Sathrovarr the Corruptor

    constexpr uint8 KALECGOS_GROUP_COUNT = 4;
    constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
    struct KalecgosRealmState
    {
        uint32 lastEnterMs = 0;
        uint32 lastExitMs = 0;
        bool inSpectralRealm = false;
    };
    struct KalecgosEncounterState
    {
        uint32 activeRiftSequence = 0;
        uint32 activeRiftOpenedMs = 0;
        uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
        ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
        ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
        std::array<ObjectGuid, KALECGOS_GROUP_COUNT> groupTankGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };
        std::unordered_map<ObjectGuid, uint8> playerToGroup;
    };
    extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
    extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;
    extern std::unordered_map<ObjectGuid, bool> hasReachedKalecgosInitialRangedPosition;
    extern const Position KALECGOS_TANK_POSITION;
    extern const Position KALECGOS_INITIAL_RANGED_POSITION;
    bool IsKalecgosDecurser(PlayerbotAI* botAI, Player* bot);
    void EnsureKalecgosGroupAssignments(PlayerbotAI* botAI, Player* bot);
    Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot);
    bool HasReachedKalecgosInitialRangedPosition(Player* bot);
    void SetKalecgosInitialRangedPositionReached(Player* bot, bool reached);
    bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot);
    bool IsInKalecgosSpectralRealm(Player* bot);
    void RecordKalecgosSpectralBlastTarget(PlayerbotAI* botAI, Player* bot);
    void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot);
    void RecordKalecgosNormalRealmEnter(Player* bot);

    // Brutallus

    extern const Position BRUTALLUS_MAIN_TANK_POSITION;
    constexpr float BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET = -M_PI_2;
    constexpr float BRUTALLUS_TANK_POSITION_RADIUS = 20.25f;
    constexpr float BRUTALLUS_RANGED_TANK_OFFSET = 10.0f;
    constexpr uint8 BRUTALLUS_RANGED_POSITIONS_PER_GROUP = 10;
    constexpr uint8 BRUTALLUS_TOTAL_RANGED_POSITIONS = BRUTALLUS_RANGED_POSITIONS_PER_GROUP * 2;

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

    extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
    extern std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;
    bool ShouldMoveForBrutallusBurn(Player* bot);
    Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z);
    bool TryGetBrutallusMeleePosition(
        Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position);
    bool TryGetBrutallusRangedPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position);
    bool TryGetBrutallusRangedBurnStepPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position);
    bool TryGetBrutallusRangedBurnMirrorStepPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position);
    bool TryGetBrutallusRangedBurnArcPosition(
        Unit* brutallus, uint8 rangedIndex, bool moveTowardMirror,
        float currentX, float currentY, float z, Position& position);
    bool TryGetBrutallusRangedBurnPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position);
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

    extern const Position FELMYST_TANK_POSITION;
    constexpr float FELMYST_ENCAPSULATE_SAFE_DISTANCE = 21.0f;
    constexpr float FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE = 8.0f;
    constexpr float FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE = 3.0f;
    constexpr float FELMYST_FOG_DESTINATION_MATCH_DISTANCE = 1.0f;
    struct FelmystFogOfCorruptionState
    {
        FelmystFogLane lane = FelmystFogLane::None;
        FelmystFogPhase phase = FelmystFogPhase::None;
        uint32 expireMs = 0;
    };
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
    bool TryGetFelmystRangedPosition(PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position);
    Player* GetFelmystGasNovaDispelTarget(Player* bot);

    // Eredar Twins (Grand Warlock Alythess and Lady Sacrolash)
    constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;
    extern const Position SACROLASH_TANK_POSITION;
    extern const std::array<Position, 5> ALYTHESS_TANK_POSITIONS;
    extern const Position EREDAR_TWINS_P1_RANGED_POSITION;
    extern const Position EREDAR_TWINS_P2_STACK_POSITION;
    extern const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION;
    extern const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> alythessTankStep;
    bool IsSacrolashTank(PlayerbotAI* botAI, Player* bot);
    bool IsAlythessTank(PlayerbotAI* botAI, Player* bot);
    bool ShouldHoldSacrolashThreat(PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash);
    bool IsAlythessTankPositionSafe(Player* bot, Position const& position);
    bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot);
    bool IsEredarTwinsConflagrationTarget(Unit* alythess, Player* bot);
}

#endif
