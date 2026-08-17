/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalStrategy.h"
#include "AiObjectContext.h"
#include "HyjalHelpers.h"
#include "HyjalMultipliers.h"
#include "Playerbots.h"

using namespace HyjalHelpers;

void RaidHyjalSummitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("hyjal summit bot is not in combat", {
        NextAction("hyjal summit reset encounter states", ACTION_EMERGENCY + 10) }));

    // Rage Winterchill
    triggers.push_back(new TriggerNode("rage winterchill pulling boss", {
        NextAction("rage winterchill misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill boss engaged by main tank", {
        NextAction("rage winterchill main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("rage winterchill boss casts death and decay on ranged", {
        NextAction("rage winterchill spread ranged in circle", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("rage winterchill melee is standing in death and decay", {
        NextAction("rage winterchill melee get out of death and decay", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill ranged is standing in death and decay", {
        NextAction("rage winterchill ranged get out of death and decay", ACTION_EMERGENCY + 1) }));

    // Anetheron
    triggers.push_back(new TriggerNode("anetheron pulling boss or infernal", {
        NextAction("anetheron misdirect boss and infernals to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("anetheron boss engaged by main tank", {
        NextAction("anetheron main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anetheron boss casts carrion swarm", {
        NextAction("anetheron spread ranged in circle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("anetheron bot is targeted by infernal", {
        NextAction("anetheron bring infernal to infernal tank", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("anetheron bot is near inferno target", {
        NextAction("anetheron move away from inferno target", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("anetheron infernals need to be kept away from raid", {
        NextAction("anetheron infernal tank take position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("anetheron should determine dps priority", {
        NextAction("anetheron assign dps priority", ACTION_RAID) }));

    // Kaz'rogal
    triggers.push_back(new TriggerNode("kaz'rogal pulling boss", {
        NextAction("kaz'rogal misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal boss engaged by main tank", {
        NextAction("kaz'rogal main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal malevolent cleave splits damage", {
        NextAction("kaz'rogal assist tanks move in front of boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal low mana bots need escape path", {
        NextAction("kaz'rogal spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal bot is low on mana", {
        NextAction("kaz'rogal move away from group", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("kaz'rogal hunter should preserve mana", {
        NextAction("kaz'rogal activate aspect of the viper", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kaz'rogal mark on mage or paladin", {
        NextAction("kaz'rogal cancel mark", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kaz'rogal warlock should manage mana", {
        NextAction("kaz'rogal warlock manage mana", ACTION_EMERGENCY + 6) }));

    // Azgalor
    triggers.push_back(new TriggerNode("azgalor pulling boss", {
        NextAction("azgalor misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("azgalor boss engaged by main tank", {
        NextAction("azgalor main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("azgalor boss engaged by ranged", {
        NextAction("azgalor disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("azgalor melee should maneuver through fire", {
        NextAction("azgalor melee maneuver through fire", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("azgalor ranged is standing in rain of fire", {
        NextAction("azgalor ranged get out of rain of fire", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("azgalor bot is doomed", {
        NextAction("azgalor move to doomguard tank", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("azgalor doomguards must be controlled", {
        NextAction("azgalor first assist tank position doomguard", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("azgalor melee and ranged should divide dps", {
        NextAction("azgalor determine dps priority", ACTION_RAID) }));

    // Archimonde
    triggers.push_back(new TriggerNode("archimonde pulling boss", {
        NextAction("archimonde misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("archimonde boss engaged by main tank", {
        NextAction("archimonde move boss to initial position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("archimonde boss casts fear", {
        NextAction("archimonde cast fear immunity spell", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("archimonde boss casting air burst", {
        NextAction("archimonde spread to avoid air burst", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("archimonde ranged should spread", {
        NextAction("archimonde spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("archimonde boss summoned doomfire", {
        NextAction("archimonde avoid doomfire", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("archimonde bot stood in doomfire", {
        NextAction("archimonde remove doomfire dot", ACTION_EMERGENCY + 7) }));
}

void RaidHyjalSummitStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Trash
    multipliers.push_back(new HyjalSummitDelayDpsCooldownsMultiplier(botAI));

    // Rage Winterchill
    multipliers.push_back(new RageWinterchillDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new RageWinterchillMeleeControlAvoidanceMultiplier(botAI));
    multipliers.push_back(new RageWinterchillRangedControlAvoidanceMultiplier(botAI));

    // Anetheron
    multipliers.push_back(new AnetheronDisableAssistTargetingMultiplier(botAI));
    multipliers.push_back(new AnetheronAvoidAccidentalInfernalAggroMultiplier(botAI));
    multipliers.push_back(new AnetheronInfernalTargetRunToPositionMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMovementMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMisdirectionMultiplier(botAI));

    // Kaz'rogal
    multipliers.push_back(new KazrogalDisableDisperseAndTankFaceMultiplier(botAI));
    multipliers.push_back(new KazrogalControlLowManaMovementMultiplier(botAI));
    multipliers.push_back(new KazrogalKeepAspectOfTheViperActiveMultiplier(botAI));

    // Azgalor
    multipliers.push_back(new AzgalorDisableAutoTargetingAndPositioningMultiplier(botAI));
    multipliers.push_back(new AzgalorDoomedBotPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new AzgalorMeleeDpsControlAvoidanceMultiplier(botAI));
    multipliers.push_back(new AzgalorRangedControlAvoidanceMultiplier(botAI));

    // Archimonde
    multipliers.push_back(new ArchimondeDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new ArchimondeControlDoomfireAvoidanceMultiplier(botAI));
    multipliers.push_back(new ArchimondeSetTremorTotemMultiplier(botAI));
}

// Limit Infernals to Infernal tank
void RaidHyjalSummitStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::Tank)
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return;

    GuidVector const& infernals = GetInfernalGuids(botAI);
    if (infernals.empty())
        return;

    if (IsInfernalTank(botAI->GetBot()))
        exclusions.insert(anetheron->GetGUID());
    else
        exclusions.insert(infernals.begin(), infernals.end());
}
