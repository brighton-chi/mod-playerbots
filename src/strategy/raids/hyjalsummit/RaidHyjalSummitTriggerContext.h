#ifndef _PLAYERBOT_RAIDHYJALSUMMITTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDHYJALSUMMITTRIGGERCONTEXT_H

#include "RaidHyjalSummitTriggers.h"
#include "AiObjectContext.h"

class RaidHyjalSummitTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidHyjalSummitTriggerContext()
    {
        // Rage Winterchill
        creators["rage winterchill pulling boss"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_pulling_boss;

        creators["rage winterchill boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_boss_engaged_by_main_tank;

        creators["rage winterchill boss casts death and decay"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_boss_casts_death_and_decay;

        // Anetheron
        creators["anetheron pulling boss"] =
            &RaidHyjalSummitTriggerContext::anetheron_pulling_boss;

        creators["anetheron boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::anetheron_boss_engaged_by_main_tank;

        // Kaz'rogal

        // Azgalor

        // Archimonde

    }

private:
    // Rage Winterchill
    static Trigger* rage_winterchill_pulling_boss(
        PlayerbotAI* botAI) { return new RageWinterchillPullingBossTrigger(botAI); }

    static Trigger* rage_winterchill_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new RageWinterchillBossEngagedByMainTankTrigger(botAI); }

    static Trigger* rage_winterchill_boss_casts_death_and_decay(
        PlayerbotAI* botAI) { return new RageWinterchillBossCastsDeathAndDecayTrigger(botAI); }

    // Anetheron
    static Trigger* anetheron_pulling_boss(
        PlayerbotAI* botAI) { return new AnetheronPullingBossTrigger(botAI); }

    static Trigger* anetheron_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new AnetheronBossEngagedByMainTankTrigger(botAI); }

    // Kaz'rogal

    // Azgalor

    // Archimonde

};

#endif
