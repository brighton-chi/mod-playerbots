#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleIllidanBossAI.h"
#include "Group.h"
#include "Playerbots.h"

namespace BlackTempleHelpers
{
    // High Warlord Naj'entus
    // const Position NAJENTUS_TANK_POSITION = { 437.838f, 789.927f, 12.028f };
    const Position NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

    // Supremus
    const Position SUPREMUS_TANK_POSITION = { 704.651f, 684.401f, 72.608f };
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
    const Position GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f };
    const Position GOREFIEND_DIE_POSITION = { 525.709f, 377.177f, 193.203f };
    std::unordered_map<ObjectGuid, Position> gorefiendRangedPositions;

    // Gurtogg Bloodboil
    const Position GURTOGG_TANK_POSITION = { 735.987f, 272.451f, 63.554f };
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
    // N/A

    // Mother Shahraz
    const Position SHAHRAZ_TANK_POSITION = { 926.761f, 179.666f, 192.831f };
    const Position SHAHRAZ_TRANSITION_POSITION = { 941.557f, 182.719f, 192.418f };
    const Position SHAHRAZ_RANGED_POSITION = { 958.632f, 180.111f, 192.826f };
    std::unordered_map<ObjectGuid, uint8> shahrazTankStep;

    // Illidari Council
    const Position GATHIOS_TANK_POSITION_1 = { 662.977f, 296.246f, 271.688f };
    const Position GATHIOS_TANK_POSITION_2 = { 636.238f, 283.719f, 271.629f };
    const Position GATHIOS_TANK_POSITION_3 = { 655.571f, 261.377f, 271.687f };
    const Position GATHIOS_TANK_POSITION_4 = { 673.789f, 274.139f, 271.689f };
    const Position ZEREVOR_TANK_POSITION = { 686.219f, 377.644f, 271.689f };
    const Position ZEREVOR_HEALER_POSITION_1 = { 661.385f, 351.219f, 271.690f };
    const Position ZEREVOR_HEALER_POSITION_2 = { 667.003f, 363.768f, 271.690f };
    const Position MALANDE_TANK_POSITION = { 690.101f, 305.166f, 277.443f };
    const Position DARKSHADOW_TANK_POSITION = { 644.156f, 338.085f, 271.688f };
    std::unordered_map<uint32, time_t> councilDpsWaitTimer;
    std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
    std::unordered_map<ObjectGuid, uint8> zerevorHealStep;


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

    bool HasDangerousCouncilAura(Unit* unit)
    {
        static const uint32 dangerousAuras[] =
            { SPELL_CONSECRATION, SPELL_BLIZZARD, SPELL_FLAMESTRIKE };

        for (uint32 aura : dangerousAuras)
        {
            if (unit->HasAura(aura))
                return true;
        }

        return false;
    }

    // Illidan Stormrage <The Betrayer>
    const Position ILLIDAN_LANDING_POSITION = { 676.648f, 304.761f, 354.189f };
    const Position ILLIDAN_MAIN_TANK_POSITION_1 = { 0.0f, 0.0f, 0.0f, };
    const Position ILLIDAN_MAIN_TANK_POSITION_2 = { 0.0f, 0.0f, 0.0f, };
    const Position ILLIDAN_MAIN_TANK_POSITION_3 = { 0.0f, 0.0f, 0.0f, };
    const Position ILLIDAN_MAIN_TANK_POSITION_4 = { 0.0f, 0.0f, 0.0f, };
    const Position ILLIDAN_MAIN_TANK_POSITION_5 = { 0.0f, 0.0f, 0.0f, };
    const Position MAIN_TANK_POSITIONS[5] =
    {
        ILLIDAN_MAIN_TANK_POSITION_1,
        ILLIDAN_MAIN_TANK_POSITION_2,
        ILLIDAN_MAIN_TANK_POSITION_3,
        ILLIDAN_MAIN_TANK_POSITION_4,
        ILLIDAN_MAIN_TANK_POSITION_5,
    };
    const Position ILLIDAN_C_GRATE_POSITION = { 676.000f, 305.000f, 353.192f };
    const Position ILLIDAN_N_GRATE_POSITION = { 682.500f, 305.000f, 353.192f };
    const Position ILLIDAN_S_GRATE_POSITION = { 670.000f, 305.000f, 353.192f };
    const Position ILLIDAN_E_GRATE_POSITION = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_W_GRATE_POSITION = { 676.114f, 311.830f, 353.192f }; // as far out as I can get but too close to N and S spots
    const Position ILLIDAN_SW_GRATE_POSITION = { 672.828f, 311.496f, 353.192f };
    const Position ILLIDAN_SE_GRATE_POSITION = { 672.928f, 298.357f, 353.192f };
    const Position GRATE_POSITIONS[2] =
    {
        ILLIDAN_N_GRATE_POSITION,
        ILLIDAN_S_GRATE_POSITION,
        // ILLIDAN_E_GRATE_POSITION,
        // ILLIDAN_W_GRATE_POSITION,
    };
    const Position ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
    /* const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1 = { 697.097f, 287.966f, 353.983f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2 = { 689.442f, 282.678f, 354.303f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3 = { 680.054f, 279.540f, 354.089f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4 = { 670.698f, 279.068f, 353.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5 = { 661.933f, 282.331f, 354.848f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6 = { 659.002f, 298.441f, 354.110f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7 = { 664.911f, 291.331f, 354.225f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_8 = { 673.610f, 288.028f, 354.140f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_9 = { 682.190f, 289.148f, 354.136f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_10 = { 689.861f, 293.783f, 354.149f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_11 = { 694.867f, 300.910f, 354.186f }; */
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1 = { 696.969f, 300.982f, 354.302f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2 = { 691.112f, 287.461f, 354.363f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3 = { 676.674f, 280.797f, 354.268f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4 = { 664.414f, 284.834f, 354.271f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5 = { 656.826f, 295.113f, 354.165f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6 = { 670.884f, 293.726f, 353.907f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7 = { 681.942f, 293.551f, 353.192f };
    const Position E_GLAIVE_TANK_POSITIONS[7] =
    {
        ILLIDAN_E_GLAIVE_TANK_POSITION_1,
        ILLIDAN_E_GLAIVE_TANK_POSITION_2,
        ILLIDAN_E_GLAIVE_TANK_POSITION_3,
        ILLIDAN_E_GLAIVE_TANK_POSITION_4,
        ILLIDAN_E_GLAIVE_TANK_POSITION_5,
        ILLIDAN_E_GLAIVE_TANK_POSITION_6,
        ILLIDAN_E_GLAIVE_TANK_POSITION_7,
    };
    const Position ILLIDAN_W_GLAIVE_WAITING_POSITION = { 676.102f, 316.305f, 353.192f };
    /* const Position ILLIDAN_W_GLAIVE_TANK_POSITION_1 = { 697.363f, 323.860f, 353.948f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_2 = { 690.218f, 329.326f, 353.868f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_3 = { 681.647f, 322.243f, 353.813f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_4 = { 672.612f, 331.890f, 353.865f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_5 = { 663.833f, 329.083f, 353.783f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_6 = { 656.462f, 324.135f, 353.834f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_7 = { 657.054f, 314.865f, 354.156f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_8 = { 665.202f, 319.824f, 354.108f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_9 = { 674.817f, 322.193f, 354.125f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_10 = { 684.370f, 322.058f, 354.063f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_11 = { 692.023f, 315.866f, 354.127f }; */
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_1 = { 656.161f, 314.132f, 354.092f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_2 = { 665.080f, 326.905f, 354.128f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_3 = { 678.809f, 329.968f, 354.387f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_4 = { 690.889f, 324.277f, 354.204f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_5 = { 697.208f, 313.475f, 354.234f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_6 = { 682.285f, 316.558f, 353.187f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_7 = { 670.187f, 316.777f, 353.192f };
    const Position W_GLAIVE_TANK_POSITIONS[7] =
    {
        ILLIDAN_W_GLAIVE_TANK_POSITION_1,
        ILLIDAN_W_GLAIVE_TANK_POSITION_2,
        ILLIDAN_W_GLAIVE_TANK_POSITION_3,
        ILLIDAN_W_GLAIVE_TANK_POSITION_4,
        ILLIDAN_W_GLAIVE_TANK_POSITION_5,
        ILLIDAN_W_GLAIVE_TANK_POSITION_6,
        ILLIDAN_W_GLAIVE_TANK_POSITION_7,
    };
    const Position ILLIDAN_P4_WARLOCK_TANK_POSITION = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_1 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_2 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_3 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_4 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_5 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_6 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_7 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_8 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_9 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_RANGED_POSITION_10 = { 0.0f, 0.0f, 0.0f };
    const Position P4_RANGED_POSITIONS[10] =
    {
        ILLIDAN_P4_RANGED_POSITION_1,
        ILLIDAN_P4_RANGED_POSITION_2,
        ILLIDAN_P4_RANGED_POSITION_3,
        ILLIDAN_P4_RANGED_POSITION_4,
        ILLIDAN_P4_RANGED_POSITION_5,
        ILLIDAN_P4_RANGED_POSITION_6,
        ILLIDAN_P4_RANGED_POSITION_7,
        ILLIDAN_P4_RANGED_POSITION_8,
        ILLIDAN_P4_RANGED_POSITION_9,
        ILLIDAN_P4_RANGED_POSITION_10,
    };
    const Position ILLIDAN_P4_MELEE_POSITION_1 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_MELEE_POSITION_2 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_MELEE_POSITION_3 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_MELEE_POSITION_4 = { 0.0f, 0.0f, 0.0f };
    const Position ILLIDAN_P4_MELEE_POSITION_5 = { 0.0f, 0.0f, 0.0f };
    const Position P4_MELEE_POSITIONS[5] =
    {
        ILLIDAN_P4_MELEE_POSITION_1,
        ILLIDAN_P4_MELEE_POSITION_2,
        ILLIDAN_P4_MELEE_POSITION_3,
        ILLIDAN_P4_MELEE_POSITION_4,
        ILLIDAN_P4_MELEE_POSITION_5,
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
    std::unordered_map<uint32, time_t> illidanBossDpsWaitTimer;
    std::unordered_map<uint32, time_t> illidanFlameDpsWaitTimer;
    std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
    std::unordered_map<uint32, ObjectGuid> westFlameGuid;
    std::unordered_map<ObjectGuid, uint8> illidanGrateStep;

    int GetIllidanPhase(Unit* illidan)
    {
        if (!illidan || illidan->GetHealth() == 1)
            return -1;

        // Phase 1: Health > 65%
        if (illidan->GetHealthPct() > 65.0f)
            return 1;

        // Transitioning from Phase 2 to Phase 3
        float x, y, z;
        illidan->GetMotionMaster()->GetDestination(x, y, z);
        Position dest(x, y, z);
        if ((dest.GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
             illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f) &&
             illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && !illidan->HasAura(SPELL_SHADOW_PRISON))
             return 0;

        // Phase 2: Flying
        if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && !illidan->HasAura(SPELL_SHADOW_PRISON))
            return 2;

        // Phase 3: Normal (ground, 65-30%, not demon)
        if (illidan->GetHealthPct() > 30.0f && !illidan->HasAura(SPELL_DEMON_FORM) && !illidan->HasAura(SPELL_DEMON_TRANSFORM_1) &&
            !illidan->HasAura(SPELL_DEMON_TRANSFORM_2) && !illidan->HasAura(SPELL_DEMON_TRANSFORM_3))
            return 3;

        // Phase 4: Demon Form
        if (illidan->HasAura(SPELL_DEMON_FORM))
        {
            LOG_DEBUG("playerbots", "Illidan has SPELL_DEMON_FORM");
        }
        if (illidan->HasAura(SPELL_DEMON_TRANSFORM_1))
        {
            LOG_DEBUG("playerbots", "Illidan has SPELL_DEMON_TRANSFORM_1");
        }
        if (illidan->HasAura(SPELL_DEMON_TRANSFORM_2))
        {
            LOG_DEBUG("playerbots", "Illidan has SPELL_DEMON_TRANSFORM_2");
        }
        if (illidan->HasAura(SPELL_DEMON_TRANSFORM_3))
        {
            LOG_DEBUG("playerbots", "Illidan has SPELL_DEMON_TRANSFORM_3");
        }
        if (illidan->HasAura(SPELL_DEMON_FORM) || illidan->HasAura(SPELL_DEMON_TRANSFORM_1) ||
            illidan->HasAura(SPELL_DEMON_TRANSFORM_2) || illidan->HasAura(SPELL_DEMON_TRANSFORM_3))
            return 4;

        // Phase 5: Health <= 30%
        if (illidan->GetHealthPct() <= 30.0f)
            return 5;

        return -1;
    }

    std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(PlayerbotAI* botAI, Player* bot)
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

        const uint32 instanceId = bot->GetMap()->GetInstanceId();
        // If both GUIDs are not set and there are exactly 2 flames, assign by position
        if (eastFlameGuid.find(instanceId) == eastFlameGuid.end() &&
            westFlameGuid.find(instanceId) == westFlameGuid.end() &&
            flames.size() == 2)
        {
            float eastDist0 = flames[0]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);
            float eastDist1 = flames[1]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);

            if (eastDist0 < eastDist1)
            {
                eastFlameGuid[instanceId] = flames[0]->GetGUID();
                westFlameGuid[instanceId] = flames[1]->GetGUID();
            }
            else
            {
                eastFlameGuid[instanceId] = flames[1]->GetGUID();
                westFlameGuid[instanceId] = flames[0]->GetGUID();
            }
        }

        // Always return the flames by their snapshotted GUIDs
        for (Unit* unit : flames)
        {
            if (eastFlameGuid.find(instanceId) != eastFlameGuid.end() &&
                unit->GetGUID() == eastFlameGuid[instanceId])
                eastFlame = unit;
            else if (westFlameGuid.find(instanceId) != westFlameGuid.end() &&
                unit->GetGUID() == westFlameGuid[instanceId])
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

        float eyeBlastWidth = 9.0f; // Was using 10, trying 9
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
