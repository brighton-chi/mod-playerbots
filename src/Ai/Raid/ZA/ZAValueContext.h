/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAVALUECONTEXT_H
#define PLAYERBOTS_ZAVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"
#include "ZAHelpers.h"

class JanalaiFireBombsValue : public CalculatedValue<GuidVector>
{
public:
    JanalaiFireBombsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "jan'alai fire bombs", ZaHelpers::FIRE_BOMB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return ZaHelpers::FindNearbyFireBombGuids(bot); }
};

class JanalaiAddsValue : public CalculatedValue<GuidVector>
{
public:
    JanalaiAddsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "jan'alai adds", ZaHelpers::JANALAI_ADD_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return ZaHelpers::FindJanalaiAddGuids(bot); }
};

class HexLordMalacrassFreezingTrapValue : public ObjectGuidCalculatedValue
{
public:
    HexLordMalacrassFreezingTrapValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(
              botAI, "hex lord malacrass freezing trap",
              ZaHelpers::FREEZING_TRAP_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return ZaHelpers::FindNearbyFreezingTrapGuid(bot); }
};

class HexLordMalacrassDpsTargetsValue : public CalculatedValue<GuidVector>
{
public:
    HexLordMalacrassDpsTargetsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "hex lord malacrass dps targets",
              ZaHelpers::HEX_LORD_MALACRASS_DPS_TARGET_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return ZaHelpers::FindHexLordMalacrassDpsTargetGuids(bot); }
};

class RaidZulAmanValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidZulAmanValueContext()
    {
        creators["jan'alai fire bombs"] = &RaidZulAmanValueContext::janalai_fire_bombs;
        creators["jan'alai adds"] = &RaidZulAmanValueContext::janalai_adds;
        creators["hex lord malacrass freezing trap"] =
            &RaidZulAmanValueContext::hex_lord_malacrass_freezing_trap;
        creators["hex lord malacrass dps targets"] =
            &RaidZulAmanValueContext::hex_lord_malacrass_dps_targets;
    }

private:
    static UntypedValue* janalai_fire_bombs(PlayerbotAI* botAI)
    {
        return new JanalaiFireBombsValue(botAI);
    }

    static UntypedValue* janalai_adds(PlayerbotAI* botAI)
    {
        return new JanalaiAddsValue(botAI);
    }

    static UntypedValue* hex_lord_malacrass_freezing_trap(PlayerbotAI* botAI)
    {
        return new HexLordMalacrassFreezingTrapValue(botAI);
    }

    static UntypedValue* hex_lord_malacrass_dps_targets(PlayerbotAI* botAI)
    {
        return new HexLordMalacrassDpsTargetsValue(botAI);
    }
};

#endif
