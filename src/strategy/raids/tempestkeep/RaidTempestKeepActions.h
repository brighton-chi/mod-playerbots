#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class VoidReaverPositionBossAction : public AttackAction
{
public:
    VoidReaverPositionBossAction(PlayerbotAI* botAI, std::string const name = "void reaver position boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class VoidReaverSpreadRangedAction : public MovementAction
{
public:
    VoidReaverSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "void reaver spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    std::vector<Unit*> GetAllArcaneOrbTargets(PlayerbotAI* botAI, Player* bot);
    bool IsInArcaneOrbRadius(Player* bot, const std::vector<Unit*>& arcaneOrbTargets, float safeDistance = 25.0f)
};

class VoidReaverArcaneOrbMoveAwayAction : public MovementAction
{
public:
    VoidReaverArcaneOrbMoveAwayAction(PlayerbotAI* botAI, std::string const name = "void reaver arcane orb move away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
