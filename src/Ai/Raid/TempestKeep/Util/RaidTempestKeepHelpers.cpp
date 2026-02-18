#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace TempestKeepHelpers
{
    // General

    Unit* GetNearestNonTankPlayerInRadius(Player* bot, float radius)
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

                if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
                {
                    if (memberAI->IsTank(member))
                        continue;
                }

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

    std::vector<Unit*> GetAllHazardTriggers(
        PlayerbotAI* botAI, Player* bot, uint32 npcEntry, float maxSearchRadius)
    {
        std::vector<Unit*> hazardTriggers;
        auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == npcEntry && bot->GetExactDist2d(unit) < maxSearchRadius)
                hazardTriggers.push_back(unit);
        }

        return hazardTriggers;
    }

    Position FindSafestNearbyPosition(Player* bot, const std::vector<Unit*>& hazards,
        float hazardRadius, const Position* center)
    {
        constexpr float searchStep = M_PI / 8.0f;
        constexpr float minDistance = 2.0f;
        constexpr float maxDistance = 30.0f;
        constexpr float distanceStep = 1.0f;

        Position bestPos;
        float minMoveDistance = std::numeric_limits<float>::max();
        bool foundSafe = false;

        for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
        {
            for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
            {
                const Position& searchCenter = center ? *center : bot->GetPosition();
                float x = searchCenter.GetPositionX() + distance * std::cos(angle);
                float y = searchCenter.GetPositionY() + distance * std::sin(angle);

                bool isSafe = true;
                for (Unit* hazard : hazards)
                {
                    if (hazard->GetExactDist2d(x, y) < hazardRadius)
                    {
                        isSafe = false;
                        break;
                    }
                }

                if (!isSafe)
                    continue;

                Position testPos(x, y, bot->GetPositionZ());

                bool pathSafe = IsPathSafeFromHazards(bot->GetPosition(), testPos, hazards, hazardRadius);
                if (pathSafe || !foundSafe)
                {
                    float moveDistance = bot->GetExactDist2d(x, y);

                    if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                    {
                        bestPos = testPos;
                        minMoveDistance = moveDistance;
                        foundSafe = true;
                    }
                    else if (!foundSafe && moveDistance < minMoveDistance)
                    {
                        bestPos = testPos;
                        minMoveDistance = moveDistance;
                    }
                }
            }

            if (foundSafe)
                break;
        }

        return bestPos;
    }

    bool IsPathSafeFromHazards(
        const Position& start, const Position& end, const std::vector<Unit*>& hazards, float hazardRadius)
    {
        constexpr uint8 numChecks = 10;
        float dx = end.GetPositionX() - start.GetPositionX();
        float dy = end.GetPositionY() - start.GetPositionY();

        for (uint8 i = 1; i <= numChecks; ++i)
        {
            float ratio = static_cast<float>(i) / numChecks;
            float checkX = start.GetPositionX() + dx * ratio;
            float checkY = start.GetPositionY() + dy * ratio;

            for (Unit* hazard : hazards)
            {
                float distToHazard = hazard->GetExactDist2d(checkX, checkY);
                if (distToHazard < hazardRadius)
                    return false;
            }
        }

        return true;
    }

    // Al'ar <Phoenix God>

    const Position ALAR_PLATFORM_0 = { 335.638f,  59.4879f, 17.9319f }; // West Platform
    const Position ALAR_PLATFORM_1 = { 388.751f,  31.7312f, 20.2636f }; // Northwest Platform
    const Position ALAR_PLATFORM_2 = { 388.791f, -33.1059f, 20.2636f }; // Northeast Platform
    const Position ALAR_PLATFORM_3 = { 332.723f,  -61.159f, 17.9791f }; // East Platform
    const std::array<Position, 4> PLATFORM_POSITIONS =
    {
        ALAR_PLATFORM_0,
        ALAR_PLATFORM_1,
        ALAR_PLATFORM_2,
        ALAR_PLATFORM_3
    };
    const Position ALAR_GROUND_0 = { 336.439f,  48.181f, -2.389f }; // Ground counterpart to West Platform
    const Position ALAR_GROUND_1 = { 379.122f,  25.146f, -2.385f }; // Ground counterpart to Northwest Platform
    const Position ALAR_GROUND_2 = { 378.583f, -27.481f, -2.385f }; // Ground counterpart to Northeast Platform
    const Position ALAR_GROUND_3 = { 331.631f, -49.716f, -2.389f }; // Ground counterpart to East Platform
    const std::array<Position, 4> GROUND_POSITIONS =
    {
        ALAR_GROUND_0,
        ALAR_GROUND_1,
        ALAR_GROUND_2,
        ALAR_GROUND_3
    };
    const Position ALAR_ROOM_CENTER =         { 330.611f,  -2.540f, -2.389f };
    const Position ALAR_POINT_QUILL_OR_DIVE = { 332.000f,   0.010f, 43.000f };
    const Position ALAR_POINT_MIDDLE =        { 331.000f,   0.010f, -2.380f };
    const Position ALAR_SE_RAMP_BASE =        { 281.064f, -36.590f, -2.389f };
    const Position ALAR_SW_RAMP_BASE =        { 281.064f,  36.590f, -2.389f };
    const Position ALAR_ROOM_S_CENTER =       { 281.064f,   0.000f, -2.389f };

    std::unordered_map<uint32, bool> lastRebirthState;
    std::unordered_map<uint32, bool> isAlarInPhase2;

    int8 GetAlarDestinationLocationIndex(Unit* alar, Position& dest)
    {
        if (!alar)
            return LOCATION_NONE;

        float x, y, z;
        if (!alar->GetMotionMaster()->GetDestination(x, y, z))
            return LOCATION_NONE;

        dest.Relocate(x, y, z);

        const std::array<Position, 6> locations =
        {
            ALAR_PLATFORM_0,
            ALAR_PLATFORM_1,
            ALAR_PLATFORM_2,
            ALAR_PLATFORM_3,
            ALAR_POINT_QUILL_OR_DIVE,
            ALAR_POINT_MIDDLE,
        };

        float minDist = std::numeric_limits<float>::max();
        int8 locationIndex = LOCATION_NONE;
        for (int8 i = 0; i < TOTAL_ALAR_LOCATIONS; ++i)
        {
            float dist = dest.GetExactDist2d(&locations[i]);
            if (dist < minDist)
            {
                minDist = dist;
                locationIndex = i;
            }
        }
        if (minDist > 0.1f)
            return LOCATION_NONE;

        return locationIndex;
    }

    int8 GetAlarCurrentLocationIndex(Unit* alar)
    {
        if (!alar)
            return LOCATION_NONE;

        const std::array<Position, 6> locations =
        {
            ALAR_PLATFORM_0,
            ALAR_PLATFORM_1,
            ALAR_PLATFORM_2,
            ALAR_PLATFORM_3,
            ALAR_POINT_QUILL_OR_DIVE,
            ALAR_POINT_MIDDLE,
        };

        float minDist = std::numeric_limits<float>::max();
        int8 locationIndex = LOCATION_NONE;
        for (int8 i = 0; i < TOTAL_ALAR_LOCATIONS; ++i)
        {
            float dist = alar->GetPosition().GetExactDist2d(&locations[i]);
            if (dist < minDist)
            {
                minDist = dist;
                locationIndex = i;
            }
        }
        if (minDist > 0.1f)
            return LOCATION_NONE;

        return locationIndex;
    }

    void GetClosestPlatformAndGround(const Position& botPos, int8& closestPlatform, Position& ground)
    {
        float minDist = std::numeric_limits<float>::max();
        closestPlatform = -1;
        for (int8 i = 0; i < 4; ++i)
        {
            float dist = botPos.GetExactDist2d(&PLATFORM_POSITIONS[i]);
            if (dist < minDist)
            {
                minDist = dist;
                closestPlatform = i;
            }
        }
        ground = GROUND_POSITIONS[closestPlatform];
    }

    std::pair<Unit*, Unit*> GetFirstTwoEmbersOfAlar(PlayerbotAI* botAI)
    {
        Unit* firstEmber = nullptr;
        Unit* secondEmber = nullptr;

        for (auto const& guid :
             botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_EMBER_OF_ALAR)
            {
                if (!firstEmber)
                    firstEmber = unit;
                else if (!secondEmber)
                {
                    secondEmber = unit;
                    break;
                }
            }
        }

        return { firstEmber, secondEmber };
    }

    Player* GetSecondEmberTank(PlayerbotAI* botAI)
    {
        Player* mainTank = GetGroupMainTank(botAI, botAI->GetBot());
        Player* assistTank = GetGroupFirstAssistTank(botAI, botAI->GetBot());

        bool mainTankHasMelt = mainTank && mainTank->HasAura(SPELL_MELT_ARMOR);
        bool assistTankHasMelt = assistTank && assistTank->HasAura(SPELL_MELT_ARMOR);

        if (mainTankHasMelt)
            return mainTank;

        if (assistTankHasMelt || (!mainTankHasMelt && !assistTankHasMelt))
            return assistTank;

        return nullptr;
    }

    // Void Reaver

    const Position VOID_REAVER_TANK_POSITION = { 423.845f, 371.733f, 14.897f };

    // Kael'thas Sunstrider <Lord of the Blood Elves>

    const Position SANGUINAR_TANK_POSITION = { 775.478f, 39.888f, 46.780f };
    const Position SANGUINAR_WAITING_POSITION = { 761.850f, 27.459f, 46.779f };
    const Position TELONICUS_TANK_POSITION = { 773.717f, 44.091f, 46.780f };
    const Position TELONICUS_WAITING_POSITION = { 754.347f, 31.739f, 46.796f };
    const Position ADVISOR_HEAL_POSITION = { 752.171f, 19.494f, 46.779f };
    const Position CAPERNIAN_WAITING_POSITION = { 743.897f, -11.575f, 46.779f };
    const Position KAELTHAS_TANK_POSITION = { 799.390f, -0.837f, 48.729f };

    std::unordered_map<uint32, time_t> advisorDpsWaitTimer;

    Player* GetCapernianTank(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) Look for an assistant Warlock (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK &&
                group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fall back to first found bot Warlock
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) &&
                member->getClass() == CLASS_WARLOCK)
                return member;
        }

        // (3) Return nullptr if none found
        return nullptr;
    }

    // One Hunter will start on Sanguinar in Phase 3 with Melee to apply Armor Disruption
    Player* GetDebuffHunter(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) Look for an assistant Hunter (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
                group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fall back to first found bot Hunter
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) &&
                member->getClass() == CLASS_HUNTER)
                return member;
        }

        return nullptr;
    }

    bool IsAnyLegendaryWeaponDead(PlayerbotAI* botAI, Player* bot)
    {
        static const std::array<std::pair<const char*, uint32>, 7> weapons =
        {
            std::make_pair("staff of disintegration", NPC_STAFF_OF_DISINTEGRATION),
            std::make_pair("cosmic infuser", NPC_COSMIC_INFUSER),
            std::make_pair("infinity blade", NPC_INFINITY_BLADES),
            std::make_pair("warp slicer", NPC_WARP_SLICER),
            std::make_pair("phaseshift bulwark", NPC_PHASESHIFT_BULWARK),
            std::make_pair("netherstrand longbow", NPC_NETHERSTRAND_LONGBOW),
            std::make_pair("devastation", NPC_DEVASTATION)
        };

        for (auto const& [name, entry] : weapons)
        {
            Unit* weapon =
                botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (weapon && weapon->IsAlive())
                continue;

            auto const& corpses =
                botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest corpses")->Get();
            for (auto const& guid : corpses)
            {
                LootObject loot(bot, guid);
                WorldObject* object = loot.GetWorldObject(bot);
                if (!object)
                    continue;

                if (Creature* creature = object->ToCreature();
                    creature && creature->GetEntry() == entry && !creature->IsAlive())
                    return true;
            }
        }

        return false;
    }

    bool HasEquippableItemForSlot(Player* bot, uint8 slot)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            for (uint8 bagSlot = 0; bagSlot < MAX_BAG_SIZE; ++bagSlot)
            {
                Item* item = bot->GetItemByPos(bag, bagSlot);
                if (!item)
                    continue;

                ItemTemplate const* proto = item->GetTemplate();
                if (!proto)
                    continue;

                uint16 dest = 0;
                if (bot->CanEquipItem(slot, dest, item, false))
                    return true;
            }
        }

        return false;
    }

    bool HasEquippableOffhand(Player* bot)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            for (uint8 bagSlot = 0; bagSlot < MAX_BAG_SIZE; ++bagSlot)
            {
                Item* item = bot->GetItemByPos(bag, bagSlot);
                if (!item)
                    continue;

                ItemTemplate const* proto = item->GetTemplate();
                if (!proto)
                    continue;

                if (proto->InventoryType == INVTYPE_SHIELD ||
                    proto->InventoryType == INVTYPE_WEAPONOFFHAND ||
                    proto->InventoryType == INVTYPE_HOLDABLE)
                {
                    uint16 dest = 0;
                    if (bot->CanEquipItem(EQUIPMENT_SLOT_OFFHAND, dest, item, false))
                        return true;
                }
            }
        }

        return false;
    }
}
