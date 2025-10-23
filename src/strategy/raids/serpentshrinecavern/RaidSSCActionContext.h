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
    static Action* morogrim_tidewalker_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new MorogrimTidewalkerMisdirectBossToMainTankAction(botAI); }
    static Action* morogrim_tidewalker_move_boss_to_tank_position(PlayerbotAI* botAI) { return new MorogrimTidewalkerMoveBossToTankPositionAction(botAI); }
    static Action* morogrim_tidewalker_phase_2_reposition_dps_and_healers(PlayerbotAI* botAI) { return new MorogrimTidewalkerPhase2RepositionDPSAndHealersAction(botAI); }
};

#endif
