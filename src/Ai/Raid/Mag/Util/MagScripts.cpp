/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "DynamicObject.h"
#include "DynamicObjectScript.h"
#include "MagHelpers.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagHelpers;

class MagtheridonQuakeSpellListenerScript : public AllSpellScript
{
public:
    MagtheridonQuakeSpellListenerScript() : AllSpellScript("MagtheridonQuakeSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != static_cast<uint32>(MagSpells::SPELL_QUAKE))
            return;

        // To account for Blast Nova delay caused by Quake's DelayAll(6999ms)
        auto it = blastNovaTimer.find(caster->GetInstanceId());
        if (it != blastNovaTimer.end())
            it->second += 7 * IN_MILLISECONDS;
    }
};

class MagtheridonDebrisDynamicObjectScript : public DynamicObjectScript
{
public:
    MagtheridonDebrisDynamicObjectScript() :
        DynamicObjectScript("MagtheridonDebrisDynamicObjectScript") {}

    void OnUpdate(DynamicObject* debris, uint32 /*diff*/) override
    {
        if (debris->GetSpellId() != Id(MagSpells::SPELL_DEBRIS_SPAWN))
            return;

        Map::PlayerList const& players = debris->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("magtheridon", BOT_STATE_COMBAT) ||
                debris->GetExactDist2d(player) > DEBRIS_HAZARD_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonQuakeSpellListenerScript();
    new MagtheridonDebrisDynamicObjectScript();
}
