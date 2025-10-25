#ifndef _PLAYERBOT_RAIDSSCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDSSCACTIONCONTEXT_H

#include "RaidSSCActions.h"
#include "NamedObjectContext.h"

class RaidSSCActionContext : public NamedObjectContext<Action>
{
public:
    RaidSSCActionContext()
    {
        creators["greyheart tidecaller mark water elemental totem"] = &RaidSSCActionContext::greyheart_tidecaller_mark_water_elemental_totem;
        creators["rancid mushroom move away from mushroom spore cloud"] = &RaidSSCActionContext::rancid_mushroom_move_away_from_mushroom_spore_cloud;

        creators["hydross the unstable position frost tank"] = &RaidSSCActionContext::hydross_the_unstable_position_frost_tank;
        creators["hydross the unstable position nature tank"] = &RaidSSCActionContext::hydross_the_unstable_position_nature_tank;
        creators["hydross the unstable mark elemental adds"] = &RaidSSCActionContext::hydross_the_unstable_mark_elemental_adds;
        creators["hydross the unstable frost phase spread out"] = &RaidSSCActionContext::hydross_the_unstable_frost_phase_spread_out;
        creators["hydross the unstable misdirect boss to tank"] = &RaidSSCActionContext::hydross_the_unstable_misdirect_boss_to_tank;
        creators["hydross the unstable manage phase change timers"] = &RaidSSCActionContext::hydross_the_unstable_manage_phase_change_timers;
        creators["hydross the unstable manage dps timers"] = &RaidSSCActionContext::hydross_the_unstable_manage_dps_timers;
    
        creators["the lurker below run around behind boss"] = &RaidSSCActionContext::the_lurker_below_run_around_behind_boss;
    
        creators["leotheras the blind target spellbinders"] = &RaidSSCActionContext::leotheras_the_blind_target_spellbinders;
        creators["leotheras the blind demon form position boss"] = &RaidSSCActionContext::leotheras_the_blind_demon_form_position_boss;
        creators["leotheras the blind position ranged"] = &RaidSSCActionContext::leotheras_the_blind_position_ranged;
        creators["leotheras the blind run away from whirlwind"] = &RaidSSCActionContext::leotheras_the_blind_run_away_from_whirlwind;
        // creators["leotheras the blind demon form position melee"] = &RaidSSCActionContext::leotheras_the_blind_demon_form_position_melee;
        creators["leotheras the blind mark and attack inner demon"] = &RaidSSCActionContext::leotheras_the_blind_mark_and_attack_inner_demon;
        creators["leotheras the blind final phase assign dps priority"] = &RaidSSCActionContext::leotheras_the_blind_final_phase_assign_dps_priority;
        creators["leotheras the blind misdirect boss to demon form tank"] = &RaidSSCActionContext::leotheras_the_blind_misdirect_boss_to_demon_form_tank;
        creators["leotheras the blind manage timers and trackers"] = &RaidSSCActionContext::leotheras_the_blind_manage_timers_and_trackers;
    
        creators["fathom-lord karathress main tank position boss"] = &RaidSSCActionContext::fathom_lord_karathress_main_tank_position_boss;
        creators["fathom-lord karathress first assist tank position sharkkis"] = &RaidSSCActionContext::fathom_lord_karathress_first_assist_tank_position_sharkkis;
        creators["fathom-lord karathress second assist tank position tidalvess"] = &RaidSSCActionContext::fathom_lord_karathress_second_assist_tank_position_tidalvess;
        creators["fathom-lord karathress third assist tank position caribdis"] = &RaidSSCActionContext::fathom_lord_karathress_third_assist_tank_position_caribdis;
        creators["fathom-lord karathress position karathress tank healer"] = &RaidSSCActionContext::fathom_lord_karathress_position_karathress_tank_healer;
        creators["fathom-lord karathress position caribdis tank healer"] = &RaidSSCActionContext::fathom_lord_karathress_position_caribdis_tank_healer;
        creators["fathom-lord karathress position tidalvess tank healer"] = &RaidSSCActionContext::fathom_lord_karathress_position_tidalvess_tank_healer;
        creators["fathom-lord karathress misdirect bosses to tanks"] = &RaidSSCActionContext::fathom_lord_karathress_misdirect_bosses_to_tanks;
        creators["fathom-lord karathress assign melee dps priority"] = &RaidSSCActionContext::fathom_lord_karathress_assign_melee_dps_priority;
        creators["fathom-lord karathress assign ranged dps priority"] = &RaidSSCActionContext::fathom_lord_karathress_assign_ranged_dps_priority;
    
        creators["morogrim tidewalker misdirect boss to main tank"] = &RaidSSCActionContext::morogrim_tidewalker_misdirect_boss_to_main_tank;
        creators["morogrim tidewalker move boss to tank position"] = &RaidSSCActionContext::morogrim_tidewalker_move_boss_to_tank_position;
        creators["morogrim tidewalker phase 2 reposition dps and healers"] = &RaidSSCActionContext::morogrim_tidewalker_phase_2_reposition_dps_and_healers;
    }

private:
    static Action* greyheart_tidecaller_mark_water_elemental_totem(PlayerbotAI* botAI) { return new GreyheartTidecallerMarkWaterElementalTotemAction(botAI); }
    static Action* rancid_mushroom_move_away_from_mushroom_spore_cloud(PlayerbotAI* botAI) { return new RancidMushroomMoveAwayFromMushroomSporeCloudAction(botAI); }

