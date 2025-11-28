#ifndef _PLAYERBOT_RAIDZULAMANTRIGGERS_H
#define _PLAYERBOT_RAIDZULAMANTRIGGERS_H

#include "Trigger.h"

class AkilzonBossCastsStaticDisruptionTrigger : public Trigger
{
public:
    AkilzonBossCastsStaticDisruptionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "akilzon boss casts static disruption") {}
    bool IsActive() override;
};

class AkilzonElectricalStormHasFormed : public Trigger
{
public:
    AkilzonElectricalStormHasFormed(PlayerbotAI* botAI) : Trigger(botAI, "akilzon electrical storm has formed") {}
    bool IsActive() override;
};

#endif
