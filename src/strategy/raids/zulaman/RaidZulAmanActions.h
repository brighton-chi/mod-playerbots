#ifndef _PLAYERBOT_RAIDZULAMANACTIONS_H
#define _PLAYERBOT_RAIDZULAMANACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class AkilzonSpreadRangedAction : public MovementAction
{
public:
    AkilzonSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "akilzon spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class AkilzonMoveToEyeOfTheStormAction : public MovementAction
{
public:
    AkilzonMoveToEyeOfTheStormAction(PlayerbotAI* botAI, std::string const name = "akilzon move to eye of the storm") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
