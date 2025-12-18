#ifndef _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H
#define _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H

#include "Multiplier.h"

class HyjalSummitTimeBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    HyjalSummitTimeBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hyjal summit time bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

// Rage Winterchill

class RageWinterchillDisableMainTankAvoidAoeMultiplier : public Multiplier
{
public:
    RageWinterchillDisableMainTankAvoidAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill disable main tank avoid aoe multiplier") {}
    virtual float GetValue(Action* action);
};

class RageWinterchillDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    RageWinterchillDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

// Anetheron

class AnetheronDisableTankActionsMultiplier : public Multiplier
{
public:
    AnetheronDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class AnetheronDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AnetheronDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

class AnetheronInfernalTankMaintainPositionMultiplier : public Multiplier
{
public:
    AnetheronInfernalTankMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron infernal tank maintain position multiplier") {}
    virtual float GetValue(Action* action);
};

class AnetheronControlMisdirectionMultiplier : public Multiplier
{
public:
    AnetheronControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron control misdirection multiplier") {}
    virtual float GetValue(Action* action);
};

// Kaz'rogal

class KazrogalLowManaBotStayAwayFromGroupMultiplier : public Multiplier
{
public:
    KazrogalLowManaBotStayAwayFromGroupMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal low mana bot stay away from group multiplier") {}
    virtual float GetValue(Action* action);
};

// Azgalor

class AzgalorDisableTankAssistMultiplier : public Multiplier
{
public:
    AzgalorDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class AzgalorTanksMaintainPositionMultiplier : public Multiplier
{
public:
    AzgalorTanksMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor tanks maintain position multiplier") {}
    virtual float GetValue(Action* action);
};

// Archimonde

#endif
