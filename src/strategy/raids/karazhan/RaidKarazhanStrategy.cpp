#include "RaidKarazhanStrategy.h"
#include "RaidKarazhanMultipliers.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Attumen the Huntsman
    triggers.push_back(new TriggerNode(
        "attumen the huntsman need target priority",
        NextAction::array(0, new NextAction("attumen the huntsman mark target", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "attumen the huntsman attumen spawned",
        NextAction::array(0, new NextAction("attumen the huntsman split bosses", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "attumen the huntsman attumen mounted",
        NextAction::array(0, new NextAction("attumen the huntsman stack behind", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "attumen the huntsman manage timer",
        NextAction::array(0, new NextAction("attumen the huntsman manage timer", ACTION_RAID + 2), nullptr)
    ));

    // Moroes
    triggers.push_back(new TriggerNode(
        "moroes need target priority",
        NextAction::array(0, new NextAction("moroes mark target", ACTION_RAID + 1), nullptr)
    ));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode(
        "maiden of virtue main tank movement",
        NextAction::array(0, new NextAction("maiden of virtue main tank movement", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "maiden of virtue position ranged",
        NextAction::array(0, new NextAction("maiden of virtue position ranged", ACTION_RAID + 1), nullptr)
    ));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode(
        "big bad wolf run away",
        NextAction::array(0, new NextAction("big bad wolf run away", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "big bad wolf position boss",
        NextAction::array(0, new NextAction("big bad wolf position boss", ACTION_RAID + 1), nullptr)
    ));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode(
        "romulo and julianne mark target",
        NextAction::array(0, new NextAction("romulo and julianne mark target", ACTION_RAID + 1), nullptr)
    ));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode(
        "wizard of oz mark target",
        NextAction::array(0, new NextAction("wizard of oz mark target", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "wizard of oz scorch strawman",
        NextAction::array(0, new NextAction("wizard of oz scorch strawman", ACTION_RAID + 2), nullptr)
    ));

    // The Curator
    triggers.push_back(new TriggerNode(
        "the curator mark astral flare",
        NextAction::array(0, new NextAction("the curator mark astral flare", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "the curator position boss",
        NextAction::array(0, new NextAction("the curator position boss", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "the curator spread ranged",
        NextAction::array(0, new NextAction("the curator spread ranged", ACTION_RAID + 2), nullptr)
    ));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode(
        "terestian illhoof mark target",
        NextAction::array(0, new NextAction("terestian illhoof mark target", ACTION_RAID + 1), nullptr)
    ));

    // Shade of Aran
    triggers.push_back(new TriggerNode(
        "shade of aran arcane explosion run away",
        NextAction::array(0, new NextAction("shade of aran arcane explosion run away", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "shade of aran flame wreath stop movement",
        NextAction::array(0, new NextAction("shade of aran flame wreath stop movement", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "shade of aran mark conjured elemental",
        NextAction::array(0, new NextAction("shade of aran mark conjured elemental", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "shade of aran spread ranged",
        NextAction::array(0, new NextAction("shade of aran spread ranged", ACTION_RAID + 2), nullptr)
    ));

    // Netherspite
    triggers.push_back(new TriggerNode(
        "netherspite block red beam",
        NextAction::array(0, new NextAction("netherspite block red beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "netherspite block blue beam",
        NextAction::array(0, new NextAction("netherspite block blue beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "netherspite block green beam",
        NextAction::array(0, new NextAction("netherspite block green beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "netherspite avoid beam and void zone",
        NextAction::array(0, new NextAction("netherspite avoid beam and void zone", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "netherspite banish phase avoid void zone",
        NextAction::array(0, new NextAction("netherspite banish phase avoid void zone", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "netherspite manage timers",
        NextAction::array(0, new NextAction("netherspite manage timers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode(
        "prince malchezaar enfeebled avoid hazard",
        NextAction::array(0, new NextAction("prince malchezaar enfeebled avoid hazard", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "prince malchezaar non tank avoid infernal",
        NextAction::array(0, new NextAction("prince malchezaar non tank avoid infernal", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "prince malchezaar main tank movement",
        NextAction::array(0, new NextAction("prince malchezaar main tank movement", ACTION_EMERGENCY + 6), nullptr)
    ));

    // Nightbane
    triggers.push_back(new TriggerNode(
        "nightbane ground phase position boss",
        NextAction::array(0, new NextAction("nightbane ground phase position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "nightbane ground phase rotate ranged positions",
        NextAction::array(0, new NextAction("nightbane ground phase rotate ranged positions", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "nightbane cast fear ward on main tank",
        NextAction::array(0, new NextAction("nightbane cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "nightbane control pet aggression",
        NextAction::array(0, new NextAction("nightbane control pet aggression", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "nightbane flight phase movement",
        NextAction::array(0, new NextAction("nightbane flight phase movement", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode(
        "nightbane manage timers and trackers",
        NextAction::array(0, new NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AttumenTheHuntsmanDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDPSMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionDisableChargeMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteKeepBlockingBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteWaitForDPSMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleKeepDistanceMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneWaitForDPSMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}
