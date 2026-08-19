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
#include <unordered_set>
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
    SPELL_PROTECTION_OF_ELUNE = 38528,

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

// General
inline constexpr uint32 HYJAL_MAP_ID = 534;
// Ground hazards are cached per bot rather than searched for on every ask. The interval matches
// the default AiPlayerbot.ReactDelay, so a hazard is re-read once per decision cycle and never
// twice within one--which is the whole point, since Engine runs every multiplier against every
// action in the queue. It is also the floor that can be stated in milliseconds at all:
// CalculatedValue reads any interval below 100 as seconds
inline constexpr uint32 HAZARD_CACHE_INTERVAL = 100;
// Each hazard is searched only as far as its widest consumer actually looks, plus the ground a
// bot covers between refreshes. Derived from that consumer rather than picked, so widening a
// control radius widens the search feeding it instead of silently truncating it
inline constexpr float HAZARD_SEARCH_MARGIN = 2.0f;
// Held back from the edge of melee range so rounding and drift cannot leave a bot just out of
// reach. GetMeleeRange is both combat reaches plus 4/3, so any buffer under that stays clear of
// contact whatever the boss's hitbox. A fixed yard suits that; a proportion of the range would
// only stand the bot deeper the wider the boss is, which nothing calls for
inline constexpr float MELEE_RANGE_INSET = 1.0f;
// A boss still at full health has not been pulled properly yet: the tank is still walking him to
// his spot and threat is still being handed over. Health leaving full is the cheapest proxy for
// that being over, and every gate here that means "the opening is not finished" keys on it
inline constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
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
// Out to here, melee movement near the pool belongs to the maneuver action and nothing else. The
// same arrangement as Azgalor's pools, for the same reason: where the trigger that runs that action
// and the multiplier that clears the way for it disagree, the gap between them is a band in which
// the trigger has stopped firing but the suppression has not lifted. A melee bot in that band has
// nothing left that closes distance--the formation movers are already off for the fight--so it
// stands where it is while the main tank walks Winterchill out of reach
inline constexpr float DEATH_AND_DECAY_MELEE_CONTROL_RADIUS = DEATH_AND_DECAY_RADIUS + 10.0f;
// Widest consumer is the melee control radius, which the maneuver trigger and its multiplier
// both read
inline constexpr float DEATH_AND_DECAY_SEARCH_RADIUS =
    DEATH_AND_DECAY_MELEE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
// Back towards the centre of the base, for more room to maneuver
inline Position const WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
bool GetDeathAndDecayPosition(PlayerbotAI* botAI, Position& deathAndDecay); // one at most
// For callers wanting a margin on the hazard
bool IsNearDeathAndDecay(PlayerbotAI* botAI, float radius);
bool IsInDeathAndDecay(PlayerbotAI* botAI);

