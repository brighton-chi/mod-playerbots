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

// General

class BlackTempleEncounterMultiplier : public Multiplier
{
public:
    BlackTempleEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, BlackTempleHelpers::BLACK_TEMPLE_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
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

class SupremusFocusOnAvoidanceInPhase2Multiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusFocusOnAvoidanceInPhase2Multiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus focus on avoidance in phase 2") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SupremusHitboxIsBuggedMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusHitboxIsBuggedMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "supremus hitbox is bugged") {}

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

class TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "teron gorefiend spirits attack only shadowy constructs") {}

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

// Reliquary of Souls

class ReliquaryOfSoulsDontWasteHealingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsDontWasteHealingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "reliquary of souls don't waste healing") {}

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

class MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier : public BlackTempleEncounterMultiplier
{
public:
    MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "mother shahraz bots with fatal attraction only run away") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Illidari Council

class IllidariCouncilDisableTankActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilControlMisdirectionMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilDisableArcaneShotOnZerevorMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableArcaneShotOnZerevorMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council disable arcane shot on zerevor") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilDisableIceBlockMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableIceBlockMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidari council disable ice block") {}

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

class IllidanStormrageDelayDpsCooldownsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "illidan stormrage delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

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
