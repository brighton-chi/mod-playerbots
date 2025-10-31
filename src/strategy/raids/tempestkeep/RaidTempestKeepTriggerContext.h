#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["alar need to log info to develop boss strategy"] = &RaidTempestKeepTriggerContext::alar_need_to_log_info_to_develop_boss_strategy;
        creators["alar pulling boss"] = &RaidTempestKeepTriggerContext::alar_pulling_boss;
        creators["alar tanks need to be at platforms to aggro boss"] = &RaidTempestKeepTriggerContext::alar_tanks_need_to_be_at_platforms_to_aggro_boss;
        creators["alar boss engaged by melee dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_melee_dps;
        creators["alar boss engaged by ranged dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_ranged_dps;
        creators["alar boss engaged by healer"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_healer;
        creators["alar need tank to pick up adds"] = &RaidTempestKeepTriggerContext::alar_need_tank_to_pick_up_adds;
        creators["alar incoming flame quills"] = &RaidTempestKeepTriggerContext::alar_incoming_flame_quills;
        creators["alar rising from the ashes"] = &RaidTempestKeepTriggerContext::alar_rising_from_the_ashes;
        creators["alar need to manage timers and trackers"] = &RaidTempestKeepTriggerContext::alar_need_to_manage_timers_and_trackers;
        creators["void reaver boss engaged by tank"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_tank;
        creators["void reaver boss engaged by ranged"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_ranged;
        creators["high astromancer solarian phase 1 and 2 movement"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_phase_1_and_2_movement;
        creators["high astromancer solarian bot has wrath of the astromancer"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;
        creators["high astromancer solarian solarium priests spawned"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;
        creators["high astromancer solarian transformed into voidwalker"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_transformed_into_voidwalker;
        creators["high astromancer solarian can cast fear ward on main tank"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_can_cast_fear_ward_on_main_tank;
    }

private:
    static Trigger* alar_need_to_log_info_to_develop_boss_strategy(PlayerbotAI* botAI) { return new AlarNeedToLogInfoToDevelopBossStrategyTrigger(botAI); }
    static Trigger* alar_pulling_boss(PlayerbotAI* botAI) { return new AlarPullingBossTrigger(botAI); }
    static Trigger* alar_tanks_need_to_be_at_platforms_to_aggro_boss(PlayerbotAI* botAI) { return new AlarTanksNeedToBeAtPlatformsToAggroBossTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_melee_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByMeleeDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_ranged_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByRangedDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_healer(PlayerbotAI* botAI) { return new AlarBossEngagedByHealerTrigger(botAI); }
    static Trigger* alar_need_tank_to_pick_up_adds(PlayerbotAI* botAI) { return new AlarNeedTankToPickUpAddsTrigger(botAI); }
    static Trigger* alar_incoming_flame_quills(PlayerbotAI* botAI) { return new AlarIncomingFlameQuillsTrigger(botAI); }
    static Trigger* alar_rising_from_the_ashes(PlayerbotAI* botAI) { return new AlarRisingFromTheAshesTrigger(botAI); }
    static Trigger* alar_need_to_manage_timers_and_trackers(PlayerbotAI* botAI) { return new AlarNeedToManageTimersAndTrackersTrigger(botAI); }
    static Trigger* void_reaver_boss_engaged_by_tank(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByTankTrigger(botAI); }
    static Trigger* void_reaver_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByRangedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_phase_1_and_2_movement(PlayerbotAI* botAI) { return new HighAstromancerSolarianPhase1And2MovementTrigger(botAI); }
    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(PlayerbotAI* botAI) { return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_solarium_priests_spawned(PlayerbotAI* botAI) { return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_transformed_into_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTransformedIntoVoidwalkerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_can_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCanCastFearWardOnMainTankTrigger(botAI); }
};

#endif
