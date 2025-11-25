#include "RaidSSCStrategy.h"
#include "RaidSSCMultipliers.h"

void RaidSSCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash Mobs
    triggers.push_back(new TriggerNode("underbog colossus spawned toxic pool after death",
        NextAction::array(0, new NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("greyheart tidecaller water elemental totem spawned",
        NextAction::array(0, new NextAction("greyheart tidecaller mark water elemental totem", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("rancid mushroom spawned",
        NextAction::array(0, new NextAction("rancid mushroom move away from mushroom spore cloud", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode("hydross the unstable bot is frost tank",
        NextAction::array(0, new NextAction("hydross the unstable position frost tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable bot is nature tank",
        NextAction::array(0, new NextAction("hydross the unstable position nature tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable elementals spawned",
        NextAction::array(0, new NextAction("hydross the unstable prioritize elemental adds", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable danger from water tombs",
        NextAction::array(0, new NextAction("hydross the unstable frost phase spread out", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable tank needs aggro upon phase change",
        NextAction::array(0, new NextAction("hydross the unstable misdirect boss to tank", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable aggro resets upon phase change",
        NextAction::array(0, new NextAction("hydross the unstable stop dps upon phase change", ACTION_EMERGENCY + 9), nullptr)
    ));
    triggers.push_back(new TriggerNode("hydross the unstable need to manage timers",
        NextAction::array(0, new NextAction("hydross the unstable manage timers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // The Lurker Below
    triggers.push_back(new TriggerNode("the lurker below spout is active",
        NextAction::array(0, new NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("the lurker below boss is active for main tank",
        NextAction::array(0, new NextAction("the lurker below position main tank", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("the lurker below boss casts geyser",
        NextAction::array(0, new NextAction("the lurker below spread ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("the lurker below need to prepare timer for spout",
        NextAction::array(0, new NextAction("the lurker below manage spout timer", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Leotheras the Blind
    triggers.push_back(new TriggerNode("leotheras the blind boss is inactive",
        NextAction::array(0, new NextAction("leotheras the blind target spellbinders", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind engaged by demon form tank",
        NextAction::array(0, new NextAction("leotheras the blind demon form tank attack boss", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind boss engaged by ranged",
        NextAction::array(0, new NextAction("leotheras the blind position ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind boss channeling whirlwind",
        NextAction::array(0, new NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind demon form engaged by melee without warlock tank",
        NextAction::array(0, new NextAction("leotheras the blind demon form position melee", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("leotheras the blind inner demon cheat",
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
    triggers.push_back(new TriggerNode("fathom-lord karathress caribdis tank needs dedicated healer",
        NextAction::array(0, new NextAction("fathom-lord karathress position caribdis tank healer", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress pulling bosses",
        NextAction::array(0, new NextAction("fathom-lord karathress misdirect bosses to tanks", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("fathom-lord karathress determining kill order",
        NextAction::array(0, new NextAction("fathom-lord karathress assign dps priority", ACTION_RAID + 1), nullptr)
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

    // Lady Vashj <Coilfang Matron>
    triggers.push_back(new TriggerNode("lady vashj boss engaged by main tank",
        NextAction::array(0, new NextAction("lady vashj main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj boss engaged by ranged in phase 1",
        NextAction::array(0, new NextAction("lady vashj phase 1 position ranged", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj casts shock blast on highest aggro",
        NextAction::array(0, new NextAction("lady vashj set grounding totem in main tank group", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj bot has static charge",
        NextAction::array(0, new NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj pulling boss in phase 1 and phase 3",
        NextAction::array(0, new NextAction("lady vashj misdirect boss to main tank", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj coilfang strider is approaching",
        NextAction::array(0,
            new NextAction("lady vashj misdirect strider to first assist tank", ACTION_EMERGENCY + 1),
            new NextAction("lady vashj tank attack and move away strider", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj tainted elemental cheat",
        NextAction::array(0,
            new NextAction("lady vashj teleport to tainted elemental", ACTION_EMERGENCY + 10),
            new NextAction("lady vashj loot tainted core", ACTION_EMERGENCY + 10), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj tainted core was looted",
        NextAction::array(0, new NextAction("lady vashj pass the tainted core", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj core handler is dead",
        NextAction::array(0, new NextAction("lady vashj destroy tainted core", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj determining kill order of adds",
        NextAction::array(0, new NextAction("lady vashj assign dps priority", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj toxic sporebats are spewing poison clouds",
        NextAction::array(0, new NextAction("lady vashj avoid toxic spores", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj bot is entangled in toxic spores or static charge",
        NextAction::array(0, new NextAction("lady vashj use free action abilities", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("lady vashj need to manage trackers",
        NextAction::array(0, new NextAction("lady vashj manage trackers", ACTION_EMERGENCY + 10), nullptr)
    ));
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new UnderbogColossusEscapeToxicPoolMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDpsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowStayAwayFromSpoutMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindMeleeTankMaintainDemonFormPositionMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDpsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAoeMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDpsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerDisablePhase2FleeActionMultiplier(botAI));
    multipliers.push_back(new LadyVashjDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new LadyVashjStaticChargeStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new LadyVashjDoNotLootTheTaintedCoreMultiplier(botAI));
    multipliers.push_back(new LadyVashjDisableAutomaticTargetingAndMovementModifier(botAI));
}
