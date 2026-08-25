/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPSharedConstants.h"
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct MuruEncounterTargets
{
    Unit* muru = nullptr;
    Unit* entropius = nullptr;
    std::vector<Unit*> voidSentinels;
    std::vector<Unit*> voidSpawns;
    std::vector<Unit*> furyMages;
    std::vector<Unit*> berserkers;
};

// What the "muru encounter targets" value stores. Guids rather than the pointers above: the value
// outlives the tick that produced it, and CalculatedValue carries no IsInWorld guard on RefGet or
// on any non-Unit specialisation, so a stored Unit* would dangle on a despawn.
struct MuruEncounterGuids
{
    ObjectGuid muru;
    ObjectGuid entropius;
    GuidVector voidSentinels;
    GuidVector voidSpawns;
    GuidVector furyMages;
    GuidVector berserkers;
};

struct MuruDarknessState
{
    uint32 startMs = 0;
    uint32 expireMs = 0;
};

inline Position const MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };
inline Position const MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
inline Position const MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
inline Position const MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
inline Position const MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };

// Ability reaches from SpellRange.dbc (MaxRangeHostile). Berserker and fury mage selection filters
// candidates on these because PlayerbotAI::CanCastSpell whitelists SPELL_FAILED_OUT_OF_RANGE and
// returns true anyway - an unreachable candidate would go on to fail in Spell::prepare, and that
// failure path returns before CastSpell restores the bot's previous selection.
inline constexpr float MURU_MELEE_ABILITY_REACH = 5.0f;
inline constexpr float MURU_WAR_STOMP_REACH = 8.0f;
inline constexpr float MURU_HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float MURU_WIND_SHEAR_REACH = 25.0f;
inline constexpr float MURU_RANGED_ABILITY_REACH = 30.0f;
inline constexpr float MURU_SILENCING_SHOT_REACH = 35.0f;

// Feeds the "muru encounter targets" value. CalculatedValue reads any interval between 2 and 99 as
// seconds, so this has to stay at or above 100 to mean milliseconds. Only list membership is
// cached; every state read (Flurry, casting, health) comes from the freshly resolved unit.
inline constexpr uint32 MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS = 200;

// Darkness cycle: 45998 ticks every 45s and triggers the 3s pre-effect 45999, whose own tick casts
// 45996 - a 15 yard zone doing 3k a second and cutting healing to zero. 45996 also lands on M'uru
// itself (effect 2 targets the caster), so once it is up the window is read off that aura and
// these two are only the estimate used during the telegraph, before the aura exists.
inline constexpr uint32 MURU_DARKNESS_PRE_EFFECT_MS = 3000;
inline constexpr uint32 MURU_DARKNESS_AURA_MS = 20000;

// Melee are let go this far before the zone actually drops, so the run back overlaps its tail
// instead of starting after it
inline constexpr uint32 MURU_DARKNESS_RUN_BACK_ALLOWANCE_MS = 2000;

// How long after a darkness starts tanks are still expected to be running for their holding spot
inline constexpr uint32 MURU_DARKNESS_EARLY_WINDOW_MS = 10000;

// Darkness damages within 15 yards of M'uru; the rest is the usual avoidance padding
inline constexpr float MURU_DARKNESS_SAFE_DISTANCE = 20.0f;

// How far from its own holding spot a target may sit and still count as worth reaching for.
inline constexpr float MURU_HOLDING_POSITION_RADIUS = 20.0f;

// Nearest wins the DPS slot, but only by a clear margin
inline constexpr float MURU_TARGET_SWITCH_MARGIN = 10.0f;

// Radius of Shadow Bolt Volley (46082), which is centred on the enslaved Void Spawn
inline constexpr float MURU_SHADOW_BOLT_VOLLEY_RADIUS = 20.0f;

// The void zone (25879) carries a permanent 46262, ticking 46264 for 3k in a 3 yard radius, and
// nothing despawns it - so the margin here is for movement lag, not for the hazard itself
inline constexpr float MURU_VOID_ZONE_SAFE_DISTANCE = 8.0f;
inline constexpr float MURU_VOID_ZONE_SEARCH_RADIUS = 12.0f;

// Feeds the "muru void zones" value.
inline constexpr uint32 MURU_VOID_ZONE_CACHE_INTERVAL_MS = 200;

inline constexpr float MURU_SINGULARITY_SEARCH_RADIUS = 30.0f;

// Feeds the "muru singularity" value. Only one exists at a time - Entropius casts Black Hole every
// 29 seconds and npc_singularity despawns itself after 18 - so caching the nearest cannot pick the
// wrong one. It chases a player, but only the guid is cached.
inline constexpr uint32 MURU_SINGULARITY_CACHE_INTERVAL_MS = 200;

// A Dark Fiend detonates within 2 yards of whoever it is chasing. The safe distance is
// deliberately wide as touching a single Dark Fiend is almost a guaranteed wipe.
inline constexpr float MURU_DARK_FIEND_SAFE_DISTANCE = 10.0f;

// Tanks drag nothing further than this from the ranged stack
inline constexpr float MURU_MAX_TARGET_DIST_FROM_STACK = 25.0f;

inline constexpr float MURU_MISDIRECT_MIN_TARGET_HP_PERCENT = 80.0f;

// DPS cooldowns are held until 97% to allow for initial positioning
inline constexpr float MURU_MAX_DPS_HP_PERCENT = 97.0f;

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool IsMuruPhaseActive(Unit* muru);
bool TryGetMuruDarknessEarlyState(
    Player* bot, Unit* muru, uint32 earlyWindowMs = MURU_DARKNESS_EARLY_WINDOW_MS);
MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI);
bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI);
GuidVector FindMuruVoidZoneGuids(Player* bot);
ObjectGuid FindMuruSingularityGuid(Player* bot);
Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI);
Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI);

}

#endif
