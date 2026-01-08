#ifndef _PLAYERBOT_RAIDHYJALSUMMITACTIONCONTEXT_H
#define _PLAYERBOT_RAIDHYJALSUMMITACTIONCONTEXT_H

#include "RaidHyjalSummitActions.h"
#include "NamedObjectContext.h"

class RaidHyjalSummitActionContext : public NamedObjectContext<Action>
{
public:
    RaidHyjalSummitActionContext()
    {
        // Rage Winterchill
        creators["rage winterchill misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::rage_winterchill_misdirect_boss_to_main_tank;

        creators["rage winterchill main tank position boss"] =
            &RaidHyjalSummitActionContext::rage_winterchill_main_tank_position_boss;

        creators["rage winterchill spread ranged in circle"] =
            &RaidHyjalSummitActionContext::rage_winterchill_spread_ranged_in_circle;

        // Anetheron
        creators["anetheron misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::anetheron_misdirect_boss_to_main_tank;

        creators["anetheron main tank position boss"] =
            &RaidHyjalSummitActionContext::anetheron_main_tank_position_boss;

        creators["anetheron spread ranged in arc"] =
            &RaidHyjalSummitActionContext::anetheron_spread_ranged_in_arc;

        creators["anetheron bring infernal to infernal tank"] =
            &RaidHyjalSummitActionContext::anetheron_bring_infernal_to_infernal_tank;

        creators["anetheron first assist tank pick up infernals"] =
            &RaidHyjalSummitActionContext::anetheron_first_assist_tank_pick_up_infernals;

        creators["anetheron focus all dps on boss"] =
            &RaidHyjalSummitActionContext::anetheron_focus_all_dps_on_boss;

        // Kaz'rogal
        creators["kaz'rogal misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::kazrogal_misdirect_boss_to_main_tank;

        creators["kaz'rogal main tank position boss"] =
            &RaidHyjalSummitActionContext::kazrogal_main_tank_position_boss;

        creators["kaz'rogal assist tanks move in front of boss"] =
            &RaidHyjalSummitActionContext::kazrogal_assist_tanks_move_in_front_of_boss;

        creators["kaz'rogal spread ranged in arc"] =
            &RaidHyjalSummitActionContext::kazrogal_spread_ranged_in_arc;

        creators["kaz'rogal low mana bot move from group"] =
            &RaidHyjalSummitActionContext::kazrogal_low_mana_bot_move_from_group;

        creators["kaz'rogal remove mark of kaz'rogal"] =
            &RaidHyjalSummitActionContext::kazrogal_remove_mark_of_kazrogal;

        // Azgalor
        creators["azgalor misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::azgalor_misdirect_boss_to_main_tank;

        creators["azgalor main tank position boss"] =
            &RaidHyjalSummitActionContext::azgalor_main_tank_position_boss;

        creators["azgalor disperse ranged"] =
            &RaidHyjalSummitActionContext::azgalor_disperse_ranged;

        creators["azgalor move to doomguard tank"] =
            &RaidHyjalSummitActionContext::azgalor_move_to_doomguard_tank;

        creators["azgalor first assist tank position doomguard"] =
            &RaidHyjalSummitActionContext::azgalor_first_assist_tank_position_doomguard;

        creators["azgalor assign dps priority"] =
            &RaidHyjalSummitActionContext::azgalor_assign_dps_priority;

        // Archimonde
        creators["archimonde misdirect boss to main tank"] =
            &RaidHyjalSummitActionContext::archimonde_misdirect_boss_to_main_tank;

        creators["archimonde cast fear ward on main tank"] =
            &RaidHyjalSummitActionContext::archimonde_cast_fear_ward_on_main_tank;

        creators["archimonde disperse ranged bots"] =
            &RaidHyjalSummitActionContext::archimonde_disperse_ranged_bots;

        creators["archimonde avoid doomfire"] =
            &RaidHyjalSummitActionContext::archimonde_avoid_doomfire;

        creators["archimonde remove doomfire dot"] =
            &RaidHyjalSummitActionContext::archimonde_remove_doomfire_dot;
    }

private:
    // Rage Winterchill
    static Action* rage_winterchill_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new RageWinterchillMisdirectBossToMainTankAction(botAI); }

    static Action* rage_winterchill_main_tank_position_boss(
        PlayerbotAI* botAI) { return new RageWinterchillMainTankPositionBossAction(botAI); }

    static Action* rage_winterchill_spread_ranged_in_circle(
        PlayerbotAI* botAI) { return new RageWinterchillSpreadRangedInCircleAction(botAI); }

    // Anetheron
    static Action* anetheron_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new AnetheronMisdirectBossToMainTankAction(botAI); }

    static Action* anetheron_main_tank_position_boss(
        PlayerbotAI* botAI) { return new AnetheronMainTankPositionBossAction(botAI); }

    static Action* anetheron_spread_ranged_in_arc(
        PlayerbotAI* botAI) { return new AnetheronSpreadRangedInArcAction(botAI); }

    static Action* anetheron_bring_infernal_to_infernal_tank(
        PlayerbotAI* botAI) { return new AnetheronBringInfernalToInfernalTankAction(botAI); }

    static Action* anetheron_first_assist_tank_pick_up_infernals(
        PlayerbotAI* botAI) { return new AnetheronFirstAssistTankPickUpInfernalsAction(botAI); }

    static Action* anetheron_focus_all_dps_on_boss(
        PlayerbotAI* botAI) { return new AnetheronFocusAllDpsOnBossAction(botAI); }

    // Kaz'rogal
    static Action* kazrogal_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new KazrogalMisdirectBossToMainTankAction(botAI); }

    static Action* kazrogal_main_tank_position_boss(
        PlayerbotAI* botAI) { return new KazrogalMainTankPositionBossAction(botAI); }

    static Action* kazrogal_assist_tanks_move_in_front_of_boss(
        PlayerbotAI* botAI) { return new KazrogalAssistTanksMoveInFrontOfBossAction(botAI); }

    static Action* kazrogal_spread_ranged_in_arc(
        PlayerbotAI* botAI) { return new KazrogalSpreadRangedInArcAction(botAI); }

    static Action* kazrogal_low_mana_bot_move_from_group(
        PlayerbotAI* botAI) { return new KazrogalLowManaBotMoveFromGroupAction(botAI); }

    static Action* kazrogal_remove_mark_of_kazrogal(
        PlayerbotAI* botAI) { return new KazrogalRemoveMarkOfKazrogalAction(botAI); }

    // Azgalor
    static Action* azgalor_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new AzgalorMisdirectBossToMainTankAction(botAI); }

