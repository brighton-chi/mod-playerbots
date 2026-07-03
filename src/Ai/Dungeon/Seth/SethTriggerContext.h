/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHTRIGGERCONTEXT_H
#define PLAYERBOTS_SETHTRIGGERCONTEXT_H

#include "SethTriggers.h"
#include "NamedObjectContext.h"

class TbcDungeonSethekkHallsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonSethekkHallsTriggerContext()
    {
        creators["time-lost controller drops charming totem"] =
            &TbcDungeonSethekkHallsTriggerContext::time_lost_controller_drops_charming_totem;
    }
private:
    static Trigger* time_lost_controller_drops_charming_totem(PlayerbotAI* botAI) {
        return new TimeLostControllerDropsCharmingTotemTrigger(botAI);
    }
};

#endif
