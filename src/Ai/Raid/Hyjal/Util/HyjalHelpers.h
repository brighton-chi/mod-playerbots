/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALHELPERS_H
#define PLAYERBOTS_HYJALHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
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
    SPELL_AIR_BURST           = 32014,
    SPELL_PROTECTION_OF_ELUNE = 38528,

    // Hunter
    SPELL_ASPECT_OF_THE_VIPER = 34074,
    SPELL_MISDIRECTION        = 35079,

    // Mage
    SPELL_ICE_BLOCK           = 45438,

    // Paladin
    SPELL_DIVINE_SHIELD       = 642,

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
// The interval matches the default AiPlayerbot.ReactDelay.
inline constexpr uint32 HAZARD_CACHE_INTERVAL = 100;
inline constexpr float HAZARD_SEARCH_MARGIN = 2.0f;
// The trigger for ranged hazard avoidance ends at the hazard's edge. FleePosition() moves in fixed
// AiPlayerbot.FleeDistance steps (5 by default) so it can end up to 5y outside of the hazard.
// This distance, used for multipliers, covers that 5y plus a bit to prevent ReachSpellAction from
// moving the bot back into the hazard after escaping it.
inline constexpr float HAZARD_RANGED_CONTROL_MARGIN = 7.0f;
// This distance is subtracted from the edge of melee range so rounding and drift cannot leave a
// bot just out of reach.
inline constexpr float MELEE_RANGE_INSET = 1.0f;

struct RangedGroups
{
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
};
// A span of headings around a ring that is unavailable.
struct BlockedArc
{
    float center;
    float halfWidth;
};
// The span of a melee ring that a circular ground hazard covers.
bool GetHazardBlockedArc(
    Position const& ringCenter, float ringRadius, Position const& hazard,
    float hazardRadius, BlockedArc& arc);
// The angle nearest to the preferred one that clears every blocked arc.
bool FindNearestUnblockedAngle(
    std::vector<BlockedArc> const& blocked, float preferred, float& unblocked);
// A step towards a point on a circle, at the angle nearest to preferred that the bot can reach.
bool FindStepToCircle(
    Player* bot, Position const& center, float radius, float preferredAngle, float moveDist,
    float& stepX, float& stepY, float& stepZ,
    std::function<bool(float, float)> const& isAcceptable = {},
    float* chosenX = nullptr, float* chosenY = nullptr);
// The same search, except aimed straight out of a hazard.
bool GetHazardEscapeStep(
    Player* bot, Position const& hazard, float escapeRadius, float moveDist, float& stepX,
    float& stepY, float& stepZ, std::function<bool(float, float)> const& isAcceptable = {});
// Every ranged raid member on the map, in group order. The dead are kept in the list so that a
// death does not shift every survivor's ring index.
std::vector<Player*> GetRangedMembers(Player* bot);
RangedGroups GetRangedGroups(Player* bot);
std::pair<size_t, size_t> GetBotCircleIndexAndCount(Player* bot, RangedGroups const& groups);

// Rage Winterchill

// 20y radius + 1.95y max player CombatReach (1.5y increased by 30% during Bloodlust). Range should
// not be increased by CombatReach, but the range of persistent ground-based AoEs is bugged in AC.
inline constexpr float DEATH_AND_DECAY_RADIUS = 22.0f;
// Out to this distance, melee movement is controlled only by the avoidance action.
inline constexpr float DEATH_AND_DECAY_MELEE_CONTROL_RADIUS = DEATH_AND_DECAY_RADIUS + 10.0f;
inline constexpr float DEATH_AND_DECAY_RANGED_CONTROL_RADIUS =
    DEATH_AND_DECAY_RADIUS + HAZARD_RANGED_CONTROL_MARGIN;
inline constexpr float DEATH_AND_DECAY_SEARCH_RADIUS =
    DEATH_AND_DECAY_MELEE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
// Back towards the center of the base
inline Position const WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };
bool GetDeathAndDecayPosition(PlayerbotAI* botAI, Position& deathAndDecay);
bool IsNearDeathAndDecay(PlayerbotAI* botAI, float radius);
bool IsInDeathAndDecay(PlayerbotAI* botAI);

// Anetheron

