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

    float const distanceToMainTank = bot->GetExactDist2d(mainTank);
    return distanceToMainTank < AIR_BURST_SAFE_DISTANCE;
}

// Interrupts a cast when a Doomfire NPC comes too close. The trail it leaves behind is made of
// SPELL_DOOMFIRE_TRAIL dynamic objects, which bots query directly, so nothing is recorded here.
//
// Keyed on the Doomfire itself and not on the Doomfire Spirit it follows. The Spirit is the invisible
// one, and it does not walk--it NearTeleportTo's up to 8 yards every 1600ms--so reading it would let
// bots react to a position no human can see, and to one the fire has not reached yet. The Doomfire
// walks after it carrying 31945, a 1s periodic that drops the next patch at its own feet, so its
// position is where fire is about to be. That is what makes DOOMFIRE_DANGER_RADIUS the right figure
// here: the same distance the avoidance keeps from patches already on the ground
class ArchimondeDoomfireTrailScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailScript() : AllCreatureScript("ArchimondeDoomfireTrailScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != Id(HyjalNpcs::NPC_DOOMFIRE))
            return;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            // Centre to centre, as the avoidance measures. GetDistance would subtract both object
            // sizes, quietly making this wider than the figure it shares
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                creature->GetExactDist2d(player) > DOOMFIRE_DANGER_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// Air Burst is a 2s cast that hits all players within 13y of the target
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

// Inferno summons a Towering Infernal at its target's then-current position after a 3.5s cast
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
        if (!target)
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
        if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
            return;

        botAI->RequestSpellInterrupt();
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new ArchimondeDoomfireTrailScript();
    new ArchimondeAirBurstSpellListenerScript();
    new AnetheronInfernoSpellListenerScript();
}
