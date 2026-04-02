/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINGREATERBLESSINGTRIGGER_H
#define _PLAYERBOT_PALADINGREATERBLESSINGTRIGGER_H

#include "GenericTriggers.h"
#include "Playerbots.h"

// Fires when ANY raid member needs a blessing assignment from this bot
// that they don't currently have.
class GreaterBlessingNeededTrigger : public Trigger
{
public:
    GreaterBlessingNeededTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "greater blessing needed", 4) {} // 4s check interval

    bool IsActive() override;
};

#endif
