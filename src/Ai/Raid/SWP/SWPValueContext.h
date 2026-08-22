/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "SWPEncounter_Muru.h"
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

// Collapses the four per-tick sweeps of "possible targets no los" this encounter used to run into
// one per interval. Guids only - see MuruEncounterGuids for why the resolved pointers stay local.
class MuruEncounterTargetsValue : public CalculatedValue<SwpHelpers::MuruEncounterGuids>
{
public:
    MuruEncounterTargetsValue(PlayerbotAI* botAI)
        : CalculatedValue<SwpHelpers::MuruEncounterGuids>(
              botAI, "muru encounter targets",
              SwpHelpers::MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS) {}

protected:
    SwpHelpers::MuruEncounterGuids Calculate() override
    {
        return SwpHelpers::FindMuruEncounterGuids(botAI);
    }
};

class RaidSunwellValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSunwellValueContext()
    {
        creators["eredar twins blaze"] = &RaidSunwellValueContext::eredar_twins_blaze;
        creators["muru encounter targets"] = &RaidSunwellValueContext::muru_encounter_targets;
    }

private:
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI) {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
    static UntypedValue* muru_encounter_targets(PlayerbotAI* botAI) {
        return new MuruEncounterTargetsValue(botAI);
    }
};

#endif
