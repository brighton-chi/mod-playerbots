/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "SWPActions.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Kalec.h"
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

// The pools are static and permanent, so this is a straight cost saving: the trigger and the action
// both ask every tick for the whole Entropius phase, and the room keeps accumulating them.
class MuruVoidZonesValue : public CalculatedValue<GuidVector>
{
public:
    MuruVoidZonesValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "muru void zones", SwpHelpers::MURU_VOID_ZONE_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindMuruVoidZoneGuids(bot); }
};

// The four values below all replace a grid search that a trigger ran and then the action it gates
// ran again with the same entry and the same radius. Guids only, so a despawn inside the window
// resolves to null at the call site rather than dangling. FindNearestCreature filters the dead by
// default, which caching a guid would otherwise discard, so the creature values are re-tested with
// IsAlive() where they are resolved.
class SwpVolatileFiendValue : public CalculatedValue<ObjectGuid>
{
public:
    SwpVolatileFiendValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "swp volatile fiend", SwpHelpers::SWP_VOLATILE_FIEND_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindSwpVolatileFiendGuid(bot); }
};

class KalecgosSpectralRiftValue : public CalculatedValue<ObjectGuid>
{
public:
    KalecgosSpectralRiftValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "kalecgos spectral rift",
              SwpHelpers::KALECGOS_SPECTRAL_RIFT_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindKalecgosSpectralRiftGuid(bot); }
};

class MuruSingularityValue : public CalculatedValue<ObjectGuid>
{
public:
    MuruSingularityValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "muru singularity", SwpHelpers::MURU_SINGULARITY_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindMuruSingularityGuid(bot); }
};

class KiljaedenDragonOrbsValue : public CalculatedValue<GuidVector>
{
public:
    KiljaedenDragonOrbsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "kiljaeden dragon orbs",
              SwpHelpers::KILJAEDEN_DRAGON_ORB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindKiljaedenDragonOrbGuids(bot); }
};

class RaidSunwellValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSunwellValueContext()
    {
        creators["eredar twins blaze"] = &RaidSunwellValueContext::eredar_twins_blaze;
        creators["muru encounter targets"] = &RaidSunwellValueContext::muru_encounter_targets;
        creators["muru void zones"] = &RaidSunwellValueContext::muru_void_zones;
        creators["swp volatile fiend"] = &RaidSunwellValueContext::swp_volatile_fiend;
        creators["kalecgos spectral rift"] = &RaidSunwellValueContext::kalecgos_spectral_rift;
        creators["muru singularity"] = &RaidSunwellValueContext::muru_singularity;
        creators["kiljaeden dragon orbs"] = &RaidSunwellValueContext::kiljaeden_dragon_orbs;
    }

private:
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI) {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
    static UntypedValue* muru_encounter_targets(PlayerbotAI* botAI) {
        return new MuruEncounterTargetsValue(botAI);
    }
    static UntypedValue* muru_void_zones(PlayerbotAI* botAI) {
        return new MuruVoidZonesValue(botAI);
    }
    static UntypedValue* swp_volatile_fiend(PlayerbotAI* botAI) {
        return new SwpVolatileFiendValue(botAI);
    }
    static UntypedValue* kalecgos_spectral_rift(PlayerbotAI* botAI) {
        return new KalecgosSpectralRiftValue(botAI);
    }
    static UntypedValue* muru_singularity(PlayerbotAI* botAI) {
        return new MuruSingularityValue(botAI);
    }
    static UntypedValue* kiljaeden_dragon_orbs(PlayerbotAI* botAI) {
        return new KiljaedenDragonOrbsValue(botAI);
    }
};

#endif
