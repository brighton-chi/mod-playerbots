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
        // Anetheron
        SPELL_INFERNO = 31299,

        // Kaz'rogal
        SPELL_MARK_OF_KAZROGAL         = 31447,

        // Azgalor
        SPELL_RAIN_OF_FIRE             = 31340,
        SPELL_DOOM                     = 31347,

        // Archimonde
        SPELL_DOOMFIRE = 31944,
        SPELL_AIR_BURST = 32014,

        // Hunter
        SPELL_MISDIRECTION             = 35079,

        // Priest
        SPELL_FEAR_WARD                 =  6346,
    };

    enum HyjalSummitNPCs
    {
        // Anetheron
        NPC_TOWERING_INFERNAL = 17818,

        // Archimonde
        NPC_DOOMFIRE           = 18095,
        // NPC_DOOMFIRE_TARGETING    = 18104, // Known as "DOOMFIRE_SPIRIT" in the boss script
    };

    // General
    constexpr uint32 HYJAL_SUMMIT_MAP_ID = 534;

    // Rage Winterchill
    extern const Position RAGE_WINTERCHILL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> winterchillRangedPositions;
    extern std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron
    extern const Position ANETHERON_MAIN_TANK_POSITION;
    extern const Position ANETHERON_INFERNAL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> anetheronRangedPositions;
    bool IsBotTargetedByInferno(Unit* anetheron, Player* bot);

    // Kaz'rogal
    extern const Position KAZROGAL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> kazrogalRangedPositions;

    // Azgalor
    extern const Position AZGALOR_MAIN_TANK_TRANSITION_POSITION;
    extern const Position AZGALOR_MAIN_TANK_FINAL_POSITION;
    extern const Position AZGALOR_DOOMGUARD_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> azgalorTankStep;
    bool AnyGroupMemberHasDoom(Player* bot);

    // Archimonde
    // N/A
}

#endif