    static Action* hydross_the_unstable_position_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionFrostTankAction(botAI); }
    static Action* hydross_the_unstable_position_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionNatureTankAction(botAI); }
    static Action* hydross_the_unstable_mark_elemental_adds(PlayerbotAI* botAI) { return new HydrossTheUnstableMarkElementalAddsAction(botAI); }
    static Action* hydross_the_unstable_frost_phase_spread_out(PlayerbotAI* botAI) { return new HydrossTheUnstableFrostPhaseSpreadOutAction(botAI); }
    static Action* hydross_the_unstable_misdirect_boss_to_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableMisdirectBossToTankAction(botAI); }
    static Action* hydross_the_unstable_manage_phase_change_timers(PlayerbotAI* botAI) { return new HydrossTheUnstableManagePhaseChangeTimersAction(botAI); }
    static Action* hydross_the_unstable_manage_dps_timers(PlayerbotAI* botAI) { return new HydrossTheUnstableManageDPSTimersAction(botAI); }

    static Action* the_lurker_below_run_around_behind_boss(PlayerbotAI* botAI) { return new TheLurkerBelowRunAroundBehindBossAction(botAI); }

    static Action* leotheras_the_blind_target_spellbinders(PlayerbotAI* botAI) { return new LeotherasTheBlindTargetSpellbindersAction(botAI); }
    static Action* leotheras_the_blind_demon_form_position_boss(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormPositionBossAction(botAI); }
    static Action* leotheras_the_blind_position_ranged(PlayerbotAI* botAI) { return new LeotherasTheBlindPositionRangedAction(botAI); }
    static Action* leotheras_the_blind_run_away_from_whirlwind(PlayerbotAI* botAI) { return new LeotherasTheBlindRunAwayFromWhirlwindAction(botAI); }
    // static Action* leotheras_the_blind_demon_form_position_melee(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormPositionMeleeAction(botAI); }
    static Action* leotheras_the_blind_mark_and_attack_inner_demon(PlayerbotAI* botAI) { return new LeotherasTheBlindMarkAndAttackInnerDemonAction(botAI); }
    static Action* leotheras_the_blind_misdirect_boss_to_demon_form_tank(PlayerbotAI* botAI) { return new LeotherasTheBlindMisdirectBossToDemonFormTankAction(botAI); }
    static Action* leotheras_the_blind_final_phase_assign_dps_priority(PlayerbotAI* botAI) { return new LeotherasTheBlindFinalPhaseAssignDPSPriorityAction(botAI); }
    static Action* leotheras_the_blind_manage_timers_and_trackers(PlayerbotAI* botAI) { return new LeotherasTheBlindManageTimersAndTrackersAction(botAI); }

    static Action* fathom_lord_karathress_main_tank_position_boss(PlayerbotAI* botAI) { return new FathomLordKarathressMainTankPositionBossAction(botAI); }
    static Action* fathom_lord_karathress_first_assist_tank_position_sharkkis(PlayerbotAI* botAI) { return new FathomLordKarathressFirstAssistTankPositionSharkkisAction(botAI); }
    static Action* fathom_lord_karathress_second_assist_tank_position_tidalvess(PlayerbotAI* botAI) { return new FathomLordKarathressSecondAssistTankPositionTidalvessAction(botAI); }
    static Action* fathom_lord_karathress_third_assist_tank_position_caribdis(PlayerbotAI* botAI) { return new FathomLordKarathressThirdAssistTankPositionCaribdisAction(botAI); }
    static Action* fathom_lord_karathress_position_karathress_tank_healer(PlayerbotAI* botAI) { return new FathomLordKarathressPositionKarathressTankHealerAction(botAI); }
    static Action* fathom_lord_karathress_position_caribdis_tank_healer(PlayerbotAI* botAI) { return new FathomLordKarathressPositionCaribdisTankHealerAction(botAI); }
    static Action* fathom_lord_karathress_position_tidalvess_tank_healer(PlayerbotAI* botAI) { return new FathomLordKarathressPositionTidalvessTankHealerAction(botAI); }
    static Action* fathom_lord_karathress_misdirect_bosses_to_tanks(PlayerbotAI* botAI) { return new FathomLordKarathressMisdirectBossesToTanksAction(botAI); }
    static Action* fathom_lord_karathress_assign_melee_dps_priority(PlayerbotAI* botAI) { return new FathomLordKarathressAssignMeleeDPSPriorityAction(botAI); }
    static Action* fathom_lord_karathress_assign_ranged_dps_priority(PlayerbotAI* botAI) { return new FathomLordKarathressAssignRangedDPSPriorityAction(botAI); }

    static Action* morogrim_tidewalker_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerMisdirectBossToMainTankAction(botAI); }
    static Action* morogrim_tidewalker_move_boss_to_tank_position(PlayerbotAI* botAI) { return new MorogrimTidewalkerMoveBossToTankPositionAction(botAI); }
    static Action* morogrim_tidewalker_phase_2_reposition_dps_and_healers(PlayerbotAI* botAI) { return new MorogrimTidewalkerPhase2RepositionDPSAndHealersAction(botAI); }
};

#endif
