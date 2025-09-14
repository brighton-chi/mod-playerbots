#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("high king maulgar", NextAction::array(0,
        new NextAction("high king maulgar avoid blast wave", ACTION_EMERGENCY + 8),
        new NextAction("high king maulgar avoid whirlwind", ACTION_EMERGENCY + 7),
        new NextAction("high king maulgar mage tank", ACTION_EMERGENCY + 6),
        new NextAction("high king maulgar control felstalker", ACTION_RAID + 3),
        new NextAction("high king maulgar hunter misdirection", ACTION_RAID + 2),
        new NextAction("high king maulgar blindeye tank", ACTION_RAID + 1),
        new NextAction("high king maulgar first off tank", ACTION_RAID + 1),
        new NextAction("high king maulgar main tank", ACTION_RAID + 1),
        new NextAction("high king maulgar dps priority", ACTION_RAID + 1),
        nullptr)));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarMultiplier(botAI));
    // multipliers.push_back(new GruulTheDragonkillerMultiplier(botAI));
}
