#ifndef _PLAYERBOT_RAIDBLACKTEMPLEHELPERS_H_
#define _PLAYERBOT_RAIDBLACKTEMPLEHELPERS_H_

#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace BlackTempleHelpers
{
    enum BlackTempleSpells
    {
        // High Warlord Naj'entus
        SPELL_IMPALING_SPINE            = 39837,
        SPELL_TIDAL_SHIELD              = 39872,

        // Supremus
        SPELL_SNARE_SELF                = 41922,

        // Shade of Akama
        // N/A

        // Teron Gorefiend
        SPELL_SHADOW_OF_DEATH = 40251, // 55s duration
        SPELL_SPIRITUAL_VENGEANCE = 40268,

        SPELL_SPIRIT_LANCE = 40157,
        SPELL_SPIRIT_CHAINS = 40175,
        SPELL_SPIRIT_VOLLEY = 40314,
        SPELL_SPIRIT_STRIKE = 40325,

        // Gurtogg Bloodboil

        // Reliquary of Souls

        // Mother Shahraz

        // Illidari Council

        // Illidan Stormrage <The Betrayer>

        // Hunter
        SPELL_MISDIRECTION             = 35079,

        // Priest
        // SPELL_FEAR_WARD                 =  6346,
    };

    enum BlackTempleNPCs
    {
        // High Warlord Naj'entus
        // N/A

        // Supremus
        NPC_SUPREMUS_VOLCANO = 23085,

        // Shade of Akama
        // N/A

        // Teron Gorefiend
        NPC_SHADOWY_CONSTRUCT = 23111,

        // Gurtogg Bloodboil

        // Reliquary of Souls

        // Mother Shahraz

        // Illidari Council

        // Illidan Stormrage <The Betrayer>
    };

    enum BlackTempleItems
    {
        // High Warlord Naj'entus
        ITEM_NAJENTUS_SPINE             = 32408,
    };

    enum BlackTempleObjects
    {
        // High Warlord Naj'entus
        GO_NAJENTUS_SPINE = 185584,
    };

    // General
    const uint32 BLACK_TEMPLE_MAP_ID = 564;
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot);

    // High Warlord Naj'entus
    extern const Position NAJENTUS_TANK_POSITION;

    // Supremus
    extern const Position SUPREMUS_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> supremusRangedPositions;
    extern std::unordered_map<uint32, time_t> supremusPhaseTimer;
    bool HasSupremusVolcanoNearby(PlayerbotAI* botAI, Player* bot);

    // Shade of Akama
    // N/A

    // Teron Gorefiend
    extern const Position GOREFIEND_TANK_POSITION;
    extern const Position GOREFIEND_DIE_POSITION;
    extern std::unordered_map<ObjectGuid, Position> gorefiendRangedPositions;

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}

#endif
