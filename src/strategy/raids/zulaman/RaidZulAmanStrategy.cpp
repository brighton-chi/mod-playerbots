#include "RaidZulAmanStrategy.h"
#include "RaidZulAmanMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akilzon boss casts static disruption",
        NextAction::array(0, new NextAction("akilzon spread ranged", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("akilzon electrical storm has formed",
        NextAction::array(0, new NextAction("akilzon move to eye of the storm", ACTION_EMERGENCY + 2), nullptr)
    ));
    // Nalorakk <Bear Avatar>

    // Jan'alai <Dragonhawk Avatar>

    // Halazzi <Lynx Avatar>

    // Hex Lord Malacrass

    // Zul'jin
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // multipliers.push_back(new UnderbogColossusEscapeToxicPoolMultiplier(botAI));
}
