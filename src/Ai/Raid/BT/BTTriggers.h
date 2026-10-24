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

// Shared

class BlackTempleEncounterTrigger : public Trigger
{
public:
    BlackTempleEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, BtHelpers::BT_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class BlackTempleNoEncounterInProgressTrigger : public Trigger
{
public:
    BlackTempleNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "black temple no encounter in progress", 1000) {}
    bool IsActive() override;
};

// A Hunter while the named boss is untouched, so Misdirection goes out on the pull. Used for
// High Warlord Naj'entus, Supremus, Teron Gorefiend, Gurtogg Bloodboil, Mother Shahraz and the
// Illidari Council (on Gathios).
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
class SisterOfPleasureShouldBeFocusedTrigger : public Trigger
{
public:
    SisterOfPleasureShouldBeFocusedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sister of pleasure should be focused", 1000) {}
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

class SupremusFixatedOnBotTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusFixatedOnBotTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus fixated on bot") {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusNearFireTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusNearFireTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus near fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusReachBlockedByFireTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusReachBlockedByFireTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "supremus reach blocked by fire") {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Akama

class ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger : public BlackTempleEncounterTrigger
{
public:
    ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "shade of akama should attack channelers and sorcerers") {}

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

class TeronGorefiendShouldPositionForVengefulSpiritTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendShouldPositionForVengefulSpiritTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend should position for vengeful spirit") {}

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

class GurtoggBloodboilShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilShouldPositionForBloodboilTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldPositionForBloodboilTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil should position for bloodboil") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilMeleeShouldStayBehindTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilMeleeShouldStayBehindTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil melee should stay behind") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil ranged should avoid frontal cone") {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilFelRageOnBotTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilFelRageOnBotTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "gurtogg bloodboil fel rage on bot") {}

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

class ReliquaryOfSoulsShouldPositionForSufferingTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsShouldPositionForSufferingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "reliquary of souls should position for suffering") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsHealersShouldAttackSufferingTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsHealersShouldAttackSufferingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls healers should attack suffering") {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls essence of desire has rune shield") {}

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

class MotherShahrazBeingPositionedTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazBeingPositionedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "mother shahraz being positioned") {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazRangedShouldPositionUnderStatueTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazRangedShouldPositionUnderStatueTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "mother shahraz ranged should position under statue") {}

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
        : BlackTempleEncounterTrigger(
            botAI, "illidari council gathios casts judgement of command") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMalandeCastsCircleOfHealingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMalandeCastsCircleOfHealingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council malande casts circle of healing") {}

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

class IllidariCouncilVerasShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilVerasShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council veras should be tanked") {}

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

class IllidariCouncilMageTankHealerShouldRepositionTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMageTankHealerShouldRepositionTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council mage tank healer should reposition") {}

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

class IllidariCouncilPetShouldSwitchTargetTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council pet should switch target") {}

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

class IllidariCouncilShouldManageDpsTimerTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidari council should manage dps timer") {}

protected:
    bool IsActiveInEncounter() override;
};

// Illidan Stormrage <The Betrayer>

// Not gated on the encounter, which starts as the speech ends, so throttled to once per second.
class IllidanStormrageShouldPositionForPullTrigger : public Trigger
{
public:
    IllidanStormrageShouldPositionForPullTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "illidan stormrage should position for pull", 1000) {}
    bool IsActive() override;
};

class IllidanStormrageHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should be tanked") {}

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

class IllidanStormrageShouldTrackInfectionsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldTrackInfectionsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should track infections") {}

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

class IllidanStormragePetsDieToHazardsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetsDieToHazardsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage pets die to hazards") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetShouldAttackShadowfiendTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetShouldAttackShadowfiendTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage pet should attack shadowfiend") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetShouldAttackShadowDemonTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetShouldAttackShadowDemonTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage pet should attack shadow demon") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldPositionAboveGrateTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldPositionAboveGrateTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should position above grate") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageImmunityCanClearDebuffTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageImmunityCanClearDebuffTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage immunity can clear debuff") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageImmunityNoLongerNeededTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageImmunityNoLongerNeededTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage immunity no longer needed") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger
    : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage shaman has totems before tanks have aggro") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldPositionForLandingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldPositionForLandingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should position for landing") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageMeleeShouldStopAttackingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageMeleeShouldStopAttackingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage melee should stop attacking") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldStopShootingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldStopShootingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should stop shooting") {}

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

class IllidanStormrageRangedShouldReachShadowDemonTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageRangedShouldReachShadowDemonTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage ranged should reach shadow demon") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage melee should stay away from demon form") {}

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

class IllidanStormrageWarlockShouldSeedShadowDemonsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageWarlockShouldSeedShadowDemonsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage warlock should seed shadow demons")
    {
    }

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldAssignTargetPriorityTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldAssignTargetPriorityTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should assign target priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldManageDpsTimersTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldManageDpsTimersTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(botAI, "illidan stormrage should manage dps timers") {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldClearHazardsBeforeLandingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldClearHazardsBeforeLandingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should clear hazards before landing") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
