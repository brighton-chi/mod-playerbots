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

        // Supremus

        // Shade of Akama

        // Teron Gorefiend

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

        // Supremus

        // Shade of Akama

        // Teron Gorefiend

        // Gurtogg Bloodboil

        // Reliquary of Souls

        // Mother Shahraz

        // Illidari Council

        // Illidan Stormrage <The Betrayer>
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
    extern Position NAJENTUS_TANK_POSITION;

    // Supremus

    // Shade of Akama

    // Teron Gorefiend

    // Gurtogg Bloodboil

    // Reliquary of Souls

    // Mother Shahraz

    // Illidari Council

    // Illidan Stormrage <The Betrayer>
}

#endif
