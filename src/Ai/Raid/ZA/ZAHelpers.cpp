/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace ZaHelpers
{

// General
bool SafeZoneQuad::Contains(float x, float y) const
{
    // The point is inside a convex quad when it falls on the same side of all four edges.
    int8 insideSign = 0;
    for (uint8 i = 0; i < 4; ++i)
    {
        Position const& edgeStart = corners[i];
        Position const& edgeEnd = corners[(i + 1) % 4];

        float cross =
            (edgeEnd.GetPositionX() - edgeStart.GetPositionX()) * (y - edgeStart.GetPositionY()) -
            (edgeEnd.GetPositionY() - edgeStart.GetPositionY()) * (x - edgeStart.GetPositionX());

        if (cross == 0.0f)
            continue;  // Exactly on this edge, so it rules nothing out.

        int8 const sign = cross > 0.0f ? 1 : -1;
        if (insideSign == 0)
            insideSign = sign;
        else if (insideSign != sign)
            return false;
    }

    return true;
}

bool IsOnFlatFloor(Player* bot, float x, float y, float floorZ, float floorTolerance)
{
    // Searches downward from floorZ, so a spot over a hole reports the floor far below and a spot
    // over nothing at all reports INVALID_HEIGHT. Both miss the tolerance by a wide margin.
    float const groundZ = bot->GetMapHeight(x, y, floorZ);

    return std::fabs(groundZ - floorZ) <= floorTolerance;
}

bool FindSafeStepInZone(Player* bot,
    std::vector<Unit*> const& hazards, SafeZoneQuad const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float distanceStep = 1.0f;

    // Cheapest first: the quad is arithmetic, the hazard sweep is a loop over every bomb, and the
    // floor probe reads the map, so it only runs on what survives the other two.
    auto const isAcceptable = [bot, &hazards, &safeZone, hazardRadius](float x, float y)
    {
        return safeZone.Contains(x, y) &&
            IsPositionSafeFromHazards(x, y, hazards, hazardRadius) &&
            IsOnFlatFloor(bot, x, y, safeZone.floorZ, safeZone.floorTolerance);
    };

    // Rings are walked nearest first and every candidate within a ring is the same distance out,
    // so the first one that validates is the shortest move on offer.
    for (float distance = distanceStep;
            distance <= maxSearchDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (!isAcceptable(x, y))
                continue;

            if (!EncounterHelpers::CanTakeStepTowards(bot, x, y, moveDist, stepX, stepY, stepZ))
                continue;

            // The step stops short whenever the spot is further out than moveDist, and the line to
            // it can cross ground the spot itself never touches, so where it lands is checked too.
            if (!isAcceptable(stepX, stepY))
                continue;

            return true;
        }
    }

    return false;
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

bool GetSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex)
{
    if (slotCount == 0)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != ZA_MAP_ID || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    auto const healerIt = std::find(healers.begin(), healers.end(), bot);
    if (healerIt != healers.end())
    {
        slotIndex = static_cast<size_t>(std::distance(healers.begin(), healerIt)) % slotCount;
        return true;
    }

    auto const dpsIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
    if (dpsIt == rangedDps.end())
        return false;

    // Healers occupy the head of the list, so the dps ordinal picks up where they left off.
    size_t const ordinal =
        healers.size() + static_cast<size_t>(std::distance(rangedDps.begin(), dpsIt));
    slotIndex = ordinal % slotCount;
    return true;
}

uint32 CountAttackersByEntry(PlayerbotAI* botAI, uint32 entry)
{
    uint32 count = 0;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& targetGuid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            ++count;
    }

    return count;
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

std::vector<Unit*> GetNearbyFireBombs(Player* bot)
{
    std::vector<Unit*> bombs;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(ZaNpcs::NPC_FIRE_BOMB), JANALAI_FIRE_BOMB_SEARCH_RADIUS);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            bombs.push_back(creature);
    }

    return bombs;
}

bool HasFireBombNearby(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(ZaNpcs::NPC_FIRE_BOMB), JANALAI_FIRE_BOMB_SEARCH_RADIUS);

    return std::any_of(creatureList.begin(), creatureList.end(),
        [](Creature* creature) { return creature && creature->IsAlive(); });
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
