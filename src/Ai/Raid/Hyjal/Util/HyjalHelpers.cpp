/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <string>

using namespace EncounterHelpers;

namespace HyjalHelpers
{

namespace
{
// Every ground hazard is read through a cached value rather than searched for directly.
std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, std::string const& value)
{
    return botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet();
}
}

// General

bool GetHazardBlockedArc(
    Position const& ringCenter, float ringRadius, Position const& hazard,
    float hazardRadius, BlockedArc& arc)
{
    float const centerToHazard =
        hazard.GetExactDist2d(ringCenter.GetPositionX(), ringCenter.GetPositionY());

    if (centerToHazard <= 0.0f)
    {
        arc = { 0.0f, static_cast<float>(M_PI) };
        return ringRadius < hazardRadius;
    }

    float const cosHalfWidth =
        (centerToHazard * centerToHazard + ringRadius * ringRadius - hazardRadius * hazardRadius) /
        (2.0f * centerToHazard * ringRadius);

    if (cosHalfWidth >= 1.0f)
        return false;

    arc.center = std::atan2(
        hazard.GetPositionY() - ringCenter.GetPositionY(),
        hazard.GetPositionX() - ringCenter.GetPositionX());
    arc.halfWidth = (cosHalfWidth <= -1.0f) ? static_cast<float>(M_PI) : std::acos(cosHalfWidth);

    return true;
}

bool FindNearestUnblockedAngle(
    std::vector<BlockedArc> const& blocked, float preferred, float& unblocked)
{
    auto offsetFrom = [](float angle, float from)
    {
        float offset = Position::NormalizeOrientation(angle - from);
        if (offset > M_PI)
            offset -= 2.0f * M_PI;

        return offset;
    };

    auto isUnblocked = [&blocked, &offsetFrom](float angle)
    {
        for (BlockedArc const& arc : blocked)
        {
            if (std::fabs(offsetFrom(angle, arc.center)) < arc.halfWidth)
                return false;
        }

        return true;
    };

    if (isUnblocked(preferred))
    {
        unblocked = preferred;
        return true;
    }

    constexpr float edgeNudge = 0.01f;
    bool found = false;
    float bestOffset = std::numeric_limits<float>::max();

    for (BlockedArc const& arc : blocked)
    {
        for (int8 side = -1; side <= 1; side += 2)
        {
            float const edge = arc.center + side * (arc.halfWidth + edgeNudge);
            if (!isUnblocked(edge))
                continue;

            float const offset = offsetFrom(edge, preferred);
            if (std::fabs(offset) < std::fabs(bestOffset))
            {
                bestOffset = offset;
                unblocked = edge;
                found = true;
            }
        }
    }

    return found;
}

bool FindStepToCircle(
    Player* bot, Position const& center, float radius, float preferredAngle, float moveDist,
    float& stepX, float& stepY, float& stepZ, std::function<bool(float, float)> const& isAcceptable,
    float* chosenX, float* chosenY)
{
    float const centerX = center.GetPositionX();
    float const centerY = center.GetPositionY();

    constexpr uint8 fanSteps = 8;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        float const delta = fanStep * step;
        uint8 const candidates = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < candidates; ++i)
        {
            float const angle = preferredAngle + (i == 0 ? delta : -delta);
            float const targetX = centerX + std::cos(angle) * radius;
            float const targetY = centerY + std::sin(angle) * radius;

            if (isAcceptable && !isAcceptable(targetX, targetY))
                continue;

            if (!CanTakeStepTowards(bot, targetX, targetY, moveDist, stepX, stepY, stepZ))
                continue;

            if (isAcceptable && !isAcceptable(stepX, stepY))
                continue;

            if (chosenX)
                *chosenX = targetX;
            if (chosenY)
                *chosenY = targetY;

            return true;
        }
    }

    return false;
}

bool GetHazardEscapeStep(
    Player* bot, Position const& hazard, float escapeRadius, float moveDist, float& stepX,
    float& stepY, float& stepZ, std::function<bool(float, float)> const& isAcceptable)
{
    float const centerX = hazard.GetPositionX();
    float const centerY = hazard.GetPositionY();
    float escapeAngle =
        std::atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);

    if (bot->GetExactDist2d(centerX, centerY) <= 0.1f)
        escapeAngle = bot->GetOrientation();

    return FindStepToCircle(
        bot, hazard, escapeRadius, escapeAngle, moveDist, stepX, stepY, stepZ, isAcceptable);
}

