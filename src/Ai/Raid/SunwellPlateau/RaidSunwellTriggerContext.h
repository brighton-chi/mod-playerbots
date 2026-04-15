/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDSUNWELLTRIGGERCONTEXT_H

#include "RaidSunwellTriggers.h"
#include "AiObjectContext.h"

class RaidSunwellTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSunwellTriggerContext()
    {
        // General
        creators["sunwell plateau bot is not in combat"] =
            &RaidSunwellTriggerContext::sunwell_plateau_bot_is_not_in_combat;

        // Kalecgos & Sathrovarr
        creators["kalecgos boss engaged by tank"] =
            &RaidSunwellTriggerContext::kalecgos_boss_engaged_by_tank;

        creators["kalecgos spectral rift is open"] =
            &RaidSunwellTriggerContext::kalecgos_spectral_rift_is_open;

        creators["kalecgos bots take splash damage"] =
            &RaidSunwellTriggerContext::kalecgos_bots_take_splash_damage;

        creators["kalecgos both bosses must be defeated"] =
            &RaidSunwellTriggerContext::kalecgos_both_bosses_must_be_defeated;

        // Brutallus
        creators["brutallus pulling boss"] =
            &RaidSunwellTriggerContext::brutallus_pulling_boss;

        creators["brutallus boss engaged by tanks"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_tanks;

        creators["brutallus boss engaged by melee"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_melee;

        creators["brutallus boss engaged by ranged"] =
            &RaidSunwellTriggerContext::brutallus_boss_engaged_by_ranged;

        creators["brutallus bot is burning"] =
            &RaidSunwellTriggerContext::brutallus_bot_is_burning;

        // Felmyst
        creators["felmyst pulling boss"] =
            &RaidSunwellTriggerContext::felmyst_pulling_boss;

        creators["felmyst boss engaged by main tank on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_main_tank_on_ground;

        creators["felmyst boss engaged by ranged on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_ranged_on_ground;

        creators["felmyst boss engaged by melee on ground"] =
            &RaidSunwellTriggerContext::felmyst_boss_engaged_by_melee_on_ground;

        creators["felmyst bot is encapsulated"] =
            &RaidSunwellTriggerContext::felmyst_bot_is_encapsulated;

        creators["felmyst bot near encapsulated player"] =
            &RaidSunwellTriggerContext::felmyst_bot_near_encapsulated_player;

        creators["felmyst player has gas nova"] =
            &RaidSunwellTriggerContext::felmyst_player_has_gas_nova;

        creators["felmyst boss summons demonic vapor"] =
            &RaidSunwellTriggerContext::felmyst_boss_summons_demonic_vapor;

        creators["felmyst bot is demonic vapor target"] =
            &RaidSunwellTriggerContext::felmyst_bot_is_demonic_vapor_target;

        creators["felmyst fog of corruption is active"] =
            &RaidSunwellTriggerContext::felmyst_fog_of_corruption_is_active;

        // Eredar Twins (Alythess & Sacrolash)
        creators["eredar twins"] =
            &RaidSunwellTriggerContext::eredar_twins;

        // M'uru & Entropius
        creators["m'uru"] =
            &RaidSunwellTriggerContext::muru;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden"] =
            &RaidSunwellTriggerContext::kiljaeden;
    }

private:
    // General
    static Trigger* sunwell_plateau_bot_is_not_in_combat(
        PlayerbotAI* botAI) { return new SunwellPlateauBotIsNotInCombatTrigger(botAI); }

    // Kalecgos & Sathrovarr
    static Trigger* kalecgos_boss_engaged_by_tank(
        PlayerbotAI* botAI) { return new KalecgosBossEngagedByTankTrigger(botAI); }

    static Trigger* kalecgos_spectral_rift_is_open(
        PlayerbotAI* botAI) { return new KalecgosSpectralRiftIsOpenTrigger(botAI); }

    static Trigger* kalecgos_bots_take_splash_damage(
        PlayerbotAI* botAI) { return new KalecgosBotsTakeSplashDamageTrigger(botAI); }

    static Trigger* kalecgos_both_bosses_must_be_defeated(
        PlayerbotAI* botAI) { return new KalecgosBothBossesMustBeDefeatedTrigger(botAI); }

    // Brutallus
    static Trigger* brutallus_pulling_boss(
        PlayerbotAI* botAI) { return new BrutallusPullingBossTrigger(botAI); }

    static Trigger* brutallus_boss_engaged_by_tanks(
        PlayerbotAI* botAI) { return new BrutallusBossEngagedByTanksTrigger(botAI); }

    static Trigger* brutallus_boss_engaged_by_melee(
        PlayerbotAI* botAI) { return new BrutallusBossEngagedByMeleeTrigger(botAI); }

    static Trigger* brutallus_boss_engaged_by_ranged(
        PlayerbotAI* botAI) { return new BrutallusBossEngagedByRangedTrigger(botAI); }

    static Trigger* brutallus_bot_is_burning(
        PlayerbotAI* botAI) { return new BrutallusBotIsBurningTrigger(botAI); }

    // Felmyst
    static Trigger* felmyst_pulling_boss(
        PlayerbotAI* botAI) { return new FelmystPullingBossTrigger(botAI); }

    static Trigger* felmyst_boss_engaged_by_main_tank_on_ground(
        PlayerbotAI* botAI) { return new FelmystBossEngagedByMainTankOnGroundTrigger(botAI); }

    static Trigger* felmyst_boss_engaged_by_ranged_on_ground(
        PlayerbotAI* botAI) { return new FelmystBossEngagedByRangedOnGroundTrigger(botAI); }

    static Trigger* felmyst_boss_engaged_by_melee_on_ground(
        PlayerbotAI* botAI) { return new FelmystBossEngagedByMeleeOnGroundTrigger(botAI); }

    static Trigger* felmyst_bot_is_encapsulated(
        PlayerbotAI* botAI) { return new FelmystBotIsEncapsulatedTrigger(botAI); }

    static Trigger* felmyst_bot_near_encapsulated_player(
        PlayerbotAI* botAI) { return new FelmystBotNearEncapsulatedPlayerTrigger(botAI); }

    static Trigger* felmyst_player_has_gas_nova(
        PlayerbotAI* botAI) { return new FelmystPlayerHasGasNovaTrigger(botAI); }

    static Trigger* felmyst_boss_summons_demonic_vapor(
        PlayerbotAI* botAI) { return new FelmystBossSummonsDemonicVaporTrigger(botAI); }

    static Trigger* felmyst_bot_is_demonic_vapor_target(
        PlayerbotAI* botAI) { return new FelmystBotIsDemonicVaporTargetTrigger(botAI); }

    static Trigger* felmyst_fog_of_corruption_is_active(
        PlayerbotAI* botAI) { return new FelmystFogOfCorruptionIsActiveTrigger(botAI); }

    // Eredar Twins (Alythess & Sacrolash)
    static Trigger* eredar_twins(
        PlayerbotAI* botAI) { return new EredarTwinsTrigger(botAI); }

    // M'uru & Entropius
    static Trigger* muru(
        PlayerbotAI* botAI) { return new MuruTrigger(botAI); }

    // Kil'jaeden <The Deceiver>
    static Trigger* kiljaeden(
        PlayerbotAI* botAI) { return new KiljaedenTrigger(botAI); }
};

#endif
