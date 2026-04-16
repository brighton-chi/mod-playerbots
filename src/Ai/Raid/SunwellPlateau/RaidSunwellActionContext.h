/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLACTIONCONTEXT_H
#define _PLAYERBOT_RAIDSUNWELLACTIONCONTEXT_H

#include "RaidSunwellActions.h"
#include "NamedObjectContext.h"

class RaidSunwellActionContext : public NamedObjectContext<Action>
{
public:
    RaidSunwellActionContext()
    {
        // General
        creators["sunwell plateau erase timers and trackers"] =
            &RaidSunwellActionContext::sunwell_plateau_erase_timers_and_trackers;

        // Kalecgos & Sathrovarr
        creators["kalecgos tank position boss"] =
            &RaidSunwellActionContext::kalecgos_tank_position_boss;

        creators["kalecgos enter spectral rift"] =
            &RaidSunwellActionContext::kalecgos_enter_spectral_rift;

        creators["kalecgos disperse ranged"] =
            &RaidSunwellActionContext::kalecgos_disperse_ranged;

        creators["kalecgos determine boss to attack"] =
            &RaidSunwellActionContext::kalecgos_determine_boss_to_attack;

        // Brutallus
        creators["brutallus misdirect boss to main tank"] =
            &RaidSunwellActionContext::brutallus_misdirect_boss_to_main_tank;

        creators["brutallus tanks handle boss"] =
            &RaidSunwellActionContext::brutallus_tanks_handle_boss;

        creators["brutallus position melee"] =
            &RaidSunwellActionContext::brutallus_position_melee;

        creators["brutallus position ranged"] =
            &RaidSunwellActionContext::brutallus_position_ranged;

        creators["brutallus handle burn"] =
            &RaidSunwellActionContext::brutallus_handle_burn;

        // Felmyst
        creators["felmyst misdirect boss to main tank"] =
            &RaidSunwellActionContext::felmyst_misdirect_boss_to_main_tank;

        creators["felmyst main tank position boss on ground"] =
            &RaidSunwellActionContext::felmyst_main_tank_position_boss_on_ground;

        creators["felmyst position ranged on ground"] =
            &RaidSunwellActionContext::felmyst_position_ranged_on_ground;

        creators["felmyst remove encapsulate"] =
            &RaidSunwellActionContext::felmyst_remove_encapsulate;

        creators["felmyst run away from encapsulated player"] =
            &RaidSunwellActionContext::felmyst_run_away_from_encapsulated_player;

        creators["felmyst cast mass dispel on gas nova"] =
            &RaidSunwellActionContext::felmyst_cast_mass_dispel_on_gas_nova;

        creators["felmyst avoid demonic vapor"] =
            &RaidSunwellActionContext::felmyst_avoid_demonic_vapor;

        creators["felmyst kite demonic vapor"] =
            &RaidSunwellActionContext::felmyst_kite_demonic_vapor;

        creators["felmyst avoid fog of corruption"] =
            &RaidSunwellActionContext::felmyst_avoid_fog_of_corruption;

        // Eredar Twins (Alythess & Sacrolash)
        creators["eredar twins jump down from balcony"] =
            &RaidSunwellActionContext::eredar_twins_jump_down_from_balcony;

        creators["eredar twins misdirect bosses to tanks"] =
            &RaidSunwellActionContext::eredar_twins_misdirect_bosses_to_tanks;

        creators["eredar twins main and second assist tanks position sacrolash"] =
            &RaidSunwellActionContext::eredar_twins_main_and_second_assist_tanks_position_sacrolash;

        creators["eredar twins first assist tank move out of blaze"] =
            &RaidSunwellActionContext::eredar_twins_first_assist_tank_move_out_of_blaze;

        creators["eredar twins dps prioritize lady sacrolash"] =
            &RaidSunwellActionContext::eredar_twins_dps_prioritize_lady_sacrolash;

        creators["eredar twins conflagrated bot move from group"] =
            &RaidSunwellActionContext::eredar_twins_conflagrated_bot_move_from_group;

        // M'uru & Entropius
        creators["m'uru"] =
            &RaidSunwellActionContext::muru;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden"] =
            &RaidSunwellActionContext::kiljaeden;
    }

private:
    // General
    static Action* sunwell_plateau_erase_timers_and_trackers(
        PlayerbotAI* botAI) { return new SunwellPlateauEraseTimersAndTrackersAction(botAI); }

