#ifndef _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H

#include "RaidSSCTriggers.h"
#include "AiObjectContext.h"

class RaidSSCTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSSCTriggerContext()
    {
        creators["hydross the unstable boss in frost phase"] = &RaidSSCTriggerContext::hydross_the_unstable_boss_in_frost_phase;
        creators["hydross the unstable boss transitioning to nature phase"] = &RaidSSCTriggerContext::hydross_the_unstable_boss_transitioning_to_nature_phase;
        creators["hydross the unstable boss in nature phase"] = &RaidSSCTriggerContext::hydross_the_unstable_boss_in_nature_phase;
        creators["hydross the unstable boss transitioning to frost phase"] = &RaidSSCTriggerContext::hydross_the_unstable_boss_transitioning_to_frost_phase;
        creators["hydross the unstable waiting for dps"] = &RaidSSCTriggerContext::hydross_the_unstable_waiting_for_dps;
    }

private:
    static Trigger* hydross_the_unstable_boss_in_frost_phase(PlayerbotAI* botAI) { return new HydrossTheUnstableBossInFrostPhaseTrigger(botAI); }
    static Trigger* hydross_the_unstable_boss_transitioning_to_nature_phase(PlayerbotAI* botAI) { return new HydrossTheUnstableBossTransitioningToNaturePhaseTrigger(botAI); }
    static Trigger* hydross_the_unstable_boss_in_nature_phase(PlayerbotAI* botAI) { return new HydrossTheUnstableBossInNaturePhaseTrigger(botAI); }
    static Trigger* hydross_the_unstable_boss_transitioning_to_frost_phase(PlayerbotAI* botAI) { return new HydrossTheUnstableBossTransitioningToFrostPhaseTrigger(botAI); }
    static Trigger* hydross_the_unstable_waiting_for_dps(PlayerbotAI* botAI) { return new HydrossTheUnstableWaitingForDPSTrigger(botAI); }
};

#endif