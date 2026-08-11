/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Timer.h"
#include <algorithm>

namespace HyjalHelpers
{

// General

RangedGroups GetRangedGroups(Player* bot)
{
    RangedGroups result;
    Group* group = bot->GetGroup();
    if (!group)
        return result;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != HYJAL_MAP_ID || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            result.healers.push_back(member);
        else
            result.rangedDps.push_back(member);
    }

    return result;
}

std::pair<size_t, size_t> GetBotCircleIndexAndCount(Player* bot, RangedGroups const& groups)
{
    std::vector<Player*> const& vec = PlayerbotAI::IsHeal(bot) ? groups.healers : groups.rangedDps;
    auto it = std::find(vec.begin(), vec.end(), bot);
    size_t index = (it != vec.end()) ? std::distance(vec.begin(), it) : 0;

    return {index, vec.size()};
}

// Rage Winterchill

bool GetDeathAndDecayPosition(Player* bot, Position& deathAndDecay)
{
    std::vector<Position> const positions = GetDynamicObjectPositions(
        bot, HAZARD_SEARCH_RADIUS, Id(HyjalSpells::SPELL_DEATH_AND_DECAY));

    if (positions.empty())
        return false;

    deathAndDecay = positions.front();
    return true;
}

bool IsNearDeathAndDecay(Player* bot, float radius)
{
    Position deathAndDecay;
    return GetDeathAndDecayPosition(bot, deathAndDecay) &&
        bot->GetExactDist2d(deathAndDecay) < radius;
}

bool IsInDeathAndDecay(Player* bot)
{
    return IsNearDeathAndDecay(bot, DEATH_AND_DECAY_SAFE_RADIUS);
}

// Anetheron

Player* GetInfernoTarget(Unit* anetheron)
{
    if (!anetheron)
        return nullptr;

    Spell* spell = anetheron->FindCurrentSpellBySpellId(Id(HyjalSpells::SPELL_INFERNO));
    if (!spell)
        return nullptr;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target ? target->ToPlayer() : nullptr;
}

Position const& GetClosestInfernalTankPosition(Player* bot)
{
    Position const& east = ANETHERON_E_INFERNAL_POSITION;
    Position const& west = ANETHERON_W_INFERNAL_POSITION;
    return bot->GetExactDist2d(east.GetPositionX(), east.GetPositionY()) <=
        bot->GetExactDist2d(west.GetPositionX(), west.GetPositionY()) ? east : west;
}

// Kaz'rogal

std::unordered_map<ObjectGuid, TankPositionState> kazrogalTankStep;
std::unordered_map<ObjectGuid, bool> isBelowManaThreshold;

// Azgalor

std::unordered_map<ObjectGuid, TankPositionState> azgalorTankStep;

TankPositionState GetAzgalorTankPositionState(PlayerbotAI* botAI, Player* bot)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return TankPositionState::Unknown;

    auto it = azgalorTankStep.find(mainTank->GetGUID());
    if (it != azgalorTankStep.end())
        return it->second;

    return TankPositionState::Unknown;
}

// Each Rain of Fire is its own dynamic object that expires after 10s on its own, so nothing has
// to be recorded to know whether one is still active. Azgalor casts on a timer that lets two
// overlap, so callers have to weigh all of them rather than just the nearest
std::vector<Position> GetRainOfFirePositions(Player* bot)
{
    return GetDynamicObjectPositions(
        bot, HAZARD_SEARCH_RADIUS, Id(HyjalSpells::SPELL_RAIN_OF_FIRE));
}

bool IsNearRainOfFire(Player* bot, float radius)
{
    for (Position const& position : GetRainOfFirePositions(bot))
    {
        if (bot->GetExactDist2d(position) < radius)
            return true;
    }

    return false;
}

bool IsInRainOfFire(Player* bot)
{
    return IsNearRainOfFire(bot, RAIN_OF_FIRE_RADIUS);
}

// Standing behind Azgalor is immune at any range, which is where melee want to be anyway. The
// range clause only matters for anyone who has to pass through his front
bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y)
{
    Unit* victim = azgalor->GetVictim();
    if (!victim)
        return true;

    if (victim->GetExactDist2d(x, y) > CLEAVE_CHAIN_RADIUS)
        return true;

    Position const candidate(x, y, azgalor->GetPositionZ());
    return !azgalor->HasInArc(CLEAVE_DANGER_ARC, &candidate);
}

bool AnyGroupMemberHasDoom(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(HyjalSpells::SPELL_DOOM)))
            return true;
    }

    return false;
}

// Archimonde

std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;

AirBurstData* GetRecentArchimondeAirBurst(uint32 instanceId)
{
    auto instanceIt = archimondeAirBurstTargets.find(instanceId);
    if (instanceIt == archimondeAirBurstTargets.end())
        return nullptr;

    constexpr uint32 airBurstReactionWindow = 2000;
    uint32 const now = getMSTime();
    if (getMSTimeDiff(instanceIt->second.castTime, now) >= airBurstReactionWindow)
    {
        archimondeAirBurstTargets.erase(instanceIt);
        return nullptr;
    }

    return &instanceIt->second;
}

}
