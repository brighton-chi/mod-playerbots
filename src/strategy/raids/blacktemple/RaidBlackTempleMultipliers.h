#ifndef _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H

#include "Multiplier.h"

// High Warlord Naj'entus
// N/A

// Supremus
class SupremusFocusOnAvoidanceInPhase2Multiplier : public Multiplier
{
public:
    SupremusFocusOnAvoidanceInPhase2Multiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus focus on avoidance in phase 2 multiplier") {}
    virtual float GetValue(Action* action);
};

// Shade of Akama

// Teron Gorefiend
class TeronGorefiendDisableMovementMultiplier : public Multiplier
{
public:
    TeronGorefiendDisableMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend disable movement multiplier") {}
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

class GurtoggBloodboilDisableMovementMultiplier : public Multiplier
{
public:
    GurtoggBloodboilDisableMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gurtogg bloodboil disable movement multiplier") {}
    virtual float GetValue(Action* action);
};

// Reliquary of Souls

class ReliquaryOfSoulsWaitForDpsMultiplier : public Multiplier
{
public:
    ReliquaryOfSoulsWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "reliquary of souls wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class ReliquaryOfSoulsDontWasteHealingMultiplier : public Multiplier
{
public:
    ReliquaryOfSoulsDontWasteHealingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "reliquary of souls don't waste healing multiplier") {}
    virtual float GetValue(Action* action);
};

class ReliquaryOfSoulsDontInterruptDeadenIfReflectableMultiplier : public Multiplier
{
public:
    ReliquaryOfSoulsDontInterruptDeadenIfReflectableMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "reliquary of souls don't interrupt deaden if reflectable multiplier") {}
    virtual float GetValue(Action* action);
};

// Mother Shahraz

class MotherShahrazDisableMovementMultiplier : public Multiplier
{
public:
    MotherShahrazDisableMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz disable movement multiplier") {}
    virtual float GetValue(Action* action);
};

class MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier : public Multiplier
{
public:
    MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz bots with fatal attraction only run away multiplier") {}
    virtual float GetValue(Action* action);
};

// Illidari Council

class IllidariCouncilDisableTankActionsMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class IllidariCouncilDisableAoeMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable aoe multiplier") {}
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

class IllidariCouncilManageInterruptsMultiplier : public Multiplier
{
public:
    IllidariCouncilManageInterruptsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council manage interrupts multiplier") {}
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

#endif
