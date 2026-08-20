/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "MagHelpers.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagHelpers;

// NOTE: Need to add DynObj Script also for spell interrupt in Debris
class MagtheridonBotSpellScript : public AllSpellScript
{
public:
    MagtheridonBotSpellScript() : AllSpellScript("MagtheridonBotSpellScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != static_cast<uint32>(MagSpells::SPELL_QUAKE))
            return;

        // To account for Blast Nova delay caused by Quake's DelayAll(6999ms)
        auto it = blastNovaTimer.find(caster->GetMap()->GetInstanceId());
        if (it != blastNovaTimer.end())
            it->second += 7 * IN_MILLISECONDS;
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonBotSpellScript();
}
