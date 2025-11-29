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
        SPELL_ARCANE_FLURRY             = 34821,

        // Al'ar
        SPELL_REBIRTH_PHASE2            = 34342,
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
        NPC_NETHER_VAPOR                = 21002, // Improperly implemented in AC; method needed if fixed
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

    namespace TempestKeepPositions
    {
        extern const Position AlarRoomCenter;
        extern const Position AlarRangedCenter;
        extern const Position AlarPlatform1;
        extern const Position AlarPlatform2;
        extern const Position AlarPlatform3;
        extern const Position AlarPlatform4;
        extern const Position AlarPlatform1To2MidpointA;
        extern const Position AlarPlatform1To2MidpointB;
        extern const Position AlarPlatform2To3MidpointA;
        extern const Position AlarPlatform2To3MidpointB;
        extern const Position AlarGround1;
        extern const Position AlarGround2;
        extern const Position AlarGround3;
        extern const Position AlarGround4;
        extern const Position AlarSERampBase;
        extern const Position AlarSWRampBase;
        extern const Position AlarRoomSouthCenter;

        extern const Position VoidReaverTankPosition;

        extern const Position SanguinarTankPosition;
        extern const Position TelonicusTankPosition;
        extern const Position KaelthasWeaponStackPosition;
        extern const Position KaelthasAxeTankPosition;
        extern const Position KaelthasBowTankPosition;
        extern const Position KaelthasTankPosition;
    }

    constexpr uint32 TEMPESTKEEP_MAP_ID = 550;

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithCross(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
    Unit* GetNearestNonTankPlayerInRadius(Player* bot, float radius);

    void UpdateAlarLastPlatform(Unit* alar, const std::vector<Position>& platforms);
    bool IsAlarMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot);

    bool IsKaelthasInPhase1(PlayerbotAI* botAI);
    bool IsKaelthasInPhase2(PlayerbotAI* botAI);
    bool IsKaelthasInPhase2To3Transition(PlayerbotAI* botAI);
    bool IsKaelthasInPhase3(PlayerbotAI* botAI);
    bool IsKaelthasInPhase4(PlayerbotAI* botAI);
    bool IsKaelthasInPhase4To5Transition(PlayerbotAI* botAI);
    bool IsKaelthasInPhase5(PlayerbotAI* botAI);
    bool IsKaelthasMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    Player* GetCapernianTank(PlayerbotAI* botAI, Player* bot);
    Player* GetNetherstrandLongbowTank(PlayerbotAI* botAI, Player* bot);
    bool IsAnyLegendaryWeaponDead(PlayerbotAI* botAI, Player* bot);

    extern std::unordered_map<uint32, int8> lastAlarPlatform;
    extern std::unordered_map<uint32, bool> lastRebirthState;
    extern std::unordered_map<uint32, bool> isAlarInPhase2;
    extern std::unordered_map<ObjectGuid, bool> mainTankAtPlatform2;
    extern std::unordered_map<ObjectGuid, bool> assistTankAtPlatform3;
    extern std::unordered_map<ObjectGuid, std::vector<bool>> mtBalconyMidpointVisited;
    extern std::unordered_map<ObjectGuid, std::vector<bool>> atBalconyMidpointVisited;
    extern std::unordered_map<ObjectGuid, std::vector<bool>> meleeDpsWaypointVisited;

    extern std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    extern std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

    extern std::unordered_map<uint32, time_t> advisorDpsWaitTimer;
}

#endif
