/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinGreaterBlessingAction.h"

#include "AiFactory.h"
#include "Chat.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using namespace ai::gbless;

// ── helpers (file-local) ─────────────────────────────────────────

// Compute a talent score for deterministic Paladin ordering.
// Higher score = higher priority slot.
static int TalentScore(Player* p)
{
    int score = 0;
    if (HasImprovedMight(p))  score += 2;
    if (HasImprovedWisdom(p)) score += 1;
    return score;
}

// Check if the bot already has a blessing of the given base category
// active on the target (cast by THIS bot specifically).
static bool HasMyBlessingOfCategory(PlayerbotAI* botAI, Unit* target,
                                    BaseBlessingCategory cat)
{
    auto check = [&](char const* single, char const* greater) -> bool
    {
        return botAI->HasAura(single, target, false, true) ||
               botAI->HasAura(greater, target, false, true);
    };

    switch (cat)
    {
        case BASE_MIGHT:     return check("blessing of might",
                                          "greater blessing of might");
        case BASE_WISDOM:    return check("blessing of wisdom",
                                          "greater blessing of wisdom");
        case BASE_KINGS:     return check("blessing of kings",
                                          "greater blessing of kings");
        case BASE_SANCTUARY: return check("blessing of sanctuary",
                                          "greater blessing of sanctuary");
        default:             return false;
    }
}

// ── CastGreaterBlessingAssignmentAction ──────────────────────────

CastGreaterBlessingAssignmentAction::CastGreaterBlessingAssignmentAction(
    PlayerbotAI* botAI)
    : Action(botAI, "cast greater blessing assignment") {}

bool CastGreaterBlessingAssignmentAction::isUseful()
{
    Group* g = bot->GetGroup();
    return g && g->isRaidGroup();
}

bool CastGreaterBlessingAssignmentAction::Execute(Event /*event*/)
{
    std::vector<PlayerAssignment> assignments;
    if (!ComputeAssignments(assignments))
        return false;

    // Find the first raid member that needs a blessing from this bot.
    for (auto const& a : assignments)
    {
        if (a.blessing == BLESSING_NONE || !a.player)
            continue;

        BaseBlessingCategory cat = BaseBlessingOf(a.blessing);
        if (HasMyBlessingOfCategory(botAI, a.player, cat))
            continue; // already buffed by me

        // Determine spell to cast.
        BlessingType castType = a.blessing;
        std::string spellName = BlessingSpellName(castType);
        if (spellName.empty())
            continue;

        // For Greater blessings, verify reagents. Fall back to Single
        // if missing.
        if (IsGreaterVariant(castType))
        {
            uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
            if (!spellId || !ai::buff::HasRequiredReagents(bot, spellId))
            {
                castType = ToSingleVariant(castType);
                spellName = BlessingSpellName(castType);
                if (spellName.empty())
                    continue;

                // Announce reagent shortage.
                if (Group* g = bot->GetGroup())
                {
                    std::string msg = "Missing reagents for " +
                        BlessingSpellName(a.blessing) + ". Using " +
                        spellName + ".";
                    WorldPacket data;
                    ChatMsg type = g->isRaidGroup()
                        ? CHAT_MSG_RAID : CHAT_MSG_PARTY;
                    ChatHandler::BuildChatPacket(
                        data, type, LANG_UNIVERSAL, bot,
                        nullptr, msg.c_str());
                    g->BroadcastPacket(
                        &data, true, -1, bot->GetGUID());
                }
            }
        }

        // Verify bot knows the spell.
        uint32 finalId = AI_VALUE2(uint32, "spell id", spellName);
        if (!finalId)
            continue;

        LOG_DEBUG("playerbots",
                  "[gblessing] {} casting {} on {}",
                  bot->GetName(), spellName, a.player->GetName());

        return botAI->CastSpell(spellName, a.player);
    }

    return false; // everyone is buffed
}

// ── Assignment algorithm ─────────────────────────────────────────

