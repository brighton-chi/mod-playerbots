/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLACTIONS_H
#define _PLAYERBOT_RAIDSUNWELLACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class SunwellPlateauEraseTimersAndTrackersAction : public Action
{
public:
    SunwellPlateauEraseTimersAndTrackersAction(
        PlayerbotAI* botAI, std::string const name = "sunwell plateau erase timers and trackers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Kalecgos & Sathrovarr the Corruptor

class KalecgosTankPositionBossAction : public AttackAction
{
public:
    KalecgosTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KalecgosEnterSpectralRiftAction : public MovementAction
{
public:
    KalecgosEnterSpectralRiftAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos enter spectral rift") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KalecgosDisperseRangedAction : public MovementAction
{
public:
    KalecgosDisperseRangedAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos disperse ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KalecgosDetermineBossToAttackAction : public AttackAction
{
public:
    KalecgosDetermineBossToAttackAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos determine boss to attack") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Brutallus

class BrutallusMisdirectBossToMainTankAction : public AttackAction
{
public:
    BrutallusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "brutallus misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BrutallusTanksHandleBossAction : public AttackAction
{
public:
    BrutallusTanksHandleBossAction(
        PlayerbotAI* botAI, std::string const name = "brutallus tanks handle boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BrutallusPositionMeleeAction : public MovementAction
{
public:
    BrutallusPositionMeleeAction(
        PlayerbotAI* botAI, std::string const name = "brutallus position melee") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BrutallusPositionRangedAction : public MovementAction
{
public:
    BrutallusPositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "brutallus position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class BrutallusHandleBurnAction : public MovementAction
{
public:
    BrutallusHandleBurnAction(
        PlayerbotAI* botAI, std::string const name = "brutallus handle burn") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool RemoveBurnWithCooldown(Player* bot);
};

// Felmyst

class FelmystAction : public MovementAction
{
public:
    FelmystAction(
        PlayerbotAI* botAI, std::string const name = "felmyst") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Eredar Twins (Alythess & Sacrolash)

class EredarTwinsAction : public MovementAction
{
public:
    EredarTwinsAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// M'uru & Entropius

class MuruAction : public MovementAction
{
public:
    MuruAction(
        PlayerbotAI* botAI, std::string const name = "m'uru") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenAction : public MovementAction
{
public:
    KiljaedenAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
