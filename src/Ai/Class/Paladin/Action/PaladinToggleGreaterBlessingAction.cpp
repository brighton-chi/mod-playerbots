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

bool ToggleGreaterBlessingStrategyAction::Execute(Event /*event*/)
{
    // If the config option is disabled, never auto-toggle.
    if (!sPlayerbotAIConfig.autoGreaterBlessings)
        return false;

    Group* g = bot->GetGroup();
    bool inRaid = g && g->isRaidGroup();
    bool hasGblessing =
        botAI->HasStrategy("gblessing", BOT_STATE_NON_COMBAT);

    // ── Leaving raid: deactivate gblessing ───────────────────────
    if (!inRaid && wasInRaid_)
    {
        wasInRaid_ = false;
        userDisabled_ = false; // reset user-disabled flag on raid exit

        if (hasGblessing)
        {
            // Remove gblessing and restore an appropriate single-blessing
            // strategy based on this Paladin's spec.
            int tab = AiFactory::GetPlayerSpecTab(bot);
            std::string restore;
            switch (tab)
            {
                case PALADIN_TAB_HOLY:
                    restore = "+bmana";
                    break;
                case PALADIN_TAB_PROTECTION:
                    restore = "+bstats";
                    break;
                case PALADIN_TAB_RETRIBUTION:
                default:
                    restore = "+bdps";
                    break;
            }

            botAI->ChangeStrategy("-gblessing," + restore,
                                  BOT_STATE_NON_COMBAT);

            LOG_DEBUG("playerbots",
                      "[gblessing] {} left raid - restored single-blessing "
                      "strategy",
                      bot->GetName());
        }

        return true;
    }

    // ── Entering or staying in raid ──────────────────────────────
    if (inRaid)
    {
        wasInRaid_ = true;

        // If user manually removed gblessing, don't re-enable.
        if (userDisabled_)
            return false;

        if (!hasGblessing)
        {
            // Activate gblessing and remove existing blessing strategies.
            botAI->ChangeStrategy(
                "+gblessing,-bmana,-bstats,-bdps,-bhealth",
                BOT_STATE_NON_COMBAT);

            LOG_DEBUG("playerbots",
                      "[gblessing] {} entered raid - activated gblessing",
                      bot->GetName());
            return true;
        }
    }

    return false;
}
