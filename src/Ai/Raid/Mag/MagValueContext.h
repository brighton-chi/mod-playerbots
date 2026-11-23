/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGVALUECONTEXT_H
#define PLAYERBOTS_MAGVALUECONTEXT_H

#include "MagHelpers.h"
#include "NamedObjectContext.h"
#include "Position.h"
#include "Value.h"
#include <vector>

class MagDebrisPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    MagDebrisPositionsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
            botAI, "mag debris positions", MagHelpers::MAG_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override { return MagHelpers::FindDebrisPositions(bot); }
};

class RaidMagtheridonValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidMagtheridonValueContext()
    {
        creators["mag debris positions"] = &RaidMagtheridonValueContext::mag_debris_positions;
    }

private:
    static UntypedValue* mag_debris_positions(PlayerbotAI* botAI)
    {
        return new MagDebrisPositionsValue(botAI);
    }
};

#endif
