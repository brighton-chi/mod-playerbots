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
        creators["sunwell plateau erase trackers"] =
            &RaidSunwellActionContext::sunwell_plateau_erase_trackers;

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

        // Felmyst
        creators["felmyst"] =
            &RaidSunwellActionContext::felmyst;

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
    static Action* sunwell_plateau_erase_trackers(
        PlayerbotAI* botAI) { return new SunwellPlateauEraseTrackersAction(botAI); }

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

    // Felmyst
    static Action* felmyst(
        PlayerbotAI* botAI) { return new FelmystAction(botAI); }

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
