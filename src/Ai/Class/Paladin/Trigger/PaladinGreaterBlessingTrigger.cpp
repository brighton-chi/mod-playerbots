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
    if (!bot) return false;

    Group* g = bot->GetGroup();
    if (!g || !g->isRaidGroup())
        return false;

    // Count bot Paladins.
    std::vector<Player*> botPaladins;
    for (GroupReference* ref = g->GetFirstMember(); ref; ref = ref->next())
    {
        Player* p = ref->GetSource();
        if (!p || !p->IsInWorld() || !p->IsAlive()) continue;
        if (p->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(p))
            botPaladins.push_back(p);
    }
    if (botPaladins.empty())
        return false;

    uint8 tierIndex = static_cast<uint8>(
        std::min<size_t>(botPaladins.size(), 4u) - 1u);

    // Check Sanctuary availability.
    bool anySanctuaryAvailable = false;
    for (Player* pal : botPaladins)
    {
        if (KnowsSanctuary(pal))
        {
            anySanctuaryAvailable = true;
            break;
        }
    }

    // Sort Paladins deterministically.
    std::sort(botPaladins.begin(), botPaladins.end(),
              [](Player* a, Player* b)
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

    // Quick scan: for each raid member, check if they need a blessing
    // from this bot's assigned slot.
    auto needsBlessing = [&](BaseBlessingCategory cat, Unit* target) -> bool
    {
        auto check = [&](char const* single, char const* greater) -> bool
        {
            return botAI->HasAura(single, target, false, true) ||
                   botAI->HasAura(greater, target, false, true);
        };
        switch (cat)
        {
            case BASE_MIGHT:     return !check("blessing of might",
                                               "greater blessing of might");
            case BASE_WISDOM:    return !check("blessing of wisdom",
                                               "greater blessing of wisdom");
            case BASE_KINGS:     return !check("blessing of kings",
                                               "greater blessing of kings");
            case BASE_SANCTUARY: return !check("blessing of sanctuary",
                                               "greater blessing of sanctuary");
            default:             return false;
        }
    };

    int numPals = static_cast<int>(botPaladins.size());

    for (GroupReference* ref = g->GetFirstMember(); ref; ref = ref->next())
    {
        Player* p = ref->GetSource();
        if (!p || !p->IsInWorld() || !p->IsAlive())
            continue;

        SpecProfile spec = ResolveSpecProfile(p);
        auto const& entry = BLESSING_PRIORITIES[spec][tierIndex];

        // Get this player's blessing at mySlot.
        if (mySlot >= 4)
            continue;

        BlessingType bt = entry.blessings[mySlot];
        if (bt == BLESSING_NONE)
            continue;

        // Apply Sanctuary fallback.
        if (!anySanctuaryAvailable &&
            BaseBlessingOf(bt) == BASE_SANCTUARY)
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
                continue; // would be NONE after fallback
            bt = IsSingleVariant(bt) ? BLESSING_KINGS_SINGLE
                                     : BLESSING_KINGS_GREATER;
        }

        // Verify this bot can cast it.
        if (BaseBlessingOf(bt) == BASE_SANCTUARY &&
            !KnowsSanctuary(bot))
            continue;

        BaseBlessingCategory cat = BaseBlessingOf(bt);
        if (cat == BASE_NONE)
            continue;

        if (needsBlessing(cat, p))
            return true;
    }

    // Phase 7 in the action may reassign this bot to Sanctuary via
    // talent-aware swapping, but the simplified check above only
    // looks at the bot's default sorted slot.  If this bot knows
    // Sanctuary, fire the trigger whenever any raid member still
    // lacks a Sanctuary aura — the action will handle correctness.
    if (KnowsSanctuary(bot))
    {
        for (GroupReference* ref = g->GetFirstMember(); ref;
             ref = ref->next())
        {
            Player* p = ref->GetSource();
            if (!p || !p->IsInWorld() || !p->IsAlive())
                continue;
            if (!botAI->HasAura("blessing of sanctuary", p) &&
                !botAI->HasAura(
                    "greater blessing of sanctuary", p))
                return true;
        }
    }

    return false;
}
