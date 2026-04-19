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

class KalecgosReturnToSpectralRealmGroundAction : public MovementAction
{
public:
    KalecgosReturnToSpectralRealmGroundAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos return to spectral realm ground") : MovementAction(botAI, name) {}
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

class FelmystMisdirectBossToMainTankAction : public AttackAction
{
public:
    FelmystMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "felmyst misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystMainTankPositionBossOnGroundAction : public AttackAction
{
public:
    FelmystMainTankPositionBossOnGroundAction(
        PlayerbotAI* botAI, std::string const name = "felmyst main tank position boss on ground") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystPositionRangedOnGroundAction : public MovementAction
{
public:
    FelmystPositionRangedOnGroundAction(
        PlayerbotAI* botAI, std::string const name = "felmyst position ranged on ground") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystPositionMeleeOnGroundAction : public MovementAction
{
public:
    FelmystPositionMeleeOnGroundAction(
        PlayerbotAI* botAI, std::string const name = "felmyst position melee on ground") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystRemoveEncapsulateAction : public Action
{
public:
    FelmystRemoveEncapsulateAction(
        PlayerbotAI* botAI, std::string const name = "felmyst remove encapsulate") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystRunAwayFromEncapsulatedPlayerAction : public MovementAction
{
public:
    FelmystRunAwayFromEncapsulatedPlayerAction(
        PlayerbotAI* botAI, std::string const name = "felmyst run away from encapsulated player") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystCastMassDispelOnGasNovaAction : public Action
{
public:
    FelmystCastMassDispelOnGasNovaAction(
        PlayerbotAI* botAI, std::string const name = "felmyst cast mass dispel on gas nova") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystAvoidDemonicVaporAction : public MovementAction
{
public:
    FelmystAvoidDemonicVaporAction(
        PlayerbotAI* botAI, std::string const name = "felmyst avoid demonic vapor") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystKiteDemonicVaporAction : public MovementAction
{
public:
    FelmystKiteDemonicVaporAction(
        PlayerbotAI* botAI, std::string const name = "felmyst kite demonic vapor") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class FelmystAvoidFogOfCorruptionAction : public MovementAction
{
public:
    FelmystAvoidFogOfCorruptionAction(
        PlayerbotAI* botAI, std::string const name = "felmyst avoid fog of corruption") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Eredar Twins (Alythess & Sacrolash)

class EredarTwinsMeleeJumpDownFromBalconyAction : public MovementAction
{
public:
    EredarTwinsMeleeJumpDownFromBalconyAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins melee jump down from balcony") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsMisdirectBossesToTanksAction : public AttackAction
{
public:
    EredarTwinsMisdirectBossesToTanksAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins misdirect bosses to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction : public AttackAction
{
public:
    EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins main and second assist tanks position sacrolash") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsFirstAssistTankMoveOutOfBlazeAction : public AttackAction
{
public:
    EredarTwinsFirstAssistTankMoveOutOfBlazeAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins first assist tank move out of blaze") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsPositionRangedAction : public MovementAction
{
public:
    EredarTwinsPositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsStackInRoomCenterAction : public MovementAction
{
public:
    EredarTwinsStackInRoomCenterAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins stack in room center") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsRemoveFlameSearAction : public Action
{
public:
    EredarTwinsRemoveFlameSearAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins remove flame sear") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsDpsPrioritizeLadySacrolashAction : public AttackAction
{
public:
    EredarTwinsDpsPrioritizeLadySacrolashAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins dps prioritize lady sacrolash") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class EredarTwinsConflagratedBotMoveFromGroupAction : public MovementAction
{
public:
    EredarTwinsConflagratedBotMoveFromGroupAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins conflagrated bot move from group") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// M'uru & Entropius

class MuruKillDarkFiendsWithDispelAction : public Action
{
public:
    MuruKillDarkFiendsWithDispelAction(
        PlayerbotAI* botAI, std::string const name = "m'uru kill dark fiends with dispel") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruAvoidDarknessAction : public MovementAction
{
public:
    MuruAvoidDarknessAction(
        PlayerbotAI* botAI, std::string const name = "m'uru avoid darkness") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruWarlockEnslaveVoidSpawnAction : public MovementAction
{
public:
    MuruWarlockEnslaveVoidSpawnAction(
        PlayerbotAI* botAI, std::string const name = "m'uru warlock enslave void spawn") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruEnslavedVoidSpawnCastShadowBoltVolleyAction : public Action
{
public:
    MuruEnslavedVoidSpawnCastShadowBoltVolleyAction(
        PlayerbotAI* botAI, std::string const name = "m'uru enslaved void spawn cast shadow bolt volley") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruSetDpsPriorityAction : public AttackAction
{
public:
    MuruSetDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "m'uru set dps priority") : AttackAction(botAI, name) {}
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
