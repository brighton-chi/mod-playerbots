/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericBuffUtils.h"

#include "AiObjectContext.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "SpellMgr.h"
#include "Value.h"

namespace ai::buff
{
    static bool IsEligibleGroupForPartyBuffs(Group const* group)
    {
        if (!group)
            return false;

        switch (sPlayerbotAIConfig.autoPartyBuffs)
        {
            case AutoPartyBuffMode::RAID_ONLY:
                return group->isRaidGroup();
            case AutoPartyBuffMode::GROUP_OR_RAID:
                return true;
            case AutoPartyBuffMode::DISABLED:
                return false;
        }

        return false;
    }

    std::string MakeAuraQualifierForBuff(std::string const& name)
    {
        // Paladin
        if (name == "blessing of kings")        return "blessing of kings,greater blessing of kings";
        if (name == "blessing of might")        return "blessing of might,greater blessing of might";
        if (name == "blessing of wisdom")       return "blessing of wisdom,greater blessing of wisdom";
        if (name == "blessing of sanctuary")    return "blessing of sanctuary,greater blessing of sanctuary";
        // Druid
        if (name == "mark of the wild")         return "mark of the wild,gift of the wild";
        // Mage
        if (name == "arcane intellect")         return "arcane intellect,arcane brilliance";
        // Priest
        if (name == "power word: fortitude")    return "power word: fortitude,prayer of fortitude";
        if (name == "divine spirit")            return "divine spirit,prayer of spirit";
        if (name == "shadow protection")        return "shadow protection,prayer of shadow protection";

        return name;
    }

    std::string GroupVariantFor(std::string const& name)
    {
        // Druid
        if (name == "mark of the wild")         return "gift of the wild";
        // Mage
        if (name == "arcane intellect")         return "arcane brilliance";
        // Priest
        if (name == "power word: fortitude")    return "prayer of fortitude";
        if (name == "divine spirit")            return "prayer of spirit";
        if (name == "shadow protection")        return "prayer of shadow protection";

        // Paladin blessings are NOT included here — they are
        // coordinated by the auto greater blessing system instead.
        return std::string();
    }

    bool HasRequiredReagents(Player* bot, uint32 spellId)
    {
        if (!spellId)
            return false;

        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        {
            for (int i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                if (info->Reagent[i] > 0)
                {
                    uint32 const itemId = info->Reagent[i];
                    int32 const need = info->ReagentCount[i];
                    if ((int32)bot->GetItemCount(itemId, false) < need)
                        return false;
                }
            }
            return true;
        }
        return false;
    }

    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName)
    {
        Group* group = bot->GetGroup();
        if (!IsEligibleGroupForPartyBuffs(group))
            return baseName;

        std::string const groupName = GroupVariantFor(baseName);
        if (groupName.empty())
            return baseName;

        uint32 const groupSpellId = botAI->GetAiObjectContext()
            ->GetValue<uint32>("spell id", groupName)->Get();

        if (groupSpellId && HasRequiredReagents(bot, groupSpellId))
            return groupName;

        return baseName;
    }
}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId)
    {
        if (bot->HasSpellCooldown(spellId))
            return true;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return false;

        uint32 category = spellInfo->GetCategory();
        if (!category)
            return false;

        for (auto const& [cooldownSpellId, cooldown] : bot->GetSpellCooldownMap())
        {
            if (cooldown.category == category && bot->GetSpellCooldownDelay(cooldownSpellId) > 0)
                return true;
        }

        return false;
    }
}
