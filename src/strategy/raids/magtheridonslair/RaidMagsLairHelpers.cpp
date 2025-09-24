#include "RaidMagsLairHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"

#include "Log.h"

Creature* GetChanneler(Player* bot, uint32 dbGuid)
{
    Map* map = bot->GetMap();
    if (!map)
    {
        LOG_DEBUG("playerbots", "GetChanneler: map is nullptr (bot={})", bot->GetName());
        return nullptr;
    }

    auto bounds = map->GetCreatureBySpawnIdStore().equal_range(dbGuid);
    if (bounds.first == bounds.second)
    {
        LOG_DEBUG("playerbots", "GetChanneler: dbGuid={} not found in GetCreatureBySpawnIdStore (bot={})", dbGuid, bot->GetName());
        return nullptr;
    }

    Creature* creature = bounds.first->second;
    LOG_DEBUG("playerbots", "GetChanneler: dbGuid={} found={} (bot={})", dbGuid, creature ? "yes" : "no", bot->GetName());
    return creature;
}

bool IsSouthTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            return member == bot;
        }
    }
    return false;
}

bool IsWestTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 1)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

bool IsEastTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 2)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

/* bool IsSouthWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
            return member == bot;
    }
    return false;
}

bool IsWestWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    int warlockIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
        {
            if (warlockIndex == 1)
                return member == bot;
            ++warlockIndex;
        }
    }
    return false;
}

bool IsEastWarlock(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
        return false;

    int warlockIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_WARLOCK)
        {
            if (warlockIndex == 2)
                return member == bot;
            ++warlockIndex;
        }
    }
    return false;
} */

/* bool IsWestHunter(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_HUNTER)
            return member == bot;
    }
    return false;
}

bool IsEastHunter(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

    int hunterIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (member->getClass() == CLASS_HUNTER)
        {
            if (hunterIndex == 1)
                return member == bot;
            ++hunterIndex;
        }
    }
    return false;
} */

// const TankSpot MagtheridonTankSpot = { -46.834f, -17.760f, -0.412f, 3.333f };
const TankSpot MagtheridonTankSpot = { -51.681f, 26.805f, -0.408f, 5.528f };

const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, const std::vector<uint32>& cubeDbGuids)
{
    std::vector<CubeInfo> cubes;
    if (!map)
    {
        LOG_DEBUG("playerbots", "GetAllCubeInfosByDbGuids: No map provided");
        return cubes;
    }

    for (uint32 dbGuid : cubeDbGuids)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
        if (bounds.first == bounds.second)
        {
            LOG_DEBUG("playerbots", "GetAllCubeInfosByDbGuids: DB GUID {} not found in instance", dbGuid);
            continue;
        }

        GameObject* go = bounds.first->second;
        if (!go)
        {
            LOG_DEBUG("playerbots", "GetAllCubeInfosByDbGuids: DB GUID {} found but GameObject is nullptr", dbGuid);
            continue;
        }

        CubeInfo info;
        info.guid = go->GetGUID();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        LOG_DEBUG("playerbots", "GetAllCubeInfosByDbGuids: Found cube DB GUID {} at ({}, {}, {})", dbGuid, info.x, info.y, info.z);
        cubes.push_back(info);
    }
    LOG_DEBUG("playerbots", "GetAllCubeInfosByDbGuids: Returning {} cubes", cubes.size());
    return cubes;
}

std::unordered_map<ObjectGuid, CubeInfo> botToCubeAssignment;

void AssignBotsToCubesByGuidAndCoords(Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI)
{
    botToCubeAssignment.clear();
    size_t cubeIndex = 0;
    std::vector<Player*> candidates;

    // First pass: ranged DPS, excluding warlocks
    for (GroupReference* ref = group->GetFirstMember(); ref && cubeIndex < cubes.size(); ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (!botAI->IsRangedDps(member, true) || member->getClass() == CLASS_WARLOCK)
            continue;
        candidates.push_back(member);
    }

    // Second pass: add ranged DPS warlocks if needed
    if (candidates.size() < cubes.size())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref && candidates.size() < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;
            if (!botAI->IsRangedDps(member, true) || member->getClass() != CLASS_WARLOCK)
                continue;
            if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                candidates.push_back(member);
        }
    }

    // Third pass: add anyone except SouthTank if still not enough
    if (candidates.size() < cubes.size())
    {
        Player* southTank = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;
            if (IsSouthTank(botAI, member))
            {
                southTank = member;
                break;
            }
        }

        for (GroupReference* ref = group->GetFirstMember(); ref && candidates.size() < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;
            if (member == southTank)
                continue;
            if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                candidates.push_back(member);
        }
    }

    // Assign cubes by GUID and coordinates
    for (Player* member : candidates)
    {
        if (cubeIndex >= cubes.size())
            break;
        botToCubeAssignment[member->GetGUID()] = cubes[cubeIndex++];
    }
}

/* bool IsLocationSafe(float x, float y, float z, Map* map)
{
    // Check Blaze hazards
    for (auto* blaze : GetAllGameObjects(map, 181832))
    {
        if (blaze->IsSpawned() && blaze->IsAlive())
        {
            if (blaze->GetExactDist2d(x, y) < 5.0f)
                return false;
        }
    }
    // Check Debris hazards
    for (auto* debris : GetAllCreatures(map, 17474))
    {
        if (debris->IsAlive())
        {
            if (debris->GetExactDist2d(x, y) < 4.0f)
                return false;
        }
    }
    return true;
} */
