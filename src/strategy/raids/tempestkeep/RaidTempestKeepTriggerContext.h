#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["void reaver position boss"] = &RaidTempestKeepTriggerContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepTriggerContext::void_reaver_spread_ranged;
        creators["void reaver arcane orb move away"] = &RaidTempestKeepTriggerContext::void_reaver_arcane_orb_move_away;
        creators["high astromancer solarian stack"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_stack;
        creators["high astromancer solarian move away from group"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_move_away_from_group;
        creators["high astromancer solarian target priest adds"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_target_priest_adds;
        creators["high astromancer solarian tank voidwalker"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_tank_voidwalker;
        creators["high astromancer solarian cast fear ward on main tank"] = &RaidTempestKeepTriggerContext::high_astromancer_solarian_cast_fear_ward_on_main_tank;
    }

private:
    static Trigger* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossTrigger(botAI); }
    static Trigger* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedTrigger(botAI); }
    static Trigger* void_reaver_arcane_orb_move_away(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbMoveAwayTrigger(botAI); }
    static Trigger* high_astromancer_solarian_stack(PlayerbotAI* botAI) { return new HighAstromancerSolarianStackTrigger(botAI); }
    static Trigger* high_astromancer_solarian_move_away_from_group(PlayerbotAI* botAI) { return new HighAstromancerSolarianMoveAwayFromGroupTrigger(botAI); }
    static Trigger* high_astromancer_solarian_target_priest_adds(PlayerbotAI* botAI) { return new HighAstromancerSolarianTargetPriestAddsTrigger(botAI); }
    static Trigger* high_astromancer_solarian_tank_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTankVoidwalkerTrigger(botAI); }
    static Trigger* high_astromancer_solarian_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCastFearWardOnMainTankTrigger(botAI); }
};

#endif
