#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "RaidTempestKeepActions.h"
#include "NamedObjectContext.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        creators["void reaver position boss"] = &RaidTempestKeepActionContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepActionContext::void_reaver_spread_ranged;
        creators["void reaver arcane orb move away"] = &RaidTempestKeepActionContext::void_reaver_arcane_orb_move_away;
    }

private:
    static Action* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossAction(botAI); }
    static Action* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedAction(botAI); }
    static Action* void_reaver_arcane_orb_move_away(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbMoveAwayAction(botAI); }
};

#endif
