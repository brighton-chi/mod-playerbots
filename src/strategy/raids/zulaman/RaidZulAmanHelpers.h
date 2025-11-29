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
        SPELL_BEARFORM          = 42377,

        // Jan'alai <Dragonhawk Avatar>

        // Halazzi <Lynx Avatar>

        // Hex Lord Malacrass
        SPELL_LIFEBLOOM = 43421,
        SPELL_AVENGING_WRATH = 43430,
        SPELL_BLIND = 43433,
        SPELL_MIND_CONTROL = 43550,

        // Zul'jin
    };

    enum ZulAmanNPCs
    {
        // Jan'alai <Dragonhawk Avatar>
        NPC_AMANI_HATCHER           = 23818,
        NPC_FIRE_BOMB               = 23920,

        // Halazzi <Lynx Avatar>
        // NPC_SPIRIT_OF_THE_LYNX      = 24143,
        NPC_CORRUPTED_LIGHTNING_TOTEM = 24224,
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
