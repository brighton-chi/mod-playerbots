/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H

#include "Multiplier.h"

// Trash
class ShadowmoonReaverDontBuildChaoticChargesMultiplier : public Multiplier
{
public:
    ShadowmoonReaverDontBuildChaoticChargesMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "shadowmoon reaver don't build chaotic charges multiplier") {}
    virtual float GetValue(Action* action);
};

// High Warlord Naj'entus
class HighWarlordNajentusDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    HighWarlordNajentusDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high warlord naj'entus disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

// Supremus
class SupremusFocusOnAvoidanceInPhase2Multiplier : public Multiplier
{
public:
    SupremusFocusOnAvoidanceInPhase2Multiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus focus on avoidance in phase 2 multiplier") {}
    virtual float GetValue(Action* action);
};

class SupremusHitboxIsBuggedMultiplier : public Multiplier
{
public:
    SupremusHitboxIsBuggedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus hitbox is bugged multiplier") {}
    virtual float GetValue(Action* action);
};

// Teron Gorefiend
class TeronGorefiendControlMovementMultiplier : public Multiplier
{
public:
    TeronGorefiendControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend control movement multiplier") {}
    virtual float GetValue(Action* action);
};

class TeronGorefiendMarkedBotOnlyMoveToDieMultiplier : public Multiplier
{
public:
    TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend marked bot only move to die multiplier") {}
    virtual float GetValue(Action* action);
};

class TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier : public Multiplier
{
public:
    TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend spirits attack only shadowy constructs multiplier") {}
    virtual float GetValue(Action* action);
};

class TeronGorefiendDisableAttackingConstructsMultiplier : public Multiplier
{
public:
    TeronGorefiendDisableAttackingConstructsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend disable attacking constructs multiplier") {}
    virtual float GetValue(Action* action);
};

// Gurtogg Bloodboil

class GurtoggBloodboilControlMovementMultiplier : public Multiplier
{
public:
    GurtoggBloodboilControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gurtogg bloodboil control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// Reliquary of Souls

class ReliquaryOfSoulsDontWasteHealingMultiplier : public Multiplier
{
public:
    ReliquaryOfSoulsDontWasteHealingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "reliquary of souls don't waste healing multiplier") {}
    virtual float GetValue(Action* action);
};

// Mother Shahraz

class MotherShahrazControlMovementMultiplier : public Multiplier
{
public:
    MotherShahrazControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz control movement multiplier") {}
    virtual float GetValue(Action* action);
};

class MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier : public Multiplier
{
public:
    MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz bots with fatal attraction only run away multiplier") {}
    virtual float GetValue(Action* action);
};

class MotherShahrazDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    MotherShahrazDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

// Illidari Council

class IllidariCouncilDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    IllidariCouncilDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilDisableTankActionsMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilControlMovementMultiplier : public Multiplier
{
public:
    IllidariCouncilControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council control movement multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilControlMisdirectionMultiplier : public Multiplier
{
public:
    IllidariCouncilControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council control misdirection multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilDisableArcaneShotOnZerevorMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableArcaneShotOnZerevorMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable arcane shot on zerevor multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilDisableIceBlockMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableIceBlockMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable ice block multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilWaitForDpsMultiplier : public Multiplier
{
public:
    IllidariCouncilWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageDelayCooldownsMultiplier : public Multiplier
{
public:
    IllidanStormrageDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage delay cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidanStormrageControlTankActionsMultiplier : public Multiplier
{
public:
    IllidanStormrageControlTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage control tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidanStormrageDisableDefaultTargetingMultiplier : public Multiplier
{
public:
    IllidanStormrageDisableDefaultTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage disable default targeting multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidanStormrageControlNonTankMovementMultiplier : public Multiplier
{
public:
    IllidanStormrageControlNonTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage control non-tank movement multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidanStormrageWaitForDpsMultiplier : public Multiplier
{
public:
    IllidanStormrageWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
