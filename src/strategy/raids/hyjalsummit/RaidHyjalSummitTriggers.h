#ifndef _PLAYERBOT_RAIDHYJALSUMMITTRIGGERS_H
#define _PLAYERBOT_RAIDHYJALSUMMITTRIGGERS_H

#include "Trigger.h"

// Rage Winterchill

class RageWinterchillPullingBossTrigger : public Trigger
{
public:
    RageWinterchillPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill pulling boss") {}
    bool IsActive() override;
};

class RageWinterchillBossEngagedByMainTankTrigger : public Trigger
{
public:
    RageWinterchillBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill boss engaged by main tank") {}
    bool IsActive() override;
};

class RageWinterchillBossCastsDeathAndDecayTrigger : public Trigger
{
public:
    RageWinterchillBossCastsDeathAndDecayTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "rage winterchill boss casts death and decay") {}
    bool IsActive() override;
};

// Anetheron

class AnetheronPullingBossTrigger : public Trigger
{
public:
    AnetheronPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron pulling boss") {}
    bool IsActive() override;
};

class AnetheronBossEngagedByMainTankTrigger : public Trigger
{
public:
    AnetheronBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron boss engaged by main tank") {}
    bool IsActive() override;
};

class AnetheronBossCastsCarrionSwarmTrigger : public Trigger
{
public:
    AnetheronBossCastsCarrionSwarmTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron boss casts carrion swarm") {}
    bool IsActive() override;
};

class AnetheronBotIsTargetedByInfernalTrigger : public Trigger
{
public:
    AnetheronBotIsTargetedByInfernalTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron bot is targeted by infernal") {}
    bool IsActive() override;
};

class AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger : public Trigger
{
public:
    AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron infernals need to be kept away from raid") {}
    bool IsActive() override;
};

class AnetheronInfernalsDespawnWhenBossDiesTrigger : public Trigger
{
public:
    AnetheronInfernalsDespawnWhenBossDiesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anetheron infernals despawn when boss dies") {}
    bool IsActive() override;
};

// Kaz'rogal

class KazrogalPullingBossTrigger : public Trigger
{
public:
    KazrogalPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal pulling boss") {}
    bool IsActive() override;
};

class KazrogalBossEngagedByMainTankTrigger : public Trigger
{
public:
    KazrogalBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal boss engaged by main tank") {}
    bool IsActive() override;
};

class KazrogalBossEngagedByAssistTanksTrigger : public Trigger
{
public:
    KazrogalBossEngagedByAssistTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal boss engaged by assist tanks") {}
    bool IsActive() override;
};

class KazrogalLowManaBotsNeedEscapePathTrigger : public Trigger
{
public:
    KazrogalLowManaBotsNeedEscapePathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal low mana bots need escape path") {}
    bool IsActive() override;
};

class KazrogalBotIsLowOnManaTrigger : public Trigger
{
public:
    KazrogalBotIsLowOnManaTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal bot is low on mana") {}
    bool IsActive() override;
};

class KazrogalMageOrPaladinHasMarkOfKazrogalTrigger : public Trigger
{
public:
    KazrogalMageOrPaladinHasMarkOfKazrogalTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kaz'rogal mage or paladin has mark of kaz'rogal") {}
    bool IsActive() override;
};

// Azgalor

class AzgalorPullingBossTrigger : public Trigger
{
public:
    AzgalorPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor pulling boss") {}
    bool IsActive() override;
};

class AzgalorBossEngagedByMainTankTrigger : public Trigger
{
public:
    AzgalorBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor boss engaged by main tank") {}
    bool IsActive() override;
};

class AzgalorBossCastsRainOfFireTrigger : public Trigger
{
public:
    AzgalorBossCastsRainOfFireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor boss casts rain of fire") {}
    bool IsActive() override;
};

class AzgalorBotIsDoomedTrigger : public Trigger
{
public:
    AzgalorBotIsDoomedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor bot is doomed") {}
    bool IsActive() override;
};

class AzgalorDoomguardSpawnedTrigger : public Trigger
{
public:
    AzgalorDoomguardSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "azgalor doomguard spawned") {}
    bool IsActive() override;
};

// Archimonde

class ArchimondePullingBossTrigger : public Trigger
{
public:
    ArchimondePullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde pulling boss") {}
    bool IsActive() override;
};

class ArchimondeBossEngagedByMainTankTrigger : public Trigger
{
public:
    ArchimondeBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss engaged by main tank") {}
    bool IsActive() override;
};

class ArchimondeBossCastsFearTrigger : public Trigger
{
public:
    ArchimondeBossCastsFearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "archimonde boss casts fear") {}
    bool IsActive() override;
};

#endif
