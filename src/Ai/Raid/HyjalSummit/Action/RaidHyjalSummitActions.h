/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

using HyjalSummitMovementAction = MovementAction;
using HyjalSummitAttackAction = AttackAction;

// General

class HyjalSummitEraseTrackersAction : public Action
{
public:
    HyjalSummitEraseTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "hyjal summit erase trackers") {}
    bool Execute(Event event) override;
};

// Rage Winterchill

class RageWinterchillMisdirectBossToMainTankAction : public HyjalSummitAttackAction
{
public:
    RageWinterchillMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "rage winterchill misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class RageWinterchillMainTankPositionBossAction : public HyjalSummitAttackAction
{
public:
    RageWinterchillMainTankPositionBossAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "rage winterchill main tank position boss") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public HyjalSummitMovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
};

class RageWinterchillMeleeGetOutOfDeathAndDecayAction : public HyjalSummitAttackAction
{
public:
    RageWinterchillMeleeGetOutOfDeathAndDecayAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "rage winterchill melee get out of death and decay") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossAndInfernalsToTanksAction : public HyjalSummitAttackAction
{
public:
    AnetheronMisdirectBossAndInfernalsToTanksAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "anetheron misdirect boss and infernals to tanks") {}
    bool Execute(Event event) override;
};

class AnetheronMainTankPositionBossAction : public HyjalSummitAttackAction
{
public:
    AnetheronMainTankPositionBossAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "anetheron main tank position boss") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInCircleAction : public HyjalSummitMovementAction
{
public:
    AnetheronSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "anetheron spread ranged in circle") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public HyjalSummitMovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronFirstAssistTankPickUpInfernalsAction : public HyjalSummitAttackAction
{
public:
    AnetheronFirstAssistTankPickUpInfernalsAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "anetheron first assist tank pick up infernals") {}
    bool Execute(Event event) override;
};

class AnetheronAssignDpsPriorityAction : public HyjalSummitAttackAction
{
public:
    AnetheronAssignDpsPriorityAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "anetheron assign dps priority") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

class KazrogalMisdirectBossToMainTankAction : public HyjalSummitAttackAction
{
public:
    KazrogalMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "kaz'rogal misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class KazrogalMainTankPositionBossAction : public HyjalSummitAttackAction
{
public:
    KazrogalMainTankPositionBossAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "kaz'rogal main tank position boss") {}
    bool Execute(Event event) override;
};

class KazrogalAssistTanksMoveInFrontOfBossAction : public HyjalSummitAttackAction
{
public:
    KazrogalAssistTanksMoveInFrontOfBossAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "kaz'rogal assist tanks move in front of boss") {}
    bool Execute(Event event) override;
};

class KazrogalSpreadRangedInArcAction : public HyjalSummitMovementAction
{
public:
    KazrogalSpreadRangedInArcAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "kaz'rogal spread ranged in arc") {}
    bool Execute(Event event) override;
};

class KazrogalLowManaBotTakeDefensiveMeasuresAction : public HyjalSummitMovementAction
{
public:
KazrogalLowManaBotTakeDefensiveMeasuresAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "kaz'rogal low mana bot take defensive measures") {}
    bool Execute(Event event) override;
};

class KazrogalCastShadowProtectionSpellAction : public Action
{
public:
    KazrogalCastShadowProtectionSpellAction(
        PlayerbotAI* botAI) : Action(botAI, "kaz'rogal cast shadow protection spell") {}
    bool Execute(Event event) override;
};

// Azgalor

class AzgalorMisdirectBossToMainTankAction : public HyjalSummitAttackAction
{
public:
    AzgalorMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "azgalor misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AzgalorMainTankPositionBossAction : public HyjalSummitAttackAction
{
public:
    AzgalorMainTankPositionBossAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "azgalor main tank position boss") {}
    bool Execute(Event event) override;
};

class AzgalorWaitAtSafePositionAction : public HyjalSummitMovementAction
{
public:
    AzgalorWaitAtSafePositionAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "azgalor wait at safe position") {}
    bool Execute(Event event) override;
};

class AzgalorDisperseRangedAction : public HyjalSummitMovementAction
{
public:
    AzgalorDisperseRangedAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "azgalor disperse ranged") {}
    bool Execute(Event event) override;
};

class AzgalorMeleeGetOutOfFireAndSwapTargetsAction : public HyjalSummitAttackAction
{
public:
    AzgalorMeleeGetOutOfFireAndSwapTargetsAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "azgalor melee get out of fire and swap targets") {}
    bool Execute(Event event) override;
};

class AzgalorMoveToDoomguardTankAction : public HyjalSummitMovementAction
{
public:
    AzgalorMoveToDoomguardTankAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "azgalor move to doomguard tank") {}
    bool Execute(Event event) override;
};

class AzgalorFirstAssistTankPositionDoomguardAction : public HyjalSummitAttackAction
{
public:
    AzgalorFirstAssistTankPositionDoomguardAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "azgalor first assist tank position doomguard") {}
    bool Execute(Event event) override;
};

class AzgalorRangedDpsPrioritizeDoomguardsAction : public HyjalSummitAttackAction
{
public:
    AzgalorRangedDpsPrioritizeDoomguardsAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "azgalor ranged dps prioritize doomguards") {}
    bool Execute(Event event) override;
};

// Archimonde

class ArchimondeMisdirectBossToMainTankAction : public HyjalSummitAttackAction
{
public:
    ArchimondeMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "archimonde misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ArchimondeMoveBossToInitialPositionAction : public HyjalSummitAttackAction
{
public:
    ArchimondeMoveBossToInitialPositionAction(
        PlayerbotAI* botAI) : HyjalSummitAttackAction(botAI, "archimonde move boss to initial position") {}
    bool Execute(Event event) override;
};

class ArchimondeCastFearImmunitySpellAction : public Action
{
public:
    ArchimondeCastFearImmunitySpellAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde cast fear immunity spell") {}
    bool Execute(Event event) override;

private:
    bool CastFearWardOnMainTank();
    bool UseTremorTotemStrategy();
};

class ArchimondeSpreadToAvoidAirBurstAction : public HyjalSummitMovementAction
{
public:
    ArchimondeSpreadToAvoidAirBurstAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "archimonde spread to avoid air burst") {}
    bool Execute(Event event) override;
};

class ArchimondeAvoidDoomfireAction : public HyjalSummitMovementAction
{
public:
    ArchimondeAvoidDoomfireAction(
        PlayerbotAI* botAI) : HyjalSummitMovementAction(botAI, "archimonde avoid doomfire") {}
    bool Execute(Event event) override;
};

class ArchimondeRemoveDoomfireDotAction : public Action
{
public:
    ArchimondeRemoveDoomfireDotAction(
        PlayerbotAI* botAI) : Action(botAI, "archimonde remove doomfire dot") {}
    bool Execute(Event event) override;
};

#endif
