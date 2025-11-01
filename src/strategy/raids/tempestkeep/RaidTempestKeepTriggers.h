#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "Trigger.h"

class AlarNeedToLogInfoToDevelopBossStrategyTrigger : public Trigger
{
public:
    AlarNeedToLogInfoToDevelopBossStrategyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar need to log info to develop boss strategy") {}
    bool IsActive() override;
};

class AlarPullingBossTrigger : public Trigger
{
public:
    AlarPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar pulling boss") {}
    bool IsActive() override;
};

class AlarEngagedByTanksInPhase1Trigger : public Trigger
{
public:
    AlarEngagedByTanksInPhase1Trigger(PlayerbotAI* botAI) : Trigger(botAI, "alar engaged by tanks in phase 1") {}
    bool IsActive() override;
};

class AlarBossEngagedByMeleeDpsTrigger : public Trigger
{
public:
    AlarBossEngagedByMeleeDpsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar boss engaged by melee dps") {}
    bool IsActive() override;
};

class AlarBossEngagedByRangedDpsTrigger : public Trigger
{
public:
    AlarBossEngagedByRangedDpsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar boss engaged by ranged dps") {}
    bool IsActive() override;
};

class AlarBossEngagedByHealerTrigger : public Trigger
{
public:
    AlarBossEngagedByHealerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar boss engaged by healer") {}
    bool IsActive() override;
};

class AlarEmbersOfAlarSpawnedTrigger : public Trigger
{
public:
    AlarEmbersOfAlarSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar embers of alar spawned") {}
    bool IsActive() override;
};

class AlarIncomingFlameQuillsTrigger : public Trigger
{
public:
    AlarIncomingFlameQuillsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar incoming flame quills") {}
    bool IsActive() override;
};

class AlarRisingFromTheAshesTrigger : public Trigger
{
public:
    AlarRisingFromTheAshesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar rising from the ashes") {}
    bool IsActive() override;
};

class AlarEngagedByTanksInPhase2Trigger : public Trigger
{
public:
    AlarEngagedByTanksInPhase2Trigger(PlayerbotAI* botAI) : Trigger(botAI, "alar engaged by tanks in phase 2") {}
    bool IsActive() override;
};

class AlarPreparingToDiveBombTrigger : public Trigger
{
public:
    AlarPreparingToDiveBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar preparing to dive bomb") {}
    bool IsActive() override;
};

class AlarNeedToManageTimersAndTrackersTrigger : public Trigger
{
public:
    AlarNeedToManageTimersAndTrackersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar need to manage timers and trackers") {}
    bool IsActive() override;
};

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
