#ifndef _PLAYERBOT_RAIDBLACKTEMPLETRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDBLACKTEMPLETRIGGERCONTEXT_H

#include "RaidBlackTempleTriggers.h"
#include "AiObjectContext.h"

class RaidBlackTempleTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBlackTempleTriggerContext()
    {
        // High Warlord Naj'entus

        creators["high warlord naj'entus pulling boss"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_pulling_boss;

        creators["high warlord naj'entus boss engaged by main tank"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_engaged_by_main_tank;

        creators["high warlord naj'entus casts needle spines"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_casts_needle_spines;

        creators["high warlord naj'entus player is impaled"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_player_is_impaled;

        creators["high warlord naj'entus boss has tidal shield"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_has_tidal_shield;

        // Supremus

        creators["supremus pulling boss or changing phase"] =
            &RaidBlackTempleTriggerContext::supremus_pulling_boss_or_changing_phase;

        creators["supremus boss engaged by main tank"] =
            &RaidBlackTempleTriggerContext::supremus_boss_engaged_by_main_tank;

        creators["supremus boss engaged by ranged"] =
            &RaidBlackTempleTriggerContext::supremus_boss_engaged_by_ranged;

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

    static Trigger* high_warlord_najentus_pulling_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusPullingBossTrigger(botAI); }

    static Trigger* high_warlord_najentus_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusBossEngagedByMainTankTrigger(botAI); }

    static Trigger* high_warlord_najentus_casts_needle_spines(
        PlayerbotAI* botAI) { return new HighWarlordNajentusCastsNeedleSpinesTrigger(botAI); }

    static Trigger* high_warlord_najentus_player_is_impaled(
        PlayerbotAI* botAI) { return new HighWarlordNajentusPlayerIsImpaledTrigger(botAI); }

    static Trigger* high_warlord_najentus_boss_has_tidal_shield(
        PlayerbotAI* botAI) { return new HighWarlordNajentusBossHasTidalShieldTrigger(botAI); }

    // Supremus

    static Trigger* supremus_pulling_boss_or_changing_phase(
        PlayerbotAI* botAI) { return new SupremusPullingBossOrChangingPhaseTrigger(botAI); }

    static Trigger* supremus_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new SupremusBossEngagedByMainTankTrigger(botAI); }

    static Trigger* supremus_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new SupremusBossEngagedByRangedTrigger(botAI); }

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
};

#endif
