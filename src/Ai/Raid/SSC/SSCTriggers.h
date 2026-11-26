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

// Shared

class SscEncounterTrigger : public Trigger
{
public:
    SscEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, SscHelpers::SscBosses boss,
        int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, SscHelpers::SSC_MAP_ID, SscHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    Unit* GetEncounterBoss() { return SscHelpers::GetSscBoss(bot, _boss); }
    SscHelpers::SscBosses GetEncounterBossId() const { return _boss; }

private:
    SscHelpers::SscBosses const _boss;
};

class SscNoEncounterInProgressTrigger : public Trigger
{
public:
    SscNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "ssc no encounter in progress", 1000) {}
    bool IsActive() override;
};

class SscClassRolesShouldBeAssignedTrigger : public Trigger
{
public:
    SscClassRolesShouldBeAssignedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "ssc class roles should be assigned", 1000) {}
    bool IsActive() override;
};

// Used for Fathom-Lord Karathress, Morogrim Tidewalker, and Lady Vashj.
class SscHunterShouldMisdirectTrigger : public SscEncounterTrigger
{
public:
    SscHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, SscHelpers::SscBosses boss)
        : SscEncounterTrigger(botAI, name, boss) {}

protected:
    bool IsActiveInEncounter() override;
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
        : SscEncounterTrigger(
            botAI, "hydross the unstable should be tanked by frost tank",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldBeTankedByNatureTankTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldBeTankedByNatureTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable should be tanked by nature tank",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable ranged should spread in frost phase",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable should misdirect upon phase change",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableDpsHoldIsActiveTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable dps hold is active",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldManagePhaseTimersTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldManagePhaseTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable should manage phase timers",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldManagePetAggressionTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "hydross the unstable should manage pet aggression",
            SscHelpers::SscBosses::DATA_HYDROSS_THE_UNSTABLE) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Lurker Below

class TheLurkerBelowSpoutIsActiveTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below spout is active",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below should be tanked",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below ranged should spread",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowGuardiansShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowGuardiansShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below guardians should be tanked",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowMeleeCannotReachTargetTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowMeleeCannotReachTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below melee cannot reach target",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowMeleeInWaterTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowMeleeInWaterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "the lurker below melee in water",
            SscHelpers::SscBosses::DATA_THE_LURKER_BELOW) {}

protected:
    bool IsActiveInEncounter() override;
};

// Leotheras the Blind

// Not encounter gated. See IsSpellbinderPhase.
class LeotherasTheBlindRangedShouldSpreadUponPullTrigger : public Trigger
{
public:
    LeotherasTheBlindRangedShouldSpreadUponPullTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind ranged should spread upon pull", 1000) {}
    bool IsActive() override;
};

class LeotherasTheBlindWarlockShouldTankDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind warlock should tank demon form",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindRangedShouldKeepDistanceTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindRangedShouldKeepDistanceTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind ranged should keep distance",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindChannelingWhirlwindTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind channeling whirlwind",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindTooManyChaosBlastStacksTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindTooManyChaosBlastStacksTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind too many chaos blast stacks",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindInnerDemonHasAwakenedTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindInnerDemonHasAwakenedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind inner demon has awakened",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldFocusBossInFinalPhaseTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldFocusBossInFinalPhaseTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind should focus boss in final phase",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldSeparateBossFromDemonTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldSeparateBossFromDemonTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind should separate boss from demon",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind hunter should misdirect demon form",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindDpsHoldIsActiveTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind dps hold is active",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldManageDpsHoldTimersTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldManageDpsHoldTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind should manage dps hold timers",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldManagePetAggressionTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind should manage pet aggression",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindPetShouldSwitchTargetTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "leotheras the blind pet should switch target",
            SscHelpers::SscBosses::DATA_LEOTHERAS_THE_BLIND) {}

protected:
    bool IsActiveInEncounter() override;
};

// Fathom-Lord Karathress

class FathomLordKarathressTargetsShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressTargetsShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress targets should be tanked",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldHealCaribdisTankTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldHealCaribdisTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should heal caribdis tank",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldAssignDpsPriorityTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should assign dps priority",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressDpsHoldIsActiveTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressDpsHoldIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress dps hold is active",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldManageDpsTimerTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should manage dps timer",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldManagePetAggressionTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should manage pet aggression",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressPetShouldSwitchTargetTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress pet should switch target",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress ranged should spread",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressStuckMidairAfterCycloneTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressStuckMidairAfterCycloneTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress stuck midair after cyclone",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressStrandedOnPillarLedgeTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressStrandedOnPillarLedgeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress stranded on pillar ledge",
            SscHelpers::SscBosses::DATA_FATHOM_LORD_KARATHRESS, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "morogrim tidewalker should be tanked",
            SscHelpers::SscBosses::DATA_MOROGRIM_TIDEWALKER) {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerRangedShouldStackTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerRangedShouldStackTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "morogrim tidewalker ranged should stack",
            SscHelpers::SscBosses::DATA_MOROGRIM_TIDEWALKER) {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerTooFarFromBossTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerTooFarFromBossTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "morogrim tidewalker too far from boss",
            SscHelpers::SscBosses::DATA_MOROGRIM_TIDEWALKER) {}

protected:
    bool IsActiveInEncounter() override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should be tanked",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedShouldSpreadInPhase1Trigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedShouldSpreadInPhase1Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj ranged should spread in phase 1",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStationSlotsNeedHoldersTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStationSlotsNeedHoldersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj station slots need holders",
            SscHelpers::SscBosses::DATA_LADY_VASHJ, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldHoldStationInPhase2Trigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldHoldStationInPhase2Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should hold station in phase 2",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedShouldPositionInPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedShouldPositionInPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj ranged should position in phase 3",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjOutOfSightInPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjOutOfSightInPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj out of sight in phase 3",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjMainTankNeedsGroundingShamanTrigger : public SscEncounterTrigger
{
public:
    LadyVashjMainTankNeedsGroundingShamanTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj main tank needs grounding shaman",
            SscHelpers::SscBosses::DATA_LADY_VASHJ, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShamanShouldGroundShockBlastTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShamanShouldGroundShockBlastTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj shaman should ground shock blast",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStaticChargeOnGroupMemberTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStaticChargeOnGroupMemberTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj static charge on group member",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldAssignTargetPriorityTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldAssignTargetPriorityTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should assign target priority",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTankNeedsFearWardTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTankNeedsFearWardTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj tank needs fear ward",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCoilfangStriderShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCoilfangStriderShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj coilfang strider should be tanked",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCoilfangEliteShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCoilfangEliteShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj coilfang elite should be tanked",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTankIsIdleAwayFromTheMiddleTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTankIsIdleAwayFromTheMiddleTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj tank is idle away from the middle",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedElementalNeedsLooterTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedElementalNeedsLooterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj tainted elemental needs looter",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldAttackTaintedElementalTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldAttackTaintedElementalTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should attack tainted elemental",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedCoreLooterTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedCoreLooterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj tainted core looter",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCorePassingChainMemberTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCorePassingChainMemberTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj core passing chain member",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldDestroyTaintedCoreTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldDestroyTaintedCoreTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should destroy tainted core",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldManagePetAggressionTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj should manage pet aggression",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjPetShouldSwitchTargetTrigger : public SscEncounterTrigger
{
public:
    LadyVashjPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj pet should switch target",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjBotAboveTheGroundTrigger : public SscEncounterTrigger
{
public:
    LadyVashjBotAboveTheGroundTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj bot above the ground",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjBotInToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjBotInToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj bot in toxic spores",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjMeleeNearToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjMeleeNearToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj melee near toxic spores",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedReachBlockedByToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedReachBlockedByToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj ranged reach blocked by toxic spores",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjEntangleOnMeleeTrigger : public SscEncounterTrigger
{
public:
    LadyVashjEntangleOnMeleeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj entangle on melee",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStaticChargeOnRogueTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStaticChargeOnRogueTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "lady vashj static charge on rogue",
            SscHelpers::SscBosses::DATA_LADY_VASHJ) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
