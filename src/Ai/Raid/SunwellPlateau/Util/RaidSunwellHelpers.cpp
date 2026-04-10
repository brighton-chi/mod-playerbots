/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <array>

#include "RaidSunwellHelpers.h"
#include "RaidBossHelpers.h"
#include "Playerbots.h"

namespace SunwellHelpers
{
    // Kalecgos & Sathrovarr the Corruptor

    const Position KALECGOS_TANK_POSITION = { 1703.584f, 895.626f, 53.076f };
    const Position KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

    std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;

    KalecgosEncounterState& GetKalecgosEncounterState(uint32 instanceId)
    {
        return kalecgosEncounterStates[instanceId];
    }

    void ResetExpiredKalecgosRift(KalecgosEncounterState& state, uint32 now)
    {
        if (!state.activeRiftOpenedMs)
            return;

        if (getMSTimeDiff(state.activeRiftOpenedMs, now) <= KALECGOS_RIFT_ENTRY_WINDOW_MS)
            return;

        state.activeRiftOpenedMs = 0;
        state.activeRiftGroup = KALECGOS_INVALID_GROUP;
        state.blastedPlayerGuid = ObjectGuid::Empty;
        state.firstEntrantGuid = ObjectGuid::Empty;
    }

    uint8 GetAssignedGroup(const KalecgosEncounterState& state, ObjectGuid playerGuid)
    {
        auto assignment = state.playerToGroup.find(playerGuid);
        return assignment != state.playerToGroup.end() ? assignment->second : KALECGOS_INVALID_GROUP;
    }

