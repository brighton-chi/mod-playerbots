#ifndef _PLAYERBOT_RAIDSSCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDSSCACTIONCONTEXT_H

#include "RaidSSCActions.h"
#include "NamedObjectContext.h"

class RaidSSCActionContext : public NamedObjectContext<Action>
{
public:
    RaidSSCActionContext()
    {
        creators["hydross the unstable frost tank position boss"] = &RaidSSCActionContext::hydross_the_unstable_frost_tank_position_boss;
        creators["hydross the unstable frost tank move boss to nature tank"] = &RaidSSCActionContext::hydross_the_unstable_frost_tank_move_boss_to_nature_tank;
        creators["hydross the unstable nature tank position boss"] = &RaidSSCActionContext::hydross_the_unstable_nature_tank_position_boss;
        creators["hydross the unstable nature tank move boss to frost tank"] = &RaidSSCActionContext::hydross_the_unstable_nature_tank_move_boss_to_frost_tank;
        creators["hydross the unstable manage dps timer"] = &RaidSSCActionContext::hydross_the_unstable_manage_dps_timer;
    }

private:
    static Action* hydross_the_unstable_frost_tank_position_boss(PlayerbotAI* botAI) { return new HydrossTheUnstableFrostTankPositionBossAction(botAI); }
    static Action* hydross_the_unstable_frost_tank_move_boss_to_nature_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableFrostTankMoveBossToNatureTankAction(botAI); }
    static Action* hydross_the_unstable_nature_tank_position_boss(PlayerbotAI* botAI) { return new HydrossTheUnstableNatureTankPositionBossAction(botAI); }
    static Action* hydross_the_unstable_nature_tank_move_boss_to_frost_tank(PlayerbotAI* botAI) { return new HydrossTheUnstableNatureTankMoveBossToFrostTankAction(botAI); }
    static Action* hydross_the_unstable_manage_dps_timer(PlayerbotAI* botAI) { return new HydrossTheUnstableManageDPSTimerAction(botAI); }
};

#endif
