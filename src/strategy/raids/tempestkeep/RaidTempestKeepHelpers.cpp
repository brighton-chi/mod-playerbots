#include <algorithm>
#include <map>

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"


namespace TempestKeepHelpers
{

    namespace TempestKeepLocations
    {
        // Al'ar platform coordinates correspond with "OLDWorld Trigger (DO NOT DELETE) NPCs (15384)"
        const Location AlarRoomCenter = { 331.0f, 0.01f, -2.38f };
        const Location AlarPlatform1 = { 335.638f, 59.4879f, 17.9319f }; // West Platform
        const Location AlarPlatform2 = { 388.751f, 31.7312f, 20.2636f }; // Northwest Platform
        const Location AlarPlatform3 = { 388.791f, -33.1059f, 20.2636f }; // Northeast Platform
        const Location AlarPlatform4 = { 332.723f, -61.159f, 17.9791f }; // East Platform
        const Location AlarGround1 = { 336.439f, 48.181f, -2.389f }; // Landing point for jumping from West Platform
        const Location AlarGround2 = { 379.122f, 25.146f, -2.385f }; // Landing point for jumping from Northwest Platform
        const Location AlarGround3 = { 378.583f, -27.481f, -2.385f }; // Landing point for jumping from Northeast Platform
        const Location AlarGround4 = { 331.631f, -49.716f, -2.389f }; // Landing point for jumping from East Platform

        const Location VoidReaverTankPosition   = { 423.845f,  371.733f, 14.897f }; // middle of room

        // const Location AstromancerStackPosition = { 448.398f, -346.526f, 16.802f }; // directly from entrance, on platform, <25 yards from boss
    }

    /*
    // From boss_alar.cpp
    const Position alarPoints[9] =
    {
        {335.638f, 59.4879f, 17.9319f, 4.60f}, //first platform
        {388.751007f, 31.731199f, 20.263599f, 1.61f},
        {388.790985f, -33.105900f, 20.263599f, 0.52f},
        {332.722992f, -61.159f, 17.979099f, 5.71f},
        {258.959015f, -38.687099f, 20.262899f, 5.21f}, //pre-nerf only
        {259.2277997, 35.879002f, 20.263f, 4.81f}, //pre-nerf only
        {332.0f, 0.01f, 43.0f, 0.0f}, //quill
        {331.0f, 0.01f, -2.38f, 0.0f}, //middle (p2)
        {332.0f, 0.01f, 43.0f, 0.0f} // dive
    };
    */

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        if (!target)
            return;

        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentGuid = group->GetTargetIcon(iconId);
            if (currentGuid != target->GetGUID())
            {
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
            }
        }
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
    }

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
        if (!target)
            return;

        std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

        if (currentRti != rtiName || currentTarget != target)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
        }
    }

    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && !botAI->IsTank(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return true;
    }

    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
    {
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (const auto& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);

            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }

        return nullptr;
    }

    Unit* GetNearestPlayerInRadius(Player* bot, float radius)
    {
        Unit* nearestPlayer = nullptr;
        float nearestDistance = radius;
        
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->GetSource();

                if (!member || !member->IsAlive() || member == bot)
                    continue;

                float distance = bot->GetExactDist2d(member);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestPlayer = member;
                }
            }
        }

        return nearestPlayer;
    }

    std::unordered_map<uint32, int8> lastAlarPlatform;
    std::unordered_map<uint32, bool> postFlameQuillsState;
    std::unordered_map<uint32, int8> postFlameQuillsAlarPlatform;
    std::unordered_map<uint32, bool> postQuillsPhase;
    std::unordered_map<uint32, bool> lastRebirthState;

    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot)
    {
        return botAI->IsTank(bot) &&
               !botAI->IsMainTank(bot) &&
               !botAI->IsAssistTankOfIndex(bot, 0);
    }

    void UpdateAlarLastPlatform(Unit* alar, uint32 mapId, const std::vector<Location>& platforms)
    {
        // Find which platform Al'ar is closest to
        int8 closestIndex = -1;
        float minDist = std::numeric_limits<float>::max();
        for (size_t i = 0; i < platforms.size(); ++i)
        {
            float dist = alar->GetExactDist2d(platforms[i].x, platforms[i].y);
            if (dist < minDist)
            {
                minDist = dist;
                closestIndex = static_cast<int>(i);
            }
        }

        // If within threshold (e.g., 5 yards), update last platform
        if (minDist < 5.0f && closestIndex != -1)
            lastAlarPlatform[mapId] = closestIndex;
    }

    std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

}
