#include "RaidSSCStrategy.h"
#include "RaidSSCMultipliers.h"

void RaidSSCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode(
        "hydross the unstable waiting for dps",
        NextAction::array(0, new NextAction("hydross the unstable manage dps timer", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable boss in frost phase",
        NextAction::array(0, new NextAction("hydross the unstable frost tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable boss transitioning to nature phase",
        NextAction::array(0, new NextAction("hydross the unstable frost tank move boss to nature tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable boss in nature phase",
        NextAction::array(0, new NextAction("hydross the unstable nature tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable boss transitioning to frost phase",
        NextAction::array(0, new NextAction("hydross the unstable nature tank move boss to frost tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable elementals spawned",
        NextAction::array(0, new NextAction("hydross the unstable mark elemental adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable danger from water tombs",
        NextAction::array(0, new NextAction("hydross the unstable frost phase spread out", ACTION_EMERGENCY + 1), nullptr)
    ));
    // The Lurker Below
    // Leotheras the Blind
    // Fathom-Lord Karathress
    // Morogrim Tidewalker
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HydrossTheUnstableDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDPSMultiplier(botAI));
}
