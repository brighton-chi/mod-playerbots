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
        NextAction::array(0, new NextAction("anetheron misdirect boss to main tank", ACTION_RAID + 3), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron boss engaged by main tank",
        NextAction::array(0, new NextAction("anetheron main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron boss casts carrion swarm",
        NextAction::array(0, new NextAction("anetheron spread ranged in arc", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron bot is targeted by infernal",
        NextAction::array(0, new NextAction("anetheron bring infernal to infernal tank", ACTION_EMERGENCY + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron infernals need to be kept away from raid",
        NextAction::array(0, new NextAction("anetheron first assist tank pick up infernals", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("anetheron infernals despawn when boss dies",
        NextAction::array(0, new NextAction("anetheron focus all dps on boss", ACTION_RAID + 1), nullptr)
    ));

    // Kaz'rogal
    triggers.push_back(new TriggerNode("kaz'rogal pulling boss",
        NextAction::array(0, new NextAction("kaz'rogal misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaz'rogal boss engaged by main tank",
        NextAction::array(0, new NextAction("kaz'rogal main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaz'rogal boss engaged by assist tanks",
        NextAction::array(0, new NextAction("kaz'rogal assist tanks move in front of boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaz'rogal low mana bots need escape path",
        NextAction::array(0, new NextAction("kaz'rogal spread ranged in arc", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaz'rogal bot is low on mana",
        NextAction::array(0, new NextAction("kaz'rogal low mana bot move from group", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("kaz'rogal mage or paladin has mark of kaz'rogal",
        NextAction::array(0, new NextAction("kaz'rogal remove mark of kaz'rogal", ACTION_EMERGENCY + 6), nullptr)
    ));

    // Azgalor
    triggers.push_back(new TriggerNode("azgalor pulling boss",
        NextAction::array(0, new NextAction("azgalor misdirect boss to main tank", ACTION_RAID + 3), nullptr)
    ));
    triggers.push_back(new TriggerNode("azgalor boss engaged by main tank",
        NextAction::array(0, new NextAction("azgalor main tank position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("azgalor boss casts rain of fire",
        NextAction::array(0, new NextAction("azgalor disperse ranged", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("azgalor bot is doomed",
        NextAction::array(0,
            new NextAction("azgalor move to doomguard tank", ACTION_EMERGENCY + 2),
            new NextAction("azgalor first assist tank position doomguard", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("azgalor doomguard spawned",
        NextAction::array(0,
            new NextAction("azgalor melee dps prioritize doomguards", ACTION_RAID + 1),
            new NextAction("azgalor first assist tank position doomguard", ACTION_RAID + 1), nullptr)
    ));

    // Archimonde
    triggers.push_back(new TriggerNode("archimonde pulling boss",
        NextAction::array(0, new NextAction("archimonde misdirect boss to main tank", ACTION_RAID + 2), nullptr)
    ));
    /* triggers.push_back(new TriggerNode("archimonde boss engaged by main tank",
        NextAction::array(0, new NextAction("archimonde main tank position boss", ACTION_RAID + 1), nullptr)
    )); */
    triggers.push_back(new TriggerNode("archimonde boss casts fear",
        NextAction::array(0, new NextAction("archimonde cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("archimonde boss casts air burst",
        NextAction::array(0, new NextAction("archimonde disperse ranged bots", ACTION_RAID + 1), nullptr)
    ));
    /* triggers.push_back(new TriggerNode("archimonde boss summoned doomfire",
        NextAction::array(0, new NextAction("archimonde avoid doomfire", ACTION_EMERGENCY + 6), nullptr)
    )); */
    triggers.push_back(new TriggerNode("archimonde bot stood in doomfire",
        NextAction::array(0, new NextAction("archimonde remove doomfire dot", ACTION_EMERGENCY + 6), nullptr)
    ));
}

void RaidHyjalSummitStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HyjalSummitTimeBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new RageWinterchillDisableMainTankAvoidAoeMultiplier(botAI));
    multipliers.push_back(new RageWinterchillDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AnetheronDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new AnetheronDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new AnetheronInfernalTankMaintainPositionMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KazrogalLowManaBotStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new AzgalorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AzgalorTanksMaintainPositionMultiplier(botAI));
    multipliers.push_back(new AzgalorDoomedBotPrioritizePositioningMultiplier(botAI));
}
