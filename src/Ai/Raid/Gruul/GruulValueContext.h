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

// Both ogre caster tanks are chosen by walking the raid; the moonkin walk additionally runs a
// talent scan per druid. Caching the answer as a guid rather than a pointer keeps a tank who logs
// out or dies inside the interval from being handed back as a dangling Player*.
class HighKingMaulgarKroshMageTankValue : public ObjectGuidCalculatedValue
{
public:
    HighKingMaulgarKroshMageTankValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(
              botAI, "high king maulgar krosh mage tank",
              GruulHelpers::CASTER_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return GruulHelpers::FindKroshMageTankGuid(bot); }
};

class HighKingMaulgarKigglerMoonkinTankValue : public ObjectGuidCalculatedValue
{
public:
    HighKingMaulgarKigglerMoonkinTankValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(
              botAI, "high king maulgar kiggler moonkin tank",
              GruulHelpers::CASTER_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return GruulHelpers::FindKigglerMoonkinTankGuid(bot); }
};

class RaidGruulsLairValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidGruulsLairValueContext()
    {
        creators["high king maulgar wild fel stalkers"] =
            &RaidGruulsLairValueContext::high_king_maulgar_wild_fel_stalkers;
        creators["high king maulgar krosh mage tank"] =
            &RaidGruulsLairValueContext::high_king_maulgar_krosh_mage_tank;
        creators["high king maulgar kiggler moonkin tank"] =
            &RaidGruulsLairValueContext::high_king_maulgar_kiggler_moonkin_tank;
    }

private:
    static UntypedValue* high_king_maulgar_wild_fel_stalkers(PlayerbotAI* botAI) {
        return new HighKingMaulgarWildFelStalkersValue(botAI);
    }

    static UntypedValue* high_king_maulgar_krosh_mage_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKroshMageTankValue(botAI);
    }

    static UntypedValue* high_king_maulgar_kiggler_moonkin_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKigglerMoonkinTankValue(botAI);
    }
};

#endif
