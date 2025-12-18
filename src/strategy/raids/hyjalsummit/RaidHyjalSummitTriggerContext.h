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
        creators["kaz'rogal pulling boss"] =
            &RaidHyjalSummitTriggerContext::kazrogal_pulling_boss;

        creators["kaz'rogal boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::kazrogal_boss_engaged_by_main_tank;

        creators["kaz'rogal boss engaged by assist tanks"] =
            &RaidHyjalSummitTriggerContext::kazrogal_boss_engaged_by_assist_tanks;

        creators["kaz'rogal bot is low on mana"] =
            &RaidHyjalSummitTriggerContext::kazrogal_bot_is_low_on_mana;

        creators["kaz'rogal low mana bots need escape path"] =
            &RaidHyjalSummitTriggerContext::kazrogal_low_mana_bots_need_escape_path;

        creators["kaz'rogal mage or paladin has mark of kaz'rogal"] =
            &RaidHyjalSummitTriggerContext::kazrogal_mage_or_paladin_has_mark_of_kazrogal;

        // Azgalor
        creators["azgalor pulling boss"] =
            &RaidHyjalSummitTriggerContext::azgalor_pulling_boss;

        creators["azgalor boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_engaged_by_main_tank;

        creators["azgalor boss casts rain of fire"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_casts_rain_of_fire;

        creators["azgalor bot is doomed"] =
            &RaidHyjalSummitTriggerContext::azgalor_bot_is_doomed;

        creators["azgalor doomguard spawned"] =
            &RaidHyjalSummitTriggerContext::azgalor_doomguard_spawned;

        // Archimonde
        creators["archimonde pulling boss"] =
            &RaidHyjalSummitTriggerContext::archimonde_pulling_boss;

        creators["archimonde boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_engaged_by_main_tank;

        creators["archimonde boss casts fear"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_casts_fear;

        creators["archimonde boss summoned doomfire"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_summoned_doomfire;
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
    static Trigger* kazrogal_pulling_boss(
        PlayerbotAI* botAI) { return new KazrogalPullingBossTrigger(botAI); }

    static Trigger* kazrogal_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new KazrogalBossEngagedByMainTankTrigger(botAI); }

    static Trigger* kazrogal_boss_engaged_by_assist_tanks(
        PlayerbotAI* botAI) { return new KazrogalBossEngagedByAssistTanksTrigger(botAI); }

    static Trigger* kazrogal_low_mana_bots_need_escape_path(
        PlayerbotAI* botAI) { return new KazrogalLowManaBotsNeedEscapePathTrigger(botAI); }

    static Trigger* kazrogal_bot_is_low_on_mana(
        PlayerbotAI* botAI) { return new KazrogalBotIsLowOnManaTrigger(botAI); }

    static Trigger* kazrogal_mage_or_paladin_has_mark_of_kazrogal(
        PlayerbotAI* botAI) { return new KazrogalMageOrPaladinHasMarkOfKazrogalTrigger(botAI); }

    // Azgalor
    static Trigger* azgalor_pulling_boss(
        PlayerbotAI* botAI) { return new AzgalorPullingBossTrigger(botAI); }

    static Trigger* azgalor_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new AzgalorBossEngagedByMainTankTrigger(botAI); }

    static Trigger* azgalor_boss_casts_rain_of_fire(
        PlayerbotAI* botAI) { return new AzgalorBossCastsRainOfFireTrigger(botAI); }

    static Trigger* azgalor_bot_is_doomed(
        PlayerbotAI* botAI) { return new AzgalorBotIsDoomedTrigger(botAI); }

    static Trigger* azgalor_doomguard_spawned(
        PlayerbotAI* botAI) { return new AzgalorDoomguardSpawnedTrigger(botAI); }

    // Archimonde
    static Trigger* archimonde_pulling_boss(
        PlayerbotAI* botAI) { return new ArchimondePullingBossTrigger(botAI); }

    static Trigger* archimonde_boss_engaged_by_main_tank(
        PlayerbotAI* botAI) { return new ArchimondeBossEngagedByMainTankTrigger(botAI); }

    static Trigger* archimonde_boss_casts_fear(
        PlayerbotAI* botAI) { return new ArchimondeBossCastsFearTrigger(botAI); }

    static Trigger* archimonde_boss_summoned_doomfire(
        PlayerbotAI* botAI) { return new ArchimondeBossSummonedDoomfireTrigger(botAI); }
};

#endif
