#ifndef _PLAYERBOT_RAIDZULAMANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDZULAMANTRIGGERCONTEXT_H

#include "RaidZulAmanTriggers.h"
#include "AiObjectContext.h"

class RaidZulAmanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZulAmanTriggerContext()
    {
        creators["akilzon boss casts static disruption"] = &RaidZulAmanTriggerContext::akilzon_boss_casts_static_disruption;
    }

private:
    static Trigger* akilzon_boss_casts_static_disruption(PlayerbotAI* botAI) { return new AkilzonBossCastsStaticDisruptionTrigger(botAI); }
};

#endif