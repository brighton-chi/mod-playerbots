/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHTRIGGERS_H
#define PLAYERBOTS_SETHTRIGGERS_H

#include "Trigger.h"

class TimeLostControllerDropsCharmingTotemTrigger : public Trigger
{
public:
    TimeLostControllerDropsCharmingTotemTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "time-lost controller drops charming totem") {}
    bool IsActive() override;
};

class DarkweaverSythSummonsElementalsTrigger : public Trigger
{
public:
    DarkweaverSythSummonsElementalsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "darkweaver syth summons elementals") {}
    bool IsActive() override;
};

#endif