// Anetheron
// Back towards the centre of the base, near the crossroads
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
// Near the gate, so the raid can get started on DPS ASAP
inline Position const KAZROGAL_TANK_POSITION = { 5505.440f, -2665.059f, 1480.598f };
inline constexpr float KAZROGAL_RANGED_ARC_RADIUS = 15.0f;
// Kaz'rogal is still being walked to his tanking spot at the pull, and ranged closing to the
// fighting radius while that happens puts them in front of him. Hold them further out until the
// tank has him engaged, for which his health leaving full is the cheapest proxy
inline constexpr float KAZROGAL_RANGED_ARC_APPROACH_RADIUS = 25.0f;
// Measured in game: the heading from Kaz'rogal down the open approach
inline constexpr float KAZROGAL_RANGED_ARC_CENTER = 4.225f;
// Obstacles flank that approach at fixed world positions, so what bounds the arc is lateral
// clearance rather than an angle--pull the ring in and the same clearance subtends more of it.
// Deriving the span keeps the two consistent whenever the radius moves. This half-width is a
// measurement, so re-check that the outer slots still path cleanly after changing either
inline constexpr float KAZROGAL_RANGED_ARC_HALF_WIDTH = 10.0f;
float GetKazrogalRangedArcRadius(Unit* kazrogal);
// Takes the radius rather than reading the constant, since which radius is in force depends on how
// far into the pull the raid is. Pulling the ring in widens the span for the same clearance, so the
// arc holds roughly its length either way and spacing does not collapse on the approach
inline float GetKazrogalRangedArcSpan(float radius)
{
    float const ratio = KAZROGAL_RANGED_ARC_HALF_WIDTH / radius;
    return 2.0f * std::asin(ratio < 1.0f ? ratio : 1.0f);
}
// Mark of Kaz'rogal 31447 drains 600 a tick, five 1s ticks for 3000 in all, and detonates on the
// first tick that finds the victim holding less than one tick's worth
inline constexpr float MARK_TICK_DRAIN = 600.0f;
inline constexpr float MARK_DANGER_MANA = 3200.0f;
inline constexpr float MARK_REJOIN_MANA = 5000.0f;
// Life Tap runs far above the danger line on purpose: it is the only response that stops a warlock
// needing to run at all, and health is the cheaper resource here. Below the AI's low-health mark
// there is nothing left to spend, and Shadow Ward takes over
inline constexpr float MARK_LIFE_TAP_MANA = 5000.0f;
// 31463 carries a flat 15y radius. Its caster and its victims are all player controlled, which is
// the one combination the post-#26967 area check adds no combat reach for, and its
// TARGET_UNIT_SRC_AREA_ALLY is not among the three target types that earn movement leeway--so
// nothing widens it, Bloodlust included. The extra yard is slack between a bot reading its own
// position and the server ticking the aura
inline constexpr float MARK_EXPLOSION_RADIUS = 15.0f;
inline constexpr float MARK_ESCAPE_DISTANCE = MARK_EXPLOSION_RADIUS + 1.0f;
extern std::unordered_set<ObjectGuid> botsBelowManaThreshold;
// Whether the Mark can land on this bot at all, and so whether any of the low-mana handling means
// anything for it. 31447 filters to units whose current power is mana, which rules out the three
// classes that never have any and a druid shifted into bear or cat--forms whose bar is rage or
// energy. Hunters are mana users and are not excluded here: what they do about it differs, not
// whether the Mark can reach them
bool IsKazrogalManaUser(PlayerbotAI* botAI, Player* bot);
bool HasMarkOfKazrogal(Player* bot);

// Azgalor
inline constexpr float RAIN_OF_FIRE_RADIUS = 16.5f; // 15y radius + 1.5y player hitbox
// Out to here, melee movement near a pool belongs to the maneuver action and nothing else. The
// trigger that runs that action and the multiplier that clears the way for it have to read the same
// figure: where they disagree is a band in which the trigger has stopped firing but the suppression
// has not lifted, and a melee bot that has just stepped clear of a pool stands frozen in it for the
// pool's whole life--following Azgalor nowhere while the tank drags him out of reach
inline constexpr float RAIN_OF_FIRE_MELEE_CONTROL_RADIUS = RAIN_OF_FIRE_RADIUS + 10.0f;
// As at Winterchill: the melee control radius is the furthest anything asks about a pool
inline constexpr float RAIN_OF_FIRE_SEARCH_RADIUS =
    RAIN_OF_FIRE_MELEE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
