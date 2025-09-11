#ifndef _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H
#define _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H

#include "Multiplier.h"

class KarazhanShadeOfAranMultiplier : public Multiplier
{
public:
    KarazhanShadeOfAranMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan shade of aran") {}
    virtual float GetValue(Action* action);
};

class KarazhanPrinceMalchezaarMultiplier : public Multiplier
{
public:
    KarazhanPrinceMalchezaarMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan prince malchezaar") {}
    virtual float GetValue(Action* action);
};

#endif