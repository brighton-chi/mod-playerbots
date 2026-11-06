/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraStrategy.h"
#include "AiObjectContext.h"
#include "KaraHelpers.h"
#include "KaraMultipliers.h"
#include "Playerbots.h"
#include "vector"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shared
    triggers.push_back(new TriggerNode("karazhan no encounter in progress",
        { NextAction("karazhan reset encounter states", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("karazhan enemies cast fear",
        { NextAction("karazhan set tremor totem", ACTION_RAID) }));

    // Trash
    triggers.push_back(new TriggerNode("mana warp casts warp breach",
        { NextAction("mana warp stun warp breach", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("mana warp stunner should hold",
        { NextAction("mana warp stunner hold for warp breach", ACTION_RAID) }));

    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen the huntsman should be tanked",
        { NextAction("attumen the huntsman assist tank position unmounted", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman midnight should be focused",
        { NextAction("attumen the huntsman attack midnight", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("attumen the huntsman mounted should be tanked",
        { NextAction("attumen the huntsman tank position mounted", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman should stack behind",
        { NextAction("attumen the huntsman stack behind", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman should manage dps timer",
        { NextAction("attumen the huntsman manage dps timer", ACTION_EMERGENCY + 10) }));

    // Moroes
    triggers.push_back(new TriggerNode("moroes should prioritize adds",
        { NextAction("moroes mark target", ACTION_RAID) }));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden of virtue should be tanked",
        { NextAction("maiden of virtue tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("maiden of virtue should break repentance",
        { NextAction("maiden of virtue move boss to stunned healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("maiden of virtue ranged should spread",
        { NextAction("maiden of virtue position ranged between pillars", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("maiden of virtue should ground holy fire",
        { NextAction("maiden of virtue set grounding totem", ACTION_RAID) }));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode("big bad wolf chasing red hood",
        { NextAction("big bad wolf red hood run away", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("big bad wolf paladin should protect red hood",
        { NextAction("big bad wolf cast hand of protection on red hood", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("big bad wolf should be tanked",
        { NextAction("big bad wolf position boss", ACTION_RAID) }));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode("romulo and julianne should be killed together",
        { NextAction("romulo and julianne mark target", ACTION_RAID) }));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode("wizard of oz should prioritize targets",
        { NextAction("wizard of oz mark target", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("wizard of oz mage should burn strawman",
        { NextAction("wizard of oz cast scorch on strawman", ACTION_RAID + 1) }));

    // The Curator
    triggers.push_back(new TriggerNode("the curator should prioritize astral flares",
        { NextAction("the curator mark astral flare", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("the curator should be tanked",
        { NextAction("the curator position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the curator ranged should spread",
        { NextAction("the curator spread ranged", ACTION_RAID) }));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode("terestian illhoof should prioritize chains",
        { NextAction("terestian illhoof mark target", ACTION_RAID) }));

    // Shade of Aran
    triggers.push_back(new TriggerNode("shade of aran arcane explosion is casting",
        { NextAction("shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("shade of aran ranged is in blizzard",
        { NextAction("shade of aran run out of blizzard", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("shade of aran flame wreath is active",
        { NextAction("shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("shade of aran conjured elementals summoned",
        { NextAction("shade of aran mark conjured elemental", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("shade of aran ranged should maintain distance",
        { NextAction("shade of aran ranged maintain distance", ACTION_RAID + 1) }));

    // Netherspite
    triggers.push_back(new TriggerNode("netherspite should block red beam",
        { NextAction("netherspite block red beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should block beam",
        { NextAction("netherspite block beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite next beam blocker should prepare",
        { NextAction("netherspite wait to block beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should position around beams",
        { NextAction("netherspite move to beam area", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should spread for netherbreath",
        { NextAction("netherspite move to banish slot", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("netherspite should hold dps",
        { NextAction("netherspite stop attacking", ACTION_RAID + 3),
          NextAction("netherspite select target for hold", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("netherspite should manage timers and trackers",
        { NextAction("netherspite manage timers and trackers", ACTION_EMERGENCY + 10) }));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode("prince malchezaar bot is enfeebled",
        { NextAction("prince malchezaar enfeebled bot avoid hazard", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("prince malchezaar engaged by non-tanks",
        { NextAction("prince malchezaar non-tank avoid infernal", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("prince malchezaar should be tanked",
        { NextAction("prince malchezaar tanks position boss", ACTION_EMERGENCY + 6) }));

    // Nightbane
    triggers.push_back(new TriggerNode("nightbane should be tanked",
        { NextAction("nightbane tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("nightbane ranged should stack",
        { NextAction("nightbane ranged stack", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nightbane pets should be held",
        { NextAction("nightbane control pet aggression", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nightbane should hold dps",
        { NextAction("nightbane stop attacking", ACTION_RAID + 3),
          NextAction("nightbane select target for hold", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("nightbane should stack in flight",
        { NextAction("nightbane stack in flight", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("nightbane should stand down",
        { NextAction("nightbane stand down", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nightbane should manage timers and trackers",
        { NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10) }));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KarazhanSetTremorTotemMultiplier(botAI));
    multipliers.push_back(new KarazhanDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new ManaWarpSaveStunsMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanDisableAutoTargetingMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDpsMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueSetGroundingTotemMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new TerestianIllhoofDontDotFiendishImpsMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionRunAwayMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranDisableReactiveMovementMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteDisableReactiveMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteHoldDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneHoldDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}

void RaidKarazhanStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    Unit* nightbane =
        KaraHelpers::GetKaraBoss(botAI->GetBot(), KaraHelpers::KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return;

    if (KaraHelpers::IsNightbaneFlying(*nightbane))
        exclusions.insert(nightbane->GetGUID());

    // Reach would walk a bot off the terrace after a skeleton that spawned out of bounds.
    uint32 const skeletonEntry = KaraHelpers::Id(KaraHelpers::KaraNpcs::NPC_RESTLESS_SKELETON);
    GuidVector const targets =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const& guid : targets)
    {
        if (guid.GetEntry() != skeletonEntry)
            continue;

        Unit* skeleton = botAI->GetUnit(guid);
        if (skeleton && KaraHelpers::IsOutsideNightbaneTerrace(*skeleton))
            exclusions.insert(guid);
    }
}
