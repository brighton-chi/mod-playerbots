#ifndef _PLAYERBOT_RAIDHYJALSUMMITHELPERS_H_
#define _PLAYERBOT_RAIDHYJALSUMMITHELPERS_H_

#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace HyjalSummitHelpers
{
    enum HyjalSummitSpells
    {
        // Rage Winterchill
        SPELL_DEATH_AND_DECAY          = 31258,

        // Hunter
        SPELL_MISDIRECTION             = 35079,
    };

    enum HyjalSummitNPCs
    {
        // Example NPC
        NPC_EXAMPLE           = 67890,
    };

    // General
    const uint32 HYJAL_SUMMIT_MAP_ID = 534;

    // Rage Winterchill
    extern const Position RAGE_WINTERCHILL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> winterchillRangedPositions;

    // Anetheron
    extern const Position ANETHERON_TANK_POSITION;

    // Kaz'rogal

    // Azgalor

    // Archimonde

}

#endif
