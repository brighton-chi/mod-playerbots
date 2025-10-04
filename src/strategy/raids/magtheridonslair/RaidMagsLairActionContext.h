#ifndef _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMAGSLAIRACTIONCONTEXT_H

#include "RaidMagsLairActions.h"
#include "NamedObjectContext.h"

class RaidMagsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidMagsLairActionContext()
    {
        creators["magtheridon hellfire channeler magtheridon tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_magtheridon_tank;
        creators["magtheridon hellfire channeler northwest channeler tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_nw_channeler_tank;
        creators["magtheridon hellfire channeler northeast channeler tank"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_ne_channeler_tank;
        creators["magtheridon hellfire channeler misdirection"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_misdirection;
        creators["magtheridon hellfire channeler dps priority"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_dps_priority;
        creators["magtheridon hellfire channeler warlock"] = &RaidMagsLairActionContext::magtheridon_hellfire_channeler_warlock;
        creators["magtheridon position boss"] = &RaidMagsLairActionContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagsLairActionContext::magtheridon_spread_ranged;
        creators["magtheridon spread healer"] = &RaidMagsLairActionContext::magtheridon_spread_healer;
        creators["magtheridon use manticron cube"] = &RaidMagsLairActionContext::magtheridon_use_manticron_cube;
    }

private:
    static Action* magtheridon_hellfire_channeler_magtheridon_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMagtheridonTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_nw_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNWChannelerTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_ne_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNEChannelerTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_misdirection(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMisdirectionAction(botAI); }
    static Action* magtheridon_hellfire_channeler_dps_priority(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerDPSPriorityAction(botAI); }
    static Action* magtheridon_hellfire_channeler_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWarlockAction(botAI); }
    static Action* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_spread_healer(PlayerbotAI* botAI) { return new MagtheridonSpreadHealerAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
};

#endif
