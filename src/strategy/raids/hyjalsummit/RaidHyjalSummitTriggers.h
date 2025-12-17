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

// Kaz'rogal

// Azgalor

// Archimonde


#endif
