/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULHELPERS_H
#define PLAYERBOTS_GRUULHELPERS_H

#include "Common.h"
#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Position.h"
#include <type_traits>
#include <vector>

class Creature;
class Player;
class Unit;

namespace GruulHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Boss ids from gruuls_lair.h, matching DungeonEncounter.dbc's encounterIndex.
enum class GruulBosses : uint32
{
    DATA_HIGH_KING_MAULGAR      = 0,
    DATA_GRUUL_THE_DRAGONKILLER = 1,
};

enum class GruulSpells : uint32
{
    // High King Maulgar <Lord of the Ogres>
    SPELL_WHIRLWIND         = 33238, // Maulgar
    SPELL_SPELL_SHIELD      = 33054, // Krosh Firehand

    // Hunter
    SPELL_MISDIRECTION      = 34477,
    SPELL_MISDIRECTION_AURA = 35079, // the aura on the hunter

    // Mage
    SPELL_SPELLSTEAL        = 30449,

    // Gruul the Dragonkiller
    SPELL_GROUND_SLAM_1     = 33525,
    SPELL_GROUND_SLAM_2     = 39187,
    SPELL_CAVE_IN           = 36240,
};

enum class GruulNpcs : uint32
{
    // High King Maulgar <Lord of the Ogres>
    NPC_HIGH_KING_MAULGAR  = 18831,
    NPC_KROSH_FIREHAND     = 18832,
    NPC_OLM_THE_SUMMONER   = 18834,
    NPC_KIGGLER_THE_CRAZED = 18835,
    NPC_BLINDEYE_THE_SEER  = 18836,
    NPC_WILD_FEL_STALKER   = 18847,
};

// Shared

inline constexpr uint32 GRUUL_MAP_ID = 565;
inline constexpr float GRUUL_ARRIVAL_DISTANCE = 3.0f;

Unit* GetGruulsLairBoss(Player* bot, GruulBosses boss);

// High King Maulgar

// Ogre combat reaches:
// Maulgar = 3.5 yd, Olm = 2.2 yd, Blindeye = 3.525 yd, Krosh = 2 yd, Kiggler = 3.3 yd

inline constexpr EncounterHelpers::ClassRole KROSH_MAGE_TANK_ROLE = {
    .playerClass = CLASS_MAGE,
    .role = { "class_role_krosh_mage_tank", "Krosh mage tank" },
    .candidate = { "class_role_class_mage", "Mage" },
};
inline constexpr EncounterHelpers::ClassRole KIGGLER_MOONKIN_TANK_ROLE = {
    .playerClass = CLASS_DRUID,
    .requiredSpec = DRUID_TAB_BALANCE,
    .role = { "class_role_kiggler_moonkin_tank", "Kiggler moonkin tank" },
    .candidate = { "class_role_class_balance_druid", "Balance Druid" },
};
// 90 yd is about the distance from Maulgar to where the hallway opens into the council room.
inline constexpr float MAULGAR_CLASS_ROLE_ANNOUNCE_DISTANCE = 90.0f;
// 150 yd is short of the instance portal and Gruul's room.
inline constexpr float MAULGAR_CLASS_ROLE_REARM_DISTANCE = 150.0f;

// Hold cooldowns until Blindeye is at this percent health. Also the gate for on-pull Misdirects.
inline constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;
inline constexpr float KIGGLER_ARCANE_EXPLOSION_HAZARD_RADIUS = 30.0f;
// Stays inside the "enemy out of spell" threshold (spellDistance + CONTACT_DISTANCE + both reaches,
// ~34 yd exact against Kiggler), allowing the boomie to attack safely without Nature's Reach.
inline constexpr float KIGGLER_ARCANE_EXPLOSION_CLEARANCE =
    KIGGLER_ARCANE_EXPLOSION_HAZARD_RADIUS + 2.0f;
inline constexpr float KROSH_BLAST_WAVE_HAZARD_RADIUS = 15.0f;
inline constexpr float KROSH_BLAST_WAVE_CLEARANCE = KROSH_BLAST_WAVE_HAZARD_RADIUS + 2.0f;
// Whirlwind's padding is greater than usual. The tl;dr is MoveAway()'s math results in its fallback
// candidates (when running straight away is blocked) moving the bot less than the specified
// distance. Melee DPS often ends up between Maulgar and a wall and needing fallback escape angles.
inline constexpr float MAULGAR_WHIRLWIND_HAZARD_RADIUS = 8.0f;
inline constexpr float MAULGAR_WHIRLWIND_CLEARANCE = MAULGAR_WHIRLWIND_HAZARD_RADIUS + 4.0f;
inline constexpr float MAULGAR_WHIRLWIND_HOLD_DISTANCE = 15.0f;
// Measured from Maulgar's spawn. The ogre tank positions are all within about 60 yd of it, so
// 100 yd covers the whole room.
inline constexpr float MAULGAR_COUNCIL_SEARCH_RADIUS = 100.0f;
inline constexpr uint32 MAULGAR_COUNCIL_CACHE_INTERVAL_MS = 1000;
// Olm summons a Wild Fel Stalker every 48.5s (practically, that means you're not going to see more
// than 1 or 2).
inline constexpr uint32 WILD_FEL_STALKER_CACHE_INTERVAL_MS = 1000;
inline constexpr float WILD_FEL_STALKER_SEARCH_RADIUS = MAULGAR_COUNCIL_SEARCH_RADIUS;