    bool IsKalecgosActiveRiftCandidate(Player* bot, Player* candidate, KalecgosEncounterState& state)
    {
        if (!candidate->IsAlive() || candidate->GetMapId() != SUNWELL_MAP_ID)
            return false;

        if (!state.activeRiftOpenedMs || state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            return false;

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return true;

        return GetAssignedGroup(state, candidate->GetGUID()) == state.activeRiftGroup;
    }

    void AssignPlayerToGroup(KalecgosEncounterState& state, std::array<size_t, 3>& groupSizes,
        std::array<bool, 3>& groupHasTank, std::array<bool, 3>& groupHasDecurser,
        Player* member, uint8 groupIndex)
    {
        if (!member || groupIndex >= 3)
            return;

        state.playerToGroup[member->GetGUID()] = groupIndex;
        ++groupSizes[groupIndex];

        if (GET_PLAYERBOT_AI(member))
        {
            groupHasTank[groupIndex] = groupHasTank[groupIndex] || PlayerbotAI::IsTank(member, true);
            groupHasDecurser[groupIndex] = groupHasDecurser[groupIndex] ||
                IsKalecgosDecurser(GET_PLAYERBOT_AI(member), member);
        }
    }

    uint8 GetLeastFilledGroup(const std::array<size_t, 3>& groupSizes,
                              const std::array<bool, 3>* requiredFlags = nullptr,
                              bool preferMissingFlag = false)
    {
        uint8 bestGroup = KALECGOS_INVALID_GROUP;
        size_t smallestSize = std::numeric_limits<size_t>::max();

        for (uint8 groupIndex = 0; groupIndex < 3; ++groupIndex)
        {
            if (requiredFlags && preferMissingFlag && (*requiredFlags)[groupIndex])
                continue;

            if (groupSizes[groupIndex] < smallestSize)
            {
                bestGroup = groupIndex;
                smallestSize = groupSizes[groupIndex];
            }
        }

        if (bestGroup != KALECGOS_INVALID_GROUP || !requiredFlags || !preferMissingFlag)
            return bestGroup;

        return GetLeastFilledGroup(groupSizes);
    }

    bool IsKalecgosDecurser(PlayerbotAI* botAI, Player* bot)
    {
        switch (bot->getClass())
        {
            case CLASS_MAGE:
                return true;
            case CLASS_DRUID:
                return botAI->IsHeal(bot) || botAI->IsRangedDps(bot);
            case CLASS_SHAMAN:
                return botAI->IsHeal(bot);
            default:
                return false;
        }
    }

    void EnsureKalecgosGroupAssignments(PlayerbotAI* botAI, Player* bot)
    {
        if (!bot->GetGroup() || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        Group* group = bot->GetGroup();
        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        std::vector<Player*> members;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsInWorld() || !member->IsAlive() ||
                member->GetMapId() != SUNWELL_MAP_ID ||
                member->GetInstanceId() != bot->GetInstanceId())
            {
                continue;
            }

            members.push_back(member);
        }

        bool needsRebuild = state.playerToGroup.size() != members.size();
        if (!needsRebuild)
        {
            for (Player* member : members)
            {
                if (state.playerToGroup.find(member->GetGUID()) == state.playerToGroup.end())
                {
                    needsRebuild = true;
                    break;
                }
            }
        }

        if (!needsRebuild)
            return;

        state.playerToGroup.clear();

        std::array<size_t, 3> groupSizes = { 0, 0, 0 };
        std::array<bool, 3> groupHasTank = { false, false, false };
        std::array<bool, 3> groupHasDecurser = { false, false, false };

        Player* mainTank = GetGroupMainTank(botAI, bot);
        Player* assistTank0 = GetGroupAssistTank(botAI, bot, 0);
        Player* assistTank1 = GetGroupAssistTank(botAI, bot, 1);

        std::vector<Player*> extraTanks;

        for (Player* member : members)
        {
            if (member == mainTank || member == assistTank0 || member == assistTank1)
                continue;

            if (GET_PLAYERBOT_AI(member) && botAI->IsTank(member))
                extraTanks.push_back(member);
        }

        AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, mainTank, 0);
        AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, assistTank0, 1);
        AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, assistTank1, 2);

        if (mainTank)
            groupHasTank[0] = true;
        if (assistTank0)
            groupHasTank[1] = true;
        if (assistTank1)
            groupHasTank[2] = true;

        for (Player* tank : extraTanks)
        {
            uint8 groupIndex = GetLeastFilledGroup(groupSizes, &groupHasTank, true);
            AssignPlayerToGroup(state, groupSizes, groupHasTank,
                                groupHasDecurser, tank, groupIndex);
        }

        std::vector<Player*> decursers;
        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        std::vector<Player*> meleeDps;
        std::vector<Player*> others;
        std::vector<Player*> humans;

        for (Player* member : members)
        {
            if (state.playerToGroup.find(member->GetGUID()) != state.playerToGroup.end())
                continue;

            if (!GET_PLAYERBOT_AI(member))
                humans.push_back(member);
            else if (IsKalecgosDecurser(botAI, member))
                decursers.push_back(member);
            else if (botAI->IsHeal(member))
                healers.push_back(member);
            else if (botAI->IsRangedDps(member))
                rangedDps.push_back(member);
            else if (botAI->IsMelee(member) && botAI->IsDps(member))
                meleeDps.push_back(member);
            else
                others.push_back(member);
        }

        for (Player* decurser : decursers)
        {
            uint8 groupIndex = GetLeastFilledGroup(groupSizes, &groupHasDecurser, true);
            AssignPlayerToGroup(state, groupSizes, groupHasTank,
                                groupHasDecurser, decurser, groupIndex);
        }

        for (Player* human : humans)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, human, 0);

        for (Player* healer : healers)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, healer,
                                GetLeastFilledGroup(groupSizes));

        for (Player* ranged : rangedDps)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, ranged,
                                GetLeastFilledGroup(groupSizes));

        for (Player* melee : meleeDps)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, melee,
                                GetLeastFilledGroup(groupSizes));

        for (Player* other : others)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, other,
                                GetLeastFilledGroup(groupSizes));

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        {
            if (state.blastedPlayerGuid != ObjectGuid::Empty)
                state.activeRiftGroup = GetAssignedGroup(state, state.blastedPlayerGuid);
            else if (state.firstEntrantGuid != ObjectGuid::Empty)
                state.activeRiftGroup = GetAssignedGroup(state, state.firstEntrantGuid);
        }
    }

    uint8 GetKalecgosGroupAssignment(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return KALECGOS_INVALID_GROUP;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        EnsureKalecgosGroupAssignments(botAI, bot);
        return GetAssignedGroup(GetKalecgosEncounterState(bot->GetInstanceId()), bot->GetGUID());
    }

    bool IsKalecgosAssignedToActiveRift(PlayerbotAI* botAI, Player* bot, Player* candidate)
    {
        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        uint32 now = getMSTime();
        ResetExpiredKalecgosRift(state, now);
        if (!state.activeRiftOpenedMs)
            return false;

        EnsureKalecgosGroupAssignments(botAI, bot);
        return IsKalecgosActiveRiftCandidate(bot, candidate, state);
    }

    std::vector<Player*> GetKalecgosBotsAssignedToActiveRift(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Player*> enteringBots;

        if (!bot->GetGroup() || bot->GetMapId() != SUNWELL_MAP_ID)
            return enteringBots;

        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        uint32 now = getMSTime();
        ResetExpiredKalecgosRift(state, now);
        if (!state.activeRiftOpenedMs)
            return enteringBots;

        EnsureKalecgosGroupAssignments(botAI, bot);
        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            return enteringBots;

        Group* group = bot->GetGroup();
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !GET_PLAYERBOT_AI(member))
                continue;

            if (!IsKalecgosActiveRiftCandidate(bot, member, state))
                continue;

            enteringBots.push_back(member);
        }

        return enteringBots;
    }

    Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot)
    {
        std::array<Player*, 3> tankOrder = {
            GetGroupMainTank(botAI, bot),
            GetGroupAssistTank(botAI, bot, 0),
            GetGroupAssistTank(botAI, bot, 1)
        };

        for (Player* tank : tankOrder)
        {
            if (!tank || !tank->IsAlive() || tank->GetMapId() != SUNWELL_MAP_ID)
                continue;

            if (IsInKalecgosSpectralRealm(tank))
                continue;

            if (IsKalecgosAssignedToActiveRift(botAI, bot, tank))
                continue;

            return tank;
        }

        return nullptr;
    }

    bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot, Player* candidate)
    {
        if (!IsKalecgosAssignedToActiveRift(botAI, bot, candidate))
            return false;

        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return false;

        if (candidate->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)) ||
            IsInKalecgosSpectralRealm(candidate))
            return false;

        return true;
    }

    bool ShouldEnterKalecgosSpectralRift(Player* bot)
    {
        return ShouldEnterKalecgosSpectralRift(GET_PLAYERBOT_AI(bot), bot, bot);
    }

    bool IsInKalecgosSpectralRealm(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return false;

        auto stateItr = kalecgosEncounterStates.find(bot->GetInstanceId());
        if (stateItr == kalecgosEncounterStates.end())
            return false;

        auto playerState = stateItr->second.playerStates.find(bot->GetGUID());
        return playerState != stateItr->second.playerStates.end() &&
               playerState->second.inSpectralRealm;
    }

    void RecordKalecgosSpectralBlastPortal(PlayerbotAI* botAI, Player* bot)
    {
        if (!bot || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        uint32 now = getMSTime();

        ResetExpiredKalecgosRift(state, now);
        EnsureKalecgosGroupAssignments(botAI, bot);

        state.activeRiftOpenedMs = now;
        state.activeRiftSequence++;
        state.blastedPlayerGuid = bot->GetGUID();
        state.firstEntrantGuid = bot->GetGUID();
        state.activeRiftGroup = GetAssignedGroup(state, bot->GetGUID());
    }

    void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot)
    {
        if (!bot || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        uint32 now = getMSTime();

        ResetExpiredKalecgosRift(state, now);
        EnsureKalecgosGroupAssignments(botAI, bot);

        KalecgosPlayerState& playerState = state.playerStates[bot->GetGUID()];
        playerState.inSpectralRealm = true;
        playerState.lastEnterMs = now;

        if (!state.activeRiftOpenedMs)
        {
            state.activeRiftOpenedMs = now;
            state.activeRiftSequence++;
            state.blastedPlayerGuid = bot->GetGUID();
            state.firstEntrantGuid = bot->GetGUID();
        }

        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = bot->GetGUID();

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = GetAssignedGroup(state, bot->GetGUID());
    }

    void RecordKalecgosNormalRealmEnter(Player* bot)
    {
        if (!bot || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosPlayerState& playerState = GetKalecgosEncounterState(
            bot->GetInstanceId()).playerStates[bot->GetGUID()];
        playerState.inSpectralRealm = false;
        playerState.lastExitMs = getMSTime();
    }

    void RecordKalecgosSpectralExhaustion(Player* bot)
    {
        if (!bot || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        GetKalecgosEncounterState(bot->GetInstanceId()).playerStates[
            bot->GetGUID()].lastExhaustionMs = getMSTime();
    }
}
