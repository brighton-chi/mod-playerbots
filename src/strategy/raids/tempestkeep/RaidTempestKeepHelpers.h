#ifndef _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_

#include <ctime>
#include <unordered_map>
#include <vector>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace TempestKeepHelpers
{
    enum TempestKeepSpells
    {
        // Trash
        SPELL_ARCANE_FLURRY             = 37268,

        // Al'ar
        SPELL_REBIRTH_PHASE2            = 34342,
        SPELL_REBIRTH_DIVE              = 35369,
        SPELL_MELT_ARMOR                = 35410,

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

        // Mage
        SPELL_POLYMORPH_SHEEP           = 12826,
        SPELL_POLYMORPH_TURTLE          = 28271,
        SPELL_POLYMORPH_PIG             = 28272,

        // Priest
        SPELL_FEAR_WARD                 =  6346,
    };

    enum TempestKeepNPCs
    {
        // Al'ar
        NPC_EMBER_OF_ALAR               = 19551,
        NPC_FLAME_PATCH                 = 20602,

        // High Astromancer Solarian
        NPC_SOLARIUM_PRIEST             = 18806,

        // Kael'thas Sunstrider
        NPC_KAELTHAS_SUNSTRIDER         = 19622,
        NPC_LORD_SANGUINAR              = 20060,
        NPC_GRAND_ASTROMANCER_CAPERNIAN = 20062,
        NPC_MASTER_ENGINEER_TELONICUS   = 20063,
        NPC_THALADRED_THE_DARKENER      = 20064,
        NPC_NETHERSTRAND_LONGBOW        = 21268,
        NPC_DEVASTATION                 = 21269,
        NPC_COSMIC_INFUSER              = 21270,
        NPC_INFINITY_BLADES             = 21271, // Item is singular, but NPC is plural
        NPC_WARP_SLICER                 = 21272,
        NPC_PHASESHIFT_BULWARK          = 21273,
        NPC_STAFF_OF_DISINTEGRATION     = 21274,
        // NPC_NETHER_VAPOR             = 21002, // Unimplemented in AC; method needed if fixed
        NPC_PHOENIX                     = 21362,
        NPC_FLAME_STRIKE_TRIGGER        = 21369,
    };

    enum TempestKeepItems
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

    // General
    const uint32 TEMPEST_KEEP_MAP_ID = 550;
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithCross(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
    Unit* GetNearestNonTankPlayerInRadius(Player* bot, float radius);
    std::vector<Unit*> GetAllHazardTriggers(
        PlayerbotAI* botAI, Player* bot, uint32 npcEntry, float maxSearchRadius);
    Position FindSafestNearbyPosition(Player* bot, const std::vector<Unit*>& hazards,
        float maxRadius, float hazardRadius, const Position* center = nullptr);
    bool IsPathSafeFromHazards(
        const Position& start, const Position& end, const std::vector<Unit*>& hazards,
        float hazardRadius);

    // Al'ar <Phoenix God>
    const float ALAR_BALCONY_Z = 17.0f;
    extern const Position ALAR_PLATFORM_0;
    extern const Position ALAR_PLATFORM_1;
    extern const Position ALAR_PLATFORM_2;
    extern const Position ALAR_PLATFORM_3;
    extern const Position PLATFORM_POSITIONS[4];
    extern const Position ALAR_GROUND_0;
    extern const Position ALAR_GROUND_1;
    extern const Position ALAR_GROUND_2;
    extern const Position ALAR_GROUND_3;
    extern const Position GROUND_POSITIONS[4];
    extern const Position ALAR_ROOM_CENTER;
    extern const Position ALAR_POINT_QUILL_OR_DIVE;
    extern const Position ALAR_POINT_MIDDLE;
    extern const Position ALAR_SE_RAMP_BASE;
    extern const Position ALAR_SW_RAMP_BASE;
    extern const Position ALAR_ROOM_S_CENTER;
    extern std::unordered_map<uint32, bool> lastRebirthState;
    extern std::unordered_map<uint32, bool> isAlarInPhase2;
    int8 GetAlarDestinationLocationIndex(Unit* alar, Position& dest);
    int8 GetAlarCurrentLocationIndex(Unit* alar);
    void GetClosestPlatformAndGround(
        const Position& botPos, int8& closestPlatform, Position& ground);
    std::pair<Unit*, Unit*> GetFirstTwoEmbersOfAlar(PlayerbotAI* botAI);
    Player* GetSecondEmberTank(PlayerbotAI* botAI, Unit* alar);

    // Void Reaver
    extern const Position VOID_REAVER_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    extern std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

    // High Astromancer Solarian
    // N/A

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    extern const Position SANGUINAR_TANK_POSITION;
    extern const Position SANGUINAR_WAITING_POSITION;
    extern const Position TELONICUS_TANK_POSITION;
    extern const Position TELONICUS_WAITING_POSITION;
    extern const Position CAPERNIAN_WAITING_POSITION;
    extern const Position ADVISOR_HEAL_POSITION;
    extern const Position KAELTHAS_WEAPON_STACK_POSITION;
    extern const Position KAELTHAS_AXE_TANK_POSITION;
    extern const Position KAELTHAS_BOW_TANK_POSITION;
    extern const Position KAELTHAS_TANK_POSITION;
    extern std::unordered_map<uint32, time_t> advisorDpsWaitTimer;
    Player* GetCapernianTank(PlayerbotAI* botAI, Player* bot);
    Player* GetNetherstrandLongbowTank(PlayerbotAI* botAI, Player* bot);
    bool IsAnyLegendaryWeaponDead(PlayerbotAI* botAI, Player* bot);
    bool HasEquippableItemForSlot(Player* bot, uint8 slot);
    bool HasEquippableOffhand(Player* bot);
}

#endif
