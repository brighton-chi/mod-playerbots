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

class BlackTempleNoThreatRedirectsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleNoThreatRedirectsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple no threat redirects") {}

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

class SupremusDisableAvoidAoeInKitePhaseMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusDisableAvoidAoeInKitePhaseMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus disable avoid aoe in kite phase") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shade of Akama

class ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "shade of akama do not drop out of sight target") {}

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

class GurtoggBloodboilFelRageTargetHoldsAggroMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilFelRageTargetHoldsAggroMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "gurtogg bloodboil fel rage target holds aggro") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

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

class ReliquaryOfSoulsControlMeleeMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsControlMeleeMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls control melee movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls let mages steal rune shield") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier : public BlackTempleEncounterMultiplier
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
        : BlackTempleEncounterMultiplier(botAI, "mother shahraz fatal attraction run away") {}

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
    float GetHealerValue(Action* action);
};

class IllidariCouncilDisableMageTankActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableMageTankActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council disable mage tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilHoldMisdirectionAndDispelsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilHoldMisdirectionAndDispelsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council hold misdirection and dispels") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilManageInterruptsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilManageInterruptsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council manage interrupts") {}

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

class IllidanStormrageHoldNearNewShadowfiendMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldNearNewShadowfiendMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage hold near new shadowfiend") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageHoldGuardianSummonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldGuardianSummonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage hold guardian summons") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

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

class IllidanStormrageDisableAoeOnShadowDemonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableAoeOnShadowDemonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage disable aoe on shadow demons") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
