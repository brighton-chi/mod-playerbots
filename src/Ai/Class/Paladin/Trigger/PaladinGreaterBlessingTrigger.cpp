/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinGreaterBlessingTrigger.h"

#include "AiFactory.h"
#include "GenericBuffUtils.h"
#include "PaladinBlessingPriorityData.h"
#include "Playerbots.h"

using namespace ai::gbless;

// Lightweight check: does any raid member need a blessing from this bot?
// Uses the same algorithm as the action but exits early on first match.
bool GreaterBlessingNeededTrigger::IsActive()
{
    Player* bot = this->bot;
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    // Collect bot Paladins regardless of alive/dead so tier stays
    // stable when a paladin dies.
    std::vector<Player*> botPaladins;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->GetSource();
        if (!player || !player->IsInWorld())
            continue;

        if (player->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(player))
            botPaladins.push_back(player);
    }
    if (botPaladins.empty())
        return false;

    uint8 tierIndex = static_cast<uint8>(
        std::min<size_t>(botPaladins.size(), 4u) - 1u);

    // Check Sanctuary availability.
    bool anySanctuaryAvailable = false;
    for (Player* paladin : botPaladins)
    {
        if (KnowsSanctuary(paladin))
        {
            anySanctuaryAvailable = true;
            break;
        }
    }

    // Sort Paladins deterministically.
    std::sort(botPaladins.begin(), botPaladins.end(), [](Player* a, Player* b)
    {
        int sa = 0, sb = 0;
        if (HasImprovedMight(a))  sa += 2;
        if (HasImprovedWisdom(a)) sa += 1;
        if (HasImprovedMight(b))  sb += 2;
        if (HasImprovedWisdom(b)) sb += 1;
        if (sa != sb) return sa > sb;
        return a->GetGUID() < b->GetGUID();
    });

    int mySlot = -1;
    for (size_t i = 0; i < botPaladins.size(); ++i)
    {
        if (botPaladins[i]->GetGUID() == bot->GetGUID())
        {
            mySlot = static_cast<int>(i);
            break;
        }
    }
    if (mySlot < 0)
        return false;

    // Check whether this bot has the EXACT assigned blessing on the target.
    // Distinguishes single vs greater so stale singles trigger an upgrade.
    auto hasExact = [&](BlessingType bt, Unit* target) -> bool
    {
        std::string name = BlessingSpellName(bt);
        if (name.empty())
            return false;

        return botAI->HasAura(name.c_str(), target, false, true);
    };

    if (mySlot >= 4)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->GetSource();
        if (!player || !player->IsInWorld() || !player->IsAlive())
            continue;

        SpecProfile spec = ResolveSpecProfile(player);
        auto const& entry = BLESSING_PRIORITIES[spec][tierIndex];

        BlessingType type = entry.blessings[mySlot];
        if (type == BLESSING_NONE)
            continue;

        // Apply Sanctuary fallback.
        if (!anySanctuaryAvailable && BaseBlessingOf(type) == BASE_SANCTUARY)
        {
            bool kingsExists = false;
            for (int j = 0; j < 4; ++j)
            {
                if (j == mySlot) continue;
                if (BaseBlessingOf(entry.blessings[j]) == BASE_KINGS)
                {
                    kingsExists = true;
                    break;
                }
            }
            if (kingsExists)
                continue;

            type = IsSingleVariant(type) ? BLESSING_KINGS_SINGLE
                                         : BLESSING_KINGS_GREATER;
        }

        // Verify this bot can cast it.
        if (BaseBlessingOf(type) == BASE_SANCTUARY && !KnowsSanctuary(bot))
            continue;

        // For Paladin targets, the action forces Singles via Phase 6.
        // Check both variants so we don't fire needlessly when the
        // Single is already present but the raw table says Greater.
        if (player->getClass() == CLASS_PALADIN && IsGreaterVariant(type))
        {
            if (!hasExact(type, player) && !hasExact(ToSingleVariant(type), player))
                return true;
        }
        else if (!hasExact(type, player))
            return true;
    }

    // Phase 7 in the action may reassign this bot to Sanctuary via talent-aware swapping.
    // If this bot knows Sanctuary, fire the trigger whenever any alive raid member still
    // lacks a Sanctuary aura from this bot.
    if (KnowsSanctuary(bot))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref;
             ref = ref->next())
        {
            Player* player = ref->GetSource();
            if (!player || !player->IsInWorld() || !player->IsAlive())
                continue;

            if (!botAI->HasAura("blessing of sanctuary", player, false, true) &&
                !botAI->HasAura("greater blessing of sanctuary", player, false, true))
                return true;
        }
    }

    return false;
}
