#ifndef _PLAYERBOT_RAIDMAGSLAIRTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDMAGSLAIRTRIGGERCONTEXT_H

#include "RaidMagsLairTriggers.h"
#include "AiObjectContext.h"

class RaidMagsLairTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMagsLairTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["magtheridon"] = &RaidMagsLairTriggerContext::magtheridon;
    }

private:
    static Trigger* magtheridon(PlayerbotAI* botAI) { return new MagtheridonTrigger(botAI); }
};

#endif
