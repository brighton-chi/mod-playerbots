/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALHELPERS_H
#define PLAYERBOTS_HYJALHELPERS_H

#include "ObjectGuid.h"
#include "Position.h"
#include <cmath>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace HyjalHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class HyjalSpells : uint32
{
    // Rage Winterchill
    SPELL_DEATH_AND_DECAY     = 31258,

    // Anetheron
    SPELL_INFERNO             = 31299,

    // Kaz'rogal
    SPELL_MARK_OF_KAZROGAL    = 31447,

    // Azgalor
    SPELL_RAIN_OF_FIRE        = 31340,
    SPELL_DOOM                = 31347,

    // Archimonde
    SPELL_DOOMFIRE_TRAIL      = 31943, // 6y persistent area aura dropped along the trail, lasts 18s
    SPELL_DOOMFIRE            = 31944, // Damaging part of trail
    SPELL_DOOMFIRE_DOT        = 31969, // DoT after exiting trail
    // SPELL_ARCHIMONDE_FEAR    = 31970,
    SPELL_AIR_BURST           = 32014,

    // Hunter
    SPELL_ASPECT_OF_THE_VIPER = 34074,
    SPELL_MISDIRECTION        = 35079,

    // Mage
    SPELL_ICE_BLOCK           = 45438,

    // Paladin
    SPELL_DIVINE_SHIELD       = 642,

    // Priest
    SPELL_FEAR_WARD           = 6346,

    // Rogue
    SPELL_CLOAK_OF_SHADOWS    = 31224,

    // Shaman
    SPELL_TREMOR_TOTEM        = 8143,
};

enum class HyjalNpcs : uint32
{
    // Anetheron
    NPC_TOWERING_INFERNAL = 17818,

    // Archimonde
    NPC_DOOMFIRE          = 18095,
};

enum class TankPositionState : uint8
{
    MovingToTransition = 0,
    MovingToFinal      = 1,
    Positioned         = 2,
    Unknown            = 255,
};

// General
inline constexpr uint32 HYJAL_MAP_ID = 534;
inline constexpr float HAZARD_SEARCH_RADIUS = 60.0f; // For Death & Decay and Rain of Fire
// Held back from the edge of melee range so rounding and drift cannot leave a bot just out of
// reach. GetMeleeRange is both combat reaches plus 4/3, so any buffer under that stays clear of
// contact whatever the boss's hitbox. A fixed yard suits that; a proportion of the range would
// only stand the bot deeper the wider the boss is, which nothing calls for
inline constexpr float MELEE_RING_BUFFER = 1.0f;
struct RangedGroups
{
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
};
// A span of headings around a ring that is unavailable, as a centre heading and a half-width.
// Melee positioning is the same problem at every boss here: stand on the ring at melee range and
// pick a heading that no hazard, and no boss ability, has taken away
struct BlockedArc
{
    float center;
    float halfWidth;
};

// The span of a melee ring that a circular ground hazard covers. False when it reaches none of it
bool GetHazardBlockedArc(
    Position const& ringCenter, float ringRadius, Position const& hazard,
    float hazardRadius, BlockedArc& arc);
// The angle nearest to preferred that clears every blocked arc. False when none does, which means
// the ring is entirely unavailable and the caller has to give up on attacking. This is a place to
// stand on the ring, not a direction to travel in
bool FindNearestUnblockedAngle(
    std::vector<BlockedArc> const& blocked, float preferred, float& unblocked);
// A step towards a point on a circle, taking the angle nearest to preferred that the bot can
// actually reach. Widening on refusal is the point of it: the angle only has to be roughly right,
// so a candidate blocked by terrain or by geometry costs nothing to abandon. isAcceptable rejects
// angles a caller cannot use for its own reasons, and may be empty
// chosenX/chosenY report the point on the circle the step is aimed at, which is not the preferred
// one when that had to be abandoned. A caller deciding whether it has arrived has to measure
// against this, not against what it asked for, or a bot settling on a neighbouring angle is never
// finished and keeps asking for the rest of the fight
// allowUnvalidatedFallback sweeps the fan a second time without the reachability check, for a
// caller that would rather move badly than stand still. isAcceptable is honoured in both sweeps
bool FindStepToCircle(
    Player* bot, Position const& center, float radius, float preferredAngle, float moveDist,
    float& stepX, float& stepY, float& stepZ,
    std::function<bool(float, float)> const& isAcceptable = {},
    float* chosenX = nullptr, float* chosenY = nullptr, bool allowUnvalidatedFallback = false);
