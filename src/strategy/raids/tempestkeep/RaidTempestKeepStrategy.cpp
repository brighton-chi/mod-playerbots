#include "RaidTempestKeepStrategy.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("crimson hand centurion casts arcane volley",
        NextAction::array(0. new NextAction("crimson hand centurion cast polymorph", ACTION_RAID + 1), nullptr)
    ));
    // Al'ar
    triggers.push_back(new TriggerNode("alar need to log info to develop boss strategy",
        NextAction::array(0, new NextAction("alar log debug info", ACTION_EMERGENCY + 11), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar pulling boss",
        NextAction::array(0, new NextAction("alar misdirect boss to main tank", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar engaged by tanks in phase 1",
        NextAction::array(0, new NextAction("alar boss tanks move between platforms", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by melee dps",
        NextAction::array(0, new NextAction("alar melee dps prioritize adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by ranged dps",
        NextAction::array(0, new NextAction("alar ranged dps prioritize boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar boss engaged by healer",
        NextAction::array(0, new NextAction("alar position healer", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar embers of alar spawned",
        NextAction::array(0, new NextAction("alar add tank pick up embers", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar incoming flame quills",
        NextAction::array(0, new NextAction("alar jump from platform", ACTION_EMERGENCY + 9), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar rising from the ashes",
        NextAction::array(0, new NextAction("alar move away from rebirth", ACTION_EMERGENCY + 9), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar engaged by tanks in phase 2",
        NextAction::array(0, new NextAction("alar swap tanks on boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar preparing to dive bomb",
        NextAction::array(0, new NextAction("alar dive bomb spread", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("alar need to manage timers and trackers",
        NextAction::array(0, new NextAction("alar manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Void Reaver
    triggers.push_back(new TriggerNode("void reaver boss engaged by tank",
        NextAction::array(0, new NextAction("void reaver position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("void reaver boss engaged by ranged",
        NextAction::array(0, new NextAction("void reaver spread ranged", ACTION_RAID + 1), nullptr)
    ));

    // High Astromancer Solarian
    triggers.push_back(new TriggerNode("high astromancer solarian phase 1 and 2 movement",
        NextAction::array(0, new NextAction("high astromancer solarian stack bots", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian bot has wrath of the astromancer",
        NextAction::array(0, new NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian solarium priests spawned",
        NextAction::array(0, new NextAction("high astromancer solarian target solarium priests", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian transformed into voidwalker",
        NextAction::array(0, new NextAction("high astromancer solarian tank voidwalker", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("high astromancer solarian can cast fear ward on main tank",
        NextAction::array(0, new NextAction("high astromancer solarian cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AlarPhase1StickToTheScriptMultiplier(botAI));
    multipliers.push_back(new AlarStayAwayFromRebirthMultiplier(botAI));
    multipliers.push_back(new AlarPhase2NoTankingIfArmorMeltedMultiplier(botAI));
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianStayStackedMultiplier(botAI));
}
