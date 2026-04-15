/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <iomanip>
#include <sstream>

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

    char const* GetFelmystFogLaneName(FelmystFogLane lane)
    {
        switch (lane)
        {
            case FelmystFogLane::Top:
                return "top";
            case FelmystFogLane::Middle:
                return "middle";
            case FelmystFogLane::Bottom:
                return "bottom";
            default:
                return "none";
        }
    }

    std::string FormatFelmystFogPoint(Position const& position)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << '(' << position.GetPositionX() << ", "
            << position.GetPositionY() << ", "
            << position.GetPositionZ() << ')';
        return out.str();
    }

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

    void RequestInterruptForBotsNeedingFelmystFogMovement(Unit* contextUnit, Player* groupReference)
    {
        if (!contextUnit)
            return;

        Group* group = groupReference ? groupReference->GetGroup() : nullptr;
        Map::PlayerList const& players = contextUnit->GetMap()->GetPlayers();
        uint32 botCount = 0;
        uint32 activeFogCount = 0;
        uint32 requestedCount = 0;
        uint32 missingFelmystCount = 0;
        uint32 noDestinationCount = 0;

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

            ++botCount;

            Unit* felmyst = PAI_VALUE2(Unit*, "find target", "felmyst");
            if (!felmyst || !felmyst->IsFlying())
            {
                ++missingFelmystCount;
                continue;
            }

            FelmystFogOfCorruptionState fogState;
            if (!GetActiveFelmystFogOfCorruptionState(player, felmyst, fogState))
                continue;

            ++activeFogCount;

            Position destination;
            if (!TryGetFelmystFogSidewaysShiftDestination(player, fogState.lane, destination))
            {
                ++noDestinationCount;
                continue;
            }

            LOG_DEBUG("playerbots",
                "[FelmystFog] {} interrupt requested lane={} destination={}",
                player->GetName(), GetFelmystFogLaneName(fogState.lane),
                FormatFelmystFogPoint(destination));
            botAI->RequestSpellInterrupt();
            ++requestedCount;
        }

        LOG_DEBUG("playerbots",
            "[FelmystFog] interrupt scan casterEntry={} groupReference={} bots={} active={} requested={} missingFelmyst={} noDestination={}",
            contextUnit->GetEntry(), groupReference ? groupReference->GetName() : "none",
            botCount, activeFogCount, requestedCount,
            missingFelmystCount, noDestinationCount);
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
        if (!spell || !caster || !spellInfo || caster->GetMapId() != SUNWELL_MAP_ID)
            return;

        if (spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION) ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_TOP ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_MIDDLE ||
            spellInfo->Id == SPELL_FELMYST_STRAFE_BOTTOM)
        {
            Player* targetPlayer = GetFirstPlayerSpellTarget(spell, caster);
            Player* groupReference = caster->ToPlayer() ? caster->ToPlayer() : targetPlayer;
            if (targetPlayer)
            {
                LOG_DEBUG("playerbots",
                    "[FelmystFog] spell seen id={} casterEntry={} target={}",
                    spellInfo->Id, caster->GetEntry(), targetPlayer->GetName());
            }
            else
            {
                LOG_DEBUG("playerbots",
                    "[FelmystFog] spell seen id={} casterEntry={} target=none",
                    spellInfo->Id, caster->GetEntry());
            }

            RequestInterruptForBotsNeedingFelmystFogMovement(caster, groupReference);

            return;
        }

        if (caster->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_FELMYST))
            return;

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