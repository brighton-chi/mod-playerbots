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

        creators["anetheron boss casts carrion swarm"] =
            &RaidHyjalSummitTriggerContext::anetheron_boss_casts_carrion_swarm;

        creators["anetheron bot is targeted by infernal"] =
            &RaidHyjalSummitTriggerContext::anetheron_bot_is_targeted_by_infernal;

        creators["anetheron infernals need to be kept away from raid"] =
            &RaidHyjalSummitTriggerContext::anetheron_infernals_need_to_be_kept_away_from_raid;

        creators["anetheron infernals despawn when boss dies"] =
            &RaidHyjalSummitTriggerContext::anetheron_infernals_despawn_when_boss_dies;

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

    static Trigger* anetheron_boss_casts_carrion_swarm(
        PlayerbotAI* botAI) { return new AnetheronBossCastsCarrionSwarmTrigger(botAI); }

    static Trigger* anetheron_bot_is_targeted_by_infernal(
        PlayerbotAI* botAI) { return new AnetheronBotIsTargetedByInfernalTrigger(botAI); }

    static Trigger* anetheron_infernals_need_to_be_kept_away_from_raid(
        PlayerbotAI* botAI) { return new AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger(botAI); }

    static Trigger* anetheron_infernals_despawn_when_boss_dies(
        PlayerbotAI* botAI) { return new AnetheronInfernalsDespawnWhenBossDiesTrigger(botAI); }

    // Kaz'rogal

    // Azgalor

    // Archimonde

};

#endif
