#ifndef PLAYERBOTS_KARAACTIONS_H
#define PLAYERBOTS_KARAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class KarazhanEraseEncounterStatesAction : public Action
{
public:
    KarazhanEraseEncounterStatesAction(
        PlayerbotAI* botAI) : Action(botAI, "karazhan erase encounter states") {}
    bool Execute(Event event) override;
};

class ManaWarpStunCreatureBeforeWarpBreachAction : public AttackAction
{
public:
    ManaWarpStunCreatureBeforeWarpBreachAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "mana warp stun creature before warp breach") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanHandlePhaseOneAction : public AttackAction
{
public:
    AttumenTheHuntsmanHandlePhaseOneAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "attumen the huntsman handle phase one") {}
    bool Execute(Event event) override;

private:
    bool AssistTankMoveAttumenFromGroup(Unit* midnight, Unit* attumen);
};

class AttumenTheHuntsmanHandlePhaseTwoAction : public AttackAction
{
public:
    AttumenTheHuntsmanHandlePhaseTwoAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "attumen the huntsman handle phase two") {}
    bool Execute(Event event) override;

private:
    bool CurrentTankPositionAttumen(Unit* attumen);
    bool StackBehindAttumen(Unit* attumen);
};

class AttumenTheHuntsmanManageDpsTimerAction : public Action
{
public:
    AttumenTheHuntsmanManageDpsTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "attumen the huntsman manage dps timer") {}
    bool Execute(Event event) override;
};

class MoroesMainTankAttackBossAction : public AttackAction
{
public:
    MoroesMainTankAttackBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "moroes main tank attack boss") {}
    bool Execute(Event event) override;
};

class MoroesMarkTargetAction : public Action
{
public:
    MoroesMarkTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "moroes mark target") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueTankPositionBossAction : public AttackAction
{
public:
    MaidenOfVirtueTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "maiden of virtue tank position boss") {}
    bool Execute(Event event) override;

private:
    bool MoveBossToStunnedHealer(Unit* healer);
};

class MaidenOfVirtuePositionRangedAction : public MovementAction
{
public:
    MaidenOfVirtuePositionRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "maiden of virtue position ranged") {}
    bool Execute(Event event) override;
};

class BigBadWolfPositionBossAction : public AttackAction
{
public:
    BigBadWolfPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "big bad wolf position boss") {}
    bool Execute(Event event) override;
};

class BigBadWolfRunAwayFromBossAction : public MovementAction
{
public:
    BigBadWolfRunAwayFromBossAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "big bad wolf run away from boss") {}
    bool Execute(Event event) override;

private:
    uint8 _runIndex = 0;
};

class RomuloAndJulianneMarkTargetAction : public Action
{
public:
    RomuloAndJulianneMarkTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "romulo and julianne mark target") {}
    bool Execute(Event event) override;
};

class WizardOfOzMarkTargetAction : public Action
{
public:
    WizardOfOzMarkTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "wizard of oz mark target") {}
    bool Execute(Event event) override;
};

class WizardOfOzScorchStrawmanAction : public Action
{
public:
    WizardOfOzScorchStrawmanAction(
        PlayerbotAI* botAI) : Action(botAI, "wizard of oz scorch strawman") {}
    bool Execute(Event event) override;
};

class TheCuratorMarkAstralFlareAction : public Action
{
public:
    TheCuratorMarkAstralFlareAction(
        PlayerbotAI* botAI) : Action(botAI, "the curator mark astral flare") {}
    bool Execute(Event event) override;
};

class TheCuratorPositionBossAction : public AttackAction
{
public:
    TheCuratorPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "the curator position boss") {}
    bool Execute(Event event) override;
};

class TheCuratorSpreadRangedAction : public MovementAction
{
public:
    TheCuratorSpreadRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "the curator spread ranged") {}
    bool Execute(Event event) override;
};

class TerestianIllhoofMarkTargetAction : public Action
{
public:
    TerestianIllhoofMarkTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "terestian illhoof mark target") {}
    bool Execute(Event event) override;
};

class ShadeOfAranRunAwayFromArcaneExplosionAction : public MovementAction
{
public:
    ShadeOfAranRunAwayFromArcaneExplosionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "shade of aran run away from arcane explosion") {}
    bool Execute(Event event) override;
};

class ShadeOfAranStopMovingDuringFlameWreathAction : public MovementAction
{
public:
    ShadeOfAranStopMovingDuringFlameWreathAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "shade of aran stop moving during flame wreath") {}
    bool Execute(Event event) override;
};

class ShadeOfAranMarkConjuredElementalAction : public Action
{
public:
    ShadeOfAranMarkConjuredElementalAction(
        PlayerbotAI* botAI) : Action(botAI, "shade of aran mark conjured elemental") {}
    bool Execute(Event event) override;
};

