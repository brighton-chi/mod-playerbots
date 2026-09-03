/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAStrategy.h"
#include "ZAMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("zul'aman no encounter in progress", {
        NextAction("zul'aman reset encounter states", ACTION_EMERGENCY + 10) }));

    // The same Misdirect upon pull action is used for all ZA bosses.
    triggers.push_back(new TriggerNode("zul'aman pulling boss", {
        NextAction("zul'aman misdirect boss to main tank", ACTION_RAID + 1) }));

    // Trash
    triggers.push_back(new TriggerNode("amani'shi medicine man summoned ward", {
        NextAction("amani'shi medicine man mark ward", ACTION_RAID) }));

    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akil'zon boss engaged by tanks", {
        NextAction("akil'zon tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("akil'zon spread for static disruption", {
        NextAction("akil'zon spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("akil'zon electrical storm incoming", {
        NextAction("akil'zon move to eye of the storm", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("akil'zon bots need to prepare for electrical storm", {
        NextAction("akil'zon manage electrical storm timer", ACTION_EMERGENCY + 10) }));

    // Nalorakk <Bear Avatar>
    triggers.push_back(new TriggerNode("nalorakk boss switches forms", {
        NextAction("nalorakk tanks position boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nalorakk spread for surge", {
        NextAction("nalorakk spread ranged", ACTION_RAID) }));

    // Jan'alai <Dragonhawk Avatar>
    triggers.push_back(new TriggerNode("jan'alai boss engaged by tanks", {
        NextAction("jan'alai tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("jan'alai spread for flame breath", {
        NextAction("jan'alai spread ranged in circle", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("jan'alai boss summoning fire bombs", {
        NextAction("jan'alai avoid fire bombs", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("jan'alai amani'shi hatchers spawned", {
        NextAction("jan'alai mark amani'shi hatchers", ACTION_RAID + 1) }));

    // Halazzi <Lynx Avatar>
    triggers.push_back(new TriggerNode("halazzi should be tanked", {
        NextAction("halazzi main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("halazzi spirit lynx has appeared", {
        NextAction("halazzi first assist tank attack spirit lynx", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("halazzi should focus dps", {
        NextAction("halazzi dps attack totem and boss", ACTION_RAID) }));

    // Hex Lord Malacrass
    triggers.push_back(new TriggerNode("hex lord malacrass should prioritize adds", {
        NextAction("hex lord malacrass assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hex lord malacrass boss is channeling whirlwind", {
        NextAction("hex lord malacrass run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("hex lord malacrass boss placed freezing trap", {
        NextAction("hex lord malacrass move away from freezing trap", ACTION_EMERGENCY + 1) }));

    // Zul'jin
    triggers.push_back(new TriggerNode("zul'jin boss engaged by tanks", {
        NextAction("zul'jin tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("zul'jin boss is channeling whirlwind in troll form", {
        NextAction("zul'jin run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("zul'jin boss is summoning cyclones in eagle form", {
        NextAction("zul'jin spread raid for cyclones", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("zul'jin spread for dragonhawk aoe", {
        NextAction("zul'jin spread ranged", ACTION_RAID) }));
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new ZulAmanDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new ZulAmanDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new ZulAmanControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new ZulAmanDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new ZulAmanAvoidWhirlwindMultiplier(botAI));

    // Akil'zon <Eagle Avatar>
    multipliers.push_back(new AkilzonStayInEyeOfTheStormMultiplier(botAI));

    // Jan'alai <Dragonhawk Avatar>
    multipliers.push_back(new JanalaiStayAwayFromFireBombsMultiplier(botAI));
    multipliers.push_back(new JanalaiDoNotCrowdControlHatchersMultiplier(botAI));

    // Halazzi <Lynx Avatar>
    multipliers.push_back(new HalazziDisableAutoDpsTargetingMultiplier(botAI));

    // Hex Lord Malacrass
    multipliers.push_back(new HexLordMalacrassUnstableAfflictionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassSpellReflectionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassStayAwayFromFreezingTrapMultiplier(botAI));

    // Zul'jin
    multipliers.push_back(new ZuljinEagleDisableAvoidAoeMultiplier(botAI));
}
