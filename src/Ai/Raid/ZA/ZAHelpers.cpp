/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAHelpers.h"
#include "Playerbots.h"
#include <cmath>
#include <limits>
#include <list>

namespace ZaHelpers
{

// General
Position FindSafestNearbyPosition(Player* bot,
    std::vector<Unit*> const& hazards, const Position& safeZoneCenter,
    float safeZoneRadius, float hazardRadius, bool requireSafePath)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = 0.0f;
            distance <= safeZoneRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);

            if (safeZoneCenter.GetExactDist2d(x, y) > safeZoneRadius)
                continue;

            if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                continue;

            Position testPos(x, y, bot->GetPositionZ());

            bool pathSafe = true;
            if (requireSafePath)
            {
                pathSafe =
                    IsPathSafeFromHazards(bot->GetPosition(), testPos, hazards, hazardRadius);
                if (!pathSafe)
                    continue;
            }

            float moveDistance = bot->GetExactDist2d(x, y);
            if (!foundSafe || moveDistance < minMoveDistance)
            {
                bestPos = testPos;
                minMoveDistance = moveDistance;
                foundSafe = pathSafe;
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool IsPathSafeFromHazards(Position const& start, Position const& end,
    std::vector<Unit*> const& hazards, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        if (!IsPositionSafeFromHazards(checkX, checkY, hazards, hazardRadius))
            return false;
    }

    return true;
}

bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        if (hazard->GetDistance2d(x, y) < hazardRadius)
            return false;
    }

    return true;
}

std::vector<Unit*> GetAllHazardTriggers(Player* bot, uint32 entry, float searchRadius)
{
    std::vector<Unit*> triggers;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(creatureList, entry, searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            triggers.push_back(creature);
    }

    return triggers;
}

// Akil'zon <Eagle Avatar>

std::unordered_map<uint32, uint32> akilzonStormTimer;

bool IsInStormWindow(uint32 startMs)
{
    uint32 const elapsed = GetMSTimeDiffToNow(startMs);
    if (elapsed < AKILZON_STORM_PERIOD_MS - AKILZON_STORM_LEAD_MS)
        return false;

    uint32 const phase = (elapsed + AKILZON_STORM_LEAD_MS) % AKILZON_STORM_PERIOD_MS;
    return phase < AKILZON_STORM_LEAD_MS + AKILZON_STORM_DURATION_MS;
}

Player* GetElectricalStormTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(ZaSpells::SPELL_ELECTRICAL_STORM)))
            return member;
    }

    return nullptr;
}

// Nalorakk <Bear Avatar>

bool IsNalorakkInBearForm(Unit* nalorakk)
{
    return nalorakk && nalorakk->HasAura(Id(ZaSpells::SPELL_BEARFORM));
}

// Jan'alai <Dragonhawk Avatar>

bool HasFireBombNearby(Player* bot)
{
    constexpr float searchRadius = 30.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(creatureList, Id(ZaNpcs::NPC_FIRE_BOMB), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            return true;
    }

    return false;
}

std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& targetGuid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (unit && unit->GetEntry() == Id(ZaNpcs::NPC_AMANISHI_HATCHER))
        {
            if (!lowest || unit->GetGUID().GetCounter() < lowest->GetGUID().GetCounter())
                lowest = unit;

            if (!highest || unit->GetGUID().GetCounter() > highest->GetGUID().GetCounter())
                highest = unit;
        }
    }

    return {lowest, highest};
}

// Halazzi <Lynx Avatar>
// N/A

// Hex Lord Malacrass
// N/A

// Zul'jin
// N/A

}
