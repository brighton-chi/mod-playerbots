#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "Trigger.h"

class VoidReaverBossEngagedByTankTrigger : public Trigger
{
public:
    VoidReaverBossEngagedByTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver boss engaged by tank") {}
    bool IsActive() override;
};

class VoidReaverBossEngagedByRangedTrigger : public Trigger
{
public:
    VoidReaverBossEngagedByRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver boss engaged by ranged") {}
    bool IsActive() override;
};

class HighAstromancerSolarianPhase1And2MovementTrigger : public Trigger
{
public:
    HighAstromancerSolarianPhase1And2MovementTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian phase 1 and 2 movement") {}
    bool IsActive() override;
};

class HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger : public Trigger
{
public:
    HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian bot has wrath of the astromancer") {}
    bool IsActive() override;
};

class HighAstromancerSolarianSolariumPriestsSpawnedTrigger : public Trigger
{
public:
    HighAstromancerSolarianSolariumPriestsSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian solarium priests spawned") {}
    bool IsActive() override;
};

class HighAstromancerSolarianTransformedIntoVoidwalkerTrigger : public Trigger
{
public:
    HighAstromancerSolarianTransformedIntoVoidwalkerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian transformed into voidwalker") {}
    bool IsActive() override;
};

class HighAstromancerSolarianCanCastFearWardOnMainTankTrigger : public Trigger
{
public:
    HighAstromancerSolarianCanCastFearWardOnMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian can cast fear ward on main tank") {}
    bool IsActive() override;
};

#endif
