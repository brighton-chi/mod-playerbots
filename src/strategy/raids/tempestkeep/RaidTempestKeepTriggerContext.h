#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["crimson hand centurion casts arcane volley"] = &RaidTempestKeepTriggerContext::crimson_hand_centurion_casts_arcane_volley;
        creators["alar pulling boss"] = &RaidTempestKeepTriggerContext::alar_pulling_boss;
        creators["alar engaged by tanks in phase 1"] = &RaidTempestKeepTriggerContext::alar_engaged_by_tanks_in_phase_1;
        creators["alar boss engaged by melee dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_melee_dps;
        creators["alar boss engaged by ranged dps"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_ranged_dps;
        creators["alar boss engaged by healer"] = &RaidTempestKeepTriggerContext::alar_boss_engaged_by_healer;
        creators["alar embers of alar spawned"] = &RaidTempestKeepTriggerContext::alar_embers_of_alar_spawned;
        creators["alar incoming flame quills"] = &RaidTempestKeepTriggerContext::alar_incoming_flame_quills;
        creators["alar rising from the ashes"] = &RaidTempestKeepTriggerContext::alar_rising_from_the_ashes;
        creators["alar engaged by tanks in phase 2"] = &RaidTempestKeepTriggerContext::alar_engaged_by_tanks_in_phase_2;
        creators["alar preparing to dive bomb"] = &RaidTempestKeepTriggerContext::alar_preparing_to_dive_bomb;
        creators["alar need to manage timers and trackers"] = &RaidTempestKeepTriggerContext::alar_need_to_manage_timers_and_trackers;
        creators["void reaver boss engaged by tank"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_tank;
        creators["void reaver boss engaged by ranged"] = &RaidTempestKeepTriggerContext::void_reaver_boss_engaged_by_ranged;
        creators["high astromancer solarian phase 1 and 2 movement"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_phase_1_and_2_movement;
        creators["high astromancer solarian bot has wrath of the astromancer"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_bot_has_wrath_of_the_astromancer;
        creators["high astromancer solarian solarium priests spawned"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_solarium_priests_spawned;
        creators["high astromancer solarian transformed into voidwalker"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_transformed_into_voidwalker;
        creators["high astromancer solarian boss casts psychic scream"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_boss_casts_psychic_scream;
        creators["kaelthas sunstrider need to test strategies on boss"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_need_to_test_strategies_on_boss;
        creators["kaelthas sunstrider thaladred is fixated on bot"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_thaladred_is_fixated_on_bot;
        creators["kaelthas sunstrider sanguinar engaged by main tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_engaged_by_main_tank;
        creators["kaelthas sunstrider sanguinar casts bellowing roar"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_sanguinar_casts_bellowing_roar;
        creators["kaelthas sunstrider capernian engaged by warlock tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_capernian_engaged_by_warlock_tank;
        creators["kaelthas sunstrider telonicus engaged by first assist tank"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank;
        creators["kaelthas sunstrider cheat to test"] = &RaidTempestKeepTriggerContext::kaelthas_sunstrider_cheat_to_test;
    }

private:
    static Trigger* crimson_hand_centurion_casts_arcane_volley(PlayerbotAI* botAI) { return new CrimsonHandCenturionCastsArcaneVolleyTrigger(botAI); }
    static Trigger* alar_pulling_boss(PlayerbotAI* botAI) { return new AlarPullingBossTrigger(botAI); }
    static Trigger* alar_engaged_by_tanks_in_phase_1(PlayerbotAI* botAI) { return new AlarEngagedByTanksInPhase1Trigger(botAI); }
    static Trigger* alar_boss_engaged_by_melee_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByMeleeDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_ranged_dps(PlayerbotAI* botAI) { return new AlarBossEngagedByRangedDpsTrigger(botAI); }
    static Trigger* alar_boss_engaged_by_healer(PlayerbotAI* botAI) { return new AlarBossEngagedByHealerTrigger(botAI); }
    static Trigger* alar_embers_of_alar_spawned(PlayerbotAI* botAI) { return new AlarEmbersOfAlarSpawnedTrigger(botAI); }
    static Trigger* alar_incoming_flame_quills(PlayerbotAI* botAI) { return new AlarIncomingFlameQuillsTrigger(botAI); }
    static Trigger* alar_rising_from_the_ashes(PlayerbotAI* botAI) { return new AlarRisingFromTheAshesTrigger(botAI); }
    static Trigger* alar_engaged_by_tanks_in_phase_2(PlayerbotAI* botAI) { return new AlarEngagedByTanksInPhase2Trigger(botAI); }
    static Trigger* alar_preparing_to_dive_bomb(PlayerbotAI* botAI) { return new AlarPreparingToDiveBombTrigger(botAI); }
    static Trigger* alar_need_to_manage_timers_and_trackers(PlayerbotAI* botAI) { return new AlarNeedToManageTimersAndTrackersTrigger(botAI); }
    static Trigger* void_reaver_boss_engaged_by_tank(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByTankTrigger(botAI); }
    static Trigger* void_reaver_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new VoidReaverBossEngagedByRangedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_phase_1_and_2_movement(PlayerbotAI* botAI) { return new HighAstromancerSolarianPhase1And2MovementTrigger(botAI); }
    static Trigger* high_astromancer_solarian_bot_has_wrath_of_the_astromancer(PlayerbotAI* botAI) { return new HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_solarium_priests_spawned(PlayerbotAI* botAI) { return new HighAstromancerSolarianSolariumPriestsSpawnedTrigger(botAI); }
    static Trigger* high_astromancer_solarian_transformed_into_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTransformedIntoVoidwalkerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_boss_casts_psychic_scream(PlayerbotAI* botAI) { return new HighAstromancerSolarianBossCastsPsychicScreamTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_need_to_test_strategies_on_boss(PlayerbotAI* botAI) { return new KaelthasSunstriderNeedToTestStrategiesOnBossTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_thaladred_is_fixated_on_bot(PlayerbotAI* botAI) { return new KaelthasSunstriderThaladredIsFixatedOnBotTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_sanguinar_engaged_by_main_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarEngagedByMainTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_sanguinar_casts_bellowing_roar(PlayerbotAI* botAI) { return new KaelthasSunstriderSanguinarCastsBellowingRoarTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_capernian_engaged_by_warlock_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderCapernianEngagedByWarlockTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_telonicus_engaged_by_first_assist_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger(botAI); }
    static Trigger* kaelthas_sunstrider_cheat_to_test(PlayerbotAI* botAI) { return new KaelthasSunstriderCheatToTestTrigger(botAI); }
};

#endif
