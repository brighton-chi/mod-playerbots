#ifndef _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H

#include "RaidSSCTriggers.h"
#include "AiObjectContext.h"

class RaidSSCTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSSCTriggerContext()
    {
        // creators["nightbane manage timers and trackers"] = &RaidSSCTriggerContext::nightbane_manage_timers_and_trackers;
    }

private:
    // static Trigger* nightbane_manage_timers_and_trackers(PlayerbotAI* botAI) { return new NightbaneManageTimersAndTrackersTrigger(botAI); }
};

#endif