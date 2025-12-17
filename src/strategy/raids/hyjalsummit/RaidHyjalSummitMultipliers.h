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

class RageWinterchillDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    RageWinterchillDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

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

class AnetheronDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    AnetheronDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

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

class KazrogalDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    KazrogalDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

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

class AzgalorDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    AzgalorDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

// Archimonde

class ArchimondeDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    ArchimondeDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "archimonde delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
