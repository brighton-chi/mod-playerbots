#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

/* class AttumenTheHuntsmanMainTankAction : public AttackAction
{
public:
    AttumenTheHuntsmanMainTankAction(PlayerbotAI* botAI, std::string const name = "attumen the huntsman main tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AttumenTheHuntsmanSplitBossesAction : public AttackAction
{
public:
    AttumenTheHuntsmanSplitBossesAction(PlayerbotAI* botAI, std::string const name = "attumen the huntsman split bosses") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AttumenTheHuntsmanStackBehindAction : public MovementAction
{
public:
    AttumenTheHuntsmanStackBehindAction(PlayerbotAI* botAI, std::string const name = "attumen the huntsman stack behind") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AttumenTheHuntsmanManageTimerAction : public Action
{
public:
    AttumenTheHuntsmanManageTimerAction(PlayerbotAI* botAI, std::string const name = "attumen the huntsman manage timer") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class MoroesMarkTargetAction : public Action
{
public:
    MoroesMarkTargetAction(PlayerbotAI* botAI, std::string const name = "moroes mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class MaidenOfVirtueMainTankMovementAction : public AttackAction
{
public:
    MaidenOfVirtueMainTankMovementAction(PlayerbotAI* botAI, std::string const name = "maiden of virtue main tank movement") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class MaidenOfVirtuePositionRangedAction : public MovementAction
{
public:
    MaidenOfVirtuePositionRangedAction(PlayerbotAI* botAI, std::string const name = "maiden of virtue position ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class BigBadWolfPositionBossAction : public AttackAction
{
public:
    BigBadWolfPositionBossAction(PlayerbotAI* botAI, std::string const name = "big bad wolf position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class BigBadWolfRunAwayAction : public MovementAction
{
public:
    BigBadWolfRunAwayAction(PlayerbotAI* botAI, std::string const name = "big bad wolf run away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class RomuloAndJulianneMarkTargetAction : public Action
{
public:
    RomuloAndJulianneMarkTargetAction(PlayerbotAI* botAI, std::string const name = "romulo and julianne mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class WizardOfOzMarkTargetAction : public Action
{
public:
    WizardOfOzMarkTargetAction(PlayerbotAI* botAI, std::string const name = "wizard of oz mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class WizardOfOzScorchStrawmanAction : public Action
{
public:
    WizardOfOzScorchStrawmanAction(PlayerbotAI* botAI, std::string const name = "wizard of oz scorch strawman") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class TheCuratorMarkAstralFlareAction : public Action
{
public:
    TheCuratorMarkAstralFlareAction(PlayerbotAI* botAI, std::string const name = "the curator mark astral flare") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class TheCuratorPositionBossAction : public AttackAction
{
public:
    TheCuratorPositionBossAction(PlayerbotAI* botAI, std::string const name = "the curator position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TheCuratorSpreadRangedAction : public MovementAction
{
public:
    TheCuratorSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "the curator spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class TerestianIllhoofMarkTargetAction : public Action
{
public:
    TerestianIllhoofMarkTargetAction(PlayerbotAI* botAI, std::string const name = "terestian illhoof mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class ShadeOfAranArcaneExplosionRunAwayAction : public MovementAction
{
public:
    ShadeOfAranArcaneExplosionRunAwayAction(PlayerbotAI* botAI, std::string const name = "shade of aran arcane explosion run away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class ShadeOfAranFlameWreathStopMovementAction : public MovementAction
{
public:
    ShadeOfAranFlameWreathStopMovementAction(PlayerbotAI* botAI, std::string const name = "shade of aran flame wreath stop bot") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class ShadeOfAranMarkConjuredElementalAction : public Action
{
public:
    ShadeOfAranMarkConjuredElementalAction(PlayerbotAI* botAI, std::string const name = "shade of aran mark conjured elemental") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class ShadeOfAranSpreadRangedAction : public MovementAction
{
public:
    ShadeOfAranSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "shade of aran spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteBlockRedBeamAction : public MovementAction
{
public:
    NetherspiteBlockRedBeamAction(PlayerbotAI* botAI, std::string const name = "netherspite block red beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteBlockBlueBeamAction : public MovementAction
{
public:
    NetherspiteBlockBlueBeamAction(PlayerbotAI* botAI, std::string const name = "netherspite block blue beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteBlockGreenBeamAction : public MovementAction
{
public:
    NetherspiteBlockGreenBeamAction(PlayerbotAI* botAI, std::string const name = "netherspite block green beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteAvoidBeamAndVoidZoneAction : public MovementAction
{
public:
    NetherspiteAvoidBeamAndVoidZoneAction(PlayerbotAI* botAI, std::string const name = "netherspite avoid beam and void zone") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteBanishPhaseAvoidVoidZoneAction : public MovementAction
{
public:
    NetherspiteBanishPhaseAvoidVoidZoneAction(PlayerbotAI* botAI, std::string const name = "netherspite banish phase avoid void zone") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NetherspiteManageTimersAction : public Action
{
public:
    NetherspiteManageTimersAction(PlayerbotAI* botAI, std::string const name = "netherspite manage timers") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class PrinceMalchezaarEnfeebledAvoidHazardAction : public MovementAction
{
public:
    PrinceMalchezaarEnfeebledAvoidHazardAction(PlayerbotAI* botAI, std::string const name = "prince malchezaar enfeebled avoid hazard") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class PrinceMalchezaarNonTankAvoidInfernalAction : public MovementAction
{
public:
    PrinceMalchezaarNonTankAvoidInfernalAction(PlayerbotAI* botAI, std::string const name = "prince malchezaar non tank avoid infernal") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class PrinceMalchezaarMainTankMovementAction : public AttackAction
{
public:
    PrinceMalchezaarMainTankMovementAction(PlayerbotAI* botAI, std::string const name = "prince malchezaar main tank movement") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneGroundPhasePositionBossAction : public AttackAction
{
public:
    NightbaneGroundPhasePositionBossAction(PlayerbotAI* botAI, std::string const name = "nightbane ground phase position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneGroundPhaseRotateRangedPositionsAction : public MovementAction
{
public:
    NightbaneGroundPhaseRotateRangedPositionsAction(PlayerbotAI* botAI, std::string const name = "nightbane ground phase rotate ranged positions") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneCastFearWardOnMainTankAction : public Action
{
public:
    NightbaneCastFearWardOnMainTankAction(PlayerbotAI* botAI, std::string const name = "nightbane cast fear ward on main tank") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneControlPetAggressionAction : public Action
{
public:
    NightbaneControlPetAggressionAction(PlayerbotAI* botAI, std::string const name = "nightbane control pet aggression") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneFlightPhaseMovementAction : public MovementAction
{
public:
    NightbaneFlightPhaseMovementAction(PlayerbotAI* botAI, std::string const name = "nightbane flight phase movement") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class NightbaneManageTimersAndTrackersAction : public Action
{
public:
    NightbaneManageTimersAndTrackersAction(PlayerbotAI* botAI, std::string const name = "nightbane manage timers and trackers") : Action(botAI, name) {}

    bool Execute(Event event) override;
}; */

#endif
