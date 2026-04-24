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

// Returns true when config says this buff family should prefer the group
// variant for the bot's current group type.
bool IsGroupVariantEnabled(Player* bot, std::string const& name);

// Build an aura qualifier "single + greater" to avoid double-buffing
std::string MakeAuraQualifierForBuff(std::string const& name);

// Returns the group spell name for a given single-target buff.
// Only covers non-Paladin buffs (Druid, Mage, Priest).
// Paladin blessings are handled by auto greater blessing assignment.
// Returns "" if no group equivalent exists.
std::string GroupVariantFor(std::string const& name);

// Checks if the bot has the required reagents to cast a spell (by its spellId).
// Returns false if the spellId is invalid.
bool HasRequiredReagents(Player* bot, uint32 spellId);

// Returns the throttle key for a group-variant spell cast. In raids this is
// subgroup-aware, otherwise it is the spell name itself. Returns an empty
// string for non-group-variant spells.
std::string GetGroupVariantThrottleKey(Player* bot, std::string const& spellName, Unit* target);

// Returns true if the group variant was cast by this bot recently enough to
// suppress immediate recasts of the same buff family.
bool IsGroupVariantRecentlyCast(
    Player* bot,
    PlayerbotAI* botAI,
    std::string const& baseName,
    Unit* target,
    uint32 throttleSeconds = 10);

// If the bot is in an eligible group and knows the group variant of baseName,
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
