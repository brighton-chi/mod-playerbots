/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "BTHelpers.h"
#include "Playerbots.h"
#include "SpellHistory.h"

using namespace BlackTempleHelpers;

// A mage mid-cast when Essence of Desire raises Rune Shield drops the cast so it can steal the
// shield. Interrupting a preparing cast also cancels its global cooldown.
class ReliquaryOfSoulsRuneShieldSpellListenerScript : public AllSpellScript
{
public:
    ReliquaryOfSoulsRuneShieldSpellListenerScript()
        : AllSpellScript("ReliquaryOfSoulsRuneShieldSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(BlackTempleSpells::SPELL_RUNE_SHIELD))
            return;

        constexpr float spellstealRange = 30.0f;
        uint32 const spellsteal = Id(BlackTempleSpells::SPELL_SPELLSTEAL);
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || player->getClass() != CLASS_MAGE ||
                !player->IsWithinCombatRange(caster, spellstealRange))
            {
                continue;
            }

            if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
                !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("blacktemple", BOT_STATE_COMBAT))
                continue;

            // A stolen Rune Shield can still be up when the next is cast.
            if (!player->HasSpell(spellsteal) ||
                player->GetSpellHistory()->HasCooldown(spellsteal) ||
                player->HasAura(Id(BlackTempleSpells::SPELL_RUNE_SHIELD)))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_BlackTempleBotScripts()
{
    new ReliquaryOfSoulsRuneShieldSpellListenerScript();
}
