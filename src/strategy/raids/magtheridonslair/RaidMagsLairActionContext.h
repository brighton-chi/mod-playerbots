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
        //creators["magtheridon hellfire channeler south warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_south_warlock;
        //creators["magtheridon hellfire channeler west warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_west_warlock;
        //creators["magtheridon hellfire channeler east warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_east_warlock;
        creators["magtheridon hellfire channeler misdirection"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_misdirection;
        creators["magtheridon hellfire channeler dps priority"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_dps_priority;
        creators["magtheridon cc burning abyssal"] = &RaidMagsLairActionContext::magtheridon_cc_burning_abyssal;
        creators["magtheridon position boss"] = &RaidMagsLairActionContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagsLairActionContext::magtheridon_spread_ranged;
        creators["magtheridon spread healer"] = &RaidMagsLairActionContext::magtheridon_spread_healer;
        creators["magtheridon use manticron cube"] = &RaidMagsLairActionContext::magtheridon_use_manticron_cube;
    }

private:
    static Action* magtheridon_hellfire_channeler_south_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerSouthTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_west_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_east_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastTankAction(botAI); }
    //static Action* magtheridon_hellfire_channeler_south_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerSouthWarlockAction(botAI); }
    //static Action* magtheridon_hellfire_channeler_west_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWestWarlockAction(botAI); }
    //static Action* magtheridon_hellfire_channeler_east_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerEastWarlockAction(botAI); }
    static Action* magtheridon_hellfire_channeler_misdirection(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMisdirectionAction(botAI); }
    static Action* magtheridon_hellfire_channeler_dps_priority(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerDPSPriorityAction(botAI); }
    static Action* magtheridon_cc_burning_abyssal(PlayerbotAI* botAI) { return new MagtheridonCCBurningAbyssalAction(botAI); }
    static Action* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_spread_healer(PlayerbotAI* botAI) { return new MagtheridonSpreadHealerAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
};

#endif
