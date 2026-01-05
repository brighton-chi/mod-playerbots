#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleIllidanBossAI.h"
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

    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
    {
        auto const& targets =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
        for (auto const& npcGuid : targets)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }

        return nullptr;
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
    // const Position SHAHRAZ_TANK_POSITION = { 925.683f, 179.920f, 192.833f, 0.102f };
    const Position SHAHRAZ_TANK_POSITION = { 926.761f, 179.666f, 192.831f, 6.215f }; // UNTESTED
    const Position SHAHRAZ_TRANSITION_POSITION = { 941.557f, 182.719f, 192.418f, 4.943f };
    // const Position SHAHRAZ_MELEE_POSITION = { 930.534f, 179.791f, 192.827f, 3.384f };
    // const Position SHAHRAZ_RANGED_POSITION = { 957.749f, 176.927f, 192.822f, 3.079f };
    const Position SHAHRAZ_RANGED_POSITION = { 958.632f, 180.111f, 192.826f, 3.266f }; // UNTESTED
    std::unordered_map<ObjectGuid, uint8> shahrazTankStep;

    // Illidari Council
    const Position GATHIOS_TANK_POSITION_1 = { 662.977f, 296.246f, 271.688f, 6.275f };
    const Position GATHIOS_TANK_POSITION_2 = { 636.238f, 283.719f, 271.629f, 0.357f };
    const Position GATHIOS_TANK_POSITION_3 = { 655.571f, 261.377f, 271.687f, 2.141f };
    const Position GATHIOS_TANK_POSITION_4 = { 673.789f, 274.139f, 271.689f, 3.411f };
    const Position ZEREVOR_TANK_POSITION = { 680.924f, 351.857f, 271.701f, 5.051f };
    const Position MALANDE_TANK_POSITION = { 690.101f, 305.166f, 277.443f, 0.000f };
    const Position DARKSHADOW_TANK_POSITION = { 644.156f, 338.085f, 271.688f, 5.669f };
    std::unordered_map<uint32, time_t> councilDpsWaitTimer;
    std::unordered_map<ObjectGuid, uint8> gathiosTankStep;

    Player* GetZerevorMageTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) Look for an assistant Mage (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fall back to bot Mage with highest HP
        Player* highestHpMage = nullptr;
        uint32 highestHp = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_MAGE)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Mage tank, or nullptr if none found
        return highestHpMage;
    }

    // Illidan Stormrage <The Betrayer>
    const Position ILLIDAN_C_GRATE_POSITION = { 683.569f, 305.245f, 353.192f };
    // const Position ILLIDAN_N_GRATE_POSITION = { 684.115f, 305.215f, 353.192f };
    const Position ILLIDAN_N_GRATE_POSITION = { 682.884f, 305.06f, 353.192f };
    const Position ILLIDAN_SW_GRATE_POSITION = { 672.828f, 311.496f, 353.192f };
    const Position ILLIDAN_SE_GRATE_POSITION = { 672.928f, 298.357f, 353.192f };
    const Position* gratePositions[3] =
    {
        &ILLIDAN_N_GRATE_POSITION,
        &ILLIDAN_SW_GRATE_POSITION,
        &ILLIDAN_SE_GRATE_POSITION
    };
    const Position ILLIDAN_E_GLAIVE_WAITING_POSITION = { 682.140f, 287.685f, 354.135f };
    /* const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1 = { 694.867f, 300.910f, 354.186f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2 = { 689.861f, 293.783f, 354.149f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3 = { 682.190f, 289.148f, 354.136f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4 = { 673.610f, 288.028f, 354.140f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5 = { 664.911f, 291.331f, 354.225f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6 = { 659.002f, 298.441f, 354.110f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7 = { 661.933f, 282.331f, 354.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_8 = { 670.698f, 279.068f, 353.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_9 = { 680.054f, 279.540f, 354.089f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_10 = { 689.442f, 282.678f, 354.303f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_11 = { 697.097f, 287.966f, 353.983f }; */
    // BELOW IS FLIPPED ORDER (so start outside first)
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1 = { 697.097f, 287.966f, 353.983f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2 = { 689.442f, 282.678f, 354.303f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3 = { 680.054f, 279.540f, 354.089f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4 = { 670.698f, 279.068f, 353.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5 = { 661.933f, 282.331f, 354.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6 = { 659.002f, 298.441f, 354.110f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7 = { 664.911f, 291.331f, 354.225f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_8 = { 673.610f, 288.028f, 354.140f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_9 = { 682.190f, 289.148f, 354.136f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_10 = { 689.861f, 293.783f, 354.149f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_11 = { 694.867f, 300.910f, 354.186f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITIONS[11] =
    {
        ILLIDAN_E_GLAIVE_TANK_POSITION_1,
        ILLIDAN_E_GLAIVE_TANK_POSITION_2,
        ILLIDAN_E_GLAIVE_TANK_POSITION_3,
        ILLIDAN_E_GLAIVE_TANK_POSITION_4,
        ILLIDAN_E_GLAIVE_TANK_POSITION_5,
        ILLIDAN_E_GLAIVE_TANK_POSITION_6,
        ILLIDAN_E_GLAIVE_TANK_POSITION_7,
        ILLIDAN_E_GLAIVE_TANK_POSITION_8,
        ILLIDAN_E_GLAIVE_TANK_POSITION_9,
        ILLIDAN_E_GLAIVE_TANK_POSITION_10,
        ILLIDAN_E_GLAIVE_TANK_POSITION_11,
    };
    const Position ILLIDAN_W_GLAIVE_WAITING_POSITION = { 679.849f, 322.339f, 354.118f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_1 = { 697.363f, 323.860f, 353.948f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_2 = { 690.218f, 329.326f, 353.868f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_3 = { 681.647f, 322.243f, 353.813f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_4 = { 672.612f, 331.890f, 353.865f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_5 = { 663.833f, 329.083f, 353.783f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_6 = { 656.462f, 324.135f, 353.834f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_7 = { 657.054f, 314.865f, 354.156f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_8 = { 665.202f, 319.824f, 354.108f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_9 = { 674.817f, 322.193f, 354.125f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_10 = { 684.370f, 322.058f, 354.063f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_11 = { 692.023f, 315.866f, 354.127f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITIONS[11] =
    {
        ILLIDAN_W_GLAIVE_TANK_POSITION_1,
        ILLIDAN_W_GLAIVE_TANK_POSITION_2,
        ILLIDAN_W_GLAIVE_TANK_POSITION_3,
        ILLIDAN_W_GLAIVE_TANK_POSITION_4,
        ILLIDAN_W_GLAIVE_TANK_POSITION_5,
        ILLIDAN_W_GLAIVE_TANK_POSITION_6,
        ILLIDAN_W_GLAIVE_TANK_POSITION_7,
        ILLIDAN_W_GLAIVE_TANK_POSITION_8,
        ILLIDAN_W_GLAIVE_TANK_POSITION_9,
        ILLIDAN_W_GLAIVE_TANK_POSITION_10,
        ILLIDAN_W_GLAIVE_TANK_POSITION_11,
    };
    const Position eyeBeamPos[MAX_EYE_BEAM_POS * 2] =
    {
        {639.97f, 301.63f, 354.0f, 0.0f},
        {658.83f, 265.10f, 354.0f, 0.0f},
        {656.86f, 344.07f, 354.0f, 0.0f},
        {640.70f, 310.47f, 354.0f, 0.0f},

        {706.22f, 273.26f, 354.0f, 0.0f},
        {717.55f, 328.33f, 354.0f, 0.0f},
        {718.06f, 286.08f, 354.0f, 0.0f},
        {705.92f, 337.14f, 354.0f, 0.0f}
    };
    std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
    std::unordered_map<uint32, time_t> illidanDpsWaitTimer;

    int GetIllidanPhase(Unit* illidan)
    {
        if (!illidan)
            return -1;

        // Phase 1: Health > 65%
        if (illidan->GetHealthPct() > 65.0f)
            return 1;

        // Phase 2: Flying
        if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && !illidan->HasAura(SPELL_SHADOW_PRISON))
            return 2;

        // Phase 3: Normal (ground, 65-30%, not demon)
        if (illidan->GetHealthPct() > 30.0f && !illidan->HasAura(SPELL_DEMON_FORM))
            return 3;

        // Phase 4: Demon Form
        if (illidan->HasAura(SPELL_DEMON_FORM))
            return 4;

        // Phase 5: Health <= 30%
        if (illidan->GetHealthPct() <= 30.0f)
            return 5;

        return 0;
    }

    /* Position GetClosestPointInDrawSoulSafeSector(
        Unit* illidan, Position botPos, float angleOffset, int direction, float minRadius, float maxRadius, float sectorAngleWidth)
    {
        if (!illidan)
            return Position();

        float bossX = illidan->GetPositionX();
        float bossY = illidan->GetPositionY();
        float bossZ = illidan->GetPositionZ();
        float bossFacing = illidan->GetOrientation();

        // Center angle of the cone
        float centerAngle = bossFacing + direction * angleOffset;

        // Vector from boss to bot
        float dx = botPos.GetPositionX() - bossX;
        float dy = botPos.GetPositionY() - bossY;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Clamp distance to [minRadius, maxRadius]
        float clampedDist = std::min(std::max(distance, minRadius), maxRadius);

        // Angle from boss to bot
        float angleToBot = std::atan2(dy, dx);
        float deltaAngle = angleToBot - centerAngle;
        // Normalize to [-pi, pi]
        while (deltaAngle > M_PI) deltaAngle -= 2.0f * M_PI;
        while (deltaAngle < -M_PI) deltaAngle += 2.0f * M_PI;

        // Clamp angle to cone
        float halfCone = sectorAngleWidth / 2.0f;
        float clampedAngle = centerAngle;
        if (deltaAngle > halfCone)
            clampedAngle = centerAngle + halfCone;
        else if (deltaAngle < -halfCone)
            clampedAngle = centerAngle - halfCone;
        else
            clampedAngle = angleToBot;

        // Calculate target position
        float targetX = bossX + clampedDist * std::cos(clampedAngle);
        float targetY = bossY + clampedDist * std::sin(clampedAngle);

        return Position(targetX, targetY, bossZ, clampedAngle);
    } */

    ObjectGuid eastFlameGuid;
    ObjectGuid westFlameGuid;

    std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(PlayerbotAI* botAI)
    {
        Unit* eastFlame = nullptr;
        Unit* westFlame = nullptr;

        // Gather all flames
        std::vector<Unit*> flames;
        for (auto const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_FLAME_OF_AZZINOTH)
                flames.push_back(unit);
        }

        // If both GUIDs are not set and there are exactly 2 flames, assign by position
        if (eastFlameGuid.IsEmpty() && westFlameGuid.IsEmpty() && flames.size() == 2)
        {
            float eastDist0 = flames[0]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);
            float eastDist1 = flames[1]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);

            if (eastDist0 < eastDist1)
            {
                eastFlameGuid = flames[0]->GetGUID();
                westFlameGuid = flames[1]->GetGUID();
            }
            else
            {
                eastFlameGuid = flames[1]->GetGUID();
                westFlameGuid = flames[0]->GetGUID();
            }
        }

        // Always return the flames by their snapshotted GUIDs
        for (Unit* unit : flames)
        {
            if (unit->GetGUID() == eastFlameGuid)
                eastFlame = unit;
            else if (unit->GetGUID() == westFlameGuid)
                westFlame = unit;
        }

        return { eastFlame, westFlame };
    }

    Player* GetIllidanWarlockTank(PlayerbotAI* botAI, Player* bot)
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

    Player* GetIllidanTrapperHunter(PlayerbotAI* botAI, Player* bot)
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

        // (2) Fall back to any bot Hunter
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_HUNTER)
                continue;

            return member;
        }

        // (3) Return the found Hunter, or nullptr if none found
        return nullptr;
    }

    Player* HasParasiticShadowfiend(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* infectedPlayer = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!member || !member->IsAlive() || memberAI && memberAI->IsMainTank(member))
                continue;

            if (member->HasAura(SPELL_PARASITIC_SHADOWFIEND))
                return member;
        }

        return nullptr;
    }

    EyeBlastDangerArea GetEyeBlastDangerArea(PlayerbotAI* botAI, Unit* illidan)
    {
        boss_illidan_stormrage* illidanAI = dynamic_cast<boss_illidan_stormrage*>(illidan->GetAI());
        if (!illidanAI)
            return {};

        uint8 beamPosId = illidanAI->GetBeamPosId();

        Unit* eyeBlastTrigger = nullptr;
        auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_ILLIDAN_DB_TARGET)
            {
                eyeBlastTrigger = unit;
                break;
            }
        }

        if (!eyeBlastTrigger)
            return {};

        Position startPos = Position(eyeBlastTrigger->GetPositionX(), eyeBlastTrigger->GetPositionY(), eyeBlastTrigger->GetPositionZ());
        Position endPos = eyeBeamPos[beamPosId + MAX_EYE_BEAM_POS];

        float eyeBlastWidth = 10.0f;
        return { startPos, endPos, eyeBlastWidth };
    }

    bool IsPositionInEyeBlastDangerArea(const Position& pos, const EyeBlastDangerArea& area)
    {
        float dx = area.end.GetPositionX() - area.start.GetPositionX();
        float dy = area.end.GetPositionY() - area.start.GetPositionY();
        float length = area.start.GetExactDist2d(area.end.GetPositionX(), area.end.GetPositionY());

        float projectionFactor = ((pos.GetPositionX() - area.start.GetPositionX()) * dx +
                                  (pos.GetPositionY() - area.start.GetPositionY()) * dy) / (length * length);
        projectionFactor = std::clamp(projectionFactor, 0.0f, 1.0f);

        float closestX = area.start.GetPositionX() + projectionFactor * dx;
        float closestY = area.start.GetPositionY() + projectionFactor * dy;

        float distToLine = pos.GetExactDist2d(closestX, closestY);

        return distToLine < area.width;
    }
}
