/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLACTIONS_H
#define _PLAYERBOT_RAIDSUNWELLACTIONS_H

#include <vector>

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

// Trash

class VolatileFiendKeepEnemyAwayFromGroupAction : public AttackAction
{
public:
    VolatileFiendKeepEnemyAwayFromGroupAction(
        PlayerbotAI* botAI, std::string const name = "volatile fiend keep enemy away from group") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ApocalypseGuardAttackWithHolyMagicAction : public Action
{
public:
    ApocalypseGuardAttackWithHolyMagicAction(
        PlayerbotAI* botAI, std::string const name = "apocalypse guard attack with holy magic") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Kalecgos

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

class KalecgosRemoveArcaneBuffetAction : public Action
{
public:
    KalecgosRemoveArcaneBuffetAction(
        PlayerbotAI* botAI, std::string const name = "kalecgos remove arcane buffet") : Action(botAI, name) {}
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

// Eredar Twins

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

class EredarTwinsStackInRoomCenterAction : public AttackAction
{
public:
    EredarTwinsStackInRoomCenterAction(
        PlayerbotAI* botAI, std::string const name = "eredar twins stack in room center") : AttackAction(botAI, name) {}
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

// M'uru

class MuruMisdirectEnemiesToTanksAction : public AttackAction
{
public:
    MuruMisdirectEnemiesToTanksAction(
        PlayerbotAI* botAI, std::string const name = "m'uru misdirect enemies to tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruMainTankPickUpEntropiusAction : public AttackAction
{
public:
    MuruMainTankPickUpEntropiusAction(
        PlayerbotAI* botAI, std::string const name = "m'uru main tank pick up entropius") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruPositionRangedAction : public MovementAction
{
public:
    MuruPositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "m'uru position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    void SetEntropiusInitialRangedPositionReached(bool reached);
    bool TryGetEntropiusInitialRangedPosition(Position& position) const;
};

class MuruSetDpsPriorityAction : public AttackAction
{
public:
    MuruSetDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "m'uru set dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Unit* ResolveMuruDpsTarget(
        Unit* muru, Unit* entropius, Unit*& currentTarget, Unit* currentVictim, bool& isMeleeDps);
    Unit* SelectMuruEncounterTarget(
        Unit* currentTarget, Unit* currentVictim, bool isMeleeDps,
        uint32 entry, std::vector<Unit*> const& candidates) const;
};

class MuruKillDarkFiendsWithDispelAction : public Action
{
public:
    MuruKillDarkFiendsWithDispelAction(
        PlayerbotAI* botAI, std::string const name = "m'uru kill dark fiends with dispel") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruDontTouchTheDarkFiendAction : public MovementAction
{
public:
    MuruDontTouchTheDarkFiendAction(
        PlayerbotAI* botAI, std::string const name = "m'uru don't touch the dark fiend") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruTanksGetThatSentinelOutOfHereAction : public AttackAction
{
public:
    MuruTanksGetThatSentinelOutOfHereAction(
        PlayerbotAI* botAI, std::string const name = "m'uru tanks get that sentinel out of here") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    const Position* GetAssignedVoidSentinelTankPosition(Unit* voidSentinel) const;
};

class MuruSetGroundingTotemInFirstAssistTankGroupAction : public Action
{
public:
    MuruSetGroundingTotemInFirstAssistTankGroupAction(
        PlayerbotAI* botAI, std::string const name = "m'uru set grounding totem in first assist tank group") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruSecondAssistTankGuardRangedAction : public MovementAction
{
public:
    MuruSecondAssistTankGuardRangedAction(
        PlayerbotAI* botAI, std::string const name = "m'uru second assist tank guard ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruFleeTheNightAction : public MovementAction
{
public:
    MuruFleeTheNightAction(
        PlayerbotAI* botAI, std::string const name = "m'uru flee the night") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruMooresLawIsDeadAction : public MovementAction
{
public:
    MuruMooresLawIsDeadAction(
        PlayerbotAI* botAI, std::string const name = "m'uru moore's law is dead") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruCastStunOnShadowswordBerserkerAction : public Action
{
public:
    MuruCastStunOnShadowswordBerserkerAction(
        PlayerbotAI* botAI, std::string const name = "m'uru cast stun on shadowsword berserker") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruInterruptFelFireballAction : public Action
{
public:
    MuruInterruptFelFireballAction(
        PlayerbotAI* botAI, std::string const name = "m'uru interrupt fel fireball") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruCastSpellStealOnSpellFuryAction : public Action
{
public:
    MuruCastSpellStealOnSpellFuryAction(
        PlayerbotAI* botAI, std::string const name = "m'uru cast spellsteal on spell fury") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruWarlockEnslaveVoidSpawnAction : public MovementAction
{
public:
    MuruWarlockEnslaveVoidSpawnAction(
        PlayerbotAI* botAI, std::string const name = "m'uru warlock enslave void spawn") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class MuruEnslavedVoidSpawnAttackAction : public Action
{
public:
    MuruEnslavedVoidSpawnAttackAction(
        PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

protected:
    Unit* GetControlledVoidSpawn() const;
    bool CommandControlledCreatureToAttack(Unit* controlled, Unit* target) const;
    Unit* GetVoidSpawnVolleyPriorityTarget(Unit* muru, Unit* entropius) const;
};

class MuruEnslavedVoidSpawnCastShadowBoltVolleyAction : public MuruEnslavedVoidSpawnAttackAction
{
public:
    MuruEnslavedVoidSpawnCastShadowBoltVolleyAction(
        PlayerbotAI* botAI, std::string const name = "m'uru enslaved void spawn cast shadow bolt volley") : MuruEnslavedVoidSpawnAttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenTanksHandleHandsOfTheDeceiverAction : public AttackAction
{
public:
    KiljaedenTanksHandleHandsOfTheDeceiverAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden tanks handle hands of the deceiver") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KiljaedenAvoidArmageddonsAction : public MovementAction
{
public:
    KiljaedenAvoidArmageddonsAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden avoid armageddons") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KiljaedenStackForShieldOfTheBlueAction : public MovementAction
{
public:
    KiljaedenStackForShieldOfTheBlueAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden stack for shield of the blue") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KiljaedenPositionTanksAction : public AttackAction
{
public:
    KiljaedenPositionTanksAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden position tanks") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KiljaedenPositionMeleeAction : public MovementAction
{
public:
    KiljaedenPositionMeleeAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden position melee") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class KiljaedenPositionRangedAction : public MovementAction
{
public:
    KiljaedenPositionRangedAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden position ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool TryGetRangedPosition(Position& position) const;
};

class KiljaedenRemoveFireBloomAction : public Action
{
public:
    KiljaedenRemoveFireBloomAction(
        PlayerbotAI* botAI, std::string const name = "kil'jaeden remove fire bloom") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
