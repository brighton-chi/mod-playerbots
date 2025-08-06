#ifndef _PLAYERBOT_RAIDZGACTIONCONTEXT_H
#define _PLAYERBOT_RAIDZGACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidZgActions.h"

class RaidZgActionContext : public NamedObjectContext<Action>
{
public:
    RaidZgActionContext()
    {
        creators["mandokir threatening gaze freeze"] = &RaidZgActionContext::mandokir_threatening_gaze_freeze;
        creators["mandokir threatening gaze resume"] = &RaidZgActionContext::mandokir_threatening_gaze_resume;
    }

private:
    static Action* mandokir_threatening_gaze_freeze(PlayerbotAI* botAI)
    {
        return new MandokirThreateningGazeFreezeAction(botAI);
    }

    static Action* mandokir_threatening_gaze_resume(PlayerbotAI* botAI)
    {
        return new MandokirThreateningGazeResumeAction(botAI);
    }
};

#endif
