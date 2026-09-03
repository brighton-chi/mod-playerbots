/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALTRIGGERS_H
#define PLAYERBOTS_HYJALTRIGGERS_H

#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Trigger.h"
#include <string>

// General

class HyjalSummitEncounterTrigger : public Trigger
{
public:
    HyjalSummitEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, HyjalHelpers::HYJAL_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class HyjalSummitNoEncounterInProgress : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    HyjalSummitNoEncounterInProgress(PlayerbotAI* botAI)
        : Trigger(botAI, "hyjal summit no encounter in progress", 1000) {}
    bool IsActive() override;
};

// For Misdirection to the boss. Anetheron is not included because Misdirection is used there for
// picking up Infernals as well.
class HyjalPullingBossTrigger : public Trigger
{
public:
    HyjalPullingBossTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Trigger(botAI, name), _bossName(bossName) {}
    bool IsActive() override;

private:
    std::string const _bossName;
};

// This covers all five boss tanking actions, and activeAboveHealthPct is used for Archimonde
// only. Anetheron, Kaz'rogal, and Azgalor need their offtanks free for the Infernals, the
// Malevolent Cleave split, and the Doomguards, respectively, so those three are main tank only.
class HyjalBossShouldBeTankedTrigger : public HyjalSummitEncounterTrigger
{
public:
    HyjalBossShouldBeTankedTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        float activeAboveHealthPct = 0.0f, bool mainTankOnly = true)
        : HyjalSummitEncounterTrigger(botAI, name), _bossName(bossName),
          _activeAboveHealthPct(activeAboveHealthPct), _mainTankOnly(mainTankOnly) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
    float const _activeAboveHealthPct;
    bool const _mainTankOnly;
};

// Rage Winterchill

class RageWinterchillRangedShouldSpreadTrigger : public HyjalSummitEncounterTrigger
{
public:
    RageWinterchillRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "rage winterchill ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class RageWinterchillMeleeNearDeathAndDecayTrigger : public HyjalSummitEncounterTrigger
{
public:
    RageWinterchillMeleeNearDeathAndDecayTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "rage winterchill melee near death and decay") {}

protected:
    bool IsActiveInEncounter() override;
};

class RageWinterchillRangedInDeathAndDecayTrigger : public HyjalSummitEncounterTrigger
{
public:
    RageWinterchillRangedInDeathAndDecayTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "rage winterchill ranged in death and decay") {}

protected:
    bool IsActiveInEncounter() override;
};

// Anetheron

class AnetheronPullingBossOrInfernalTrigger : public Trigger
{
public:
    AnetheronPullingBossOrInfernalTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron pulling boss or infernal") {}
    bool IsActive() override;
};

class AnetheronRangedShouldSpreadTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronBotIsNearInfernoTargetTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronBotIsNearInfernoTargetTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron bot is near inferno target") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronBotIsTargetedByInfernalTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronBotIsTargetedByInfernalTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron bot is targeted by infernal") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronInfernalsPulseImmolationTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronInfernalsPulseImmolationTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron infernals pulse immolation") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronInfernalsShouldBeTankedAwayTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronInfernalsShouldBeTankedAwayTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron infernals should be tanked away") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronShouldDivideDpsTrigger : public HyjalSummitEncounterTrigger
{
public:
    AnetheronShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "anetheron should divide dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kaz'rogal

class KazrogalCanSplitMalevolentCleaveDamageTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalCanSplitMalevolentCleaveDamageTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal can split malevolent cleave damage") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalRangedShouldAvoidWarStompTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalRangedShouldAvoidWarStompTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal ranged should avoid war stomp") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalBotIsLowOnManaTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalBotIsLowOnManaTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal bot is low on mana") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalHunterShouldPreserveManaTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalHunterShouldPreserveManaTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal hunter should preserve mana") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalMarkOnMageOrPaladinTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalMarkOnMageOrPaladinTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal mark on mage or paladin") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalImmunityNoLongerNeededTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalImmunityNoLongerNeededTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal immunity no longer needed") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalWarlockShouldManageManaTrigger : public HyjalSummitEncounterTrigger
{
public:
    KazrogalWarlockShouldManageManaTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "kaz'rogal warlock should manage mana") {}

protected:
    bool IsActiveInEncounter() override;
};

// Azgalor

class AzgalorRangedShouldSpreadTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorMeleeNearRainOfFireTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorMeleeNearRainOfFireTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor melee near rain of fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorRangedInRainOfFireTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorRangedInRainOfFireTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor ranged in rain of fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorBotIsDoomedTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorBotIsDoomedTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor bot is doomed") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorShouldControlDoomguardsTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorShouldControlDoomguardsTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor should control doomguards") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorShouldDivideDpsTrigger : public HyjalSummitEncounterTrigger
{
public:
    AzgalorShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "azgalor should divide dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Archimonde

class ArchimondeBossCastsFearTrigger : public HyjalSummitEncounterTrigger
{
public:
    ArchimondeBossCastsFearTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "archimonde boss casts fear") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeBossCastingAirBurstTrigger : public HyjalSummitEncounterTrigger
{
public:
    ArchimondeBossCastingAirBurstTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "archimonde boss casting air burst") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeRangedShouldSpreadTrigger : public HyjalSummitEncounterTrigger
{
public:
    ArchimondeRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "archimonde ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeBotIsNearDoomfireTrigger : public HyjalSummitEncounterTrigger
{
public:
    ArchimondeBotIsNearDoomfireTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "archimonde bot is near doomfire") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeBotStoodInDoomfireTrigger : public HyjalSummitEncounterTrigger
{
public:
    ArchimondeBotStoodInDoomfireTrigger(PlayerbotAI* botAI)
        : HyjalSummitEncounterTrigger(botAI, "archimonde bot stood in doomfire") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