// Back towards the center of the base, near the crossroads
inline Position const ANETHERON_TANK_POSITION =       { 5033.177f, -1765.996f, 1324.195f };
inline Position const ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
inline Position const ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
inline constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
// A landing Infernal stuns everybody within 10y for 2s (31302) and then burns everything within
// 10y of itself for as long as it lives with Immolation (31304 triggering 31303).
inline constexpr float INFERNAL_DANGER_RADIUS = 10.0f;
inline constexpr float INFERNAL_ESCAPE_DISTANCE = INFERNAL_DANGER_RADIUS + 2.0f;
// Past this, ranged stay on the boss rather than switching to the Infernal. Arbitrary, but near
// enough that ranged do not bunch up and risk too many getting hit by a Carrion Swarm. Measured
// center to center; a Towering Infernal's CombatReach is 4y.
inline constexpr float INFERNAL_RANGED_ENGAGE_DISTANCE = 50.0f;
Player* GetInfernoTarget(Unit* anetheron);
// Every living Towering Infernal, oldest first, read through the "hyjal infernals" value
GuidVector FindInfernalGuids(Player* bot);
GuidVector const& GetInfernalGuids(PlayerbotAI* botAI);
// The first Infernal that the Infernal tank does not have aggro on.
Unit* GetLooseInfernal(PlayerbotAI* botAI);
Unit* GetNearestInfernal(PlayerbotAI* botAI);
// The Infernal a ranged bot should attack instead of the boss, if any. It is the oldest Infernal
// alive, but in practice, a raid should have only one up at a time.
Unit* GetInfernalToAttack(PlayerbotAI* botAI, Unit* anetheron);
Unit* GetInfernalTargetingBot(PlayerbotAI* botAI);
// Both resolve the first assist tank among the living, so keep them in step.
bool IsInfernalTank(Player* bot);
Player* GetInfernalTank(Player* bot);
// Whichever of the two spots the Infernal tank stands nearer.
Position const& GetInfernalTankPosition(Player* bot);

// Kaz'rogal

// Near the gate, so the raid can get started immediately to beat the soft enrage due to Marks
inline Position const KAZROGAL_TANK_POSITION = { 5505.440f, -2665.059f, 1480.598f };
// War Stomp is 12y + up to 1.95y CombatReach
inline constexpr float KAZROGAL_RANGED_ARC_RADIUS = 15.0f;
// For keeping extra distance during the pull to avoid War Stomp
inline constexpr float KAZROGAL_RANGED_ARC_APPROACH_RADIUS = 25.0f;
// The heading from Kaz'rogal for the center of the ranged arc, measured ingame
inline constexpr float KAZROGAL_RANGED_ARC_CENTER = 4.225f;
// This is about the maximum width that allows reasonable escape paths due to obstacles
inline constexpr float KAZROGAL_RANGED_ARC_HALF_WIDTH = 10.0f;
// Mark of Kaz'rogal (31447) drains 600 mana a tick, five 1s ticks for 3000 in all. It detonates on
// the first tick where the victim has less than 600 mana.
inline constexpr float MARK_TICK_DRAIN = 600.0f;
inline constexpr float MARK_DANGER_MANA = 3200.0f;
inline constexpr float MARK_REJOIN_MANA = 5000.0f;
// Life Tap gets a higher threshold than usual to keep Warlocks from having to run away from the
// group at all.
inline constexpr float MARK_LIFE_TAP_MANA = 5000.0f;
// The radius of the Mark of Kaz'rogal explosion (31463) is 15y, with a 1y buffer added.
inline constexpr float MARK_ESCAPE_DISTANCE = 16.0f;
// This set allows for a gap between the mana threshold to run away and the mana threshold to come
// back to the group to avoid bouncing back and forth.
extern std::unordered_set<ObjectGuid> botsBelowManaThreshold;
float GetKazrogalRangedArcRadius(Unit* kazrogal);
float GetKazrogalRangedArcSpan(float radius);
bool IsKazrogalManaUser(PlayerbotAI* botAI);
bool HasMarkOfKazrogal(Player* bot);
uint32 GetKazrogalImmunitySpell(Player* bot);

// Azgalor

