#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HydrossTheUnstableFrostTankPositionBossAction : public AttackAction
{
public:
    HydrossTheUnstableFrostTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable frost tank position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostTankMoveBossToNatureTankAction : public MovementAction
{
public:
    HydrossTheUnstableFrostTankMoveBossToNatureTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable frost tank move boss to nature tank") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableNatureTankPositionBossAction : public AttackAction
{
public:
    HydrossTheUnstableNatureTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable nature tank position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableNatureTankMoveBossToFrostTankAction : public MovementAction
{
public:
    HydrossTheUnstableNatureTankMoveBossToFrostTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable nature tank move boss to frost tank") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableManageDPSTimerAction : public Action
{
public:
    HydrossTheUnstableManageDPSTimerAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable manage dps timer") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
