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
        "hydross the unstable bot is frost tank",
        NextAction::array(0, new NextAction("hydross the unstable position frost tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "hydross the unstable bot is nature tank",
        NextAction::array(0, new NextAction("hydross the unstable position nature tank", ACTION_RAID + 1), nullptr)
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
    triggers.push_back(new TriggerNode(
        "morogrim tidewalker boss engaged by main tank",
        NextAction::array(0, new NextAction("morogrim tidewalker move boss to tank position", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "morogrim tidewalker pulling boss",
        NextAction::array(0, new NextAction("morogrim tidewalker misdirect boss to main tank", ACTION_RAID + 1), nullptr)
    ));
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HydrossTheUnstableDisableTankAssistMultiplier(botAI));
    // multipliers.push_back(new HydrossTheUnstableWaitForDPSMultiplier(botAI));
}
