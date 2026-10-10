/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "BTHelpers.h"
#include "Pet.h"
#include "Playerbots.h"
#include "Spell.h"

using namespace BtHelpers;

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
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_RUNE_SHIELD))
            return;

        constexpr float spellstealRange = 30.0f;
        uint32 const spellsteal = Id(BtSpells::SPELL_SPELLSTEAL);
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
                player->HasSpellCooldown(spellsteal) ||
                player->HasAura(Id(BtSpells::SPELL_RUNE_SHIELD)))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// While her melee can't interrupt (Blessing of Protection, or a Zerevor patch on her), a ranged
// bot that could interrupt Lady Malande's Circle of Healing but would still be casting when it
// closes drops its own cast.
class IllidariCouncilCircleOfHealingSpellListenerScript : public AllSpellScript
{
public:
    IllidariCouncilCircleOfHealingSpellListenerScript()
        : AllSpellScript("IllidariCouncilCircleOfHealingSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_CIRCLE_OF_HEALING) ||
            caster->GetEntry() != Id(BtNpcs::NPC_LADY_MALANDE))
        {
            return;
        }

        if (!caster->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION)) &&
            !IsMalandeInZerevorPatch(
                caster, caster->FindNearestCreature(
                    Id(BtNpcs::NPC_HIGH_NETHERMANCER_ZEREVOR), COUNCIL_ZEREVOR_SEARCH_RADIUS)))
        {
            return;
        }

        int32 const latestCastEnd = spell->GetCastTime() - CIRCLE_OF_HEALING_CASTER_MARGIN_MS;
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            Spell* ownCast = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (!ownCast || ownCast->getState() != SPELL_STATE_PREPARING ||
                ownCast->GetCastTimeRemaining() <= latestCastEnd)
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("blacktemple", BOT_STATE_COMBAT) ||
                !PlayerbotAI::IsRangedDps(player) || IsZerevorMageTank(botAI) ||
                !CanInterrupt(player, caster))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }

private:
    static bool CanInterrupt(Player* player, Unit* malande)
    {
        uint32 interrupt = 0;
        float range = 0.0f;
        switch (player->getClass())
        {
            case CLASS_MAGE:
                interrupt = Id(BtSpells::SPELL_COUNTERSPELL);
                range = 30.0f;
                break;
            case CLASS_SHAMAN:
                interrupt = Id(BtSpells::SPELL_WIND_SHEAR);
                range = 25.0f;
                break;
            case CLASS_WARLOCK:
                return GetReadySpellLock(player->GetPet(), malande) != 0;
            default:
                return false;
        }

        return player->HasSpell(interrupt) && !player->HasSpellCooldown(interrupt) &&
            player->IsWithinCombatRange(malande, range) && player->IsWithinLOSInMap(malande);
    }
};

void AddSC_BlackTempleBotScripts()
{
    new ReliquaryOfSoulsRuneShieldSpellListenerScript();
    new IllidariCouncilCircleOfHealingSpellListenerScript();
}