std::vector<Player*> GetRangedMembers(Player* bot)
{
    std::vector<Player*> members;
    Group* group = bot->GetGroup();
    if (!group)
        return members;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == HYJAL_MAP_ID && GET_PLAYERBOT_AI(member) &&
            PlayerbotAI::IsRanged(member))
        {
            members.push_back(member);
        }
    }

    return members;
}

RangedGroups GetRangedGroups(Player* bot)
{
    RangedGroups result;
    for (Player* member : GetRangedMembers(bot))
    {
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

bool GetDeathAndDecayPosition(PlayerbotAI* botAI, Position& deathAndDecay)
{
    std::vector<Position> const& positions =
        GetCachedHazardPositions(botAI, "hyjal death and decay");
    if (positions.empty())
        return false;

    deathAndDecay = positions.front();
    return true;
}

bool IsNearDeathAndDecay(PlayerbotAI* botAI, float radius)
{
    Position deathAndDecay;
    return GetDeathAndDecayPosition(botAI, deathAndDecay) &&
        botAI->GetBot()->GetExactDist2d(deathAndDecay) < radius;
}

bool IsInDeathAndDecay(PlayerbotAI* botAI)
{
    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_RADIUS);
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

GuidVector FindInfernalGuids(Player* bot)
{
    std::list<Creature*> infernals;
    bot->GetCreatureListWithEntryInGrid(
        infernals, Id(HyjalNpcs::NPC_TOWERING_INFERNAL), INFERNAL_SEARCH_RADIUS);

    std::vector<Creature*> alive;
    alive.reserve(infernals.size());
    for (Creature* infernal : infernals)
    {
        if (infernal && infernal->IsAlive())
            alive.push_back(infernal);
    }

    std::sort(alive.begin(), alive.end(), [](Creature const* first, Creature const* second)
    {
        return first->GetGUID().GetCounter() < second->GetGUID().GetCounter();
    });

    GuidVector guids;
    guids.reserve(alive.size());
    for (Creature* infernal : alive)
        guids.push_back(infernal->GetGUID());

    return guids;
}

GuidVector const& GetInfernalGuids(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<GuidVector>("hyjal infernals")->RefGet();
}

Unit* GetLooseInfernal(PlayerbotAI* botAI)
{
    Player* infernalTank = GetInfernalTank(botAI->GetBot());
    if (!infernalTank)
        return nullptr;

    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        Unit* infernal = botAI->GetUnit(guid);
        if (infernal && infernal->GetVictim() != infernalTank)
            return infernal;
    }

    return nullptr;
}

Unit* GetNearestInfernal(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Unit* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        Unit* infernal = botAI->GetUnit(guid);
        if (!infernal)
            continue;

        float const distance = bot->GetExactDist2d(infernal);
        if (distance < nearestDistance)
        {
            nearest = infernal;
            nearestDistance = distance;
        }
    }

    return nearest;
}

Unit* GetInfernalToAttack(PlayerbotAI* botAI, Unit* anetheron)
{
    if (!anetheron || anetheron->GetHealthPct() < BOSS_BURN_HEALTH_PCT)
        return nullptr;

    Unit* infernal = nullptr;
    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        infernal = botAI->GetUnit(guid);
        if (infernal)
            break;
    }

    if (!infernal || botAI->GetBot()->GetExactDist2d(infernal) >= INFERNAL_RANGED_ENGAGE_DISTANCE)
        return nullptr;

    return infernal;
}

Unit* GetInfernalTargetingBot(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        Unit* infernal = botAI->GetUnit(guid);
        if (infernal && infernal->GetVictim() == bot)
            return infernal;
    }

    return nullptr;
}

bool IsInfernalTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

Player* GetInfernalTank(Player* bot)
{
    return GetGroupAssistTank(bot, 0);
}

Position const& GetInfernalTankPosition(Player* bot)
{
    Player* infernalTank = GetInfernalTank(bot);
    Player* from = infernalTank ? infernalTank : bot;

    Position const& east = ANETHERON_E_INFERNAL_POSITION;
    Position const& west = ANETHERON_W_INFERNAL_POSITION;
    return from->GetExactDist2d(east.GetPositionX(), east.GetPositionY()) <=
        from->GetExactDist2d(west.GetPositionX(), west.GetPositionY()) ? east : west;
}

