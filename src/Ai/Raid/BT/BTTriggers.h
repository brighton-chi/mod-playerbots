/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERS_H
#define PLAYERBOTS_BTTRIGGERS_H

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Trigger.h"
#include <string>

// General

class BlackTempleEncounterTrigger : public Trigger
{
public:
    BlackTempleEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, BlackTempleHelpers::BT_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class BlackTempleNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    BlackTempleNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "black temple no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Shared Bosses

// A Hunter while the named boss is untouched, so Misdirection goes out on the pull. Used for
// High Warlord Naj'entus, Supremus, Teron Gorefiend, Mother Shahraz and the Illidari Council (on
// Gathios).
class BlackTempleHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    BlackTempleHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : BlackTempleEncounterTrigger(botAI, name), _bossName(bossName) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
};

// Trash

// Not gated on an encounter, so throttled to once per second. A skull is a raid leader's call,
// and the Sister of Pain's first Shell of Pain is 20 s away.
class SisterOfPleasureShouldBeMarkedTrigger : public Trigger
{
public:
    SisterOfPleasureShouldBeMarkedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sister of pleasure should be marked", 1000) {}
    bool IsActive() override;
};

// A wand keeps shooting once started, so a multiplier can't stop it.
class ShadowmoonReaverWandBuildsChargesTrigger : public Trigger
{
public:
    ShadowmoonReaverWandBuildsChargesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shadowmoon reaver wand builds charges") {}
    bool IsActive() override;
};

// A pet keeps its victim when its owner switches (PetAI::OwnerAttacked), so it needs commanding.
class ShadowmoonReaverShouldControlCasterPetTrigger : public Trigger
{
public:
    ShadowmoonReaverShouldControlCasterPetTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shadowmoon reaver should control caster pet") {}
    bool IsActive() override;
};

// High Warlord Naj'entus

class HighWarlordNajentusShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "high warlord naj'entus should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "high warlord naj'entus ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
              botAI, "high warlord naj'entus impaled player needs remover") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusImpalingSpineOnGroupMemberTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusImpalingSpineOnGroupMemberTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
              botAI, "high warlord naj'entus impaling spine on group member") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusNeedsSpineThrowerTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusNeedsSpineThrowerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "high warlord naj'entus needs spine thrower", 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusHasTidalShieldTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusHasTidalShieldTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "high warlord naj'entus has tidal shield") {}

protected:
    bool IsActiveInEncounter() override;
};

// Supremus

class SupremusRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusFixatesOnBotTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusFixatesOnBotTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus fixates on bot") {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusNearVolcanoTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusNearVolcanoTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus near volcano") {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Akama

class ShadeOfAkamaKillingChannelersStartsPhase2Trigger : public BlackTempleEncounterTrigger
{
public:
    ShadeOfAkamaKillingChannelersStartsPhase2Trigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "shade of akama killing channelers starts phase 2") {}

protected:
    bool IsActiveInEncounter() override;
};

// Teron Gorefiend

class TeronGorefiendShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "teron gorefiend should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendRangedShouldPositionOnBalconyTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendRangedShouldPositionOnBalconyTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "teron gorefiend ranged should position on balcony") {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendCastsShadowOfDeathTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendCastsShadowOfDeathTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "teron gorefiend casts shadow of death") {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendShadowOfDeathTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendShadowOfDeathTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "teron gorefiend shadow of death") {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendTransformedIntoVengefulSpiritTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendTransformedIntoVengefulSpiritTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "teron gorefiend transformed into vengeful spirit") {}

protected:
    bool IsActiveInEncounter() override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilCastsBloodboilTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilCastsBloodboilTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil casts bloodboil") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilFelRageOnGroupMemberTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilFelRageOnGroupMemberTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil fel rage on group member") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilShouldManagePhaseTimerTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldManagePhaseTimerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil should manage phase timer") {}

protected:
    bool IsActiveInEncounter() override;
};

// Reliquary of Souls

class ReliquaryOfSoulsHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "reliquary of souls hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls essence of suffering fixates on closest target") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls essence of suffering disables healing") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "reliquary of souls essence of desire has rune shield") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "reliquary of souls essence of desire casts deaden") {}

protected:
    bool IsActiveInEncounter() override;
};

// Mother Shahraz

class MotherShahrazShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "mother shahraz should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazTanksArePositioningBossTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazTanksArePositioningBossTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "mother shahraz tanks are positioning boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazRangedShouldPositionUnderPillarTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazRangedShouldPositionUnderPillarTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "mother shahraz ranged should position under pillar") {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazFatalAttractionTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazFatalAttractionTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "mother shahraz fatal attraction") {}

protected:
    bool IsActiveInEncounter() override;
};

// Illidari Council

class IllidariCouncilGathiosShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilGathiosShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council gathios should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilGathiosCastsJudgementOfCommandTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilGathiosCastsJudgementOfCommandTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council gathios casts judgement of command") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMalandeShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMalandeShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council malande should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilDarkshadowShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilDarkshadowShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council darkshadow should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilZerevorShouldBeTankedByMageTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilZerevorShouldBeTankedByMageTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council zerevor should be tanked by mage") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMageTankNeedsDedicatedHealerTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMageTankNeedsDedicatedHealerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council mage tank needs dedicated healer") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilPetsScrewUpThePullTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilPetsScrewUpThePullTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council pets screw up the pull") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilShouldManageDpsTimerTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council should manage dps timer") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilShouldAssignDpsPriorityTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageCastsFlameCrashTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageCastsFlameCrashTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage casts flame crash") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage parasitic shadowfiend on group member") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageParasiticShadowfiendsRunWildTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageParasiticShadowfiendsRunWildTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage parasitic shadowfiends run wild") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage flames of azzinoth should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetsDieToFireTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetsDieToFireTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage pets die to fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageGrateIsSafeFromFlamesTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageGrateIsSafeFromFlamesTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage grate is safe from flames") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageDarkBarrageOnImmunityClassTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageDarkBarrageOnImmunityClassTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage dark barrage on immunity class") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePreparesToLandTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePreparesToLandTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage prepares to land") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageThisExpansionHatesMeleeTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageThisExpansionHatesMeleeTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage this expansion hates melee") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageWarlockShouldTankDemonFormTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage warlock should tank demon form") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldAssignDpsPriorityTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageMaievPlacedShadowTrapTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageMaievPlacedShadowTrapTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage maiev placed shadow trap") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldManageDpsTimerAndRtiTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldManageDpsTimerAndRtiTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should manage dps timer and rti") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldClearHazardsBetweenPhasesTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldClearHazardsBetweenPhasesTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should clear hazards between phases") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageCheatTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageCheatTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage cheat") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
