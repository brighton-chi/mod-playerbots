/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "Playerbots.h"
#include "SSCHelpers.h"

using namespace SscHelpers;

// A bot can't step out of Toxic Spores until a cast bar finishes, and no action can cancel one, so
// a new pool cancels the cast of every bot it lands on.
class LadyVashjToxicSporesSpellListenerScript : public AllSpellScript
{
public:
    LadyVashjToxicSporesSpellListenerScript()
        : AllSpellScript("LadyVashjToxicSporesSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SscSpells::SPELL_TOXIC_SPORES))
            return;

        // The caster is the Spore Drop Trigger, which stands at the center of the pool it lays
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() ||
                caster->GetExactDist2d(player) >= TOXIC_SPORES_HIT_RADIUS)
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("ssc", BOT_STATE_COMBAT))
                continue;

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_SerpentshrineCavernBotScripts()
{
    new LadyVashjToxicSporesSpellListenerScript();
}