    static Action* azgalor_main_tank_position_boss(
        PlayerbotAI* botAI) { return new AzgalorMainTankPositionBossAction(botAI); }

    static Action* azgalor_disperse_ranged(
        PlayerbotAI* botAI) { return new AzgalorDisperseRangedAction(botAI); }

    static Action* azgalor_move_to_doomguard_tank(
        PlayerbotAI* botAI) { return new AzgalorMoveToDoomguardTankAction(botAI); }

    static Action* azgalor_first_assist_tank_position_doomguard(
        PlayerbotAI* botAI) { return new AzgalorFirstAssistTankPositionDoomguardAction(botAI); }

    static Action* azgalor_assign_dps_priority(
        PlayerbotAI* botAI) { return new AzgalorAssignDpsPriorityAction(botAI); }

    // Archimonde
    static Action* archimonde_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new ArchimondeMisdirectBossToMainTankAction(botAI); }

    static Action* archimonde_cast_fear_ward_on_main_tank(
        PlayerbotAI* botAI) { return new ArchimondeCastFearWardOnMainTankAction(botAI); }

    static Action* archimonde_disperse_ranged_bots(
        PlayerbotAI* botAI) { return new ArchimondeDisperseRangedBotsAction(botAI); }

    static Action* archimonde_avoid_doomfire(
        PlayerbotAI* botAI) { return new ArchimondeAvoidDoomfireAction(botAI); }

    static Action* archimonde_remove_doomfire_dot(
        PlayerbotAI* botAI) { return new ArchimondeRemoveDoomfireDotAction(botAI); }
};

#endif
