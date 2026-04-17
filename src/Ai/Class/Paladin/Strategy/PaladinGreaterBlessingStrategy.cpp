/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinGreaterBlessingStrategy.h"

#include "Playerbots.h"

void PaladinGreaterBlessingStrategy::InitTriggers(
    std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("greater blessing needed",
        { NextAction("cast greater blessing assignment", 11.0f) }));
}
