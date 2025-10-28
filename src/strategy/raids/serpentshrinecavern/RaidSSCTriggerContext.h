#ifndef _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSSCTRIGGERCONTEXT_H

#include "RaidSSCTriggers.h"
#include "AiObjectContext.h"

class RaidSSCTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSSCTriggerContext()
    {
        creators["greyheart tidecaller water elemental totem spawned"] = &RaidSSCTriggerContext::greyheart_tidecaller_water_elemental_totem_spawned;
        creators["rancid mushroom spawned"] = &RaidSSCTriggerContext::rancid_mushroom_spawned;

        creators["hydross the unstable bot is frost tank"] = &RaidSSCTriggerContext::hydross_the_unstable_bot_is_frost_tank;
        creators["hydross the unstable bot is nature tank"] = &RaidSSCTriggerContext::hydross_the_unstable_bot_is_nature_tank;
        creators["hydross the unstable elementals spawned"] = &RaidSSCTriggerContext::hydross_the_unstable_elementals_spawned;
        creators["hydross the unstable danger from water tombs"] = &RaidSSCTriggerContext::hydross_the_unstable_danger_from_water_tombs;
        creators["hydross the unstable tank needs aggro upon phase change"] = &RaidSSCTriggerContext::hydross_the_unstable_tank_needs_aggro_upon_phase_change;
        creators["hydross the unstable need to transition before fourth mark"] = &RaidSSCTriggerContext::hydross_the_unstable_need_to_transition_before_fourth_mark;
        creators["hydross the unstable need to manage phase change aggro reset"] = &RaidSSCTriggerContext::hydross_the_unstable_need_to_manage_phase_change_aggro_reset;

        creators["the lurker below boss is active for main tank"] = &RaidSSCTriggerContext::the_lurker_below_boss_is_active_for_main_tank;
        creators["the lurker below boss is active for other melee"] = &RaidSSCTriggerContext::the_lurker_below_boss_is_active_for_other_melee;
        creators["the lurker below spout is active for melee"] = &RaidSSCTriggerContext::the_lurker_below_spout_is_active_for_melee;
        creators["the lurker below boss is active for ranged dps"] = &RaidSSCTriggerContext::the_lurker_below_boss_is_active_for_ranged_dps;
        creators["the lurker below boss is active for healer"] = &RaidSSCTriggerContext::the_lurker_below_boss_is_active_for_healer;
        creators["the lurker below need to prepare timer for spout"] = &RaidSSCTriggerContext::the_lurker_below_need_to_prepare_timer_for_spout;

        creators["leotheras the blind boss is inactive"] = &RaidSSCTriggerContext::leotheras_the_blind_boss_is_inactive;
        creators["leotheras the blind engaged by demon form tank"] = &RaidSSCTriggerContext::leotheras_the_blind_engaged_by_demon_form_tank;
        creators["leotheras the blind boss engaged by ranged"] = &RaidSSCTriggerContext::leotheras_the_blind_boss_engaged_by_ranged;
        creators["leotheras the blind boss channeling whirlwind"] = &RaidSSCTriggerContext::leotheras_the_blind_boss_channeling_whirlwind;
        // creators["leotheras the blind demon form engaged by melee"] = &RaidSSCTriggerContext::leotheras_the_blind_demon_form_engaged_by_melee;
        creators["leotheras the blind inner demon has taken form"] = &RaidSSCTriggerContext::leotheras_the_blind_inner_demon_has_taken_form;
        creators["leotheras the blind entered final phase"] = &RaidSSCTriggerContext::leotheras_the_blind_entered_final_phase;
        creators["leotheras the blind demon form tank needs aggro"] = &RaidSSCTriggerContext::leotheras_the_blind_demon_form_tank_needs_aggro;
        creators["leotheras the blind need to manage timers and trackers"] = &RaidSSCTriggerContext::leotheras_the_blind_need_to_manage_timers_and_trackers;

        creators["fathom-lord karathress boss engaged by main tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_boss_engaged_by_main_tank;
        creators["fathom-lord karathress sharkkis engaged by first assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_sharkkis_engaged_by_first_assist_tank;
        creators["fathom-lord karathress tidalvess engaged by second assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_tidalvess_engaged_by_second_assist_tank;
        creators["fathom-lord karathress caribdis engaged by third assist tank"] = &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_engaged_by_third_assist_tank;
        creators["fathom-lord karathress pulling bosses"] = &RaidSSCTriggerContext::fathom_lord_karathress_pulling_bosses;
        creators["fathom-lord karathress determining kill order"] = &RaidSSCTriggerContext::fathom_lord_karathress_determining_kill_order;
        creators["fathom-lord karathress caribdis tank needs dedicated healer"] = &RaidSSCTriggerContext::fathom_lord_karathress_caribdis_tank_needs_dedicated_healer;
        creators["fathom-lord karathress tanks need to establish aggro"] = &RaidSSCTriggerContext::fathom_lord_karathress_tanks_need_to_establish_aggro;

        creators["morogrim tidewalker boss engaged by main tank"] = &RaidSSCTriggerContext::morogrim_tidewalker_boss_engaged_by_main_tank;
        creators["morogrim tidewalker pulling boss"] = &RaidSSCTriggerContext::morogrim_tidewalker_pulling_boss;
        creators["morogrim tidewalker water globules are incoming"] = &RaidSSCTriggerContext::morogrim_tidewalker_water_globules_are_incoming;
        creators["morogrim tidewalker encounter reset"] = &RaidSSCTriggerContext::morogrim_tidewalker_encounter_reset;
    }

private:
    static Trigger* greyheart_tidecaller_water_elemental_totem_spawned(PlayerbotAI* botAI) { return new GreyheartTidecallerWaterElementalTotemSpawnedTrigger(botAI); }
    static Trigger* rancid_mushroom_spawned(PlayerbotAI* botAI) { return new RancidMushroomSpawnedTrigger(botAI); }

    static Trigger* hydross_the_unstable_bot_is_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsFrostTankTrigger(botAI); }
    static Trigger* hydross_the_unstable_bot_is_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableBotIsNatureTankTrigger(botAI); }
    static Trigger* hydross_the_unstable_elementals_spawned(PlayerbotAI* botAI) { return new HydrossTheUnstableElementalsSpawnedTrigger(botAI); }
    static Trigger* hydross_the_unstable_danger_from_water_tombs(PlayerbotAI* botAI) { return new HydrossTheUnstableDangerFromWaterTombsTrigger(botAI); }
    static Trigger* hydross_the_unstable_tank_needs_aggro_upon_phase_change(PlayerbotAI* botAI) { return new HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(botAI); }
    static Trigger* hydross_the_unstable_need_to_transition_before_fourth_mark(PlayerbotAI* botAI) { return new HydrossTheUnstableNeedToTransitionBeforeFourthMarkTrigger(botAI); }
    static Trigger* hydross_the_unstable_need_to_manage_phase_change_aggro_reset(PlayerbotAI* botAI) { return new HydrossTheUnstableNeedToManagePhaseChangeAggroResetTrigger(botAI); }

    static Trigger* the_lurker_below_boss_is_active_for_main_tank(PlayerbotAI* botAI) { return new TheLurkerBelowBossIsActiveForMainTankTrigger(botAI); }
    static Trigger* the_lurker_below_boss_is_active_for_other_melee(PlayerbotAI* botAI) { return new TheLurkerBelowBossIsActiveForOtherMeleeTrigger(botAI); }
    static Trigger* the_lurker_below_spout_is_active_for_melee(PlayerbotAI* botAI) { return new TheLurkerBelowSpoutIsActiveForMeleeTrigger(botAI); }
    static Trigger* the_lurker_below_boss_is_active_for_ranged_dps(PlayerbotAI* botAI) { return new TheLurkerBelowBossIsActiveForRangedDpsTrigger(botAI); }
    static Trigger* the_lurker_below_boss_is_active_for_healer(PlayerbotAI* botAI) { return new TheLurkerBelowBossIsActiveForHealerTrigger(botAI); }
    static Trigger* the_lurker_below_need_to_prepare_timer_for_spout(PlayerbotAI* botAI) { return new TheLurkerBelowNeedToPrepareTimerForSpoutTrigger(botAI); }

    static Trigger* leotheras_the_blind_boss_is_inactive(PlayerbotAI* botAI) { return new LeotherasTheBlindBossIsInactiveTrigger(botAI); }
    static Trigger* leotheras_the_blind_engaged_by_demon_form_tank(PlayerbotAI* botAI) { return new LeotherasTheBlindEngagedByDemonFormTankTrigger(botAI); }
    static Trigger* leotheras_the_blind_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new LeotherasTheBlindBossEngagedByRangedTrigger(botAI); }
    static Trigger* leotheras_the_blind_boss_channeling_whirlwind(PlayerbotAI* botAI) { return new LeotherasTheBlindBossChannelingWhirlwindTrigger(botAI); }
    // static Trigger* leotheras_the_blind_demon_form_engaged_by_melee(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormEngagedByMeleeTrigger(botAI); }
    static Trigger* leotheras_the_blind_inner_demon_has_taken_form(PlayerbotAI* botAI) { return new LeotherasTheBlindInnerDemonHasTakenForm(botAI); }
    static Trigger* leotheras_the_blind_entered_final_phase(PlayerbotAI* botAI) { return new LeotherasTheBlindEnteredFinalPhaseTrigger(botAI); }
    static Trigger* leotheras_the_blind_demon_form_tank_needs_aggro(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormTankNeedsAggro(botAI); }
    static Trigger* leotheras_the_blind_need_to_manage_timers_and_trackers(PlayerbotAI* botAI) { return new LeotherasTheBlindNeedToManageTimersAndTrackersTrigger(botAI); }

    static Trigger* fathom_lord_karathress_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new FathomLordKarathressBossEngagedByMainTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_sharkkis_engaged_by_first_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_tidalvess_engaged_by_second_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_caribdis_engaged_by_third_assist_tank(PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger(botAI); }
    static Trigger* fathom_lord_karathress_pulling_bosses(PlayerbotAI* botAI) { return new FathomLordKarathressPullingBossesTrigger(botAI); }
    static Trigger* fathom_lord_karathress_determining_kill_order(PlayerbotAI* botAI) { return new FathomLordKarathressDeterminingKillOrderTrigger(botAI); }
    static Trigger* fathom_lord_karathress_caribdis_tank_needs_dedicated_healer(PlayerbotAI* botAI) { return new FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(botAI); }
    static Trigger* fathom_lord_karathress_tanks_need_to_establish_aggro(PlayerbotAI* botAI) { return new FathomLordKarathressTanksNeedToEstablishAggroTrigger(botAI); }

    static Trigger* morogrim_tidewalker_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerBossEngagedByMainTankTrigger(botAI); }
    static Trigger* morogrim_tidewalker_pulling_boss(PlayerbotAI* botAI) { return new MorogrimTidewalkerPullingBossTrigger(botAI); }
    static Trigger* morogrim_tidewalker_water_globules_are_incoming(PlayerbotAI* botAI) { return new MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(botAI); }
    static Trigger* morogrim_tidewalker_encounter_reset(PlayerbotAI* botAI) { return new MorogrimTidewalkerEncounterResetTrigger(botAI); }
};

#endif