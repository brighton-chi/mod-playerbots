/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace MagHelpers
{

std::unordered_map<uint32, uint32> blastNovaTimer;
std::unordered_map<uint32, uint32> dpsWaitTimer;
std::unordered_map<uint32, bool> ceilingCollapseApplied;
std::unordered_map<uint32, bool> lastBlastNovaState;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, CubeInfo>> botToCubeAssignments;

std::vector<uint32> const MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

// Get the positions of all Manticron Cubes by their database GUIDs
std::vector<CubeInfo> GetAllCubeInfosByDbGuids(
    Map* map, std::vector<uint32> const& cubeDbGuids)
{
    std::vector<CubeInfo> cubes;
    if (!map)
        return cubes;

    for (uint32 dbGuid : cubeDbGuids)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
        if (bounds.first == bounds.second)
            continue;

        GameObject* go = bounds.first->second;
        if (!go)
            continue;

        CubeInfo info;
        info.guid = go->GetGUID();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        cubes.push_back(info);
    }

    return cubes;
}

// Identify channelers by their database GUIDs
Creature* GetChanneler(Player* bot, uint32 dbGuid)
{
    Map* map = bot->GetMap();
    if (!map)
        return nullptr;

    auto it = map->GetCreatureBySpawnIdStore().find(dbGuid);
    if (it == map->GetCreatureBySpawnIdStore().end())
        return nullptr;

    Creature* channeler = it->second;
    if (!channeler->IsAlive())
        return nullptr;

    return channeler;
}

bool IsMagtheridonActive(Unit* magtheridon)
{
    return magtheridon && !magtheridon->HasAura(Id(MagSpells::SPELL_SHADOW_CAGE));
}

bool IsBlastNovaCasting(Unit* magtheridon)
{
    return magtheridon && magtheridon->FindCurrentSpellBySpellId(Id(MagSpells::SPELL_BLAST_NOVA));
}

bool IsCubeClicker(Player* bot)
{
    auto mapIt = botToCubeAssignments.find(bot->GetMap()->GetInstanceId());
    return mapIt != botToCubeAssignments.end() &&
        mapIt->second.find(bot->GetGUID()) != mapIt->second.end();
}

bool IsPositionInActiveDebris(Player* bot, float x, float y, float radius)
{
    constexpr float searchRadius = 30.0f;
    std::vector<Position> debrisPositions = GetDynamicObjectPositions(
        bot, searchRadius, Id(MagSpells::SPELL_DEBRIS_SPAWN));
    if (debrisPositions.empty())
        return false;

    return debrisPositions.front().GetExactDist2d(x, y) <= radius;
}

bool IsPositionInActiveConflagration(PlayerbotAI* botAI, float x, float y)
{
    constexpr float conflagrationHazardRadius = 5.0f;
    auto const& gameObjects =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
    for (auto const& goGuid : gameObjects)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !go->isSpawned() || go->GetEntry() != Id(MagObjs::GO_BLAZE))
            continue;

        if (go->GetDistance2d(x, y) < conflagrationHazardRadius)
            return true;
    }

    return false;
}

}
