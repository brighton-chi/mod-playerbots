#include "RaidMagsLairStrategy.h"
#include "RaidMagsLairMultipliers.h"

void RaidMagsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon", NextAction::array(0,
        new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10),
        new NextAction("magtheridon manticron cube clicker position", ACTION_RAID + 2),
        new NextAction("magtheridon position boss", ACTION_RAID + 1),
        new NextAction("magtheridon spread ranged", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler south tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler west tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler east tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler south warlock", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler west warlock", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler east warlock", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler west healer", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler east healer", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler west hunter", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler east hunter", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler dps priority", ACTION_RAID + 1),
        nullptr)));
}

void RaidMagsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonMultiplier(botAI));
}
