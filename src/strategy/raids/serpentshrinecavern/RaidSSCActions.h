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

class HydrossTheUnstablePositionFrostTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionFrostTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable position frost tank") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HydrossTheUnstablePositionNatureTankAction : public AttackAction
{
public:
    HydrossTheUnstablePositionNatureTankAction(PlayerbotAI* botAI, std::string const name = "hydross the unstable position nature tank") : AttackAction(botAI, name) {}

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

class MorogrimTidewalkerMisdirectBossToMainTankAction : public AttackAction
{
public:
    MorogrimTidewalkerMisdirectBossToMainTankAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerMoveBossToTankPositionAction : public AttackAction
{
public:
    MorogrimTidewalkerMoveBossToTankPositionAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker move boss to tank position") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerPhase2RepositionDPSAndHealersAction : public MovementAction
{
public:
    MorogrimTidewalkerPhase2RepositionDPSAndHealersAction(PlayerbotAI* botAI, std::string const name = "morogrim tidewalker phase 2 reposition dps and healers") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