inline Position const AZGALOR_TANK_POSITION =      { 5494.594f, -2747.069f, 1487.800f };
// TODO: re-measure. The old value was chosen relative to the tank's transition spot, which no
// longer exists now that the boss is walked straight to the final position
inline Position const AZGALOR_DOOMGUARD_POSITION = { 5452.166f, -2723.282f, 1485.480f };
std::vector<Position> GetRainOfFirePositions(PlayerbotAI* botAI);
bool GetNearestRainOfFirePosition(PlayerbotAI* botAI, Position& pool);
// For callers wanting a margin on the hazard
bool IsNearRainOfFire(PlayerbotAI* botAI, float radius);
bool IsInRainOfFire(PlayerbotAI* botAI);
bool IsDoomed(Player* bot);
// The tank that holds Lesser Doomguards: the first assist tank among the living, or the second once
// the first is Doomed and so about to spawn one of its own. Numbered among the living because
// GetGroupAssistTank has no dead-inclusive mode--count corpses here and the two disagree the moment
// a tank dies. Its own positioning keeps it at AZGALOR_DOOMGUARD_POSITION, well away from the raid,
// which is why nothing else may walk it: dragging it to Azgalor brings the Doomguard along
bool IsDoomguardTank(PlayerbotAI* botAI, Player* bot);
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
// Up the hill a bit, for space from the World Tree. The tank walks him here at the opening only
inline Position const ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };
// Where the ground actually burns, and the only figure here that is a fact rather than a choice.
// The Doomfire carries 31945, a 1s periodic that drops a fresh 31943 patch at its feet; 31943 is a
// persistent area aura of 6y, and persistent area auras add the victim's own combat reach--1.5
// ordinarily, 1.95 for the 40s Bloodlust is up. So the fire reaches nearly 8, not 6, and a stopping
// distance set against the raw 6 leaves a bot standing in it
inline constexpr float DOOMFIRE_BURN_RADIUS = 8.0f;
// Where the avoidance parks the bot. Everything below is derived from the burn radius rather than
// picked, because the gap between them is the whole safety margin: the Doomfire walks, and the aura
// re-reads its targets every second, so a bot left on the edge is clipped between pushes
inline constexpr float DOOMFIRE_DANGER_RADIUS = DOOMFIRE_BURN_RADIUS + 2.0f;
// Suppression has to reach past where the push fades. If the two met at the same figure, a bot that
// had just been pushed to the edge would find the pull resuming in the same tick, be dragged back
// inside, and be pushed out again--the jitter is the two trading it back and forth. The band
// between them is where an escaped bot waits for the trail to burn out or wander off, and it is
// kept narrow on purpose: trails are laid close together, so every yard of suppression is a yard of
// floor a bot may not use to get around the next one
inline constexpr float DOOMFIRE_CONTROL_RADIUS = DOOMFIRE_DANGER_RADIUS + 2.0f;
// Patches are gathered this far out but only counted as dangerous inside the danger radius above.
// Direction and magnitude are read separately, so a patch further off bends which way the bot goes
// without adding any push of its own--letting it steer around a cluster it has not entered instead
// of being shoved out of one patch straight into the neighbour it could not see
inline constexpr float DOOMFIRE_FIELD_RADIUS = 18.0f;
// The furthest a patch can matter is not the field radius but the trapped sweep: it ranks
// landing points up to DOOMFIRE_DANGER_RADIUS from the bot against whatever sits within
// DOOMFIRE_FIELD_RADIUS of them, so a patch that far out again can still decide a bearing
inline constexpr float DOOMFIRE_SEARCH_RADIUS =
    DOOMFIRE_FIELD_RADIUS + DOOMFIRE_DANGER_RADIUS + HAZARD_SEARCH_MARGIN;
inline constexpr float ARCHIMONDE_RANGED_SPREAD_DISTANCE = 10.0f;
inline constexpr uint32 ARCHIMONDE_RANGED_SPREAD_INTERVAL = 3000;
bool HasProtectionOfElune(Player* bot);
bool IsNearDoomfire(PlayerbotAI* botAI, float radius);
// The same question asked of a point the bot is about to step to rather than of where it stands
bool IsPositionNearDoomfire(PlayerbotAI* botAI, float x, float y, float radius);
// Every live trail patch the cache holds, for callers that weigh the field rather than just
// asking whether one is near. By value on purpose: these are bound to a local and held
// across other helper calls, and a reference into the cache would dangle the moment one of
// those refreshed it
std::vector<Position> GetDoomfirePositions(PlayerbotAI* botAI);
extern std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;
// The Air Burst currently being cast, if one is. Recorded when the cast starts and left to lapse
// on its own, so it answers "is one on the way", not "was one cast"
AirBurstData* GetPendingAirBurstCast(uint32 instanceId);

}

#endif
