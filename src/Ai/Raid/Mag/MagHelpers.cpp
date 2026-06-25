#include "MagHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

namespace MagtheridonHelpers
{
    const Position WAITING_FOR_MAGTHERIDON_POSITION = {   1.359f,   2.048f, -0.406f, 3.135f };
    const Position MAGTHERIDON_TANK_POSITION =        {  22.827f,   2.105f, -0.406f, 3.135f };
    const Position NW_CHANNELER_TANK_POSITION =       { -11.764f,  30.818f, -0.411f,   0.0f };
    const Position NE_CHANNELER_TANK_POSITION =       { -12.490f, -26.211f, -0.411f,   0.0f };
    const Position RANGED_SPREAD_POSITION =           { -14.890f,   1.995f, -0.406f,   0.0f };
    const Position HEALER_SPREAD_POSITION =           {  -2.265f,   1.874f, -0.404f,   0.0f };

    std::unordered_map<uint32, time_t> blastNovaTimer;
    std::unordered_map<uint32, time_t> dpsWaitTimer;

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
        return magtheridon &&
        !magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SPELL_SHADOW_CAGE));
    }

    const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

    // Get the positions of all Manticron Cubes by their database GUIDs
    std::vector<CubeInfo> GetAllCubeInfosByDbGuids(
        Map* map, const std::vector<uint32>& cubeDbGuids)
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

    static std::unordered_map<uint32, std::unordered_map<ObjectGuid, CubeInfo>>
        botToCubeAssignments;

    CubeInfo const* GetAssignedCube(Player* bot)
    {
        uint32 const instanceId = bot->GetMap()->GetInstanceId();
        auto mapIt = botToCubeAssignments.find(instanceId);
        if (mapIt == botToCubeAssignments.end())
            return nullptr;

        auto it = mapIt->second.find(bot->GetGUID());
        return it != mapIt->second.end() ? &it->second : nullptr;
    }

    bool IsCubeClicker(Player* bot)
    {
        uint32 const instanceId = bot->GetMap()->GetInstanceId();
        auto mapIt = botToCubeAssignments.find(instanceId);
        return mapIt != botToCubeAssignments.end() &&
               mapIt->second.find(bot->GetGUID()) != mapIt->second.end();
    }

    bool NeedsCubeReassignment(uint32 instanceId)
    {
        auto mapIt = botToCubeAssignments.find(instanceId);
        if (mapIt == botToCubeAssignments.end() || mapIt->second.empty())
            return true;

        for (auto const& pair : mapIt->second)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
                return true;
        }

        return false;
    }

    void AssignCubeClickers(Group* group, Map* map, PlayerbotAI* botAI)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(map, MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI, map->GetInstanceId());
    }

    void RemoveCubeClicker(Player* bot)
    {
        uint32 const instanceId = bot->GetMap()->GetInstanceId();
        auto mapIt = botToCubeAssignments.find(instanceId);
        if (mapIt != botToCubeAssignments.end())
            mapIt->second.erase(bot->GetGUID());
    }

    void AssignBotsToCubesByGuidAndCoords(
        Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI, uint32 instanceId)
    {
        auto& assignment = botToCubeAssignments[instanceId];
        assignment.clear();
        if (!group)
            return;

        size_t cubeIndex = 0;
        std::vector<Player*> candidates;

        // Assign ranged DPS (excluding Warlocks) to cubes first
        for (GroupReference* ref = group->GetFirstMember(); ref && cubeIndex < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRangedDps(member, true) ||
                member->getClass() == CLASS_WARLOCK || !GET_PLAYERBOT_AI(member))
                continue;

            candidates.push_back(member);
            if (candidates.size() >= cubes.size())
                break;
        }

        // If there are still cubes left, assign any other non-tank bots
        if (candidates.size() < cubes.size())
        {
            for (GroupReference* ref = group->GetFirstMember();
                ref && candidates.size() < cubes.size(); ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
                    continue;

                if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                    candidates.push_back(member);
            }
        }

        for (Player* member : candidates)
        {
            if (cubeIndex >= cubes.size())
                break;

            if (!member || !member->IsAlive())
                continue;

            assignment[member->GetGUID()] = cubes[cubeIndex++];
        }
    }

    bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y)
    {
        // Debris
        std::list<Creature*> debrisList;
        constexpr float searchRadius = 40.0f;
        constexpr float debrisHazardRadius = 9.0f;
        bot->GetCreatureListWithEntryInGrid(debrisList, NPC_TARGET_TRIGGER, searchRadius);

        for (Creature* creature : debrisList)
        {
            if (creature && creature->GetDistance2d(x, y) < debrisHazardRadius)
                return false;
        }

        // Conflagration
        constexpr float conflagrationHazardRadius = 5.0f;
        GuidVector const& gameObjects =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
        for (auto const& goGuid : gameObjects)
        {
            GameObject* go = botAI->GetGameObject(goGuid);
            if (!go || !go->isSpawned() ||
                go->GetEntry() !=
                    static_cast<uint32>(MagtheridonHelpers::MagtheridonObjects::GO_BLAZE))
            {
                continue;
            }

            if (go->GetDistance2d(x, y) < conflagrationHazardRadius)
                return false;
        }

        return true;
    }
}
