/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellStrategy.h"
#include "RaidSunwellMultipliers.h"

void RaidSunwellStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("sunwell plateau bot is not in combat", {
        NextAction("sunwell plateau erase timers and trackers", ACTION_EMERGENCY + 11) }));

    // Kalecgos & Sathrovarr the Corruptor
    triggers.push_back(new TriggerNode("kalecgos boss engaged by tank", {
        NextAction("kalecgos tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos spectral rift is open", {
        NextAction("kalecgos enter spectral rift", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kalecgos bots take splash damage", {
        NextAction("kalecgos disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos both bosses must be defeated", {
        NextAction("kalecgos determine boss to attack", ACTION_RAID + 2) }));

    // Brutallus
    triggers.push_back(new TriggerNode("brutallus pulling boss", {
        NextAction("brutallus misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by tanks", {
        NextAction("brutallus tanks handle boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by melee", {
        NextAction("brutallus position melee", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by ranged", {
        NextAction("brutallus position ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus bot is burning", {
        NextAction("brutallus handle burn", ACTION_EMERGENCY + 1) }));

    // Felmyst
    triggers.push_back(new TriggerNode("felmyst pulling boss", {
        NextAction("felmyst misdirect boss to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by main tank on ground", {
        NextAction("felmyst main tank position boss on ground", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by ranged on ground", {
        NextAction("felmyst position ranged on ground", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst bot is encapsulated", {
        NextAction("felmyst remove encapsulate", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst bot near encapsulated player", {
        NextAction("felmyst run away from encapsulated player", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("felmyst player has gas nova", {
        NextAction("felmyst cast mass dispel on gas nova", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst fog of corruption is active", {
        NextAction("felmyst avoid fog of corruption", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst demonic vapor is summoned", {
        NextAction("felmyst avoid demonic vapor", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst manual targeting is required", {
        NextAction("felmyst assign target priority", ACTION_RAID + 2) }));

    // Eredar Twins (Alythess & Sacrolash)
    triggers.push_back(new TriggerNode("eredar twins", {
        NextAction("eredar twins", ACTION_RAID + 1) }));

    // M'uru & Entropius
    triggers.push_back(new TriggerNode("m'uru", {
        NextAction("m'uru", ACTION_RAID + 1) }));

    // Kiljaeden <The Deceiver>
    triggers.push_back(new TriggerNode("kil'jaeden", {
        NextAction("kil'jaeden", ACTION_RAID + 1) }));
}

void RaidSunwellStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Kalecgos & Sathrovarr the Corruptor
    multipliers.push_back(new KalecgosControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KalecgosWaitToDecurseMultiplier(botAI));
    multipliers.push_back(new KalecgosControlMovementMultiplier(botAI));
    multipliers.push_back(new KalecgosDelayCooldownsForSathrovarrMultiplier(botAI));

    // Brutallus
    multipliers.push_back(new BrutallusControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new BrutallusControlMovementMultiplier(botAI));
    multipliers.push_back(new BrutallusNoTankingWithTooManyMeteorStacksMultiplier(botAI));
    multipliers.push_back(new BrutallusDelayCooldownsMultiplier(botAI));

    // Felmyst
    multipliers.push_back(new FelmystDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new FelmystControlMovementMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeFogAvoidanceMultiplier(botAI));

    // Eredar Twins (Alythess & Sacrolash)
    multipliers.push_back(new EredarTwinsMultiplier(botAI));

    // M'uru & Entropius
    multipliers.push_back(new MuruMultiplier(botAI));

    // Kil'jaeden <The Deceiver>
    multipliers.push_back(new KiljaedenMultiplier(botAI));
}
