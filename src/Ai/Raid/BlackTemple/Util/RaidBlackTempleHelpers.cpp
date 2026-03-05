/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleIllidanBossAI.h"
#include "Group.h"
#include "Playerbots.h"

namespace BlackTempleHelpers
{
    // High Warlord Naj'entus
    const Position NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

    // Supremus
    const Position SUPREMUS_TANK_POSITION = { 704.651f, 684.401f, 72.608f };

    std::unordered_map<ObjectGuid, Position> supremusRangedPositions;
    std::unordered_map<uint32, time_t> supremusPhaseTimer;

    bool HasSupremusVolcanoNearby(PlayerbotAI* botAI, Player* bot)
    {
        constexpr float searchRadius = 20.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, NPC_SUPREMUS_VOLCANO, searchRadius);

        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
                return true;
        }

        return false;
    }

    // Teron Gorefiend
    const Position GOREFIEND_TANK_POSITION = { 597.653f, 402.284f, 187.090f };
    const Position GOREFIEND_DIE_POSITION = { 525.709f, 377.177f, 193.203f };

    // Gurtogg Bloodboil
    const Position GURTOGG_TANK_POSITION = { 735.987f, 272.451f, 63.554f };
    const Position GURTOGG_RANGED_POSITION = { 762.265f, 277.183f, 63.781f };
    const Position GURTOGG_SOAKER_POSITION = { 769.348f, 280.116f, 63.780f };

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

    // Mother Shahraz
    const Position SHAHRAZ_TANK_POSITION = { 928.553f, 219.060f, 192.846f };
    const Position SHAHRAZ_TRANSITION_POSITION = { 941.899f, 184.689f, 192.328f };
    const Position SHAHRAZ_RANGED_POSITION = { 959.963f, 210.571f, 192.849f };
    std::unordered_map<ObjectGuid, uint8> shahrazTankStep;

    // Illidari Council
    const Position GATHIOS_TANK_POSITION_1 = { 662.977f, 296.246f, 271.688f };
    const Position GATHIOS_TANK_POSITION_2 = { 636.238f, 283.719f, 271.629f };
    const Position GATHIOS_TANK_POSITION_3 = { 655.571f, 261.377f, 271.687f };
    const Position GATHIOS_TANK_POSITION_4 = { 673.789f, 274.139f, 271.689f };
    const std::array<Position, 4> GATHIOS_TANK_POSITIONS =
    {
        GATHIOS_TANK_POSITION_1,
        GATHIOS_TANK_POSITION_2,
        GATHIOS_TANK_POSITION_3,
        GATHIOS_TANK_POSITION_4
    };
    const Position ZEREVOR_TANK_POSITION = { 686.219f, 377.644f, 271.689f };
    const Position ZEREVOR_HEALER_POSITION_1 = { 661.385f, 351.219f, 271.690f };
    const Position ZEREVOR_HEALER_POSITION_2 = { 667.003f, 363.768f, 271.690f };
    const std::array<Position, 2> ZEREVOR_HEALER_POSITIONS =
    {
        ZEREVOR_HEALER_POSITION_1,
        ZEREVOR_HEALER_POSITION_2
    };
    // const Position MALANDE_TANK_POSITION = { 690.101f, 305.166f, 277.443f };
    const Position MALANDE_TANK_POSITION = { 701.059f, 287.306f, 277.443f };
    const Position MALANDE_PULL_POSITION = {712.401f, 264.761f, 277.443f };

    std::unordered_map<uint32, time_t> councilDpsWaitTimer;
    std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
    std::unordered_map<ObjectGuid, uint8> zerevorHealStep;

    // (1) First priority is an assistant Mage (real player or bot)
    // (2) If no assistant Mage, then look for any Mage bot
    Player* GetZerevorMageTank(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* fallbackMage = nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;

            if (!fallbackMage && GET_PLAYERBOT_AI(member))
                fallbackMage = member;
        }

        return fallbackMage;
    }

    bool HasDangerousCouncilAura(Unit* unit)
    {
        static const std::array<uint32, 3> dangerousAuras =
        {
            SPELL_CONSECRATION, SPELL_BLIZZARD, SPELL_FLAMESTRIKE
        };

        for (uint32 aura : dangerousAuras)
        {
            if (unit->HasAura(aura))
                return true;
        }

        return false;
    }

    // Illidan Stormrage <The Betrayer>
    const Position ILLIDAN_LANDING_POSITION = { 676.648f, 304.761f, 354.189f };
    const Position ILLIDAN_N_GRATE_POSITION = { 682.500f, 305.000f, 353.192f };
    const Position ILLIDAN_S_GRATE_POSITION = { 670.000f, 305.000f, 353.192f };
    const std::array<Position, 2> GRATE_POSITIONS =
    {
        ILLIDAN_N_GRATE_POSITION,
        ILLIDAN_S_GRATE_POSITION,
    };

    const Position ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1 =  { 696.969f, 300.982f, 354.302f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2 =  { 691.112f, 287.461f, 354.363f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3 =  { 676.674f, 280.797f, 354.268f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4 =  { 664.414f, 284.834f, 354.271f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5 =  { 656.826f, 295.113f, 354.165f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6 =  { 670.884f, 293.726f, 353.907f };
    const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7 =  { 681.942f, 293.551f, 353.192f };
    const std::array<Position, 7> E_GLAIVE_TANK_POSITIONS =
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
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_1 =  { 656.161f, 314.132f, 354.092f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_2 =  { 665.080f, 326.905f, 354.128f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_3 =  { 678.809f, 329.968f, 354.387f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_4 =  { 690.889f, 324.277f, 354.204f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_5 =  { 697.208f, 313.475f, 354.234f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_6 =  { 682.285f, 316.558f, 353.187f };
    const Position ILLIDAN_W_GLAIVE_TANK_POSITION_7 =  { 670.187f, 316.777f, 353.192f };
    const std::array<Position, 7> W_GLAIVE_TANK_POSITIONS =
    {
        ILLIDAN_W_GLAIVE_TANK_POSITION_1,
        ILLIDAN_W_GLAIVE_TANK_POSITION_2,
        ILLIDAN_W_GLAIVE_TANK_POSITION_3,
        ILLIDAN_W_GLAIVE_TANK_POSITION_4,
        ILLIDAN_W_GLAIVE_TANK_POSITION_5,
        ILLIDAN_W_GLAIVE_TANK_POSITION_6,
        ILLIDAN_W_GLAIVE_TANK_POSITION_7,
    };

    const std::array<Position, MAX_EYE_BEAM_POS * 2> eyeBeamPos =
    {{
        {639.97f, 301.63f, 354.0f, 0.0f},
        {658.83f, 265.10f, 354.0f, 0.0f},
        {656.86f, 344.07f, 354.0f, 0.0f},
        {640.70f, 310.47f, 354.0f, 0.0f},
        {706.22f, 273.26f, 354.0f, 0.0f},
        {717.55f, 328.33f, 354.0f, 0.0f},
        {718.06f, 286.08f, 354.0f, 0.0f},
        {705.92f, 337.14f, 354.0f, 0.0f}
    }};

    std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
    std::unordered_map<uint32, time_t> illidanBossDpsWaitTimer;
    std::unordered_map<uint32, time_t> illidanFlameDpsWaitTimer;
    std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
    std::unordered_map<uint32, ObjectGuid> westFlameGuid;

    int GetIllidanPhase(Unit* illidan)
    {
        if (!illidan || illidan->GetHealth() == 1 || illidan->HasAura(SPELL_SHADOW_PRISON))
            return -1;

        // Transitioning from Phase 2 to Phase 3
        float x, y, z;
        illidan->GetMotionMaster()->GetDestination(x, y, z);
        Position dest(x, y, z);
        if ((dest.GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
             illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f) &&
             illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
             return 0;

        // Phase 2: Flying
        if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return 2;

        // Phase 1: Health > 65%
        if (illidan->GetHealthPct() > 65.0f)
            return 1;

        // Phase 4: Demon Form
        if (illidan->HasAura(SPELL_DEMON_FORM) || illidan->HasAura(SPELL_DEMON_TRANSFORM_1) ||
            illidan->HasAura(SPELL_DEMON_TRANSFORM_2) || illidan->HasAura(SPELL_DEMON_TRANSFORM_3))
            return 4;

        // Phase 3: Normal (ground, 65-30%, not demon)
        if (illidan->GetHealthPct() > 30.0f)
            return 3;

        // Phase 5: Health <= 30%
        if (illidan->GetHealthPct() <= 30.0f)
            return 5;

        return -1;
    }

    std::vector<Unit*> GetAllFlameCrashes(Player* bot)
    {
        std::vector<Unit*> flameCrashes;
        std::list<Creature*> creatureList;
        constexpr float searchRadius = 30.0f;
        bot->GetCreatureListWithEntryInGrid(creatureList, NPC_FLAME_CRASH, searchRadius);

        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
                flameCrashes.push_back(creature);
        }

        return flameCrashes;
    }

    std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(PlayerbotAI* /*botAI*/, Player* bot)
    {
        Unit* eastFlame = nullptr;
        Unit* westFlame = nullptr;

        const uint32 instanceId = bot->GetMap()->GetInstanceId();

        if (eastFlameGuid.find(instanceId) != eastFlameGuid.end())
        {
            if (Unit* unit = ObjectAccessor::GetUnit(*bot, eastFlameGuid[instanceId]))
            {
                if (unit->IsAlive())
                    eastFlame = unit;
            }
        }

        if (westFlameGuid.find(instanceId) != westFlameGuid.end())
        {
            if (Unit* unit = ObjectAccessor::GetUnit(*bot, westFlameGuid[instanceId]))
            {
                if (unit->IsAlive())
                    westFlame = unit;
            }
        }

        if (!eastFlame || !westFlame)
        {
            LOG_DEBUG("playerbots", "[BT] Bot {} (GUID: {}) GetFlamesOfAzzinoth missing flames! eastFlame={} westFlame={}",
                bot->GetName(),
                bot->GetGUID().ToString(),
                eastFlame ? "FOUND" : "NONE",
                westFlame ? "FOUND" : "NONE"
            );
        }

        return { eastFlame, westFlame };
    }

    // (1) First priority is an assistant Warlock (real player or bot)
    // (2) If no assistant Warlock, then look for any Warlock bot
    Player* GetIllidanWarlockTank(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* fallbackWarlock = nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;

            if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
                fallbackWarlock = member;
        }

        return fallbackWarlock;
    }

    EyeBlastDangerArea GetEyeBlastDangerArea(PlayerbotAI* botAI, Player* bot, Unit* illidan)
    {
        boss_illidan_stormrage* illidanAI = dynamic_cast<boss_illidan_stormrage*>(illidan->GetAI());
        if (!illidanAI)
            return {};

        uint8 beamPosId = illidanAI->GetBeamPosId();

        constexpr float searchRadius = 100.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, NPC_ILLIDAN_DB_TARGET, searchRadius);

        Creature* eyeBlastTrigger = nullptr;
        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
            {
                eyeBlastTrigger = creature;
                break;
            }
        }

        if (!eyeBlastTrigger)
            return {};

        Position startPos = Position(eyeBlastTrigger->GetPositionX(), eyeBlastTrigger->GetPositionY(),
                                     eyeBlastTrigger->GetPositionZ());
        Position endPos = eyeBeamPos[beamPosId + MAX_EYE_BEAM_POS];

        constexpr float eyeBlastWidth = 9.0f;
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
