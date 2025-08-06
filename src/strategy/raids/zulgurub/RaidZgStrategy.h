#ifndef _PLAYERBOT_RAIDZGSTRATEGY_H
#define _PLAYERBOT_RAIDZGSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"

class RaidZgStrategy : public Strategy
{
public:
    RaidZgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "zg"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;

private:
    void InitMandokirTriggers(std::vector<TriggerNode*>& triggers);
};

#endif
