/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"
#include "TKHelpers.h"

using namespace TkHelpers;

class VoidReaverSpellListenerScript : public AllSpellScript
{
public:
    VoidReaverSpellListenerScript() : AllSpellScript("VoidReaverSpellListenerScript") { }

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != static_cast<uint32>(TkSpells::SPELL_ARCANE_ORB))
            return;

        std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
        if (targets.empty())
            return;

        Player* target = ObjectAccessor::GetPlayer(*caster, targets.front().targetGUID);
        if (!target)
            return;

        auto& orbs = voidReaverArcaneOrbs[caster->GetMap()->GetInstanceId()];
        uint32 currentTime = getMSTime();

        ArcaneOrbData orbData;
        orbData.destination = target->GetPosition();
        orbData.castTime = currentTime;

        orbs.push_back(orbData);

        orbs.erase(std::remove_if(orbs.begin(), orbs.end(),
            [currentTime](ArcaneOrbData const& orb) {
                return getMSTimeDiff(orb.castTime, currentTime) > 5000;
            }), orbs.end());
    }
};

void AddSC_TempestKeepBotScripts()
{
    new VoidReaverSpellListenerScript();
}
