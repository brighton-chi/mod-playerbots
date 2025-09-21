#include "RaidMagsLairStrategy.h"
#include "RaidMagsLairMultipliers.h"

void RaidMagsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon", NextAction::array(0,
        new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10),
        new NextAction("magtheridon manticron cube clicker position", ACTION_RAID + 2),
        new NextAction("magtheridon position boss", ACTION_RAID + 1),
        new NextAction("magtheridon spread ranged", ACTION_RAID + 1),
        nullptr)));
}

void RaidMagsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonMultiplier(botAI));
}
