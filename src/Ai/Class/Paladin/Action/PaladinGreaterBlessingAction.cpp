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
#include "PlayerbotTextMgr.h"
#include "SharedDefines.h"

using namespace ai::gbless;

bool ai::gbless::IsEligibleGroupForAutoBlessings(Group const* group)
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

bool ai::gbless::IsAutoGreaterBlessingActive(Player const* bot)
{
    return bot && IsEligibleGroupForAutoBlessings(bot->GetGroup());
}

static int TalentScore(Player* player)
{
    int score = 0;
    if (HasImprovedMight(player))
        score += 2;
    if (HasImprovedWisdom(player))
        score += 1;

    return score;
}

static bool HasMyExactBlessing(PlayerbotAI* botAI, Unit* target, BlessingType type)
{
    std::string name = BlessingSpellName(type);
    if (name.empty())
        return false;

    return botAI->HasAura(name.c_str(), target, false, true);
}

CastGreaterBlessingAssignmentAction::CastGreaterBlessingAssignmentAction(
    PlayerbotAI* botAI) : Action(botAI, "cast greater blessing assignment") {}

bool CastGreaterBlessingAssignmentAction::isUseful()
{
    return IsAutoGreaterBlessingActive(bot);
}

bool CastGreaterBlessingAssignmentAction::HasPendingAssignment()
{
    PlayerAssignment assignment;
    BlessingType castType = BLESSING_NONE;
    std::string spellName;

    return FindPendingAssignment(assignment, castType, spellName);
}

bool CastGreaterBlessingAssignmentAction::Execute(Event /*event*/)
{
    PlayerAssignment assignment;
    BlessingType castType = BLESSING_NONE;
    std::string spellName;
    if (!FindPendingAssignment(assignment, castType, spellName))
        return false;

    uint32 finalId = AI_VALUE2(uint32, "spell id", spellName);
    if (!finalId)
        return false;

    return botAI->CastSpell(spellName, assignment.player);
}

bool CastGreaterBlessingAssignmentAction::FindPendingAssignment(
    PlayerAssignment& outAssignment, BlessingType& outCastType, std::string& outSpellName)
{
    std::vector<PlayerAssignment> assignments;
    if (!ComputeAssignments(assignments))
        return false;

    for (auto const& assigned : assignments)
    {
        if (assigned.blessing == BLESSING_NONE || !assigned.player)
            continue;

        if (HasMyExactBlessing(botAI, assigned.player, assigned.blessing))
            continue;

        BlessingType castType = assigned.blessing;
        std::string spellName = BlessingSpellName(castType);
        if (spellName.empty())
            continue;

        if (IsGreaterVariant(castType))
        {
            uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
            if (!spellId || !ai::buff::HasRequiredReagents(bot, spellId))
            {
                castType = ToSingleVariant(castType);
                spellName = BlessingSpellName(castType);
                if (spellName.empty())
                    continue;

                if (HasMyExactBlessing(botAI, assigned.player, castType))
                    continue;
            }
        }

        outAssignment = assigned;
        outCastType = castType;
        outSpellName = spellName;
        return true;
    }

    return false;
}

// CastGreaterBlessingAssignmentAction computes blessing assignments for the group
// composition and casts one buff per call when auto greater blessings are active.

