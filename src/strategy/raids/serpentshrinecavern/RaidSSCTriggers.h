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

class TheLurkerBelowSpoutIsActiveTrigger : public Trigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the lurker below spout is active") {}
    bool IsActive() override;
};

class LeotherasTheBlindHumanFormEngagedByMainTankTrigger : public Trigger
{
public:
    LeotherasTheBlindHumanFormEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind human form engaged by main tank") {}
    bool IsActive() override;
};

class LeotherasTheBlindDemonFormEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    LeotherasTheBlindDemonFormEngagedByFirstAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind demon form engaged by first assist tank") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossEngagedByRangedTrigger : public Trigger
{
public:
    LeotherasTheBlindBossEngagedByRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss engaged by ranged") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossChannelingWhirlwindTrigger : public Trigger
{
public:
    LeotherasTheBlindBossChannelingWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss channeling whirlwind") {}
    bool IsActive() override;
};

class LeotherasTheBlindDemonFormEngagedByMeleeTrigger : public Trigger
{
public:
    LeotherasTheBlindDemonFormEngagedByMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind demon form engaged by melee") {}
    bool IsActive() override;
};

class LeotherasTheBlindInnerDemonHasTakenForm : public Trigger
{
public:
    LeotherasTheBlindInnerDemonHasTakenForm(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind inner demon has taken form") {}
    bool IsActive() override;
};

class LeotherasTheBlindEnteredFinalPhaseTrigger : public Trigger
{
public:
    LeotherasTheBlindEnteredFinalPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind entered final phase") {}
    bool IsActive() override;
};

class LeotherasTheBlindNeedToManageTimersAndTrackersTrigger : public Trigger
{
public:
    LeotherasTheBlindNeedToManageTimersAndTrackersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind need to manage timers and trackers") {}
    bool IsActive() override;
};

class FathomLordKarathressBossEngagedByMainTankTrigger : public Trigger
{
public:
    FathomLordKarathressBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress boss engaged by main tank") {}
    bool IsActive() override;
};

class FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress sharkkis engaged by first assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress tidalvess engaged by second assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress caribdis engaged by third assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress caribdis tank needs dedicated healer") {}
    bool IsActive() override;
};

class FathomLordKarathressPullingBossesTrigger : public Trigger
{
public:
    FathomLordKarathressPullingBossesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress pulling bosses") {}
    bool IsActive() override;
};

class FathomLordKarathressDeterminingMeleeDPSKillOrderTrigger : public Trigger
{
public:
    FathomLordKarathressDeterminingMeleeDPSKillOrderTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress determining melee dps kill order") {}
    bool IsActive() override;
};

class FathomLordKarathressDeterminingRangedDPSKillOrderTrigger : public Trigger
{
public:
    FathomLordKarathressDeterminingRangedDPSKillOrderTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress determining ranged dps kill order") {}
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
