/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERMURU_H
#define PLAYERBOTS_SWPENCOUNTERMURU_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
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

struct MuruEncounterGuids
{
    ObjectGuid muru;
    ObjectGuid entropius;
    GuidVector voidSentinels;
    GuidVector voidSpawns;
    GuidVector furyMages;
    GuidVector berserkers;
};

inline constexpr float MURU_MISDIRECT_MIN_TARGET_HP_PERCENT = 80.0f;
// Dps cooldowns are held until 97% to allow for initial positioning.
inline constexpr float MURU_MAX_DPS_HP_PERCENT = 97.0f;

inline constexpr uint32 MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 VOID_ZONE_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 DARK_FIEND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 SINGULARITY_CACHE_INTERVAL_MS = 200;

// Darkness cycle: 45998 ticks every 45s and triggers the 3s pre-effect 45999 on M'uru, whose own
// tick casts 45996, a zone at M'uru's position doing 3k a second for 20s. The zone is M'uru's
// dynamic object, so it outlasts the aura wipe at the transition to Entropius.
inline constexpr uint32 MURU_DARKNESS_PRE_EFFECT_MS = 3 * IN_MILLISECONDS;
inline constexpr uint32 MURU_DARKNESS_DURATION_MS = 20 * IN_MILLISECONDS;
// This is an arbitrary window to allow tanks a bit more time to get positioned after Darkness.
inline constexpr uint32 MURU_DARKNESS_EARLY_WINDOW_MS = 10 * IN_MILLISECONDS;
// The zone hits a player whose center is within 16.5y of M'uru's (15y plus the player's reach).
inline constexpr float MURU_DARKNESS_RADIUS = 15.0f;
inline constexpr float MURU_DARKNESS_CLEARANCE = MURU_DARKNESS_RADIUS + 5.0f;
// Melee don't attack a unit this close to M'uru during Darkness: 1.5y for their reach, 5y for melee
// range, and a 1.5y margin.
inline constexpr float MURU_DARKNESS_MELEE_TARGET_DISTANCE = MURU_DARKNESS_RADIUS + 8.0f;
// Tanks won't try to pick up anything farther than this distance from the ranged stack.
inline constexpr float MURU_MAX_TARGET_DIST_FROM_STACK = 30.0f;
// The maximum distance from the melee dps holding spot that they wander to attack during Darkness.
inline constexpr float MURU_HOLDING_POSITION_RADIUS = 25.0f;
// Targeting is based on the nearest mob; this buffer is to keep targets sticky.
inline constexpr float MURU_TARGET_SWITCH_MARGIN = 10.0f;
// Radius of Shadow Bolt Volley (46082), which is centered on the enslaved Void Spawn.
inline constexpr float MURU_SHADOW_BOLT_VOLLEY_RADIUS = 20.0f;
// Shadow Pulse (46087) is a 10y AoE centered on the Void Sentinel, pulsed every 3s by 46086. Melee
// end up ~4y from their target, and there's also some variability from the tanking moving adds.
inline constexpr float MURU_MELEE_ADD_MIN_DIST_FROM_SENTINEL = 15.0f;

// Void Zones (25879) have aura 46262, ticking 46264 for 3k in a 3y radius, and spawn Dark Fiends.
// The wide safe distance is in anticipation of the Dark Fiend spawn. Search is measured by
// IsWithinDist, which adds both combat reaches for a total of 14.5y.
inline constexpr float MURU_VOID_ZONE_CLEARANCE = 10.0f;
inline constexpr float MURU_VOID_ZONE_SEARCH_RADIUS = MURU_VOID_ZONE_CLEARANCE + 2.0f;
// Dark Fiend search radii for killing (dispelling) and avoiding, respectively.
inline constexpr float DARK_FIEND_DISPEL_SEARCH_RADIUS = 50.0f;
inline constexpr float DARK_FIEND_AVOID_SEARCH_RADIUS = 15.0f;
// A Dark Fiend detonates within 7y (center to center) of whoever it is chasing. The safe distance
// is deliberately wide as touching a single Dark Fiend is almost a guaranteed wipe.
inline constexpr float DARK_FIEND_SAFE_DISTANCE = 12.0f;
inline constexpr float SINGULARITY_SEARCH_RADIUS = 30.0f;
// Distance kept from a Singularity. The active tank's distance is greater in order to leave space
// for melee on Entropius.
inline constexpr float SINGULARITY_SAFE_DISTANCE = 15.0f;
inline constexpr float SINGULARITY_TANK_SAFE_DISTANCE = 20.0f;
// Hunters and shamans leave a new Spell Fury this long for a mage to Spellsteal, which allows for a
// mage's global cooldown.
inline constexpr uint32 SPELL_FURY_MAGE_PRIORITY_MS = 2 * IN_MILLISECONDS;

inline constexpr float MURU_RANGED_STACK_ARRIVAL_DISTANCE = 3.5f;
inline constexpr float MURU_MELEE_STACK_ARRIVAL_DISTANCE = 4.5f;
inline constexpr float MURU_TANK_STACK_ARRIVAL_DISTANCE = 3.0f;
inline constexpr float MURU_TANK_HOLDING_ARRIVAL_DISTANCE = 2.5f;
inline constexpr float MURU_ENTRANCE_ARRIVAL_DISTANCE = 1.0f;
inline constexpr float MURU_VOID_SENTINEL_TANK_ARRIVAL_DISTANCE = 2.0f;
inline constexpr float ENTROPIUS_RANGED_ARRIVAL_DISTANCE = 2.0f;

inline Position const MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };
inline Position const MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
inline Position const MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
inline Position const MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
inline Position const MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };
// Where the main tank waits for Entropius while a Darkness from phase 1 lasts: on the line from
// M'uru to the ranged stack, at the Darkness clearance.
inline Position const ENTROPIUS_TANK_DARKNESS_POSITION =   { 1831.754f, 612.850f, 70.841f };

bool IsMuruPhaseActive(Unit* muru);
bool IsMuruDarknessActive(Unit* muru);
bool IsMuruDarknessEarly(Unit* muru);
bool IsMeleeTargetInMuruDarkness(Unit* muru, Unit* target);
bool IsEntropiusDpsHeld(Unit* muru, Unit* entropius);
bool IsSpellFuryReservedForMages(Unit* furyMage);
MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Unit* SelectNearestMuruTargetByEntry(
    Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates,
    Position const& origin);
bool IsMuruAddInVoidSentinelPulse(Unit* add, std::vector<Unit*> const& voidSentinels);
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI);
Position const& GetAssignedVoidSentinelTankPosition(Unit* voidSentinel);
bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI);
GuidVector FindMuruVoidZoneGuids(Player* bot);
GuidVector FindMuruDarkFiendGuids(Player* bot);
std::vector<Creature*> GetMuruDarkFiends(PlayerbotAI* botAI);
Creature* GetNearestMuruDarkFiend(PlayerbotAI* botAI, float radius);
ObjectGuid FindMuruSingularityGuid(Player* bot);
Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI);
Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI);

// Shared encounter state

struct MuruEncounterState
{
    std::unordered_map<ObjectGuid, uint8> voidSentinelTankAssignments;
};

MuruEncounterState& MuruState(uint32 instanceId);
MuruEncounterState* FindMuruState(uint32 instanceId);

}

#endif
