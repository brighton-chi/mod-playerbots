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

    static Trigger* high_warlord_najentus_pulling_boss(
        PlayerbotAI* botAI) { return new HighWarlordNajentusPullingBossTrigger(botAI); }

    static Trigger* high_warlord_najentus_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new HighWarlordNajentusBossEngagedByMainTankTrigger(botAI); }

    static Trigger* high_warlord_najentus_casts_needle_spines(
        PlayerbotAI* botAI) { return new HighWarlordNajentusCastsNeedleSpinesTrigger(botAI); }

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