bool CastGreaterBlessingAssignmentAction::ComputeAssignments(
    std::vector<PlayerAssignment>& outAssignments)
{
    Group* g = bot->GetGroup();
    if (!g || !g->isRaidGroup())
        return false;

    // ── Phase 1: gather raid state ───────────────────────────────
    std::vector<Player*> botPaladins;
    struct RaidMember
    {
        Player* player;
        SpecProfile spec;
    };
    std::vector<RaidMember> raidMembers;

    for (GroupReference* ref = g->GetFirstMember(); ref; ref = ref->next())
    {
        Player* p = ref->GetSource();
        if (!p || !p->IsInWorld() || !p->IsAlive())
            continue;

        raidMembers.push_back({p, ResolveSpecProfile(p)});

        if (p->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(p))
            botPaladins.push_back(p);
    }

    if (botPaladins.empty())
        return false;

    // ── Phase 2: determine tier ──────────────────────────────────
    uint8 tierIndex = static_cast<uint8>(
        std::min<size_t>(botPaladins.size(), 4u) - 1u);

    // ── Phase 3: check Sanctuary availability ────────────────────
    bool anySanctuaryAvailable = false;
    for (Player* pal : botPaladins)
    {
        if (KnowsSanctuary(pal))
        {
            anySanctuaryAvailable = true;
            break;
        }
    }

    // ── Phase 4: sort Paladins deterministically ─────────────────
    std::sort(botPaladins.begin(), botPaladins.end(),
              [](Player* a, Player* b)
              {
                  int sa = TalentScore(a);
                  int sb = TalentScore(b);
                  if (sa != sb) return sa > sb; // higher score first
                  return a->GetGUID() < b->GetGUID(); // stable tiebreaker
              });

    // Find this bot's slot index.
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
        return false; // shouldn't happen

    // ── Phase 5: build effective priority lists ──────────────────
    // For each raid member, copy their priority entry and apply the
    // Sanctuary fallback if needed.
    struct EffectivePriority
    {
        Player* player;
        SpecProfile spec;
        BlessingType blessings[4];
    };
    std::vector<EffectivePriority> effective;
    effective.reserve(raidMembers.size());

    for (auto const& rm : raidMembers)
    {
        EffectivePriority ep;
        ep.player = rm.player;
        ep.spec   = rm.spec;

        auto const& entry = BLESSING_PRIORITIES[rm.spec][tierIndex];
        for (int i = 0; i < 4; ++i)
            ep.blessings[i] = entry.blessings[i];

        // Apply Sanctuary fallback: replace with Kings if no Paladin
        // knows Sanctuary.
        if (!anySanctuaryAvailable)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (BaseBlessingOf(ep.blessings[i]) != BASE_SANCTUARY)
                    continue;

                // Check if Kings already exists in this player's list.
                bool kingsExists = false;
                for (int j = 0; j < 4; ++j)
                {
                    if (j == i) continue;
                    if (BaseBlessingOf(ep.blessings[j]) == BASE_KINGS)
                    {
                        kingsExists = true;
                        break;
                    }
                }

                if (kingsExists)
                    ep.blessings[i] = BLESSING_NONE; // skip duplicate
                else
                {
                    // Replace with same variant (single/greater) of Kings.
                    ep.blessings[i] = IsSingleVariant(ep.blessings[i])
                                          ? BLESSING_KINGS_SINGLE
                                          : BLESSING_KINGS_GREATER;
                }
            }
        }

        effective.push_back(ep);
    }

    // ── Phase 6: resolve class-level assignments ─────────────────
    // Group raid members by WoW class. For each class and each slot,
    // determine if Greater or Single should be used.
    //
    // classSlotBlessing[classId][slotIndex] holds the resolved blessing
    // when all specs agree ("homogeneous"). When specs disagree
    // ("heterogeneous"), we fall back to per-player singles.

    constexpr int MAX_SLOTS = 4;
    // CLASS_DRUID = 11 (ID 10 is unused), so arrays indexed by
    // class ID need at least 12 elements.
    constexpr int MAX_CLASS_ID = 12;

    struct SlotInfo
    {
        bool heterogeneous = false;
        BlessingType homogeneous = BLESSING_NONE;
    };
    SlotInfo classSlots[MAX_CLASS_ID][MAX_SLOTS];

    // First pass: collect what each slot needs per class.
    // Track whether all specs of a class agree on the same base
    // blessing for each slot.
    bool classPresent[MAX_CLASS_ID] = {};

    for (auto const& ep : effective)
    {
        uint8 cls = ep.player->getClass();
        if (cls >= MAX_CLASS_ID) continue;
        classPresent[cls] = true;

        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            BlessingType bt = ep.blessings[slot];
            SlotInfo& si = classSlots[cls][slot];

            if (si.homogeneous == BLESSING_NONE && !si.heterogeneous)
            {
                // First player of this class — set initial value.
                si.homogeneous = bt;
            }
            else if (!si.heterogeneous)
            {
                // Subsequent player — check agreement.
                if (BaseBlessingOf(si.homogeneous) != BaseBlessingOf(bt))
                {
                    si.heterogeneous = true;
                    si.homogeneous = BLESSING_NONE;
                }
                else if (IsSingleVariant(si.homogeneous) ||
                         IsSingleVariant(bt))
                {
                    // If either says Single, force Single for this slot.
                    si.homogeneous = ToSingleVariant(si.homogeneous);
                }
                // else both Greater — keep Greater.
            }
        }
    }

    // Force Paladin class to always use Singles.
    if (classPresent[CLASS_PALADIN])
    {
        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            SlotInfo& si = classSlots[CLASS_PALADIN][slot];
            if (!si.heterogeneous && si.homogeneous != BLESSING_NONE)
                si.homogeneous = ToSingleVariant(si.homogeneous);
        }
    }

    // ── Phase 7: talent-aware slot swapping ──────────────────────
    // For each class, check if swapping two Paladins' slot assignments
    // would better match Improved Might/Wisdom talents.
    // We represent per-class slot overrides: classSlotPaladin[cls][slot]
    // gives the index into botPaladins that should handle that slot.
    // Default: slot i handled by paladin i.

    int classSlotPaladin[MAX_CLASS_ID][MAX_SLOTS];
    for (int c = 0; c < MAX_CLASS_ID; ++c)
        for (int s = 0; s < MAX_SLOTS; ++s)
            classSlotPaladin[c][s] = s;

    int numPals = static_cast<int>(botPaladins.size());

    for (int c = 0; c < MAX_CLASS_ID; ++c)
    {
        if (!classPresent[c]) continue;

        // For each pair of slots, check if swapping improves talent match.
        for (int s1 = 0; s1 < numPals && s1 < MAX_SLOTS; ++s1)
        {
            for (int s2 = s1 + 1; s2 < numPals && s2 < MAX_SLOTS; ++s2)
            {
                int p1 = classSlotPaladin[c][s1];
                int p2 = classSlotPaladin[c][s2];

                // Determine what blessing category each slot provides.
                BaseBlessingCategory cat1 = classSlots[c][s1].heterogeneous
                    ? BASE_NONE
                    : BaseBlessingOf(classSlots[c][s1].homogeneous);
                BaseBlessingCategory cat2 = classSlots[c][s2].heterogeneous
                    ? BASE_NONE
                    : BaseBlessingOf(classSlots[c][s2].homogeneous);

                // For heterogeneous slots, find the most common base
                // blessing among members of this class.
                if (classSlots[c][s1].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& ep : effective)
                    {
                        if (ep.player->getClass() != c) continue;
                        BaseBlessingCategory bc =
                            BaseBlessingOf(ep.blessings[s1]);
                        if (bc < 5) counts[bc]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                        if (counts[i] > counts[best]) best = i;
                    cat1 = static_cast<BaseBlessingCategory>(best);
                }
                if (classSlots[c][s2].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& ep : effective)
                    {
                        if (ep.player->getClass() != c) continue;
                        BaseBlessingCategory bc =
                            BaseBlessingOf(ep.blessings[s2]);
                        if (bc < 5) counts[bc]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                        if (counts[i] > counts[best]) best = i;
                    cat2 = static_cast<BaseBlessingCategory>(best);
                }

                // Score current assignment.
                // Sanctuary uses score 2 because it is a hard
                // requirement (only Prot can cast it at all), not a
                // soft bonus like Improved Might/Wisdom.
                auto talentMatchScore = [&](int palIdx,
                                            BaseBlessingCategory cat) -> int
                {
                    if (palIdx >= numPals) return 0;
                    Player* pal = botPaladins[palIdx];
                    if (cat == BASE_SANCTUARY && KnowsSanctuary(pal))
                        return 2;
                    if (cat == BASE_MIGHT && HasImprovedMight(pal))
                        return 1;
                    if (cat == BASE_WISDOM && HasImprovedWisdom(pal))
                        return 1;
                    return 0;
                };

                int currentScore = talentMatchScore(p1, cat1) +
                                   talentMatchScore(p2, cat2);
                int swappedScore = talentMatchScore(p1, cat2) +
                                   talentMatchScore(p2, cat1);

                if (swappedScore > currentScore)
                {
                    // Swap the paladin assignment for this class.
                    classSlotPaladin[c][s1] = p2;
                    classSlotPaladin[c][s2] = p1;
                }
            }
        }
    }

    // ── Phase 8: build per-player assignments for THIS bot ───────
    outAssignments.clear();
    outAssignments.reserve(effective.size());

    for (auto const& ep : effective)
    {
        PlayerAssignment pa;
        pa.player = ep.player;
        pa.blessing = BLESSING_NONE;

        uint8 cls = ep.player->getClass();
        if (cls >= MAX_CLASS_ID)
        {
            outAssignments.push_back(pa);
            continue;
        }

        // Find which slot this bot handles for this class.
        int myClassSlot = -1;
        for (int s = 0; s < numPals && s < MAX_SLOTS; ++s)
        {
            if (classSlotPaladin[cls][s] == mySlot)
            {
                myClassSlot = s;
                break;
            }
        }

        if (myClassSlot < 0 || myClassSlot >= MAX_SLOTS)
        {
            outAssignments.push_back(pa);
            continue;
        }

        SlotInfo const& si = classSlots[cls][myClassSlot];

        if (si.heterogeneous)
        {
            // Use per-player blessing from this player's effective list.
            BlessingType bt = ep.blessings[myClassSlot];
            // Force to single since specs disagree.
            pa.blessing = IsSingleVariant(bt) ? bt : ToSingleVariant(bt);
        }
        else
        {
            pa.blessing = si.homogeneous;
        }

        // Verify that the assigned Paladin can actually cast Sanctuary.
        if (BaseBlessingOf(pa.blessing) == BASE_SANCTUARY)
        {
            if (!KnowsSanctuary(bot))
            {
                // This bot can't cast Sanctuary — skip this assignment.
                // Another Paladin should handle it via slot swapping.
                pa.blessing = BLESSING_NONE;
            }
        }

        outAssignments.push_back(pa);
    }

    return !outAssignments.empty();
}
