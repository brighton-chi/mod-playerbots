#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "Trigger.h"

/*
class AttumenTheHuntsmanMainTankTrigger : public Trigger
{
public:
    AttumenTheHuntsmanMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman main tank") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanSplitBossesTrigger : public Trigger
{
public:
    AttumenTheHuntsmanSplitBossesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman split bosses") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanStackBehindTrigger : public Trigger
{
public:
    AttumenTheHuntsmanStackBehindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman stack behind") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanManageTimerTrigger : public Trigger
{
public:
    AttumenTheHuntsmanManageTimerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman manage timer") {}
    bool IsActive() override;
};

class MoroesMarkTargetTrigger : public Trigger
{
public:
    MoroesMarkTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "moroes mark target") {}
    bool IsActive() override;
};

class MaidenOfVirtueMainTankMovementTrigger : public Trigger
{
public:
    MaidenOfVirtueMainTankMovementTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue main tank movement") {}
    bool IsActive() override;
};

class MaidenOfVirtuePositionRangedTrigger : public Trigger
{
public:
    MaidenOfVirtuePositionRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue position ranged") {}
    bool IsActive() override;
};

class BigBadWolfRunAwayTrigger : public Trigger
{
public:
    BigBadWolfRunAwayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf run away") {}
    bool IsActive() override;
};

class BigBadWolfPositionBossTrigger : public Trigger
{
public:
    BigBadWolfPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf position boss") {}
    bool IsActive() override;
};

class RomuloAndJulianneMarkTargetTrigger : public Trigger
{
public:
    RomuloAndJulianneMarkTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "romulo and julianne mark target") {}
    bool IsActive() override;
};

class WizardOfOzMarkTargetTrigger : public Trigger
{
public:
    WizardOfOzMarkTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz mark target") {}
    bool IsActive() override;
};

class WizardOfOzScorchStrawmanTrigger : public Trigger
{
public:
    WizardOfOzScorchStrawmanTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz scorch strawman") {}
    bool IsActive() override;
};

class TheCuratorMarkAstralFlareTrigger : public Trigger
{
public:
    TheCuratorMarkAstralFlareTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator mark astral flare") {}
    bool IsActive() override;
};

class TheCuratorPositionBossTrigger : public Trigger
{
public:
    TheCuratorPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator position boss") {}
    bool IsActive() override;
};

class TheCuratorSpreadRangedTrigger : public Trigger
{
public:
    TheCuratorSpreadRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "the curator spread ranged") {}
    bool IsActive() override;
};

class TerestianIllhoofMarkTargetTrigger : public Trigger
{
public:
    TerestianIllhoofMarkTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "terestian illhoof mark target") {}
    bool IsActive() override;
};

class ShadeOfAranArcaneExplosionRunAwayTrigger : public Trigger
{
public:
    ShadeOfAranArcaneExplosionRunAwayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran arcane explosion run away") {}
    bool IsActive() override;
};

class ShadeOfAranFlameWreathStopMovementTrigger : public Trigger
{
public:
    ShadeOfAranFlameWreathStopMovementTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran flame wreath stop movement") {}
    bool IsActive() override;
};

class ShadeOfAranMarkConjuredElementalTrigger : public Trigger
{
public:
    ShadeOfAranMarkConjuredElementalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran mark conjured elemental") {}
    bool IsActive() override;
};

class ShadeOfAranSpreadRangedTrigger : public Trigger
{
public:
    ShadeOfAranSpreadRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shade of aran spread ranged") {}
    bool IsActive() override;
};

class NetherspiteBlockRedBeamTrigger : public Trigger
{
public:
    NetherspiteBlockRedBeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite block red beam") {}
    bool IsActive() override;
};

class NetherspiteBlockBlueBeamTrigger : public Trigger
{
public:
    NetherspiteBlockBlueBeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite block blue beam") {}
    bool IsActive() override;
};

class NetherspiteBlockGreenBeamTrigger : public Trigger
{
public:
    NetherspiteBlockGreenBeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite block green beam") {}
    bool IsActive() override;
};

class NetherspiteAvoidBeamAndVoidZoneTrigger : public Trigger
{
public:
    NetherspiteAvoidBeamAndVoidZoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite avoid beam and void zone") {}
    bool IsActive() override;
};

class NetherspiteBanishPhaseAvoidVoidZoneTrigger : public Trigger
{
public:
    NetherspiteBanishPhaseAvoidVoidZoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite banish phase avoid void zone") {}
    bool IsActive() override;
};

class NetherspiteManageTimersTrigger : public Trigger
{
public:
    NetherspiteManageTimersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "netherspite manage timers") {}
    bool IsActive() override;
};

class PrinceMalchezaarEnfeebledAvoidHazardTrigger : public Trigger
{
public:
    PrinceMalchezaarEnfeebledAvoidHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar enfeebled avoid hazard") {}
    bool IsActive() override;
};

class PrinceMalchezaarNonTankAvoidInfernalTrigger : public Trigger
{
public:
    PrinceMalchezaarNonTankAvoidInfernalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar non tank avoid infernal") {}
    bool IsActive() override;
};

class PrinceMalchezaarMainTankMovementTrigger : public Trigger
{
public:
    PrinceMalchezaarMainTankMovementTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar main tank movement") {}
    bool IsActive() override;
};

class NightbaneGroundPhasePositionBossTrigger : public Trigger
{
public:
    NightbaneGroundPhasePositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane ground phase position boss") {}
    bool IsActive() override;
};

class NightbaneGroundPhaseRotateRangedPositionsTrigger : public Trigger
{
public:
    NightbaneGroundPhaseRotateRangedPositionsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane ground phase rotate ranged positions") {}
    bool IsActive() override;
};

class NightbaneCastFearWardOnMainTankTrigger : public Trigger
{
public:
    NightbaneCastFearWardOnMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane cast fear ward on main tank") {}
    bool IsActive() override;
};

class NightbaneControlPetAggressionTrigger : public Trigger
{
public:
    NightbaneControlPetAggressionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane control pet aggression") {}
    bool IsActive() override;
};

class NightbaneFlightPhaseMovementTrigger : public Trigger
{
public:
    NightbaneFlightPhaseMovementTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane flight phase movement") {}
    bool IsActive() override;
};

class NightbaneManageTimersAndTrackersTrigger : public Trigger
{
public:
    NightbaneManageTimersAndTrackersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nightbane manage timers and trackers") {}
    bool IsActive() override;
}; */

#endif
