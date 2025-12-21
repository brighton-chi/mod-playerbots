#include "RaidBlackTempleStrategy.h"
#include "RaidBlackTempleMultipliers.h"

void RaidBlackTempleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High Warlord Naj'entus
    triggers.push_back(new TriggerNode("high warlord naj'entus pulling boss",
        NextAction::array(0, new NextAction("high warlord naj'entus misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus boss engaged by main tank",
        NextAction::array(0, new NextAction("high warlord naj'entus main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus casts needle spines",
        NextAction::array(0, new NextAction("high warlord naj'entus disperse ranged", ACTION_RAID + 1), nullptr)
    ));

    // Supremus

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}

void RaidBlackTempleStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{

}