// The same search, aimed straight out of a hazard. escapeRadius should sit past the radius the
// caller reacts at, or the bot settles on the threshold and slides around it instead of leaving
bool GetHazardEscapeStep(
    Player* bot, Position const& hazard, float escapeRadius, float moveDist, float& stepX,
    float& stepY, float& stepZ, std::function<bool(float, float)> const& isAcceptable = {});

RangedGroups GetRangedGroups(Player* bot);
std::pair<size_t, size_t> GetBotCircleIndexAndCount(Player* bot, RangedGroups const& groups);

// Rage Winterchill
// 20y radius + 1.5y player hitbox, rounded up. Combat reach is not fixed: a scale aura takes it to
// 1.95, so the aura reaches 21.95 for the 40s Bloodlust is up, and a bot parked on an exact 21.5
// would be taking damage while every check here called it clear
inline constexpr float DEATH_AND_DECAY_RADIUS = 22.0f;
inline Position const WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
bool GetDeathAndDecayPosition(Player* bot, Position& deathAndDecay); // at most one is ever up
bool IsNearDeathAndDecay(Player* bot, float radius); // for callers wanting a margin on the hazard
bool IsInDeathAndDecay(Player* bot);

// Anetheron
inline Position const ANETHERON_TANK_POSITION =       { 5033.177f, -1765.996f, 1324.195f };
inline Position const ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
inline Position const ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
// Infernals are summoned wherever the Inferno target stands and then walked to the tanking spot,
// so the search has to span the whole platform rather than the area around any one bot
inline constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
// A landing Infernal stuns everything within 10y for 2s (31302), and then burns everything within
// 10y of itself for as long as it lives (31304 triggering 31303). Both are cast by the creature, so
// the post-#26967 area check adds no combat reach to either--10 is 10, Bloodlust included. Since
// Inferno drops the Infernal on its target's feet, clearing this much of that target during the
// 3.5s cast dodges the stun and starts the bot outside the immolation for free
inline constexpr float INFERNAL_DANGER_RADIUS = 10.0f;
inline constexpr float INFERNAL_ESCAPE_DISTANCE = INFERNAL_DANGER_RADIUS + 2.0f;
Player* GetInfernoTarget(Unit* anetheron);
// Every living Towering Infernal, oldest first. Creature GUIDs are handed out in spawn order, so
// that ordering is both stable as the fight goes on and identical for every bot that asks--which
// is what a name lookup through "find target" cannot give, since it returns whichever match an
// unordered threat list happens to yield first. Read it through the "hyjal infernals" value
GuidVector FindInfernalGuids(Player* bot);
GuidVector const& GetInfernalGuids(PlayerbotAI* botAI);
// The one the raid kills: the oldest alive. Focus fire keeps that the most damaged one as well,
// and unlike a lowest-health rule it cannot change its mind as two Infernals cross
Unit* GetFocusedInfernal(PlayerbotAI* botAI);
// The first Infernal the Infernal tank does not have, which is the one worth handing over
Unit* GetLooseInfernal(PlayerbotAI* botAI, Player* bot);
Unit* GetNearestInfernal(PlayerbotAI* botAI, Player* bot);
Unit* GetInfernalTargetingBot(PlayerbotAI* botAI, Player* bot);
bool IsInfernalTank(Player* bot);
Player* GetInfernalTank(Player* bot);
// Whichever of the two spots the Infernal tank stands nearer. Reading it off the tank rather than
// off the asking bot is what stops the player carrying a summon and the tank waiting to receive it
// from setting out for opposite sides
Position const& GetInfernalTankPosition(Player* bot);

