/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLMULTIPLIERS_H
#define _PLAYERBOT_RAIDSUNWELLMULTIPLIERS_H

#include "Multiplier.h"

// Kalecgos & Sathrovarr the Corruptor

class KalecgosControlMisdirectionMultiplier : public Multiplier
{
public:
    KalecgosControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos control misdirection") {}
    virtual float GetValue(Action* action);
};

class KalecgosWaitToDecurseMultiplier : public Multiplier
{
public:
    KalecgosWaitToDecurseMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos wait to decurse") {}
    virtual float GetValue(Action* action);
};

class KalecgosControlMovementMultiplier : public Multiplier
{
public:
    KalecgosControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos control movement") {}
    virtual float GetValue(Action* action);
};

class KalecgosDelayCooldownsForSathrovarrMultiplier : public Multiplier
{
public:
    KalecgosDelayCooldownsForSathrovarrMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos delay cooldowns for sathrovarr") {}
    virtual float GetValue(Action* action);
};

// Brutallus

class BrutallusControlMisdirectionMultiplier : public Multiplier
{
public:
    BrutallusControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "brutallus control misdirection") {}
    virtual float GetValue(Action* action);
};

class BrutallusControlMovementMultiplier : public Multiplier
{
public:
    BrutallusControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "brutallus control movement") {}
    virtual float GetValue(Action* action);
};

class BrutallusNoTankingWithTooManyMeteorStacksMultiplier : public Multiplier
{
public:
    BrutallusNoTankingWithTooManyMeteorStacksMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "brutallus no tanking with too many meteor stacks") {}
    virtual float GetValue(Action* action);
};

class BrutallusDelayCooldownsMultiplier : public Multiplier
{
public:
    BrutallusDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "brutallus delay cooldowns") {}
    virtual float GetValue(Action* action);
};

// Felmyst

class FelmystDisableDefaultTargetingMultiplier : public Multiplier
{
public:
    FelmystDisableDefaultTargetingMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst disable default targeting") {}
    virtual float GetValue(Action* action);
};

class FelmystControlMovementMultiplier : public Multiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst control movement") {}
    virtual float GetValue(Action* action);
};

class FelmystPrioritizeFogAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst prioritize fog avoidance") {}
    virtual float GetValue(Action* action);
};

// Eredar Twins (Alythess & Sacrolash)

class EredarTwinsMultiplier : public Multiplier
{
public:
    EredarTwinsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins") {}
    virtual float GetValue(Action* action);
};

// M'uru & Entropius

class MuruMultiplier : public Multiplier
{
public:
    MuruMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru") {}
    virtual float GetValue(Action* action);
};

// Kil'jaeden <The Deceiver>

class KiljaedenMultiplier : public Multiplier
{
public:
    KiljaedenMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kil'jaeden") {}
    virtual float GetValue(Action* action);
};

#endif
