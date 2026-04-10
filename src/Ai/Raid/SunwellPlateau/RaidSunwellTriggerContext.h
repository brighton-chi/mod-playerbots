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
        creators["brutallus"] =
            &RaidSunwellTriggerContext::brutallus;

        // Felmyst
        creators["felmyst"] =
            &RaidSunwellTriggerContext::felmyst;

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
    static Trigger* brutallus(
        PlayerbotAI* botAI) { return new BrutallusTrigger(botAI); }

    // Felmyst
    static Trigger* felmyst(
        PlayerbotAI* botAI) { return new FelmystTrigger(botAI); }

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
