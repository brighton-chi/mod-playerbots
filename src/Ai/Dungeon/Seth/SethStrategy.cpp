/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethTriggers.h"
#include "SethStrategy.h"
#include "SethMultipliers.h"

void TbcDungeonSethekkHallsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("time-lost controller drops charming totem", {
        NextAction("time-lost controller mark charming totem with skull", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("sethekk prophet casts fear", {
        NextAction("sethekk prophet drop tremor totem", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("darkweaver syth boss summons elementals", {
        NextAction("darkweaver syth mark elementals with skull", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anzu encounter has two phases", {
        NextAction("anzu alternate marks on boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anzu bird spirits provide buffs", {
        NextAction("anzu cast heal over time spell on bird spirit", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("talon king ikiss boss engaged by tank", {
        NextAction("talon king ikiss tank move boss to pillar position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("talon king ikiss ranged prepare for arcane explosion", {
        NextAction("talon king ikiss ranged stay near victim of boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("talon king ikiss boss casting arcane explosion", {
        NextAction("talon king ikiss los arcane explosion", ACTION_EMERGENCY + 10) }));
}

void TbcDungeonSethekkHallsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SethekkProphetUseTremorTotemMultiplier(botAI));
    multipliers.push_back(new AnzuControlSpellCastingWithSpellBombMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new TalonKingIkissControlMovementMultiplier(botAI));
}
