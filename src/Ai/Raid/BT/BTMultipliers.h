/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTMULTIPLIERS_H
#define PLAYERBOTS_BTMULTIPLIERS_H

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Multiplier.h"
#include <string>

// Shared

class BlackTempleEncounterMultiplier : public Multiplier
{
public:
    BlackTempleEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, BtHelpers::BT_MAP_ID) ?
            GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class BlackTempleDelayDpsCooldownsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BlackTempleDisableKillingSpreeMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleDisableKillingSpreeMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple disable killing spree") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class ShadowmoonReaverHoldChargeBuildingSpellsMultiplier : public Multiplier
{
public:
    ShadowmoonReaverHoldChargeBuildingSpellsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shadowmoon reaver hold charge building spells") {}

    float GetValue(Action* action) override;
};

// High Warlord Naj'entus

class HighWarlordNajentusDisableCombatFormationMoveMultiplier
    : public BlackTempleEncounterMultiplier
{
public:
    HighWarlordNajentusDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "high warlord naj'entus disable combat formation move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighWarlordNajentusTidalShieldHoldPositionMultiplier : public BlackTempleEncounterMultiplier
{
public:
    HighWarlordNajentusTidalShieldHoldPositionMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "high warlord naj'entus tidal shield hold position") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Supremus

class SupremusFocusOnAvoidanceInKitePhaseMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusFocusOnAvoidanceInKitePhaseMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus focus on avoidance in kite phase") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SupremusDoNotReachThroughFireMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusDoNotReachThroughFireMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus do not reach through fire") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Stock avoid aoe flees one hazard at a time, and a volcano only once within 15 yd of it; in the
// kite phase the fire avoidance sees them all. In the tank phase it handles Molten Flame again.
class SupremusDisableAvoidAoeInKitePhaseMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusDisableAvoidAoeInKitePhaseMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus disable avoid aoe in kite phase") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shade of Akama

// Walking up to the platform can lose sight of the target, which would otherwise drop it.
class ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "shade of akama do not drop out of sight target") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ShadeOfAkamaCannotTankBossMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ShadeOfAkamaCannotTankBossMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "shade of akama cannot tank boss") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Teron Gorefiend

class TeronGorefiendControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "teron gorefiend control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TeronGorefiendMarkedBotOnlyMoveToDieMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "teron gorefiend marked bot only move to die") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TeronGorefiendSpiritsAttackShadowyConstructsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendSpiritsAttackShadowyConstructsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "teron gorefiend spirits attack shadowy constructs") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "gurtogg bloodboil control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Fel Rage fixes him on its target, so tanks don't taunt and hunters don't misdirect until it ends.
class GurtoggBloodboilNoThreatRedirectsDuringFelRageMultiplier
    : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilNoThreatRedirectsDuringFelRageMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "gurtogg bloodboil no threat redirects during fel rage") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// The Fel Rage target keeps him for all of Fel Rage, so it uses nothing that drops its threat.
class GurtoggBloodboilFelRageTargetHoldsAggroMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilFelRageTargetHoldsAggroMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "gurtogg bloodboil fel rage target holds aggro") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Non-tanks other than healers hold below the second tank's threat. No hold under Insignificance,
// when no one gains threat.
class GurtoggBloodboilHoldThreatMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilHoldThreatMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "gurtogg bloodboil hold threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsDoNotWasteHealingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsDoNotWasteHealingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls do not waste healing") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Tank face, set behind and disperse move tanks and melee around Suffering, which can carry
// them nearer to her than the tanks.
class ReliquaryOfSoulsControlMeleeMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsControlMeleeMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls control melee movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Lets a mage steal Rune Shield before a Purge or Tranquilizing Shot removes it.
class ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls let mages steal rune shield") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Stock Spell Reflection fires on any cast at the tank, so it would go on Spirit Shock, which
// can't be reflected; the raid's own reflect still answers Deaden.
class ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier
    : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "reliquary of souls save spell reflection for deaden") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Mother Shahraz

class MotherShahrazControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    MotherShahrazControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "mother shahraz control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MotherShahrazFatalAttractionRunAwayMultiplier : public BlackTempleEncounterMultiplier
{
public:
    MotherShahrazFatalAttractionRunAwayMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "mother shahraz fatal attraction run away") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Illidari Council

class IllidariCouncilControlMovementAndTargetingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council control movement and targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;

private:
    float GetTankValue(Action* action);
    float GetNonTankValue(Action* action);
};

class IllidariCouncilDisableMageTankActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableMageTankActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council disable mage tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Dampen Magic is the only buff on the council that can be dispelled, and the mage tank steals it.
class IllidariCouncilHoldMisdirectionAndDispelsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilHoldMisdirectionAndDispelsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council hold misdirection and dispels") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Ranged interrupts are invaluable for Circle of Healing. Melee ones wait until she is in reach.
class IllidariCouncilSaveInterruptsForCircleOfHealingMultiplier
    : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilSaveInterruptsForCircleOfHealingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council save interrupts for circle of healing") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilWaitForDpsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilWaitForDpsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Illidan Stormrage <The Betrayer>

// While he talks, bots move only to their pull places. Not gated on the encounter, which starts as
// the speech ends.
class IllidanStormrageHoldPullPositionsMultiplier : public Multiplier
{
public:
    IllidanStormrageHoldPullPositionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "illidan stormrage hold pull positions") {}

    float GetValue(Action* action) override;
};

class IllidanStormrageControlMovementAndTargetingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage control movement and targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;

private:
    float GetTargetingValue(
        BtHelpers::IllidanPhase phase, Action* action, bool isTankAssist, bool isDpsAssist);
    float GetTankMovementValue(Unit* illidan, BtHelpers::IllidanPhase phase, Action* action);
    float GetNonTankMovementValue(BtHelpers::IllidanPhase phase, Action* action);
};

// If near a Shadowfiend that is not low on health, nothing goes onto a Shadowfiend and no AoE
// spell or heal is cast. This is a proxy for having just birthed a Shadowfiend, which can
// immediately infect the birther if any threat is generated. Switching targets stays free, so a
// bot on a Shadowfiend can still turn to him.
class IllidanStormrageHoldNearNewShadowfiendMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldNearNewShadowfiendMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage hold near new shadowfiend") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Guardians can't be set passive like pets, so they are summoned only in Phases 3 and 5 (to keep
// them from dying in Phase 4 and to save the cooldown through Phases 1 and 2), and not in the
// bot's own hold, which lets Mirror Image and Army of the Dead through as buffs.
class IllidanStormrageHoldGuardianSummonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldGuardianSummonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage hold guardian summons") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shamans set no totems from each phase change until the tanks have aggro, so the ones they
// recalled aren't set again at once. In Phases 1, 3 and 5 they also set none while nearer the
// infection position than their nearest totem: it lies about 52 yd from him, past the 30 yd at
// which totems are recast.
class IllidanStormrageHoldTotemsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldTotemsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage hold totems") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageWaitForDpsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageWaitForDpsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageDisableTankSoulshatterMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableTankSoulshatterMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage disable tank soulshatter") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// While Shadow Demons are up, the only area damage is the warlocks' Seed of Corruption on them,
// which has its own action.
class IllidanStormrageDisableAoeOnShadowDemonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableAoeOnShadowDemonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage disable aoe on shadow demons") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
