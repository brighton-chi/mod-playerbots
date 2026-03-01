/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

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
        SPELL_INFERNO          = 31299,

        // Kaz'rogal
        SPELL_MARK_OF_KAZROGAL = 31447,

        // Azgalor
        SPELL_RAIN_OF_FIRE     = 31340,
        SPELL_DOOM             = 31347,

        // Archimonde
        SPELL_DOOMFIRE         = 31944,
        SPELL_AIR_BURST        = 32014,

        // Hunter
        SPELL_MISDIRECTION     = 35079,

        // Priest
        SPELL_FEAR_WARD        =  6346,
    };

    enum HyjalSummitNPCs
    {
        // Anetheron
        NPC_TOWERING_INFERNAL  = 17818,

        // Archimonde
        NPC_DOOMFIRE           = 18095,
    };

    // General
    constexpr uint32 HYJAL_SUMMIT_MAP_ID = 534;

    // Rage Winterchill
    extern const Position WINTERCHILL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, bool> hasReachedWinterchillPosition;

    // Anetheron
    extern const Position ANETHERON_MAIN_TANK_POSITION;
    extern const Position ANETHERON_E_INFERNAL_TANK_POSITION;
    extern const Position ANETHERON_W_INFERNAL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, bool> hasReachedAnetheronPosition;
    Player* GetInfernoTarget(Unit* anetheron);
    const Position& GetClosestInfernalTankPosition(Player* bot);

    // Kaz'rogal
    extern const Position KAZROGAL_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

    // Azgalor
    extern const Position AZGALOR_MAIN_TANK_TRANSITION_POSITION;
    extern const Position AZGALOR_MAIN_TANK_FINAL_POSITION;
    extern const Position AZGALOR_DOOMGUARD_TANK_POSITION;
    extern const Position AZGALOR_WAITING_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> azgalorTankStep;
    int GetAzgalorTankStep(PlayerbotAI* botAI, Player* bot);
    bool AnyGroupMemberHasDoom(Player* bot);
}

#endif
