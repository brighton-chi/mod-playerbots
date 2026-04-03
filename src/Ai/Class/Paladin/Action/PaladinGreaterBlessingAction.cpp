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
static int TalentScore(Player* player)
{
    int score = 0;
    if (HasImprovedMight(player))  score += 2;
    if (HasImprovedWisdom(player)) score += 1;
    return score;
}

// Check if the bot already has this EXACT blessing type active on the target (cast by THIS bot
// specifically). Checks the specific variant (single vs greater) so that stale single blessings
// from party context are upgraded to greater when the assignment changes.
static bool HasMyExactBlessing(PlayerbotAI* botAI, Unit* target, BlessingType type)
{
    std::string name = BlessingSpellName(type);
    if (name.empty())
        return false;

    return botAI->HasAura(name.c_str(), target, false, true);
}

// ── CastGreaterBlessingAssignmentAction ──────────────────────────

CastGreaterBlessingAssignmentAction::CastGreaterBlessingAssignmentAction(
    PlayerbotAI* botAI) : Action(botAI, "cast greater blessing assignment") {}

bool CastGreaterBlessingAssignmentAction::isUseful()
{
    Group* group = bot->GetGroup();
    return group && group->isRaidGroup();
}

bool CastGreaterBlessingAssignmentAction::Execute(Event /*event*/)
{
    std::vector<PlayerAssignment> assignments;
    if (!ComputeAssignments(assignments))
        return false;

    // Find the first raid member that needs a blessing from this bot.
    for (auto const& assigned : assignments)
    {
        if (assigned.blessing == BLESSING_NONE || !assigned.player)
            continue;

        // Check whether this bot already has the EXACT assigned blessing on the target.
        // This intentionally distinguishes single vs greater so that stale single blessings
        // (from party context or a previous tier) get upgraded.
        if (HasMyExactBlessing(botAI, assigned.player, assigned.blessing))
            continue;

        // Determine spell to cast.
        BlessingType castType = assigned.blessing;
        std::string spellName = BlessingSpellName(castType);
        if (spellName.empty())
            continue;

        // For Greater blessings, verify reagents. Fall back to Single if missing.
        if (IsGreaterVariant(castType))
        {
            uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
            if (!spellId || !ai::buff::HasRequiredReagents(bot, spellId))
            {
                castType = ToSingleVariant(castType);
                spellName = BlessingSpellName(castType);
                if (spellName.empty())
                    continue;

                // If the Single fallback is already present, skip to
                // avoid re-casting every tick while out of reagents.
                if (HasMyExactBlessing(botAI, assigned.player, castType))
                    continue;

                // Announce reagent shortage.
                if (Group* group = bot->GetGroup())
                {
                    std::string msg = "Missing reagents for " +
                        BlessingSpellName(assigned.blessing) + ". Using " +
                        spellName + ".";
                    WorldPacket data;
                    ChatMsg type = group->isRaidGroup() ? CHAT_MSG_RAID : CHAT_MSG_PARTY;
                    ChatHandler::BuildChatPacket(
                        data, type, LANG_UNIVERSAL, bot, nullptr, msg.c_str());
                    group->BroadcastPacket(
                        &data, true, -1, bot->GetGUID());
                }
            }
        }

        // Verify bot knows the spell.
        uint32 finalId = AI_VALUE2(uint32, "spell id", spellName);
        if (!finalId)
            continue;

        return botAI->CastSpell(spellName, assigned.player);
    }

    return false; // everyone is buffed
}

// ── Assignment algorithm ─────────────────────────────────────────

