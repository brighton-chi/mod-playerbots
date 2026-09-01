/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULHELPERS_H
#define PLAYERBOTS_GRUULHELPERS_H

#include "Common.h"
#include "Position.h"
#include <type_traits>

class Player;

namespace GruulHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class GruulSpells : uint32
{
    // High King Maulgar
    SPELL_WHIRLWIND     = 33238,

    // Krosh Firehand
    SPELL_SPELL_SHIELD  = 33054,

    // Hunter
    SPELL_MISDIRECTION  = 35079,

    // Mage
    SPELL_SPELLSTEAL    = 30449,

    // Priest
    SPELL_FEAR_WARD     = 6346,

    // Gruul the Dragonkiller
    SPELL_GROUND_SLAM_1 = 33525,
    SPELL_GROUND_SLAM_2 = 39187,
};

enum class GruulNpcs : uint32
{
    NPC_WILD_FEL_STALKER = 18847,
};

// Ogre combat reaches:
// (1) Maulgar = 3.5y, (2) Olm = 2.2y, (3) Blindeye = 3.525y, (4) Krosh = 2y, (5) Kiggler = 3.3y
//
// Safe distances below are exact 2D, center to center. Compare them with GetExactDist2d, never
// GetDistance2d, which subtracts both combat reaches and so adds a silent, per-boss amount on top
// of the 2y. The figure each one pads is the raw spell radius, because every hazard here reaches
// players through the SRC branch of the membership check, where neither reach is added
// (Spell.cpp:9162).

inline constexpr uint32 GRUUL_MAP_ID = 565;
inline constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;
// Radius is 15y with 2y of MoveAway padding.
inline constexpr float KROSH_BLAST_WAVE_SAFE_DISTANCE = 17.0f;
// Radius is 8y, padded to 8 * sqrt(2) rather than the 2y used elsewhere. Maulgar is tanked against
// a wall, so MoveAway's fan routinely falls through to its +/-90 degree candidates. An off-axis
// landing sits at safeDistance * cos(delta/2) in the worst case, which puts +/-90 at safeDistance /
// sqrt(2); only a request above 11.31y keeps all nine candidates clear of the radius. 2y of padding
// would leave the two +/-90 candidates landing at 7.07y, inside the whirlwind.
inline constexpr float MAULGAR_WHIRLWIND_SAFE_DISTANCE = 12.0f;
// Held wider than the safe distance: the run-away action stops at the safe distance, and without a
// wider hold band the multiplier would release at that same point and let another movement action
// walk the bot straight back in.
inline constexpr float MAULGAR_WHIRLWIND_HOLD_DISTANCE = 15.0f;
// Radius is 30y with 2y of MoveAway padding. Stays inside the stock "enemy out of spell" threshold
// (spellDistance + CONTACT_DISTANCE + both reaches, ~34y exact against Kiggler), so holding here
// does not let reach spell drag the moonkin back into the blast.
inline constexpr float KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE = 32.0f;
// Radius is 20y with 2y of MoveAway padding.
inline constexpr float GRUUL_SHATTER_SAFE_DISTANCE = 22.0f;

inline Position const MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
inline Position const OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
inline Position const BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
inline Position const KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
inline Position const GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

bool IsMaulgarTank(Player* bot);
bool IsOlmTank(Player* bot);
bool IsBlindeyeTank(Player* bot);
Player* GetKroshMageTank(Player* bot);
bool IsKroshMageTank(Player* bot);
Player* GetKigglerMoonkinTank(Player* bot);
bool IsKigglerMoonkinTank(Player* bot);
bool HasGroundSlam(Player* bot);

}

#endif
