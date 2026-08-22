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

extern std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
extern std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru);
bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs = 10000);
MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI);
void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets);
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI);
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI);
Creature* FindAvailableVoidSpawnForEnslave(Player* bot);

}

#endif
