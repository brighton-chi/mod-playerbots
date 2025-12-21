#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONCONTEXT_H

#include "RaidBlackTempleActions.h"
#include "NamedObjectContext.h"

class RaidBlackTempleActionContext : public NamedObjectContext<Action>
{
public:
    RaidBlackTempleActionContext()
    {
        // High Warlord Naj'entus

        creators["high warlord naj'entus misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_misdirect_boss_to_main_tank;

        creators["high warlord naj'entus main tank position boss"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_main_tank_position_boss;

        creators["high warlord naj'entus disperse ranged"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_disperse_ranged;

        // Supremus

        // Shade of Akama

        // Teron Gorefiend

        // Gurtogg Bloodboil

        // Reliquary of Souls

        // Mother Shahraz

        // Illidari Council

        // Illidan Stormrage <The Betrayer>
    }

private:
    // High Warlord Naj'entus

    static Action* high_warlord_najentus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusMisdirectBossToMainTankAction(botAI); }

    static Action* high_warlord_najentus_main_tank_position_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusMainTankPositionBossAction(botAI); }

    static Action* high_warlord_najentus_disperse_ranged(
        PlayerbotAI* botAI) { return new HighWarlordNajentusDisperseRangedAction(botAI); }

    // Supremus

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
};

#endif
