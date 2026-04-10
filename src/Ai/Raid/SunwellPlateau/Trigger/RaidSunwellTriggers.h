/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLTRIGGERS_H
#define _PLAYERBOT_RAIDSUNWELLTRIGGERS_H

#include "Trigger.h"

// Kalecgos & Sathrovarr the Corruptor

class KalecgosBossEngagedByTankTrigger : public Trigger
{
public:
    KalecgosBossEngagedByTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos boss engaged by tank") {}
    bool IsActive() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public Trigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos spectral rift is open") {}
    bool IsActive() override;
};

class KalecgosBotsTakeSplashDamageTrigger : public Trigger
{
public:
    KalecgosBotsTakeSplashDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos bots take splash damage") {}
    bool IsActive() override;
};

class KalecgosBothBossesMustBeDefeatedTrigger : public Trigger
{
public:
    KalecgosBothBossesMustBeDefeatedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos both bosses must be defeated") {}
    bool IsActive() override;
};

// Brutallus

class BrutallusTrigger : public Trigger
{
public:
    BrutallusTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus") {}
    bool IsActive() override;
};

// Felmyst

class FelmystTrigger : public Trigger
{
public:
    FelmystTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst") {}
    bool IsActive() override;
};

// Eredar Twins (Alythess & Sacrolash)

class EredarTwinsTrigger : public Trigger
{
public:
    EredarTwinsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins") {}
    bool IsActive() override;
};

// M'uru & Entropius

class MuruTrigger : public Trigger
{
public:
    MuruTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru") {}
    bool IsActive() override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenTrigger : public Trigger
{
public:
    KiljaedenTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden") {}
    bool IsActive() override;
};

#endif
