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

// Azgalor

// Archimonde


#endif
