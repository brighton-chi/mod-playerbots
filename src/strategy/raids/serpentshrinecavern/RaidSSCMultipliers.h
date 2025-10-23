#ifndef _PLAYERBOT_RAIDSSCMULTIPLIERS_H
#define _PLAYERBOT_RAIDSSCMULTIPLIERS_H

#include "Multiplier.h"

class HydrossTheUnstableDisableTankAssistMultiplier : public Multiplier
{
public:
    HydrossTheUnstableDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable disable tank assist") {}
    virtual float GetValue(Action* action);
};

/* class HydrossTheUnstableWaitForDPSMultiplier : public Multiplier
{
public:
    HydrossTheUnstableWaitForDPSMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable wait for dps") {}
    virtual float GetValue(Action* action);
}; */

#endif
