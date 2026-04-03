/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

 #ifndef _PLAYERBOT_PALADINTOGGLEGREATERBLESSINGACTION_H
 #define _PLAYERBOT_PALADINTOGGLEGREATERBLESSINGACTION_H

#include "Action.h"
#include "Playerbots.h"

// Periodically checks raid membership and toggles the "gblessing" strategy on/off accordingly.
// Replaces existing blessing strategies (bmana, bdps, bstats, bhealth) when entering a raid, restores
// an appropriate one when leaving.
class ToggleGreaterBlessingStrategyAction : public Action
{
public:
    ToggleGreaterBlessingStrategyAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override { return true; }

private:
    // Track whether the user explicitly disabled gblessing.
    // Reset when the bot leaves a raid group.
    bool userDisabled_ = false;
    bool wasInRaid_    = false;
};

#endif