// 15y radius + 1.95y max player CombatReach. Like for D&D, the addition of CombatReach to the
// range is due to an AC bug.
inline constexpr float RAIN_OF_FIRE_RADIUS = 17.0f;
// Out to this distance, melee movement is controlled only by the avoidance action.
inline constexpr float RAIN_OF_FIRE_MELEE_CONTROL_RADIUS = RAIN_OF_FIRE_RADIUS + 10.0f;
// Used for both the multiplier for avoidance and the standard dispersal action's trigger
inline constexpr float RAIN_OF_FIRE_RANGED_CONTROL_RADIUS =
    RAIN_OF_FIRE_RADIUS + HAZARD_RANGED_CONTROL_MARGIN;
inline constexpr float RAIN_OF_FIRE_SEARCH_RADIUS =
    RAIN_OF_FIRE_MELEE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
inline Position const AZGALOR_TANK_POSITION =      { 5494.594f, -2747.069f, 1487.800f };
inline Position const AZGALOR_DOOMGUARD_POSITION = { 5452.166f, -2723.282f, 1485.480f };
std::vector<Position> GetRainOfFirePositions(PlayerbotAI* botAI);
bool GetNearestRainOfFirePosition(PlayerbotAI* botAI, Position& pool);
bool IsNearRainOfFire(PlayerbotAI* botAI, float radius);
bool IsInRainOfFire(PlayerbotAI* botAI);
bool IsDoomed(Player* bot);
// The tank that holds Lesser Doomguards: the first assist tank among the living, or the second if
// the first is Doomed and so about to spawn one of its own.
bool IsDoomguardTank(Player* bot);
// Cleave chains from Azgalor's victim to 4 more players, but only within this distance of that
// victim and inside his frontal arc.
inline constexpr float CLEAVE_CHAIN_RADIUS = 12.0f;
inline constexpr float CLEAVE_DANGER_ARC = 200.0f * static_cast<float>(M_PI) / 180.0f;
bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y);
bool AnyGroupMemberHasDoom(Player* bot);

// Archimonde

struct AirBurstData
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 castTime = 0;
};
inline constexpr float AIR_BURST_SAFE_DISTANCE = 15.0f;
// Up the hill a bit, for space from the World Tree. The tank walks him here at the opening only.
// Archimonde slaps so the tank will pause moving whenever below 60% HP.
inline Position const ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };
// Where the ground actually burns. Doomfire (31495) is a 1s periodic that drops a Doomfire (31943)
// pool, which has a persistent area aura of 6y (plus 1.95y CombatReach).
inline constexpr float DOOMFIRE_BURN_RADIUS = 8.0f;
// How far from Doomfires will avoidance place the bot.
inline constexpr float DOOMFIRE_DANGER_RADIUS = DOOMFIRE_BURN_RADIUS + 2.0f;
// Out to this distance, movement is controlled only by the avoidance action.
inline constexpr float DOOMFIRE_CONTROL_RADIUS = DOOMFIRE_DANGER_RADIUS + 2.0f;
// The maximum distance away from the bot at which a Doomfire patch affects the escape direction.
inline constexpr float DOOMFIRE_FIELD_RADIUS = 18.0f;
inline constexpr float DOOMFIRE_SEARCH_RADIUS =
    DOOMFIRE_FIELD_RADIUS + DOOMFIRE_DANGER_RADIUS + HAZARD_SEARCH_MARGIN; // 30y
// General spread to prevent clumping due to the risk of Air Burst.
inline constexpr float ARCHIMONDE_RANGED_SPREAD_DISTANCE = 10.0f;
// Invincibility applied by Tyrande when Archimonde is at 10% HP. Used to cut off boss strategies
// since the fight is effectively over at this point.
bool HasProtectionOfElune(Player* bot);
bool IsNearDoomfire(PlayerbotAI* botAI, float radius);
bool IsPositionNearDoomfire(PlayerbotAI* botAI, float x, float y, float radius);
// Every live Doomfire trail patch the cache holds, for callers that weigh the field rather than
// just asking whether one is near.
std::vector<Position> GetDoomfirePositions(PlayerbotAI* botAI);
extern std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;
bool GetPendingAirBurstCast(uint32 instanceId, AirBurstData& airBurst);

}

#endif
