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

class HyjalSummitBotIsNotInCombatTrigger : public Trigger
{
public:
    HyjalSummitBotIsNotInCombatTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hyjal summit bot is not in combat") {}
    bool IsActive() override;
};

// For Misdirection. Anetheron is not included because Hunters also Misdirect the Infernals.
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

// This covers all five MT actions, and activeAboveHealthPct is used for Archimonde only
class HyjalBossEngagedByMainTankTrigger : public Trigger
{
public:
    HyjalBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        float activeAboveHealthPct = 0.0f)
        : Trigger(botAI, name), _bossName(bossName), _activeAboveHealthPct(activeAboveHealthPct) {}
    bool IsActive() override;

private:
    std::string const _bossName;
    float const _activeAboveHealthPct;
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

class RageWinterchillRangedIsStandingInDeathAndDecayTrigger : public Trigger
{
public:
    RageWinterchillRangedIsStandingInDeathAndDecayTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "rage winterchill ranged is standing in death and decay") {}
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

class AnetheronShouldDetermineDpsPriorityTrigger : public Trigger
{
public:
    AnetheronShouldDetermineDpsPriorityTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anetheron should determine dps priority") {}
    bool IsActive() override;
};

// Kaz'rogal

class KazrogalMalevolentCleaveSplitsDamageTrigger : public Trigger
{
public:
    KazrogalMalevolentCleaveSplitsDamageTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal malevolent cleave splits damage") {}
    bool IsActive() override;
};

class KazrogalLowManaBotsNeedEscapePathTrigger : public Trigger
{
public:
    KazrogalLowManaBotsNeedEscapePathTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kaz'rogal low mana bots need escape path") {}
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

class AzgalorBossEngagedByRangedTrigger : public Trigger
{
public:
    AzgalorBossEngagedByRangedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor boss engaged by ranged") {}
    bool IsActive() override;
};

class AzgalorMeleeNearRainOfFireTrigger : public Trigger
{
public:
    AzgalorMeleeNearRainOfFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor melee near rain of fire") {}
    bool IsActive() override;
};

class AzgalorRangedIsStandingInRainOfFireTrigger : public Trigger
{
public:
    AzgalorRangedIsStandingInRainOfFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor ranged is standing in rain of fire") {}
    bool IsActive() override;
};

class AzgalorBotIsDoomedTrigger : public Trigger
{
public:
    AzgalorBotIsDoomedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor bot is doomed") {}
    bool IsActive() override;
};

class AzgalorDoomguardsMustBeControlledTrigger : public Trigger
{
public:
    AzgalorDoomguardsMustBeControlledTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "azgalor doomguards must be controlled") {}
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
