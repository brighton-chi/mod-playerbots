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

// Trash

class ShadowmoonReaversValue : public CalculatedValue<GuidVector>
{
public:
    ShadowmoonReaversValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "shadowmoon reavers",
              BtHelpers::SHADOWMOON_REAVER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindShadowmoonReaverGuids(botAI); }
};

// Supremus

// Positions, not units, so nothing dangles when a volcano despawns.
class SupremusHazardsValue : public CalculatedValue<std::vector<BtHelpers::SupremusHazard>>
{
public:
    SupremusHazardsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<BtHelpers::SupremusHazard>>(
              botAI, "supremus hazards", BtHelpers::SUPREMUS_HAZARD_CACHE_INTERVAL_MS) {}

protected:
    std::vector<BtHelpers::SupremusHazard> Calculate() override
    {
        return BtHelpers::FindSupremusHazards(botAI);
    }
};

// Shade of Akama

class ShadeOfAkamaAddsValue : public CalculatedValue<GuidVector>
{
public:
    ShadeOfAkamaAddsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "shade of akama adds",
              BtHelpers::SHADE_OF_AKAMA_ADD_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindShadeOfAkamaAddGuids(botAI); }
};

// Teron Gorefiend

class ShadowyConstructsValue : public CalculatedValue<GuidVector>
{
public:
    ShadowyConstructsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "shadowy constructs",
              BtHelpers::GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindShadowyConstructGuids(botAI); }
};

// Gurtogg Bloodboil

class GurtoggBloodboilSecondTankThreatValue : public FloatCalculatedValue
{
public:
    GurtoggBloodboilSecondTankThreatValue(PlayerbotAI* botAI)
        : FloatCalculatedValue(
              botAI, "gurtogg bloodboil second tank threat",
              BtHelpers::GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS) {}

protected:
    float Calculate() override { return BtHelpers::FindGurtoggSecondTankThreat(botAI); }
};

// Illidari Council

class IllidariCouncilZerevorMageTankValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidariCouncilZerevorMageTankValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidari council zerevor mage tank",
              BtHelpers::ZEREVOR_MAGE_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return BtHelpers::FindZerevorMageTankGuid(bot); }
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageWarlockTankValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidanStormrageWarlockTankValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidan stormrage warlock tank",
              BtHelpers::ILLIDAN_WARLOCK_TANK_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return BtHelpers::FindIllidanWarlockTankGuid(bot); }
};

class IllidanStormrageBotWithParasiticShadowfiendValue : public CalculatedValue<ObjectGuid>
{
public:
    IllidanStormrageBotWithParasiticShadowfiendValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "illidan stormrage bot with parasitic shadowfiend",
              BtHelpers::PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override
    {
        return BtHelpers::FindBotWithParasiticShadowfiendGuid(bot);
    }
};

class RaidBlackTempleValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidBlackTempleValueContext()
    {
        creators["shadowmoon reavers"] = &RaidBlackTempleValueContext::shadowmoon_reavers;
        creators["supremus hazards"] = &RaidBlackTempleValueContext::supremus_hazards;
        creators["shade of akama adds"] = &RaidBlackTempleValueContext::shade_of_akama_adds;
        creators["shadowy constructs"] = &RaidBlackTempleValueContext::shadowy_constructs;
        creators["gurtogg bloodboil second tank threat"] =
            &RaidBlackTempleValueContext::gurtogg_bloodboil_second_tank_threat;
        creators["illidari council zerevor mage tank"] =
            &RaidBlackTempleValueContext::illidari_council_zerevor_mage_tank;
        creators["illidan stormrage warlock tank"] =
            &RaidBlackTempleValueContext::illidan_stormrage_warlock_tank;
        creators["illidan stormrage bot with parasitic shadowfiend"] =
            &RaidBlackTempleValueContext::illidan_stormrage_bot_with_parasitic_shadowfiend;
    }

private:
    static UntypedValue* shadowmoon_reavers(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaversValue(botAI);
    }
    static UntypedValue* supremus_hazards(PlayerbotAI* botAI)
    {
        return new SupremusHazardsValue(botAI);
    }
    static UntypedValue* shade_of_akama_adds(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaAddsValue(botAI);
    }
    static UntypedValue* shadowy_constructs(PlayerbotAI* botAI)
    {
        return new ShadowyConstructsValue(botAI);
    }
    static UntypedValue* gurtogg_bloodboil_second_tank_threat(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilSecondTankThreatValue(botAI);
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
