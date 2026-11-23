/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULVALUECONTEXT_H
#define PLAYERBOTS_GRUULVALUECONTEXT_H

#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "NamedObjectContext.h"
#include "Value.h"
#include <vector>

class GruulTheDragonkillerCaveInValue : public CalculatedValue<std::vector<Position>>
{
public:
    GruulTheDragonkillerCaveInValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "gruul the dragonkiller cave in", GruulHelpers::CAVE_IN_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(
            bot, GruulHelpers::CAVE_IN_SEARCH_RADIUS,
            GruulHelpers::Id(GruulHelpers::GruulSpells::SPELL_CAVE_IN));
    }
};

class RaidGruulsLairValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidGruulsLairValueContext()
    {
        creators["gruul the dragonkiller cave in"] =
            &RaidGruulsLairValueContext::gruul_the_dragonkiller_cave_in;
    }

private:
    static UntypedValue* gruul_the_dragonkiller_cave_in(PlayerbotAI* botAI)
    {
        return new GruulTheDragonkillerCaveInValue(botAI);
    }
};

#endif
