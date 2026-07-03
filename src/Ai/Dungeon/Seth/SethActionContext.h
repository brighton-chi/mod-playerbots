/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONCONTEXT_H
#define PLAYERBOTS_SETHACTIONCONTEXT_H

#include "SethActions.h"
#include "NamedObjectContext.h"

class TbcDungeonSethekkHallsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonSethekkHallsActionContext()
    {
        creators["time-lost controller mark charming totem with skull"] =
            &TbcDungeonSethekkHallsActionContext::time_lost_controller_mark_charming_totem_with_skull;

        creators["darkweaver syth mark elementals with skull"] =
            &TbcDungeonSethekkHallsActionContext::darkweaver_syth_mark_elementals_with_skull;
    }
private:
    static Action* time_lost_controller_mark_charming_totem_with_skull(PlayerbotAI* botAI) {
        return new TimeLostControllerMarkCharmingTotemWithSkullAction(botAI);
    }

    static Action* darkweaver_syth_mark_elementals_with_skull(PlayerbotAI* botAI) {
        return new DarkweaverSythMarkElementalsWithSkullAction(botAI);
    }
};

#endif
