/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
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

            if (member->GetMapId() != player->GetMapId())
                continue;

            if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
                return memberAI;
        }

        return nullptr;
    }

    Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
    {
        if (!spell || !caster)
            return nullptr;

        std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
        if (targets.empty())
            return nullptr;

        return ObjectAccessor::GetPlayer(*caster, targets.front().targetGUID);
    }

    void RequestInterruptForBotsNear(Unit* center, float radius)
    {
        if (!center)
            return;

        Group* group = center->ToPlayer() ? center->ToPlayer()->GetGroup() : nullptr;
        Map::PlayerList const& players = center->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            if (group && player->GetGroup() != group)
                continue;

            if (center->GetExactDist2d(player) > radius)
                continue;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
                botAI->RequestSpellInterrupt();
        }
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

class FelmystSpellListenerScript : public AllSpellScript
{
public:
    FelmystSpellListenerScript() : AllSpellScript("FelmystSpellListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!spell || !caster || !spellInfo || caster->GetMapId() != SUNWELL_MAP_ID ||
            caster->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_FELMYST))
        {
            return;
        }

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE):
                RequestInterruptForBotsNear(target, FELMYST_ENCAPSULATE_SAFE_DISTANCE);
                break;
            case static_cast<uint32>(SunwellSpells::SPELL_SUMMON_DEMONIC_VAPOR):
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(target))
                    botAI->RequestSpellInterrupt();
                break;
            default:
                break;
        }
    }
};

void AddSC_SunwellPlateauBotScripts()
{
    new KalecgosSpellListenerScript();
    new FelmystSpellListenerScript();
}