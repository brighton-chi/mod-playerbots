#include "RaidHyjalSummitStrategy.h"
#include "RaidHyjalSummitMultipliers.h"

void RaidHyjalSummitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Rage Winterchill
    triggers.push_back(new TriggerNode("rage winterchill pulling boss",
        NextAction::array(0, new NextAction("rage winterchill misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("rage winterchill boss engaged by main tank",
        NextAction::array(0, new NextAction("rage winterchill main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("rage winterchill boss casts death and decay",
        NextAction::array(0, new NextAction("rage winterchill spread ranged in circle", ACTION_RAID + 1), nullptr)
    ));

    // Anetheron
    triggers.push_back(new TriggerNode("anetheron pulling boss",
        NextAction::array(0, new NextAction("anetheron misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron boss engaged by main tank",
        NextAction::array(0, new NextAction("anetheron main tank position boss", ACTION_RAID + 1), nullptr)
    ));

    // Kaz'rogal

    // Azgalor

    // Archimonde
}

void RaidHyjalSummitStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RageWinterchillDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new RageWinterchillDisableTankFaceMultiplier(botAI));
    multipliers.push_back(new AnetheronDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AnetheronDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new KazrogalDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new AzgalorDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ArchimondeDelayBloodlustAndHeroismMultiplier(botAI));
}
