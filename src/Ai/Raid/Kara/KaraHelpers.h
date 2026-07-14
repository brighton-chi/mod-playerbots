/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAHELPERS_H
#define PLAYERBOTS_KARAHELPERS_H

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"
#include <ctime>
#include <unordered_map>

namespace KarazhanHelpers
{

enum class KarazhanSpells : uint32
{
    // Maiden of Virtue
    SPELL_REPENTANCE                 = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD     = 30756,

    // The Curator
    SPELL_CURATOR_EVOCATION          = 30254,

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST          = 30004,
    SPELL_FLAME_WREATH_AURA          = 29946,
    SPELL_ARCANE_EXPLOSION           = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF            = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF          = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF           = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_GREEN_BEAM_HEAL            = 30467, // "Nether Portal - Serenity" (Netherspite aura)
    SPELL_NETHER_EXHAUSTION_RED      = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN    = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE     = 38639,
    SPELL_NETHERSPITE_BANISHED       = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                   = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH              = 30129,
    SPELL_BELLOWING_ROAR             = 36922,
    SPELL_RAIN_OF_BONES              = 37091,

    // Priest
    SPELL_FEAR_WARD                  = 6346,

    // Shaman
    SPELL_TREMOR_TOTEM               = 8143,
    SPELL_GROUNDING_TOTEM            = 8177,
};

enum class KarazhanNpcs : uint32
{
    // Trash
    NPC_MANA_WARP                    = 16530,

    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN         = 15550,
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Terestian Illhoof
    NPC_TERESTIAN_ILLHOOF            = 15688,
    NPC_DEMON_CHAINS                 = 17248,
    NPC_KILREK                       = 17229,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    NPC_VOID_ZONE                    = 16697,
    NPC_GREEN_PORTAL                 = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL                  = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                   = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL         = 17646,
};

constexpr uint32 KARAZHAN_MAP_ID = 532;
constexpr float NIGHTBANE_FLIGHT_Z = 95.0f;

// Attumen the Huntsman
extern std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
// Netherspite
extern std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
// Nightbane
extern std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
extern std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

bool IsCastingArcaneExplosion(Unit* aran);
bool IsFlameWreathActive(Player* bot);
bool IsBanishPhase(Unit* netherspite);
std::vector<Player*> GetRedBlockers(Player* bot);
std::vector<Player*> GetBlueBlockers(Player* bot);
std::vector<Player*> GetGreenBlockers(Player* bot);
std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot);
std::vector<Unit*> GetAllVoidZones(Player* bot);
bool IsSafePosition (float x, float y, const std::vector<Unit*>& hazards, float hazardRadius);
bool FindBeamPosition(
    Unit* boss, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos);
std::vector<Unit*> GetSpawnedInfernals(Player* bot);
bool IsStraightPathSafe(
    float startX, float startY, float targetX, float targetY,
    std::vector<Unit*> const& hazards, float hazardRadius);
bool TryFindSafePositionWithSafePath(
    Player* bot, Position const& origin, Position const& center, std::vector<Unit*> const& hazards,
    float safeDistance, float maxSampleDist, float& outX, float& outY);

}

#endif
