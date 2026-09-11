/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCTRIGGERCONTEXT_H
#define PLAYERBOTS_SSCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SSCTriggers.h"

class RaidSscTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSscTriggerContext()
    {
        // General
        creators["ssc no encounter in progress"] =
            &RaidSscTriggerContext::ssc_no_encounter_in_progress;

        // Trash
        creators["underbog colossus in toxic pool"] =
            &RaidSscTriggerContext::underbog_colossus_in_toxic_pool;

        creators["greyheart tidecaller water elemental totem spawned"] =
            &RaidSscTriggerContext::greyheart_tidecaller_water_elemental_totem_spawned;

        // Hydross the Unstable <Duke of Currents>
        creators["hydross the unstable should be tanked by frost tank"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_be_tanked_by_frost_tank;

        creators["hydross the unstable should be tanked by nature tank"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_be_tanked_by_nature_tank;

        creators["hydross the unstable elementals spawned"] =
            &RaidSscTriggerContext::hydross_the_unstable_elementals_spawned;

        creators["hydross the unstable ranged should spread"] =
            &RaidSscTriggerContext::hydross_the_unstable_ranged_should_spread;

        creators["hydross the unstable tank needs aggro upon phase change"] =
            &RaidSscTriggerContext::hydross_the_unstable_tank_needs_aggro_upon_phase_change;

        creators["hydross the unstable aggro resets upon phase change"] =
            &RaidSscTriggerContext::hydross_the_unstable_aggro_resets_upon_phase_change;

        creators["hydross the unstable should manage phase timers"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_manage_phase_timers;

        // The Lurker Below
        creators["the lurker below spout is active"] =
            &RaidSscTriggerContext::the_lurker_below_spout_is_active;

        creators["the lurker below should be tanked"] =
            &RaidSscTriggerContext::the_lurker_below_should_be_tanked;

        creators["the lurker below ranged should spread"] =
            &RaidSscTriggerContext::the_lurker_below_ranged_should_spread;

        creators["the lurker below is submerged"] =
            &RaidSscTriggerContext::the_lurker_below_is_submerged;

        creators["the lurker below should manage spout timer"] =
            &RaidSscTriggerContext::the_lurker_below_should_manage_spout_timer;

        // Leotheras the Blind
        creators["leotheras the blind demon form should be tanked by warlock"] =
            &RaidSscTriggerContext::leotheras_the_blind_demon_form_should_be_tanked_by_warlock;

        creators["leotheras the blind only warlock should tank demon form"] =
            &RaidSscTriggerContext::leotheras_the_blind_only_warlock_should_tank_demon_form;

        creators["leotheras the blind ranged should spread"] =
            &RaidSscTriggerContext::leotheras_the_blind_ranged_should_spread;

        creators["leotheras the blind channeling whirlwind"] =
            &RaidSscTriggerContext::leotheras_the_blind_channeling_whirlwind;

        creators["leotheras the blind too many chaos blast stacks"] =
            &RaidSscTriggerContext::leotheras_the_blind_too_many_chaos_blast_stacks;

        creators["leotheras the blind inner demon has awakened"] =
            &RaidSscTriggerContext::leotheras_the_blind_inner_demon_has_awakened;

        creators["leotheras the blind in final phase"] =
            &RaidSscTriggerContext::leotheras_the_blind_in_final_phase;

        creators["leotheras the blind demon form tank needs aggro"] =
            &RaidSscTriggerContext::leotheras_the_blind_demon_form_tank_needs_aggro;

        creators["leotheras the blind should manage dps wait timers"] =
            &RaidSscTriggerContext::leotheras_the_blind_should_manage_dps_wait_timers;

        // Fathom-Lord Karathress
        creators["fathom-lord karathress should be tanked"] =
            &RaidSscTriggerContext::fathom_lord_karathress_should_be_tanked;

        creators["fathom-lord karathress caribdis should be tanked"] =
            &RaidSscTriggerContext::fathom_lord_karathress_caribdis_should_be_tanked;

        creators["fathom-lord karathress sharkkis should be tanked"] =
            &RaidSscTriggerContext::fathom_lord_karathress_sharkkis_should_be_tanked;

        creators["fathom-lord karathress tidalvess should be tanked"] =
            &RaidSscTriggerContext::fathom_lord_karathress_tidalvess_should_be_tanked;

        creators["fathom-lord karathress caribdis tank needs dedicated healer"] =
            &RaidSscTriggerContext::fathom_lord_karathress_caribdis_tank_needs_dedicated_healer;

        creators["fathom-lord karathress pulling bosses"] =
            &RaidSscTriggerContext::fathom_lord_karathress_pulling_bosses;

        creators["fathom-lord karathress determining kill order"] =
            &RaidSscTriggerContext::fathom_lord_karathress_determining_kill_order;

        creators["fathom-lord karathress should manage dps timer"] =
            &RaidSscTriggerContext::fathom_lord_karathress_should_manage_dps_timer;

        // Morogrim Tidewalker
        creators["morogrim tidewalker should be tanked"] =
            &RaidSscTriggerContext::morogrim_tidewalker_should_be_tanked;

        creators["morogrim tidewalker pulling boss"] =
            &RaidSscTriggerContext::morogrim_tidewalker_pulling_boss;

        creators["morogrim tidewalker in phase 2"] =
            &RaidSscTriggerContext::morogrim_tidewalker_in_phase_2;

        // Lady Vashj <Coilfang Matron>
        creators["lady vashj should be tanked"] =
            &RaidSscTriggerContext::lady_vashj_should_be_tanked;

        creators["lady vashj ranged should spread in phase 1"] =
            &RaidSscTriggerContext::lady_vashj_ranged_should_spread_in_phase_1;

        creators["lady vashj shaman should ground shock blast"] =
            &RaidSscTriggerContext::lady_vashj_shaman_should_ground_shock_blast;

        creators["lady vashj static charge on group member"] =
            &RaidSscTriggerContext::lady_vashj_static_charge_on_group_member;

        creators["lady vashj pulling boss in phase 1 and phase 3"] =
            &RaidSscTriggerContext::lady_vashj_pulling_boss_in_phase_1_and_phase_3;

        creators["lady vashj adds spawn in phase 2 and phase 3"] =
            &RaidSscTriggerContext::lady_vashj_adds_spawn_in_phase_2_and_phase_3;

        creators["lady vashj coilfang strider is approaching"] =
            &RaidSscTriggerContext::lady_vashj_coilfang_strider_is_approaching;

        creators["lady vashj tainted elemental cheat"] =
            &RaidSscTriggerContext::lady_vashj_tainted_elemental_cheat;

        creators["lady vashj tainted core was looted"] =
            &RaidSscTriggerContext::lady_vashj_tainted_core_was_looted;

        creators["lady vashj in phase 3"] =
            &RaidSscTriggerContext::lady_vashj_in_phase_3;

        creators["lady vashj entangle on melee"] =
            &RaidSscTriggerContext::lady_vashj_entangle_on_melee;
    }

private:
    // General
    static Trigger* ssc_no_encounter_in_progress(PlayerbotAI* botAI) {
        return new SscNoEncounterInProgressTrigger(botAI);
    }

    // Trash
    static Trigger* underbog_colossus_in_toxic_pool(PlayerbotAI* botAI) {
        return new UnderbogColossusInToxicPoolTrigger(botAI);
    }
    static Trigger* greyheart_tidecaller_water_elemental_totem_spawned(PlayerbotAI* botAI) {
        return new GreyheartTidecallerWaterElementalTotemSpawnedTrigger(botAI);
    }

    // Hydross the Unstable <Duke of Currents>
    static Trigger* hydross_the_unstable_should_be_tanked_by_frost_tank(PlayerbotAI* botAI) {
        return new HydrossTheUnstableShouldBeTankedByFrostTankTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_should_be_tanked_by_nature_tank(PlayerbotAI* botAI) {
        return new HydrossTheUnstableShouldBeTankedByNatureTankTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_elementals_spawned(PlayerbotAI* botAI) {
        return new HydrossTheUnstableElementalsSpawnedTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_ranged_should_spread(PlayerbotAI* botAI) {
        return new HydrossTheUnstableRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_tank_needs_aggro_upon_phase_change(PlayerbotAI* botAI) {
        return new HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_aggro_resets_upon_phase_change(PlayerbotAI* botAI) {
        return new HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_should_manage_phase_timers(PlayerbotAI* botAI) {
        return new HydrossTheUnstableShouldManagePhaseTimersTrigger(botAI);
    }

    // The Lurker Below
    static Trigger* the_lurker_below_spout_is_active(PlayerbotAI* botAI) {
        return new TheLurkerBelowSpoutIsActiveTrigger(botAI);
    }
    static Trigger* the_lurker_below_should_be_tanked(PlayerbotAI* botAI) {
        return new TheLurkerBelowShouldBeTankedTrigger(botAI);
    }
    static Trigger* the_lurker_below_ranged_should_spread(PlayerbotAI* botAI) {
        return new TheLurkerBelowRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* the_lurker_below_is_submerged(PlayerbotAI* botAI) {
        return new TheLurkerBelowIsSubmergedTrigger(botAI);
    }
    static Trigger* the_lurker_below_should_manage_spout_timer(PlayerbotAI* botAI) {
        return new TheLurkerBelowShouldManageSpoutTimerTrigger(botAI);
    }

    // Leotheras the Blind
    static Trigger* leotheras_the_blind_demon_form_should_be_tanked_by_warlock(PlayerbotAI* botAI) {
        return new LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_only_warlock_should_tank_demon_form(PlayerbotAI* botAI) {
        return new LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_ranged_should_spread(PlayerbotAI* botAI) {
        return new LeotherasTheBlindRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_channeling_whirlwind(PlayerbotAI* botAI) {
        return new LeotherasTheBlindChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_too_many_chaos_blast_stacks(PlayerbotAI* botAI) {
        return new LeotherasTheBlindTooManyChaosBlastStacksTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_inner_demon_has_awakened(PlayerbotAI* botAI) {
        return new LeotherasTheBlindInnerDemonHasAwakenedTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_in_final_phase(PlayerbotAI* botAI) {
        return new LeotherasTheBlindInFinalPhaseTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_demon_form_tank_needs_aggro(PlayerbotAI* botAI) {
        return new LeotherasTheBlindDemonFormTankNeedsAggro(botAI);
    }
    static Trigger* leotheras_the_blind_should_manage_dps_wait_timers(PlayerbotAI* botAI) {
        return new LeotherasTheBlindShouldManageDpsWaitTimersTrigger(botAI);
    }

    // Fathom-Lord Karathress
    static Trigger* fathom_lord_karathress_should_be_tanked(PlayerbotAI* botAI) {
        return new FathomLordKarathressShouldBeTankedTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_caribdis_should_be_tanked(PlayerbotAI* botAI) {
        return new FathomLordKarathressCaribdisShouldBeTankedTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_sharkkis_should_be_tanked(PlayerbotAI* botAI) {
        return new FathomLordKarathressSharkkisShouldBeTankedTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_tidalvess_should_be_tanked(PlayerbotAI* botAI) {
        return new FathomLordKarathressTidalvessShouldBeTankedTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_caribdis_tank_needs_dedicated_healer(PlayerbotAI* botAI) {
        return new FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_pulling_bosses(PlayerbotAI* botAI) {
        return new FathomLordKarathressPullingBossesTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_determining_kill_order(PlayerbotAI* botAI) {
        return new FathomLordKarathressDeterminingKillOrderTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_should_manage_dps_timer(PlayerbotAI* botAI) {
        return new FathomLordKarathressShouldManageDpsTimerTrigger(botAI);
    }

    // Morogrim Tidewalker
    static Trigger* morogrim_tidewalker_should_be_tanked(PlayerbotAI* botAI) {
        return new MorogrimTidewalkerShouldBeTankedTrigger(botAI);
    }
    static Trigger* morogrim_tidewalker_pulling_boss(PlayerbotAI* botAI) {
        return new MorogrimTidewalkerPullingBossTrigger(botAI);
    }
    static Trigger* morogrim_tidewalker_in_phase_2(PlayerbotAI* botAI) {
        return new MorogrimTidewalkerInPhase2Trigger(botAI);
    }

    // Lady Vashj <Coilfang Matron>
    static Trigger* lady_vashj_should_be_tanked(PlayerbotAI* botAI) {
        return new LadyVashjShouldBeTankedTrigger(botAI);
    }
    static Trigger* lady_vashj_ranged_should_spread_in_phase_1(PlayerbotAI* botAI) {
        return new LadyVashjRangedShouldSpreadInPhase1Trigger(botAI);
    }
    static Trigger* lady_vashj_shaman_should_ground_shock_blast(PlayerbotAI* botAI) {
        return new LadyVashjShamanShouldGroundShockBlastTrigger(botAI);
    }
    static Trigger* lady_vashj_static_charge_on_group_member(PlayerbotAI* botAI) {
        return new LadyVashjStaticChargeOnGroupMemberTrigger(botAI);
    }
    static Trigger* lady_vashj_pulling_boss_in_phase_1_and_phase_3(PlayerbotAI* botAI) {
        return new LadyVashjPullingBossInPhase1AndPhase3Trigger(botAI);
    }
    static Trigger* lady_vashj_adds_spawn_in_phase_2_and_phase_3(PlayerbotAI* botAI) {
        return new LadyVashjAddsSpawnInPhase2AndPhase3Trigger(botAI);
    }
    static Trigger* lady_vashj_coilfang_strider_is_approaching(PlayerbotAI* botAI) {
        return new LadyVashjCoilfangStriderIsApproachingTrigger(botAI);
    }
    static Trigger* lady_vashj_tainted_elemental_cheat(PlayerbotAI* botAI) {
        return new LadyVashjTaintedElementalCheatTrigger(botAI);
    }
    static Trigger* lady_vashj_tainted_core_was_looted(PlayerbotAI* botAI) {
        return new LadyVashjTaintedCoreWasLootedTrigger(botAI);
    }
    static Trigger* lady_vashj_in_phase_3(PlayerbotAI* botAI) {
        return new LadyVashjInPhase3Trigger(botAI);
    }
    static Trigger* lady_vashj_entangle_on_melee(PlayerbotAI* botAI) {
        return new LadyVashjEntangleOnMeleeTrigger(botAI);
    }
};

#endif
