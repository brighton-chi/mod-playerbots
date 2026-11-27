/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKTRIGGERS_H
#define PLAYERBOTS_TKTRIGGERS_H

#include "EncounterHelpers.h"
#include "TKHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class TempestKeepEncounterTrigger : public Trigger
{
public:
    TempestKeepEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, TkHelpers::TkBosses boss)
        : Trigger(botAI, name), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, TkHelpers::TK_MAP_ID, TkHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    Unit* GetEncounterBoss() { return TkHelpers::GetTkBoss(bot, _boss); }

private:
    TkHelpers::TkBosses const _boss;
};

class TempestKeepNoEncounterInProgressTrigger : public Trigger
{
public:
    TempestKeepNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep no encounter in progress", 1000) {}
    bool IsActive() override;
};

class TempestKeepStuckFallingTrigger : public Trigger
{
public:
    // This is to address a fringe wipe scenario during Kael'thas.
    TempestKeepStuckFallingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep stuck falling", 1000) {}
    bool IsActive() override;
};

class TempestKeepClassRolesShouldBeAssignedTrigger : public Trigger
{
public:
    TempestKeepClassRolesShouldBeAssignedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep class roles should be assigned", 1000) {}
    bool IsActive() override;
};

// Trash

class CrimsonHandCenturionCastsArcaneFlurryTrigger : public Trigger
{
public:
    CrimsonHandCenturionCastsArcaneFlurryTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "crimson hand centurion casts arcane flurry") {}
    bool IsActive() override;
};

// Al'ar <Phoenix God>

class AlarHunterShouldMisdirectTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar hunter should misdirect",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarFliesBetweenPlatformsTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarFliesBetweenPlatformsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar flies between platforms",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarEmbersShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarEmbersShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar embers should be tanked",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarShouldAssignNonTankTargetTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarShouldAssignNonTankTargetTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar should assign non-tank target",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarIncomingFlameQuillsTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarIncomingFlameQuillsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar incoming flame quills",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarRisingFromTheAshesTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarRisingFromTheAshesTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar rising from the ashes",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarInPhase2Trigger : public TempestKeepEncounterTrigger
{
public:
    AlarInPhase2Trigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar in phase 2",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarShouldManagePhaseTrackerTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarShouldManagePhaseTrackerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "al'ar should manage phase tracker",
            TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Void Reaver

class VoidReaverShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "void reaver should be tanked",
            TkHelpers::TkBosses::DATA_VOID_REAVER) {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverKnockAwayPullsAggroToNonTanksTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverKnockAwayPullsAggroToNonTanksTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "void reaver knock away pulls aggro to non-tanks",
            TkHelpers::TkBosses::DATA_VOID_REAVER) {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverRangedShouldStandBackTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverRangedShouldStandBackTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "void reaver ranged should stand back",
            TkHelpers::TkBosses::DATA_VOID_REAVER) {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverArcaneOrbIsIncomingTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverArcaneOrbIsIncomingTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "void reaver arcane orb is incoming",
            TkHelpers::TkBosses::DATA_VOID_REAVER) {}

protected:
    bool IsActiveInEncounter() override;
};

// High Astromancer Solarian

class HighAstromancerSolarianShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian should be tanked",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighAstromancerSolarianWrathOfTheAstromancerTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianWrathOfTheAstromancerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian wrath of the astromancer",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighAstromancerSolarianSolariumPriestsSpawnedTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianSolariumPriestsSpawnedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian solarium priests spawned",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighAstromancerSolarianShamanShouldProtectAgainstFearTrigger
    : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianShamanShouldProtectAgainstFearTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian shaman should protect against fear",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderChasedByThaladredTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderChasedByThaladredTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider chased by thaladred",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderHunterShouldMisdirectAdvisorsTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderHunterShouldMisdirectAdvisorsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider hunter should misdirect advisors",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger
    : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider sanguinar or telonicus should be tanked",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider capernian should be tanked by warlock",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldStandBackFromCapernianTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldStandBackFromCapernianTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should stand back from capernian",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldHoldPhase3PositionsTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldHoldPhase3PositionsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should hold phase 3 positions",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldAssignAdvisorDpsPriorityTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldAssignAdvisorDpsPriorityTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should assign advisor dps priority",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should manage advisor dps timer",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreAliveTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary weapons are alive",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary axe casts whirlwind",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreDeadTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreDeadTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary weapons are dead",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary weapons are equipped",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsWereLostTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsWereLostTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons were lost") {}
    bool IsActive() override;
};

class KaelthasSunstriderHasEnteredTheFightTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderHasEnteredTheFightTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider has entered the fight",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should assign final phase target",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderRaidMemberIsMindControlledTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderRaidMemberIsMindControlledTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider raid member is mind controlled",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderInGravityLapsePhaseTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderInGravityLapsePhaseTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider in gravity lapse phase",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldManagePetAggressionTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldManagePetAggressionTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should manage pet aggression",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderPetShouldSwitchTargetTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider pet should switch target",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
