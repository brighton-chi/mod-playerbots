/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllCreatureScript.h"
#include "DynamicObjectScript.h"
#include "HyjalHelpers.h"
#include "Player.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace HyjalHelpers;

// TEMPORARY DIAGNOSTIC. Measures the distance at which a ground hazard's aura is actually held,
// to settle whether membership is decided on centre distance or on centre distance plus the
// target's combat reach. Set hazardProbeEnabled to false to silence it, and delete the class once
// the radii in HyjalHelpers.h are confirmed.
class HyjalHazardRadiusProbeScript : public DynamicObjectScript
{
public:
    HyjalHazardRadiusProbeScript() : DynamicObjectScript("HyjalHazardRadiusProbeScript") {}

    void OnUpdate(DynamicObject* dynobj, uint32 /*diff*/) override
    {
        constexpr bool hazardProbeEnabled = false; // Set to true to reactivate
        if (!hazardProbeEnabled)
            return;

        uint32 const spellId = dynobj->GetSpellId();
        if (spellId != Id(HyjalSpells::SPELL_DEATH_AND_DECAY) &&
            spellId != Id(HyjalSpells::SPELL_RAIN_OF_FIRE))
        {
            return;
        }

        // No point sampling faster than membership is recalculated
        constexpr uint32 probeInterval = 500;
        uint32 const now = getMSTime();
        if (getMSTimeDiff(_lastProbe, now) < probeInterval)
            return;

        _lastProbe = now;

        // Deliberately not filtered to bots: the point is to stand a real character at chosen
        // distances, which a bot cannot be asked to do
        constexpr float probeRange = 30.0f;
        Map::PlayerList const& players = dynobj->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            float const distance3d = player->GetExactDist(dynobj);
            if (distance3d > probeRange)
                continue;

            LOG_INFO("playerbots",
                "[HazardProbe] spell={} radius={:.2f} player={} dist2d={:.2f} dist3d={:.2f} "
                "reach={:.2f} aura={}",
                spellId, dynobj->GetRadius(), player->GetName(),
                player->GetExactDist2d(dynobj), distance3d, player->GetCombatReach(),
                player->HasAura(spellId) ? 1 : 0);
        }
    }

private:
    uint32 _lastProbe = 0;
};

// The explicitly cast-at target, which is all that exists when a cast begins: Spell::prepare only
// resolves a target list for item casts, so for a creature's cast GetUniqueTargetInfo stays empty
// until Spell::cast runs at the end of the cast time. Both listeners below are driven by
// DoCastRandomTarget, which always supplies an explicit unit target
static Player* GetSpellPlayerTarget(Spell* spell)
{
    if (!spell)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

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

// Everything this listener is for happens before the spell lands: bots have to be clear of the
// target while it is still being cast, and a bot part-way through a cast of its own cannot move
// until that cast is dropped. It therefore hooks OnSpellPrepare, which fires once as the cast
// begins. OnSpellCast is no use here--it runs at the end of Spell::cast, so for anything with a
// cast time it reports the spell only after it has already gone off
class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetSpellPlayerTarget(spell);
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

// Inferno summons a Towering Infernal at its target's position, and that position is not read until
// the 3.5s cast completes--Spell::SelectSpellTargets runs from Spell::cast, not Spell::prepare, for
// a creature's cast. The target therefore carries the landing point with it and can walk the
// Infernal clear of the raid, but only if it starts moving at once, so a cast of its own has to go
class AnetheronInfernoSpellListenerScript : public AllSpellScript
{
public:
    AnetheronInfernoSpellListenerScript() :
        AllSpellScript("AnetheronInfernoSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_INFERNO))
            return;

        Player* target = GetSpellPlayerTarget(spell);
        if (!target || !target->IsAlive())
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
        if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
            return;

        botAI->RequestSpellInterrupt();
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new HyjalHazardRadiusProbeScript(); // temporary, see the class comment
    new ArchimondeDoomfireTrailScript();
    new ArchimondeAirBurstSpellListenerScript();
    new AnetheronInfernoSpellListenerScript();
}
