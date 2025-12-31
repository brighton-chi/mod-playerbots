#include "RaidBlackTempleHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace BlackTempleHelpers
{
    // General Helpers

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

    void MarkTargetWithSkull(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
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

    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsDps(member) &&
                    GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return false;
    }

    // High Warlord Naj'entus
    const Position NAJENTUS_TANK_POSITION = { 437.838f, 789.927f, 12.028f, 4.738f };

    // Supremus
    const Position SUPREMUS_TANK_POSITION = { 704.651f, 684.401f, 72.608f, 4.681f };
    std::unordered_map<ObjectGuid, Position> supremusRangedPositions;
    std::unordered_map<uint32, time_t> supremusPhaseTimer;

    bool HasSupremusVolcanoNearby(PlayerbotAI* botAI, Player* bot)
    {
        auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_SUPREMUS_VOLCANO &&
                bot->GetDistance2d(unit) < 50.0f)
                return true;
        }

        return false;
    }

    // Shade of Akama
    // N/A

    // Teron Gorefiend
    const Position GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f, 6.269f };
    const Position GOREFIEND_DIE_POSITION = { 525.709f, 377.177f, 193.203f, 3.879f };
    std::unordered_map<ObjectGuid, Position> gorefiendRangedPositions;

    // Gurtogg Bloodboil
    const Position GURTOGG_TANK_POSITION = { 735.987f, 272.451f, 63.554f, 0.048f };
    const Position GURTOGG_ABSORB_BLOODBOIL_POSITION = { 777.279f, 274.639f, 63.732f, 3.166f };
    std::unordered_map<uint32, time_t> gurtoggPhaseTimer;

    std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot)
    {
        Group* group = bot->GetGroup();
        std::vector<Player*> rangedMembers;
        std::vector<std::vector<Player*>> groups(3);

        if (!group)
            return groups;

        // Collect all alive ranged members
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive())
            {
                PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
                if (memberAI && memberAI->IsRanged(member))
                    rangedMembers.push_back(member);
            }
        }

        // Split into 3 groups of 5
        for (size_t i = 0; i < rangedMembers.size(); ++i)
        {
            groups[i / 5].push_back(rangedMembers[i]);
            if (groups[2].size() == 5)
                break; // Only fill up to 15
        }

        return groups;
    }

    int GetGurtoggActiveRotationGroup(Unit* gurtogg)
    {
        if (!gurtogg)
            return -1;

        auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
        if (it == gurtoggPhaseTimer.end())
            return -1;

        time_t now = std::time(nullptr);
        time_t elapsed = now - it->second;
        int groupIndex = (elapsed % 30) / 10; // 0 for 0-9s, 1 for 10-19s, 2 for 20-29s

        return groupIndex;
    }

    // Reliquary of Souls
    std::unordered_map<uint32, time_t> reliquaryDpsWaitTimer;

    // Mother Shahraz
    const Position SHAHRAZ_TANK_POSITION = { 933.881f, 178.022f, 192.824f, 6.234f };
    const Position SHAHRAZ_RANGED_POSITION = { 957.749f, 176.927f, 192.822f, 3.079f };

    // Illidari Council

    // Illidan Stormrage <The Betrayer>

}
