/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "Common.h"

class Player;
class PlayerbotAI;

namespace ai::buff
{

// Build an aura qualifier "single + greater" to avoid double-buffing
std::string MakeAuraQualifierForBuff(std::string const& name);

// Returns the group spell name for a given single-target buff.
// Only covers non-Paladin buffs (Druid, Mage, Priest).
// Paladin blessings are handled by the gblessing strategy.
// Returns "" if no group equivalent exists.
std::string GroupVariantFor(std::string const& name);

// Checks if the bot has the required reagents to cast a spell (by its spellId).
// Returns false if the spellId is invalid.
bool HasRequiredReagents(Player* bot, uint32 spellId);

// If the bot is in a group and knows the group variant of baseName,
// returns the group spell name (provided reagents are available).
// Otherwise returns baseName unchanged.
std::string UpgradeToGroupIfAppropriate(
    Player* bot,
    PlayerbotAI* botAI,
    std::string const& baseName);

}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId);
}
