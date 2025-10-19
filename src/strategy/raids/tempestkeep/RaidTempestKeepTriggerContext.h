#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "RaidTempestKeepTriggers.h"
#include "AiObjectContext.h"

class RaidTempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTempestKeepTriggerContext()
    {
        creators["void reaver position boss"] = &RaidTempestKeepTriggerContext::void_reaver_position_boss;
        creators["void reaver spread ranged"] = &RaidTempestKeepTriggerContext::void_reaver_spread_ranged;
        creators["void reaver arcane orb move away"] = &RaidTempestKeepTriggerContext::void_reaver_arcane_orb_move_away;
    }

private:
    static Trigger* void_reaver_position_boss(PlayerbotAI* botAI) { return new VoidReaverPositionBossTrigger(botAI); }
    static Trigger* void_reaver_spread_ranged(PlayerbotAI* botAI) { return new VoidReaverSpreadRangedTrigger(botAI); }
    static Trigger* void_reaver_arcane_orb_move_away(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbMoveAwayTrigger(botAI); }
};

#endif
