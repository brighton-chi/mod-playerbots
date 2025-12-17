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

    // Kaz'rogal

    // Azgalor

    // Archimonde

};

#endif
