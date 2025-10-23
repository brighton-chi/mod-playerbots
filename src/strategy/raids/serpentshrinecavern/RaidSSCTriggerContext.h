#ifndef _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H

#include "RaidSSCTriggers.h"
#include "AiObjectContext.h"

class RaidSSCTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSSCTriggerContext()
    {
        creators["hydross the unstable bot is frost tank"] = &RaidSSCTriggerContext::hydross_the_unstable_bot_is_frost_tank;
        creators["hydross the unstable bot is nature tank"] = &RaidSSCTriggerContext::hydross_the_unstable_bot_is_nature_tank;
        creators["hydross the unstable waiting for dps"] = &RaidSSCTriggerContext::hydross_the_unstable_waiting_for_dps;
        creators["hydross the unstable elementals spawned"] = &RaidSSCTriggerContext::hydross_the_unstable_elementals_spawned;
        creators["hydross the unstable danger from water tombs"] = &RaidSSCTriggerContext::hydross_the_unstable_danger_from_water_tombs;
        creators["morogrim tidewalker boss engaged by main tank"] = &RaidSSCTriggerContext::morogrim_tidewalker_boss_engaged_by_main_tank;
        creators["morogrim tidewalker pulling boss"] = &RaidSSCTriggerContext::morogrim_tidewalker_pulling_boss;
        creators["morogrim tidewalker water globules are incoming"] = &RaidSSCTriggerContext::morogrim_tidewalker_water_globules_are_incoming;
    }

private:
    static Trigger* hydross_the_unstable_bot_is_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsFrostTankTrigger(botAI); }
    static Trigger* hydross_the_unstable_bot_is_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsNatureTankTrigger(botAI); }
    static Trigger* hydross_the_unstable_waiting_for_dps(PlayerbotAI* botAI) { return new HydrossTheUnstableWaitingForDPSTrigger(botAI); }
    static Trigger* hydross_the_unstable_elementals_spawned(PlayerbotAI* botAI) { return new HydrossTheUnstableElementalsSpawnedTrigger(botAI); }
    static Trigger* hydross_the_unstable_danger_from_water_tombs(PlayerbotAI* botAI) { return new HydrossTheUnstableDangerFromWaterTombsTrigger(botAI); }
    static Trigger* morogrim_tidewalker_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerBossEngagedByMainTankTrigger(botAI); }
    static Trigger* morogrim_tidewalker_pulling_boss(PlayerbotAI* botAI) { return new MorogrimTidewalkerPullingBossTrigger(botAI); }
    static Trigger* morogrim_tidewalker_water_globules_are_incoming(PlayerbotAI* botAI) { return new MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(botAI); }
};

#endif