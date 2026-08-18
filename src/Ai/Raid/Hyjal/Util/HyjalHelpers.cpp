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
#include <cmath>
#include <list>

namespace HyjalHelpers
{

// General

// Every ground hazard here is read through a cached value rather than searched for directly. The
// cache is keyed per bot and refreshed on its own interval, so the several triggers, multipliers
// and actions that all ask about the same pool each tick share one grid search between them
static std::vector<Position> const& GetCachedHazardPositions(Player* bot, std::string const& value)
{
    static std::vector<Position> const none;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    return botAI ? botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet()
                 : none;
}

bool GetHazardBlockedArc(
    Position const& ringCenter, float ringRadius, Position const& hazard,
    float hazardRadius, BlockedArc& arc)
{
    float const centerToHazard =
        hazard.GetExactDist2d(ringCenter.GetPositionX(), ringCenter.GetPositionY());

    // Sitting on the ring centre it either swallows the whole ring or none of it
    if (centerToHazard <= 0.0f)
    {
        arc = { 0.0f, static_cast<float>(M_PI) };
        return ringRadius < hazardRadius;
    }

    // Cosine of the half-angle it blocks, by the law of cosines on the ring centre
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

    // The nearest unblocked angle always lies on the edge of one of the blocked arcs, so testing
    // every edge finds it without having to merge the arcs into their union first
    constexpr float edgeNudge = 0.01f;
    bool found = false;
    float bestOffset = 0.0f;

    for (BlockedArc const& arc : blocked)
    {
        for (int8 side = -1; side <= 1; side += 2)
        {
            float const edge = arc.center + side * (arc.halfWidth + edgeNudge);
            if (!isUnblocked(edge))
                continue;

            float const offset = offsetFrom(edge, preferred);
            if (!found || std::fabs(offset) < std::fabs(bestOffset))
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
    float& stepX, float& stepY, float& stepZ,
    std::function<bool(float, float)> const& isAcceptable, float* chosenX, float* chosenY,
    bool allowUnvalidatedFallback)
{
    float const centerX = center.GetPositionX();
    float const centerY = center.GetPositionY();

    // Counted rather than accumulated, so the sweep always reaches exactly 180 degrees instead of
    // depending on where eight roundings of an inexact step happen to land
    constexpr uint8 fanSteps = 8;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    // Two sweeps at most. The first asks whether each step can actually be taken, which is the
    // only place in Hyjal that does: a refusal costs nothing here because there are sixteen more
    // angles, and without it the bot commits to the first whatever is in the way--a rise too steep
    // for the navmesh, the foot of a small hill, leaves MoveTo with no path at all.
    //
    // The second sweep runs only for callers that would rather move badly than not move, and drops
    // the check while keeping the caller's own predicate. MovementAction::MoveAway does the same
    // thing: a failed collision check downgrades how it moves rather than whether it moves
    uint8 const sweeps = allowUnvalidatedFallback ? 2 : 1;
    for (uint8 sweep = 0; sweep < sweeps; ++sweep)
    {
        bool const validate = (sweep == 0);

        for (uint8 step = 0; step <= fanSteps; ++step)
        {
            float const delta = fanStep * step;
            // Both offsets are the same angle at zero, so only try it once
            uint8 const candidates = (step == 0) ? 1 : 2;
            for (uint8 i = 0; i < candidates; ++i)
            {
                float const angle = preferredAngle + (i == 0 ? delta : -delta);
                float const targetX = centerX + std::cos(angle) * radius;
                float const targetY = centerY + std::sin(angle) * radius;

                // Kept in both sweeps. Whatever a caller rules out is a rule about the fight, not
                // about the ground, and giving up on the ground is no reason to break it
                if (isAcceptable && !isAcceptable(targetX, targetY))
                    continue;

                if (validate)
                {
                    if (!CanTakeStepTowards(bot, targetX, targetY, moveDist, stepX, stepY, stepZ))
                        continue;
                }
                else
                {
                    float const botX = bot->GetPositionX();
                    float const botY = bot->GetPositionY();
                    float const distance = bot->GetExactDist2d(targetX, targetY);

                    constexpr float minStepDistance = 0.5f;
                    if (distance < minStepDistance)
                        continue;

                    float const stepDistance = std::min(moveDist, distance);
                    stepX = botX + ((targetX - botX) / distance) * stepDistance;
                    stepY = botY + ((targetY - botY) / distance) * stepDistance;
                    stepZ = bot->GetPositionZ();
                }

                // The step is where the bot actually stops, and that is not the target whenever the
                // target lies further off than moveDist. A caller's rule has to hold where it comes
                // to rest, not only where it was heading, or a heading cleared on the strength of
                // its endpoint parks the bot somewhere the caller had ruled out
                if (isAcceptable && !isAcceptable(stepX, stepY))
                    continue;

                if (chosenX)
                    *chosenX = targetX;
                if (chosenY)
                    *chosenY = targetY;

                return true;
            }
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

    // Dead centre gives no direction of its own, so take the bot's own facing
    if (bot->GetExactDist2d(centerX, centerY) <= 0.1f)
        escapeAngle = bot->GetOrientation();

    // Standing in a hazard is worse than walking into something, so if nothing validates, go
    // anyway. It is often not futile either: the navmesh holds no gameobjects at all, so a step
    // refused for scenery may still find a path straight through where that scenery stands
    return FindStepToCircle(
        bot, hazard, escapeRadius, escapeAngle, moveDist, stepX, stepY, stepZ, isAcceptable,
        nullptr, nullptr, true);
}

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
    std::vector<Position> const& positions = GetCachedHazardPositions(bot, "hyjal death and decay");
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
    return IsNearDeathAndDecay(bot, DEATH_AND_DECAY_RADIUS);
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

Unit* GetFocusedInfernal(PlayerbotAI* botAI)
{
    // Already alive-filtered and ordered oldest first, so the first one that still resolves wins
    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        if (Unit* infernal = botAI->GetUnit(guid))
            return infernal;
    }

    return nullptr;
}

Unit* GetLooseInfernal(PlayerbotAI* botAI, Player* bot)
{
    // Loose is only meaningful against a tank to be loose from. Without one the comparison below
    // would invert, passing over an Infernal that has taken nobody at all as though it were held
    Player* infernalTank = GetInfernalTank(bot);
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

Unit* GetNearestInfernal(PlayerbotAI* botAI, Player* bot)
{
    Unit* nearest = nullptr;
    float nearestDistance = 0.0f;
    for (ObjectGuid const guid : GetInfernalGuids(botAI))
    {
        Unit* infernal = botAI->GetUnit(guid);
        if (!infernal)
            continue;

        float const distance = bot->GetDistance2d(infernal);
        if (!nearest || distance < nearestDistance)
        {
            nearest = infernal;
            nearestDistance = distance;
        }
    }

    return nearest;
}

Unit* GetInfernalTargetingBot(PlayerbotAI* botAI, Player* bot)
{
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
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && IsInfernalTank(member))
            return member;
    }

    return nullptr;
}

Position const& GetInfernalTankPosition(Player* bot)
{
    // Without a tank there is nobody to converge on, so the asking bot answers for itself
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

float GetDistanceFromGroupCenter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0.0f;

    float x = 0.0f;
    float y = 0.0f;
    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() ||
            member->GetMapId() != bot->GetMapId())
        {
            continue;
        }

        x += member->GetPositionX();
        y += member->GetPositionY();
        ++count;
    }

    if (!count)
        return 0.0f;

    return bot->GetExactDist2d(x / count, y / count);
}

bool IsKazrogalManaUser(PlayerbotAI* botAI, Player* bot)
{
    switch (bot->getClass())
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

// Each Rain of Fire is its own dynamic object that expires after 10s on its own, so nothing has
// to be recorded to know whether one is still active. Azgalor casts on a timer that lets two
// overlap, so callers have to weigh all of them rather than just the nearest
std::vector<Position> GetRainOfFirePositions(Player* bot)
{
    return GetCachedHazardPositions(bot, "hyjal rain of fire");
}

// Fleeing the nearest can walk a bot into a second pool, which then becomes the nearest and is
// fled in turn. That resolves itself a step at a time and is no worse than standing in the first
bool GetNearestRainOfFirePosition(Player* bot, Position& pool)
{
    bool found = false;
    float nearestDistance = 0.0f;
    for (Position const& position : GetCachedHazardPositions(bot, "hyjal rain of fire"))
    {
        float const distance = bot->GetExactDist2d(position);
        if (!found || distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearRainOfFire(Player* bot, float radius)
{
    for (Position const& position : GetCachedHazardPositions(bot, "hyjal rain of fire"))
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

bool IsDoomed(Player* bot)
{
    return bot->HasAura(Id(HyjalSpells::SPELL_DOOM));
}

// Standing behind Azgalor is immune at any range, which is where melee want to be anyway. The
// range clause only matters for anyone who has to pass through his front
bool IsDoomguardTank(PlayerbotAI* botAI, Player* bot)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return true;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
        return false;

    // The second assist tank takes over only once the first is Doomed. A first tank that has died
    // needs no handover: the indices compact, so the second has already become index 0 above
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    return !firstAssistTank || IsDoomed(firstAssistTank);
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

std::vector<Position> GetDoomfirePositions(Player* bot)
{
    return GetCachedHazardPositions(bot, "hyjal doomfire trail");
}

// Centre to centre, as every other hazard test here measures. This used to lean on the grid
// search's own range check, which quietly pads by both object sizes--so the radius handed in
// reached about two yards further than it said, and disagreed with IsPositionNearDoomfire below
bool IsNearDoomfire(Player* bot, float radius)
{
    for (Position const& patch : GetCachedHazardPositions(bot, "hyjal doomfire trail"))
    {
        if (bot->GetExactDist2d(patch) < radius)
            return true;
    }

    return false;
}

bool IsPositionNearDoomfire(Player* bot, float x, float y, float radius)
{
    for (Position const& patch : GetCachedHazardPositions(bot, "hyjal doomfire trail"))
    {
        if (patch.GetExactDist2d(x, y) < radius)
            return true;
    }

    return false;
}

AirBurstData* GetPendingAirBurstCast(uint32 instanceId)
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
