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

        // Anetheron
        SPELL_INFERNO = 31299,
        SPELL_IMMOLATION = 31303,

        // Kaz'rogal
        SPELL_MARK_OF_KAZROGAL         = 31447,

        // Azgalor
        SPELL_DOOM                     = 31347,

        // Hunter
        SPELL_MISDIRECTION             = 35079,
    };

    enum HyjalSummitNPCs
    {
        // Anetheron
        NPC_TOWERING_INFERNAL = 17818,
    };

    // General
    const uint32 HYJAL_SUMMIT_MAP_ID = 534;
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot);

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

    // Azgalor
    extern const Position AZGALOR_TANK_POSITION;

    // Archimonde
    extern const Position ARCHIMONDE_TANK_POSITION;
}

#endif
