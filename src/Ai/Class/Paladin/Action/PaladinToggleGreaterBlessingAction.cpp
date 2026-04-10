/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinToggleGreaterBlessingAction.h"

#include "AiFactory.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

ToggleGreaterBlessingStrategyAction::ToggleGreaterBlessingStrategyAction(
    PlayerbotAI* botAI)
    : Action(botAI, "toggle greater blessing strategy") {}

bool ToggleGreaterBlessingStrategyAction::IsEligibleGroup(Group const* group) const
{
    if (!group)
        return false;

    switch (sPlayerbotAIConfig.autoGreaterBlessings)
    {
        case AutoGreaterBlessingMode::RAID_ONLY:
            return group->isRaidGroup();
        case AutoGreaterBlessingMode::GROUP_OR_RAID:
            return true;
        case AutoGreaterBlessingMode::DISABLED:
        default:
            return false;
    }
}

std::string ToggleGreaterBlessingStrategyAction::GetRestoreStrategy() const
{
    switch (AiFactory::GetPlayerSpecTab(bot))
    {
        case PALADIN_TAB_HOLY:
            return "+bwisdom";
        case PALADIN_TAB_PROTECTION:
            return "+bkings";
        case PALADIN_TAB_RETRIBUTION:
        default:
            return "+bmight";
    }
}

char const* ToggleGreaterBlessingStrategyAction::GetScopeDescription() const
{
    switch (sPlayerbotAIConfig.autoGreaterBlessings)
    {
        case AutoGreaterBlessingMode::RAID_ONLY:
            return "raid";
        case AutoGreaterBlessingMode::GROUP_OR_RAID:
            return "group/raid";
        case AutoGreaterBlessingMode::DISABLED:
        default:
            return "group";
    }
}

bool ToggleGreaterBlessingStrategyAction::Execute(Event /*event*/)
{
    // If the config option is disabled, never auto-toggle.
    if (sPlayerbotAIConfig.autoGreaterBlessings == AutoGreaterBlessingMode::DISABLED)
        return false;

    bool hasGblessing =
        botAI->HasStrategy("gblessing", BOT_STATE_NON_COMBAT);

    Group* group = bot->GetGroup();
    // Remove gblessing immediately when the bot is no longer in the configured scope.
    if (!IsEligibleGroup(group))
    {
        if (wasEligibleGroup_)
            userDisabled_ = false;

        wasEligibleGroup_ = false;

        if (hasGblessing)
        {
            // Remove gblessing and restore an appropriate single-blessing
            // strategy based on this Paladin's spec.
            botAI->ChangeStrategy("-gblessing," + GetRestoreStrategy(),
                                  BOT_STATE_NON_COMBAT);

            LOG_DEBUG("playerbots",
                      "[gblessing] {} no longer in {} - restored single-blessing "
                      "strategy",
                      bot->GetName(),
                      GetScopeDescription());

            return true;
        }

        return false;
    }

    // ── Entering or staying in the configured scope ──────────────
    wasEligibleGroup_ = true;

    // If user manually removed gblessing, don't re-enable.
    if (userDisabled_)
        return false;

    if (!hasGblessing)
    {
        // Activate gblessing and remove existing blessing strategies.
        botAI->ChangeStrategy(
            "+gblessing,-bwisdom,-bkings,-bmight,-bsanc",
            BOT_STATE_NON_COMBAT);

        LOG_DEBUG("playerbots",
                  "[gblessing] {} entered {} - activated gblessing",
                  bot->GetName(),
                  GetScopeDescription());
        return true;
    }

    return false;
}
