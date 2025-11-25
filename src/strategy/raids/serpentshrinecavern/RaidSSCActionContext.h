#ifndef _PLAYERBOT_RAIDSSCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDSSCACTIONCONTEXT_H

#include "RaidSSCActions.h"
#include "NamedObjectContext.h"

class RaidSSCActionContext : public NamedObjectContext<Action>
{
public:
    RaidSSCActionContext()
    {
        creators["underbog colossus escape toxic pool"] = &RaidSSCActionContext::underbog_colossus_escape_toxic_pool;
        creators["greyheart tidecaller mark water elemental totem"] = &RaidSSCActionContext::greyheart_tidecaller_mark_water_elemental_totem;
        creators["rancid mushroom move away from mushroom spore cloud"] = &RaidSSCActionContext::rancid_mushroom_move_away_from_mushroom_spore_cloud;

        creators["hydross the unstable position frost tank"] = &RaidSSCActionContext::hydross_the_unstable_position_frost_tank;
        creators["hydross the unstable position nature tank"] = &RaidSSCActionContext::hydross_the_unstable_position_nature_tank;
        creators["hydross the unstable prioritize elemental adds"] = &RaidSSCActionContext::hydross_the_unstable_prioritize_elemental_adds;
        creators["hydross the unstable frost phase spread out"] = &RaidSSCActionContext::hydross_the_unstable_frost_phase_spread_out;
        creators["hydross the unstable misdirect boss to tank"] = &RaidSSCActionContext::hydross_the_unstable_misdirect_boss_to_tank;
        creators["hydross the unstable stop dps upon phase change"] = &RaidSSCActionContext::hydross_the_unstable_stop_dps_upon_phase_change;
        creators["hydross the unstable manage timers"] = &RaidSSCActionContext::hydross_the_unstable_manage_timers;

        creators["the lurker below run around behind boss"] = &RaidSSCActionContext::the_lurker_below_run_around_behind_boss;
        creators["the lurker below position main tank"] = &RaidSSCActionContext::the_lurker_below_position_main_tank;
        creators["the lurker below spread ranged"] = &RaidSSCActionContext::the_lurker_below_spread_ranged;
        creators["the lurker below manage spout timer"] = &RaidSSCActionContext::the_lurker_below_manage_spout_timer;

        creators["leotheras the blind target spellbinders"] = &RaidSSCActionContext::leotheras_the_blind_target_spellbinders;
        creators["leotheras the blind demon form tank attack boss"] = &RaidSSCActionContext::leotheras_the_blind_demon_form_tank_attack_boss;
        creators["leotheras the blind position ranged"] = &RaidSSCActionContext::leotheras_the_blind_position_ranged;
        creators["leotheras the blind run away from whirlwind"] = &RaidSSCActionContext::leotheras_the_blind_run_away_from_whirlwind;
        creators["leotheras the blind demon form position melee"] = &RaidSSCActionContext::leotheras_the_blind_demon_form_position_melee;
        creators["leotheras the blind inner demon cheat"] = &RaidSSCActionContext::leotheras_the_blind_inner_demon_cheat;
        creators["leotheras the blind final phase assign dps priority"] = &RaidSSCActionContext::leotheras_the_blind_final_phase_assign_dps_priority;
        creators["leotheras the blind misdirect boss to demon form tank"] = &RaidSSCActionContext::leotheras_the_blind_misdirect_boss_to_demon_form_tank;
        creators["leotheras the blind manage timers and trackers"] = &RaidSSCActionContext::leotheras_the_blind_manage_timers_and_trackers;

        creators["fathom-lord karathress main tank position boss"] = &RaidSSCActionContext::fathom_lord_karathress_main_tank_position_boss;
        creators["fathom-lord karathress first assist tank position sharkkis"] = &RaidSSCActionContext::fathom_lord_karathress_first_assist_tank_position_sharkkis;
        creators["fathom-lord karathress second assist tank position tidalvess"] = &RaidSSCActionContext::fathom_lord_karathress_second_assist_tank_position_tidalvess;
        creators["fathom-lord karathress third assist tank position caribdis"] = &RaidSSCActionContext::fathom_lord_karathress_third_assist_tank_position_caribdis;
        creators["fathom-lord karathress position caribdis tank healer"] = &RaidSSCActionContext::fathom_lord_karathress_position_caribdis_tank_healer;
        creators["fathom-lord karathress misdirect bosses to tanks"] = &RaidSSCActionContext::fathom_lord_karathress_misdirect_bosses_to_tanks;
        creators["fathom-lord karathress assign dps priority"] = &RaidSSCActionContext::fathom_lord_karathress_assign_dps_priority;
        creators["fathom-lord karathress manage dps timer"] = &RaidSSCActionContext::fathom_lord_karathress_manage_dps_timer;

        creators["morogrim tidewalker misdirect boss to main tank"] = &RaidSSCActionContext::morogrim_tidewalker_misdirect_boss_to_main_tank;
        creators["morogrim tidewalker move boss to tank position"] = &RaidSSCActionContext::morogrim_tidewalker_move_boss_to_tank_position;
        creators["morogrim tidewalker phase 2 reposition ranged"] = &RaidSSCActionContext::morogrim_tidewalker_phase_2_reposition_ranged;
        creators["morogrim tidewalker reset phase transition steps"] = &RaidSSCActionContext::morogrim_tidewalker_reset_phase_transition_steps;

        creators["lady vashj main tank position boss"] = &RaidSSCActionContext::lady_vashj_main_tank_position_boss;
        creators["lady vashj phase 1 position ranged"] = &RaidSSCActionContext::lady_vashj_phase_1_position_ranged;
        creators["lady vashj set grounding totem in main tank group"] = &RaidSSCActionContext::lady_vashj_set_grounding_totem_in_main_tank_group;
        creators["lady vashj static charge move away from group"] = &RaidSSCActionContext::lady_vashj_static_charge_move_away_from_group;
        creators["lady vashj misdirect boss to main tank"] = &RaidSSCActionContext::lady_vashj_misdirect_boss_to_main_tank;
        creators["lady vashj misdirect strider to first assist tank"] = &RaidSSCActionContext::lady_vashj_misdirect_strider_to_first_assist_tank;
        creators["lady vashj tank attack and move away strider"] = &RaidSSCActionContext::lady_vashj_tank_attack_and_move_away_strider;
        creators["lady vashj assign dps priority"] = &RaidSSCActionContext::lady_vashj_assign_dps_priority;
        creators["lady vashj loot tainted core"] = &RaidSSCActionContext::lady_vashj_loot_tainted_core;
        creators["lady vashj teleport to tainted elemental"] = &RaidSSCActionContext::lady_vashj_teleport_to_tainted_elemental;
        creators["lady vashj pass the tainted core"] = &RaidSSCActionContext::lady_vashj_pass_the_tainted_core;
        creators["lady vashj destroy tainted core"] = &RaidSSCActionContext::lady_vashj_destroy_tainted_core;
        creators["lady vashj avoid toxic spores"] = &RaidSSCActionContext::lady_vashj_avoid_toxic_spores;
        creators["lady vashj use free action abilities"] = &RaidSSCActionContext::lady_vashj_use_free_action_abilities;
        creators["lady vashj manage trackers"] = &RaidSSCActionContext::lady_vashj_manage_trackers;
    }

private:
    static Action* underbog_colossus_escape_toxic_pool(PlayerbotAI* botAI) { return new UnderbogColossusEscapeToxicPoolAction(botAI); }
    static Action* greyheart_tidecaller_mark_water_elemental_totem(PlayerbotAI* botAI) { return new GreyheartTidecallerMarkWaterElementalTotemAction(botAI); }
    static Action* rancid_mushroom_move_away_from_mushroom_spore_cloud(PlayerbotAI* botAI) { return new RancidMushroomMoveAwayFromMushroomSporeCloudAction(botAI); }

