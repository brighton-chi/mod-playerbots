/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethTriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace
{
constexpr uint32 SETHEKK_HALLS_MAP_ID = 556;
}

bool TimeLostControllerDropsCharmingTotemTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, SETHEKK_HALLS_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "time-lost controller");
}

bool DarkweaverSythSummonsElementalsTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, SETHEKK_HALLS_MAP_ID))
        return false;

    Unit* syth = AI_VALUE2(Unit*, "find target", "darkweaver syth");
    return syth && syth->GetHealthPct() > 10.0f;
}
