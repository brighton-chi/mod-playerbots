/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCVALUECONTEXT_H
#define PLAYERBOTS_SSCVALUECONTEXT_H

#include "SSCHelpers.h"
#include "EncounterHelpers.h"
#include "NamedObjectContext.h"
#include "Value.h"
#include <string>
#include <vector>

class SscHazardPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    SscHazardPositionsValue(
        PlayerbotAI* botAI, std::string const& name, uint32 spellId, float searchRadius)
        : CalculatedValue<std::vector<Position>>(
              botAI, name, SscHelpers::HAZARD_CACHE_INTERVAL_MS),
          _spellId(spellId), _searchRadius(searchRadius) {}

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(bot, _searchRadius, _spellId);
    }

private:
    uint32 const _spellId;
    float const _searchRadius;
};

class SscWaterElementalTotemValue : public CalculatedValue<ObjectGuid>
{
public:
    SscWaterElementalTotemValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "ssc water elemental totem",
              SscHelpers::WATER_ELEMENTAL_TOTEM_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SscHelpers::FindWaterElementalTotemGuid(bot); }
};

class SscLurkerGuardianTanksValue : public CalculatedValue<GuidVector>
{
public:
    SscLurkerGuardianTanksValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "ssc lurker guardian tanks",
              SscHelpers::LURKER_GUARDIAN_TANK_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SscHelpers::FindLurkerGuardianTankGuids(bot); }
};

class RaidSscValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSscValueContext()
    {
        creators["ssc toxic pool"] = &RaidSscValueContext::ssc_toxic_pool;
        creators["ssc water elemental totem"] = &RaidSscValueContext::ssc_water_elemental_totem;
        creators["ssc lurker guardian tanks"] = &RaidSscValueContext::ssc_lurker_guardian_tanks;
        creators["ssc toxic spores"] = &RaidSscValueContext::ssc_toxic_spores;
    }

private:
    static UntypedValue* ssc_toxic_pool(PlayerbotAI* botAI)
    {
        return new SscHazardPositionsValue(
            botAI, "ssc toxic pool", SscHelpers::Id(SscHelpers::SscSpells::SPELL_TOXIC_POOL),
            SscHelpers::TOXIC_POOL_SEARCH_RADIUS);
    }
    static UntypedValue* ssc_water_elemental_totem(PlayerbotAI* botAI)
    {
        return new SscWaterElementalTotemValue(botAI);
    }
    static UntypedValue* ssc_lurker_guardian_tanks(PlayerbotAI* botAI)
    {
        return new SscLurkerGuardianTanksValue(botAI);
    }
    static UntypedValue* ssc_toxic_spores(PlayerbotAI* botAI)
    {
        return new SscHazardPositionsValue(
            botAI, "ssc toxic spores", SscHelpers::Id(SscHelpers::SscSpells::SPELL_TOXIC_SPORES),
            SscHelpers::TOXIC_SPORES_SEARCH_RADIUS);
    }
};

#endif
