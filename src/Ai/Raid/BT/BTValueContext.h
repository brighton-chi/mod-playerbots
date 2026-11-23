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
#include <vector>

// Trash

class SistersOfPleasureValue : public CalculatedValue<GuidVector>
{
public:
    SistersOfPleasureValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "sisters of pleasure", BtHelpers::SISTER_OF_PLEASURE_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindSisterOfPleasureGuids(bot); }
};

class ShadowmoonReaversValue : public CalculatedValue<GuidVector>
{
public:
    ShadowmoonReaversValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "shadowmoon reavers", BtHelpers::SHADOWMOON_REAVER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindAttackerGuidsWithEntry(
            botAI, BtHelpers::Id(BtHelpers::BtNpcs::NPC_SHADOWMOON_REAVER));
    }
};

// Supremus

class SupremusHazardsValue : public CalculatedValue<std::vector<BtHelpers::SupremusHazard>>
{
public:
    SupremusHazardsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<BtHelpers::SupremusHazard>>(
            botAI, "supremus hazards", BtHelpers::SUPREMUS_HAZARD_CACHE_INTERVAL_MS) {}

protected:
    std::vector<BtHelpers::SupremusHazard> Calculate() override
    {
        return BtHelpers::FindSupremusHazards(bot);
    }
};

// Teron Gorefiend

class TeronGorefiendShadowyConstructsValue : public CalculatedValue<GuidVector>
{
public:
    TeronGorefiendShadowyConstructsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "teron gorefiend shadowy constructs",
            BtHelpers::GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindAttackerGuidsWithEntry(
            botAI, BtHelpers::Id(BtHelpers::BtNpcs::NPC_SHADOWY_CONSTRUCT));
    }
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
    float Calculate() override { return BtHelpers::FindGurtoggSecondTankThreat(bot); }
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageInfectedPlayersValue : public CalculatedValue<GuidVector>
{
public:
    IllidanStormrageInfectedPlayersValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "illidan stormrage infected players",
            BtHelpers::ILLIDAN_PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindInfectedPlayerGuids(bot);
    }
};

class IllidanStormrageTrapHuntersValue : public CalculatedValue<GuidVector>
{
public:
    IllidanStormrageTrapHuntersValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "illidan stormrage trap hunters",
            BtHelpers::ILLIDAN_TRAP_HUNTER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindIllidanTrapHunterGuids(bot); }
};

class RaidBlackTempleValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidBlackTempleValueContext()
    {
        creators["sisters of pleasure"] = &RaidBlackTempleValueContext::sisters_of_pleasure;
        creators["shadowmoon reavers"] = &RaidBlackTempleValueContext::shadowmoon_reavers;
        creators["supremus hazards"] = &RaidBlackTempleValueContext::supremus_hazards;
        creators["teron gorefiend shadowy constructs"] =
            &RaidBlackTempleValueContext::teron_gorefiend_shadowy_constructs;
        creators["gurtogg bloodboil second tank threat"] =
            &RaidBlackTempleValueContext::gurtogg_bloodboil_second_tank_threat;
        creators["illidan stormrage infected players"] =
            &RaidBlackTempleValueContext::illidan_stormrage_infected_players;
        creators["illidan stormrage trap hunters"] =
            &RaidBlackTempleValueContext::illidan_stormrage_trap_hunters;
    }

private:
    static UntypedValue* sisters_of_pleasure(PlayerbotAI* botAI)
    {
        return new SistersOfPleasureValue(botAI);
    }
    static UntypedValue* shadowmoon_reavers(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaversValue(botAI);
    }
    static UntypedValue* supremus_hazards(PlayerbotAI* botAI)
    {
        return new SupremusHazardsValue(botAI);
    }
    static UntypedValue* teron_gorefiend_shadowy_constructs(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShadowyConstructsValue(botAI);
    }
    static UntypedValue* gurtogg_bloodboil_second_tank_threat(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilSecondTankThreatValue(botAI);
    }
    static UntypedValue* illidan_stormrage_infected_players(PlayerbotAI* botAI)
    {
        return new IllidanStormrageInfectedPlayersValue(botAI);
    }
    static UntypedValue* illidan_stormrage_trap_hunters(PlayerbotAI* botAI)
    {
        return new IllidanStormrageTrapHuntersValue(botAI);
    }
};

#endif
