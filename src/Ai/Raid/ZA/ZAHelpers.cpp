/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAHelpers.h"
#include "EncounterHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <mutex>
#include <string_view>
#include <unordered_map>

using namespace EncounterHelpers;

namespace ZaHelpers
{

namespace
{

bool IsInsideSafeZone(std::vector<Position> const& corners, float x, float y)
{
    // The safe zone's shape is pentagonal, sort of like a rectangle with one of the corners cut
    // out (to exclude a fenced-off broken corner of the stage).
    size_t const count = corners.size();
    int8 insideSign = 0;
    for (size_t i = 0; i < count; ++i)
    {
        Position const& edgeStart = corners[i];
        Position const& edgeEnd = corners[(i + 1) % count];

        float cross =
            (edgeEnd.GetPositionX() - edgeStart.GetPositionX()) * (y - edgeStart.GetPositionY()) -
            (edgeEnd.GetPositionY() - edgeStart.GetPositionY()) * (x - edgeStart.GetPositionX());

        if (cross == 0.0f)
            continue;

        int8 const sign = cross > 0.0f ? 1 : -1;
        if (insideSign == 0)
            insideSign = sign;
        else if (insideSign != sign)
            return false;
    }

    return true;
}

bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        if (hazard->GetExactDist2d(x, y) < hazardRadius)
            return false;
    }

    return true;
}

}  // end anonymous namespace

// Shared

namespace
{

std::mutex zaStateMutex;
std::unordered_map<uint32, ZaInstanceState> zaStates;

constexpr std::string_view BossName(ZaBosses boss)
{
    switch (boss)
    {
        case ZaBosses::DATA_AKILZON:
            return "akil'zon";
        case ZaBosses::DATA_NALORAKK:
            return "nalorakk";
        case ZaBosses::DATA_JANALAI:
            return "jan'alai";
        case ZaBosses::DATA_HALAZZI:
            return "halazzi";
        case ZaBosses::DATA_HEX_LORD_MALACRASS:
            return "hex lord malacrass";
        case ZaBosses::DATA_ZULJIN:
            return "zul'jin";
        default:
            return {};
    }
}

}

Unit* GetZaBoss(Player* bot, ZaBosses boss)
{
    if (!IsBossInProgress(bot, ZA_MAP_ID, Id(boss)))
        return nullptr;

    return FindThreatTarget(bot, BossName(boss));
}

// Akil'zon <Eagle Avatar>

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

GuidVector FindJanalaiAddGuids(Player* bot)
{
    std::vector<uint32> const entries = {
        Id(ZaNpcs::NPC_AMANISHI_HATCHER),
        Id(ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING),
    };

    return FindCreatureGuidsAround(bot, JANALAI_TANK_POSITION, JANALAI_ADD_SEARCH_RADIUS, entries);
}

std::vector<Unit*> GetJanalaiAdds(Player* bot, ZaNpcs npc)
{
    GuidVector const& guids = GetInstanceCreatureList(
        ZaState(bot->GetInstanceId()).janalaiAdds, JANALAI_ADD_CACHE_INTERVAL_MS, bot,
        FindJanalaiAddGuids);

    std::vector<Unit*> adds;
    for (ObjectGuid const& guid : guids)
    {
        if (guid.GetEntry() != Id(npc))
            continue;

        Creature* add = ObjectAccessor::GetCreature(*bot, guid);
        if (add && add->IsAlive() && add->IsInWorld())
            adds.push_back(add);
    }

    return adds;
}

std::pair<Unit*, Unit*> GetAmanishiHatcherPair(Player* bot)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    for (Unit* unit : GetJanalaiAdds(bot, ZaNpcs::NPC_AMANISHI_HATCHER))
    {
        if (!lowest || unit->GetGUID().GetCounter() < lowest->GetGUID().GetCounter())
            lowest = unit;

        if (!highest || unit->GetGUID().GetCounter() > highest->GetGUID().GetCounter())
            highest = unit;
    }

    return {lowest, highest};
}

uint32 CountJanalaiHatchlingsByEntry(Player* bot)
{
    return static_cast<uint32>(
        GetJanalaiAdds(bot, ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING).size());
}

// Jan'alai casts Fire Bomb Channel at the beginning of each Fire Bomb event until bombs detonate.
bool IsJanalaiBombing(Unit* janalai)
{
    return janalai && janalai->HasAura(Id(ZaSpells::SPELL_FIRE_BOMB_CHANNEL));
}

