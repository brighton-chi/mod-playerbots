#include <algorithm>

#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace TempestKeepHelpers
{
    const Position ALAR_ROOM_CENTER = { 330.611f, -2.540f, -2.389f };
    const Position ALAR_RANGED_CENTER = { 346.758f, 3.794f, -2.389f };
    const Position ALAR_PLATFORM_0 = { 335.638f, 59.4879f, 17.9319f }; // West Platform
    const Position ALAR_PLATFORM_1 = { 388.751f, 31.7312f, 20.2636f }; // Northwest Platform
    const Position ALAR_PLATFORM_2 = { 388.791f, -33.1059f, 20.2636f }; // Northeast Platform
    const Position ALAR_PLATFORM_3 = { 332.723f, -61.159f, 17.9791f }; // East Platform
    const Position ALAR_GROUND_0 = { 336.439f, 48.181f, -2.389f }; // Landing point for jumping from West Platform
    const Position ALAR_GROUND_1 = { 379.122f, 25.146f, -2.385f }; // Landing point for jumping from Northwest Platform
    const Position ALAR_GROUND_2 = { 378.583f, -27.481f, -2.385f }; // Landing point for jumping from Northeast Platform
    const Position ALAR_GROUND_3 = { 331.631f, -49.716f, -2.389f }; // Landing point for jumping from East Platform
    const Position ALAR_SE_RAMP_BASE = { 281.064f, -36.590f, -2.389f };
    const Position ALAR_SW_RAMP_BASE = { 281.064f, 36.590f, -2.389f };
    const Position ALAR_ROOM_S_CENTER = { 281.064f, 0.0f, -2.389f };

    const Position VOID_REAVER_TANK_POSITION = { 423.845f, 371.733f, 14.897f }; // middle of room
    const Position SANGUINAR_TANK_POSITION = { 775.478f, 39.888f, 46.780f };
    // const Position CAPERNIAN_TANK_POSITION = { 743.772f, -13.762f, 46.780f };
    const Position TELONICUS_TANK_POSITION = { 773.717f, 44.091f, 46.780f };
    const Position KAELTHAS_WEAPON_STACK_POSITION = { 775.296f, -0.822f, 48.729f };
    const Position KAELTHAS_AXE_TANK_POSITION = { 775.621f, 20.717f, 48.729f };
    const Position KAELTHAS_BOW_TANK_POSITION = { 777.713f, -28.857f, 48.729f };
    const Position KAELTHAS_TANK_POSITION = { 799.390f, -0.837f, 48.729f }; // need method for this

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

    bool IsAlarAddTank(PlayerbotAI* botAI, Player* bot)
    {
        return botAI->IsTank(bot) &&
               !botAI->IsMainTank(bot) &&
               !botAI->IsAssistTankOfIndex(bot, 0);
    }

    // Phase 1: Single Advisor Phase
    // Trigger: Any advisor has the NON_ATTACKABLE flag or PERMANENT_FEIGN_DEATH aura, and no weapons exist
    bool IsKaelthasInPhase1(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();
        if (!kaelthas || !kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return false;

        const char* weapons[] = {"netherstrand longbow", "cosmic infuser", "devastation",
                                 "infinity blades", "warp slicer", "staff of disintegration",
                                 "phaseshift bulwark"};
        for (const char* name : weapons)
        {
            Unit* weapon = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (weapon && weapon->IsAlive())
                return false;
        }

        const char* advisors[] = {"thaladred the darkener", "lord sanguinar",
                                  "grand astromancer capernian", "master engineer telonicus"};
        for (const char* name : advisors)
        {
            Unit* advisor = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (!advisor)
                continue;

            if (advisor->IsAlive() &&
                (advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
                 advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH)))
            {
                return true;
            }
        }

        return false;
    }

    // Phase 2: Legendary Weapons Phase
    // Trigger: At least one weapon exists, and at least one advisor has PERMANENT_FEIGN_DEATH aura or is NOT_SELECTABLE
    bool IsKaelthasInPhase2(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();
        if (!kaelthas || !kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return false;

        const char* weapons[] = {"netherstrand longbow", "cosmic infuser", "devastation",
                                "infinity blades", "warp slicer", "staff of disintegration",
                                "phaseshift bulwark"};
        bool hasWeapon = false;
        for (const char* name : weapons)
        {
            Unit* weapon = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (weapon && weapon->IsAlive())
            {
                hasWeapon = true;
                break;
            }
        }

        if (!hasWeapon)
            return false;

        const char* advisors[] = {"thaladred the darkener", "lord sanguinar",
                                "grand astromancer capernian", "master engineer telonicus"};
        for (const char* name : advisors)
        {
            Unit* advisor = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (advisor && (advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH) ||
                            advisor->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE)))
                return true;
        }

        return false;
    }

    // Phase 2 to 3 Transition: Post-Resurrect Waiting Window
    bool IsKaelthasInPhase2To3Transition(PlayerbotAI* botAI)
    {
        const char* advisors[] = {
            "thaladred the darkener",
            "lord sanguinar",
            "grand astromancer capernian",
            "master engineer telonicus"
        };

        // 1) Every advisor is resurrected
        for (const char* name : advisors)
        {
            Unit* advisor = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (!advisor)
                return false;
            if (!advisor->IsAlive())
                return false;
            if (advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                return false;
        }

        // 2) None of the advisors should have re-entered combat yet
        //    If any advisor is attacking, the window ended
        for (const char* name : advisors)
        {
            Unit* advisor = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (advisor->GetVictim())
                return false;
        }

        return true;
    }

    // Phase 3: All Advisors Phase
    // Trigger: Each advisor is either (1) alive without PERMANENT_FEIGN_DEATH aura or NON_ATTACKABLE or NON_SELECTABLE flag,
    // or (2) dead but found as a dead creature object
    bool IsKaelthasInPhase3(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();
        if (!kaelthas || !kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return false;

        const uint32 advisorEntries[] =
        {
            NPC_THALADRED_THE_DARKENER,
            NPC_LORD_SANGUINAR,
            NPC_GRAND_ASTROMANCER_CAPERNIAN,
            NPC_MASTER_ENGINEER_TELONICUS
        };

        uint8 aliveAttackable = 0;
        uint8 aliveTotal = 0;
        uint8 corpseCount = 0;
        bool anyFeignOrNonAttackable = false;

        GuidVector corpses = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest corpses")->Get();
        for (auto const& guid : corpses)
        {
            LootObject loot(botAI->GetBot(), guid);
            WorldObject* object = loot.GetWorldObject(botAI->GetBot());
            if (!object)
                continue;
            if (Creature* creature = object->ToCreature())
            {
                for (auto entry : advisorEntries)
                {
                    if (creature->GetEntry() == entry)
                    {
                        ++corpseCount;
                        break;
                    }
                }
            }
        }

        for (auto entry : advisorEntries)
        {
            Unit* advisor = GetFirstAliveUnitByEntry(botAI, entry);
            if (!advisor)
                continue;
            ++aliveTotal;

            if (advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH) ||
                advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
                advisor->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            {
                anyFeignOrNonAttackable = true;
            }

            bool isAttackable = advisor->IsAlive() &&
                                !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
                                !advisor->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
                                !advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
            if (isAttackable)
                ++aliveAttackable;
        }

        if (anyFeignOrNonAttackable)
            return false;

        if (aliveAttackable > 0 || aliveTotal > 0 || corpseCount > 0)
            return true;

        return false;
    }

    // Phase 4: Kael'thas
    // Trigger: Kael'thas is attackable and above 50% HP
    bool IsKaelthasInPhase4(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();

        return kaelthas && !kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !kaelthas->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
               kaelthas->GetHealthPct() > 50.0f;
    }

    // Phase 4 to 5 Transition: Kael'thas Performing Power-Up RP
    // Trigger: Kael'thas at or below 50% HP and has NOT_SELECTABLE flag
    bool IsKaelthasInPhase4To5Transition(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();

        return kaelthas && kaelthas->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
               kaelthas->GetHealthPct() <= 50.0f;
    }

    // Phase 5: Kael'thas at Full Power
    // Trigger: Kael'thas is attackable and at or below 50% HP
    bool IsKaelthasInPhase5(PlayerbotAI* botAI)
    {
        Unit* kaelthas = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kael'thas sunstrider")->Get();

        return kaelthas && kaelthas->GetHealthPct() <= 50.0f && !kaelthas->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    }

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

        // (2) Fallback to bot Warlock with highest HP
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

    std::unordered_map<ObjectGuid, Position> initialVoidReaverPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedInitialVoidReaverPosition;

    std::unordered_map<uint32, time_t> advisorDpsWaitTimer;
}
