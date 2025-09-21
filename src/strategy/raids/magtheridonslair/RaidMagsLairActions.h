#ifndef _PLAYERBOT_RAIDMAGSLAIRACTIONS_H
#define _PLAYERBOT_RAIDMAGSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class MagtheridonPositionBossAction : public MovementAction
{
public:
    MagtheridonPositionBossAction(PlayerbotAI* botAI, std::string const name = "magtheridon position boss") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonManticronCubeClickerPositionAction : public MovementAction
{
public:
    MagtheridonManticronCubeClickerPositionAction(PlayerbotAI* botAI, std::string const name = "magtheridon manticron cube clicker position") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
