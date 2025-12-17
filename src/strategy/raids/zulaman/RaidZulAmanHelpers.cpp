#include "RaidZulAmanHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace ZulAmanHelpers
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

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void MarkTargetWithCircle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
    }

    void MarkTargetWithMoon(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
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

    Unit* GetFirstAliveUnitByEntries(PlayerbotAI* botAI, const std::vector<uint32>& entries)
    {
        if (!botAI)
            return nullptr;

        auto npcValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcValue)
            return nullptr;

        auto const& npcs = npcValue->Get();
        for (uint32 entry : entries)
        {
            for (auto const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                    return unit;
            }
        }

        return nullptr;
    }

    bool AnyNearbyNpcWithEntry(PlayerbotAI* botAI, uint32 entry)
    {
        if (!botAI)
            return false;

        auto npcValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
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

    // Akil'zon <Eagle Avatar>
    const Position AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };
    std::unordered_map<uint32, time_t> electricalStormTimer;

    bool IsElectricalStormTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsDps(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return false;
    }

    bool IsElectricalStormWindowActive(Unit* akilzon)
    {
        if (!akilzon)
            return false;

        // Electrical Storm forms every 60 seconds and lasts for 8.5 seconds
        // The window begins 5 seconds before the Electrical Storm forms
        const uint32 windowStartSeconds = 55;
        const uint32 windowEndSeconds = 69;
        const uint32 windowRecurrenceSeconds = 60;

        const uint32 instanceId = akilzon->GetMap()->GetInstanceId();
        time_t now = time(nullptr);

        auto it = electricalStormTimer.find(instanceId);
        if (it == electricalStormTimer.end())
            return false;

        long elapsed = static_cast<long>(now - it->second);
        if (elapsed < 0)
            return false;

        uint32 remainder = static_cast<uint32>(elapsed % windowRecurrenceSeconds);

        return remainder >= windowStartSeconds && remainder <= windowEndSeconds;
    }

    /* bool AnyGroupMemberHasElectricalStorm(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_ELECTRICAL_STORM))
                return true;
        }
        return false;
    } */

    // Nalorakk <Bear Avatar>
    const Position NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };

    // Jan'alai <Dragonhawk Avatar>
    const Position JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };
    std::unordered_map<ObjectGuid, Position> janalaiRangedPositions;

    std::pair<Unit*, Unit*> GetAmaniHatcherPair(PlayerbotAI* botAI)
    {
        Unit* lowest = nullptr;
        Unit* highest = nullptr;

        auto npcValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcValue)
            return {nullptr, nullptr};

        for (auto const& guid : npcValue->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_AMANI_HATCHER)
            {
                if (!lowest || unit->GetGUID().GetRawValue() < lowest->GetGUID().GetRawValue())
                    lowest = unit;
                if (!highest || unit->GetGUID().GetRawValue() > highest->GetGUID().GetRawValue())
                    highest = unit;
            }
        }

        return {lowest, highest};
    }

    // Halazzi <Lynx Avatar>
    const Position HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

    // Hex Lord Malacrass
    const Position MALACRASS_TANK_POSITION = { 118.713f, 996.234f, 30.650f }; // 30.62356f is actual Z

    // Zul'jin
    const Position ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };
}
