#ifndef _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H

#include "RaidMagsLairActions.h"
#include "NamedObjectContext.h"

class RaidMagsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidMagsLairActionContext()
    {
        creators["magtheridon hellfire channeler south tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_south_tank;
        creators["magtheridon hellfire channeler west tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_west_tank;
        creators["magtheridon hellfire channeler east tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_east_tank;
        creators["magtheridon hellfire channeler south warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_south_warlock;
        creators["magtheridon hellfire channeler west warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_west_warlock;
        creators["magtheridon hellfire channeler east warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_east_warlock;
        creators["magtheridon hellfire channeler west healer"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_west_healer;
        creators["magtheridon hellfire channeler east healer"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_east_healer;
        creators["magtheridon hellfire channeler west hunter"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_west_hunter;
        creators["magtheridon hellfire channeler east hunter"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_east_hunter;
        creators["magtheridon hellfire channeler dps priority"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_dps_priority;
        creators["magtheridon position boss"] = &RaidMagsLairActionContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagsLairActionContext::magtheridon_spread_ranged;
        creators["magtheridon manticron cube clicker position"] = &RaidMagsLairActionContext::magtheridon_manticron_cube_clicker_position;
        creators["magtheridon use manticron cube"] = &RaidMagsLairActionContext::magtheridon_use_manticron_cube;
    }

private:
    static Action* magtheridon_hellfire_channeler_south_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerSouthTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_west_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_east_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_south_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerSouthWarlockAction(botAI); }
    static Action* magtheridon_hellfire_channeler_west_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestWarlockAction(botAI); }
    static Action* magtheridon_hellfire_channeler_east_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastWarlockAction(botAI); }
    static Action* magtheridon_hellfire_channeler_west_healer(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestHealerAction(botAI); }
    static Action* magtheridon_hellfire_channeler_east_healer(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastHealerAction(botAI); }
    static Action* magtheridon_hellfire_channeler_west_hunter(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestHunterAction(botAI); }
    static Action* magtheridon_hellfire_channeler_east_hunter(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastHunterAction(botAI); }
    static Action* magtheridon_hellfire_channeler_dps_priority(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerDPSPriorityAction(botAI); }
    static Action* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_manticron_cube_clicker_position(PlayerbotAI* botAI) { return new MagtheridonManticronCubeClickerPositionAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
};

#endif
