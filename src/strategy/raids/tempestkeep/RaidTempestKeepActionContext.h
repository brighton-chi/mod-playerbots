#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "RaidTempestKeepActions.h"
#include "NamedObjectContext.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        creators["crimson hand centurion cast polymorph"] = &RaidTempestKeepActionContext::crimson_hand_centurion_cast_polymorph;

        creators["alar misdirect boss to main tank"] = &RaidTempestKeepActionContext::alar_misdirect_boss_to_main_tank;
        creators["alar boss tanks move between platforms"] = &RaidTempestKeepActionContext::alar_boss_tanks_move_between_platforms;
        creators["alar melee dps prioritize adds"] = &RaidTempestKeepActionContext::alar_melee_dps_prioritize_adds;
        creators["alar ranged dps prioritize boss"] = &RaidTempestKeepActionContext::alar_ranged_dps_prioritize_boss;
        creators["alar position healer"] = &RaidTempestKeepActionContext::alar_position_healer;
        creators["alar add tank pick up embers"] = &RaidTempestKeepActionContext::alar_add_tank_pick_up_embers;
        creators["alar jump from platform"] = &RaidTempestKeepActionContext::alar_jump_from_platform;
        creators["alar move away from rebirth"] = &RaidTempestKeepActionContext::alar_move_away_from_rebirth;
        creators["alar swap tanks on boss"] = &RaidTempestKeepActionContext::alar_swap_tanks_on_boss;
        creators["alar return to room center"] = &RaidTempestKeepActionContext::alar_return_to_room_center;
        creators["alar dive bomb spread"] = &RaidTempestKeepActionContext::alar_dive_bomb_spread;
        creators["alar manage timers and trackers"] = &RaidTempestKeepActionContext::alar_manage_timers_and_trackers;

        creators["void reaver position boss"] = &RaidTempestKeepActionContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepActionContext::void_reaver_spread_ranged;
        creators["void reaver use aggro dump ability"] = &RaidTempestKeepActionContext::void_reaver_use_aggro_dump_ability;

        creators["high astromancer solarian stack bots"] = &RaidTempestKeepActionContext::high_astromancer_solarian_stack_bots;
        creators["high astromancer solarian move away from group"] = &RaidTempestKeepActionContext::high_astromancer_solarian_move_away_from_group;
        creators["high astromancer solarian target solarium priests"] = &RaidTempestKeepActionContext::high_astromancer_solarian_target_solarium_priests;
        creators["high astromancer solarian tank voidwalker"] = &RaidTempestKeepActionContext::high_astromancer_solarian_tank_voidwalker;
        creators["high astromancer solarian cast fear ward on main tank"] = &RaidTempestKeepActionContext::high_astromancer_solarian_cast_fear_ward_on_main_tank;

        creators["kaelthas sunstrider log for testing"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_log_for_testing;
        creators["kaelthas sunstrider kite thaladred"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_kite_thaladred;
        creators["kaelthas sunstrider main tank position sanguinar"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_main_tank_position_sanguinar;
        creators["kaelthas sunstrider cast fear ward on sanguinar tank"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_cast_fear_ward_on_sanguinar_tank;
        creators["kaelthas sunstrider warlock tank position capernian"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_warlock_tank_position_capernian;
        creators["kaelthas sunstrider move away from capernian"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_move_away_from_capernian;
        creators["kaelthas sunstrider first assist tank position telonicus"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_first_assist_tank_position_telonicus;
        creators["kaelthas sunstrider misdirect advisors to tanks"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_misdirect_advisors_to_tanks;
        creators["kaelthas sunstrider manage advisor dps timer"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_manage_advisor_dps_timer;
        creators["kaelthas sunstrider group up legendary weapons"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_group_up_legendary_weapons;
        creators["kaelthas sunstrider main tank move devastation away"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_main_tank_move_devastation_away;
        creators["kaelthas sunstrider hunter turn away netherstrand longbow"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_hunter_turn_away_netherstrand_longbow;
        creators["kaelthas sunstrider loot legendary weapons"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_loot_legendary_weapons;
        creators["kaelthas sunstrider use legendary weapons"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_use_legendary_weapons;
        creators["kaelthas sunstrider assign dps priority"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_dps_priority;
        creators["kaelthas sunstrider cheat to test"] = &RaidTempestKeepActionContext::kaelthas_sunstrider_cheat_to_test;
    }

private:
    static Action* crimson_hand_centurion_cast_polymorph(PlayerbotAI* botAI) { return new CrimsonHandCenturionCastPolymorphAction(botAI); }

    static Action* alar_misdirect_boss_to_main_tank(PlayerbotAI* botAI) { return new AlarMisdirectBossToMainTankAction(botAI); }
    static Action* alar_boss_tanks_move_between_platforms(PlayerbotAI* botAI) { return new AlarBossTanksMoveBetweenPlatformsAction(botAI); }
    static Action* alar_melee_dps_prioritize_adds(PlayerbotAI* botAI) { return new AlarMeleeDpsPrioritizeAddsAction(botAI); }
    static Action* alar_ranged_dps_prioritize_boss(PlayerbotAI* botAI) { return new AlarRangedDpsPrioritizeBossAction(botAI); }
    static Action* alar_position_healer(PlayerbotAI* botAI) { return new AlarPositionHealerAction(botAI); }
    static Action* alar_add_tank_pick_up_embers(PlayerbotAI* botAI) { return new AlarAddTankPickUpEmbersAction(botAI); }
    static Action* alar_jump_from_platform(PlayerbotAI* botAI) { return new AlarJumpFromPlatformAction(botAI); }
    static Action* alar_move_away_from_rebirth(PlayerbotAI* botAI) { return new AlarMoveAwayFromRebirthAction(botAI); }
    static Action* alar_swap_tanks_on_boss(PlayerbotAI* botAI) { return new AlarSwapTanksOnBossAction(botAI); }
    static Action* alar_return_to_room_center(PlayerbotAI* botAI) { return new AlarReturnToRoomCenterAction(botAI); }
    static Action* alar_dive_bomb_spread(PlayerbotAI* botAI) { return new AlarDiveBombSpreadAction(botAI); }
    static Action* alar_manage_timers_and_trackers(PlayerbotAI* botAI) { return new AlarManageTimersAndTrackersAction(botAI); }

    static Action* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossAction(botAI); }
    static Action* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedAction(botAI); }
    static Action* void_reaver_use_aggro_dump_ability(PlayerbotAI* botAI) { return new VoidReaverUseAggroDumpAbilityAction(botAI); }

    static Action* high_astromancer_solarian_stack_bots(PlayerbotAI* botAI) { return new HighAstromancerSolarianStackBotsAction(botAI); }
    static Action* high_astromancer_solarian_move_away_from_group(PlayerbotAI* botAI) { return new HighAstromancerSolarianMoveAwayFromGroupAction(botAI); }
    static Action* high_astromancer_solarian_target_solarium_priests(PlayerbotAI* botAI) { return new HighAstromancerSolarianTargetSolariumPriestsAction(botAI); }
    static Action* high_astromancer_solarian_tank_voidwalker(PlayerbotAI* botAI) { return new HighAstromancerSolarianTankVoidwalkerAction(botAI); }
    static Action* high_astromancer_solarian_cast_fear_ward_on_main_tank(PlayerbotAI* botAI) { return new HighAstromancerSolarianCastFearWardOnMainTankAction(botAI); }

    static Action* kaelthas_sunstrider_log_for_testing(PlayerbotAI* botAI) { return new KaelthasSunstriderLogForTestingAction(botAI); }
    static Action* kaelthas_sunstrider_kite_thaladred(PlayerbotAI* botAI) { return new KaelthasSunstriderKiteThaladredAction(botAI); }
    static Action* kaelthas_sunstrider_main_tank_position_sanguinar(PlayerbotAI* botAI) { return new KaelthasSunstriderMainTankPositionSanguinarAction(botAI); }
    static Action* kaelthas_sunstrider_cast_fear_ward_on_sanguinar_tank(PlayerbotAI* botAI) { return new KaelthasSunstriderCastFearWardOnSanguinarTankAction(botAI); }
    static Action* kaelthas_sunstrider_warlock_tank_position_capernian(PlayerbotAI* botAI) { return new KaelthasSunstriderWarlockTankPositionCapernianAction(botAI); }
    static Action* kaelthas_sunstrider_move_away_from_capernian(PlayerbotAI* botAI) { return new KaelthasSunstriderMoveAwayFromCapernianAction(botAI); }
    static Action* kaelthas_sunstrider_first_assist_tank_position_telonicus(PlayerbotAI* botAI) { return new KaelthasSunstriderFirstAssistTankPositionTelonicusAction(botAI); }
    static Action* kaelthas_sunstrider_misdirect_advisors_to_tanks(PlayerbotAI* botAI) { return new KaelthasSunstriderMisdirectAdvisorsToTanksAction(botAI); }
    static Action* kaelthas_sunstrider_manage_advisor_dps_timer(PlayerbotAI* botAI) { return new KaelthasSunstriderManageAdvisorDpsTimerAction(botAI); }
    static Action* kaelthas_sunstrider_group_up_legendary_weapons(PlayerbotAI* botAI) { return new KaelthasSunstriderGroupUpLegendaryWeaponsAction(botAI); }
    static Action* kaelthas_sunstrider_main_tank_move_devastation_away(PlayerbotAI* botAI) { return new KaelthasSunstriderMoveDevastationAwayAction(botAI); }
    static Action* kaelthas_sunstrider_hunter_turn_away_netherstrand_longbow(PlayerbotAI* botAI) { return new KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction(botAI); }
    static Action* kaelthas_sunstrider_loot_legendary_weapons(PlayerbotAI* botAI) { return new KaelthasSunstriderLootLegendaryWeaponsAction(botAI); }
    static Action* kaelthas_sunstrider_use_legendary_weapons(PlayerbotAI* botAI) { return new KaelthasSunstriderUseLegendaryWeaponsAction(botAI); }
    static Action* kaelthas_sunstrider_assign_dps_priority(PlayerbotAI* botAI) { return new KaelthasSunstriderPhase3AssignDpsPriorityAction(botAI); }
    static Action* kaelthas_sunstrider_cheat_to_test(PlayerbotAI* botAI) { return new KaelthasSunstriderCheatToTestAction(botAI); }
};

#endif
