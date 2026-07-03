/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONS_H
#define PLAYERBOTS_SETHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class TimeLostControllerMarkCharmingTotemWithSkullAction : public Action
{
public:
    TimeLostControllerMarkCharmingTotemWithSkullAction(
        PlayerbotAI* botAI) : Action(botAI, "time-lost controller mark charming totem with skull") {}
    bool Execute(Event event) override;
};

class DarkweaverSythMarkElementalsWithSkullAction : public Action
{
public:
    DarkweaverSythMarkElementalsWithSkullAction(
        PlayerbotAI* botAI) : Action(botAI, "darkweaver syth mark elementals with skull") {}
    bool Execute(Event event) override;
};

#endif
