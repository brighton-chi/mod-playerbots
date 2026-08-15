/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALACTIONS_H
#define PLAYERBOTS_HYJALACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class HyjalSummitResetEncounterStatesAction : public Action
{
public:
    HyjalSummitResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "hyjal summit reset encounter states") {}
    bool Execute(Event event) override;
};

// Rage Winterchill

class RageWinterchillMisdirectBossToMainTankAction : public Action
{
public:
    RageWinterchillMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "rage winterchill misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class RageWinterchillMainTankPositionBossAction : public AttackAction
{
public:
    RageWinterchillMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "rage winterchill main tank position boss") {}
    bool Execute(Event event) override;
};

class RageWinterchillRangedGetOutOfDeathAndDecayAction : public MovementAction
{
public:
    RageWinterchillRangedGetOutOfDeathAndDecayAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "rage winterchill ranged get out of death and decay") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public MovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
    bool ResetWinterchillPositionReached()
    {
        if (!_winterchillPositionReached)
            return false;
        _winterchillPositionReached = false;
        return true;
    }

private:
    bool _winterchillPositionReached = false;
};

class RageWinterchillMeleeGetOutOfDeathAndDecayAction : public AttackAction
{
public:
    RageWinterchillMeleeGetOutOfDeathAndDecayAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "rage winterchill melee get out of death and decay") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossAndInfernalsToTanksAction : public Action
{
public:
    AnetheronMisdirectBossAndInfernalsToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "anetheron misdirect boss and infernals to tanks") {}
    bool Execute(Event event) override;
};

class AnetheronMainTankPositionBossAction : public AttackAction
{
public:
    AnetheronMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "anetheron main tank position boss") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInCircleAction : public MovementAction
{
public:
    AnetheronSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron spread ranged in circle") {}
    bool Execute(Event event) override;
    bool ResetAnetheronPositionReached()
    {
        if (!_anetheronPositionReached)
            return false;
        _anetheronPositionReached = false;
        return true;
    }

private:
    bool _anetheronPositionReached = false;
};

class AnetheronMoveAwayFromInfernoTargetAction : public MovementAction
{
public:
    AnetheronMoveAwayFromInfernoTargetAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron move away from inferno target") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public MovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronInfernalTankTakePositionAction : public MovementAction
{
public:
    AnetheronInfernalTankTakePositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron infernal tank take position") {}
    bool Execute(Event event) override;
};

class AnetheronAssignDpsPriorityAction : public AttackAction
{
public:
    AnetheronAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "anetheron assign dps priority") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

class KazrogalMisdirectBossToMainTankAction : public Action
{
public:
    KazrogalMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class KazrogalMainTankPositionBossAction : public AttackAction
{
public:
    KazrogalMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kaz'rogal main tank position boss") {}
    bool Execute(Event event) override;
};

class KazrogalAssistTanksMoveInFrontOfBossAction : public AttackAction
{
public:
    KazrogalAssistTanksMoveInFrontOfBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kaz'rogal assist tanks move in front of boss") {}
    bool Execute(Event event) override;
};

class KazrogalSpreadRangedInArcAction : public MovementAction
{
public:
    KazrogalSpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kaz'rogal spread ranged in arc") {}
    bool Execute(Event event) override;
};

class KazrogalMoveAwayFromGroupAction : public MovementAction
{
public:
    KazrogalMoveAwayFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kaz'rogal move away from group") {}
    bool Execute(Event event) override;
};

class KazrogalActivateAspectOfTheViperAction : public Action
{
public:
    KazrogalActivateAspectOfTheViperAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal activate aspect of the viper") {}
    bool Execute(Event event) override;
};

class KazrogalCancelMarkAction : public Action
{
public:
    KazrogalCancelMarkAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal cancel mark") {}
    bool Execute(Event event) override;
};

class KazrogalCastShadowWardAction : public Action
{
public:
    KazrogalCastShadowWardAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal cast shadow ward") {}
    bool Execute(Event event) override;
};

// Azgalor

class AzgalorMisdirectBossToMainTankAction : public Action
{
public:
    AzgalorMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "azgalor misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AzgalorMainTankPositionBossAction : public AttackAction
{
public:
    AzgalorMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor main tank position boss") {}
    bool Execute(Event event) override;
};

class AzgalorWaitAtSafePositionAction : public MovementAction
{
public:
    AzgalorWaitAtSafePositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor wait at safe position") {}
    bool Execute(Event event) override;
};

class AzgalorDisperseRangedAction : public MovementAction
{
public:
    AzgalorDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor disperse ranged") {}
    bool Execute(Event event) override;
};

class AzgalorMeleeManueverThroughFireAction : public AttackAction
{
public:
    AzgalorMeleeManueverThroughFireAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor melee manuever through fire") {}
    bool Execute(Event event) override;
};

class AzgalorRangedGetOutOfRainOfFireAction : public MovementAction
{
public:
    AzgalorRangedGetOutOfRainOfFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor ranged get out of rain of fire") {}
    bool Execute(Event event) override;
};

class AzgalorMoveToDoomguardTankAction : public MovementAction
{
public:
    AzgalorMoveToDoomguardTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor move to doomguard tank") {}
    bool Execute(Event event) override;
};

class AzgalorFirstAssistTankPositionDoomguardAction : public AttackAction
{
public:
    AzgalorFirstAssistTankPositionDoomguardAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor first assist tank position doomguard") {}
    bool Execute(Event event) override;
};

class AzgalorDetermineDpsPriorityAction : public AttackAction
{
public:
    AzgalorDetermineDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor determine dps priority") {}
    bool Execute(Event event) override;
};

// Archimonde

class ArchimondeMisdirectBossToMainTankAction : public Action
{
public:
    ArchimondeMisdirectBossToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "archimonde misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ArchimondeMoveBossToInitialPositionAction : public AttackAction
{
public:
    ArchimondeMoveBossToInitialPositionAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "archimonde move boss to initial position") {}
    bool Execute(Event event) override;
};

class ArchimondeCastFearImmunitySpellAction : public Action
{
public:
    ArchimondeCastFearImmunitySpellAction(PlayerbotAI* botAI)
        : Action(botAI, "archimonde cast fear immunity spell") {}
    bool Execute(Event event) override;

private:
    bool CastFearWardOnMainTank();
    bool SetTremorTotem();
};

class ArchimondeSpreadToAvoidAirBurstAction : public MovementAction
{
public:
    ArchimondeSpreadToAvoidAirBurstAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde spread to avoid air burst") {}
    bool Execute(Event event) override;
};

class ArchimondeSpreadRangedAction : public MovementAction
{
public:
    ArchimondeSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde spread ranged") {}
    bool Execute(Event event) override;
};

class ArchimondeAvoidDoomfireAction : public MovementAction
{
public:
    ArchimondeAvoidDoomfireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde avoid doomfire") {}
    bool Execute(Event event) override;
};

class ArchimondeRemoveDoomfireDotAction : public Action
{
public:
    ArchimondeRemoveDoomfireDotAction(PlayerbotAI* botAI)
        : Action(botAI, "archimonde remove doomfire dot") {}
    bool Execute(Event event) override;
};

#endif