    // Kalecgos & Sathrovarr
    static Action* kalecgos_tank_position_boss(
        PlayerbotAI* botAI) { return new KalecgosTankPositionBossAction(botAI); }

    static Action* kalecgos_enter_spectral_rift(
        PlayerbotAI* botAI) { return new KalecgosEnterSpectralRiftAction(botAI); }

    static Action* kalecgos_disperse_ranged(
        PlayerbotAI* botAI) { return new KalecgosDisperseRangedAction(botAI); }

    static Action* kalecgos_determine_boss_to_attack(
        PlayerbotAI* botAI) { return new KalecgosDetermineBossToAttackAction(botAI); }

    // Brutallus
    static Action* brutallus_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new BrutallusMisdirectBossToMainTankAction(botAI); }

    static Action* brutallus_tanks_handle_boss(
        PlayerbotAI* botAI) { return new BrutallusTanksHandleBossAction(botAI); }

    static Action* brutallus_position_melee(
        PlayerbotAI* botAI) { return new BrutallusPositionMeleeAction(botAI); }

    static Action* brutallus_position_ranged(
        PlayerbotAI* botAI) { return new BrutallusPositionRangedAction(botAI); }

    static Action* brutallus_handle_burn(
        PlayerbotAI* botAI) { return new BrutallusHandleBurnAction(botAI); }

    // Felmyst
    static Action* felmyst_misdirect_boss_to_main_tank(
        PlayerbotAI* botAI) { return new FelmystMisdirectBossToMainTankAction(botAI); }

    static Action* felmyst_main_tank_position_boss_on_ground(
        PlayerbotAI* botAI) { return new FelmystMainTankPositionBossOnGroundAction(botAI); }

    static Action* felmyst_position_ranged_on_ground(
        PlayerbotAI* botAI) { return new FelmystPositionRangedOnGroundAction(botAI); }

    static Action* felmyst_remove_encapsulate(
        PlayerbotAI* botAI) { return new FelmystRemoveEncapsulateAction(botAI); }

    static Action* felmyst_run_away_from_encapsulated_player(
        PlayerbotAI* botAI) { return new FelmystRunAwayFromEncapsulatedPlayerAction(botAI); }

    static Action* felmyst_cast_mass_dispel_on_gas_nova(
        PlayerbotAI* botAI) { return new FelmystCastMassDispelOnGasNovaAction(botAI); }

    static Action* felmyst_avoid_demonic_vapor(
        PlayerbotAI* botAI) { return new FelmystAvoidDemonicVaporAction(botAI); }

    static Action* felmyst_kite_demonic_vapor(
        PlayerbotAI* botAI) { return new FelmystKiteDemonicVaporAction(botAI); }

    static Action* felmyst_avoid_fog_of_corruption(
        PlayerbotAI* botAI) { return new FelmystAvoidFogOfCorruptionAction(botAI); }

    // Eredar Twins (Alythess & Sacrolash)
    static Action* eredar_twins_jump_down_from_balcony(
        PlayerbotAI* botAI) { return new EredarTwinsJumpDownFromBalconyAction(botAI); }

    static Action* eredar_twins_misdirect_bosses_to_tanks(
        PlayerbotAI* botAI) { return new EredarTwinsMisdirectBossesToTanksAction(botAI); }

    static Action* eredar_twins_main_and_second_assist_tanks_position_sacrolash(
        PlayerbotAI* botAI) { return new EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction(botAI); }

    static Action* eredar_twins_first_assist_tank_move_out_of_blaze(
        PlayerbotAI* botAI) { return new EredarTwinsFirstAssistTankMoveOutOfBlazeAction(botAI); }

    static Action* eredar_twins_dps_prioritize_lady_sacrolash(
        PlayerbotAI* botAI) { return new EredarTwinsDpsPrioritizeLadySacrolashAction(botAI); }

    static Action* eredar_twins_conflagrated_bot_move_from_group(
        PlayerbotAI* botAI) { return new EredarTwinsConflagratedBotMoveFromGroupAction(botAI); }

    // M'uru & Entropius
    static Action* muru(
        PlayerbotAI* botAI) { return new MuruAction(botAI); }

    // Kil'jaeden <The Deceiver>
    static Action* kiljaeden(
        PlayerbotAI* botAI) { return new KiljaedenAction(botAI); }
};

#endif
