#ifndef _PLAYERBOT_RAIDZGTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDZGTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "RaidZgTriggers.h"

class RaidZgTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZgTriggerContext()
    {
    creators["mandokir threatening gaze cast"] = &RaidZgTriggerContext::mandokir_threatening_gaze_cast;
    creators["mandokir threatening gaze end"] = &RaidZgTriggerContext::mandokir_threatening_gaze_end;
    }

private:
    static Trigger* mandokir_threatening_gaze_cast(PlayerbotAI* botAI)
    {
        return new MandokirThreateningGazeCastTrigger(botAI);
    }

    static Trigger* mandokir_threatening_gaze_end(PlayerbotAI* botAI)
    {
        return new MandokirThreateningGazeEndTrigger(botAI);
    }
};

#endif
