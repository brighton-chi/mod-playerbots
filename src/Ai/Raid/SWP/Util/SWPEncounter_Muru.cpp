/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "AiObjectContext.h"
#include "CharmInfo.h"
#include "Playerbots.h"
#include <algorithm>
#include <list>

// Note: M'uru goes invisible during the Entropius phase but remains on player threat lists

namespace SwpHelpers
{

std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> muruVoidSentinelTankAssignments;

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru)
{
    if (!muru)
        return false;

    constexpr uint32 darknessPreEffectMs = 3000;
    constexpr uint32 darknessCastMs = 2000;
    constexpr uint32 darknessPostCastDangerMs = 18000;
    constexpr uint32 darknessTotalMs =
        darknessPreEffectMs + darknessCastMs + darknessPostCastDangerMs;
    uint32 const instanceId = bot->GetInstanceId();
    uint32 const now = getMSTime();
    MuruDarknessState& state = muruDarknessStates[instanceId];

    if (Aura* darknessPreEffect = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
    {
        int32 remainingPreEffectMs = darknessPreEffect->GetDuration();
        if (remainingPreEffectMs < 0)
            remainingPreEffectMs = darknessPreEffectMs;

        uint32 const remainingPreEffect = static_cast<uint32>(remainingPreEffectMs);
        uint32 const elapsedPreEffectMs = remainingPreEffect < darknessPreEffectMs ?
            darknessPreEffectMs - remainingPreEffect : 0;
        uint32 const startMs = now > elapsedPreEffectMs ? now - elapsedPreEffectMs : 0;

        if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
            state.startMs = startMs;

        state.expireMs = std::max(state.expireMs, startMs + darknessTotalMs);
        return true;
    }

    if (muru->HasUnitState(UNIT_STATE_CASTING) &&
        muru->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_DARKNESS)))
    {
        uint32 const startMs = now > darknessPreEffectMs ? now - darknessPreEffectMs : 0;
        if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
            state.startMs = startMs;

        state.expireMs = std::max(state.expireMs, now + darknessCastMs + darknessPostCastDangerMs);
        return true;
    }

    if (state.expireMs > now)
        return true;

    muruDarknessStates.erase(instanceId);
    return false;
}

bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs)
{
    if (!TryGetMuruDarknessActiveState(bot, muru))
        return false;

    auto const stateItr = muruDarknessStates.find(bot->GetInstanceId());
    if (stateItr == muruDarknessStates.end())
        return false;

    uint32 const now = getMSTime();
    return stateItr->second.startMs < now && now - stateItr->second.startMs < earlyWindowMs;
}

namespace
{

MuruEncounterGuids const& GetCachedMuruEncounterGuids(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<MuruEncounterGuids>("muru encounter targets")->RefGet();
}

void ResolveUnits(PlayerbotAI* botAI, GuidVector const& guids, std::vector<Unit*>& units)
{
    units.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        if (Unit* unit = botAI->GetUnit(guid))
            units.push_back(unit);
    }
}

// Reach of the furthest ability the class brings to each job, or 0 when it brings none. Selection
// and the triggers both go through these, so the class lists cannot drift from the switches in
// MuruCastStunOnShadowswordBerserkerAction and MuruInterruptFelFireballAction.
float GetBerserkerStunReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MURU_MELEE_ABILITY_REACH;

        case CLASS_PALADIN:
            return MURU_HAMMER_OF_JUSTICE_REACH;

        case CLASS_WARLOCK:
            return MURU_RANGED_ABILITY_REACH;

        default:
            return bot->getRace() == RACE_TAUREN ? MURU_WAR_STOMP_REACH : 0.0f;
    }
}

float GetFuryMageInterruptReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MURU_MELEE_ABILITY_REACH;

        case CLASS_SHAMAN:
            return MURU_WIND_SHEAR_REACH;

        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_PALADIN:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return MURU_RANGED_ABILITY_REACH;

        case CLASS_HUNTER:
            return MURU_SILENCING_SHOT_REACH;

        default:
            return 0.0f;
    }
}

bool IsFlurriedBerserker(Unit* berserker)
{
    return berserker->HasAura(Id(SwpSpells::SPELL_FLURRY)) &&
        !berserker->HasUnitState(UNIT_STATE_STUNNED);
}

bool IsCastingFelFireball(Unit* furyMage)
{
    return furyMage->HasUnitState(UNIT_STATE_CASTING) &&
        furyMage->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_FEL_FIREBALL));
}

bool IsSpellFuryBuffedFuryMage(Unit* furyMage)
{
    return furyMage->HasAura(Id(SwpSpells::SPELL_SPELL_FURY));
}

// The current target wins outright when it qualifies: it is already faced and in reach, so nothing
// has to move, and it stops the pick flipping between equally valid adds on consecutive ticks.
Unit* SelectNearestQualifying(
    PlayerbotAI* botAI, GuidVector const& candidates, float reach, bool (*qualifies)(Unit*))
{
    Player* bot = botAI->GetBot();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();

    Unit* best = nullptr;
    float bestDistance = 0.0f;

    for (ObjectGuid const& guid : candidates)
    {
        Unit* candidate = botAI->GetUnit(guid);
        if (!candidate || !candidate->IsAlive() || !qualifies(candidate))
            continue;

        float const distance = bot->GetExactDist(candidate);
        if (distance > reach)
            continue;

        if (candidate == currentTarget)
            return candidate;

        if (!best || distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

} // end anonymous namespace

MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = AI_VALUE(GuidVector, "possible targets no los");

    MuruEncounterGuids guids;
    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_MURU):
                guids.muru = guid;
                break;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                guids.entropius = guid;
                break;

            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                guids.voidSentinels.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_VOID_SPAWN):
                guids.voidSpawns.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                guids.furyMages.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
                guids.berserkers.push_back(guid);
                break;

            default:
                break;
        }
    }

    return guids;
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    MuruEncounterGuids const& guids = GetCachedMuruEncounterGuids(botAI);

    targets.muru = botAI->GetUnit(guids.muru);
    targets.entropius = botAI->GetUnit(guids.entropius);
    ResolveUnits(botAI, guids.voidSentinels, targets.voidSentinels);
    ResolveUnits(botAI, guids.voidSpawns, targets.voidSpawns);
    ResolveUnits(botAI, guids.furyMages, targets.furyMages);
    ResolveUnits(botAI, guids.berserkers, targets.berserkers);
}

// Deliberately not FindTargetValue: that walks the bot's own threatened-by-me list and returns the
// first name match, so it answers "one arbitrary berserker that happens to be attacking me" - with
// four per wave it routinely misses the one that actually has Flurry.
Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI)
{
    float const reach = GetBerserkerStunReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).berserkers, reach, &IsFlurriedBerserker);
}

Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI)
{
    float const reach = GetFuryMageInterruptReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, reach, &IsCastingFelFireball);
}

// Mage-only, so the class gate is inline rather than a reach table: Spellsteal is the whole list
Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI)
{
    if (botAI->GetBot()->getClass() != CLASS_MAGE)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, MURU_RANGED_ABILITY_REACH,
        &IsSpellFuryBuffedFuryMage);
}

Creature* FindAvailableVoidSpawnForEnslave(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = AI_VALUE(GuidVector, "possible targets no los");

    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN) ||
            unit->IsCharmed() || unit->GetCharmer())
        {
            continue;
        }

        float distance = bot->GetExactDist2d(unit);
        if (distance >= closestDistance)
            continue;

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        bestSpawn = creature;
        closestDistance = distance;
    }

    return bestSpawn;
}

}
