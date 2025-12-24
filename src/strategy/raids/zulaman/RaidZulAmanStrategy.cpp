#include "RaidZulAmanStrategy.h"
#include "RaidZulAmanMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("amani'shi medicine man summoned ward",
        NextAction::array(0, new NextAction("amani'shi medicine man mark ward", ACTION_RAID + 1), nullptr)
    ));

    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akil'zon pulling boss",
        NextAction::array(0, new NextAction("akil'zon misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("akil'zon boss engaged by main tank",
        NextAction::array(0, new NextAction("akil'zon main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("akil'zon boss casts static disruption",
        NextAction::array(0, new NextAction("akil'zon spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("akil'zon electrical storm has formed",
        NextAction::array(0, new NextAction("akil'zon move to eye of the storm", ACTION_EMERGENCY + 6), nullptr)
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
    triggers.push_back(new TriggerNode("jan'alai pulling boss",
        NextAction::array(0, new NextAction("jan'alai misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("jan'alai boss engaged by main tank",
        NextAction::array(0, new NextAction("jan'alai main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("jan'alai boss casts flame breath",
        NextAction::array(0, new NextAction("jan'alai spread ranged in circle", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("jan'alai boss summoning fire bombs",
        NextAction::array(0, new NextAction("jan'alai move away from fire bombs", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("jan'alai amani hatchers spawned",
        NextAction::array(0, new NextAction("jan'alai mark amani hatchers", ACTION_RAID + 2), nullptr)
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

    // Zul'jin
    triggers.push_back(new TriggerNode("zul'jin main tank needs aggro upon pull or phase change",
        NextAction::array(0, new NextAction("zul'jin misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("zul'jin boss engaged by main tank",
        NextAction::array(0, new NextAction("zul'jin main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("zul'jin boss is channeling whirlwind in troll form",
        NextAction::array(0, new NextAction("zul'jin run away from whirlwind", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("zul'jin boss casts aoe abilities",
        NextAction::array(0, new NextAction("zul'jin spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("zul'jin boss is charging players in lynx form",
        NextAction::array(0, new NextAction("zul'jin move near group", ACTION_RAID + 1), nullptr)
    ));
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AkilzonDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AkilzonStayInEyeOfTheStormMultiplier(botAI));
    multipliers.push_back(new NalorakkDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new NalorakkControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new JanalaiDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new JanalaiDisableDispersalMultiplier(botAI));
    multipliers.push_back(new JanalaiStayAwayFromFireBombsMultiplier(botAI));
    multipliers.push_back(new JanalaiDoNotCrowdControlHatchersMultiplier(botAI));
    multipliers.push_back(new JanalaiDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new HalazziDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HalazziControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(botAI));
    multipliers.push_back(new ZuljinDisableTankFaceMultiplier(botAI));
    multipliers.push_back(new ZuljinAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new ZuljinDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ZuljinStayCloseToLynxFormMultiplier(botAI));
}
