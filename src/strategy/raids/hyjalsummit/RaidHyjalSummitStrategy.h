#ifndef _PLAYERBOT_RAIDHYJALSUMMITSTRATEGY_H_
#define _PLAYERBOT_RAIDHYJALSUMMITSTRATEGY_H_

#include "Strategy.h"
#include "Multiplier.h"

class RaidHyjalSummitStrategy : public Strategy
{
public:
    RaidHyjalSummitStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "hyjalsummit"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
