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
    SPEC_PROT_WARRIOR_TANK_DK  = 0,
    SPEC_DPS_WARRIOR_DPS_DK    = 1,
    SPEC_CASTER_SHAMAN         = 2,
    SPEC_ENHANCE_SHAMAN        = 3,
    SPEC_RET_PALADIN           = 4,
    SPEC_HOLY_PALADIN          = 5,
    SPEC_PROT_PALADIN          = 6,
    SPEC_BEAR_DRUID            = 7,
    SPEC_CAT_DRUID             = 8,
    SPEC_CASTER_DRUID          = 9,
    SPEC_ROGUE                 = 10,
    SPEC_HUNTER                = 11,
    SPEC_CASTER_CLOTH          = 12,

    SPEC_PROFILE_COUNT         = 13
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

inline constexpr BlessingType ToSingleVariant(BlessingType type)
{
    switch (BaseBlessingOf(type))
    {
        case BASE_MIGHT:     return BLESSING_MIGHT_SINGLE;
        case BASE_WISDOM:    return BLESSING_WISDOM_SINGLE;
        case BASE_KINGS:     return BLESSING_KINGS_SINGLE;
        case BASE_SANCTUARY: return BLESSING_SANCTUARY_SINGLE;
        default:             return BLESSING_NONE;
    }
}

inline constexpr BlessingType ToGreaterVariant(BlessingType type)
{
    switch (BaseBlessingOf(type))
    {
        case BASE_MIGHT:     return BLESSING_MIGHT_GREATER;
        case BASE_WISDOM:    return BLESSING_WISDOM_GREATER;
        case BASE_KINGS:     return BLESSING_KINGS_GREATER;
        case BASE_SANCTUARY: return BLESSING_SANCTUARY_GREATER;
        default:             return BLESSING_NONE;
    }
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

// ── Priority table entry ─────────────────────────────────────────

struct BlessingPriorityEntry
{
    BlessingType blessings[4];
};

// Indexed by [SpecProfile][paladinCountIndex] where paladinCountIndex:
//   0 = 1 paladin, 1 = 2 paladins, 2 = 3 paladins, 3 = 4+ paladins
//
// clang-format off
inline constexpr BlessingPriorityEntry BLESSING_PRIORITIES[SPEC_PROFILE_COUNT][4] =
{
    // SPEC_PROT_WARRIOR_TANK_DK
    {
        {{ BLESSING_KINGS_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
    },
    // SPEC_DPS_WARRIOR_DPS_DK
    {
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
    },
    // SPEC_CASTER_SHAMAN
    {
        {{ BLESSING_KINGS_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_SINGLE,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_SINGLE,  BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_GREATER, BLESSING_MIGHT_GREATER     }},
    },
    // SPEC_ENHANCE_SHAMAN
    {
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_SANCTUARY_GREATER }},
    },
    // SPEC_RET_PALADIN
    {
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_SINGLE,   BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_SINGLE,   BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_SANCTUARY_GREATER }},
    },
    // SPEC_HOLY_PALADIN
    {
        {{ BLESSING_KINGS_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_SINGLE,      BLESSING_WISDOM_SINGLE,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_SINGLE,      BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_SINGLE,  BLESSING_NONE              }},
        {{ BLESSING_KINGS_SINGLE,      BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_SINGLE,  BLESSING_MIGHT_GREATER     }},
    },
    // SPEC_PROT_PALADIN
    {
        {{ BLESSING_SANCTUARY_SINGLE,  BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_SANCTUARY_SINGLE,  BLESSING_MIGHT_SINGLE,   BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_SANCTUARY_SINGLE,  BLESSING_MIGHT_SINGLE,   BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_SANCTUARY_GREATER, BLESSING_MIGHT_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_KINGS_GREATER,    }},
    },
    // SPEC_BEAR_DRUID
    {
        {{ BLESSING_KINGS_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_SINGLE,   BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_MIGHT_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_SANCTUARY_GREATER }},
    },
    // SPEC_CAT_DRUID
    {
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_MIGHT_SINGLE,      BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_SANCTUARY_GREATER }},
    },
    // SPEC_CASTER_DRUID
    {
        {{ BLESSING_KINGS_SINGLE,      BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_SINGLE,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_SINGLE,  BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_GREATER, BLESSING_MIGHT_GREATER     }},
    },
    // SPEC_ROGUE
    {
        {{ BLESSING_MIGHT_GREATER,     BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
    },
    // SPEC_HUNTER
    {
        {{ BLESSING_MIGHT_GREATER,     BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_NONE              }},
        {{ BLESSING_MIGHT_GREATER,     BLESSING_KINGS_GREATER,  BLESSING_WISDOM_GREATER,    BLESSING_SANCTUARY_GREATER }},
    },
    // SPEC_CASTER_CLOTH (Mage, Warlock, Priest)
    {
        {{ BLESSING_KINGS_GREATER,     BLESSING_NONE,           BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_NONE,              BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
        {{ BLESSING_KINGS_GREATER,     BLESSING_WISDOM_GREATER, BLESSING_SANCTUARY_GREATER, BLESSING_NONE              }},
    },
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
        return SPEC_CASTER_CLOTH;

    uint8 cls = player->getClass();
    int tab = AiFactory::GetPlayerSpecTab(player);

    switch (cls)
    {
        case CLASS_WARRIOR:
            if (tab == WARRIOR_TAB_PROTECTION)
                return SPEC_PROT_WARRIOR_TANK_DK;
            return SPEC_DPS_WARRIOR_DPS_DK;

        case CLASS_DEATH_KNIGHT:
            if (tab == DEATH_KNIGHT_TAB_BLOOD || player->HasAura(SPELL_DK_FROST_PRESENCE))
                return SPEC_PROT_WARRIOR_TANK_DK;
            return SPEC_DPS_WARRIOR_DPS_DK;

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
        case CLASS_WARLOCK:
        case CLASS_PRIEST:
        default:
            return SPEC_CASTER_CLOTH;
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
