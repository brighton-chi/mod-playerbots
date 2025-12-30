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
class TeronGorefiendDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    TeronGorefiendDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

class TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier : public Multiplier
{
public:
    TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend spirits attack only shadowy constructs multiplier") {}
    virtual float GetValue(Action* action);
};

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
