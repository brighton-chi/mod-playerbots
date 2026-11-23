/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAVALUECONTEXT_H
#define PLAYERBOTS_ZAVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"
#include "ZAHelpers.h"

class HexLordMalacrassFreezingTrapValue : public ObjectGuidCalculatedValue
{
public:
    HexLordMalacrassFreezingTrapValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(
              botAI, "hex lord malacrass freezing trap",
              ZaHelpers::FREEZING_TRAP_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return ZaHelpers::FindNearbyFreezingTrapGuid(bot); }
};

class RaidZulAmanValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidZulAmanValueContext()
    {
        creators["hex lord malacrass freezing trap"] =
            &RaidZulAmanValueContext::hex_lord_malacrass_freezing_trap;
    }

private:
    static UntypedValue* hex_lord_malacrass_freezing_trap(PlayerbotAI* botAI)
    {
        return new HexLordMalacrassFreezingTrapValue(botAI);
    }
};

#endif
