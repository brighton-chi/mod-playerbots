/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellHelpers.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"

using namespace SunwellHelpers;

namespace
{
    PlayerbotAI* GetKalecgosReferenceBotAI(Player* player)
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            return botAI;

        Group* group = player->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsInWorld() || !member->IsAlive())
                continue;

            if (member->GetMapId() != player->GetMapId() || member->GetInstanceId() != player->GetInstanceId())
                continue;

            if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
                return memberAI;
        }

        return nullptr;
    }
}

class KalecgosSpellListenerScript : public AllSpellScript
{
public:
    KalecgosSpellListenerScript() : AllSpellScript("KalecgosSpellListenerScript") { }

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo || caster->GetMapId() != SUNWELL_MAP_ID)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        PlayerbotAI* botAI = GetKalecgosReferenceBotAI(player);

        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_BLAST_PORTAL):
                if (!botAI)
                    return;

                RecordKalecgosSpectralBlastPortal(botAI, player);
                break;
            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_SPECTRAL):
                if (!botAI)
                    return;

                RecordKalecgosSpectralRealmEnter(botAI, player);
                break;
            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_NORMAL_REALM):
                RecordKalecgosNormalRealmEnter(player);
                break;
            default:
                break;
        }
    }
};

void AddSC_SunwellPlateauBotScripts()
{
    new KalecgosSpellListenerScript();
}