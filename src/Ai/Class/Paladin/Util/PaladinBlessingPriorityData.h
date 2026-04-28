/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINBLESSINGPRIORITYDATA_H
#define _PLAYERBOT_PALADINBLESSINGPRIORITYDATA_H

#include <array>
#include <string>

#include "AiFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace ai::gbless
{

// ── Blessing recipient roles ─────────────────────────────────────
// Each value identifies a unique row in the blessing priority table.
enum RoleProfile : uint8
{
    ROLE_CASTER                = 0,
    ROLE_PHYSICAL_DPS          = 1,
    ROLE_HYBRID_DPS            = 2,
    ROLE_DRUID_TANK            = 3,
    ROLE_WARRIOR_DK_TANK       = 4,
    ROLE_PALADIN_TANK          = 5,

    ROLE_PROFILE_COUNT         = 6
};

// ── Blessing types ───────────────────────────────────────────────
enum BlessingType : uint8
{
    BLESSING_NONE              = 0,
    BLESSING_MIGHT_SINGLE      = 1,
    BLESSING_MIGHT_GREATER     = 2,
    BLESSING_WISDOM_SINGLE     = 3,
    BLESSING_WISDOM_GREATER    = 4,
    BLESSING_KINGS_SINGLE      = 5,
    BLESSING_KINGS_GREATER     = 6,
    BLESSING_SANCTUARY_SINGLE  = 7,
    BLESSING_SANCTUARY_GREATER = 8
};

// ── Base blessing category (ignoring single/greater) ─────────────
enum BaseBlessingCategory : uint8
{
    BASE_NONE      = 0,
    BASE_MIGHT     = 1,
    BASE_WISDOM    = 2,
    BASE_KINGS     = 3,
    BASE_SANCTUARY = 4
};

// ── Classification helpers ───────────────────────────────────────

inline constexpr BaseBlessingCategory BaseBlessingOf(BlessingType type)
{
    switch (type)
    {
        case BLESSING_MIGHT_SINGLE:
        case BLESSING_MIGHT_GREATER:      return BASE_MIGHT;
        case BLESSING_WISDOM_SINGLE:
        case BLESSING_WISDOM_GREATER:     return BASE_WISDOM;
        case BLESSING_KINGS_SINGLE:
        case BLESSING_KINGS_GREATER:      return BASE_KINGS;
        case BLESSING_SANCTUARY_SINGLE:
        case BLESSING_SANCTUARY_GREATER:  return BASE_SANCTUARY;
        default:                          return BASE_NONE;
    }
}

inline constexpr bool IsSingleVariant(BlessingType type)
{
    return type == BLESSING_MIGHT_SINGLE || type == BLESSING_WISDOM_SINGLE ||
           type == BLESSING_KINGS_SINGLE || type == BLESSING_SANCTUARY_SINGLE;
}

inline constexpr bool IsGreaterVariant(BlessingType type)
{
    return type == BLESSING_MIGHT_GREATER || type == BLESSING_WISDOM_GREATER ||
           type == BLESSING_KINGS_GREATER || type == BLESSING_SANCTUARY_GREATER;
}

inline constexpr BlessingType ToSingleVariant(BaseBlessingCategory category)
{
    switch (category)
    {
        case BASE_MIGHT:     return BLESSING_MIGHT_SINGLE;
        case BASE_WISDOM:    return BLESSING_WISDOM_SINGLE;
        case BASE_KINGS:     return BLESSING_KINGS_SINGLE;
        case BASE_SANCTUARY: return BLESSING_SANCTUARY_SINGLE;
        default:             return BLESSING_NONE;
    }
}

inline constexpr BlessingType ToSingleVariant(BlessingType type)
{
    return ToSingleVariant(BaseBlessingOf(type));
}

inline constexpr BlessingType ToGreaterVariant(BaseBlessingCategory category)
{
    switch (category)
    {
        case BASE_MIGHT:     return BLESSING_MIGHT_GREATER;
        case BASE_WISDOM:    return BLESSING_WISDOM_GREATER;
        case BASE_KINGS:     return BLESSING_KINGS_GREATER;
        case BASE_SANCTUARY: return BLESSING_SANCTUARY_GREATER;
        default:             return BLESSING_NONE;
    }
}

inline constexpr BlessingType ToGreaterVariant(BlessingType type)
{
    return ToGreaterVariant(BaseBlessingOf(type));
}

// ── Spell name lookup ────────────────────────────────────────────

inline std::string BlessingSpellName(BlessingType type)
{
    switch (type)
    {
        case BLESSING_MIGHT_SINGLE:      return "blessing of might";
        case BLESSING_MIGHT_GREATER:     return "greater blessing of might";
        case BLESSING_WISDOM_SINGLE:     return "blessing of wisdom";
        case BLESSING_WISDOM_GREATER:    return "greater blessing of wisdom";
        case BLESSING_KINGS_SINGLE:      return "blessing of kings";
        case BLESSING_KINGS_GREATER:     return "greater blessing of kings";
        case BLESSING_SANCTUARY_SINGLE:  return "blessing of sanctuary";
        case BLESSING_SANCTUARY_GREATER: return "greater blessing of sanctuary";
        default:                         return "";
    }
}

// ── Base priority list entry ─────────────────────────────────────

struct BaseBlessingPriorityEntry
{
    BaseBlessingCategory priorities[4];
};

// Ordered from highest to lowest priority for each bucket.
inline constexpr BaseBlessingPriorityEntry BASE_BLESSING_PRIORITIES[ROLE_PROFILE_COUNT] =
{
    // ROLE_CASTER
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_MIGHT     }},
    // ROLE_PHYSICAL_DPS
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_SANCTUARY, BASE_NONE      }},
    // ROLE_HYBRID_DPS
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
    // ROLE_DRUID_TANK
    {{ BASE_KINGS,     BASE_MIGHT,  BASE_WISDOM,    BASE_SANCTUARY }},
    // ROLE_WARRIOR_DK_TANK
    {{ BASE_KINGS,     BASE_MIGHT,  BASE_SANCTUARY, BASE_NONE      }},
    // ROLE_PALADIN_TANK
    {{ BASE_SANCTUARY, BASE_MIGHT,  BASE_WISDOM,    BASE_KINGS     }},
};

