#ifndef _PLAYERBOT_RAIDSSCACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDSSCACTIONS_CONTEXT_H

#include "RaidSSCActions.h"
#include "NamedObjectContext.h"

class RaidSSCActionContext : public NamedObjectContext<Action>
{
public:
    RaidSSCActionContext()
    {
        // creators["attumen the huntsman main tank"] = &RaidKarazhanActionContext::attumen_the_huntsman_main_tank;
    }

private:
    // static Action* attumen_the_huntsman_main_tank(PlayerbotAI* botAI) { return new AttumenTheHuntsmanMainTankAction(botAI); }
};

#endif
