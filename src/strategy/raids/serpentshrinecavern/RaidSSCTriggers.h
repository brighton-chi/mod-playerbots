#ifndef _PLAYERBOT_RAIDSSCTRIGGERS_H
#define _PLAYERBOT_RAIDSSCTRIGGERS_H

#include "Trigger.h"

class GreyheartTidecallerWaterElementalTotemSpawnedTrigger : public Trigger
{
public:
    GreyheartTidecallerWaterElementalTotemSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "greyheart tidecaller water elemental totem spawned") {}
    bool IsActive() override;
};

class HydrossTheUnstableBotIsFrostTankTrigger : public Trigger
{
public:
    HydrossTheUnstableBotIsFrostTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable bot is frost tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableBotIsNatureTankTrigger : public Trigger
{
public:
    HydrossTheUnstableBotIsNatureTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable bot is nature tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableWaitingForDPSTrigger : public Trigger
{
public:
    HydrossTheUnstableWaitingForDPSTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable waiting for dps") {}
    bool IsActive() override;
};

class HydrossTheUnstableElementalsSpawnedTrigger : public Trigger
{
public:
    HydrossTheUnstableElementalsSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable elementals spawned") {}
    bool IsActive() override;
};

class HydrossTheUnstableDangerFromWaterTombsTrigger : public Trigger
{
public:
    HydrossTheUnstableDangerFromWaterTombsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable danger from water tombs") {}
    bool IsActive() override;
};

class MorogrimTidewalkerPullingBossTrigger : public Trigger
{
public:
    MorogrimTidewalkerPullingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker pulling boss") {}
    bool IsActive() override;
};

class MorogrimTidewalkerBossEngagedByMainTankTrigger : public Trigger
{
public:
    MorogrimTidewalkerBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker boss engaged by main tank") {}
    bool IsActive() override;
};

class MorogrimTidewalkerWaterGlobulesAreIncomingTrigger : public Trigger
{
public:
    MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker water globules are incoming") {}
    bool IsActive() override;
};

#endif
