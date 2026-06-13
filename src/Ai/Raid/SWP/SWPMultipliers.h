/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SWPMULTIPLIERS_H
#define _PLAYERBOT_SWPMULTIPLIERS_H

#include <vector>

#include "Multiplier.h"
#include "ObjectGuid.h"

// Kalecgos

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

class KalecgosRestrictTauntMultiplier : public Multiplier
{
public:
    KalecgosRestrictTauntMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos restrict taunt") {}
    virtual float GetValue(Action* action);
};

class KalecgosSuppressAssistTankPullThreatMultiplier : public Multiplier
{
public:
    KalecgosSuppressAssistTankPullThreatMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kalecgos suppress assist tank pull threat") {}
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

class FelmystControlMovementMultiplier : public Multiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst control movement") {}
    virtual float GetValue(Action* action);
};

class FelmystWaitForLandingDpsMultiplier : public Multiplier
{
public:
    FelmystWaitForLandingDpsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst wait for landing dps") {}
    virtual float GetValue(Action* action);
};

class FelmystPrioritizeEncapsulateAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeEncapsulateAvoidanceMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst prioritize encapsulate avoidance") {}
    virtual float GetValue(Action* action);
};

class FelmystPrioritizeFogAvoidanceMultiplier : public Multiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst prioritize fog avoidance") {}
    virtual float GetValue(Action* action);
};

class FelmystPrioritizeDemonicVaporKiteMultiplier : public Multiplier
{
public:
    FelmystPrioritizeDemonicVaporKiteMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst prioritize demonic vapor kite") {}
    virtual float GetValue(Action* action);
};

class FelmystDelayCooldownsMultiplier : public Multiplier
{
public:
    FelmystDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "felmyst delay cooldowns") {}
    virtual float GetValue(Action* action);
};

// Eredar Twins

class EredarTwinsMeleeJumpDownFromBalconyMultiplier : public Multiplier
{
public:
    EredarTwinsMeleeJumpDownFromBalconyMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins melee jump down from balcony") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsControlMisdirectionMultiplier : public Multiplier
{
public:
    EredarTwinsControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins misdirect bosses to tanks") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsControlThreatMultiplier : public Multiplier
{
public:
    EredarTwinsControlThreatMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins control threat") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsDisableTankActionsMultiplier : public Multiplier
{
public:
    EredarTwinsDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins disable tank actions") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsDisableKillingSpreeMultiplier : public Multiplier
{
public:
    EredarTwinsDisableKillingSpreeMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins disable killing spree") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsControlMovementMultiplier : public Multiplier
{
public:
    EredarTwinsControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins control movement") {}
    virtual float GetValue(Action* action);
};

class EredarTwinsDelayCooldownsMultiplier : public Multiplier
{
public:
    EredarTwinsDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "eredar twins delay cooldowns") {}
    virtual float GetValue(Action* action);
};

// M'uru

class MuruDisableDefaultTargetingMultiplier : public Multiplier
{
public:
    MuruDisableDefaultTargetingMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru disable default targeting") {}
    virtual float GetValue(Action* action);
};

class MuruControlMisdirectionMultiplier : public Multiplier
{
public:
    MuruControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru control misdirection") {}
    virtual float GetValue(Action* action);
};

class MuruControlMovementMultiplier : public Multiplier
{
public:
    MuruControlMovementMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru control movement") {}
    virtual float GetValue(Action* action);
};

class MuruUseOnlyGroundingTotemMultiplier : public Multiplier
{
public:
    MuruUseOnlyGroundingTotemMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru use only grounding totem") {}
    virtual float GetValue(Action* action);
};

class MuruDelayCooldownsMultiplier : public Multiplier
{
public:
    MuruDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "m'uru delay cooldowns") {}
    virtual float GetValue(Action* action);
};

// Kil'jaeden <The Deceiver>

class KiljaedenControlMovementAndTargetingMultiplier : public Multiplier
{
public:
    KiljaedenControlMovementAndTargetingMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kil'jaeden control movement and targeting") {}
    virtual float GetValue(Action* action);
};

class KiljaedenDelayCooldownsMultiplier : public Multiplier
{
public:
    KiljaedenDelayCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kil'jaeden delay cooldowns") {}
    virtual float GetValue(Action* action);
};

class KiljaedenPrioritizeDarknessProtectionMultiplier : public Multiplier
{
public:
    KiljaedenPrioritizeDarknessProtectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kil'jaeden prioritize darkness protection") {}
    virtual float GetValue(Action* action);
};

class KiljaedenControlDragonMultiplier : public Multiplier
{
public:
    KiljaedenControlDragonMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "kil'jaeden control dragon") {}
    virtual float GetValue(Action* action);
};

#endif
