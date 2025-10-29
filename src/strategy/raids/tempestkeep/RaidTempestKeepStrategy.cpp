#include "RaidTempestKeepStrategy.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Al'ar
    triggers.push_back(new TriggerNode(
        "alar incoming flame quills",
        NextAction::array(0, new NextAction("alar jump from platform", ACTION_EMERGENCY + 9), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "alar need to manage timers and trackers",
        NextAction::array(0, new NextAction("alar manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Void Reaver
    triggers.push_back(new TriggerNode(
        "void reaver boss engaged by tank",
        NextAction::array(0, new NextAction("void reaver position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "void reaver boss engaged by ranged",
        NextAction::array(0, new NextAction("void reaver spread ranged", ACTION_RAID + 1), nullptr)
    ));
    
    // High Astromancer Solarian
    triggers.push_back(new TriggerNode(
        "high astromancer solarian phase 1 and 2 movement",
        NextAction::array(0, new NextAction("high astromancer solarian stack bots", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian bot has wrath of the astromancer",
        NextAction::array(0, new NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian solarium priests spawned",
        NextAction::array(0, new NextAction("high astromancer solarian target solarium priests", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian transformed into voidwalker",
        NextAction::array(0, new NextAction("high astromancer solarian tank voidwalker", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian can cast fear ward on main tank",
        NextAction::array(0, new NextAction("high astromancer solarian cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianStayStackedMultiplier(botAI));
}
