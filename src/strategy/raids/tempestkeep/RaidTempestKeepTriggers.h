#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "Trigger.h"

class CrimsonHandCenturionCastsArcaneVolleyTrigger : public Trigger
{
public:
    CrimsonHandCenturionCastsArcaneVolleyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "crimson hand centurion casts arcane volley") {}
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

class AlarBossIsPreparingToDiveBombTrigger : public Trigger
{
public:
    AlarBossIsPreparingToDiveBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar boss is preparing to dive bomb") {}
    bool IsActive() override;
};

class AlarPhase2EncounterIsAtRoomCenterTrigger : public Trigger
{
public:
    AlarPhase2EncounterIsAtRoomCenterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar phase 2 encounter is at room center") {}
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

class VoidReaverBossLaunchesArcaneOrbsTrigger : public Trigger
{
public:
    VoidReaverBossLaunchesArcaneOrbsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver boss launches arcane orbs") {}
    bool IsActive() override;
};

class VoidReaverTanksLostAggroTrigger : public Trigger
{
public:
    VoidReaverTanksLostAggroTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver tanks lost aggro") {}
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

class HighAstromancerSolarianBossCastsPsychicScreamTrigger : public Trigger
{
public:
    HighAstromancerSolarianBossCastsPsychicScreamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian boss casts psychic scream") {}
    bool IsActive() override;
};

class KaelthasSunstriderNeedToTestStrategiesOnBossTrigger : public Trigger
{
public:
    KaelthasSunstriderNeedToTestStrategiesOnBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider need to test strategies on boss") {}
    bool IsActive() override;
};

class KaelthasSunstriderThaladredIsFixatedOnBotTrigger : public Trigger
{
public:
    KaelthasSunstriderThaladredIsFixatedOnBotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider thaladred is fixated on bot") {}
    bool IsActive() override;
};

class KaelthasSunstriderSanguinarEngagedByMainTankTrigger : public Trigger
{
public:
    KaelthasSunstriderSanguinarEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider sanguinar engaged by main tank") {}
    bool IsActive() override;
};

class KaelthasSunstriderSanguinarCastsBellowingRoarTrigger : public Trigger
{
public:
    KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider sanguinar casts bellowing roar") {}
    bool IsActive() override;
};

class KaelthasSunstriderCapernianEngagedByWarlockTankTrigger : public Trigger
{
public:
    KaelthasSunstriderCapernianEngagedByWarlockTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider capernian engaged by warlock tank") {}
    bool IsActive() override;
};

class KaelthasSunstriderCapernianCastsArcaneBurstTrigger : public Trigger
{
public:
    KaelthasSunstriderCapernianCastsArcaneBurstTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider capernian casts arcane burst") {}
    bool IsActive() override;
};

class KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider telonicus engaged by first assist tank") {}
    bool IsActive() override;
};

class KaelthasSunstriderPullingTankableAdvisorsTrigger : public Trigger
{
public:
    KaelthasSunstriderPullingTankableAdvisorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider pulling tankable advisors") {}
    bool IsActive() override;
};

class KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger : public Trigger
{
public:
    KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider waiting for tanks to get aggro on advisors") {}
    bool IsActive() override;
};

class KaelthasSunstriderLegendaryWeaponsAreAliveTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider legendary weapons are alive") {}
    bool IsActive() override;
};

class KaelthasSunstriderDevastationChannelsWhirlwindTrigger : public Trigger
{
public:
    KaelthasSunstriderDevastationChannelsWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider devastation channels whirlwind") {}
    bool IsActive() override;
};

class KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger : public Trigger
{
public:
    KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider netherstrand longbow fires multishot") {}
    bool IsActive() override;
};

class KaelthasSunstriderCheatToTestTrigger : public Trigger
{
public:
    KaelthasSunstriderCheatToTestTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas sunstrider cheat to test") {}
    bool IsActive() override;
};

#endif
