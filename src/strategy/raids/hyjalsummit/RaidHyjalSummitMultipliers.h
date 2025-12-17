#ifndef _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H
#define _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H

#include "Multiplier.h"

// Rage Winterchill

class RageWinterchillDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    RageWinterchillDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

class RageWinterchillDisableTankFaceMultiplier : public Multiplier
{
public:
    RageWinterchillDisableTankFaceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill disable tank face multiplier") {}
    virtual float GetValue(Action* action);
};

// Anetheron

class AnetheronDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AnetheronDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

// Kaz'rogal

// Azgalor

// Archimonde


#endif
