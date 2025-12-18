#ifndef _PLAYERBOT_RAIDMAGTHERIDONMULTIPLIERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONMULTIPLIERS_H

#include "Multiplier.h"

class MagtheridonOnlyUseManticronCubeMultiplier : public Multiplier
{
public:
    MagtheridonOnlyUseManticronCubeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon only use manticron cube multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonWaitToAttackMultiplier : public Multiplier
{
public:
    MagtheridonWaitToAttackMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon wait to attack multiplier") {}
    float GetValue(Action* action) override;
};

class MagtheridonDisableTankActionsMultiplier : public Multiplier
{
public:
    MagtheridonDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "magtheridon disable tank actions multiplier") {}
    float GetValue(Action* action) override;
};

#endif
