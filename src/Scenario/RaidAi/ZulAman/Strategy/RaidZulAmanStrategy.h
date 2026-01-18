#ifndef _PLAYERBOT_RAIDZULAMANSTRATEGY_H_
#define _PLAYERBOT_RAIDZULAMANSTRATEGY_H_

#include "Strategy.h"
#include "Multiplier.h"

class RaidZulAmanStrategy : public Strategy
{
public:
    RaidZulAmanStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "zulaman"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
