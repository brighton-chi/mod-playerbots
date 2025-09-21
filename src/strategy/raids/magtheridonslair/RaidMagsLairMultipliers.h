#ifndef _PLAYERBOT_RAIDMAGSLAIRMULTIPLIERS_H
#define _PLAYERBOT_RAIDMAGSLAIRMULTIPLIERS_H

#include "Multiplier.h"

class MagtheridonMultiplier : public Multiplier
{
public:
    MagtheridonMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
