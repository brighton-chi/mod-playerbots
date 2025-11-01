#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "RaidTempestKeepActions.h"
#include "NamedObjectContext.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        creators["alar log debug info"] = &RaidTempestKeepActionContext::alar_log_debug_info;
        creators["alar misdirect boss to main tank"] = &RaidTempestKeepActionContext::alar_misdirect_boss_to_main_tank;
        creators["alar boss tanks move between platforms"] = &RaidTempestKeepActionContext::alar_boss_tanks_move_between_platforms;
        creators["alar melee dps focus on boss"] = &RaidTempestKeepActionContext::alar_melee_dps_focus_on_boss;
        creators["alar ranged dps prioritize adds"] = &RaidTempestKeepActionContext::alar_ranged_dps_prioritize_adds;
        creators["alar position healer"] = &RaidTempestKeepActionContext::alar_position_healer;
        creators["alar add tank pick up embers"] = &RaidTempestKeepActionContext::alar_add_tank_pick_up_embers;
        creators["alar jump from platform"] = &RaidTempestKeepActionContext::alar_jump_from_platform;
        creators["alar move away from rebirth"] = &RaidTempestKeepActionContext::alar_move_away_from_rebirth;
        creators["alar swap tanks on boss"] = &RaidTempestKeepActionContext::alar_swap_tanks_on_boss;
        creators["alar dive bomb spread"] = &RaidTempestKeepActionContext::alar_dive_bomb_spread;
        creators["alar manage timers and trackers"] = &RaidTempestKeepActionContext::alar_manage_timers_and_trackers;

        creators["void reaver position boss"] = &RaidTempestKeepActionContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepActionContext::void_reaver_spread_ranged;

        creators["high astromancer solarian stack bots"] = &RaidTempestKeepActionContext::high_astromancer_solarian_stack_bots;
        creators["high astromancer solarian move away from group"] = &RaidTempestKeepActionContext::high_astromancer_solarian_move_away_from_group;
        creators["high astromancer solarian target solarium priests"] = &RaidTempestKeepActionContext::high_astromancer_solarian_target_solarium_priests;
        creators["high astromancer solarian tank voidwalker"] = &RaidTempestKeepActionContext::high_astromancer_solarian_tank_voidwalker;
        creators["high astromancer solarian cast fear ward on main tank"] = &RaidTempestKeepActionContext::high_astromancer_solarian_cast_fear_ward_on_main_tank;
    }

private:
    static Action* alar_log_debug_info(PlayerbotAI* botAI) { return new AlarLogDebugInfoAction(botAI); }
    static Action* alar_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new AlarMisdirectBossToMainTankAction(botAI); }
    static Action* alar_boss_tanks_move_between_platforms(PlayerbotAI* botAI) { return new AlarBossTanksMoveBetweenPlatformsAction(botAI); }
    static Action* alar_melee_dps_focus_on_boss(PlayerbotAI* botAI) { return new AlarMeleeDpsFocusOnBossAction(botAI); }
    static Action* alar_ranged_dps_prioritize_adds(PlayerbotAI* botAI) { return new AlarRangedDpsPrioritizeAddsAction(botAI); }
    static Action* alar_position_healer(PlayerbotAI* botAI) { return new AlarPositionHealerAction(botAI); }
    static Action* alar_add_tank_pick_up_embers(PlayerbotAI* botAI) { return new AlarAddTankPickUpEmbersAction(botAI); }
    static Action* alar_jump_from_platform(PlayerbotAI* botAI) { return new AlarJumpFromPlatformAction(botAI); }
    static Action* alar_move_away_from_rebirth(PlayerbotAI* botAI) { return new AlarMoveAwayFromRebirthAction(botAI); }
    static Action* alar_swap_tanks_on_boss(PlayerbotAI* botAI) { return new AlarSwapTanksOnBossAction(botAI); }
    static Action* alar_dive_bomb_spread(PlayerbotAI* botAI) { return new AlarDiveBombSpreadAction(botAI); }
    static Action* alar_manage_timers_and_trackers(PlayerbotAI* botAI) { return new AlarManageTimersAndTrackersAction(botAI); }

    static Action* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossAction(botAI); }
    static Action* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedAction(botAI); }

    static Action* high_astromancer_solarian_stack_bots(PlayerbotAI* botAI) { return new HighAstromancerSolarianStackBotsAction(botAI); }
    static Action* high_astromancer_solarian_move_away_from_group(PlayerbotAI* botAI) { return new HighAstromancerSolarianMoveAwayFromGroupAction(botAI); }
    static Action* high_astromancer_solarian_target_solarium_priests(PlayerbotAI* botAI) { return new HighAstromancerSolarianTargetSolariumPriestsAction(botAI); }
    static Action* high_astromancer_solarian_tank_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTankVoidwalkerAction(botAI); }
    static Action* high_astromancer_solarian_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCastFearWardOnMainTankAction(botAI); }
};

#endif
