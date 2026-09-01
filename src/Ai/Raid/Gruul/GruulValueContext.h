/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULVALUECONTEXT_H
#define PLAYERBOTS_GRUULVALUECONTEXT_H

#include "GruulHelpers.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"

// Olm summons a Wild Fel Stalker every 48.5s, and both the spawn trigger and the banish action ask
// about them every tick, per warlock. Each of those was its own grid search - the most expensive
// check in the module - and the two did not even agree: the trigger asked "find target", which
// only walks the asking bot's own threat list, so a stalker chasing anyone other than that warlock
// was invisible to it and never got banished. One cached, canonically ordered list serves both.
class HighKingMaulgarWildFelStalkersValue : public CalculatedValue<GuidVector>
{
public:
    HighKingMaulgarWildFelStalkersValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "high king maulgar wild fel stalkers",
              GruulHelpers::WILD_FEL_STALKER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return GruulHelpers::FindNearbyWildFelStalkerGuids(bot); }
};

class RaidGruulsLairValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidGruulsLairValueContext()
    {
        creators["high king maulgar wild fel stalkers"] =
            &RaidGruulsLairValueContext::high_king_maulgar_wild_fel_stalkers;
    }

private:
    static UntypedValue* high_king_maulgar_wild_fel_stalkers(PlayerbotAI* botAI) {
        return new HighKingMaulgarWildFelStalkersValue(botAI);
    }
};

#endif
