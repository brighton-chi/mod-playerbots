#ifndef _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H

#include "RaidMagsLairActions.h"
#include "NamedObjectContext.h"

class RaidMagsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidMagsLairActionContext()
    {
        creators["magtheridon position boss"] = &RaidMagsLairActionContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagsLairActionContext::magtheridon_spread_ranged;
        creators["magtheridon manticron cube clicker position"] = &RaidMagsLairActionContext::magtheridon_manticron_cube_clicker_position;
        creators["magtheridon use manticron cube"] = &RaidMagsLairActionContext::magtheridon_use_manticron_cube;
    }

private:
    static Action* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_manticron_cube_clicker_position(PlayerbotAI* botAI) { return new MagtheridonManticronCubeClickerPositionAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
};

#endif
