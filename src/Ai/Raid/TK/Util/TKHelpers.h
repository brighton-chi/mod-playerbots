/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKHELPERS_H
#define PLAYERBOTS_TKHELPERS_H

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"
#include <ctime>
#include <unordered_map>
#include <vector>

namespace TkHelpers
{

enum class TkSpells : uint32
{
    // Trash
    SPELL_ARCANE_FLURRY             = 37268,

    // Al'ar
    SPELL_MODEL_INVISIBILITY        = 24401, // "Test Pet Passive" spell (AC hack used for Al'ar)
    SPELL_REBIRTH_PHASE2            = 34342,
    SPELL_REBIRTH_DIVE              = 35369,
    SPELL_MELT_ARMOR                = 35410,

    // Void Reaver
    SPELL_ARCANE_ORB                = 34172,

    // High Astromancer Solarian
    SPELL_SOLARIAN_TRANSFORM        = 39117,
    SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

    // Kael'thas Sunstrider
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_GRAVITY_LAPSE             = 39432,
    SPELL_KAEL_FULL_POWER           = 36187,
    SPELL_MENTAL_PROTECTION_FIELD   = 36480, // Staff of Disintegration
    SPELL_ARCANE_BARRIER            = 36481, // Phaseshift Bulwark
    SPELL_KAELTHAS_MIND_CONTROL     = 36797,
    SPELL_SHOCK_BARRIER             = 36815,
    SPELL_STAFF_FROSTBOLT           = 36990,

    // Hunter
    SPELL_MISDIRECTION              = 35079,

    // Priest
    SPELL_FEAR_WARD                 = 6346,
};

enum class TkNpcs : uint32
{
    // General
    NPC_CRIMSON_HAND_CENTURION      = 20048,

    // Al'ar
    NPC_EMBER_OF_ALAR               = 19551,
    NPC_FLAME_PATCH                 = 20602,

    // High Astromancer Solarian
    NPC_SOLARIUM_PRIEST             = 18806,

    // Kael'thas Sunstrider
    NPC_KAELTHAS_SUNSTRIDER         = 19622,
    NPC_NETHERSTRAND_LONGBOW        = 21268,
    NPC_DEVASTATION                 = 21269,
    NPC_COSMIC_INFUSER              = 21270,
    NPC_INFINITY_BLADES             = 21271,
    NPC_WARP_SLICER                 = 21272,
    NPC_PHASESHIFT_BULWARK          = 21273,
    NPC_STAFF_OF_DISINTEGRATION     = 21274,
    // NPC_NETHER_VAPOR             = 21002, // Unimplemented in AC; method needed if fixed
    NPC_PHOENIX                     = 21362,
    NPC_PHOENIX_EGG                 = 21364,
    NPC_FLAME_STRIKE_TRIGGER        = 21369,
};

enum class TkItems : uint32
{
    // Kael'thas Sunstrider
    ITEM_WARP_SLICER                = 30311,
    ITEM_INFINITY_BLADE             = 30312,
    ITEM_STAFF_OF_DISINTEGRATION    = 30313,
    ITEM_PHASESHIFT_BULWARK         = 30314,
    ITEM_DEVASTATION                = 30316,
    ITEM_COSMIC_INFUSER             = 30317,
    ITEM_NETHERSTRAND_LONGBOW       = 30318,
    ITEM_NETHER_SPIKES              = 30319,
};

// General
constexpr uint32 TK_MAP_ID = 550;
std::pair<Unit*, Unit*> GetTargetUnitPair(PlayerbotAI* botAI, uint32 entry);
Unit* GetNearestNonTankPlayerInRadius(Player* bot, float radius);
std::vector<Unit*> GetAllHazardTriggers(Player* bot, uint32 npcEntry, float searchRadius);
Position FindSafestNearbyPosition(
    Player* bot, std::vector<Unit*> const& hazards,
    float hazardRadius, Position const* center = nullptr);
bool IsPathSafeFromHazards(
    Position const start, Position const end, std::vector<Unit*> const& hazards,
    float hazardRadius);

// Al'ar <Phoenix God>
enum AlarLocationIndex
{
    PLATFORM_0_IDX,
    PLATFORM_1_IDX,
    PLATFORM_2_IDX,
    PLATFORM_3_IDX,
    POINT_QUILL_OR_DIVE_IDX,
    POINT_MIDDLE_IDX,
    LOCATION_NONE = -1
};
constexpr float ALAR_BALCONY_Z = 17.0f;
extern Position const ALAR_PLATFORM_0;
extern Position const ALAR_PLATFORM_1;
extern Position const ALAR_PLATFORM_2;
extern Position const ALAR_PLATFORM_3;
extern std::array<Position, 4> const PLATFORM_POSITIONS;
extern Position const ALAR_GROUND_0;
extern Position const ALAR_GROUND_1;
extern Position const ALAR_GROUND_2;
extern Position const ALAR_GROUND_3;
extern std::array<Position, 4> const GROUND_POSITIONS;
extern Position const ALAR_ROOM_CENTER;
extern Position const ALAR_POINT_QUILL_OR_DIVE;
extern Position const ALAR_POINT_MIDDLE;
extern Position const ALAR_SE_RAMP_BASE;
extern Position const ALAR_SW_RAMP_BASE;
extern Position const ALAR_ROOM_S_CENTER;
constexpr uint8 TOTAL_ALAR_LOCATIONS = 6;
extern std::unordered_map<uint32, bool> lastRebirthState;
extern std::unordered_map<uint32, bool> isAlarInPhase2;
int8 GetAlarDestinationLocationIndex(Unit* alar, Position dest);
int8 GetAlarCurrentLocationIndex(Unit* alar);
void GetClosestPlatformAndGround(Position const botPos, int8& closestPlatform, Position& ground);
Player* GetSecondEmberTank(Player* bot);

// Void Reaver
struct ArcaneOrbData
{
    Position destination;
    uint32 castTime;
};
extern std::unordered_map<uint32, std::vector<ArcaneOrbData>> voidReaverArcaneOrbs;

// High Astromancer Solarian
bool HasWrathOfTheAstromancer(Player* bot);

// Kael'thas Sunstrider <Lord of the Blood Elves>
extern Position const SANGUINAR_TANK_POSITION;
extern Position const SANGUINAR_WAITING_POSITION;
extern Position const TELONICUS_TANK_POSITION;
extern Position const TELONICUS_WAITING_POSITION;
extern Position const CAPERNIAN_WAITING_POSITION;
extern Position const ADVISOR_HEAL_POSITION;
extern Position const KAELTHAS_TANK_POSITION;
extern std::unordered_map<uint32, time_t> advisorDpsWaitTimer;
Player* GetCapernianTank(Player* bot);
bool IsDebuffHunter(Player* bot);
bool IsAnyLegendaryWeaponDead(Player* bot);
bool IsFeigningDeath(Unit* advisor);
bool HasEquippableItemForSlot(Player* bot, uint8 slot);

}

#endif