bool CastGreaterBlessingAssignmentAction::ComputeAssignments(
    std::vector<PlayerAssignment>& outAssignments)
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    // ── Phase 1: gather raid state ───────────────────────────────
    // Bot Paladins are collected regardless of alive/dead so that the tier and slot
    // assignments remain stable when a paladin dies. Only alive, non-paladin raid members
    // are collected as buff targets.
    std::vector<Player*> botPaladins;
    struct RaidMember
    {
        Player* player;
        SpecProfile spec;
    };
    std::vector<RaidMember> raidMembers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* player = ref->GetSource();
        if (!player || !player->IsInWorld())
            continue;

        if (player->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(player))
            botPaladins.push_back(player);

        if (player->IsAlive())
            raidMembers.push_back({player, ResolveSpecProfile(player)});
    }

    if (botPaladins.empty())
        return false;

    // ── Phase 2: determine tier ──────────────────────────────────
    uint8 tierIndex = static_cast<uint8>(
        std::min<size_t>(botPaladins.size(), 4u) - 1u);

    // ── Phase 3: check Sanctuary availability ────────────────────
    bool anySanctuaryAvailable = false;
    for (Player* paladin : botPaladins)
    {
        if (KnowsSanctuary(paladin))
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
    // For each raid member, copy their priority entry and apply Sanctuary fallback if needed.
    struct EffectivePriority
    {
        Player* player;
        SpecProfile spec;
        BlessingType blessings[4];
    };
    std::vector<EffectivePriority> effective;
    effective.reserve(raidMembers.size());

    for (auto const& member : raidMembers)
    {
        EffectivePriority priority;
        priority.player = member.player;
        priority.spec   = member.spec;

        auto const& entry = BLESSING_PRIORITIES[member.spec][tierIndex];
        for (int i = 0; i < 4; ++i)
            priority.blessings[i] = entry.blessings[i];

        // Apply Sanctuary fallback: replace with Kings if no Paladin knows Sanctuary.
        if (!anySanctuaryAvailable)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (BaseBlessingOf(priority.blessings[i]) != BASE_SANCTUARY)
                    continue;

                // Check if Kings already exists in this player's list.
                bool kingsExists = false;
                for (int j = 0; j < 4; ++j)
                {
                    if (j == i) continue;
                    if (BaseBlessingOf(priority.blessings[j]) == BASE_KINGS)
                    {
                        kingsExists = true;
                        break;
                    }
                }

                if (kingsExists)
                    priority.blessings[i] = BLESSING_NONE; // skip duplicate
                else
                {
                    // Replace with same variant (single/greater) of Kings.
                    priority.blessings[i] = IsSingleVariant(priority.blessings[i])
                                              ? BLESSING_KINGS_SINGLE
                                              : BLESSING_KINGS_GREATER;
                }
            }
        }

        effective.push_back(priority);
    }

    // ── Phase 6: resolve class-level assignments ─────────────────
    // Group raid members by WoW class. For each class and each slot, determine if
    // Greater or Single should be used.
    //
    // classSlotBlessing[classId][slotIndex] holds the resolved blessing when all specs
    // agree ("homogeneous"). When specs disagree ("heterogeneous"), we fall back to per-player singles.

    constexpr uint8 MAX_SLOTS = 4;
    // CLASS_DRUID = 11 (ID 10 is unused), so arrays indexed by
    // class ID need at least 12 elements.
    constexpr uint8 MAX_CLASS_ID = 12;

    struct SlotInfo
    {
        bool heterogeneous = false;
        BlessingType homogeneous = BLESSING_NONE;
    };
    SlotInfo classSlots[MAX_CLASS_ID][MAX_SLOTS];

    // First pass: collect what each slot needs per class. Track whether all specs of a class
    // agree on the same base blessing for each slot.
    bool classPresent[MAX_CLASS_ID] = {};

    for (auto const& priority : effective)
    {
        uint8 cls = priority.player->getClass();
        if (cls >= MAX_CLASS_ID) continue;
        classPresent[cls] = true;

        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            BlessingType type = priority.blessings[slot];
            SlotInfo& slotInfo = classSlots[cls][slot];

            if (slotInfo.homogeneous == BLESSING_NONE && !slotInfo.heterogeneous)
            {
                // First player of this class — set initial value.
                slotInfo.homogeneous = type;
            }
            else if (!slotInfo.heterogeneous)
            {
                // Subsequent player — check agreement.
                if (BaseBlessingOf(slotInfo.homogeneous) != BaseBlessingOf(type))
                {
                    slotInfo.heterogeneous = true;
                    slotInfo.homogeneous = BLESSING_NONE;
                }
                else if (IsSingleVariant(slotInfo.homogeneous) ||
                         IsSingleVariant(type))
                {
                    // If either says Single, force Single for this slot.
                    slotInfo.homogeneous = ToSingleVariant(slotInfo.homogeneous);
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
            SlotInfo& slotInfo = classSlots[CLASS_PALADIN][slot];
            if (!slotInfo.heterogeneous && slotInfo.homogeneous != BLESSING_NONE)
                slotInfo.homogeneous = ToSingleVariant(slotInfo.homogeneous);
        }
    }

    // ── Phase 7: talent-aware slot swapping ──────────────────────
    // For each class, check if swapping two Paladins' slot assignments would better match
    // Improved Might/Wisdom talents. We represent per-class slot overrides: classSlotPaladin[cls][slot]
    // gives the index into botPaladins that should handle that slot.
    // Default: slot i handled by paladin i.

    int classSlotPaladin[MAX_CLASS_ID][MAX_SLOTS];
    for (int classId = 0; classId < MAX_CLASS_ID; ++classId)
        for (int slot = 0; slot < MAX_SLOTS; ++slot)
            classSlotPaladin[classId][slot] = slot;

    int numPals = static_cast<int>(botPaladins.size());

    for (int classId = 0; classId < MAX_CLASS_ID; ++classId)
    {
        if (!classPresent[classId]) continue;

        // For each pair of slots, check if swapping improves talent match.
        for (int slot1 = 0; slot1 < numPals && slot1 < MAX_SLOTS; ++slot1)
        {
            for (int slot2 = slot1 + 1; slot2 < numPals && slot2 < MAX_SLOTS; ++slot2)
            {
                int p1 = classSlotPaladin[classId][slot1];
                int p2 = classSlotPaladin[classId][slot2];

                // Determine what blessing category each slot provides.
                BaseBlessingCategory category1 = classSlots[classId][slot1].heterogeneous
                    ? BASE_NONE
                    : BaseBlessingOf(classSlots[classId][slot1].homogeneous);
                BaseBlessingCategory category2 = classSlots[classId][slot2].heterogeneous
                    ? BASE_NONE
                    : BaseBlessingOf(classSlots[classId][slot2].homogeneous);

                // For heterogeneous slots, find the most common base
                // blessing among members of this class.
                if (classSlots[classId][slot1].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& priority : effective)
                    {
                        if (priority.player->getClass() != classId) continue;
                        BaseBlessingCategory category = BaseBlessingOf(priority.blessings[slot1]);
                        if (category < 5) counts[category]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                        if (counts[i] > counts[best]) best = i;
                    category1 = static_cast<BaseBlessingCategory>(best);
                }
                if (classSlots[classId][slot2].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& priority : effective)
                    {
                        if (priority.player->getClass() != classId) continue;
                        BaseBlessingCategory category = BaseBlessingOf(priority.blessings[slot2]);
                        if (category < 5) counts[category]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                        if (counts[i] > counts[best]) best = i;
                    category2 = static_cast<BaseBlessingCategory>(best);
                }

                // Score current assignment. Sanctuary uses score 2 because it is a hard
                // requirement (only Prot can cast it at all), not a soft bonus like Improved Might/Wisdom.
                auto talentMatchScore = [&](int palIdx, BaseBlessingCategory category) -> int
                {
                    if (palIdx >= numPals) return 0;
                    Player* paladin = botPaladins[palIdx];
                    if (category == BASE_SANCTUARY && KnowsSanctuary(paladin))
                        return 2;
                    if (category == BASE_MIGHT && HasImprovedMight(paladin))
                        return 1;
                    if (category == BASE_WISDOM && HasImprovedWisdom(paladin))
                        return 1;
                    return 0;
                };

                int currentScore = talentMatchScore(p1, category1) +
                                   talentMatchScore(p2, category2);
                int swappedScore = talentMatchScore(p1, category2) +
                                   talentMatchScore(p2, category1);

                if (swappedScore > currentScore)
                {
                    // Swap the paladin assignment for this class.
                    classSlotPaladin[classId][slot1] = p2;
                    classSlotPaladin[classId][slot2] = p1;
                }
            }
        }
    }

    // ── Phase 8: build per-player assignments for THIS bot ───────
    outAssignments.clear();
    outAssignments.reserve(effective.size());

    for (auto const& priority : effective)
    {
        PlayerAssignment assigned;
        assigned.player = priority.player;
        assigned.blessing = BLESSING_NONE;

        uint8 cls = priority.player->getClass();
        if (cls >= MAX_CLASS_ID)
        {
            outAssignments.push_back(assigned);
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
            outAssignments.push_back(assigned);
            continue;
        }

        SlotInfo const& slotInfo = classSlots[cls][myClassSlot];

        if (slotInfo.heterogeneous)
        {
            // Use per-player blessing from this player's effective list.
            BlessingType type = priority.blessings[myClassSlot];
            // Force to single since specs disagree.
            assigned.blessing = IsSingleVariant(type) ? type : ToSingleVariant(type);
        }
        else
        {
            assigned.blessing = slotInfo.homogeneous;
        }

        // Verify that the assigned Paladin can actually cast Sanctuary.
        if (BaseBlessingOf(assigned.blessing) == BASE_SANCTUARY)
        {
            if (!KnowsSanctuary(bot))
            {
                // This bot can't cast Sanctuary — skip this assignment.
                // Another Paladin should handle it via slot swapping.
                assigned.blessing = BLESSING_NONE;
            }
        }

        outAssignments.push_back(assigned);
    }

    return !outAssignments.empty();
}
