#include "RaidSSCStrategy.h"
#include "RaidSSCMultipliers.h"

void RaidSSCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash Mobs
    triggers.push_back(new TriggerNode("greyheart tidecaller water elemental totem spawned",
        NextAction::array(0, new NextAction("greyheart tidecaller mark water elemental totem", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("rancid mushroom spawned",
        NextAction::array(0, new NextAction("rancid mushroom move away from mushroom spore cloud", ACTION_EMERGENCY + 1), nullptr)
    ));

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode("hydross the unstable bot is frost tank",
        NextAction::array(0, new NextAction("hydross the unstable position frost tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable bot is nature tank",
        NextAction::array(0, new NextAction("hydross the unstable position nature tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable elementals spawned",
        NextAction::array(0, new NextAction("hydross the unstable mark elemental adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable danger from water tombs",
        NextAction::array(0, new NextAction("hydross the unstable frost phase spread out", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable tank needs aggro upon phase change",
        NextAction::array(0, new NextAction("hydross the unstable misdirect boss to tank", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable need to transition before fourth mark",
        NextAction::array(0, new NextAction("hydross the unstable manage phase change timers", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable waiting for dps",
        NextAction::array(0, new NextAction("hydross the unstable manage dps timers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // The Lurker Below
    triggers.push_back(new TriggerNode("the lurker below spout is active",
        NextAction::array(0, new NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6), nullptr)
    ));

    // Leotheras the Blind
    triggers.push_back(new TriggerNode("leotheras the blind boss is inactive",
        NextAction::array(0, new NextAction("leotheras the blind target spellbinders", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind demon form engaged by demon form tank",
        NextAction::array(0, new NextAction("leotheras the blind demon form position boss", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind boss engaged by ranged",
        NextAction::array(0, new NextAction("leotheras the blind position ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind boss channeling whirlwind",
        NextAction::array(0, new NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1), nullptr)
    ));
    /* triggers.push_back(new TriggerNode("leotheras the blind demon form engaged by melee",
        NextAction::array(0, new NextAction("leotheras the blind demon form position melee", ACTION_EMERGENCY + 1), nullptr)
    )); */
    triggers.push_back(new TriggerNode("leotheras the blind inner demon has taken form",
        NextAction::array(0, new NextAction("leotheras the blind inner demon cheat", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind entered final phase",
        NextAction::array(0, new NextAction("leotheras the blind final phase assign dps priority", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind demon form tank needs aggro",
        NextAction::array(0, new NextAction("leotheras the blind misdirect boss to demon form tank", ACTION_RAID + 3), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind need to manage timers and trackers",
        NextAction::array(0, new NextAction("leotheras the blind manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Fathom-Lord Karathress
    triggers.push_back(new TriggerNode("fathom-lord karathress boss engaged by main tank",
        NextAction::array(0, new NextAction("fathom-lord karathress main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress sharkkis engaged by first assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress first assist tank position sharkkis", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress tidalvess engaged by second assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress second assist tank position tidalvess", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress caribdis engaged by third assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress third assist tank position caribdis", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress pulling bosses",
        NextAction::array(0, new NextAction("fathom-lord karathress misdirect bosses to tanks", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress determining kill order",
        NextAction::array(0, new NextAction("fathom-lord karathress assign dps priority", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress caribdis tank needs dedicated healer",
        NextAction::array(0, new NextAction("fathom-lord karathress position caribdis tank healer", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress tanks need to establish aggro",
        NextAction::array(0, new NextAction("fathom-lord karathress manage dps timer", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Morogrim Tidewalker
    triggers.push_back(new TriggerNode("morogrim tidewalker boss engaged by main tank",
        NextAction::array(0, new NextAction("morogrim tidewalker move boss to tank position", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("morogrim tidewalker water globules are incoming",
        NextAction::array(0, new NextAction("morogrim tidewalker phase 2 reposition ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("morogrim tidewalker pulling boss",
        NextAction::array(0, new NextAction("morogrim tidewalker misdirect boss to main tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("morogrim tidewalker encounter reset",
        NextAction::array(0, new NextAction("morogrim tidewalker reset phase transition steps", ACTION_RAID + 2), nullptr)
    ));

    // Lady Vashj
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HydrossTheUnstableDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDPSMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowStayBehindBossDuringSpoutMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    // multipliers.push_back(new LeotherasTheBlindMainTankMaintainDemonFormPositionMultiplier(botAI));
    // multipliers.push_back(new LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDPSMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDPSMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(botAI));
}
