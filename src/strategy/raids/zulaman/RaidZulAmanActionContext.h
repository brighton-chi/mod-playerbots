#ifndef _PLAYERBOT_RAIDZULAMANACTIONCONTEXT_H
#define _PLAYERBOT_RAIDZULAMANACTIONCONTEXT_H

#include "RaidZulAmanActions.h"
#include "NamedObjectContext.h"

class RaidZulAmanActionContext : public NamedObjectContext<Action>
{
public:
    RaidZulAmanActionContext()
    {
        creators["akilzon spread ranged"] = &RaidZulAmanActionContext::akilzon_spread_ranged;
        creators["akilzon move to eye of the storm"] = &RaidZulAmanActionContext::akilzon_move_to_eye_of_the_storm;
    }

private:
    static Action* akilzon_spread_ranged(PlayerbotAI* botAI) { return new AkilzonSpreadRangedAction(botAI); }
    static Action* akilzon_move_to_eye_of_the_storm(PlayerbotAI* botAI) { return new AkilzonMoveToEyeOfTheStormAction(botAI); }
};

#endif
