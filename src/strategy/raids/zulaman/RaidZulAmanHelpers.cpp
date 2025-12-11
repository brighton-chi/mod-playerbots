#include "RaidZulAmanHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace ZulAmanHelpers
{
    const Position NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };
    const Position JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };
    const Position HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

    std::unordered_map<ObjectGuid, Position> janalaiRangedPositions;

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

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void MarkTargetWithCircle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
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

    Unit* GetFirstAliveUnit(const std::vector<Unit*>& units)
    {
        for (Unit* unit : units)
        {
            if (unit && unit->IsAlive())
                return unit;
        }

        return nullptr;
    }

    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
    {
        auto const& npcs =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }

        return nullptr;
    }

    bool AnyNearbyNpcWithEntry(PlayerbotAI* botAI, uint32 entry)
    {
        if (!botAI)
            return false;

        auto npcValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs");
        if (!npcValue)
            return false;

        auto const& npcs = npcValue->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return true;
        }

        return false;
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
}
