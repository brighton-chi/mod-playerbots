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

// Jan'alai drops 40 bombs across the platform at once, and four separate places ask about them
// every tick - three triggers plus the movement multiplier, which runs per candidate action. Each
// of those was its own grid search; caching turns them into one.
class JanalaiFireBombsValue : public CalculatedValue<GuidVector>
{
public:
    JanalaiFireBombsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "jan'alai fire bombs", ZaHelpers::FIRE_BOMB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return ZaHelpers::FindNearbyFireBombGuids(bot); }
};

class RaidZulAmanValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidZulAmanValueContext()
    {
        creators["jan'alai fire bombs"] = &RaidZulAmanValueContext::janalai_fire_bombs;
    }

private:
    static UntypedValue* janalai_fire_bombs(PlayerbotAI* botAI) {
        return new JanalaiFireBombsValue(botAI);
    }
};

#endif
