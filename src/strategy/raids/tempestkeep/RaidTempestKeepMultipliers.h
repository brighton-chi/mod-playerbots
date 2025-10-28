#ifndef _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H

#include "Multiplier.h"

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "void reaver maintain positions multiplier") {}
    virtual float GetValue(Action* action);
};

class HighAstromancerSolarianStayStackedMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianStayStackedMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian stay stacked multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
