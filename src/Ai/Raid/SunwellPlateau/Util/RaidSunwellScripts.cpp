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
    constexpr uint32 SPELL_FELMYST_STRAFE_TOP = 45585;
    constexpr uint32 SPELL_FELMYST_STRAFE_MIDDLE = 45633;
    constexpr uint32 SPELL_FELMYST_STRAFE_BOTTOM = 45635;
    constexpr uint32 FELMYST_FOG_INTERRUPT_SCAN_INTERVAL_MS = 500;

    std::unordered_map<ObjectGuid, uint32> felmystFogInterruptLastScanTime;

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

    void RequestInterruptForBotSpellTarget(Spell* spell, Unit* caster)
    {
        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(target))
            botAI->RequestSpellInterrupt();
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

    void RequestInterruptForBotsNeedingFelmystFogMovement(Unit* contextUnit, Player* groupReference)
    {
        if (!contextUnit)
            return;

        Group* group = groupReference ? groupReference->GetGroup() : nullptr;
        Map::PlayerList const& players = contextUnit->GetMap()->GetPlayers();

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            if (group && player->GetGroup() != group)
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI)
                continue;

            Unit* felmyst = PAI_VALUE2(Unit*, "find target", "felmyst");
            if (!felmyst || !felmyst->IsFlying())
                continue;

            FelmystFogOfCorruptionState fogState;
            if (!TryGetActiveFelmystFogOfCorruptionState(player, felmyst, fogState))
                continue;

            std::array<Position, 3> destinations;
            uint8 destinationCount = 0;
            if (!TryGetFelmystFogSafeDestinations(player, fogState.lane, destinations, destinationCount))
                continue;

            botAI->RequestSpellInterrupt();
        }
    }
}

class FelmystFogInterruptFallbackScript : public AllCreatureScript
{
public:
    FelmystFogInterruptFallbackScript() : AllCreatureScript("FelmystFogInterruptFallbackScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature || creature->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_FELMYST) ||
            !creature->IsFlying())
        {
            return;
        }

        FelmystFogOfCorruptionState fogState;
        if (!TryGetFelmystFogOfCorruptionStageState(creature, fogState) ||
            fogState.lane == FelmystFogLane::None)
            return;

        uint32 now = getMSTime();
        uint32& lastScanTime = felmystFogInterruptLastScanTime[creature->GetGUID()];
        if (getMSTimeDiff(lastScanTime, now) < FELMYST_FOG_INTERRUPT_SCAN_INTERVAL_MS)
            return;

        lastScanTime = now;
        RequestInterruptForBotsNeedingFelmystFogMovement(creature, nullptr);
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature || creature->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_FELMYST))
            return;

        felmystFogInterruptLastScanTime.erase(creature->GetGUID());
    }
};

class KalecgosSpellListenerScript : public AllSpellScript
{
public:
    KalecgosSpellListenerScript() : AllSpellScript("KalecgosSpellListenerScript") { }

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        Player* player = caster->ToPlayer();
        if (!player)
            return;

        PlayerbotAI* botAI = GetKalecgosReferenceBotAI(player);

        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_BLAST_PORTAL):
                if (!botAI)
                    return;
                RecordKalecgosSpectralBlastTarget(botAI, player);
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
        if (spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION) ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_TOP ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_MIDDLE ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_BOTTOM)
        {
            Player* targetPlayer = GetFirstPlayerSpellTarget(spell, caster);
            Player* groupReference = caster->ToPlayer() ? caster->ToPlayer() : targetPlayer;
            RequestInterruptForBotsNeedingFelmystFogMovement(caster, groupReference);

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

class EredarTwinsSpellListenerScript : public AllSpellScript
{
public:
    EredarTwinsSpellListenerScript() : AllSpellScript("EredarTwinsSpellListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (caster->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_GRAND_WARLOCK_ALYTHESS) ||
            spellInfo->Id != static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION))
        {
            return;
        }

        RequestInterruptForBotSpellTarget(spell, caster);
    }
};

void AddSC_SunwellPlateauBotScripts()
{
    new FelmystFogInterruptFallbackScript();
    new KalecgosSpellListenerScript();
    new FelmystSpellListenerScript();
    new EredarTwinsSpellListenerScript();
}