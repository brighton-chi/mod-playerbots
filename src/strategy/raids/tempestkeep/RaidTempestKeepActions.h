#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class VoidReaverPositionBossAction : public AttackAction
{
public:
    VoidReaverPositionBossAction(PlayerbotAI* botAI, std::string const name = "void reaver position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class VoidReaverSpreadRangedAction : public MovementAction
{
public:
    VoidReaverSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "void reaver spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class VoidReaverArcaneOrbMoveAwayAction : public MovementAction
{
public:
    VoidReaverArcaneOrbMoveAwayAction(PlayerbotAI* botAI, std::string const name = "void reaver arcane orb move away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    bool IsInArcaneOrbRadius(const std::vector<Unit*>& arcaneOrbTargets, float safeDistance);
};

class HighAstromancerSolarianStackBotsAction : public MovementAction
{
public:
    HighAstromancerSolarianStackBotsAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian stack bots") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianMoveAwayFromGroupAction : public MovementAction
{
public:
    HighAstromancerSolarianMoveAwayFromGroupAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian move away from group") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianTargetSolariumPriestsAction : public AttackAction
{
public:
    HighAstromancerSolarianTargetSolariumPriestsAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian target solarium priests") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetSolariumPriests();
    std::vector<Player*> GetMeleeBots(Group* group);
    Unit* AssignSolariumPriestsToBots(const std::vector<Unit*>& solariumPriests, const std::vector<Player*>& meleeMembers);
};

class HighAstromancerSolarianTankVoidwalkerAction : public AttackAction
{
public:
    HighAstromancerSolarianTankVoidwalkerAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian tank voidwalker") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class HighAstromancerSolarianCastFearWardOnMainTankAction : public Action
{
public:
    HighAstromancerSolarianCastFearWardOnMainTankAction(PlayerbotAI* botAI, std::string const name = "high astromancer solarian cast fear ward on main tank") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
