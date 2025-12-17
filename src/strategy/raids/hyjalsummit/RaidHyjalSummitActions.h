#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Rage Winterchill

class RageWinterchillMisdirectBossToMainTankAction : public AttackAction
{
public:
    RageWinterchillMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class RageWinterchillMainTankPositionBossAction : public AttackAction
{
public:
    RageWinterchillMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "rage winterchill main tank position boss") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public MovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossToMainTankAction : public AttackAction
{
public:
    AnetheronMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class AnetheronMainTankPositionBossAction : public AttackAction
{
public:
    AnetheronMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron main tank position boss") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInArcAction : public MovementAction
{
public:
    AnetheronSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron spread ranged in arc") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public MovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronFirstAssistTankPickUpInfernalsAction : public AttackAction
{
public:
    AnetheronFirstAssistTankPickUpInfernalsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron first assist tank pick up infernals") {}
    bool Execute(Event event) override;
};

class AnetheronFocusAllDpsOnBossAction : public AttackAction
{
public:
    AnetheronFocusAllDpsOnBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "anetheron focus all dps on boss") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

// Azgalor

// Archimonde

#endif
