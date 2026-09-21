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

class SscLurkerGuardiansValue : public CalculatedValue<GuidVector>
{
public:
    SscLurkerGuardiansValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "ssc lurker guardians", SscHelpers::LURKER_GUARDIAN_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SscHelpers::FindLurkerGuardianGuids(bot); }
};

class SscLeotherasValue : public CalculatedValue<ObjectGuid>
{
public:
    SscLeotherasValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "ssc leotheras", SscHelpers::LEOTHERAS_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SscHelpers::FindLeotherasGuid(bot); }
};

class SscShadowOfLeotherasValue : public CalculatedValue<ObjectGuid>
{
public:
    SscShadowOfLeotherasValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "ssc shadow of leotheras", SscHelpers::LEOTHERAS_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SscHelpers::FindShadowOfLeotherasGuid(bot); }
};

class RaidSscValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSscValueContext()
    {
        creators["ssc toxic pool"] = &RaidSscValueContext::ssc_toxic_pool;
        creators["ssc lurker guardians"] = &RaidSscValueContext::ssc_lurker_guardians;
        creators["ssc leotheras"] = &RaidSscValueContext::ssc_leotheras;
        creators["ssc shadow of leotheras"] = &RaidSscValueContext::ssc_shadow_of_leotheras;
    }

private:
    static UntypedValue* ssc_toxic_pool(PlayerbotAI* botAI) {
        return new SscHazardPositionsValue(
            botAI, "ssc toxic pool", SscHelpers::Id(SscHelpers::SscSpells::SPELL_TOXIC_POOL),
            SscHelpers::TOXIC_POOL_SEARCH_RADIUS);
    }
    static UntypedValue* ssc_lurker_guardians(PlayerbotAI* botAI) {
        return new SscLurkerGuardiansValue(botAI);
    }
    static UntypedValue* ssc_leotheras(PlayerbotAI* botAI) {
        return new SscLeotherasValue(botAI);
    }
    static UntypedValue* ssc_shadow_of_leotheras(PlayerbotAI* botAI) {
        return new SscShadowOfLeotherasValue(botAI);
    }
};

#endif
