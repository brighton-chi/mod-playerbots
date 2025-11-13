#include <algorithm>
#include <map>

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"


namespace TempestKeepHelpers
{

    namespace TempestKeepPositions
    {
        // Al'ar platform coordinates correspond with "OLDWorld Trigger (DO NOT DELETE) NPCs (15384)"
        const Position AlarRangedCenter(346.758f, 3.794f, -2.389f);
        const Position AlarPlatform1(335.638f, 59.4879f, 17.9319f); // West Platform
        const Position AlarPlatform2(388.751f, 31.7312f, 20.2636f); // Northwest Platform
        const Position AlarPlatform3(388.791f, -33.1059f, 20.2636f); // Northeast Platform
        const Position AlarPlatform4(332.723f, -61.159f, 17.9791f); // East Platform
        const Position AlarPlatform1To2MidpointA(362.264f, 83.648f, 19.797f); // 1st Midpoint between Platform 1 and 2
        const Position AlarPlatform1To2MidpointB(397.760f, 57.362f, 20.179f); // 2nd Midpoint between Platform 1 and 2
        const Position AlarPlatform2To3MidpointA(419.272f, 28.838f, 20.179f); // 1st Midpoint between Platform 2 and 3
        const Position AlarPlatform2To3MidpointB(419.272f, -28.838f, 20.179f); // 2nd Midpoint between Platform 2 and 3
        const Position AlarPlatform3To4MidpointA(397.760f, -57.362f, 20.179f); // 1st Midpoint between Platform 3 and 4
        const Position AlarPlatform3To4MidpointB(362.264f, -83.648f, 19.797f); // 2nd Midpoint between Platform 3 and 4
        const Position AlarGround1(336.439f, 48.181f, -2.389f); // Landing point for jumping from West Platform
        const Position AlarGround2(379.122f, 25.146f, -2.385f); // Landing point for jumping from Northwest Platform
        const Position AlarGround3(378.583f, -27.481f, -2.385f); // Landing point for jumping from Northeast Platform
        const Position AlarGround4(331.631f, -49.716f, -2.389f); // Landing point for jumping from East Platform
        const Position AlarSERampBase(281.064f, -36.590f, -2.389f);
        const Position AlarMidpointToSERamp(337.099f, -6.412f, -2.389f);
        const Position AlarSWRampBase(281.064f, 36.590f, -2.389f);
        const Position AlarMidpointToSWRamp(337.099f, 6.412f, -2.389f);
        const Position AlarRoomSouthCenter(281.064f, 0.0f, -2.389f);

        const Position VoidReaverTankPosition(423.845f,  371.733f, 14.897f); // middle of room

        // const Position AstromancerStackPosition = { 448.398f, -346.526f, 16.802f }; // directly from entrance, on platform, <25 yards from boss

        const Position ThaladredTankPosition = { 0.0f, 0.0f, 0.0f };
        const Position SanguinarTankPosition = { 0.0f, 0.0f, 0.0f };
        const Position CapernianTankPosition = { 0.0f, 0.0f, 0.0f };
        const Position TelonicusTankPosition = { 0.0f, 0.0f, 0.0f };
    }

    /*
    // From boss_alar.cpp, for reference while I work on this code
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
                group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
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

    void MarkTargetWithCircle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
    }

    void MarkTargetWithTriangle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
    }

    void MarkTargetWithDiamond(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
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
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
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

    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot)
    {
        return botAI->IsTank(bot) &&
               !botAI->IsMainTank(bot) &&
               !botAI->IsAssistTankOfIndex(bot, 0);
    }

    void UpdateAlarLastPlatform(Unit* alar, uint32 mapId, const std::vector<Position>& platforms)
    {
        int8 previousIndex = lastAlarPlatform.count(mapId) ? lastAlarPlatform[mapId] : -1;
        int8 closestIndex = -1;
        float minDist = std::numeric_limits<float>::max();
        for (size_t i = 0; i < platforms.size(); ++i)
        {
            float dist = alar->GetExactDist2d(platforms[i].GetPositionX(), platforms[i].GetPositionY());
            if (dist < minDist)
            {
                minDist = dist;
                closestIndex = static_cast<int>(i);
            }
        }

        if (closestIndex != -1 && closestIndex != previousIndex)
        {
            LOG_DEBUG("playerbots", "Al'ar platform switch: {} -> {} (mapId {})", previousIndex, closestIndex, mapId);
            lastAlarPlatform[mapId] = closestIndex;
        }
        else if (closestIndex != -1)
        {
            lastAlarPlatform[mapId] = closestIndex;
        }
    }

    Player* GetCapernianTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* mainTankCandidate = nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member)->HasStrategy("tank", BotState::BOT_STATE_COMBAT))
                return member;

            if (!meleeTankCandidate && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 2))
                meleeTankCandidate = member;
        }

        return meleeTankCandidate;
    }

    std::unordered_map<uint32, int8> lastAlarPlatform;
    std::unordered_map<uint32, bool> lastRebirthState;
    std::unordered_map<uint32, bool> isPhase2;
    // std::unordered_map<ObjectGuid, int8> lastMainTankPlatform;
    // std::unordered_map<ObjectGuid, int8> lastAssistTankPlatform;
    std::unordered_map<ObjectGuid, bool> mainTankAtPlatform2;
    std::unordered_map<ObjectGuid, bool> assistTankAtPlatform3;
    std::unordered_map<ObjectGuid, std::vector<bool>> mtBalconyMidpointVisited;
    std::unordered_map<ObjectGuid, std::vector<bool>> atBalconyMidpointVisited;
    // std::unordered_map<ObjectGuid, bool> mtGroundMidpointVisited;
    // std::unordered_map<ObjectGuid, bool> atGroundMidpointVisited;
    std::unordered_map<ObjectGuid, std::vector<bool>> meleeDpsWaypointVisited;
    /* std::vector<Position> midpoints_1_to_6 =
    {
        TempestKeepPositions::AlarPlatform1To2MidpointA,
        TempestKeepPositions::AlarPlatform1To2MidpointB,
        TempestKeepPositions::AlarPlatform2To3MidpointA,
        TempestKeepPositions::AlarPlatform2To3MidpointB,
        TempestKeepPositions::AlarPlatform3To4MidpointA,
        TempestKeepPositions::AlarPlatform3To4MidpointB
    };
    std::vector<Position> midpoints_6_to_1 =
    {
        TempestKeepPositions::AlarPlatform3To4MidpointB,
        TempestKeepPositions::AlarPlatform3To4MidpointA,
        TempestKeepPositions::AlarPlatform2To3MidpointB,
        TempestKeepPositions::AlarPlatform2To3MidpointA,
        TempestKeepPositions::AlarPlatform1To2MidpointB,
        TempestKeepPositions::AlarPlatform1To2MidpointA
    }; */
    std::vector<Position> midpoints_1_to_2 =
    {
        TempestKeepPositions::AlarPlatform1To2MidpointA,
        TempestKeepPositions::AlarPlatform1To2MidpointB,
    };
    std::vector<Position> midpoints_2_to_1 =
    {
        TempestKeepPositions::AlarPlatform1To2MidpointB,
        TempestKeepPositions::AlarPlatform1To2MidpointA
    };
    std::vector<Position> midpoints_4_to_3 =
    {
        TempestKeepPositions::AlarPlatform3To4MidpointA,
        TempestKeepPositions::AlarPlatform3To4MidpointB
    };
    std::vector<Position> midpoints_3_to_4 =
    {
        TempestKeepPositions::AlarPlatform3To4MidpointB,
        TempestKeepPositions::AlarPlatform3To4MidpointA,
    };

    std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

}
