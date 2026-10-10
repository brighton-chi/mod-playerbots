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

// Supremus's bounding radius is 80 yd, far beyond his combat reach, and Killing Spree places the
// rogue by bounding radius, so it takes the rogue well out of the fight.
// Essence of Suffering targets the closest player, and Killing Spree makes the Rogue the closest.
class BlackTempleDisableKillingSpreeMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleDisableKillingSpreeMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple disable killing spree") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

// Not gated on an encounter: Reavers are trash.
class ShadowmoonReaverHoldChargeBuildingSpellsMultiplier : public Multiplier
{
public:
    ShadowmoonReaverHoldChargeBuildingSpellsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shadowmoon reaver hold charge building spells") {}

    float GetValue(Action* action) override;
};

// High Warlord Naj'entus

class HighWarlordNajentusDisableCombatFormationMoveMultiplier : public BlackTempleEncounterMultiplier
{
public:
    HighWarlordNajentusDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "high warlord naj'entus disable combat formation move") {}

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

// Stock reach walks a straight line, so it gives way while that line passes through fire.
class SupremusDoNotReachThroughFireMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusDoNotReachThroughFireMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus do not reach through fire") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Stock avoid aoe flees one patch at a time and can't see volcanoes; in the kite phase the fire
// avoidance sees both. In the tank phase it handles Molten Flame again.
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
class ShadeOfAkamaDontDropOutOfSightTargetMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ShadeOfAkamaDontDropOutOfSightTargetMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "shade of akama don't drop out of sight target") {}

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

class TeronGorefiendDisableAttackingConstructsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendDisableAttackingConstructsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "teron gorefiend disable attacking constructs") {}

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

class ReliquaryOfSoulsDontWasteHealingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsDontWasteHealingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls don't waste healing") {}

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

// Lets a mage steal Rune Shield before a Purge or Tranquilizing Shot removes it.
class ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls let mages steal rune shield") {}

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

class IllidariCouncilControlHunterActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilControlHunterActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council control hunter actions") {}

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

class IllidanStormrageControlTankActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageControlTankActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage control tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageDisableDefaultTargetingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableDefaultTargetingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage disable default targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageControlNonTankMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageControlNonTankMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage control non-tank movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageUseEarthbindTotemMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageUseEarthbindTotemMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage use earthbind totem") {}

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

#endif
