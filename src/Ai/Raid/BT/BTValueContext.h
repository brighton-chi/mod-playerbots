/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTVALUECONTEXT_H
#define PLAYERBOTS_BTVALUECONTEXT_H

#include "BTHelpers.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"

// Supremus

class SupremusVolcanoesValue : public CalculatedValue<GuidVector>
{
public:
    SupremusVolcanoesValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "supremus volcanoes", BlackTempleHelpers::SUPREMUS_VOLCANO_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BlackTempleHelpers::FindSupremusVolcanoGuids(bot); }
};

// Illidari Council

class IllidariCouncilZerevorMageTankValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidariCouncilZerevorMageTankValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidari council zerevor mage tank",
              BlackTempleHelpers::ZEREVOR_MAGE_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return BlackTempleHelpers::FindZerevorMageTankGuid(bot); }
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageWarlockTankValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidanStormrageWarlockTankValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidan stormrage warlock tank",
              BlackTempleHelpers::ILLIDAN_WARLOCK_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return BlackTempleHelpers::FindIllidanWarlockTankGuid(bot); }
};

class IllidanStormrageBotWithParasiticShadowfiendValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidanStormrageBotWithParasiticShadowfiendValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidan stormrage bot with parasitic shadowfiend",
              BlackTempleHelpers::PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override
    {
        return BlackTempleHelpers::FindBotWithParasiticShadowfiendGuid(bot);
    }
};

class RaidBlackTempleValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidBlackTempleValueContext()
    {
        creators["supremus volcanoes"] = &RaidBlackTempleValueContext::supremus_volcanoes;
        creators["illidari council zerevor mage tank"] =
            &RaidBlackTempleValueContext::illidari_council_zerevor_mage_tank;
        creators["illidan stormrage warlock tank"] =
            &RaidBlackTempleValueContext::illidan_stormrage_warlock_tank;
        creators["illidan stormrage bot with parasitic shadowfiend"] =
            &RaidBlackTempleValueContext::illidan_stormrage_bot_with_parasitic_shadowfiend;
    }

private:
    static UntypedValue* supremus_volcanoes(PlayerbotAI* botAI)
    {
        return new SupremusVolcanoesValue(botAI);
    }
    static UntypedValue* illidari_council_zerevor_mage_tank(PlayerbotAI* botAI)
    {
        return new IllidariCouncilZerevorMageTankValue(botAI);
    }
    static UntypedValue* illidan_stormrage_warlock_tank(PlayerbotAI* botAI)
    {
        return new IllidanStormrageWarlockTankValue(botAI);
    }
    static UntypedValue* illidan_stormrage_bot_with_parasitic_shadowfiend(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBotWithParasiticShadowfiendValue(botAI);
    }
};

#endif
