/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALTRIGGERS_H
#define PLAYERBOTS_HYJALTRIGGERS_H

#include "Trigger.h"
#include <string>

// General

class HyjalSummitNoEncounterInProgress : public Trigger
{
public:
    HyjalSummitNoEncounterInProgress(PlayerbotAI* botAI)
        : Trigger(botAI, "hyjal summit no encounter in progress") {}
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
class HyjalBossShouldBeTankedTrigger : public Trigger
{
public:
    HyjalBossShouldBeTankedTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        float activeAboveHealthPct = 0.0f, bool mainTankOnly = true)
        : Trigger(botAI, name), _bossName(bossName), _activeAboveHealthPct(activeAboveHealthPct),
          _mainTankOnly(mainTankOnly) {}
    bool IsActive() override;

private:
    std::string const _bossName;
    float const _activeAboveHealthPct;
    bool const _mainTankOnly;
};

// Rage Winterchill

class RageWinterchillRangedShouldSpreadTrigger : public Trigger
{
public:
    RageWinterchillRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "rage winterchill ranged should spread") {}
    bool IsActive() override;
};

class RageWinterchillMeleeNearDeathAndDecayTrigger : public Trigger
{
public:
    RageWinterchillMeleeNearDeathAndDecayTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "rage winterchill melee near death and decay") {}
    bool IsActive() override;
};

class RageWinterchillRangedInDeathAndDecayTrigger : public Trigger
{
public:
    RageWinterchillRangedInDeathAndDecayTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "rage winterchill ranged in death and decay") {}
    bool IsActive() override;
};

// Anetheron

class AnetheronPullingBossOrInfernalTrigger : public Trigger
{
public:
    AnetheronPullingBossOrInfernalTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron pulling boss or infernal") {}
    bool IsActive() override;
};

class AnetheronRangedShouldSpreadTrigger : public Trigger
{
public:
    AnetheronRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron ranged should spread") {}
    bool IsActive() override;
};

class AnetheronBotIsNearInfernoTargetTrigger : public Trigger
{
public:
    AnetheronBotIsNearInfernoTargetTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron bot is near inferno target") {}
    bool IsActive() override;
};

class AnetheronBotIsTargetedByInfernalTrigger : public Trigger
{
public:
    AnetheronBotIsTargetedByInfernalTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron bot is targeted by infernal") {}
    bool IsActive() override;
};

class AnetheronInfernalsShouldBeKeptAwayTrigger : public Trigger
{
public:
    AnetheronInfernalsShouldBeKeptAwayTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron infernals should be kept away") {}
    bool IsActive() override;
};

class AnetheronShouldDivideDpsTrigger : public Trigger
{
public:
    AnetheronShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron should divide dps") {}
    bool IsActive() override;
};

// Kaz'rogal

class KazrogalCanSplitMalevolentCleaveDamageTrigger : public Trigger
{
public:
    KazrogalCanSplitMalevolentCleaveDamageTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal can split malevolent cleave damage") {}
    bool IsActive() override;
};

class KazrogalRangedShouldAvoidWarStompTrigger : public Trigger
{
public:
    KazrogalRangedShouldAvoidWarStompTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal ranged should avoid war stomp") {}
    bool IsActive() override;
};

class KazrogalBotIsLowOnManaTrigger : public Trigger
{
public:
    KazrogalBotIsLowOnManaTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal bot is low on mana") {}
    bool IsActive() override;
};

class KazrogalHunterShouldPreserveManaTrigger : public Trigger
{
public:
    KazrogalHunterShouldPreserveManaTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal hunter should preserve mana") {}
    bool IsActive() override;
};

class KazrogalMarkOnMageOrPaladinTrigger : public Trigger
{
public:
    KazrogalMarkOnMageOrPaladinTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal mark on mage or paladin") {}
    bool IsActive() override;
};

class KazrogalWarlockShouldManageManaTrigger : public Trigger
{
public:
    KazrogalWarlockShouldManageManaTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal warlock should manage mana") {}
    bool IsActive() override;
};

// Azgalor

class AzgalorRangedShouldSpreadTrigger : public Trigger
{
public:
    AzgalorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor ranged should spread") {}
    bool IsActive() override;
};

class AzgalorMeleeNearRainOfFireTrigger : public Trigger
{
public:
    AzgalorMeleeNearRainOfFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor melee near rain of fire") {}
    bool IsActive() override;
};

class AzgalorRangedInRainOfFireTrigger : public Trigger
{
public:
    AzgalorRangedInRainOfFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor ranged in rain of fire") {}
    bool IsActive() override;
};

class AzgalorBotIsDoomedTrigger : public Trigger
{
public:
    AzgalorBotIsDoomedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "azgalor bot is doomed") {}
    bool IsActive() override;
};

class AzgalorShouldControlDoomguardsTrigger : public Trigger
{
public:
    AzgalorShouldControlDoomguardsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor should control doomguards") {}
    bool IsActive() override;
};

class AzgalorShouldDivideDpsTrigger : public Trigger
{
public:
    AzgalorShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor should divide dps") {}
    bool IsActive() override;
};

// Archimonde

class ArchimondeBossCastsFearTrigger : public Trigger
{
public:
    ArchimondeBossCastsFearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "archimonde boss casts fear") {}
    bool IsActive() override;
};

class ArchimondeBossCastingAirBurstTrigger : public Trigger
{
public:
    ArchimondeBossCastingAirBurstTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "archimonde boss casting air burst") {}
    bool IsActive() override;
};

class ArchimondeRangedShouldSpreadTrigger : public Trigger
{
public:
    ArchimondeRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "archimonde ranged should spread") {}
    bool IsActive() override;
};

class ArchimondeBotIsNearDoomfireTrigger : public Trigger
{
public:
    ArchimondeBotIsNearDoomfireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "archimonde bot is near doomfire") {}
    bool IsActive() override;
};

class ArchimondeBotStoodInDoomfireTrigger : public Trigger
{
public:
    ArchimondeBotStoodInDoomfireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "archimonde bot stood in doomfire") {}
    bool IsActive() override;
};

#endif
