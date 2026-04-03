/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "Common.h"

class Player;

namespace ai::buff
{

// Build an aura qualifier "single + greater" to avoid double-buffing
std::string MakeAuraQualifierForBuff(std::string const& name);

// Checks if the bot has the required reagents to cast a spell (by its spellId).
// Returns false if the spellId is invalid.
bool HasRequiredReagents(Player* bot, uint32 spellId);

}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId);
}
