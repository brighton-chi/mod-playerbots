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

        creators["hydross the unstable position frost tank"] = &RaidSSCActionContext::hydross_the_unstable_position_frost_tank;
        creators["hydross the unstable position nature tank"] = &RaidSSCActionContext::hydross_the_unstable_position_nature_tank;
        creators["hydross the unstable manage dps timer"] = &RaidSSCActionContext::hydross_the_unstable_manage_dps_timer;
        creators["hydross the unstable mark elemental adds"] = &RaidSSCActionContext::hydross_the_unstable_mark_elemental_adds;
        creators["hydross the unstable frost phase spread out"] = &RaidSSCActionContext::hydross_the_unstable_frost_phase_spread_out;
        creators["the lurker below run around behind boss"] = &RaidSSCActionContext::the_lurker_below_run_around_behind_boss;
        creators["leotheras the blind human form position boss"] = &RaidSSCActionContext::leotheras_the_blind_human_form_position_boss;
        creators["leotheras the blind demon form position boss"] = &RaidSSCActionContext::leotheras_the_blind_demon_form_position_boss;
        creators["leotheras the blind assign dps priority"] = &RaidSSCActionContext::leotheras_the_blind_assign_dps_priority;
        creators["leotheras the blind run away from whirlwind"] = &RaidSSCActionContext::leotheras_the_blind_run_away_from_whirlwind;
        creators["leotheras the blind manage dps timers"] = &RaidSSCActionContext::leotheras_the_blind_manage_dps_timers;
        creators["fathom-lord karathress main tank position boss"] = &RaidSSCActionContext::fathom_lord_karathress_main_tank_position_boss;
        creators["fathom-lord karathress first assist tank position sharkkis"] = &RaidSSCActionContext::fathom_lord_karathress_first_assist_tank_position_sharkkis;
        creators["fathom-lord karathress second assist tank position tidalvess"] = &RaidSSCActionContext::fathom_lord_karathress_second_assist_tank_position_tidalvess;
        creators["fathom-lord karathress third assist tank position caribdis"] = &RaidSSCActionContext::fathom_lord_karathress_third_assist_tank_position_caribdis;
        creators["fathom-lord karathress position caribdis tank healer"] = &RaidSSCActionContext::fathom_lord_karathress_position_caribdis_tank_healer;
        creators["fathom-lord karathress misdirect bosses to tanks"] = &RaidSSCActionContext::fathom_lord_karathress_misdirect_bosses_to_tanks;
        creators["fathom-lord karathress assign melee dps priority"] = &RaidSSCActionContext::fathom_lord_karathress_assign_melee_dps_priority;
        creators["fathom-lord karathress assign ranged dps priority"] = &RaidSSCActionContext::fathom_lord_karathress_assign_ranged_dps_priority;
        creators["morogrim tidewalker misdirect boss to main tank"] = &RaidSSCActionContext::morogrim_tidewalker_misdirect_boss_to_main_tank;
        creators["morogrim tidewalker move boss to tank position"] = &RaidSSCActionContext::morogrim_tidewalker_move_boss_to_tank_position;
        creators["morogrim tidewalker phase 2 reposition dps and healers"] = &RaidSSCActionContext::morogrim_tidewalker_phase_2_reposition_dps_and_healers;
    }

private:
    static Action* greyheart_tidecaller_mark_water_elemental_totem(PlayerbotAI* botAI) { return new GreyheartTidecallerMarkWaterElementalTotemAction(botAI); }

    static Action* hydross_the_unstable_position_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionFrostTankAction(botAI); }
    static Action* hydross_the_unstable_position_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstablePositionNatureTankAction(botAI); }
    static Action* hydross_the_unstable_manage_dps_timer(PlayerbotAI* botAI) { return new HydrossTheUnstableManageDPSTimerAction(botAI); }
    static Action* hydross_the_unstable_mark_elemental_adds(PlayerbotAI* botAI) { return new HydrossTheUnstableMarkElementalAddsAction(botAI); }
    static Action* hydross_the_unstable_frost_phase_spread_out(PlayerbotAI* botAI) { return new HydrossTheUnstableFrostPhaseSpreadOutAction(botAI); }
    static Action* the_lurker_below_run_around_behind_boss(PlayerbotAI* botAI) { return new TheLurkerBelowRunAroundBehindBossAction(botAI); }
    static Action* leotheras_the_blind_human_form_position_boss(PlayerbotAI* botAI) { return new LeotherasTheBlindHumanFormPositionBossAction(botAI); }
    static Action* leotheras_the_blind_demon_form_position_boss(PlayerbotAI* botAI) { return new LeotherasTheBlindDemonFormPositionBossAction(botAI); }
    static Action* leotheras_the_blind_assign_dps_priority(PlayerbotAI* botAI) { return new LeotherasTheBlindAssignDPSPriorityAction(botAI); }
    static Action* leotheras_the_blind_run_away_from_whirlwind(PlayerbotAI* botAI) { return new LeotherasTheBlindRunAwayFromWhirlwindAction(botAI); }
    static Action* leotheras_the_blind_manage_dps_timers(PlayerbotAI* botAI) { return new LeotherasTheBlindManageDPSTimersAction(botAI); }
    static Action* fathom_lord_karathress_main_tank_position_boss(PlayerbotAI* botAI) { return new FathomLordKarathressMainTankPositionBossAction(botAI); }
    static Action* fathom_lord_karathress_first_assist_tank_position_sharkkis(PlayerbotAI* botAI) { return new FathomLordKarathressFirstAssistTankPositionSharkkisAction(botAI); }
    static Action* fathom_lord_karathress_second_assist_tank_position_tidalvess(PlayerbotAI* botAI) { return new FathomLordKarathressSecondAssistTankPositionTidalvessAction(botAI); }
    static Action* fathom_lord_karathress_third_assist_tank_position_caribdis(PlayerbotAI* botAI) { return new FathomLordKarathressThirdAssistTankPositionCaribdisAction(botAI); }
    static Action* fathom_lord_karathress_position_caribdis_tank_healer(PlayerbotAI* botAI) { return new FathomLordKarathressPositionCaribdisTankHealerAction(botAI); }
    static Action* fathom_lord_karathress_misdirect_bosses_to_tanks(PlayerbotAI* botAI) { return new FathomLordKarathressMisdirectBossesToTanksAction(botAI); }
    static Action* fathom_lord_karathress_assign_melee_dps_priority(PlayerbotAI* botAI) { return new FathomLordKarathressAssignMeleeDPSPriorityAction(botAI); }
    static Action* fathom_lord_karathress_assign_ranged_dps_priority(PlayerbotAI* botAI) { return new FathomLordKarathressAssignRangedDPSPriorityAction(botAI); }
    static Action* morogrim_tidewalker_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerMisdirectBossToMainTankAction(botAI); }
    static Action* morogrim_tidewalker_move_boss_to_tank_position(PlayerbotAI* botAI) { return new MorogrimTidewalkerMoveBossToTankPositionAction(botAI); }
    static Action* morogrim_tidewalker_phase_2_reposition_dps_and_healers(PlayerbotAI* botAI) { return new MorogrimTidewalkerPhase2RepositionDPSAndHealersAction(botAI); }
};

#endif
