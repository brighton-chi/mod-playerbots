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
        NextAction("time-lost controller mark charming totem with skull", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("darkweaver syth summons elementals", {
        NextAction("darkweaver syth mark elementals with skull", ACTION_RAID + 1) }));
}

void TbcDungeonSethekkHallsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
}
