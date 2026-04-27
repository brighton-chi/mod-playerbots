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

// ── Spec profiles ────────────────────────────────────────────────
// Each value identifies a unique row in the blessing priority table.
enum SpecProfile : uint8
{
    SPEC_PROT_WARRIOR          = 0,
    SPEC_TANK_DK               = 1,
    SPEC_DPS_WARRIOR           = 2,
    SPEC_DPS_DK                = 3,
    SPEC_CASTER_SHAMAN         = 4,
    SPEC_ENHANCE_SHAMAN        = 5,
    SPEC_RET_PALADIN           = 6,
    SPEC_HOLY_PALADIN          = 7,
    SPEC_PROT_PALADIN          = 8,
    SPEC_BEAR_DRUID            = 9,
    SPEC_CAT_DRUID             = 10,
    SPEC_CASTER_DRUID          = 11,
    SPEC_ROGUE                 = 12,
    SPEC_HUNTER                = 13,
    SPEC_MAGE                  = 14,
    SPEC_WARLOCK               = 15,
    SPEC_PRIEST                = 16,

    SPEC_PROFILE_COUNT         = 17
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
//
// clang-format off
inline constexpr BaseBlessingPriorityEntry BASE_BLESSING_PRIORITIES[SPEC_PROFILE_COUNT] =
{
    // SPEC_PROT_WARRIOR
    {{ BASE_KINGS,     BASE_MIGHT,  BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_TANK_DK
    {{ BASE_KINGS,     BASE_MIGHT,  BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_DPS_WARRIOR
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_DPS_DK
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_CASTER_SHAMAN
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_MIGHT     }},
    // SPEC_ENHANCE_SHAMAN
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
    // SPEC_RET_PALADIN
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
    // SPEC_HOLY_PALADIN
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_MIGHT     }},
    // SPEC_PROT_PALADIN
    {{ BASE_SANCTUARY, BASE_MIGHT,  BASE_WISDOM,    BASE_KINGS     }},
    // SPEC_BEAR_DRUID
    {{ BASE_KINGS,     BASE_MIGHT,  BASE_WISDOM,    BASE_SANCTUARY }},
    // SPEC_CAT_DRUID
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
    // SPEC_CASTER_DRUID
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_MIGHT     }},
    // SPEC_ROGUE
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_HUNTER
    {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
    // SPEC_MAGE
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_WARLOCK
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_NONE      }},
    // SPEC_PRIEST
    {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_NONE      }},
};

// ── Spec profile resolution ──────────────────────────────────────
// Maps a player to their SpecProfile based on class, talent tree, and tank role.

constexpr uint32 SPELL_IMPROVED_MIGHT_R1             = 20042;
constexpr uint32 SPELL_IMPROVED_MIGHT_R2             = 20045;
constexpr uint32 SPELL_IMPROVED_WISDOM_R1            = 20244;
constexpr uint32 SPELL_IMPROVED_WISDOM_R2            = 20245;
constexpr uint32 SPELL_BLESSING_OF_SANCTUARY         = 20911;
constexpr uint32 SPELL_GREATER_BLESSING_OF_SANCTUARY = 25899;
constexpr uint32 SPELL_DK_FROST_PRESENCE             = 48263;

inline SpecProfile ResolveSpecProfile(Player* player)
{
    if (!player)
        return SPEC_PRIEST;

    uint8 cls = player->getClass();
    int tab = AiFactory::GetPlayerSpecTab(player);

    switch (cls)
    {
        case CLASS_WARRIOR:
            if (tab == WARRIOR_TAB_PROTECTION)
                return SPEC_PROT_WARRIOR;
            return SPEC_DPS_WARRIOR;

        case CLASS_DEATH_KNIGHT:
            if (tab == DEATH_KNIGHT_TAB_BLOOD || player->HasAura(SPELL_DK_FROST_PRESENCE))
                return SPEC_TANK_DK;
            return SPEC_DPS_DK;

        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
                return SPEC_ENHANCE_SHAMAN;
            return SPEC_CASTER_SHAMAN;

        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
                return SPEC_HOLY_PALADIN;
            if (tab == PALADIN_TAB_PROTECTION)
                return SPEC_PROT_PALADIN;
            return SPEC_RET_PALADIN;

        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
            {
                if (player->HasTankSpec())
                    return SPEC_BEAR_DRUID;
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
                {
                    if (botAI->HasStrategy("bear", BOT_STATE_NON_COMBAT) ||
                        botAI->HasStrategy("bear", BOT_STATE_COMBAT) ||
                        botAI->HasStrategy("tank", BOT_STATE_NON_COMBAT) ||
                        botAI->HasStrategy("tank", BOT_STATE_COMBAT) ||
                        botAI->HasStrategy("tank face", BOT_STATE_NON_COMBAT) ||
                        botAI->HasStrategy("tank face", BOT_STATE_COMBAT))
                        return SPEC_BEAR_DRUID;
                }
                return SPEC_CAT_DRUID;
            }
            return SPEC_CASTER_DRUID;

        case CLASS_ROGUE:
            return SPEC_ROGUE;

        case CLASS_HUNTER:
            return SPEC_HUNTER;

        case CLASS_MAGE:
            return SPEC_MAGE;

        case CLASS_WARLOCK:
            return SPEC_WARLOCK;

        case CLASS_PRIEST:
            return SPEC_PRIEST;

        default:
            return SPEC_PRIEST;
    }
}

inline bool HasImprovedMight(Player* player)
{
    return player && (player->HasAura(SPELL_IMPROVED_MIGHT_R1) ||
                      player->HasAura(SPELL_IMPROVED_MIGHT_R2));
}

inline bool HasImprovedWisdom(Player* player)
{
    return player && (player->HasAura(SPELL_IMPROVED_WISDOM_R1) ||
                      player->HasAura(SPELL_IMPROVED_WISDOM_R2));
}

inline bool KnowsSanctuary(Player* player)
{
    return player && player->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
}

}

#endif
