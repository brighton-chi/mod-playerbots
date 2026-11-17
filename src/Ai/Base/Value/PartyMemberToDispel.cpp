/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PartyMemberToDispel.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellMgr.h"
#include <optional>

class PartyMemberToDispelPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PartyMemberToDispelPredicate(PlayerbotAI* botAI, uint32 dispelType, std::optional<float> range)
        : FindPlayerPredicate(), PlayerbotAIAware(botAI), dispelType(dispelType), maxRange(range)
    {
    }

    bool Check(Unit* unit) override
    {
        if (!unit->IsAlive())
            return false;

        if (maxRange && !botAI->GetBot()->IsWithinCombatRange(unit, *maxRange))
            return false;

        return botAI->HasAuraToDispel(unit, dispelType);
    }

private:
    uint32 dispelType;
    std::optional<float> maxRange;
};

// Qualifier "<dispel type>" or "<dispel type> <spell>". With a spell the bot knows, the pick is
// limited to that spell's range, tested the way the core tests a friendly target.
Unit* PartyMemberToDispel::Calculate()
{
    uint32 dispelType = atoi(qualifier.c_str());

    std::optional<float> maxRange;
    std::size_t const spellStart = qualifier.find(' ');
    if (spellStart != std::string::npos)
    {
        uint32 const spellId = AI_VALUE2(uint32, "spell id", qualifier.substr(spellStart + 1));
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            maxRange = spellInfo->GetMaxRange(true, bot);
    }

    PartyMemberToDispelPredicate predicate(botAI, dispelType, maxRange);
    return FindPartyMember(predicate);
}
