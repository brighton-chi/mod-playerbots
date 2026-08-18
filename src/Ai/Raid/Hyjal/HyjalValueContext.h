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

using HyjalHelpers::HyjalSpells;

// Anetheron's Infernals, oldest first. Cached because the grid search behind it is not something
// every trigger and multiplier should be repeating each tick for every bot in the raid
class HyjalInfernalsValue : public CalculatedValue<GuidVector>
{
public:
    HyjalInfernalsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "hyjal infernals", 200) {}

protected:
    GuidVector Calculate() override { return HyjalHelpers::FindInfernalGuids(bot); }
};

// The ground hazards, cached for the same reason the Infernals are, only more so: Engine applies
// every multiplier to every action it pulls from the queue each tick, so a grid search inside a
// multiplier body is paid once per action rather than once per bot. Between the triggers, the
// multipliers and the actions, each of these was being searched for several times a tick.
//
// Always searched at HAZARD_SEARCH_RADIUS so one cached set serves every caller's radius; the
// helpers narrow it themselves. 200ms of staleness is inside human reaction time
class HyjalHazardPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    HyjalHazardPositionsValue(PlayerbotAI* botAI, std::string const& name, uint32 spellId)
        : CalculatedValue<std::vector<Position>>(botAI, name, 200), _spellId(spellId) {}

protected:
    std::vector<Position> Calculate() override
    {
        return GetDynamicObjectPositions(
            bot, HyjalHelpers::HAZARD_SEARCH_RADIUS, _spellId);
    }

private:
    uint32 const _spellId;
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
            botAI, "hyjal death and decay", HyjalHelpers::Id(HyjalSpells::SPELL_DEATH_AND_DECAY));
    }
    static UntypedValue* hyjal_rain_of_fire(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal rain of fire", HyjalHelpers::Id(HyjalSpells::SPELL_RAIN_OF_FIRE));
    }
    static UntypedValue* hyjal_doomfire_trail(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal doomfire trail", HyjalHelpers::Id(HyjalSpells::SPELL_DOOMFIRE_TRAIL));
    }
};

#endif
