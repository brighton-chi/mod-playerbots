/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCTRIGGERS_H
#define PLAYERBOTS_SSCTRIGGERS_H

#include "EncounterHelpers.h"
#include "SSCHelpers.h"
#include "Trigger.h"
#include <string>

// General

class SscEncounterTrigger : public Trigger
{
public:
    SscEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SscHelpers::SSC_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

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

class HydrossTheUnstableShouldBeTankedByFrostTankTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldBeTankedByFrostTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should be tanked by frost tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldBeTankedByNatureTankTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldBeTankedByNatureTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should be tanked by nature tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable tank needs aggro upon phase change") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableAggroResetsUponPhaseChangeTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable aggro resets upon phase change") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldManagePhaseTimersTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldManagePhaseTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should manage phase timers") {}

protected:
    bool IsActiveInEncounter() override;
};

// The Lurker Below

class TheLurkerBelowSpoutIsActiveTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below spout is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowIsSubmergedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowIsSubmergedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below is submerged") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowShouldManageSpoutTimerTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowShouldManageSpoutTimerTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below should manage spout timer") {}

protected:
    bool IsActiveInEncounter() override;
};

// Leotheras the Blind

class LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind only warlock should tank demon form") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind demon form should be tanked by warlock") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindChannelingWhirlwindTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindTooManyChaosBlastStacksTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindTooManyChaosBlastStacksTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind too many chaos blast stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindInnerDemonHasAwakenedTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindInnerDemonHasAwakenedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind inner demon has awakened") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindInFinalPhaseTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindInFinalPhaseTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind in final phase") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindWarlockTankNeedsAggroTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindWarlockTankNeedsAggroTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind warlock tank needs aggro") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldManageDpsWaitTimersTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldManageDpsWaitTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind should manage dps wait timers") {}

protected:
    bool IsActiveInEncounter() override;
};

// Fathom-Lord Karathress

class FathomLordKarathressTargetsShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressTargetsShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress targets should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldHealCaribdisTankTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldHealCaribdisTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should heal caribdis tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressPullingBossesTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressPullingBossesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress pulling bosses") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressDeterminingKillOrderTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressDeterminingKillOrderTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress determining kill order") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldManageDpsTimerTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress should manage dps timer") {}

protected:
    bool IsActiveInEncounter() override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerPullingBossTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerPullingBossTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker pulling boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerInPhase2Trigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerInPhase2Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker in phase 2") {}

protected:
    bool IsActiveInEncounter() override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedShouldSpreadInPhase1Trigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedShouldSpreadInPhase1Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj ranged should spread in phase 1") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShamanShouldGroundShockBlastTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShamanShouldGroundShockBlastTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj shaman should ground shock blast") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStaticChargeOnGroupMemberTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStaticChargeOnGroupMemberTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj static charge on group member") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjPullingBossInPhase1AndPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjPullingBossInPhase1AndPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj pulling boss in phase 1 and phase 3") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjAddsSpawnInPhase2AndPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjAddsSpawnInPhase2AndPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj adds spawn in phase 2 and phase 3") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCoilfangStriderIsApproachingTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCoilfangStriderIsApproachingTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj coilfang strider is approaching") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedElementalCheatTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedElementalCheatTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tainted elemental cheat") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedCoreWasLootedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedCoreWasLootedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tainted core was looted") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjInPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjInPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj in phase 3") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjEntangleOnMeleeTrigger : public SscEncounterTrigger
{
public:
    LadyVashjEntangleOnMeleeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj entangle on melee") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
