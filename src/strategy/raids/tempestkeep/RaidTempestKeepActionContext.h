#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "RaidTempestKeepActions.h"
#include "NamedObjectContext.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        creators["void reaver position boss"] = &RaidTempestKeepActionContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepActionContext::void_reaver_spread_ranged;
        creators["void reaver arcane orb move away"] = &RaidTempestKeepActionContext::void_reaver_arcane_orb_move_away;
        creators["high astromancer solarian stack"] = &RaidTempestKeepActionContext::high_astromancer_solarian_stack;
        creators["high astromancer solarian move away from group"] = &RaidTempestKeepActionContext::high_astromancer_solarian_move_away_from_group;
        creators["high astromancer solarian target priest adds"] = &RaidTempestKeepActionContext::high_astromancer_solarian_target_priest_adds;
        creators["high astromancer solarian tank voidwalker"] = &RaidTempestKeepActionContext::high_astromancer_solarian_tank_voidwalker;
        creators["high astromancer solarian cast fear ward on main tank"] = &RaidTempestKeepActionContext::high_astromancer_solarian_cast_fear_ward_on_main_tank;
    }

private:
    static Action* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossAction(botAI); }
    static Action* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedAction(botAI); }
    static Action* void_reaver_arcane_orb_move_away(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbMoveAwayAction(botAI); }
    static Action* high_astromancer_solarian_stack(PlayerbotAI* botAI) { return new HighAstromancerSolarianStackAction(botAI); }
    static Action* high_astromancer_solarian_move_away_from_group(PlayerbotAI* botAI) { return new HighAstromancerSolarianMoveAwayFromGroupAction(botAI); }
    static Action* high_astromancer_solarian_target_priest_adds(PlayerbotAI* botAI) { return new HighAstromancerSolarianTargetPriestAddsAction(botAI); }
    static Action* high_astromancer_solarian_tank_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTankVoidwalkerAction(botAI); }
    static Action* high_astromancer_solarian_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCastFearWardOnMainTankAction(botAI); }
};

#endif
