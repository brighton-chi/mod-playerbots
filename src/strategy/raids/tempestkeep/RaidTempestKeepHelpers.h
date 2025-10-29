#ifndef _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

enum TempestKeepSpells
{
    // Al'ar
    SPELL_REBIRTH_PHASE2 = 34342,

    // High Astromancer Solarian
    SPELL_SOLARIAN_TRANSFORM        = 39117,
    SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

    // Priest
    SPELL_FEAR_WARD               =  6346,
};

enum TempestKeepNPCs
{
    // High Astromancer Solarian
    // NPC_SOLARIUM_AGENT                  = 18925,
    NPC_SOLARIUM_PRIEST                 = 18806,
};

namespace TempestKeepHelpers
{

    struct Location 
    {
        float x, y, z;
    };

    namespace TempestKeepLocations
    {
        extern const Location AlarRoomCenter;
        extern const Location AlarPlatform1;
        extern const Location AlarPlatform2;
        extern const Location AlarPlatform3;
        extern const Location AlarPlatform4;
        extern const Location AlarGround1;
        extern const Location AlarGround2;
        extern const Location AlarGround3;
        extern const Location AlarGround4;

        extern const Location VoidReaverTankPosition;
        
        // extern const Location AstromancerStackPosition;
    }

    extern std::unordered_map<uint32, int8> lastAlarPlatform;
    extern std::unordered_map<uint32, bool> lastRebirthState;

    void UpdateAlarLastPlatform(Unit* alar, uint32 mapId, const std::vector<Location>& platforms);

    extern std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    extern std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot);

}


#endif
