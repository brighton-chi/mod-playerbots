/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllCreatureScript.h"
#include "HyjalHelpers.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"
#include <list>

using namespace HyjalHelpers;

static Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
{
    if (!spell || !caster)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
    for (TargetInfo const& targetInfo : targets)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*caster, targetInfo.targetGUID))
            return target;
    }

    return nullptr;
}

static bool ShouldInterruptForArchimondeAirBurst(PlayerbotAI* botAI, Player* target)
{
    if (!target)
        return false;

    Player* bot = botAI->GetBot();
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || bot == mainTank)
        return false;

    if (target != mainTank && target != bot)
        return false;

    float const distanceToMainTank = bot->GetDistance2d(mainTank);
    return distanceToMainTank < AIR_BURST_SAFE_DISTANCE;
}

// Interrupts a cast when a Doomfire NPC comes too close. The trail it leaves behind is made of
// SPELL_DOOMFIRE_TRAIL dynamic objects, which bots query directly, so nothing is recorded here.
class ArchimondeDoomfireTrailScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailScript() : AllCreatureScript("ArchimondeDoomfireTrailScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != Id(HyjalNpcs::NPC_DOOMFIRE))
            return;

        constexpr float DOOMFIRE_DANGER_RANGE = 10.0f;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                creature->GetDistance(player) > DOOMFIRE_DANGER_RANGE)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        archimondeAirBurstTargets[caster->GetMap()->GetInstanceId()] =
            AirBurstData{ target->GetGUID(), getMSTime() };

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                !ShouldInterruptForArchimondeAirBurst(botAI, target))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new ArchimondeDoomfireTrailScript();
    new ArchimondeAirBurstSpellListenerScript();
}
