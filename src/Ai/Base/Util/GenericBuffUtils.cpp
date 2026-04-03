/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericBuffUtils.h"

#include "Player.h"
#include "SpellMgr.h"

namespace ai::buff
{
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

        return name;
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
            // No reagent required
            return true;
        }
        return false;
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
