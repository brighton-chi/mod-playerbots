/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALVALUECONTEXT_H
#define PLAYERBOTS_HYJALVALUECONTEXT_H

#include "HyjalHelpers.h"
#include "NamedObjectContext.h"
#include "RaidBossHelpers.h"
#include "Value.h"
#include <string>
#include <vector>

using HyjalHelpers::HyjalSpells;

class HyjalInfernalsValue : public CalculatedValue<GuidVector>
{
public:
    HyjalInfernalsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "hyjal infernals", 200) {}

protected:
    GuidVector Calculate() override { return HyjalHelpers::FindInfernalGuids(bot); }
};

class HyjalHazardPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    HyjalHazardPositionsValue(
        PlayerbotAI* botAI, std::string const& name, uint32 spellId, float searchRadius)
        : CalculatedValue<std::vector<Position>>(
              botAI, name, HyjalHelpers::HAZARD_CACHE_INTERVAL),
          _spellId(spellId), _searchRadius(searchRadius) {}

protected:
    std::vector<Position> Calculate() override
    {
        return GetDynamicObjectPositions(bot, _searchRadius, _spellId);
    }

private:
    uint32 const _spellId;
    float const _searchRadius;
};

class RaidHyjalSummitValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidHyjalSummitValueContext()
    {
        creators["hyjal infernals"] = &RaidHyjalSummitValueContext::hyjal_infernals;
        creators["hyjal death and decay"] = &RaidHyjalSummitValueContext::hyjal_death_and_decay;
        creators["hyjal rain of fire"] = &RaidHyjalSummitValueContext::hyjal_rain_of_fire;
        creators["hyjal doomfire trail"] = &RaidHyjalSummitValueContext::hyjal_doomfire_trail;
    }

private:
    static UntypedValue* hyjal_infernals(PlayerbotAI* botAI) {
        return new HyjalInfernalsValue(botAI);
    }
    static UntypedValue* hyjal_death_and_decay(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal death and decay", HyjalHelpers::Id(HyjalSpells::SPELL_DEATH_AND_DECAY),
            HyjalHelpers::DEATH_AND_DECAY_SEARCH_RADIUS);
    }
    static UntypedValue* hyjal_rain_of_fire(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal rain of fire", HyjalHelpers::Id(HyjalSpells::SPELL_RAIN_OF_FIRE),
            HyjalHelpers::RAIN_OF_FIRE_SEARCH_RADIUS);
    }
    static UntypedValue* hyjal_doomfire_trail(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal doomfire trail", HyjalHelpers::Id(HyjalSpells::SPELL_DOOMFIRE_TRAIL),
            HyjalHelpers::DOOMFIRE_SEARCH_RADIUS);
    }
};

#endif
