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

        // Azgalor

        // Archimonde

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

    // Azgalor

    // Archimonde

};

#endif
