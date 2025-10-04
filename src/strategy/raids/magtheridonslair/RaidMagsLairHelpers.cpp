#include "RaidMagsLairHelpers.h"
#include "AiObjectContext.h"
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
        return nullptr;
    }

    auto bounds = map->GetCreatureBySpawnIdStore().equal_range(dbGuid);
    if (bounds.first == bounds.second)
    {
        return nullptr;
    }

    Creature* creature = bounds.first->second;
    return creature;
}

const TankSpot MagtheridonTankSpot = { -52.391f, 30.373f, -0.406f, 5.246f };

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
        if (!member || !member->IsAlive() || !botAI->IsRangedDps(member, true))
        {
            continue;
        }
        candidates.push_back(member);
    }

    // Second pass: add anyone except Main Tank if still not enough
    if (candidates.size() < cubes.size())
    {
        Player* magtheridonTank = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
            {
                continue;
            }
            if (botAI->IsMainTank(member))
            {
                magtheridonTank = member;
                break;
            }
        }

        for (GroupReference* ref = group->GetFirstMember(); ref && candidates.size() < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == magtheridonTank)
            {
                continue;
            }
            if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                candidates.push_back(member);
        }
    }

    // Assign cubes by GUID and coordinates to living candidates only
    for (Player* member : candidates)
    {
        if (cubeIndex >= cubes.size())
        {
            break;
        }
        if (!member || !member->IsAlive())
        {
            continue;
        }
        botToCubeAssignment[member->GetGUID()] = cubes[cubeIndex++];
    }
}

bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z)
{
    // Debris hazards: NPC 17474, 9 yard radius
    std::vector<Unit*> debrisHazards;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || unit->GetEntry() != 17474)
        {
            continue;
        }
        debrisHazards.push_back(unit);
    }
    for (Unit* hazard : debrisHazards)
    {
        float dist = std::sqrt(std::pow(x - hazard->GetPositionX(), 2) + std::pow(y - hazard->GetPositionY(), 2));
        if (dist < 9.0f)
        {
            return false;
        }
    }

    // Conflagration hazards: GameObject 181832, 5 yard radius
    GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects");
    for (const auto& goGuid : gos)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || go->GetEntry() != 181832)
        {
            continue;
        }
        float dist = std::sqrt(std::pow(x - go->GetPositionX(), 2) + std::pow(y - go->GetPositionY(), 2));
        if (dist < 5.0f)
        {
            return false;
        }
    }

    return true;
}
