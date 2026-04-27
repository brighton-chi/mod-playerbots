/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "Common.h"

class Player;
class PlayerbotAI;
class Unit;

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

// Returns true when this buff family is sensitive to login-time aura races
// and should briefly defer evaluation after bot or target login.
bool NeedsPostLoginBuffGrace(std::string const& name);

// Returns true when party buff evaluation should be deferred because the bot
// or player target logged in very recently.
bool ShouldDeferPartyBuffEvaluationForRecentLogin(
    Player* bot,
    Unit* target,
    std::string const& spell);

// Returns true when greater blessing evaluation should be deferred because the
// paladin or one of its current group members logged in very recently.
bool ShouldDeferGreaterBlessingEvaluationForRecentLogin(Player* bot);

// Checks if the bot has the required reagents to cast a spell (by its spellId).
// Returns false if the spellId is invalid.
bool HasRequiredReagents(Player* bot, uint32 spellId);

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
