#include <algorithm>

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace TempestKeepHelpers
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

    void MarkTargetWithCross(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
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

    // Al'ar <Phoenix God>

    const Position ALAR_PLATFORM_0 = { 335.638f, 59.4879f, 17.9319f }; // West Platform
    const Position ALAR_PLATFORM_1 = { 388.751f, 31.7312f, 20.2636f }; // Northwest Platform
    const Position ALAR_PLATFORM_2 = { 388.791f, -33.1059f, 20.2636f }; // Northeast Platform
    const Position ALAR_PLATFORM_3 = { 332.723f, -61.159f, 17.9791f }; // East Platform
    const Position PLATFORM_POSITIONS[] =
    {
        ALAR_PLATFORM_0,
        ALAR_PLATFORM_1,
        ALAR_PLATFORM_2,
        ALAR_PLATFORM_3
    };
    const Position ALAR_GROUND_0 = { 336.439f, 48.181f, -2.389f }; // Ground counterpart to West Platform
    const Position ALAR_GROUND_1 = { 379.122f, 25.146f, -2.385f }; // Ground counterpart to Northwest Platform
    const Position ALAR_GROUND_2 = { 378.583f, -27.481f, -2.385f }; // Ground counterpart to Northeast Platform
    const Position ALAR_GROUND_3 = { 331.631f, -49.716f, -2.389f }; // Ground counterpart to East Platform
    const Position GROUND_POSITIONS[] =
    {
        ALAR_GROUND_0,
        ALAR_GROUND_1,
        ALAR_GROUND_2,
        ALAR_GROUND_3
    };
    const Position ALAR_ROOM_CENTER = { 330.611f, -2.540f, -2.389f };
    const Position ALAR_POINT_QUILL_OR_DIVE = {332.0f, 0.01f, 43.0f, 0.0f};
    const Position ALAR_POINT_MIDDLE = {331.0f, 0.01f, -2.38f, 0.0f};
    const Position ALAR_SE_RAMP_BASE = { 281.064f, -36.590f, -2.389f };
    const Position ALAR_SW_RAMP_BASE = { 281.064f, 36.590f, -2.389f };
    const Position ALAR_ROOM_S_CENTER = { 281.064f, 0.0f, -2.389f };

    int8 GetAlarDestinationLocationIndex(Unit* alar, Position& dest)
    {
        if (!alar)
            return LOCATION_NONE;

        float x, y, z;
        if (!alar->GetMotionMaster()->GetDestination(x, y, z))
            return LOCATION_NONE;

        dest.Relocate(x, y, z);

        const Position locations[] =
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
        for (int8 i = 0; i < 7; ++i)
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

        const Position locations[] =
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
        for (int8 i = 0; i < 7; ++i)
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

    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot)
    {
        return botAI->IsTank(bot) &&
               !botAI->IsMainTank(bot) &&
               !botAI->IsAssistTankOfIndex(bot, 0);
    }

    // Void Reaver

    const Position VOID_REAVER_TANK_POSITION = { 423.845f, 371.733f, 14.897f };

    std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

    // High Astromancer Solarian
    // N/A

    // Kael'thas Sunstrider <Lord of the Blood Elves>

    const Position SANGUINAR_TANK_POSITION = { 775.478f, 39.888f, 46.780f };
    const Position TELONICUS_TANK_POSITION = { 773.717f, 44.091f, 46.780f };
    const Position ADVISOR_HEAL_POSITION = {  749.443f, 26.927f, 46.780f };
    const Position KAELTHAS_WEAPON_STACK_POSITION = { 775.296f, -0.822f, 48.729f };
    const Position KAELTHAS_AXE_TANK_POSITION = { 775.621f, 20.717f, 48.729f };
    const Position KAELTHAS_BOW_TANK_POSITION = { 777.713f, -28.857f, 48.729f };
    const Position KAELTHAS_TANK_POSITION = { 799.390f, -0.837f, 48.729f };

    std::unordered_map<uint32, time_t> advisorDpsWaitTimer;

    bool IsKaelthasMapIDTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                Player* capernianTank = GetCapernianTank(botAI, member);
                if (member && member->IsAlive() && botAI->IsDps(member) &&
                    member != capernianTank && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return false;
    }

    Player* GetCapernianTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) Look for an assistant Warlock (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fall back to bot Warlock with highest HP
        Player* highestHpWarlock = nullptr;
        uint32 highestHp = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_WARLOCK)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpWarlock || hp > highestHp)
            {
                highestHpWarlock = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Warlock tank, or nullptr if none found
        return highestHpWarlock;
    }

    Player* GetNetherstrandLongbowTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) Look for an assistant Hunter (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_HUNTER)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fallback to any bot Hunter
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_HUNTER)
                continue;

            return member;
        }

        return nullptr;
    }

    bool IsAnyLegendaryWeaponDead(PlayerbotAI* botAI, Player* bot)
    {
        const std::pair<const char*, uint32> weapons[] =
        {
            { "staff of disintegration", NPC_STAFF_OF_DISINTEGRATION },
            { "cosmic infuser", NPC_COSMIC_INFUSER },
            { "infinity blade", NPC_INFINITY_BLADES },
            { "warp slicer", NPC_WARP_SLICER },
            { "phaseshift bulwark", NPC_PHASESHIFT_BULWARK },
            { "netherstrand longbow", NPC_NETHERSTRAND_LONGBOW },
            { "devastation", NPC_DEVASTATION },
        };

        for (auto const& [name, entry] : weapons)
        {
            Unit* weapon = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (weapon && weapon->IsAlive())
                continue;

            GuidVector corpses = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest corpses")->Get();
            for (auto const& guid : corpses)
            {
                LootObject loot(bot, guid);
                WorldObject* object = loot.GetWorldObject(bot);
                if (!object)
                    continue;

                if (Creature* creature = object->ToCreature())
                {
                    if (creature->GetEntry() == entry && !creature->IsAlive())
                        return true;
                }
            }
        }

        return false;
    }
}
