/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCStrategy.h"
#include "SSCMultipliers.h"

void RaidSscStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("ssc no encounter in progress", {
        NextAction("ssc reset encounter states", ACTION_EMERGENCY + 10) }));

    // Trash Mobs
    triggers.push_back(new TriggerNode("underbog colossus in toxic pool", {
        NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("greyheart tidecaller water elemental totem spawned", {
        NextAction("greyheart tidecaller mark water elemental totem", ACTION_RAID) }));

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode("hydross the unstable should be tanked by frost tank", {
        NextAction("hydross the unstable position frost tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hydross the unstable should be tanked by nature tank", {
        NextAction("hydross the unstable position nature tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hydross the unstable ranged should spread", {
        NextAction("hydross the unstable frost phase spread out", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable tank needs aggro upon phase change", {
        NextAction("hydross the unstable misdirect boss to tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("hydross the unstable aggro resets upon phase change", {
        NextAction("hydross the unstable stop dps upon phase change", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("hydross the unstable should manage phase timers", {
        NextAction("hydross the unstable manage timers", ACTION_EMERGENCY + 10) }));

    // The Lurker Below
    triggers.push_back(new TriggerNode("the lurker below spout is active", {
        NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("the lurker below should be tanked", {
        NextAction("the lurker below position main tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the lurker below ranged should spread", {
        NextAction("the lurker below spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the lurker below is submerged", {
        NextAction("the lurker below tanks pick up adds", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("the lurker below should manage spout timer", {
        NextAction("the lurker below manage spout timer", ACTION_EMERGENCY + 10) }));

    // Leotheras the Blind
    triggers.push_back(new TriggerNode(
        "leotheras the blind demon form should be tanked by warlock", {
        NextAction("leotheras the blind warlock tank attack boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind only warlock should tank demon form", {
        NextAction("leotheras the blind melee tanks don't attack demon form", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind ranged should spread", {
        NextAction("leotheras the blind position ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind channeling whirlwind", {
        NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind too many chaos blast stacks", {
        NextAction("leotheras the blind melee dps run away from boss", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("leotheras the blind inner demon has awakened", {
        NextAction("leotheras the blind destroy inner demon", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("leotheras the blind in final phase", {
        NextAction("leotheras the blind final phase assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind warlock tank needs aggro", {
        NextAction("leotheras the blind misdirect boss to warlock tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("leotheras the blind should manage dps wait timers", {
        NextAction("leotheras the blind manage dps wait timers", ACTION_EMERGENCY + 10) }));

    // Fathom-Lord Karathress
    triggers.push_back(new TriggerNode("fathom-lord karathress targets should be tanked", {
        NextAction("fathom-lord karathress tanks position targets", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress should heal caribdis tank", {
        NextAction("fathom-lord karathress position caribdis tank healer", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress pulling bosses", {
        NextAction("fathom-lord karathress misdirect bosses to tanks", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress determining kill order", {
        NextAction("fathom-lord karathress assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress should manage dps timer", {
        NextAction("fathom-lord karathress manage dps timer", ACTION_EMERGENCY + 10) }));

    // Morogrim Tidewalker
    triggers.push_back(new TriggerNode("morogrim tidewalker should be tanked", {
        NextAction("morogrim tidewalker move boss to tank position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker in phase 2", {
        NextAction("morogrim tidewalker phase 2 reposition ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker pulling boss", {
        NextAction("morogrim tidewalker misdirect boss to main tank", ACTION_RAID) }));

    // Lady Vashj <Coilfang Matron>
    triggers.push_back(new TriggerNode("lady vashj should be tanked", {
        NextAction("lady vashj main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj ranged should spread in phase 1", {
        NextAction("lady vashj phase 1 spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj shaman should ground shock blast", {
        NextAction("lady vashj set grounding totem in main tank group", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj static charge on group member", {
        NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("lady vashj pulling boss in phase 1 and phase 3", {
        NextAction("lady vashj misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj tainted elemental cheat", {
        NextAction("lady vashj teleport to tainted elemental", ACTION_EMERGENCY + 12),
        NextAction("lady vashj loot tainted core", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("lady vashj tainted core was looted", {
        NextAction("lady vashj pass the tainted core", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("lady vashj adds spawn in phase 2 and phase 3", {
        NextAction("lady vashj assign phase 2 and phase 3 dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj coilfang strider is approaching", {
        NextAction("lady vashj misdirect strider to first assist tank", ACTION_EMERGENCY + 2),
        NextAction("lady vashj tank attack and move away strider", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj in phase 3", {
        NextAction("lady vashj avoid toxic spores", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("lady vashj entangle on melee", {
        NextAction("lady vashj use free action abilities", ACTION_EMERGENCY + 8) }));
}

void RaidSscStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Trash Mobs
    multipliers.push_back(new UnderbogColossusEscapeToxicPoolMultiplier(botAI));

    // Shared Bosses
    multipliers.push_back(new SscControlMisdirectionMultiplier(botAI));

    // Hydross the Unstable <Duke of Currents>
    multipliers.push_back(new HydrossTheUnstableDisableTankReachMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableDisableAutoTargetAndMoveMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDpsMultiplier(botAI));

    // The Lurker Below
    multipliers.push_back(new TheLurkerBelowStayAwayFromSpoutMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMaintainRangedSpreadMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowDisableTankAssistMultiplier(botAI));

    // Leotheras the Blind
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindFocusOnInnerDemonMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDpsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(botAI));

    // Fathom-Lord Karathress
    multipliers.push_back(new FathomLordKarathressDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAutoTargetMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAoeMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDpsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressMaintainPositionMultiplier(botAI));

    // Morogrim Tidewalker
    multipliers.push_back(new MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerMaintainPhase2StackingMultiplier(botAI));

    // Lady Vashj <Coilfang Matron>
    multipliers.push_back(new LadyVashjDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new LadyVashjSetGroundingTotemMultiplier(botAI));
    multipliers.push_back(new LadyVashjMaintainPhase1RangedSpreadMultiplier(botAI));
    multipliers.push_back(new LadyVashjStaticChargeStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new LadyVashjDoNotLootTheTaintedCoreMultiplier(botAI));
    multipliers.push_back(new LadyVashjCorePassersPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new LadyVashjDisableAutoTargetAndMoveMultiplier(botAI));
    multipliers.push_back(new LadyVashjSaveHandOfFreedomMultiplier(botAI));
}