// Kaz'rogal
inline Position const KAZROGAL_TANK_POSITION = { 5505.440f, -2665.059f, 1480.598f };
inline constexpr float KAZROGAL_RANGED_ARC_RADIUS = 15.0f;
// Measured in game: the heading from Kaz'rogal down the open approach
inline constexpr float KAZROGAL_RANGED_ARC_CENTER = 4.225f;
// Obstacles flank that approach at fixed world positions, so what bounds the arc is lateral
// clearance rather than an angle--pull the ring in and the same clearance subtends more of it.
// Deriving the span keeps the two consistent whenever the radius moves. This half-width is a
// measurement, so re-check that the outer slots still path cleanly after changing either
inline constexpr float KAZROGAL_RANGED_ARC_HALF_WIDTH = 10.0f;
inline float GetKazrogalRangedArcSpan()
{
    float const ratio = KAZROGAL_RANGED_ARC_HALF_WIDTH / KAZROGAL_RANGED_ARC_RADIUS;
    return 2.0f * std::asin(ratio < 1.0f ? ratio : 1.0f);
}
inline constexpr float MARK_DANGER_MANA = 3200f;
inline constexpr float MARK_REJOIN_MANA = 4000f;
// 31463 carries a flat 15y radius. Its caster and its victims are all player controlled, which is
// the one combination the post-#26967 area check adds no combat reach for, and its
// TARGET_UNIT_SRC_AREA_ALLY is not among the three target types that earn movement leeway--so
// nothing widens it, Bloodlust included. The extra yard is slack between a bot reading its own
// position and the server ticking the aura
inline constexpr float MARK_EXPLOSION_RADIUS = 15.0f;
inline constexpr float MARK_ESCAPE_DISTANCE = MARK_EXPLOSION_RADIUS + 1.0f;
inline constexpr float MARK_ESCAPE_PATH_EFFICIENCY = 0.6f;
extern std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

// Azgalor
inline constexpr float RAIN_OF_FIRE_RADIUS = 16.5f; // 15y radius + 1.5y player hitbox
inline Position const AZGALOR_TANK_TRANSITION_POSITION = { 5486.787f, -2696.215f, 1482.007f };
inline Position const AZGALOR_TANK_FINAL_POSITION =      { 5496.379f, -2675.265f, 1481.053f };
inline Position const AZGALOR_DOOMGUARD_POSITION =       { 5485.555f, -2731.659f, 1485.555f };
extern std::unordered_map<ObjectGuid, TankPositionState> azgalorTankStep;
TankPositionState GetAzgalorTankPositionState(PlayerbotAI* botAI, Player* bot);
std::vector<Position> GetRainOfFirePositions(Player* bot);
bool GetNearestRainOfFirePosition(Player* bot, Position& pool);
bool IsNearRainOfFire(Player* bot, float radius); // for callers wanting a margin on the hazard
bool IsInRainOfFire(Player* bot);
// Cleave chains from Azgalor's victim to four more players, but only within this distance of
// that victim and inside his frontal arc. Both are padded past the 10y jump radius and the
// 180 degree filter in Spell::SearchChainTargets, since he turns with the tank
inline constexpr float CLEAVE_CHAIN_RADIUS = 12.0f;
inline constexpr float CLEAVE_DANGER_ARC = 200.0f * static_cast<float>(M_PI) / 180.0f;
bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y);
bool AnyGroupMemberHasDoom(Player* bot);

// Archimonde
struct AirBurstData
{
    ObjectGuid targetGuid;
    uint32 castTime;
};
inline constexpr float AIR_BURST_SAFE_DISTANCE = 15.0f;
inline Position const ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };
extern std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;
// The Air Burst currently being cast, if one is. Recorded when the cast starts and left to lapse
// on its own, so it answers "is one on the way", not "was one cast"
AirBurstData* GetPendingAirBurstCast(uint32 instanceId);

}

#endif