// Kaz'rogal

std::unordered_set<ObjectGuid> botsBelowManaThreshold;

float GetKazrogalRangedArcRadius(Unit* kazrogal)
{
    return (kazrogal && kazrogal->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        ? KAZROGAL_RANGED_ARC_APPROACH_RADIUS : KAZROGAL_RANGED_ARC_RADIUS;
}

float GetKazrogalRangedArcSpan(float radius)
{
    float const ratio = KAZROGAL_RANGED_ARC_HALF_WIDTH / radius;
    return 2.0f * std::asin(ratio < 1.0f ? ratio : 1.0f);
}

bool IsKazrogalManaUser(PlayerbotAI* botAI)
{
    switch (botAI->GetBot()->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
            return false;

        case CLASS_DRUID:
            return !botAI->HasStrategy("bear", BOT_STATE_COMBAT) &&
                !botAI->HasStrategy("cat", BOT_STATE_COMBAT);

        default:
            return true;
    }
}

bool HasMarkOfKazrogal(Player* bot)
{
    return bot->HasAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
}

// Azgalor

// Each Rain of Fire is its own dynamic object that expires after 10s; there can be 2 up at a time.
std::vector<Position> GetRainOfFirePositions(PlayerbotAI* botAI)
{
    return GetCachedHazardPositions(botAI, "hyjal rain of fire");
}

bool GetNearestRainOfFirePosition(PlayerbotAI* botAI, Position& pool)
{
    Player* bot = botAI->GetBot();
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (Position const& position : GetCachedHazardPositions(botAI, "hyjal rain of fire"))
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearRainOfFire(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    for (Position const& position : GetCachedHazardPositions(botAI, "hyjal rain of fire"))
    {
        if (bot->GetExactDist2d(position) < radius)
            return true;
    }

    return false;
}

bool IsInRainOfFire(PlayerbotAI* botAI)
{
    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_RADIUS);
}

bool IsDoomed(Player* bot)
{
    return bot->HasAura(Id(HyjalSpells::SPELL_DOOM));
}

bool IsDoomguardTank(Player* bot)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // GetGroupAssistTank() skips dead tanks so the second assist tank naturally becomes the first
    // if the first dies.
    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    if (!firstAssistTank)
        return false;

    if (firstAssistTank == bot)
        return true;

    // The second assist tank also takes over while the first is Doomed and so about to die and
    // spawn a Doomguard of its own.
    return IsDoomed(firstAssistTank) && GetGroupAssistTank(bot, 1) == bot;
}

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
        if (member && IsDoomed(member))
            return true;
    }

    return false;
}

// Archimonde

std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;

bool HasProtectionOfElune(Player* bot)
{
    return bot->HasAura(Id(HyjalSpells::SPELL_PROTECTION_OF_ELUNE));
}

std::vector<Position> GetDoomfirePositions(PlayerbotAI* botAI)
{
    return GetCachedHazardPositions(botAI, "hyjal doomfire trail");
}

bool IsNearDoomfire(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    for (Position const& patch : GetCachedHazardPositions(botAI, "hyjal doomfire trail"))
    {
        if (bot->GetExactDist2d(patch) < radius)
            return true;
    }

    return false;
}

bool IsPositionNearDoomfire(PlayerbotAI* botAI, float x, float y, float radius)
{
    for (Position const& patch : GetCachedHazardPositions(botAI, "hyjal doomfire trail"))
    {
        if (patch.GetExactDist2d(x, y) < radius)
            return true;
    }

    return false;
}

bool GetPendingAirBurstCast(uint32 instanceId, AirBurstData& airBurst)
{
    auto instanceIt = archimondeAirBurstTargets.find(instanceId);
    if (instanceIt == archimondeAirBurstTargets.end())
        return false;

    constexpr uint32 airBurstReactionWindow = 2000;
    uint32 const now = getMSTime();
    if (getMSTimeDiff(instanceIt->second.castTime, now) >= airBurstReactionWindow)
    {
        archimondeAirBurstTargets.erase(instanceIt);
        return false;
    }

    airBurst = instanceIt->second;
    return true;
}

}