bool CastGreaterBlessingAssignmentAction::ComputeAssignments(
    std::vector<PlayerAssignment>& outAssignments)
{
    Group* group = bot->GetGroup();
    if (!IsEligibleGroupForAutoBlessings(group))
        return false;

    // Step 1: Gather Raid Composition
    // Get all bot Paladins (dead or alive for stable assignment) for buffing
    // and all living raid members for buff targets.
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

    // Step 2: Determine number of Paladins
    // And get the corresponding index to determine which blessing priorities to use.
    uint8 paladinCountIndex = static_cast<uint8>(
        std::min<size_t>(botPaladins.size(), 4u) - 1u);

    // Step 3: Check if any Paladin can cast Sanctuary
    bool anySanctuaryAvailable = false;
    for (Player* paladin : botPaladins)
    {
        if (KnowsSanctuary(paladin))
        {
            anySanctuaryAvailable = true;
            break;
        }
    }

    // Step 4: Sort Paladins by Priority for Blessing Assignment
    // The purpose is so Might will be assigned to a Paladin with Improved Might,
    // Wisdom to Improved Wisdom, and Sanctuary to, well, a Paladin that actually has
    // the talent to cast it. Otherwise, lowest GUID is the tiebreaker for assignment.
    std::sort(botPaladins.begin(), botPaladins.end(),
              [](Player* a, Player* b)
              {
                  int sa = TalentScore(a);
                  int sb = TalentScore(b);
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

        auto const& entry = BLESSING_PRIORITIES[member.spec][paladinCountIndex];
        for (int i = 0; i < 4; ++i)
            priority.blessings[i] = entry.blessings[i];

        if (!anySanctuaryAvailable)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (BaseBlessingOf(priority.blessings[i]) != BASE_SANCTUARY)
                    continue;

                bool kingsExists = false;
                for (int j = 0; j < 4; ++j)
                {
                    if (j == i)
                        continue;

                    if (BaseBlessingOf(priority.blessings[j]) == BASE_KINGS)
                    {
                        kingsExists = true;
                        break;
                    }
                }

                if (kingsExists)
                {
                    priority.blessings[i] = BLESSING_NONE;
                }
                else
                {
                    priority.blessings[i] = IsSingleVariant(priority.blessings[i])
                                              ? BLESSING_KINGS_SINGLE
                                              : BLESSING_KINGS_GREATER;
                }
            }
        }

        effective.push_back(priority);
    }

    // Step 5: Group Raid Members By Class and Spec
    // For each class and each slot, determine if greater or single should be used for each blessing.
    // Determination is based on whether all specs of a class agree on the same blessing for a slot.
    // If all specs of a class agree on the same blessing for a slot, call the class "homogeneous"
    // and assign a greater blessing. If not, call it "heterogeneous" and assign one or more single
    // blessings to ensure all specs are covered.

    constexpr uint8 MAX_SLOTS = 4;
    // Weirdly, CLASS_DRUID = 11 (ID 10 is unused) so arrays indexed by class ID need 12 elements.
    constexpr uint8 MAX_CLASS_ID = 12;

    struct SlotInfo
    {
        bool heterogeneous = false;
        BlessingType homogeneous = BLESSING_NONE;
    };
    SlotInfo classSlots[MAX_CLASS_ID][MAX_SLOTS];

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
                slotInfo.homogeneous = type;
            }
            else if (!slotInfo.heterogeneous)
            {
                if (BaseBlessingOf(slotInfo.homogeneous) != BaseBlessingOf(type))
                {
                    slotInfo.heterogeneous = true;
                    slotInfo.homogeneous = BLESSING_NONE;
                }
                else if (IsSingleVariant(slotInfo.homogeneous) ||
                         IsSingleVariant(type))
                {
                    slotInfo.homogeneous = ToSingleVariant(slotInfo.homogeneous);
                }
            }
        }
    }

    if (botPaladins.size() >= MAX_SLOTS)
    {
        BlessingType const fullCoverageOrder[MAX_SLOTS] = {
            BLESSING_SANCTUARY_GREATER,
            BLESSING_MIGHT_GREATER,
            BLESSING_KINGS_GREATER,
            BLESSING_WISDOM_GREATER
        };

        for (uint8 classId = 0; classId < MAX_CLASS_ID; ++classId)
        {
            if (!classPresent[classId])
                continue;

            bool classHasBlessing[5] = {};
            for (auto const& priority : effective)
            {
                if (priority.player->getClass() != classId)
                    continue;

                for (int slot = 0; slot < MAX_SLOTS; ++slot)
                {
                    BaseBlessingCategory category = BaseBlessingOf(priority.blessings[slot]);
                    if (category > BASE_NONE && category <= BASE_SANCTUARY)
                        classHasBlessing[category] = true;
                }
            }

            if (!classHasBlessing[BASE_MIGHT] || !classHasBlessing[BASE_WISDOM] ||
                !classHasBlessing[BASE_KINGS] || !classHasBlessing[BASE_SANCTUARY])
            {
                continue;
            }

            for (int slot = 0; slot < MAX_SLOTS; ++slot)
            {
                classSlots[classId][slot].heterogeneous = false;
                classSlots[classId][slot].homogeneous = fullCoverageOrder[slot];
            }
        }
    }

    // Paladins remain the special case below 4 paladins: their specs disagree enough
    // that singles are still needed until every blessing base can be covered by greater blessings.
    if (classPresent[CLASS_PALADIN] && botPaladins.size() < MAX_SLOTS)
    {
        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            SlotInfo& slotInfo = classSlots[CLASS_PALADIN][slot];
            if (!slotInfo.heterogeneous && slotInfo.homogeneous != BLESSING_NONE)
                slotInfo.homogeneous = ToSingleVariant(slotInfo.homogeneous);
        }

        bool assignedBase[5] = {};
        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            SlotInfo const& slotInfo = classSlots[CLASS_PALADIN][slot];
            if (!slotInfo.heterogeneous && slotInfo.homogeneous != BLESSING_NONE)
                assignedBase[BaseBlessingOf(slotInfo.homogeneous)] = true;
        }

        for (int slot = 0; slot < MAX_SLOTS; ++slot)
        {
            SlotInfo& slotInfo = classSlots[CLASS_PALADIN][slot];
            if (!slotInfo.heterogeneous)
                continue;

            int counts[5] = {};
            for (auto const& ep : effective)
            {
                if (ep.player->getClass() != CLASS_PALADIN)
                    continue;
                BaseBlessingCategory cat = BaseBlessingOf(ep.blessings[slot]);
                if (cat > BASE_NONE && cat <= BASE_SANCTUARY && !assignedBase[cat])
                    counts[cat]++;
            }

            BaseBlessingCategory best = BASE_NONE;
            int bestCount = 0;
            for (int i = 1; i <= 4; ++i)
            {
                if (counts[i] > bestCount)
                {
                    bestCount = counts[i];
                    best = static_cast<BaseBlessingCategory>(i);
                }
            }

            if (best != BASE_NONE)
            {
                slotInfo.heterogeneous = false;
                switch (best)
                {
                    case BASE_MIGHT: slotInfo.homogeneous = BLESSING_MIGHT_SINGLE;
                        break;
                    case BASE_WISDOM: slotInfo.homogeneous = BLESSING_WISDOM_SINGLE;
                        break;
                    case BASE_KINGS: slotInfo.homogeneous = BLESSING_KINGS_SINGLE;
                        break;
                    case BASE_SANCTUARY: slotInfo.homogeneous = BLESSING_SANCTUARY_SINGLE;
                        break;
                    default: slotInfo.homogeneous = BLESSING_NONE;
                        break;
                }
                assignedBase[best] = true;
            }
        }
    }

    // Step 6: Talent-Aware Slot Swapping
    // For each class, check if swapping two Paladins' slot assignments would better match
    // Improved Might/Wisdom talents.

    int classSlotPaladin[MAX_CLASS_ID][MAX_SLOTS];
    for (int classId = 0; classId < MAX_CLASS_ID; ++classId)
        for (int slot = 0; slot < MAX_SLOTS; ++slot)
            classSlotPaladin[classId][slot] = slot;

    int numPals = static_cast<int>(botPaladins.size());

    for (int classId = 0; classId < MAX_CLASS_ID; ++classId)
    {
        if (!classPresent[classId]) continue;

        for (int slot1 = 0; slot1 < numPals && slot1 < MAX_SLOTS; ++slot1)
        {
            for (int slot2 = slot1 + 1; slot2 < numPals && slot2 < MAX_SLOTS; ++slot2)
            {
                int p1 = classSlotPaladin[classId][slot1];
                int p2 = classSlotPaladin[classId][slot2];

                BaseBlessingCategory category1 = classSlots[classId][slot1].heterogeneous
                    ? BASE_NONE : BaseBlessingOf(classSlots[classId][slot1].homogeneous);
                BaseBlessingCategory category2 = classSlots[classId][slot2].heterogeneous
                    ? BASE_NONE : BaseBlessingOf(classSlots[classId][slot2].homogeneous);

                if (classSlots[classId][slot1].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& priority : effective)
                    {
                        if (priority.player->getClass() != classId)
                            continue;
                        BaseBlessingCategory category = BaseBlessingOf(priority.blessings[slot1]);
                        if (category < 5)
                            counts[category]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                    {
                        if (counts[i] > counts[best])
                            best = i;
                    }
                    category1 = static_cast<BaseBlessingCategory>(best);
                }
                if (classSlots[classId][slot2].heterogeneous)
                {
                    int counts[5] = {};
                    for (auto const& priority : effective)
                    {
                        if (priority.player->getClass() != classId)
                            continue;
                        BaseBlessingCategory category = BaseBlessingOf(priority.blessings[slot2]);
                        if (category < 5)
                            counts[category]++;
                    }
                    int best = 0;
                    for (int i = 1; i < 5; ++i)
                    {
                        if (counts[i] > counts[best])
                            best = i;
                    }
                    category2 = static_cast<BaseBlessingCategory>(best);
                }

                auto talentMatchScore = [&](int palIdx, BaseBlessingCategory category) -> int
                {
                    if (palIdx >= numPals)
                        return 0;

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
                    classSlotPaladin[classId][slot1] = p2;
                    classSlotPaladin[classId][slot2] = p1;
                }
            }
        }
    }

    // Step 7: Build Final Assignments for Each Paladin
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
            BlessingType type = priority.blessings[myClassSlot];

            for (int otherSlot = 0; otherSlot < numPals && otherSlot < MAX_SLOTS; ++otherSlot)
            {
                if (otherSlot == myClassSlot)
                    continue;

                SlotInfo const& otherSlotInfo = classSlots[cls][otherSlot];
                if (otherSlotInfo.heterogeneous || otherSlotInfo.homogeneous == BLESSING_NONE)
                    continue;

                if (BaseBlessingOf(otherSlotInfo.homogeneous) == BaseBlessingOf(type))
                {
                    type = BLESSING_NONE;
                    break;
                }
            }

            assigned.blessing = IsSingleVariant(type) ? type : ToSingleVariant(type);
        }
        else
        {
            assigned.blessing = slotInfo.homogeneous;
        }

        if (BaseBlessingOf(assigned.blessing) == BASE_SANCTUARY &&
            !KnowsSanctuary(bot))
        {
            assigned.blessing = BLESSING_NONE;
        }

        outAssignments.push_back(assigned);
    }

    return !outAssignments.empty();
}