class ShadeOfAranRangedMaintainDistanceAction : public MovementAction
{
public:
    ShadeOfAranRangedMaintainDistanceAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "shade of aran ranged maintain distance") {}
    bool Execute(Event event) override;
};

class NetherspiteBlockRedBeamAction : public MovementAction
{
public:
    NetherspiteBlockRedBeamAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "netherspite block red beam") {}
    bool Execute(Event event) override;
    bool ResetRedBeamState(time_t initialMoveTimer = 0)
    {
        if (_redBeamMoveTimer == initialMoveTimer && !_lastBeamMoveSideways && !_wasBlockingRedBeam)
            return false;
        _redBeamMoveTimer = initialMoveTimer;
        _lastBeamMoveSideways = false;
        _wasBlockingRedBeam = false;
        return true;
    }

private:
    time_t _redBeamMoveTimer = 0;
    bool _lastBeamMoveSideways = false;
    bool _wasBlockingRedBeam = false;
};

class NetherspiteBlockBlueBeamAction : public MovementAction
{
public:
    NetherspiteBlockBlueBeamAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "netherspite block blue beam") {}
    bool Execute(Event event) override;

private:
    bool _wasBlockingBlueBeam = false;
};

class NetherspiteBlockGreenBeamAction : public MovementAction
{
public:
    NetherspiteBlockGreenBeamAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "netherspite block green beam") {}
    bool Execute(Event event) override;

private:
    bool _wasBlockingGreenBeam = false;
};

class NetherspiteAvoidBeamAndVoidZoneAction : public MovementAction
{
public:
    NetherspiteAvoidBeamAndVoidZoneAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "netherspite avoid beam and void zone") {}
    bool Execute(Event event) override;

private:
    struct BeamAvoid
    {
        Unit* portal;
        float minDist, maxDist;
    };
    bool IsAwayFromBeams(float x, float y, const std::vector<BeamAvoid>& beams, Unit* netherspite);
};

class NetherspiteBanishPhaseAvoidVoidZoneAction : public MovementAction
{
public:
    NetherspiteBanishPhaseAvoidVoidZoneAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "netherspite banish phase avoid void zone") {}
    bool Execute(Event event) override;
};

class NetherspiteManageTimersAndTrackersAction : public Action
{
public:
    NetherspiteManageTimersAndTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "netherspite manage timers and trackers") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarEnfeebledAvoidHazardAction : public MovementAction
{
public:
    PrinceMalchezaarEnfeebledAvoidHazardAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "prince malchezaar enfeebled avoid hazard") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarNonTankAvoidInfernalAction : public MovementAction
{
public:
    PrinceMalchezaarNonTankAvoidInfernalAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "prince malchezaar non tank avoid infernal") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarMainTankMovementAction : public AttackAction
{
public:
    PrinceMalchezaarMainTankMovementAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "prince malchezaar main tank movement") {}
    bool Execute(Event event) override;
};

class NightbaneGroundPhasePositionBossAction : public AttackAction
{
public:
    NightbaneGroundPhasePositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "nightbane ground phase position boss") {}
    bool Execute(Event event) override;
    bool ResetTankStep()
    {
        if (_tankStep == 0)
            return false;
        _tankStep = 0;
        return true;
    }

private:
    uint8 _tankStep = 0;
};

class NightbaneGroundPhaseRotateRangedPositionsAction : public MovementAction
{
public:
    NightbaneGroundPhaseRotateRangedPositionsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "nightbane ground phase rotate ranged positions") {}
    bool Execute(Event event) override;
    bool ResetRangedStep()
    {
        if (_rangedStep == 0)
            return false;
        _rangedStep = 0;
        return true;
    }

private:
    uint8 _rangedStep = 0;
};

class NightbaneCastFearWardOnMainTankAction : public Action
{
public:
    NightbaneCastFearWardOnMainTankAction(
        PlayerbotAI* botAI) : Action(botAI, "nightbane cast fear ward on main tank") {}
    bool Execute(Event event) override;
};

class NightbaneControlPetAggressionAction : public Action
{
public:
    NightbaneControlPetAggressionAction(
        PlayerbotAI* botAI) : Action(botAI, "nightbane control pet aggression") {}
    bool Execute(Event event) override;
};

class NightbaneFlightPhaseMovementAction : public MovementAction
{
public:
    NightbaneFlightPhaseMovementAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "nightbane flight phase movement") {}
    bool Execute(Event event) override;
    bool ResetRainOfBonesHit()
    {
        if (!_rainOfBonesHit)
            return false;
        _rainOfBonesHit = false;
        return true;
    }

private:
    bool _rainOfBonesHit = false;
};

class NightbaneManageTimersAndTrackersAction : public Action
{
public:
    NightbaneManageTimersAndTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "nightbane manage timers and trackers") {}
    bool Execute(Event event) override;
};

#endif
