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
        // Al'ar
        // SPELL_FLAME_QUILLS              = 34229, //aura present when actually casting
        SPELL_REBIRTH_PHASE2 = 34342,
        // SPELL_DIVE_BOMB                 = 35181,
        // SPELL_DIVE_BOMB_VISUAL          = 35367,
        SPELL_MELT_ARMOR                = 35410,

        // High Astromancer Solarian
        SPELL_SOLARIAN_TRANSFORM        = 39117,
        SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

        // Hunter
        SPELL_MISDIRECTION = 34477,

        // Priest
        SPELL_FEAR_WARD               =  6346,
    };

    enum TempestKeepNPCs
    {
        // Al'ar
        NPC_EMBER_OF_ALAR           = 19551,

        // High Astromancer Solarian
        // NPC_SOLARIUM_AGENT                  = 18925,
        NPC_SOLARIUM_PRIEST                 = 18806,
    };

    struct Location
    {
        float x, y, z;
    };

    namespace TempestKeepLocations
    {
        extern const Location AlarRangedCenter;
        extern const Location AlarPlatform1;
        extern const Location AlarPlatform2;
        extern const Location AlarPlatform3;
        extern const Location AlarPlatform4;
        extern const Location AlarPlatform1To2MidpointA;
        extern const Location AlarPlatform1To2MidpointB;
        extern const Location AlarPlatform2To3MidpointA;
        extern const Location AlarPlatform2To3MidpointB;
        extern const Location AlarPlatform3To4MidpointA;
        extern const Location AlarPlatform3To4MidpointB;
        extern const Location AlarGround1;
        extern const Location AlarGround2;
        extern const Location AlarGround3;
        extern const Location AlarGround4;
        extern const Location AlarSERampBase;
        extern const Location AlarMidpointToSERamp;
        extern const Location AlarSWRampBase;
        extern const Location AlarMidpointToSWRamp;
        extern const Location AlarRoomSouthCenter;

        extern const Location VoidReaverTankPosition;

        // extern const Location AstromancerStackPosition;
    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);

    extern std::unordered_map<uint32, int8> lastAlarPlatform;
    extern std::unordered_map<uint32, bool> lastRebirthState;
    extern std::unordered_map<uint32, bool> isPhase2;

    // extern std::unordered_map<ObjectGuid, int8> lastMainTankPlatform;
    // extern std::unordered_map<ObjectGuid, int8> lastAssistTankPlatform;
    extern std::unordered_map<ObjectGuid, bool> mainTankAtPlatform2;
    extern std::unordered_map<ObjectGuid, bool> assistTankAtPlatform3;

    extern std::unordered_map<ObjectGuid, std::vector<bool>> mtBalconyMidpointVisited;
    extern std::unordered_map<ObjectGuid, std::vector<bool>> atBalconyMidpointVisited;
    // extern std::unordered_map<ObjectGuid, bool> mtGroundMidpointVisited;
    // extern std::unordered_map<ObjectGuid, bool> atGroundMidpointVisited;

    // extern std::vector<Location> midpoints_1_to_6;
    // extern std::vector<Location> midpoints_6_to_1;
    extern std::vector<Location> midpoints_1_to_2;
    extern std::vector<Location> midpoints_2_to_1;
    extern std::vector<Location> midpoints_4_to_3;
    extern std::vector<Location> midpoints_3_to_4;

    void UpdateAlarLastPlatform(Unit* alar, uint32 mapId, const std::vector<Location>& platforms);
    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot);

    extern std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    extern std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;
}

#endif