    static Action* hydross_the_unstable_position_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionFrostTankAction(botAI); }
    static Action* hydross_the_unstable_position_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionNatureTankAction(botAI); }
    static Action* hydross_the_unstable_prioritize_elemental_adds(PlayerbotAI* botAI) { return new HydrossTheUnstablePrioritizeElementalAddsAction(botAI); }
    static Action* hydross_the_unstable_frost_phase_spread_out(PlayerbotAI* botAI) { return new HydrossTheUnstableFrostPhaseSpreadOutAction(botAI); }
    static Action* hydross_the_unstable_misdirect_boss_to_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableMisdirectBossToTankAction(botAI); }
    static Action* hydross_the_unstable_stop_dps_upon_phase_change(PlayerbotAI* botAI) { return new HydrossTheUnstableStopDpsUponPhaseChangeAction(botAI); }
    static Action* hydross_the_unstable_manage_timers(PlayerbotAI* botAI) { return new HydrossTheUnstableManageTimersAction(botAI); }

    static Action* the_lurker_below_run_around_behind_boss(PlayerbotAI* botAI) { return new TheLurkerBelowRunAroundBehindBossAction(botAI); }
    static Action* the_lurker_below_position_main_tank(PlayerbotAI* botAI) { return new TheLurkerBelowPositionMainTankAction(botAI); }
    static Action* the_lurker_below_spread_ranged(PlayerbotAI* botAI) { return new TheLurkerBelowSpreadRangedAction(botAI); }
    static Action* the_lurker_below_manage_spout_timer(PlayerbotAI* botAI) { return new TheLurkerBelowManageSpoutTimerAction(botAI); }

    static Action* leotheras_the_blind_target_spellbinders(PlayerbotAI* botAI) { return new LeotherasTheBlindTargetSpellbindersAction(botAI); }
    static Action* leotheras_the_blind_demon_form_tank_attack_boss(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormTankAttackBossAction(botAI); }
    static Action* leotheras_the_blind_position_ranged(PlayerbotAI* botAI) { return new LeotherasTheBlindPositionRangedAction(botAI); }
    static Action* leotheras_the_blind_run_away_from_whirlwind(PlayerbotAI* botAI) { return new LeotherasTheBlindRunAwayFromWhirlwindAction(botAI); }
    static Action* leotheras_the_blind_demon_form_position_melee(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormPositionMeleeAction(botAI); }
    static Action* leotheras_the_blind_inner_demon_cheat(PlayerbotAI* botAI) { return new LeotherasTheBlindInnerDemonCheatAction(botAI); }
    static Action* leotheras_the_blind_misdirect_boss_to_demon_form_tank(PlayerbotAI* botAI) { return new LeotherasTheBlindMisdirectBossToDemonFormTankAction(botAI); }
    static Action* leotheras_the_blind_final_phase_assign_dps_priority(PlayerbotAI* botAI) { return new LeotherasTheBlindFinalPhaseAssignDpsPriorityAction(botAI); }
    static Action* leotheras_the_blind_manage_timers_and_trackers(PlayerbotAI* botAI) { return new LeotherasTheBlindManageTimersAndTrackersAction(botAI); }

    static Action* fathom_lord_karathress_main_tank_position_boss(PlayerbotAI* botAI) { return new FathomLordKarathressMainTankPositionBossAction(botAI); }
    static Action* fathom_lord_karathress_first_assist_tank_position_sharkkis(PlayerbotAI* botAI) { return new FathomLordKarathressFirstAssistTankPositionSharkkisAction(botAI); }
    static Action* fathom_lord_karathress_second_assist_tank_position_tidalvess(PlayerbotAI* botAI) { return new FathomLordKarathressSecondAssistTankPositionTidalvessAction(botAI); }
    static Action* fathom_lord_karathress_third_assist_tank_position_caribdis(PlayerbotAI* botAI) { return new FathomLordKarathressThirdAssistTankPositionCaribdisAction(botAI); }
    static Action* fathom_lord_karathress_position_caribdis_tank_healer(PlayerbotAI* botAI) { return new FathomLordKarathressPositionCaribdisTankHealerAction(botAI); }
    static Action* fathom_lord_karathress_misdirect_bosses_to_tanks(PlayerbotAI* botAI) { return new FathomLordKarathressMisdirectBossesToTanksAction(botAI); }
    static Action* fathom_lord_karathress_assign_dps_priority(PlayerbotAI* botAI) { return new FathomLordKarathressAssignDpsPriorityAction(botAI); }
    static Action* fathom_lord_karathress_manage_dps_timer(PlayerbotAI* botAI) { return new FathomLordKarathressManageDpsTimerAction(botAI); }

    static Action* morogrim_tidewalker_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerMisdirectBossToMainTankAction(botAI); }
    static Action* morogrim_tidewalker_move_boss_to_tank_position(PlayerbotAI* botAI) { return new MorogrimTidewalkerMoveBossToTankPositionAction(botAI); }
    static Action* morogrim_tidewalker_phase_2_reposition_ranged(PlayerbotAI* botAI) { return new MorogrimTidewalkerPhase2RepositionRangedAction(botAI); }
    static Action* morogrim_tidewalker_reset_phase_transition_steps(PlayerbotAI* botAI) { return new MorogrimTidewalkerResetPhaseTransitionStepsAction(botAI); }

    static Action* lady_vashj_main_tank_position_boss(PlayerbotAI* botAI) { return new LadyVashjMainTankPositionBossAction(botAI); }
    static Action* lady_vashj_phase_1_position_ranged(PlayerbotAI* botAI) { return new LadyVashjPhase1PositionRangedAction(botAI); }
    static Action* lady_vashj_set_grounding_totem_in_main_tank_group(PlayerbotAI* botAI) { return new LadyVashjSetGroundingTotemInMainTankGroupAction(botAI); }
    static Action* lady_vashj_static_charge_move_away_from_group(PlayerbotAI* botAI) { return new LadyVashjStaticChargeMoveAwayFromGroupAction(botAI); }
    static Action* lady_vashj_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new LadyVashjMisdirectBossToMainTankAction(botAI); }
    static Action* lady_vashj_misdirect_strider_to_first_assist_tank(PlayerbotAI* botAI) { return new LadyVashjMisdirectStriderToFirstAssistTankAction(botAI); }
    static Action* lady_vashj_tank_attack_and_move_away_strider(PlayerbotAI* botAI) { return new LadyVashjTankAttackAndMoveAwayStriderAction(botAI); }
    static Action* lady_vashj_assign_dps_priority(PlayerbotAI* botAI) { return new LadyVashjAssignDpsPriorityAction(botAI); }
    static Action* lady_vashj_teleport_to_tainted_elemental(PlayerbotAI* botAI) { return new LadyVashjTeleportToTaintedElementalAction(botAI); }
    static Action* lady_vashj_loot_tainted_core(PlayerbotAI* botAI) { return new LadyVashjLootTaintedCoreAction(botAI); }
    static Action* lady_vashj_pass_the_tainted_core(PlayerbotAI* botAI) { return new LadyVashjPassTheTaintedCoreAction(botAI); }
    static Action* lady_vashj_destroy_tainted_core(PlayerbotAI* botAI) { return new LadyVashjDestroyTaintedCoreAction(botAI); }
    static Action* lady_vashj_avoid_toxic_spores(PlayerbotAI* botAI) { return new LadyVashjAvoidToxicSporesAction(botAI); }
    static Action* lady_vashj_use_free_action_abilities(PlayerbotAI* botAI) { return new LadyVashjUseFreeActionAbilitiesAction(botAI); }
    static Action* lady_vashj_manage_trackers(PlayerbotAI* botAI) { return new LadyVashjManageTrackersAction(botAI); }
};

#endif
