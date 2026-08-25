/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALTRIGGERCONTEXT_H
#define PLAYERBOTS_HYJALTRIGGERCONTEXT_H

#include "HyjalHelpers.h"
#include "HyjalTriggers.h"
#include "NamedObjectContext.h"

class RaidHyjalSummitTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidHyjalSummitTriggerContext()
    {
        // General
        creators["hyjal summit no encounter in progress"] =
            &RaidHyjalSummitTriggerContext::hyjal_summit_no_encounter_in_progress;

        // Rage Winterchill
        creators["rage winterchill pulling boss"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_pulling_boss;

        creators["rage winterchill boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_boss_engaged_by_main_tank;

        creators["rage winterchill ranged should spread"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_ranged_should_spread;

        creators["rage winterchill melee near death and decay"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_melee_near_death_and_decay;

        creators["rage winterchill ranged is standing in death and decay"] =
            &RaidHyjalSummitTriggerContext::rage_winterchill_ranged_is_standing_in_death_and_decay;

        // Anetheron
        creators["anetheron pulling boss or infernal"] =
            &RaidHyjalSummitTriggerContext::anetheron_pulling_boss_or_infernal;

        creators["anetheron boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::anetheron_boss_engaged_by_main_tank;

        creators["anetheron ranged should spread"] =
            &RaidHyjalSummitTriggerContext::anetheron_ranged_should_spread;

        creators["anetheron bot is near inferno target"] =
            &RaidHyjalSummitTriggerContext::anetheron_bot_is_near_inferno_target;

        creators["anetheron bot is targeted by infernal"] =
            &RaidHyjalSummitTriggerContext::anetheron_bot_is_targeted_by_infernal;

        creators["anetheron infernals should be kept away"] =
            &RaidHyjalSummitTriggerContext::anetheron_infernals_should_be_kept_away;

        creators["anetheron should determine dps priority"] =
            &RaidHyjalSummitTriggerContext::anetheron_should_determine_dps_priority;

        // Kaz'rogal
        creators["kaz'rogal pulling boss"] =
            &RaidHyjalSummitTriggerContext::kazrogal_pulling_boss;

        creators["kaz'rogal boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::kazrogal_boss_engaged_by_main_tank;

        creators["kaz'rogal malevolent cleave splits damage"] =
            &RaidHyjalSummitTriggerContext::kazrogal_malevolent_cleave_splits_damage;

        creators["kaz'rogal low mana bots need escape path"] =
            &RaidHyjalSummitTriggerContext::kazrogal_low_mana_bots_need_escape_path;

        creators["kaz'rogal bot is low on mana"] =
            &RaidHyjalSummitTriggerContext::kazrogal_bot_is_low_on_mana;

        creators["kaz'rogal hunter should preserve mana"] =
            &RaidHyjalSummitTriggerContext::kazrogal_hunter_should_preserve_mana;

        creators["kaz'rogal mark on mage or paladin"] =
            &RaidHyjalSummitTriggerContext::kazrogal_mark_on_mage_or_paladin;

        creators["kaz'rogal warlock should manage mana"] =
            &RaidHyjalSummitTriggerContext::kazrogal_warlock_should_manage_mana;

        // Azgalor
        creators["azgalor pulling boss"] =
            &RaidHyjalSummitTriggerContext::azgalor_pulling_boss;

        creators["azgalor boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_engaged_by_main_tank;

        creators["azgalor boss engaged by ranged"] =
            &RaidHyjalSummitTriggerContext::azgalor_boss_engaged_by_ranged;

        creators["azgalor melee near rain of fire"] =
            &RaidHyjalSummitTriggerContext::azgalor_melee_near_rain_of_fire;

        creators["azgalor ranged is standing in rain of fire"] =
            &RaidHyjalSummitTriggerContext::azgalor_ranged_is_standing_in_rain_of_fire;

        creators["azgalor bot is doomed"] =
            &RaidHyjalSummitTriggerContext::azgalor_bot_is_doomed;

        creators["azgalor doomguards must be controlled"] =
            &RaidHyjalSummitTriggerContext::azgalor_doomguards_must_be_controlled;

        creators["azgalor should divide dps"] =
            &RaidHyjalSummitTriggerContext::azgalor_should_divide_dps;

        // Archimonde
        creators["archimonde pulling boss"] =
            &RaidHyjalSummitTriggerContext::archimonde_pulling_boss;

        creators["archimonde boss engaged by main tank"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_engaged_by_main_tank;

        creators["archimonde boss casts fear"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_casts_fear;

        creators["archimonde boss casting air burst"] =
            &RaidHyjalSummitTriggerContext::archimonde_boss_casting_air_burst;

        creators["archimonde ranged should spread"] =
            &RaidHyjalSummitTriggerContext::archimonde_ranged_should_spread;

        creators["archimonde bot is near doomfire"] =
            &RaidHyjalSummitTriggerContext::archimonde_bot_is_near_doomfire;

        creators["archimonde bot stood in doomfire"] =
            &RaidHyjalSummitTriggerContext::archimonde_bot_stood_in_doomfire;
    }

private:
    // General
    static Trigger* hyjal_summit_no_encounter_in_progress(PlayerbotAI* botAI) {
        return new HyjalSummitNoEncounterInProgress(botAI);
    }

    // Rage Winterchill
    static Trigger* rage_winterchill_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(
            botAI, "rage winterchill pulling boss", "rage winterchill");
    }
    static Trigger* rage_winterchill_boss_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HyjalBossEngagedByMainTankTrigger(
            botAI, "rage winterchill boss engaged by main tank", "rage winterchill");
    }
    static Trigger* rage_winterchill_ranged_should_spread(PlayerbotAI* botAI) {
        return new RageWinterchillRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* rage_winterchill_melee_near_death_and_decay(PlayerbotAI* botAI) {
        return new RageWinterchillMeleeNearDeathAndDecayTrigger(botAI);
    }
    static Trigger* rage_winterchill_ranged_is_standing_in_death_and_decay(PlayerbotAI* botAI) {
        return new RageWinterchillRangedIsStandingInDeathAndDecayTrigger(botAI);
    }

    // Anetheron
    static Trigger* anetheron_pulling_boss_or_infernal(PlayerbotAI* botAI) {
        return new AnetheronPullingBossOrInfernalTrigger(botAI);
    }
    static Trigger* anetheron_boss_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HyjalBossEngagedByMainTankTrigger(
            botAI, "anetheron boss engaged by main tank", "anetheron");
    }
    static Trigger* anetheron_ranged_should_spread(PlayerbotAI* botAI) {
        return new AnetheronRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* anetheron_bot_is_near_inferno_target(PlayerbotAI* botAI) {
        return new AnetheronBotIsNearInfernoTargetTrigger(botAI);
    }
    static Trigger* anetheron_bot_is_targeted_by_infernal(PlayerbotAI* botAI) {
        return new AnetheronBotIsTargetedByInfernalTrigger(botAI);
    }
    static Trigger* anetheron_infernals_should_be_kept_away(PlayerbotAI* botAI) {
        return new AnetheronInfernalsShouldBeKeptAwayTrigger(botAI);
    }
    static Trigger* anetheron_should_determine_dps_priority(PlayerbotAI* botAI) {
        return new AnetheronShouldDetermineDpsPriorityTrigger(botAI);
    }

    // Kaz'rogal
    static Trigger* kazrogal_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "kaz'rogal pulling boss", "kaz'rogal");
    }
    static Trigger* kazrogal_boss_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HyjalBossEngagedByMainTankTrigger(
            botAI, "kaz'rogal boss engaged by main tank", "kaz'rogal");
    }
    static Trigger* kazrogal_malevolent_cleave_splits_damage(PlayerbotAI* botAI) {
        return new KazrogalMalevolentCleaveSplitsDamageTrigger(botAI);
    }
    static Trigger* kazrogal_low_mana_bots_need_escape_path(PlayerbotAI* botAI) {
        return new KazrogalLowManaBotsNeedEscapePathTrigger(botAI);
    }
    static Trigger* kazrogal_bot_is_low_on_mana(PlayerbotAI* botAI) {
        return new KazrogalBotIsLowOnManaTrigger(botAI);
    }
    static Trigger* kazrogal_hunter_should_preserve_mana(PlayerbotAI* botAI) {
        return new KazrogalHunterShouldPreserveManaTrigger(botAI);
    }
    static Trigger* kazrogal_mark_on_mage_or_paladin(PlayerbotAI* botAI) {
        return new KazrogalMarkOnMageOrPaladinTrigger(botAI);
    }
    static Trigger* kazrogal_warlock_should_manage_mana(PlayerbotAI* botAI) {
        return new KazrogalWarlockShouldManageManaTrigger(botAI);
    }

    // Azgalor
    static Trigger* azgalor_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "azgalor pulling boss", "azgalor");
    }
    static Trigger* azgalor_boss_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HyjalBossEngagedByMainTankTrigger(
            botAI, "azgalor boss engaged by main tank", "azgalor");
    }
    static Trigger* azgalor_boss_engaged_by_ranged(PlayerbotAI* botAI) {
        return new AzgalorBossEngagedByRangedTrigger(botAI);
    }
    static Trigger* azgalor_melee_near_rain_of_fire(PlayerbotAI* botAI) {
        return new AzgalorMeleeNearRainOfFireTrigger(botAI);
    }
    static Trigger* azgalor_ranged_is_standing_in_rain_of_fire(PlayerbotAI* botAI) {
        return new AzgalorRangedIsStandingInRainOfFireTrigger(botAI);
    }
    static Trigger* azgalor_bot_is_doomed(PlayerbotAI* botAI) {
        return new AzgalorBotIsDoomedTrigger(botAI);
    }
    static Trigger* azgalor_doomguards_must_be_controlled(PlayerbotAI* botAI) {
        return new AzgalorDoomguardsMustBeControlledTrigger(botAI);
    }
    static Trigger* azgalor_should_divide_dps(PlayerbotAI* botAI) {
        return new AzgalorShouldDivideDpsTrigger(botAI);
    }

    // Archimonde
    static Trigger* archimonde_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "archimonde pulling boss", "archimonde");
    }
    static Trigger* archimonde_boss_engaged_by_main_tank(PlayerbotAI* botAI) {
        return new HyjalBossEngagedByMainTankTrigger(
            botAI, "archimonde boss engaged by main tank", "archimonde",
            HyjalHelpers::BOSS_ENGAGED_HEALTH_PCT);
    }
    static Trigger* archimonde_boss_casts_fear(PlayerbotAI* botAI) {
        return new ArchimondeBossCastsFearTrigger(botAI);
    }
    static Trigger* archimonde_boss_casting_air_burst(PlayerbotAI* botAI) {
        return new ArchimondeBossCastingAirBurstTrigger(botAI);
    }
    static Trigger* archimonde_ranged_should_spread(PlayerbotAI* botAI) {
        return new ArchimondeRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* archimonde_bot_is_near_doomfire(PlayerbotAI* botAI) {
        return new ArchimondeBotIsNearDoomfireTrigger(botAI);
    }
    static Trigger* archimonde_bot_stood_in_doomfire(PlayerbotAI* botAI) {
        return new ArchimondeBotStoodInDoomfireTrigger(botAI);
    }
};

#endif
