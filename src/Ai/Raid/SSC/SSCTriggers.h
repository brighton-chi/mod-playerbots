/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCTRIGGERS_H
#define PLAYERBOTS_SSCTRIGGERS_H

#include "Trigger.h"

// General

class SscNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    SscNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "ssc no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Trash

class UnderbogColossusInToxicPoolTrigger : public Trigger
{
public:
    UnderbogColossusInToxicPoolTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "underbog colossus in toxic pool") {}
    bool IsActive() override;
};

class GreyheartTidecallerWaterElementalTotemSpawnedTrigger : public Trigger
{
public:
    GreyheartTidecallerWaterElementalTotemSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "greyheart tidecaller water elemental totem spawned") {}
    bool IsActive() override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableShouldBeTankedByFrostTankTrigger : public Trigger
{
public:
    HydrossTheUnstableShouldBeTankedByFrostTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable should be tanked by frost tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableShouldBeTankedByNatureTankTrigger : public Trigger
{
public:
    HydrossTheUnstableShouldBeTankedByNatureTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable should be tanked by nature tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableElementalsSpawnedTrigger : public Trigger
{
public:
    HydrossTheUnstableElementalsSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable elementals spawned") {}
    bool IsActive() override;
};

class HydrossTheUnstableRangedShouldSpreadTrigger : public Trigger
{
public:
    HydrossTheUnstableRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable ranged should spread") {}
    bool IsActive() override;
};

class HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger : public Trigger
{
public:
    HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable tank needs aggro upon phase change") {}
    bool IsActive() override;
};

class HydrossTheUnstableAggroResetsUponPhaseChangeTrigger : public Trigger
{
public:
    HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable aggro resets upon phase change") {}
    bool IsActive() override;
};

class HydrossTheUnstableShouldManagePhaseTimersTrigger : public Trigger
{
public:
    HydrossTheUnstableShouldManagePhaseTimersTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hydross the unstable should manage phase timers") {}
    bool IsActive() override;
};

// The Lurker Below

class TheLurkerBelowSpoutIsActiveTrigger : public Trigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the lurker below spout is active") {}
    bool IsActive() override;
};

class TheLurkerBelowShouldBeTankedTrigger : public Trigger
{
public:
    TheLurkerBelowShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the lurker below should be tanked") {}
    bool IsActive() override;
};

class TheLurkerBelowRangedShouldSpreadTrigger : public Trigger
{
public:
    TheLurkerBelowRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the lurker below ranged should spread") {}
    bool IsActive() override;
};

class TheLurkerBelowIsSubmergedTrigger : public Trigger
{
public:
    TheLurkerBelowIsSubmergedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the lurker below is submerged") {}
    bool IsActive() override;
};

class TheLurkerBelowShouldManageSpoutTimerTrigger : public Trigger
{
public:
    TheLurkerBelowShouldManageSpoutTimerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the lurker below should manage spout timer") {}
    bool IsActive() override;
};

// Leotheras the Blind

class LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger : public Trigger
{
public:
    LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind only warlock should tank demon form") {}
    bool IsActive() override;
};

class LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger : public Trigger
{
public:
    LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind demon form should be tanked by warlock") {}
    bool IsActive() override;
};

class LeotherasTheBlindRangedShouldSpreadTrigger : public Trigger
{
public:
    LeotherasTheBlindRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind ranged should spread") {}
    bool IsActive() override;
};

class LeotherasTheBlindChannelingWhirlwindTrigger : public Trigger
{
public:
    LeotherasTheBlindChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind channeling whirlwind") {}
    bool IsActive() override;
};

class LeotherasTheBlindTooManyChaosBlastStacksTrigger : public Trigger
{
public:
    LeotherasTheBlindTooManyChaosBlastStacksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind too many chaos blast stacks") {}
    bool IsActive() override;
};

class LeotherasTheBlindInnerDemonHasAwakenedTrigger : public Trigger
{
public:
    LeotherasTheBlindInnerDemonHasAwakenedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind inner demon has awakened") {}
    bool IsActive() override;
};

class LeotherasTheBlindInFinalPhaseTrigger : public Trigger
{
public:
    LeotherasTheBlindInFinalPhaseTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind in final phase") {}
    bool IsActive() override;
};

class LeotherasTheBlindDemonFormTankNeedsAggro : public Trigger
{
public:
    LeotherasTheBlindDemonFormTankNeedsAggro(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind demon form tank needs aggro") {}
    bool IsActive() override;
};

class LeotherasTheBlindShouldManageDpsWaitTimersTrigger : public Trigger
{
public:
    LeotherasTheBlindShouldManageDpsWaitTimersTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind should manage dps wait timers") {}
    bool IsActive() override;
};

// Fathom-Lord Karathress

class FathomLordKarathressShouldBeTankedTrigger : public Trigger
{
public:
    FathomLordKarathressShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress should be tanked") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisShouldBeTankedTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress caribdis should be tanked") {}
    bool IsActive() override;
};

class FathomLordKarathressSharkkisShouldBeTankedTrigger : public Trigger
{
public:
    FathomLordKarathressSharkkisShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress sharkkis should be tanked") {}
    bool IsActive() override;
};

class FathomLordKarathressTidalvessShouldBeTankedTrigger : public Trigger
{
public:
    FathomLordKarathressTidalvessShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress tidalvess should be tanked") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress caribdis tank needs dedicated healer") {}
    bool IsActive() override;
};

