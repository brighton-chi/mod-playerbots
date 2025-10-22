#ifndef _PLAYERBOT_RAIDSSCACTIONS_H
#define _PLAYERBOT_RAIDSSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class GreyheartTidecallerMarkWaterElementalTotemAction : public Action
{
public:
    GreyheartTidecallerMarkWaterElementalTotemAction(PlayerbotAI* botAI, std::string const name = "greyheart tidecaller mark water elemental totem") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostTankAction : public AttackAction
{
public:
    HydrossTheUnstableFrostTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable frost tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

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

class HydrossTheUnstableMarkElementalAddsAction : public Action
{
public:
    HydrossTheUnstableMarkElementalAddsAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable mark elemental adds") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstableFrostPhaseSpreadOutAction : public MovementAction
{
public:
    HydrossTheUnstableFrostPhaseSpreadOutAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable frost phase spread out") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
