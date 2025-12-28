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
    triggers.push_back(new TriggerNode("high warlord naj'entus player is impaled",
        NextAction::array(0, new NextAction("high warlord naj'entus remove impaling spine", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high warlord naj'entus boss has tidal shield",
        NextAction::array(0, new NextAction("high warlord naj'entus throw impaling spine", ACTION_RAID + 2), nullptr)
    ));

    // Supremus
    triggers.push_back(new TriggerNode("supremus pulling boss or changing phase",
        NextAction::array(0, new NextAction("supremus misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    /* triggers.push_back(new TriggerNode("supremus boss engaged by main tank",
        NextAction::array(0, new NextAction("supremus main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus boss engaged by ranged",
        NextAction::array(0, new NextAction("supremus spread ranged in arc", ACTION_RAID + 1), nullptr)
    )); */
    triggers.push_back(new TriggerNode("supremus boss engaged by ranged",
        NextAction::array(0, new NextAction("supremus disperse ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus boss is fixated on bot",
        NextAction::array(0, new NextAction("supremus kite boss", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("supremus changes phase every 60 seconds",
        NextAction::array(0, new NextAction("supremus manage phase timer", ACTION_EMERGENCY + 10), nullptr)
    ));

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
    // High Warlord Naj'entus

    // Supremus
    multipliers.push_back(new SupremusKiteBossMultiplier(botAI));

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}