class FathomLordKarathressPullingBossesTrigger : public Trigger
{
public:
    FathomLordKarathressPullingBossesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress pulling bosses") {}
    bool IsActive() override;
};

class FathomLordKarathressDeterminingKillOrderTrigger : public Trigger
{
public:
    FathomLordKarathressDeterminingKillOrderTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress determining kill order") {}
    bool IsActive() override;
};

class FathomLordKarathressShouldManageDpsTimerTrigger : public Trigger
{
public:
    FathomLordKarathressShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "fathom-lord karathress should manage dps timer") {}
    bool IsActive() override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerPullingBossTrigger : public Trigger
{
public:
    MorogrimTidewalkerPullingBossTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "morogrim tidewalker pulling boss") {}
    bool IsActive() override;
};

class MorogrimTidewalkerShouldBeTankedTrigger : public Trigger
{
public:
    MorogrimTidewalkerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "morogrim tidewalker should be tanked") {}
    bool IsActive() override;
};

class MorogrimTidewalkerInPhase2Trigger : public Trigger
{
public:
    MorogrimTidewalkerInPhase2Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "morogrim tidewalker in phase 2") {}
    bool IsActive() override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjShouldBeTankedTrigger : public Trigger
{
public:
    LadyVashjShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj should be tanked") {}
    bool IsActive() override;
};

class LadyVashjRangedShouldSpreadInPhase1Trigger : public Trigger
{
public:
    LadyVashjRangedShouldSpreadInPhase1Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj ranged should spread in phase 1") {}
    bool IsActive() override;
};

class LadyVashjShamanShouldGroundShockBlastTrigger : public Trigger
{
public:
    LadyVashjShamanShouldGroundShockBlastTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj shaman should ground shock blast") {}
    bool IsActive() override;
};

class LadyVashjStaticChargeOnGroupMemberTrigger : public Trigger
{
public:
    LadyVashjStaticChargeOnGroupMemberTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj static charge on group member") {}
    bool IsActive() override;
};

class LadyVashjPullingBossInPhase1AndPhase3Trigger : public Trigger
{
public:
    LadyVashjPullingBossInPhase1AndPhase3Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj pulling boss in phase 1 and phase 3") {}
    bool IsActive() override;
};

class LadyVashjAddsSpawnInPhase2AndPhase3Trigger : public Trigger
{
public:
    LadyVashjAddsSpawnInPhase2AndPhase3Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj adds spawn in phase 2 and phase 3") {}
    bool IsActive() override;
};

class LadyVashjCoilfangStriderIsApproachingTrigger : public Trigger
{
public:
    LadyVashjCoilfangStriderIsApproachingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj coilfang strider is approaching") {}
    bool IsActive() override;
};

class LadyVashjTaintedElementalCheatTrigger : public Trigger
{
public:
    LadyVashjTaintedElementalCheatTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj tainted elemental cheat") {}
    bool IsActive() override;
};

class LadyVashjTaintedCoreWasLootedTrigger : public Trigger
{
public:
    LadyVashjTaintedCoreWasLootedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj tainted core was looted") {}
    bool IsActive() override;
};

class LadyVashjInPhase3Trigger : public Trigger
{
public:
    LadyVashjInPhase3Trigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj in phase 3") {}
    bool IsActive() override;
};

class LadyVashjEntangleOnMeleeTrigger : public Trigger
{
public:
    LadyVashjEntangleOnMeleeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "lady vashj entangle on melee") {}
    bool IsActive() override;
};

#endif
