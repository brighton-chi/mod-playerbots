#ifndef _PLAYERBOT_RAIDZULAMANHELPERS_H_
#define _PLAYERBOT_RAIDZULAMANHELPERS_H_

#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace ZulAmanHelpers
{
    enum ZulAmanSpells
    {
        // Akil'zon <Eagle Avatar>
        SPELL_ELECTRICAL_STORM         = 43648,

        // Nalorakk <Bear Avatar>
        SPELL_BEARFORM                 = 42377,

        // Hex Lord Malacrass
        SPELL_LIFEBLOOM                = 43421,
        SPELL_AVENGING_WRATH           = 43430,
        SPELL_BLIND                    = 43433,
        SPELL_UNSTABLE_AFFLICTION      = 43522,
        SPELL_MIND_CONTROL             = 43550,

        // Zul'jin
        SPELL_WHIRLWIND                = 17207,
        SPELL_SHAPE_OF_THE_BEAR        = 42594,
        SPELL_SHAPE_OF_THE_EAGLE       = 42606,
        SPELL_SHAPE_OF_THE_LYNX        = 42607,
        SPELL_SHAPE_OF_THE_DRAGONHAWK  = 42608,
        SPELL_CLAW_RAGE                = 43149,

        // Hunter
        SPELL_MISDIRECTION             = 35079,
    };

    enum ZulAmanNPCs
    {
        // Trash
        NPC_AMANI_HEALING_WARD         = 23757,
        NPC_AMANI_PROTECTIVE_WARD      = 23822,

        // Jan'alai <Dragonhawk Avatar>
        NPC_AMANI_DRAGONHAWK_HATCHLING = 23598,
        NPC_EGG                        = 23817,
        NPC_AMANI_HATCHER              = 23818,
        NPC_FIRE_BOMB                  = 23920,

        // Halazzi <Lynx Avatar>
        NPC_CORRUPTED_LIGHTNING_TOTEM  = 24224,

        // Hex Lord Malacrass
        NPC_HEX_LORD_MALACRASS         = 24239,
        NPC_ALYSON_ANTILLE             = 24240,
        NPC_THURG                      = 24241,
        NPC_SLITHER                    = 24242,
        NPC_LORD_RAADAN                = 24243,
        NPC_GAZAKROTH                  = 24244,
        NPC_FENSTALKER                 = 24245,
        NPC_DARKHEART                  = 24246,
        NPC_KORAGG                     = 24247,

        // Zul'jin
        NPC_FEATHER_VORTEX             = 24136,
    };

    // General
    constexpr uint32 ZULAMAN_MAP_ID = 568;
    Unit* GetFirstAliveUnitByEntries(PlayerbotAI* botAI, const std::vector<uint32>& entries);
    bool AnyNearbyNpcWithEntry(PlayerbotAI* botAI, uint32 entry);

    // Akil'zon <Eagle Avatar>
    extern const Position AKILZON_TANK_POSITION;
    Player* GetElectricalStormTarget(Player* bot);

    // Nalorakk <Bear Avatar>
    extern const Position NALORAKK_TANK_POSITION;

    // Jan'alai <Dragonhawk Avatar>
    extern const Position JANALAI_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> janalaiRangedPositions;
    std::pair<Unit*, Unit*> GetAmaniHatcherPair(PlayerbotAI* botAI);

    // Halazzi <Lynx Avatar>
    extern const Position HALAZZI_TANK_POSITION;

    // Zul'jin
    extern const Position ZULJIN_TANK_POSITION;
}

#endif
