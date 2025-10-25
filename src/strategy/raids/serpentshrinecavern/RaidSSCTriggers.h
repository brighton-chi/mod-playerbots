#ifndef _PLAYERBOT_RAIDSSCTRIGGERS_H
#define _PLAYERBOT_RAIDSSCTRIGGERS_H

#include "Trigger.h"

class GreyheartTidecallerWaterElementalTotemSpawnedTrigger : public Trigger
{
public:
    GreyheartTidecallerWaterElementalTotemSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "greyheart tidecaller water elemental totem spawned") {}
    bool IsActive() override;
};

class RancidMushroomSpawnedTrigger : public Trigger
{
public:
    RancidMushroomSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rancid mushroom spawned") {}
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

class HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger : public Trigger
{
public:
    HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable tank needs aggro upon phase change") {}
    bool IsActive() override;
};

class HydrossTheUnstableNeedToTransitionBeforeFourthMarkTrigger : public Trigger
{
public:
    HydrossTheUnstableNeedToTransitionBeforeFourthMarkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable need to transition before fourth mark") {}
    bool IsActive() override;
};

class HydrossTheUnstableWaitingForDPSTrigger : public Trigger
{
public:
    HydrossTheUnstableWaitingForDPSTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable waiting for dps") {}
    bool IsActive() override;
};

class TheLurkerBelowSpoutIsActiveTrigger : public Trigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the lurker below spout is active") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossIsInactiveTrigger : public Trigger
{
public:
    LeotherasTheBlindBossIsInactiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss is inactive") {}
    bool IsActive() override;
};

class LeotherasTheBlindHumanFormEngagedByMainTankTrigger : public Trigger
{
public:
    LeotherasTheBlindHumanFormEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind human form engaged by main tank") {}
    bool IsActive() override;
};

class LeotherasTheBlindEngagedByDemonFormTankTrigger : public Trigger
{
public:
    LeotherasTheBlindEngagedByDemonFormTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind engaged by demon form tank") {}
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

/* class LeotherasTheBlindDemonFormEngagedByMeleeTrigger : public Trigger
{
public:
    LeotherasTheBlindDemonFormEngagedByMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind demon form engaged by melee") {}
    bool IsActive() override;
}; */

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

class LeotherasTheBlindDemonFormTankNeedsAggro : public Trigger
{
public:
    LeotherasTheBlindDemonFormTankNeedsAggro(PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind demon form tank needs aggro") {}
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

class FathomLordKarathressNeedDedicatedHealerForKarathressTankTrigger : public Trigger
{
public:
    FathomLordKarathressNeedDedicatedHealerForKarathressTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress need dedicated healer for karathress tank") {}
    bool IsActive() override;
};

class FathomLordKarathressNeedDedicatedHealerForCaribdisTankTrigger : public Trigger
{
public:
    FathomLordKarathressNeedDedicatedHealerForCaribdisTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress need dedicated healer for caribdis tank") {}
    bool IsActive() override;
};

class FathomLordKarathressNeedDedicatedHealerForTidalvessTankTrigger : public Trigger
{
public:
    FathomLordKarathressNeedDedicatedHealerForTidalvessTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress need dedicated healer for tidalvess tank") {}
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
