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

        creators["high warlord naj'entus remove impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_remove_impaling_spine;

        creators["high warlord naj'entus throw impaling spine"] =
            &RaidBlackTempleActionContext::high_warlord_najentus_throw_impaling_spine;

        // Supremus

        creators["supremus misdirect boss to main tank"] =
            &RaidBlackTempleActionContext::supremus_misdirect_boss_to_main_tank;

        creators["supremus main tank position boss"] =
            &RaidBlackTempleActionContext::supremus_main_tank_position_boss;

        creators["supremus disperse ranged"] =
            &RaidBlackTempleActionContext::supremus_disperse_ranged;

        creators["supremus kite boss"] =
            &RaidBlackTempleActionContext::supremus_kite_boss;

        creators["supremus move away from volcanos"] =
            &RaidBlackTempleActionContext::supremus_move_away_from_volcanos;

        creators["supremus manage phase timer"] =
            &RaidBlackTempleActionContext::supremus_manage_phase_timer;

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

    static Action* high_warlord_najentus_remove_impaling_spine(
        PlayerbotAI* botAI) { return new HighWarlordNajentusRemoveImpalingSpineAction(botAI); }

    static Action* high_warlord_najentus_throw_impaling_spine(
        PlayerbotAI* botAI) { return new HighWarlordNajentusThrowImpalingSpineAction(botAI); }

    // Supremus

    static Action* supremus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new SupremusMisdirectBossToMainTankAction(botAI); }

    static Action* supremus_main_tank_position_boss(
        PlayerbotAI* botAI) { return new SupremusMainTankPositionBossAction(botAI); }

    static Action* supremus_disperse_ranged(
        PlayerbotAI* botAI) { return new SupremusDisperseRangedAction(botAI); }

    static Action* supremus_kite_boss(
        PlayerbotAI* botAI) { return new SupremusKiteBossAction(botAI); }

    static Action* supremus_move_away_from_volcanos(
        PlayerbotAI* botAI) { return new SupremusMoveAwayFromVolcanosAction(botAI); }

    static Action* supremus_manage_phase_timer(
        PlayerbotAI* botAI) { return new SupremusManagePhaseTimerAction(botAI); }

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
};

#endif
