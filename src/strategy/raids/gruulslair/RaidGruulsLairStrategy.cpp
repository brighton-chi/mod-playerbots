#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

#include "Log.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("high king maulgar", NextAction::array(0,
        //new NextAction("high king maulgar avoid blast wave", ACTION_EMERGENCY + 8),
        //new NextAction("high king maulgar avoid whirlwind", ACTION_EMERGENCY + 7),
        //new NextAction("high king maulgar control felstalker", ACTION_RAID + 3),
        new NextAction("high king maulgar hunter misdirection", ACTION_RAID + 2),
        new NextAction("high king maulgar maulgar tank", ACTION_RAID + 1),
        new NextAction("high king maulgar olm tank", ACTION_RAID + 1),
        new NextAction("high king maulgar blindeye tank", ACTION_RAID + 1),
        new NextAction("high king maulgar mage tank", ACTION_RAID + 1),
        new NextAction("high king maulgar boomkin tank", ACTION_RAID + 1),
        new NextAction("high king maulgar melee dps", ACTION_RAID + 1),
        new NextAction("high king maulgar ranged dps", ACTION_RAID + 1),
        nullptr)));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarMultiplier(botAI));
    // multipliers.push_back(new GruulTheDragonkillerMultiplier(botAI));
    // melee other than tanks at max hitrange; what about spread?
}   // ranged spread - 20.0f?
    // disperse upon shatter
