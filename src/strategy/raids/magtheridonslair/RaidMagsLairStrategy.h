#ifndef _PLAYERBOT_RAIDMAGSLAIRSTRATEGY_H
#define _PLAYERBOT_RAIDMAGSLAIRSTRATEGY_H

#include "Strategy.h"
#include "Multiplier.h"

class RaidMagsLairStrategy : public Strategy
{
public:
    RaidMagsLairStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "magtheridonslair"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
