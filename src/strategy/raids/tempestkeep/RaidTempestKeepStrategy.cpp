#include "RaidTempestKeepStrategy.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Void Reaver
    triggers.push_back(new TriggerNode(
        "void reaver position boss",
        NextAction::array(0, new NextAction("void reaver position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "void reaver spread ranged",
        NextAction::array(0, new NextAction("void reaver spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "void reaver arcane orb move away",
        NextAction::array(0, new NextAction("void reaver arcane orb move away", ACTION_EMERGENCY + 6), nullptr)
    ));
    
    // High Astromancer Solarian
    triggers.push_back(new TriggerNode(
        "high astromancer solarian stack ranged",
        NextAction::array(0, new NextAction("high astromancer solarian stack ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian move away from group",
        NextAction::array(0, new NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian stack for aoe",
        NextAction::array(0, new NextAction("high astromancer solarian stack for aoe", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian target priest adds",
        NextAction::array(0, new NextAction("high astromancer solarian target priest adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian tank voidwalker",
        NextAction::array(0, new NextAction("high astromancer solarian tank voidwalker", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "high astromancer solarian cast fear ward on main tank",
        NextAction::array(0, new NextAction("high astromancer solarian cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighAstromancerSolarianStayStackedMultiplier(botAI));
}