GuidVector FindFireBombGuids(Player* bot)
{
    return FindCreatureGuidsAround(
        bot, JANALAI_TANK_POSITION, JANALAI_FIRE_BOMB_SEARCH_RADIUS, { Id(ZaNpcs::NPC_FIRE_BOMB) });
}

std::vector<Unit*> GetFireBombs(Player* bot)
{
    GuidVector const& guids = GetInstanceCreatureList(
        ZaState(bot->GetInstanceId()).janalaiFireBombs, FIRE_BOMB_CACHE_INTERVAL_MS, bot,
        FindFireBombGuids);

    std::vector<Unit*> bombs;
    bombs.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Creature* bomb = ObjectAccessor::GetCreature(*bot, guid);
        if (bomb && bomb->IsAlive())
            bombs.push_back(bomb);
    }

    return bombs;
}

// Shortest move to a position that is safe from Fire Bombs, within the measured area.
bool FindSafeStepInJanalaiZone(
    Player* bot, std::vector<Unit*> const& hazards, std::vector<Position> const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    constexpr uint8 angleCount = 16;
    constexpr float angleStep = 2.0f * M_PI / angleCount;
    constexpr float distanceStep = 1.0f;

    uint32 const ringCount = static_cast<uint32>(maxSearchDistance / distanceStep);

    for (uint32 ring = 1; ring <= ringCount; ++ring)
    {
        float const distance = ring * distanceStep;
        for (uint8 i = 0; i < angleCount; ++i)
        {
            float const angle = i * angleStep;
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (!IsInsideSafeZone(safeZone, x, y))
                continue;

            if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                continue;

            if (!CanTakeStepTowards(bot, x, y, moveDist, stepX, stepY, stepZ))
                continue;

            return true;
        }
    }

    return false;
}

// Halazzi <Lynx Avatar>

GuidVector FindCorruptedLightningTotemGuids(Player* bot)
{
    return FindCreatureGuidsAround(
        bot, HALAZZI_TANK_POSITION, HALAZZI_TOTEM_SEARCH_RADIUS,
        { Id(ZaNpcs::NPC_CORRUPTED_LIGHTNING_TOTEM) });
}

Creature* GetNearestCorruptedLightningTotem(Player* bot)
{
    GuidVector const& guids = GetInstanceCreatureList(
        ZaState(bot->GetInstanceId()).halazziTotems, HALAZZI_TOTEM_CACHE_INTERVAL_MS, bot,
        FindCorruptedLightningTotemGuids);

    Creature* nearest = nullptr;
    float nearestDistance = ZA_CREATURE_SEARCH_RADIUS;
    for (ObjectGuid const& guid : guids)
    {
        Creature* totem = ObjectAccessor::GetCreature(*bot, guid);
        if (!totem || !totem->IsAlive())
            continue;

        float const distance = bot->GetDistance(totem);
        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearest = totem;
        }
    }

    return nearest;
}

// Hex Lord Malacrass

ObjectGuid FindNearbyFreezingTrapGuid(Player* bot)
{
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

GuidVector FindHexLordMalacrassDpsTargetGuids(Player* bot)
{
    std::vector<uint32> entries;
    entries.reserve(HEX_LORD_MALACRASS_DPS_PRIORITY.size());
    for (ZaNpcs const npc : HEX_LORD_MALACRASS_DPS_PRIORITY)
        entries.push_back(Id(npc));

    return FindCreatureGuidsAround(
        bot, HEX_LORD_MALACRASS_SPAWN_POSITION, HEX_LORD_MALACRASS_DPS_TARGET_SEARCH_RADIUS,
        entries);
}

// Zul'jin

bool GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex)
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
        if (!member || !bot->IsInMap(member) || !PlayerbotAI::IsRanged(member))
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

    size_t const ordinal =
        healers.size() + static_cast<size_t>(std::distance(rangedDps.begin(), dpsIt));
    slotIndex = ordinal % slotCount;
    return true;
}

Player* GetZuljinCreepingParalysisDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(ZaSpells::SPELL_CREEPING_PARALYSIS)))
            continue;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

// Shared encounter state

ZaInstanceState& ZaState(uint32 instanceId)
{
    std::lock_guard lock(zaStateMutex);
    return zaStates[instanceId];
}

bool ZaResetInstance(uint32 instanceId)
{
    std::lock_guard lock(zaStateMutex);
    auto it = zaStates.find(instanceId);
    if (it == zaStates.end())
        return false;

    bool const wasSet = it->second.akilzonStormTimer.has_value();
    zaStates.erase(it);
    return wasSet;
}

}
