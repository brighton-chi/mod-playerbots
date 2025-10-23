#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERS_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERS_H

#include "Trigger.h"

class SpectralRetainerNeedTargetPriorityTrigger : public Trigger
{
public:
    SpectralRetainerNeedTargetPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "spectral retainer need target priority") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanNeedTargetPriorityTrigger : public Trigger
{
public:
    AttumenTheHuntsmanNeedTargetPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman need target priority") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanAttumenSpawnedTrigger : public Trigger
{
public:
    AttumenTheHuntsmanAttumenSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman attumen spawned") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanAttumenMountedTrigger : public Trigger
{
public:
    AttumenTheHuntsmanAttumenMountedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman attumen mounted") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanManageTimerTrigger : public Trigger
{
public:
    AttumenTheHuntsmanManageTimerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman manage timer") {}
    bool IsActive() override;
};

class MoroesNeedTargetPriorityTrigger : public Trigger
{
public:
    MoroesNeedTargetPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "moroes need target priority") {}
    bool IsActive() override;
};

class MaidenOfVirtueBossEngagedByMainTankTrigger : public Trigger
{
public:
    MaidenOfVirtueBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue boss engaged by main tank") {}
    bool IsActive() override;
};

class MaidenOfVirtueBossEngagedByRangedTrigger : public Trigger
{
public:
    MaidenOfVirtueBossEngagedByRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue boss engaged by ranged") {}
    bool IsActive() override;
};

class BigBadWolfBossEngagedByMainTankTrigger : public Trigger
{
public:
    BigBadWolfBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf boss engaged by main tank") {}
    bool IsActive() override;
};

class BigBadWolfBossIsChasingLittleRedRidingHoodTrigger : public Trigger
{
public:
    BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf boss is chasing little red riding hood") {}
    bool IsActive() override;
};

class RomuloAndJulianneBothBossesRevivedTrigger : public Trigger
{
public:
    RomuloAndJulianneBothBossesRevivedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "romulo and julianne both bosses revived") {}
    bool IsActive() override;
};

class WizardOfOzNeedTargetPriorityTrigger : public Trigger
{
public:
    WizardOfOzNeedTargetPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz need target priority") {}
    bool IsActive() override;
};

class WizardOfOzStrawmanIsVulnerableToFireTrigger : public Trigger
{
public:
    WizardOfOzStrawmanIsVulnerableToFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz strawman is vulnerable to fire") {}
    bool IsActive() override;
};
class TheCuratorAstralFlareSpawnedTrigger : public Trigger
{
public:
    TheCuratorAstralFlareSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator astral flare spawned") {}
    bool IsActive() override;
};

class TheCuratorBossEngagedByTanksTrigger : public Trigger
{
public:
    TheCuratorBossEngagedByTanksTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator boss engaged by tanks") {}
    bool IsActive() override;
};

class TheCuratorBossEngagedByRangedTrigger : public Trigger
{
public:
    TheCuratorBossEngagedByRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator boss engaged by ranged") {}
    bool IsActive() override;
};

class TerestianIllhoofNeedTargetPriorityTrigger : public Trigger
{
public:
    TerestianIllhoofNeedTargetPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "terestian illhoof need target priority") {}
    bool IsActive() override;
};

class ShadeOfAranArcaneExplosionIsCastingTrigger : public Trigger
{
public:
    ShadeOfAranArcaneExplosionIsCastingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran arcane explosion is casting") {}
    bool IsActive() override;
};

class ShadeOfAranFlameWreathIsCastingTrigger : public Trigger
{
public:
    ShadeOfAranFlameWreathIsCastingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran flame wreath is casting") {}
    bool IsActive() override;
};

class ShadeOfAranConjuredElementalsSummonedTrigger : public Trigger
{
public:
    ShadeOfAranConjuredElementalsSummonedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran conjured elementals summoned") {}
    bool IsActive() override;
};

class ShadeOfAranBossEngagedByRangedTrigger : public Trigger
{
public:
    ShadeOfAranBossEngagedByRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran boss engaged by ranged") {}
    bool IsActive() override;
};

class NetherspiteRedBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteRedBeamIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite red beam is active") {}
    bool IsActive() override;
};

class NetherspiteBlueBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteBlueBeamIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite blue beam is active") {}
    bool IsActive() override;
};

class NetherspiteGreenBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteGreenBeamIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite green beam is active") {}
    bool IsActive() override;
};

class NetherspiteBotIsNotBeamBlockerTrigger : public Trigger
{
public:
    NetherspiteBotIsNotBeamBlockerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite bot is not beam blocker") {}
    bool IsActive() override;
};

class NetherspiteBossIsBanishedTrigger : public Trigger
{
public:
    NetherspiteBossIsBanishedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite boss is banished") {}
    bool IsActive() override;
};

class NetherspiteNeedToManageTimersTrigger : public Trigger
{
public:
    NetherspiteNeedToManageTimersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite need to manage timers") {}
    bool IsActive() override;
};

class PrinceMalchezaarBotIsEnfeebledTrigger : public Trigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar bot is enfeebled") {}
    bool IsActive() override;
};

class PrinceMalchezaarInfernalsAreSpawnedTrigger : public Trigger
{
public:
    PrinceMalchezaarInfernalsAreSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar infernals are spawned") {}
    bool IsActive() override;
};

class PrinceMalchezaarBossEngagedByMainTankTrigger : public Trigger
{
public:
    PrinceMalchezaarBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar boss engaged by main tank") {}
    bool IsActive() override;
};

class NightbaneBossEngagedByMainTankTrigger : public Trigger
{
public:
    NightbaneBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane boss engaged by main tank") {}
    bool IsActive() override;
};

class NightbaneRangedPrepareForCharredEarthTrigger : public Trigger
{
public:
    NightbaneRangedPrepareForCharredEarthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane ranged prepare for charred earth") {}
    bool IsActive() override;
};

class NightbaneMainTankIsSusceptibleToFearTrigger : public Trigger
{
public:
    NightbaneMainTankIsSusceptibleToFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane main tank is susceptible to fear") {}
    bool IsActive() override;
};

class NightbanePetsIgnoreColllisionToChaseFlyingBossTrigger : public Trigger
{
public:
    NightbanePetsIgnoreColllisionToChaseFlyingBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane pets ignore collision to chase flying boss") {}
    bool IsActive() override;
};

class NightbaneBossIsFlyingTrigger : public Trigger
{
public:
    NightbaneBossIsFlyingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane boss is flying") {}
    bool IsActive() override;
};

class NightbaneNeedToManageTimersAndTrackersTrigger : public Trigger
{
public:
    NightbaneNeedToManageTimersAndTrackersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane need to manage timers and trackers") {}
    bool IsActive() override;
};

#endif