inline Position const MAULGAR_SPAWN_POSITION = { 145.786f, 190.979f, -11.542f };
inline Position const MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
inline Position const OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
inline Position const BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
inline Position const KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };

bool IsMaulgarTank(Player* bot);
bool IsOlmTank(Player* bot);
bool IsBlindeyeTank(Player* bot);
bool UpdateGruulClassRoles(PlayerbotAI* botAI);
bool UpdateMaulgarClassRoleAnnouncement(PlayerbotAI* botAI);
Player* GetKroshMageTank(PlayerbotAI* botAI);
bool IsKroshMageTank(PlayerbotAI* botAI);
Player* GetKigglerMoonkinTank(PlayerbotAI* botAI);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI);
GuidVector FindWildFelStalkerGuids(Player* bot);
std::vector<Unit*> GetWildFelStalkers(Player* bot);
GuidVector FindMaulgarCouncilGuids(Player* bot);
Unit* GetMaulgarCouncilMember(Player* bot, GruulNpcs npc);
Unit* GetMaulgarPetTarget(Player* bot, Creature* pet);

// Gruul the Dragonkiller

inline constexpr uint32 CAVE_IN_CACHE_INTERVAL_MS = 100;
// Cave In's radius is 8 yd, plus a player's 1.5 yd combat reach (1.95 yd with Bloodlust), rounded
// up. Pools last 15s and can be cast as often as every 3.5s, so up to 5 can be down at once.
inline constexpr float CAVE_IN_RADIUS = 10.0f;
// Distance for the multiplier. As usual, wider than the avoidance to prevent running back in.
inline constexpr float CAVE_IN_CONTROL_RADIUS = CAVE_IN_RADIUS + 5.0f;
inline constexpr float CAVE_IN_SEARCH_RADIUS = CAVE_IN_CONTROL_RADIUS + 2.0f;
// Shatter hits players within 20 yd of a stoned player, edge to edge: 23 yd center to center
// (23.9 yd with Bloodlust). Damage falls off linearly with distance.
inline constexpr float GRUUL_SHATTER_HAZARD_RADIUS = 23.0f;
// Players are still landing from Ground Slam and moving when bots start to spread, so bots aim 5 yd
// past the hazard radius, which also covers Bloodlust.
inline constexpr float GRUUL_SHATTER_CLEARANCE = GRUUL_SHATTER_HAZARD_RADIUS + 5.0f;
inline constexpr float GRUUL_SHATTER_STEP_DISTANCE = 3.5f;
inline constexpr float GRUUL_SHATTER_STEP_TOLERANCE = 0.5f;
// The portcullis (x 170.2) closes during the fight, but bot pathing ignores doors, so spread steps
// never end south of this line.
inline constexpr float GRUUL_ROOM_MIN_X = 172.0f;
// While the tank is moving and still within Gruul's melee range, Gruul chases to his reach minus
// half the tank's speed, not to the tank's lead distance, so Gruul can stop up to 4 yd past the
// center.
inline constexpr float GRUUL_BOSS_ARRIVAL_DISTANCE = 5.0f;

// Where the tank holds Gruul: the center of his room, with the most open floor (42 yd to the
// nearest navmesh edge). Ranged bots spread in a ring around it.
inline Position const GRUUL_TANKING_POSITION = { 231.217f, 363.233f, -4.420f };

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool);
bool IsNearCaveIn(PlayerbotAI* botAI, float radius);
bool IsInCaveIn(PlayerbotAI* botAI);
bool HasGroundSlam(Player* bot);
bool CanMoveStraightTo(Player* bot, Position const& destination);

// Shared encounter state

struct GruulInstanceState
{
    EncounterHelpers::ClassRoleAssignment kroshMageTank;
    EncounterHelpers::ClassRoleAssignment kigglerMoonkinTank;
    EncounterHelpers::InstanceCreatureList maulgarCouncil;
    EncounterHelpers::InstanceCreatureList wildFelStalkers;
};

GruulInstanceState& GruulState(uint32 instanceId);
GruulInstanceState* FindGruulState(uint32 instanceId);
void GruulResetInstance(uint32 instanceId);

}

#endif
