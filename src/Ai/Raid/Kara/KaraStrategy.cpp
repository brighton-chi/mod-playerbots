#include "KaraStrategy.h"
#include "KaraMultipliers.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("karazhan bot is not in combat",
        { NextAction("karazhan erase encounter states", ACTION_EMERGENCY + 11) }
    ));

    // Trash
    triggers.push_back(new TriggerNode("mana warp is about to explode",
        { NextAction("mana warp stun creature before warp breach", ACTION_EMERGENCY + 6) }
    ));

    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen the huntsman phase one active",
        { NextAction("attumen the huntsman handle phase one", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman phase two active",
        { NextAction("attumen the huntsman handle phase two", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman boss wipes aggro when mounting",
        { NextAction("attumen the huntsman manage dps timer", ACTION_RAID + 1) }
    ));

    // Moroes
    triggers.push_back(new TriggerNode("moroes boss engaged by main tank",
        { NextAction("moroes main tank attack boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("moroes dps should prioritize adds",
        { NextAction("moroes mark target", ACTION_RAID) }
    ));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden of virtue boss engaged by tanks",
        { NextAction("maiden of virtue tank position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("maiden of virtue holy wrath deals chain damage",
        { NextAction("maiden of virtue position ranged", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("maiden of virtue grounding totem consumes holy fire",
        { NextAction("maiden of virtue set grounding totem", ACTION_RAID) }
    ));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode("big bad wolf boss is chasing little red riding hood",
        { NextAction("big bad wolf run away from boss", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("big bad wolf boss engaged by tank",
        { NextAction("big bad wolf position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("big bad wolf casts terrifying howl",
        { NextAction("big bad wolf set tremor totem", ACTION_RAID) }
    ));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode("romulo and julianne both bosses revived",
        { NextAction("romulo and julianne mark target", ACTION_RAID) }
    ));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode("wizard of oz need target priority",
        { NextAction("wizard of oz mark target", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("wizard of oz strawman is vulnerable to fire",
        { NextAction("wizard of oz scorch strawman", ACTION_RAID + 1) }
    ));

    // The Curator
    triggers.push_back(new TriggerNode("the curator astral flare spawned",
        { NextAction("the curator mark astral flare", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("the curator boss engaged by tanks",
        { NextAction("the curator position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("the curator astral flares cast arcing sear",
        { NextAction("the curator spread ranged", ACTION_RAID) }
    ));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode("terestian illhoof need target priority",
        { NextAction("terestian illhoof mark target", ACTION_RAID) }
    ));

    // Shade of Aran
    triggers.push_back(new TriggerNode("shade of aran arcane explosion is casting",
        { NextAction("shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("shade of aran flame wreath is active",
        { NextAction("shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7) }
    ));
    triggers.push_back(new TriggerNode("shade of aran conjured elementals summoned",
        { NextAction("shade of aran mark conjured elemental", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("shade of aran boss uses counterspell and blizzard",
        { NextAction("shade of aran ranged maintain distance", ACTION_RAID + 1) }
    ));

    // Netherspite
    triggers.push_back(new TriggerNode("netherspite red beam is active",
        { NextAction("netherspite block red beam", ACTION_EMERGENCY + 8) }
    ));
    triggers.push_back(new TriggerNode("netherspite blue beam is active",
        { NextAction("netherspite block blue beam", ACTION_EMERGENCY + 8) }
    ));
    triggers.push_back(new TriggerNode("netherspite green beam is active",
        { NextAction("netherspite block green beam", ACTION_EMERGENCY + 8) }
    ));
    triggers.push_back(new TriggerNode("netherspite bot is not beam blocker",
        { NextAction("netherspite avoid beam and void zone", ACTION_EMERGENCY + 7) }
    ));
    triggers.push_back(new TriggerNode("netherspite boss is banished",
        { NextAction("netherspite banish phase avoid void zone", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("netherspite need to manage timers and trackers",
        { NextAction("netherspite manage timers and trackers", ACTION_EMERGENCY + 10) }
    ));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode("prince malchezaar bot is enfeebled",
        { NextAction("prince malchezaar enfeebled avoid hazard", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("prince malchezaar infernals are spawned",
        { NextAction("prince malchezaar non tank avoid infernal", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("prince malchezaar boss engaged by main tank",
        { NextAction("prince malchezaar main tank movement", ACTION_EMERGENCY + 6) }
    ));

    // Nightbane
    triggers.push_back(new TriggerNode("nightbane boss engaged by main tank",
        { NextAction("nightbane ground phase position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("nightbane ranged bots are in charred earth",
        { NextAction("nightbane ground phase rotate ranged positions", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("nightbane main tank is susceptible to fear",
        { NextAction("nightbane cast fear ward on main tank", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("nightbane pets ignore collision to chase flying boss",
        { NextAction("nightbane control pet aggression", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("nightbane boss is flying",
        { NextAction("nightbane flight phase movement", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("nightbane need to manage timers and trackers",
        { NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10) }
    ));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AttumenTheHuntsmanDisableAutomaticTargetingMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDpsMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueSetGroundingTotemMultiplier(botAI));
    multipliers.push_back(new BigBadWolfSetTremorTotemMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new TheCuratorDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionRunAwayMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteKeepBlockingBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteWaitForDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleKeepDistanceMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneWaitForDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}
