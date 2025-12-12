#include "RaidZulAmanStrategy.h"
#include "RaidZulAmanMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("amanishi medicine man summoned ward",
        NextAction::array(0, new NextAction("amanishi medicine man mark ward", ACTION_RAID + 1), nullptr)
    ));

    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akilzon pulling boss",
        NextAction::array(0, new NextAction("akilzon misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("akilzon boss engaged by main tank",
        NextAction::array(0, new NextAction("akilzon main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("akilzon boss casts static disruption",
        NextAction::array(0, new NextAction("akilzon spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("akilzon electrical storm has formed",
        NextAction::array(0, new NextAction("akilzon move to eye of the storm", ACTION_EMERGENCY + 2), nullptr)
    ));

    // Nalorakk <Bear Avatar>
    triggers.push_back(new TriggerNode("nalorakk pulling boss",
        NextAction::array(0, new NextAction("nalorakk misdirect boss to main tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("nalorakk boss switches forms",
        NextAction::array(0, new NextAction("nalorakk tanks position boss", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("nalorakk boss casts surge",
        NextAction::array(0, new NextAction("nalorakk spread ranged", ACTION_RAID + 1), nullptr)
    ));

    // Jan'alai <Dragonhawk Avatar>
    triggers.push_back(new TriggerNode("janalai pulling boss",
        NextAction::array(0, new NextAction("janalai misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("janalai boss engaged by main tank",
        NextAction::array(0, new NextAction("janalai main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("janalai boss casts flame breath",
        NextAction::array(0, new NextAction("janalai spread ranged in circle", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("janalai boss summoning fire bombs",
        NextAction::array(0, new NextAction("janalai move away from fire bombs", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("janalai amani hatchers spawned",
        NextAction::array(0, new NextAction("janalai mark amani hatchers", ACTION_RAID + 2), nullptr)
    ));

    // Halazzi <Lynx Avatar>
    triggers.push_back(new TriggerNode("halazzi pulling boss",
        NextAction::array(0, new NextAction("halazzi misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("halazzi boss engaged by main tank",
        NextAction::array(0, new NextAction("halazzi main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("halazzi boss summons spirit lynx",
        NextAction::array(0, new NextAction("halazzi first assist tank attack spirit lynx", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("halazzi determining dps target",
        NextAction::array(0, new NextAction("halazzi assign dps priority", ACTION_RAID + 1), nullptr)
    ));

    // Hex Lord Malacrass
    triggers.push_back(new TriggerNode("hex lord malacrass pulling boss",
        NextAction::array(0, new NextAction("hex lord malacrass misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("hex lord malacrass determining kill order",
        NextAction::array(0, new NextAction("hex lord malacrass assign dps priority", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hex lord malacrass boss has siphoned buff",
        NextAction::array(0, new NextAction("hex lord malacrass purge buff from boss", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hex lord malacrass party member is mind controlled",
        NextAction::array(0, new NextAction("hex lord malacrass dispel mind control", ACTION_EMERGENCY + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("hex lord malacrass all adds are down",
        NextAction::array(0, new NextAction("hex lord malacrass main tank position boss", ACTION_RAID + 1), nullptr)
    ));

    // Zul'jin
    triggers.push_back(new TriggerNode("zuljin main tank needs aggro upon pull or phase change",
        NextAction::array(0, new NextAction("zuljin misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("zuljin boss engaged by main tank",
        NextAction::array(0, new NextAction("zuljin main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("zuljin boss is channeling whirlwind in troll form",
        NextAction::array(0, new NextAction("zuljin run away from whirlwind", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("zuljin boss casts aoe abilities",
        NextAction::array(0, new NextAction("zuljin spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("zuljin boss is charging players in lynx form",
        NextAction::array(0, new NextAction("zuljin move near group", ACTION_RAID + 1), nullptr)
    ));
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AkilzonStayInEyeOfTheStormMultiplier(botAI));
    multipliers.push_back(new NalorakkDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new NalorakkControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new JanalaiDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new JanalaiStayAwayFromFireBombsMultiplier(botAI));
    multipliers.push_back(new JanalaiDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new HalazziDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HalazziControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(botAI));
    multipliers.push_back(new ZuljinAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new ZuljinDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ZuljinDoNotAvoidCyclonesMultiplier(botAI));
    multipliers.push_back(new ZuljinStayCloseToLynxFormMultiplier(botAI));
}
