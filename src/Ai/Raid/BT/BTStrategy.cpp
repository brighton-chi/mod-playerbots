/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTStrategy.h"
#include "BTMultipliers.h"

void RaidBlackTempleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("black temple no encounter in progress",
        { NextAction("black temple reset encounter states", ACTION_EMERGENCY + 11) }));

    // Trash
    triggers.push_back(new TriggerNode("sister of pleasure should be marked",
        { NextAction("mark sister of pleasure", ACTION_RAID + 1) }));

    // High Warlord Naj'entus
    triggers.push_back(new TriggerNode("high warlord naj'entus hunter should misdirect",
        { NextAction("high warlord naj'entus misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus should be tanked",
        { NextAction("high warlord naj'entus tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus ranged should spread",
        { NextAction("high warlord naj'entus disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaled player needs remover",
        { NextAction("high warlord naj'entus assign spine remover", ACTION_EMERGENCY + 12) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaling spine on group member",
        { NextAction("high warlord naj'entus remove impaling spine", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus needs spine thrower",
        { NextAction("high warlord naj'entus assign spine thrower", ACTION_EMERGENCY + 12) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus has tidal shield",
        { NextAction("high warlord naj'entus throw impaling spine", ACTION_RAID + 2) }));

    // Supremus
    triggers.push_back(new TriggerNode("supremus hunter should misdirect",
        { NextAction("supremus misdirect to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("supremus ranged should spread",
        { NextAction("supremus disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("supremus fixates on bot",
        { NextAction("supremus kite boss", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("supremus near volcano",
        { NextAction("supremus move away from volcanos", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("supremus should manage phase timer",
        { NextAction("supremus manage phase timer", ACTION_EMERGENCY + 10) }));

    // Shade of Akama
    triggers.push_back(new TriggerNode("shade of akama killing channelers starts phase 2",
        { NextAction("shade of akama melee dps prioritize channelers", ACTION_RAID + 1) }));

    // Teron Gorefiend
    triggers.push_back(new TriggerNode("teron gorefiend hunter should misdirect",
        { NextAction("teron gorefiend misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("teron gorefiend should be tanked",
        { NextAction("teron gorefiend tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend ranged should position on balcony",
        { NextAction("teron gorefiend position ranged on balcony", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend casts shadow of death",
        { NextAction("teron gorefiend avoid shadow of death", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend shadow of death",
        { NextAction("teron gorefiend move to corner to die", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend transformed into vengeful spirit",
        { NextAction(
            "teron gorefiend control and destroy shadowy constructs", ACTION_EMERGENCY + 10) }));

    // Gurtogg Bloodboil
    triggers.push_back(new TriggerNode("gurtogg bloodboil hunter should misdirect",
        { NextAction("gurtogg bloodboil misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should be tanked",
        { NextAction("gurtogg bloodboil tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil casts bloodboil",
        { NextAction("gurtogg bloodboil rotate ranged groups", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil fel rage on group member",
        { NextAction("gurtogg bloodboil ranged move away from enraged player", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should manage phase timer",
        { NextAction("gurtogg bloodboil manage phase timer", ACTION_EMERGENCY + 10) }));

    // Reliquary of Souls
    triggers.push_back(new TriggerNode("reliquary of souls hunter should misdirect",
        { NextAction("reliquary of souls misdirect to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode(
        "reliquary of souls essence of suffering fixates on closest target",
        { NextAction("reliquary of souls adjust distance from suffering", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of suffering disables healing",
        { NextAction("reliquary of souls healers dps suffering", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire has rune shield",
        { NextAction("reliquary of souls spellsteal rune shield", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire casts deaden",
        { NextAction("reliquary of souls spell reflect deaden", ACTION_EMERGENCY + 6) }));

    // Mother Shahraz
    triggers.push_back(new TriggerNode("mother shahraz hunter should misdirect",
        { NextAction("mother shahraz misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mother shahraz should be tanked",
        { NextAction("mother shahraz tanks position boss under pillar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz tanks are positioning boss",
        { NextAction("mother shahraz melee dps wait at safe position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz ranged should position under pillar",
        { NextAction("mother shahraz position ranged under pillar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz fatal attraction",
        { NextAction(
            "mother shahraz run away to break fatal attraction", ACTION_EMERGENCY + 10) }));

    // Illidari Council
    triggers.push_back(new TriggerNode("illidari council hunter should misdirect",
        { NextAction("illidari council misdirect to tanks", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidari council gathios should be tanked",
        { NextAction("illidari council main tank position gathios", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council gathios casts judgement of command",
        { NextAction(
            "illidari council main tank reflect judgement of command", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidari council malande should be tanked",
        { NextAction("illidari council first assist tank focus malande", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council darkshadow should be tanked",
        { NextAction(
            "illidari council second assist tank position darkshadow", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council zerevor should be tanked by mage",
        { NextAction("illidari council mage tank position zerevor", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidari council mage tank needs dedicated healer",
        { NextAction("illidari council position mage tank healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council ranged should spread",
        { NextAction("illidari council disperse ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidari council pets screw up the pull",
        { NextAction("illidari council command pets to attack gathios", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidari council should assign dps priority",
        { NextAction("illidari council assign dps targets", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council should manage dps timer",
        { NextAction("illidari council manage dps timer", ACTION_EMERGENCY + 10) }));

    // Illidan Stormrage <The Betrayer>
    triggers.push_back(new TriggerNode("illidan stormrage hunter should misdirect",
        { NextAction("illidan stormrage misdirect to tanks", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage casts flame crash",
        { NextAction("illidan stormrage main tank reposition boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage parasitic shadowfiend on group member",
        { NextAction("illidan stormrage isolate bot with parasite", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage parasitic shadowfiends run wild",
        { NextAction("illidan stormrage set earthbind totem", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage flames of azzinoth should be tanked",
        { NextAction(
            "illidan stormrage assist tanks handle flames of azzinoth", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage pets die to fire",
        { NextAction("illidan stormrage control pet aggression", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidan stormrage grate is safe from flames",
        { NextAction("illidan stormrage position above grate", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage dark barrage on immunity class",
        { NextAction("illidan stormrage remove dark barrage", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidan stormrage prepares to land",
        { NextAction("illidan stormrage move away from landing point", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage ranged should spread",
        { NextAction("illidan stormrage disperse ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage this expansion hates melee",
        { NextAction("illidan stormrage melee go somewhere to not die", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage warlock should tank demon form",
        { NextAction("illidan stormrage warlock tank handle demon boss", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("illidan stormrage should assign dps priority",
        { NextAction("illidan stormrage dps prioritize adds", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage maiev placed shadow trap",
        { NextAction("illidan stormrage use shadow trap", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage should manage dps timer and rti",
        { NextAction("illidan stormrage manage dps timer and rti", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("illidan stormrage should clear hazards between phases",
        { NextAction("illidan stormrage destroy hazards", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("illidan stormrage cheat",
        { NextAction("illidan stormrage handle adds cheat", ACTION_EMERGENCY + 10) }));
}

void RaidBlackTempleStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new BlackTempleDelayDpsCooldownsMultiplier(botAI));

    // High Warlord Naj'entus
    multipliers.push_back(new HighWarlordNajentusDisableCombatFormationMoveMultiplier(botAI));

    // Supremus
    multipliers.push_back(new SupremusFocusOnAvoidanceInPhase2Multiplier(botAI));
    multipliers.push_back(new SupremusHitboxIsBuggedMultiplier(botAI));

    // Teron Gorefiend
    multipliers.push_back(new TeronGorefiendControlMovementMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendDisableAttackingConstructsMultiplier(botAI));

    // Gurtogg Bloodboil
    multipliers.push_back(new GurtoggBloodboilControlMovementMultiplier(botAI));

    // Reliquary of Souls
    multipliers.push_back(new ReliquaryOfSoulsDontWasteHealingMultiplier(botAI));

    // Mother Shahraz
    multipliers.push_back(new MotherShahrazControlMovementMultiplier(botAI));
    multipliers.push_back(new MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(botAI));

    // Illidari Council
    multipliers.push_back(new IllidariCouncilDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilControlMovementMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilDisableArcaneShotOnZerevorMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilDisableIceBlockMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilWaitForDpsMultiplier(botAI));

    // Illidan Stormrage <The Betrayer>
    multipliers.push_back(new IllidanStormrageDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageControlTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageControlNonTankMovementMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageUseEarthbindTotemMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageWaitForDpsMultiplier(botAI));
}
