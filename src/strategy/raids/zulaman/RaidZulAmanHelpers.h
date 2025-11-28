#ifndef _PLAYERBOT_RAIDZULAMANHELPERS_H_
#define _PLAYERBOT_RAIDZULAMANHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace ZulAmanHelpers
{
    enum ZulAmanSpells
    {
        // Akil'zon <Eagle Avatar>
        SPELL_ELECTRICAL_STORM     = 43648,

        // Nalorakk <Bear Avatar>

        // Jan'alai <Dragonhawk Avatar>

        // Halazzi <Lynx Avatar>

        // Hex Lord Malacrass

        // Zul'jin
    };

    namespace ZulAmanPositions
    {

    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
}

#endif