// ── Role profile resolution ──────────────────────────────────────
// Maps a player to their blessing profile based on class and role.

constexpr uint32 SPELL_IMPROVED_MIGHT_R1             = 20042;
constexpr uint32 SPELL_IMPROVED_MIGHT_R2             = 20045;
constexpr uint32 SPELL_IMPROVED_WISDOM_R1            = 20244;
constexpr uint32 SPELL_IMPROVED_WISDOM_R2            = 20245;
constexpr uint32 SPELL_BLESSING_OF_SANCTUARY         = 20911;
constexpr uint32 SPELL_GREATER_BLESSING_OF_SANCTUARY = 25899;

inline RoleProfile ResolveRoleProfile(Player* player)
{
    if (!player)
        return ROLE_CASTER;

    uint8 cls = player->getClass();
    int tab = AiFactory::GetPlayerSpecTab(player);
    bool isTank = PlayerbotAI::IsTank(player);

    switch (cls)
    {
        case CLASS_WARRIOR:
            if (isTank)
                return ROLE_WARRIOR_DK_TANK;
            return ROLE_PHYSICAL_DPS;

        case CLASS_DEATH_KNIGHT:
            if (isTank)
                return ROLE_WARRIOR_DK_TANK;
            return ROLE_PHYSICAL_DPS;

        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
                return ROLE_HYBRID_DPS;
            return ROLE_CASTER;

        case CLASS_PALADIN:
            if (isTank)
                return ROLE_PALADIN_TANK;
            if (tab == PALADIN_TAB_HOLY)
                return ROLE_CASTER;
            return ROLE_HYBRID_DPS;

        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
                return isTank ? ROLE_DRUID_TANK : ROLE_HYBRID_DPS;
            return ROLE_CASTER;

        case CLASS_ROGUE:
            return ROLE_PHYSICAL_DPS;

        case CLASS_HUNTER:
            return ROLE_HYBRID_DPS;

        case CLASS_MAGE:
            return ROLE_CASTER;

        case CLASS_WARLOCK:
            return ROLE_CASTER;

        case CLASS_PRIEST:
            return ROLE_CASTER;

        default:
            return ROLE_CASTER;
    }
}

}

#endif
