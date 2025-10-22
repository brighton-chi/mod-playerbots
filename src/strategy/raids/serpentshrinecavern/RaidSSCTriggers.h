#ifndef _PLAYERBOT_RAIDSSCTRIGGERS_H
#define _PLAYERBOT_RAIDSSCTRIGGERS_H

#include "Trigger.h"

class HydrossTheUnstableBossInFrostPhaseTrigger : public Trigger
{
public:
    HydrossTheUnstableBossInFrostPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable boss in frost phase") {}
    bool IsActive() override;
};

class HydrossTheUnstableBossTransitioningToNaturePhaseTrigger : public Trigger
{
public:
    HydrossTheUnstableBossTransitioningToNaturePhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable boss transitioning to nature phase") {}
    bool IsActive() override;
};

class HydrossTheUnstableBossInNaturePhaseTrigger : public Trigger
{
public:
    HydrossTheUnstableBossInNaturePhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable boss in nature phase") {}
    bool IsActive() override;
};

class HydrossTheUnstableBossTransitioningToFrostPhaseTrigger : public Trigger
{
public:
    HydrossTheUnstableBossTransitioningToFrostPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable boss transitioning to frost phase") {}
    bool IsActive() override;
};

class HydrossTheUnstableWaitingForDPSTrigger : public Trigger
{
public:
    HydrossTheUnstableWaitingForDPSTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable waiting for dps") {}
    bool IsActive() override;
};

#endif
