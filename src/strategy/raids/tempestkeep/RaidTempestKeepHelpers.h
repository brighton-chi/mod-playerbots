#ifndef _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

enum TempestKeepSpells
{
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
    extern const Location VoidReaverTankPosition;
    extern const Location AstromancerStackPosition;
}

extern std::unordered_map<ObjectGuid, Position> initialPositions;
extern std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
void MarkTargetWithSquare(Player* bot, Unit* target);
void MarkTargetWithStar(Player* bot, Unit* target);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);

}


#endif
