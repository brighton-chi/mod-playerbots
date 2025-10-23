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
        creators["the lurker below spout is active"] = &RaidSSCTriggerContext::the_lurker_below_spout_is_active;
        creators["leotheras the blind human form engaged by main tank"] = &RaidSSCTriggerContext::leotheras_the_blind_human_form_engaged_by_main_tank;
        creators["leotheras the blind demon form engaged by tank"] = &RaidSSCTriggerContext::leotheras_the_blind_demon_form_engaged_by_tank;
        creators["leotheras the blind determining kill order"] = &RaidSSCTriggerContext::leotheras_the_blind_determining_kill_order;
        creators["leotheras the blind boss channeling whirlwind"] = &RaidSSCTriggerContext::leotheras_the_blind_boss_channeling_whirlwind;
        creators["leotheras the blind waiting for dps"] = &RaidSSCTriggerContext::leotheras_the_blind_waiting_for_dps;
        creators["fathom-lord karathress boss engaged by main tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_boss_engaged_by_main_tank;
        creators["fathom-lord karathress sharkkis engaged by first assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_sharkkis_engaged_by_first_assist_tank;
        creators["fathom-lord karathress tidalvess engaged by second assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_tidalvess_engaged_by_second_assist_tank;
        creators["fathom-lord karathress caribdis engaged by third assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_engaged_by_third_assist_tank;
        creators["fathom-lord karathress caribdis tank needs dedicated healer"] = &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_tank_needs_dedicated_healer;
        creators["fathom-lord karathress pulling bosses"] = &RaidSSCTriggerContext::fathom_lord_karathress_pulling_bosses;
        creators["fathom-lord karathress determining melee dps kill order"] = &RaidSSCTriggerContext::fathom_lord_karathress_determining_melee_dps_kill_order;
        creators["fathom-lord karathress determining ranged dps kill order"] = &RaidSSCTriggerContext::fathom_lord_karathress_determining_ranged_dps_kill_order;
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
    static Trigger* the_lurker_below_spout_is_active(PlayerbotAI* botAI) { return new TheLurkerBelowSpoutIsActiveTrigger(botAI); }
    static Trigger* leotheras_the_blind_human_form_engaged_by_main_tank(PlayerbotAI* botAI) { return new LeotherasTheBlindHumanFormEngagedByMainTankTrigger(botAI); }
    static Trigger* leotheras_the_blind_demon_form_engaged_by_tank(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormEngagedByTankTrigger(botAI); }
    static Trigger* leotheras_the_blind_determining_kill_order(PlayerbotAI* botAI) { return new LeotherasTheBlindDeterminingKillOrderTrigger(botAI); }
    static Trigger* leotheras_the_blind_boss_channeling_whirlwind(PlayerbotAI* botAI) { return new LeotherasTheBlindBossChannelingWhirlwindTrigger(botAI); }
    static Trigger* leotheras_the_blind_waiting_for_dps(PlayerbotAI* botAI) { return new LeotherasTheBlindWaitingForDPSTrigger(botAI); }
    static Trigger* fathom_lord_karathress_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new FathomLordKarathressBossEngagedByMainTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_sharkkis_engaged_by_first_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_tidalvess_engaged_by_second_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_caribdis_engaged_by_third_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_caribdis_tank_needs_dedicated_healer(PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(botAI); }
    static Trigger* fathom_lord_karathress_pulling_bosses(PlayerbotAI* botAI) { return new FathomLordKarathressPullingBossesTrigger(botAI); }
    static Trigger* fathom_lord_karathress_determining_melee_dps_kill_order(PlayerbotAI* botAI) { return new FathomLordKarathressDeterminingMeleeDPSKillOrderTrigger(botAI); }
    static Trigger* fathom_lord_karathress_determining_ranged_dps_kill_order(PlayerbotAI* botAI) { return new FathomLordKarathressDeterminingRangedDPSKillOrderTrigger(botAI); }
    static Trigger* morogrim_tidewalker_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerBossEngagedByMainTankTrigger(botAI); }
    static Trigger* morogrim_tidewalker_pulling_boss(PlayerbotAI* botAI) { return new MorogrimTidewalkerPullingBossTrigger(botAI); }
    static Trigger* morogrim_tidewalker_water_globules_are_incoming(PlayerbotAI* botAI) { return new MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(botAI); }
};

#endif