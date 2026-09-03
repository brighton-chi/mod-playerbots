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

// Called only from FindSafeStepInZone, in this file. Internal linkage rather than a ZaHelpers
// declaration: nothing outside ZA would include this header to reach it, so exporting it bought
// nothing, and the definition has to precede that first use anyway.
namespace
{

bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    // Exact, to match the caller's danger test. GetDistance2d would subtract this hazard's combat
    // reach here and both the hazard's and the bot's there, leaving the two thresholds a bot reach
    // apart - see JANALAI_FIRE_BOMB_SAFE_DISTANCE.
    for (Unit* hazard : hazards)
    {
        if (hazard->GetExactDist2d(x, y) < hazardRadius)
            return false;
    }

    return true;
}

}  // namespace

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

bool FindSafeStepInZone(Player* bot,
    std::vector<Unit*> const& hazards, SafeZoneQuad const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float distanceStep = 1.0f;

    // Only where the step is aimed is checked, never where it lands. The quad is convex and every
    // point in it is walkable, so the line between any two of its points is walkable too - and the
    // hazards here are not worth testing on the way, only on arrival. Jan'alai's bombs do nothing
    // until they all detonate at once, so crossing them is free, and testing the landing against
    // them left bots standing where they were: the gaps are rarely within one step of a bot that
    // needs one.
    //
    // Rings are walked nearest first and every candidate within a ring is the same distance out,
    // so the first one that validates is the shortest move on offer.
    for (float distance = distanceStep;
            distance <= maxSearchDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            // Quad first: it is arithmetic, where the hazard sweep loops over every bomb.
            if (!safeZone.Contains(x, y))
                continue;

            if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                continue;

            if (!EncounterHelpers::CanTakeStepTowards(bot, x, y, moveDist, stepX, stepY, stepZ))
                continue;

            return true;
        }
    }

    return false;
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

ObjectGuid FindNearbyFreezingTrapGuid(Player* bot)
{
    // The instance strategy can outlive leaving the instance (e.g. after a server reset), so the
    // grid search is gated on the map rather than run wherever the bot happens to be.
    if (bot->GetMapId() != ZA_MAP_ID)
        return ObjectGuid::Empty;

    GameObject* trap = bot->FindNearestGameObject(
        Id(ZaObjects::GO_FREEZING_TRAP), ZA_FREEZING_TRAP_SEARCH_RADIUS, true);

    return trap ? trap->GetGUID() : ObjectGuid::Empty;
}

GameObject* GetNearbyFreezingTrap(PlayerbotAI* botAI)
{
    ObjectGuid const guid = botAI->GetAiObjectContext()
        ->GetValue<ObjectGuid>("hex lord malacrass freezing trap")->Get();

    return guid.IsEmpty() ? nullptr : botAI->GetGameObject(guid);
}

GuidVector FindNearbyFireBombGuids(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(ZaNpcs::NPC_FIRE_BOMB), JANALAI_FIRE_BOMB_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    return guids;
}

std::vector<Unit*> GetNearbyFireBombs(PlayerbotAI* botAI)
{
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("jan'alai fire bombs")->RefGet();

    std::vector<Unit*> bombs;
    bombs.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive())
            bombs.push_back(unit);
    }

    return bombs;
}

bool IsJanalaiBombing(Unit* janalai)
{
    return janalai && janalai->HasAura(Id(ZaSpells::SPELL_FIRE_BOMB_CHANNEL));
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
