/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALVALUECONTEXT_H
#define PLAYERBOTS_HYJALVALUECONTEXT_H

#include "HyjalHelpers.h"
#include "NamedObjectContext.h"
#include "Value.h"

// Anetheron's Infernals, oldest first. Cached because the grid search behind it is not something
// every trigger and multiplier should be repeating each tick for every bot in the raid
class HyjalInfernalsValue : public CalculatedValue<GuidVector>
{
public:
    HyjalInfernalsValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "hyjal infernals", 200) {}

protected:
    GuidVector Calculate() override { return HyjalHelpers::FindInfernalGuids(bot); }
};

class RaidHyjalSummitValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidHyjalSummitValueContext()
    {
        creators["hyjal infernals"] = &RaidHyjalSummitValueContext::hyjal_infernals;
    }

private:
    static UntypedValue* hyjal_infernals(PlayerbotAI* botAI) { return new HyjalInfernalsValue(botAI); }
};

#endif
