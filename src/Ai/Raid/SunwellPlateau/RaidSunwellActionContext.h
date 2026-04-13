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
        creators["felmyst main tank position boss on ground"] =
            &RaidSunwellActionContext::felmyst_main_tank_position_boss_on_ground;

        // Eredar Twins (Alythess & Sacrolash)
        creators["eredar twins"] =
            &RaidSunwellActionContext::eredar_twins;

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
    static Action* felmyst_main_tank_position_boss_on_ground(
        PlayerbotAI* botAI) { return new FelmystMainTankPositionBossOnGroundAction(botAI); }

    // Eredar Twins (Alythess & Sacrolash)
    static Action* eredar_twins(
        PlayerbotAI* botAI) { return new EredarTwinsAction(botAI); }

    // M'uru & Entropius
    static Action* muru(
        PlayerbotAI* botAI) { return new MuruAction(botAI); }

    // Kil'jaeden <The Deceiver>
    static Action* kiljaeden(
        PlayerbotAI* botAI) { return new KiljaedenAction(botAI); }
};

#endif
