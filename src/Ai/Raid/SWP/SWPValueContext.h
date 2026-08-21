/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "SWPEncounter_Twins.h"
#include "Value.h"
#include "Position.h"
#include <vector>

// Cache positions, never object pointers. CalculatedValue holds whatever Calculate() returned until
// the interval lapses, and only UnitCalculatedValue::Get() carries an IsInWorld guard - RefGet and
// every non-Unit specialisation have none, so a stored Creature*/GameObject* would be a dangling
// dereference waiting on a despawn.
class EredarTwinsBlazePositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    EredarTwinsBlazePositionsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "eredar twins blaze", SwpHelpers::EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return SwpHelpers::FindEredarTwinsBlazePositions(bot);
    }
};

class RaidSunwellValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSunwellValueContext()
    {
        creators["eredar twins blaze"] = &RaidSunwellValueContext::eredar_twins_blaze;
    }

private:
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI) {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
};

#endif
