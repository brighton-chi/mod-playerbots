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
    PlayerbotAI* botAI) : Action(botAI, "toggle greater blessing strategy") {}

bool ToggleGreaterBlessingStrategyAction::IsEligibleGroup(Group const* group) const
{
    if (!group)
        return false;

    switch (sPlayerbotAIConfig.autoGreaterBlessings)
    {
        case AutoPartyBuffMode::RAID_ONLY:
            return group->isRaidGroup();
        case AutoPartyBuffMode::GROUP_OR_RAID:
            return true;
        case AutoPartyBuffMode::DISABLED:
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
            return "+bsanc";
        case PALADIN_TAB_RETRIBUTION:
        default:
            return "+bmight";
    }
}

bool ToggleGreaterBlessingStrategyAction::Execute(Event /*event*/)
{
    if (sPlayerbotAIConfig.autoGreaterBlessings == AutoPartyBuffMode::DISABLED)
        return false;

    bool hasGblessing =
        botAI->HasStrategy("gblessing", BOT_STATE_NON_COMBAT);

    Group* group = bot->GetGroup();
    if (!IsEligibleGroup(group))
    {
        if (hasGblessing)
        {
            botAI->ChangeStrategy(
                "-gblessing," + GetRestoreStrategy(), BOT_STATE_NON_COMBAT);

            return true;
        }

        return false;
    }

    if (!hasGblessing)
    {
        botAI->ChangeStrategy(
            "+gblessing,-bwisdom,-bkings,-bmight,-bsanc", BOT_STATE_NON_COMBAT);

        return true;
    }

    return false;
}
