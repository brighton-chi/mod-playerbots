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

    triggers.push_back(new TriggerNode("kalecgos bots have trouble with world transition", {
        NextAction("kalecgos return to spectral realm ground", ACTION_EMERGENCY + 10) }));

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
        NextAction("felmyst misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by main tank on ground", {
        NextAction("felmyst main tank position boss on ground", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by ranged on ground", {
        NextAction("felmyst position ranged on ground", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by melee on ground", {
        NextAction("felmyst position melee on ground", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst bot is encapsulated", {
        NextAction("felmyst remove encapsulate", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst bot near encapsulated player", {
        NextAction("felmyst run away from encapsulated player", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("felmyst player has gas nova", {
        NextAction("felmyst cast mass dispel on gas nova", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst boss summons demonic vapor", {
        NextAction("felmyst avoid demonic vapor", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst bot is demonic vapor target", {
        NextAction("felmyst kite demonic vapor", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("felmyst fog of corruption is active", {
        NextAction("felmyst avoid fog of corruption", ACTION_EMERGENCY + 9) }));

    // Eredar Twins (Alythess & Sacrolash)
    triggers.push_back(new TriggerNode("eredar twins encounter just started", {
        NextAction("eredar twins melee jump down from balcony", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("eredar twins pulling bosses", {
        NextAction("eredar twins misdirect bosses to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash engaged by two tanks", {
        NextAction("eredar twins main and second assist tanks position sacrolash", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins alythess engaged by first assist tank", {
        NextAction("eredar twins first assist tank move out of blaze", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins bosses engaged by ranged", {
        NextAction("eredar twins position ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins determining dps priority", {
        NextAction("eredar twins dps prioritize lady sacrolash", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins bot has too many flame touched stacks", {
        NextAction("eredar twins remove flame sear", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("eredar twins only one boss remains", {
        NextAction("eredar twins stack in room center", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("eredar twins bot has conflagration", {
        NextAction("eredar twins conflagrated bot move from group", ACTION_EMERGENCY + 7) }));

    // M'uru & Entropius
    triggers.push_back(new TriggerNode("m'uru determining dps priority", {
        NextAction("m'uru set dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru dark fiends spawned", {
        NextAction("m'uru kill dark fiends with dispel", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru bot near darkness", {
        NextAction("m'uru avoid darkness", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("m'uru void spawn available for enslave", {
        NextAction("m'uru warlock enslave void spawn", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("m'uru warlock has enslaved void spawn", {
        NextAction("m'uru enslaved void spawn cast shadow bolt volley", ACTION_RAID + 2) }));

    // Kil'jaeden <The Deceiver>
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
    multipliers.push_back(new FelmystControlMovementMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeDemonicVaporKiteMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeFogAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystDelayCooldownsMultiplier(botAI));

    // Eredar Twins (Alythess & Sacrolash)
    multipliers.push_back(new EredarTwinsMeleeJumpDownFromBalconyMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlThreatMultiplier(botAI));
    multipliers.push_back(new EredarTwinsDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMovementMultiplier(botAI));
    multipliers.push_back(new EredarTwinsDelayCooldownsMultiplier(botAI));

    // M'uru & Entropius
    multipliers.push_back(new MuruDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new MuruControlTankActionsMultiplier(botAI));
    multipliers.push_back(new MuruControlMovementMultiplier(botAI));
    multipliers.push_back(new MuruDelayCooldownsMultiplier(botAI));

    // Kil'jaeden <The Deceiver>
    multipliers.push_back(new KiljaedenMultiplier(botAI));
}
