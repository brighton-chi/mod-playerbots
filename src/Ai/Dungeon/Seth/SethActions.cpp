/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

bool TimeLostControllerMarkCharmingTotemWithSkullAction::Execute(Event event)
{
    constexpr uint32 NPC_CHARMING_TOTEM = 20343;
    if (Unit* totem = GetFirstAliveUnitByEntry(NPC_CHARMING_TOTEM))
        MarkTargetWithSkull(bot, totem);

    return false;
}

bool DarkweaverSythMarkElementalsWithSkullAction::Execute(Event event)
{
    if (Unit* frostElemental = AI_VALUE2(Unit*, "syth frost elemental"))
        MarkTargetWithSkull(bot, frostElemental);
    else if (Unit* shadowElemental = AI_VALUE2(Unit*, "syth shadow elemental"))
        MarkTargetWithSkull(bot, shadowElemental);
    else if (Unit* arcaneElemental = AI_VALUE2(Unit*, "syth arcane elemental"))
        MarkTargetWithSkull(bot, arcaneElemental);
    else if (Unit* fireElemental = AI_VALUE2(Unit*, "syth fire elemental"))
        MarkTargetWithSkull(bot, fireElemental);

    return false;
}
