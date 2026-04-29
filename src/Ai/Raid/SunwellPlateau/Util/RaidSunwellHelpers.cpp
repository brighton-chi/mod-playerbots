/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <cmath>
#include <list>

#include "CellImpl.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "RaidSunwellHelpers.h"
#include "RaidBossHelpers.h"
#include "Playerbots.h"
#include "Spell.h"
#include "Timer.h"
#include "ThreatManager.h"

namespace SunwellHelpers
{
    // Trash

    Creature* GetInfernalDefenseApocalypseGuard(Player* bot)
    {
        Creature* target = nullptr;
        constexpr float searchRadius = 40.0f;
        std::list<Creature*> apocalypseGuards;
        bot->GetCreatureListWithEntryInGrid(
            apocalypseGuards, static_cast<uint32>(SunwellNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

        for (Creature* apocalypseGuard : apocalypseGuards)
        {
            if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
                !apocalypseGuard->HasAura(static_cast<uint32>(SunwellSpells::SPELL_INFERNAL_DEFENSE)))
            {
                continue;
            }

            if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
                target = apocalypseGuard;
        }

        return target;
    }

    // Kalecgos

    const Position KALECGOS_TANK_POSITION =           { 1703.584f, 895.626f, 53.076f };
    const Position KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

    std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
    std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;
    std::unordered_set<ObjectGuid> hasReachedKalecgosInitialRangedPosition;

    void ClearExpiredKalecgosActiveRift(KalecgosEncounterState& state, uint32 now)
    {
        if (!state.activeRiftOpenedMs)
            return;

        constexpr uint32 riftEntryWindowMs = 10000;
        if (getMSTimeDiff(state.activeRiftOpenedMs, now) <= riftEntryWindowMs)
            return;

        state.activeRiftOpenedMs = 0;
        state.activeRiftGroup = KALECGOS_INVALID_GROUP;
        state.blastedPlayerGuid = ObjectGuid::Empty;
        state.firstEntrantGuid = ObjectGuid::Empty;
        state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    }

    uint8 GetKalecgosAssignedGroup(const KalecgosEncounterState& state, ObjectGuid playerGuid)
    {
        auto assignment = state.playerToGroup.find(playerGuid);
        return assignment != state.playerToGroup.end() ? assignment->second : KALECGOS_INVALID_GROUP;
    }

    Player* FindKalecgosGroupMember(Group* group, ObjectGuid playerGuid)
    {
        if (playerGuid == ObjectGuid::Empty || !group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != playerGuid)
                continue;

            if (member->GetMapId() != SUNWELL_MAP_ID)
                return nullptr;

            return member;
        }

        return nullptr;
    }

    KalecgosEncounterState& GetPreparedKalecgosEncounterState(PlayerbotAI* botAI, Player* bot)
    {
        KalecgosEncounterState& state = kalecgosEncounterStates[bot->GetInstanceId()];
        ClearExpiredKalecgosActiveRift(state, getMSTime());
        EnsureKalecgosGroupAssignments(botAI, bot);
        return state;
    }

    bool IsKalecgosActiveRiftCandidate(Player* candidate, KalecgosEncounterState const& state)
    {
        if (!candidate || !candidate->IsAlive() || candidate->GetMapId() != SUNWELL_MAP_ID)
            return false;

        if (!state.activeRiftOpenedMs || state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            return false;

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return true;

        return GetKalecgosAssignedGroup(state, candidate->GetGUID()) == state.activeRiftGroup;
    }

    bool IsKalecgosPortalEligibleCandidate(Player* candidate)
    {
        if (!candidate || !candidate->IsAlive() || !GET_PLAYERBOT_AI(candidate) ||
            candidate->GetMapId() != SUNWELL_MAP_ID)
            return false;

        if (candidate->HasAura(
                static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
        {
            return false;
        }

        return !IsInKalecgosSpectralRealm(candidate);
    }

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> GetExpectedKalecgosTankAssignmentGuids(
        PlayerbotAI* botAI, Player* bot)
    {
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };

        Group* group = bot->GetGroup();
        if (!group)
            return tankGuids;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;

            if (botAI->IsMainTank(member))
                tankGuids[0] = member->GetGUID();
            else if (botAI->IsAssistTankOfIndex(member, 0))
                tankGuids[1] = member->GetGUID();
            else if (botAI->IsAssistTankOfIndex(member, 1))
                tankGuids[2] = member->GetGUID();
        }

        return tankGuids;
    }

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> BuildInitialKalecgosTankPortalRotationGuids(
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
    {
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };
        uint8 nextIndex = 0;

        auto appendGuid = [&](ObjectGuid guid)
        {
            if (guid == ObjectGuid::Empty || nextIndex >= KALECGOS_TANK_COUNT)
                return;

            if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
                rotationGuids[nextIndex++] = guid;
        };

        appendGuid(tankAssignmentGuids[2]);
        appendGuid(tankAssignmentGuids[1]);
        appendGuid(tankAssignmentGuids[0]);

        return rotationGuids;
    }

    bool HasKalecgosTankAssignment(
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids, ObjectGuid guid)
    {
        if (guid == ObjectGuid::Empty)
            return false;

        return std::find(tankAssignmentGuids.begin(), tankAssignmentGuids.end(), guid) !=
               tankAssignmentGuids.end();
    }

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> RebuildKalecgosTankPortalRotationGuids(
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& existingRotationGuids,
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
    {
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };
        uint8 nextIndex = 0;

        auto appendGuid = [&](ObjectGuid guid)
        {
            if (!HasKalecgosTankAssignment(tankAssignmentGuids, guid) || nextIndex >= KALECGOS_TANK_COUNT)
                return;

            if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
                rotationGuids[nextIndex++] = guid;
        };

        for (ObjectGuid guid : existingRotationGuids)
            appendGuid(guid);

        for (ObjectGuid guid : BuildInitialKalecgosTankPortalRotationGuids(tankAssignmentGuids))
            appendGuid(guid);

        return rotationGuids;
    }

    bool IsKalecgosAssignedTank(KalecgosEncounterState const& state, Player* candidate)
    {
        return candidate && HasKalecgosTankAssignment(state.tankAssignmentGuids, candidate->GetGUID());
    }

    Player* GetKalecgosSurfaceAssignedTank(Group* group, ObjectGuid guid)
    {
        Player* tank = FindKalecgosGroupMember(group, guid);
        if (!tank || !tank->IsAlive() || tank->GetMapId() != SUNWELL_MAP_ID)
            return nullptr;

        if (IsInKalecgosSpectralRealm(tank))
            return nullptr;

        return tank;
    }

    Player* GetFirstKalecgosSurfaceAssignedTank(
        Group* group, KalecgosEncounterState const& state, ObjectGuid excludedGuid = ObjectGuid::Empty)
    {
        for (ObjectGuid guid : state.tankAssignmentGuids)
        {
            if (guid == excludedGuid)
                continue;

            if (Player* tank = GetKalecgosSurfaceAssignedTank(group, guid))
                return tank;
        }

        return nullptr;
    }

    uint8 CountKalecgosSurfaceAssignedTanks(Group* group, KalecgosEncounterState const& state)
    {
        uint8 count = 0;
        for (ObjectGuid guid : state.tankAssignmentGuids)
        {
            if (GetKalecgosSurfaceAssignedTank(group, guid))
                ++count;
        }

        return count;
    }

    Player* GetNextKalecgosSurfaceTankInPortalRotation(
        Group* group, KalecgosEncounterState const& state, ObjectGuid afterGuid)
    {
        uint8 startIndex = 0;
        for (uint8 index = 0; index < KALECGOS_TANK_COUNT; ++index)
        {
            if (state.tankPortalRotationGuids[index] == afterGuid)
            {
                startIndex = (index + 1) % KALECGOS_TANK_COUNT;
                break;
            }
        }

        for (uint8 offset = 0; offset < KALECGOS_TANK_COUNT; ++offset)
        {
            ObjectGuid guid = state.tankPortalRotationGuids[(startIndex + offset) % KALECGOS_TANK_COUNT];
            if (guid == ObjectGuid::Empty || guid == afterGuid)
                continue;

            if (Player* tank = GetKalecgosSurfaceAssignedTank(group, guid))
                return tank;
        }

        return nullptr;
    }

    Player* GetKalecgosCurrentVictimTank(
        PlayerbotAI* botAI, Player* bot, Group* group, KalecgosEncounterState const& state)
    {
        AiObjectContext* context = botAI->GetAiObjectContext();
        Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
        Player* currentVictim = kalecgos && kalecgos->GetVictim() ? kalecgos->GetVictim()->ToPlayer() : nullptr;

        if (currentVictim && IsKalecgosAssignedTank(state, currentVictim) &&
            currentVictim->IsAlive() && currentVictim->GetMapId() == SUNWELL_MAP_ID &&
            !IsInKalecgosSpectralRealm(currentVictim))
        {
            return currentVictim;
        }

        return GetFirstKalecgosSurfaceAssignedTank(group, state);
    }

    Player* SelectKalecgosOutgoingTankForRift(
        Group* group, KalecgosEncounterState const& state)
    {
        if (!state.activeRiftOpenedMs)
            return nullptr;

        if (HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid))
            return nullptr;

        if (CountKalecgosSurfaceAssignedTanks(group, state) <= 2)
            return nullptr;

        for (ObjectGuid guid : state.tankPortalRotationGuids)
        {
            Player* tank = GetKalecgosSurfaceAssignedTank(group, guid);
            if (!tank || !IsKalecgosPortalEligibleCandidate(tank))
                continue;

            return tank;
        }

        return nullptr;
    }

    void AssignKalecgosTankTargetsForActiveRift(
        PlayerbotAI* botAI, Player* bot, Group* group, KalecgosEncounterState& state)
    {
        Player* currentTank = GetKalecgosCurrentVictimTank(botAI, bot, group, state);
        Player* outgoingTank = SelectKalecgosOutgoingTankForRift(group, state);

        state.activeRiftOutgoingTankGuid = outgoingTank ? outgoingTank->GetGUID() : ObjectGuid::Empty;

        if (outgoingTank && currentTank && outgoingTank->GetGUID() == currentTank->GetGUID())
            currentTank = GetNextKalecgosSurfaceTankInPortalRotation(group, state, outgoingTank->GetGUID());

        if (!currentTank)
            currentTank = GetFirstKalecgosSurfaceAssignedTank(group, state, state.activeRiftOutgoingTankGuid);

        state.currentTankGuid = currentTank ? currentTank->GetGUID() : ObjectGuid::Empty;
    }

    void AdvanceKalecgosTankPortalRotation(KalecgosEncounterState& state, ObjectGuid tankGuid)
    {
        if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, tankGuid))
            return;

        std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };
        uint8 nextIndex = 0;

        for (ObjectGuid guid : state.tankPortalRotationGuids)
        {
            if (guid == ObjectGuid::Empty || guid == tankGuid)
                continue;

            rotationGuids[nextIndex++] = guid;
        }

        if (nextIndex < KALECGOS_TANK_COUNT)
            rotationGuids[nextIndex] = tankGuid;

        state.tankPortalRotationGuids = RebuildKalecgosTankPortalRotationGuids(
            rotationGuids, state.tankAssignmentGuids);
    }

    Player* GetKalecgosOutgoingTank(
        Group* group, KalecgosEncounterState const& state)
    {
        if (!state.activeRiftOpenedMs || state.activeRiftOutgoingTankGuid == ObjectGuid::Empty)
            return nullptr;

        return GetKalecgosSurfaceAssignedTank(group, state.activeRiftOutgoingTankGuid);
    }

    bool CanKalecgosBotEnterRift(
        Group* group, Player* candidate, KalecgosEncounterState const& state)
    {
        if (IsKalecgosAssignedTank(state, candidate))
            return false;

        if (!IsKalecgosPortalEligibleCandidate(candidate))
            return false;

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return false;

        return true;
    }

    uint8 GetNextAvailableKalecgosGroup(
        Group* group, KalecgosEncounterState const& state)
    {
        if (!group)
            return KALECGOS_INVALID_GROUP;

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member)
                    continue;

                if (GetKalecgosAssignedGroup(state, member->GetGUID()) != groupIndex)
                    continue;

                if (CanKalecgosBotEnterRift(group, member, state))
                    return groupIndex;
            }
        }

        return KALECGOS_INVALID_GROUP;
    }

    uint8 ResolveKalecgosActiveRiftGroup(
        Group* group, KalecgosEncounterState const& state)
    {
        if (state.blastedPlayerGuid != ObjectGuid::Empty)
        {
            uint8 blastedGroup = GetKalecgosAssignedGroup(state, state.blastedPlayerGuid);
            if (blastedGroup != KALECGOS_INVALID_GROUP)
                return blastedGroup;

            return GetNextAvailableKalecgosGroup(group, state);
        }

        if (state.firstEntrantGuid != ObjectGuid::Empty)
        {
            uint8 entrantGroup = GetKalecgosAssignedGroup(state, state.firstEntrantGuid);
            if (entrantGroup != KALECGOS_INVALID_GROUP)
                return entrantGroup;

            return GetNextAvailableKalecgosGroup(group, state);
        }

        return KALECGOS_INVALID_GROUP;
    }

    void AssignPlayerToGroup(
        KalecgosEncounterState& state, std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
        std::array<bool, KALECGOS_GROUP_COUNT>& groupHasTank,
        std::array<bool, KALECGOS_GROUP_COUNT>& groupHasDecurser, Player* member, uint8 groupIndex)
    {
        if (!member || groupIndex >= KALECGOS_GROUP_COUNT)
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

    uint8 GetLeastFilledGroup(
        const std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
        const std::array<bool, KALECGOS_GROUP_COUNT>* requiredFlags = nullptr,
        bool preferMissingFlag = false)
    {
        uint8 bestGroup = KALECGOS_INVALID_GROUP;
        size_t smallestSize = std::numeric_limits<size_t>::max();

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
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
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = kalecgosEncounterStates[instanceId];
        std::vector<Player*> botMembers;
        std::array<ObjectGuid, KALECGOS_TANK_COUNT> expectedTankAssignmentGuids =
            GetExpectedKalecgosTankAssignmentGuids(botAI, bot);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID)
                continue;

            if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member, true))
                botMembers.push_back(member);
        }

        bool needsRebuild = state.playerToGroup.size() != botMembers.size() ||
                            state.tankAssignmentGuids != expectedTankAssignmentGuids;
        if (!needsRebuild)
        {
            for (Player* member : botMembers)
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
        state.tankAssignmentGuids = expectedTankAssignmentGuids;
        state.tankPortalRotationGuids = RebuildKalecgosTankPortalRotationGuids(
            state.tankPortalRotationGuids, state.tankAssignmentGuids);

        if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.currentTankGuid))
        {
            if (Player* fallbackTank = GetFirstKalecgosSurfaceAssignedTank(group, state))
                state.currentTankGuid = fallbackTank->GetGUID();
            else
                state.currentTankGuid = ObjectGuid::Empty;
        }

        if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.activeRiftOutgoingTankGuid))
            state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

        std::array<size_t, KALECGOS_GROUP_COUNT> groupSizes = {
            0, 0, 0, 0 };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasTank = {
            false, false, false, false };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasDecurser = {
            false, false, false, false };

        std::vector<Player*> decursers;
        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        std::vector<Player*> meleeDps;
        std::vector<Player*> others;

        for (Player* member : botMembers)
        {
            if (state.playerToGroup.find(member->GetGUID()) != state.playerToGroup.end())
                continue;

            if (IsKalecgosDecurser(botAI, member))
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

        for (Player* healer : healers)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser,
                                healer, GetLeastFilledGroup(groupSizes));

        for (Player* ranged : rangedDps)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser,
                                ranged, GetLeastFilledGroup(groupSizes));

        for (Player* melee : meleeDps)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser,
                                melee, GetLeastFilledGroup(groupSizes));

        for (Player* other : others)
            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser,
                                other, GetLeastFilledGroup(groupSizes));

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        {
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
        }
    }

    bool HasReachedKalecgosInitialRangedPosition(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return false;

        return hasReachedKalecgosInitialRangedPosition.find(bot->GetGUID()) !=
               hasReachedKalecgosInitialRangedPosition.end();
    }

    void SetKalecgosInitialRangedPositionReached(Player* bot, bool reached)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        if (reached)
            hasReachedKalecgosInitialRangedPosition.insert(bot->GetGUID());
        else
            hasReachedKalecgosInitialRangedPosition.erase(bot->GetGUID());
    }

    Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);

        if (Player* tank = GetKalecgosSurfaceAssignedTank(group, state.currentTankGuid))
            return tank;

        if (Player* fallbackTank = GetFirstKalecgosSurfaceAssignedTank(group, state))
        {
            state.currentTankGuid = fallbackTank->GetGUID();
            return fallbackTank;
        }

        state.currentTankGuid = ObjectGuid::Empty;
        return nullptr;
    }

    bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);
        if (!state.activeRiftOpenedMs)
            return false;

        if (IsKalecgosAssignedTank(state, bot))
        {
            return GetKalecgosOutgoingTank(group, state) == bot &&
                   state.blastedPlayerGuid != bot->GetGUID() &&
                   IsKalecgosPortalEligibleCandidate(bot);
        }

        if (!IsKalecgosActiveRiftCandidate(bot, state))
            return false;

        return CanKalecgosBotEnterRift(group, bot, state);
    }

    bool IsInKalecgosSpectralRealm(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return false;

        auto realmStateItr = kalecgosRealmStates.find(bot->GetGUID());
        if (realmStateItr == kalecgosRealmStates.end())
            return false;

        return realmStateItr->second.inSpectralRealm;
    }

    bool IsKalecgosRealmTransitionGraceActive(Player* bot)
    {
        if (!bot || bot->GetMapId() != SUNWELL_MAP_ID ||
            bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)))
        {
            return false;
        }

        auto realmStateItr = kalecgosRealmStates.find(bot->GetGUID());
        if (realmStateItr == kalecgosRealmStates.end())
            return false;

        uint32 now = getMSTime();
        constexpr uint32 realmTransitionGraceMs = 2000;
        KalecgosRealmState const& realmState = realmStateItr->second;
        return (realmState.lastEnterMs &&
                getMSTimeDiff(realmState.lastEnterMs, now) < realmTransitionGraceMs) ||
               (realmState.lastExitMs &&
                getMSTimeDiff(realmState.lastExitMs, now) < realmTransitionGraceMs);
    }

    void UpdateKalecgosRealmState(Player* bot, bool inSpectralRealm, uint32 timestamp)
    {
        KalecgosRealmState& realmState = kalecgosRealmStates[bot->GetGUID()];
        realmState.inSpectralRealm = inSpectralRealm;

        if (inSpectralRealm)
            realmState.lastEnterMs = timestamp;
        else
            realmState.lastExitMs = timestamp;

        SetKalecgosInitialRangedPositionReached(bot, false);
    }

    void RecordKalecgosSpectralBlastTarget(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);
        uint32 now = getMSTime();

        state.activeRiftOpenedMs = now;
        state.blastedPlayerGuid = bot->GetGUID();
        state.firstEntrantGuid = ObjectGuid::Empty;
        state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
        AssignKalecgosTankTargetsForActiveRift(botAI, bot, group, state);
    }

    void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);
        uint32 now = getMSTime();

        UpdateKalecgosRealmState(bot, true, now);

        if (state.activeRiftOpenedMs)
        {
            if (state.firstEntrantGuid == ObjectGuid::Empty)
                state.firstEntrantGuid = bot->GetGUID();

            if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
                state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
        }

        if (IsKalecgosAssignedTank(state, bot))
        {
            AdvanceKalecgosTankPortalRotation(state, bot->GetGUID());

            if (state.activeRiftOutgoingTankGuid == bot->GetGUID())
                state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

            if (state.currentTankGuid == bot->GetGUID())
            {
                if (Player* nextTank = GetFirstKalecgosSurfaceAssignedTank(group, state, bot->GetGUID()))
                    state.currentTankGuid = nextTank->GetGUID();
                else
                    state.currentTankGuid = ObjectGuid::Empty;
            }
        }
    }

    void RecordKalecgosNormalRealmEnter(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        UpdateKalecgosRealmState(bot, false, getMSTime());
    }

    // Brutallus

    const Position BRUTALLUS_MAIN_TANK_POSITION = { 1484.779f, 582.691f, 23.460f };

    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
    std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

    bool ShouldMoveForBrutallusBurn(Player* bot)
    {
        Aura* burnAura = bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_BURN));
        if (!burnAura)
            return false;

        constexpr int32 burnMoveLeadTimeMs = 45000;
        return burnAura->GetDuration() < burnMoveLeadTimeMs;
    }

    Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z)
    {
        if (isMainTank)
            return { BRUTALLUS_MAIN_TANK_POSITION.GetPositionX(),
                     BRUTALLUS_MAIN_TANK_POSITION.GetPositionY(), z };

        float angle = GetBrutallusMainTankAngle(brutallus);
        angle = Position::NormalizeOrientation(angle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        return GetBrutallusPositionAtAngle(brutallus, angle, BRUTALLUS_TANK_POSITION_RADIUS, z);
    }

    bool TryGetBrutallusMeleePosition(
        Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        constexpr float meleeSpacing = 5.0f;
        constexpr float arcAngle = 2.0f * M_PI / 3.0f;

        float meleeRadius = std::max(1.0f, bot->GetMeleeRange(brutallus) - 2.0f);
        float meleeAngleStep = 2.0f * std::asin(meleeSpacing / (2.0f * meleeRadius));
        uint8 maxSideSlots = static_cast<uint8>(std::floor((arcAngle / 2.0f) / meleeAngleStep));
        uint8 maxMeleeSlots = 1 + 2 * maxSideSlots;
        if (meleeIndex >= maxMeleeSlots)
            return false;

        float arcCenterOffset = M_PI + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f;
        float baseAngle = Position::NormalizeOrientation(
            GetBrutallusMainTankAngle(brutallus) + arcCenterOffset);
        float arcWidth = maxSideSlots * 2.0f * meleeAngleStep;
        float angleOffset = GetCenteredArcSlotAngleOffset(meleeIndex, maxMeleeSlots, arcWidth);

        float angle = Position::NormalizeOrientation(baseAngle + angleOffset);
        position = GetBrutallusPositionAtAngle(brutallus, angle, meleeRadius, z);
        return true;
    }

    bool TryGetBrutallusAssignedPositionIndex(
        PlayerbotAI* botAI, Player* bot, bool wantRanged, uint8& positionIndex)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        if (wantRanged)
        {
            EnsureBrutallusRangedAssignments(botAI, bot);

            auto instanceItr = brutallusRangedAssignments.find(bot->GetInstanceId());
            if (instanceItr == brutallusRangedAssignments.end())
                return false;

            auto assignmentItr = instanceItr->second.find(bot->GetGUID());
            if (assignmentItr == instanceItr->second.end())
                return false;

            positionIndex = assignmentItr->second;
            return true;
        }

        positionIndex = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID)
                continue;

            bool isMelee = botAI->IsMelee(member);
            if ((wantRanged && isMelee) || (!wantRanged && !isMelee) ||
                botAI->IsMainTank(member) ||
                botAI->IsAssistTankOfIndex(member, 0, true))
            {
                continue;
            }

            if (member == bot)
                return true;

            ++positionIndex;
        }

        return false;
    }

    float GetBrutallusMainTankAngle(Unit* brutallus)
    {
        if (!brutallus)
            return 0.0f;

        return Position::NormalizeOrientation(
            brutallus->GetAngle(BRUTALLUS_MAIN_TANK_POSITION.GetPositionX(),
                                BRUTALLUS_MAIN_TANK_POSITION.GetPositionY()));
    }

    Position GetBrutallusPositionAtAngle(
        Unit* brutallus, float angle, float radius, float z)
    {
        float centerX = brutallus ? brutallus->GetPositionX() :
            BRUTALLUS_MAIN_TANK_POSITION.GetPositionX();
        float centerY = brutallus ? brutallus->GetPositionY() :
            BRUTALLUS_MAIN_TANK_POSITION.GetPositionY();
        float x = centerX + std::cos(angle) * radius;
        float y = centerY + std::sin(angle) * radius;
        return { x, y, z };
    }

    float GetCenteredArcSlotAngleOffset(
        uint8 slotIndex, uint8 slotCount, float arcWidth)
    {
        if (slotCount <= 1)
            return 0.0f;

        float angleStep = arcWidth / static_cast<float>(slotCount - 1);
        if (slotCount % 2 == 1)
        {
            if (slotIndex == 0)
                return 0.0f;

            uint8 stepIndex = (slotIndex + 1) / 2;
            float angleOffset = angleStep * stepIndex;
            if (slotIndex % 2 == 0)
                angleOffset = -angleOffset;

            return angleOffset;
        }

        float halfStep = angleStep / 2.0f;
        uint8 pairIndex = slotIndex / 2;
        float angleOffset = halfStep + angleStep * pairIndex;
        if (slotIndex % 2 == 1)
            angleOffset = -angleOffset;

        return angleOffset;
    }

    float NormalizeSignedAngle(float angle)
    {
        angle = Position::NormalizeOrientation(angle);
        if (angle > M_PI)
            angle -= 2.0f * M_PI;

        return angle;
    }

    float GetBrutallusRangedSlotAngle(
        Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo)
    {
        constexpr float rangedSpacing = 6.0f;

        float frontCenterAngle = Position::NormalizeOrientation(
            GetBrutallusMainTankAngle(brutallus) +
            BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);

        float tankAngle = GetBrutallusMainTankAngle(brutallus);
        if (!slotInfo.isMainTankGroup)
        {
            tankAngle = Position::NormalizeOrientation(
                tankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);
        }

        float angleTowardCenter = NormalizeSignedAngle(
            frontCenterAngle - tankAngle);
        float towardCenterSign = angleTowardCenter < 0.0f ? -1.0f : 1.0f;
        float stepRatio = rangedSpacing / (2.0f * BRUTALLUS_NORMAL_RANGED_RADIUS);
        stepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
        float angleStep = 2.0f * std::asin(stepRatio);
        float arcHalfWidth = angleStep * static_cast<float>(
            BRUTALLUS_RANGED_POSITIONS_PER_GROUP - 1) / 2.0f;
        float outerEdgeAngle = Position::NormalizeOrientation(
            tankAngle - towardCenterSign * arcHalfWidth);

        return Position::NormalizeOrientation(
            outerEdgeAngle + towardCenterSign * angleStep * slotInfo.arcPositionIndex);
    }

    bool TryGetBrutallusRangedStepPosition(
        Unit* brutallus, uint8 rangedIndex, bool useMirrorAngle,
        float radius, float z, Position& position)
    {
        if (!brutallus)
            return false;

        if (rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
            return false;

        BrutallusRangedSlotInfo slotInfo = {
            rangedIndex % 2 == 0,
            static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP)
        };

        float angle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        if (useMirrorAngle)
            angle = Position::NormalizeOrientation(
                angle + (slotInfo.isMainTankGroup ? M_PI_2 : -M_PI_2));

        position = GetBrutallusPositionAtAngle(brutallus, angle, radius, z);
        return true;
    }

    bool TryGetBrutallusRangedArcPosition(
        Unit* brutallus, uint8 rangedIndex, float radius, bool moveTowardMirror,
        float currentX, float currentY, float z, Position& position)
    {
        if (!brutallus)
            return false;

        if (rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
            return false;

        BrutallusRangedSlotInfo slotInfo = {
            rangedIndex % 2 == 0,
            static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP)
        };

        float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        float targetAngle = normalAngle;
        if (moveTowardMirror)
            targetAngle = Position::NormalizeOrientation(
                normalAngle + (slotInfo.isMainTankGroup ? M_PI_2 : -M_PI_2));

        float currentAngle = Position::NormalizeOrientation(
            std::atan2(currentY - brutallus->GetPositionY(), currentX - brutallus->GetPositionX()));
        float remainingAngle = NormalizeSignedAngle(targetAngle - currentAngle);

        constexpr float stepDistance = 3.0f;
        float stepRatio = stepDistance / (2.0f * radius);
        stepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
        float stepAngle = 2.0f * std::asin(stepRatio);
        float nextAngle = targetAngle;

        if (std::fabs(remainingAngle) > stepAngle)
        {
            nextAngle = Position::NormalizeOrientation(
                currentAngle + std::copysign(stepAngle, remainingAngle));
        }

        position = GetBrutallusPositionAtAngle(brutallus, nextAngle, radius, z);
        return true;
    }

    void EnsureBrutallusRangedAssignments(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        auto& assignments = brutallusRangedAssignments[bot->GetInstanceId()];

        std::array<bool, BRUTALLUS_TOTAL_RANGED_POSITIONS> usedPositions = {};
        for (auto const& assignment : assignments)
        {
            if (assignment.second < BRUTALLUS_TOTAL_RANGED_POSITIONS)
                usedPositions[assignment.second] = true;
        }

        auto assignNextOpenSlot = [&](Player* member)
        {
            for (uint8 slotIndex = 0;
                 slotIndex < BRUTALLUS_TOTAL_RANGED_POSITIONS; ++slotIndex)
            {
                if (usedPositions[slotIndex])
                    continue;

                assignments[member->GetGUID()] = slotIndex;
                usedPositions[slotIndex] = true;
                return true;
            }

            assignments[member->GetGUID()] = static_cast<uint8>(
                assignments.size() % BRUTALLUS_TOTAL_RANGED_POSITIONS);

            return true;
        };

        std::vector<Player*> healers;
        std::vector<Player*> rangedDamage;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID ||
                !botAI->IsRanged(member))
            {
                continue;
            }

            if (assignments.find(member->GetGUID()) != assignments.end())
                continue;

            if (botAI->IsHeal(member))
                healers.push_back(member);
            else
                rangedDamage.push_back(member);
        }

        for (Player* member : healers)
        {
            if (!assignNextOpenSlot(member))
                return;
        }

        for (Player* member : rangedDamage)
        {
            if (!assignNextOpenSlot(member))
                return;
        }
    }

    // Felmyst

    const Position FELMYST_TANK_POSITION = { 1470.891f, 601.720f, 23.227f };

    const std::array<Position, 3> FELMYST_FOG_LEFT_LANES = {{
        { 1494.745f, 704.000f, 50.085f, 4.747f },
        { 1469.923f, 703.239f, 50.086f, 4.747f },
        { 1446.515f, 701.518f, 50.085f, 4.747f },
    }};

    const std::array<Position, 3> FELMYST_FOG_RIGHT_LANES = {{
        { 1492.820f, 515.668f, 50.083f, 1.449f },
        { 1466.732f, 515.595f, 50.572f, 1.449f },
        { 1441.640f, 520.520f, 50.083f, 1.449f },
    }};

    const std::array<std::array<Position, 3>, 3> FELMYST_FOG_SAFE_SPOTS = {{
        {{
            { 1466.414f, 598.460f, 22.691f },
            { 1468.840f, 614.437f, 22.460f },
            { 1469.725f, 628.240f, 21.588f },
        }},
        {{
            { 1500.258f, 613.369f, 26.310f },
            { 1500.335f, 596.584f, 25.872f },
            { 1501.360f, 630.656f, 25.482f },
        }},
        {{
            { 1485.415f, 602.346f, 24.075f },
            { 1486.311f, 585.250f, 23.376f },
            { 1488.347f, 619.364f, 24.587f },
        }}
    }};

    const Position FELMYST_FOG_LEFT_SIDE = { 1469.064f, 729.585f, 59.824f, 4.677f };
    const Position FELMYST_FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

    FelmystFogLocation GetFelmystFogLocationFromLanePointIndex(uint8 laneIndex, bool useLeftPoint)
    {
        switch (laneIndex)
        {
            case 0:
                return useLeftPoint ? FelmystFogLocation::LeftTop : FelmystFogLocation::RightTop;
            case 1:
                return useLeftPoint ? FelmystFogLocation::LeftMiddle : FelmystFogLocation::RightMiddle;
            case 2:
                return useLeftPoint ? FelmystFogLocation::LeftBottom : FelmystFogLocation::RightBottom;
            default:
                return FelmystFogLocation::None;
        }
    }

    FelmystFogLane GetFelmystFogLaneFromLocation(FelmystFogLocation location)
    {
        switch (location)
        {
            case FelmystFogLocation::LeftTop:
            case FelmystFogLocation::RightTop:
                return FelmystFogLane::Top;
            case FelmystFogLocation::LeftMiddle:
            case FelmystFogLocation::RightMiddle:
                return FelmystFogLane::Middle;
            case FelmystFogLocation::LeftBottom:
            case FelmystFogLocation::RightBottom:
                return FelmystFogLane::Bottom;
            default:
                return FelmystFogLane::None;
        }
    }

    bool IsFelmystFogSideLocation(FelmystFogLocation location)
    {
        return location == FelmystFogLocation::LeftSide || location == FelmystFogLocation::RightSide;
    }

    bool IsNearFelmystFogSafeSpot(Player* bot, FelmystFogLane dangerLane, float& closestDistance)
    {
        closestDistance = std::numeric_limits<float>::max();
        if (dangerLane == FelmystFogLane::None)
            return false;

        uint8 laneIndex = static_cast<uint8>(dangerLane);
        if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
            return false;

        for (Position const& safeSpot : FELMYST_FOG_SAFE_SPOTS[laneIndex])
        {
            float distance = bot->GetExactDist2d(safeSpot.GetPositionX(), safeSpot.GetPositionY());
            if (distance < closestDistance)
                closestDistance = distance;
        }

        return closestDistance <= FELMYST_FOG_SAFE_SPOT_ARRIVAL_DISTANCE;
    }

    FelmystFogLocation GetFelmystFogLocationFromPosition(
        float positionX, float positionY, float matchDistance)
    {
        float bestDistance = matchDistance;
        FelmystFogLocation bestLocation = FelmystFogLocation::None;

        float leftSideDistance = std::hypot(
            positionX - FELMYST_FOG_LEFT_SIDE.GetPositionX(),
            positionY - FELMYST_FOG_LEFT_SIDE.GetPositionY());
        if (leftSideDistance <= bestDistance)
        {
            bestDistance = leftSideDistance;
            bestLocation = FelmystFogLocation::LeftSide;
        }

        float rightSideDistance = std::hypot(
            positionX - FELMYST_FOG_RIGHT_SIDE.GetPositionX(),
            positionY - FELMYST_FOG_RIGHT_SIDE.GetPositionY());
        if (rightSideDistance <= bestDistance)
        {
            bestDistance = rightSideDistance;
            bestLocation = FelmystFogLocation::RightSide;
        }

        for (uint8 laneIndex = 0; laneIndex < FELMYST_FOG_LEFT_LANES.size(); ++laneIndex)
        {
            float leftDistance = std::hypot(
                positionX - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionX(),
                positionY - FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionY());
            if (leftDistance <= bestDistance)
            {
                bestDistance = leftDistance;
                bestLocation = GetFelmystFogLocationFromLanePointIndex(laneIndex, true);
            }

            float rightDistance = std::hypot(
                positionX - FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionX(),
                positionY - FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionY());
            if (rightDistance <= bestDistance)
            {
                bestDistance = rightDistance;
                bestLocation = GetFelmystFogLocationFromLanePointIndex(laneIndex, false);
            }
        }

        return bestLocation;
    }

    bool TryGetFelmystMovementDestination(Unit* felmyst, Position& destination)
    {
        if (!felmyst)
            return false;

        float destinationX = 0.0f;
        float destinationY = 0.0f;
        float destinationZ = 0.0f;
        if (!felmyst->GetMotionMaster()->GetDestination(destinationX, destinationY, destinationZ))
            return false;

        destination = Position{ destinationX, destinationY, destinationZ };
        return true;
    }

    FelmystFogLocation GetFelmystCurrentFogLocation(Unit* felmyst)
    {
        if (!felmyst)
            return FelmystFogLocation::None;

        return GetFelmystFogLocationFromPosition(
            felmyst->GetPositionX(), felmyst->GetPositionY(), FELMYST_FOG_CURRENT_POINT_MATCH_DISTANCE);
    }

    FelmystFogLocation GetFelmystDestinationFogLocation(Unit* felmyst)
    {
        Position destination;
        if (!TryGetFelmystMovementDestination(felmyst, destination))
            return FelmystFogLocation::None;

        return GetFelmystFogLocationFromPosition(
            destination.GetPositionX(), destination.GetPositionY(), FELMYST_FOG_DESTINATION_MATCH_DISTANCE);
    }

    const std::array<std::array<Position, 4>, 2> FELMYST_DEMONIC_VAPOR_KITE_PATHS = {{
        {{
            { 1484.994f, 598.407f, 23.859f },
            { 1491.537f, 580.538f, 23.356f },
            { 1475.972f, 565.603f, 22.783f },
            { 1458.482f, 584.163f, 21.248f },
        }},
        {{
            { 1483.642f, 635.021f, 22.168f },
            { 1495.737f, 650.552f, 23.033f },
            { 1480.149f, 663.314f, 20.998f },
            { 1459.241f, 650.507f, 19.350f },
        }}
    }};

    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystRangedAssignments;
    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporPathIndices;
    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> felmystDemonicVaporWaypointIndices;
    std::unordered_map<uint32, FelmystFogOfCorruptionState> felmystFogOfCorruptionStates;

    void EnsureFelmystRangedAssignments(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return;

        auto& assignments = felmystRangedAssignments[bot->GetInstanceId()];

        std::vector<ObjectGuid> invalidAssignments;
        for (auto const& assignment : assignments)
        {
            bool found = false;

            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member->GetGUID() != assignment.first)
                    continue;

                found = botAI->IsRanged(member);
                break;
            }

            if (!found)
                invalidAssignments.push_back(assignment.first);
        }

        for (ObjectGuid const& guid : invalidAssignments)
            assignments.erase(guid);

        uint32 leftCount = 0;
        uint32 rightCount = 0;
        for (auto const& assignment : assignments)
        {
            if (assignment.second == 0)
                ++leftCount;
            else
                ++rightCount;
        }

        std::vector<Player*> priests;
        std::vector<Player*> healers;
        std::vector<Player*> rangedDamage;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (assignments.find(member->GetGUID()) != assignments.end())
                continue;

            if (member->getClass() == CLASS_PRIEST)
                priests.push_back(member);
            else if (botAI->IsHeal(member))
                healers.push_back(member);
            else
                rangedDamage.push_back(member);
        }

        auto sortByGuid = [](std::vector<Player*>& members)
        {
            std::sort(members.begin(), members.end(),
                [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
        };

        sortByGuid(priests);
        sortByGuid(healers);
        sortByGuid(rangedDamage);

        auto assignMembers = [&](std::vector<Player*> const& members)
        {
            for (Player* member : members)
            {
                uint8 sideIndex = leftCount <= rightCount ? 0 : 1;
                assignments[member->GetGUID()] = sideIndex;

                if (sideIndex == 0)
                    ++leftCount;
                else
                    ++rightCount;
            }
        };

        assignMembers(priests);
        assignMembers(healers);
        assignMembers(rangedDamage);
    }

    float GetFelmystFrontAngle(PlayerbotAI* botAI, Player* bot, Unit* felmyst)
    {
        float frontX = FELMYST_TANK_POSITION.GetPositionX();
        float frontY = FELMYST_TANK_POSITION.GetPositionY();

        Player* mainTank = GetGroupMainTank(botAI, bot);
        if (mainTank && mainTank->IsAlive() &&
            mainTank->GetMapId() == felmyst->GetMapId() )
        {
            frontX = mainTank->GetPositionX();
            frontY = mainTank->GetPositionY();
        }
        else if (Unit* victim = felmyst->GetVictim())
        {
            frontX = victim->GetPositionX();
            frontY = victim->GetPositionY();
        }

        return std::atan2(frontY - felmyst->GetPositionY(), frontX - felmyst->GetPositionX());
    }

    Creature* GetFelmystDemonicVaporSummonedByBot(Player* carrier)
    {
        if (!carrier)
            return nullptr;

        constexpr float searchRadius = 80.0f;
        std::list<Creature*> vapors;
        carrier->GetCreatureListWithEntryInGrid(
            vapors, static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR), searchRadius);
        for (Creature* creature : vapors)
        {
            if (creature && creature->IsAlive() &&
                creature->GetSummonerGUID() == carrier->GetGUID())
            {
                return creature;
            }
        }

        return nullptr;
    }

    void ClearFelmystDemonicVaporKiteState(Player* bot)
    {
        uint32 instanceId = bot->GetInstanceId();
        ObjectGuid guid = bot->GetGUID();

        auto pathInstanceItr = felmystDemonicVaporPathIndices.find(instanceId);
        if (pathInstanceItr != felmystDemonicVaporPathIndices.end())
        {
            pathInstanceItr->second.erase(guid);
            if (pathInstanceItr->second.empty())
                felmystDemonicVaporPathIndices.erase(pathInstanceItr);
        }

        auto waypointInstanceItr = felmystDemonicVaporWaypointIndices.find(instanceId);
        if (waypointInstanceItr != felmystDemonicVaporWaypointIndices.end())
        {
            waypointInstanceItr->second.erase(guid);
            if (waypointInstanceItr->second.empty())
                felmystDemonicVaporWaypointIndices.erase(waypointInstanceItr);
        }
    }

    float GetDistanceToSegment2d(float pointX, float pointY, Position const& start, Position const& end)
    {
        float startX = start.GetPositionX();
        float startY = start.GetPositionY();
        float endX = end.GetPositionX();
        float endY = end.GetPositionY();
        float deltaX = endX - startX;
        float deltaY = endY - startY;
        float segmentLengthSquared = deltaX * deltaX + deltaY * deltaY;

        if (segmentLengthSquared <= 0.0f)
            return std::hypot(pointX - startX, pointY - startY);

        float projection = ((pointX - startX) * deltaX + (pointY - startY) * deltaY) /
            segmentLengthSquared;
        projection = std::clamp(projection, 0.0f, 1.0f);

        float closestX = startX + projection * deltaX;
        float closestY = startY + projection * deltaY;
        return std::hypot(pointX - closestX, pointY - closestY);
    }

    float GetDistanceToFelmystDemonicVaporPath(float pointX, float pointY, uint8 pathIndex)
    {
        if (pathIndex >= FELMYST_DEMONIC_VAPOR_KITE_PATHS.size())
            return std::numeric_limits<float>::max();

        float bestDistance = std::numeric_limits<float>::max();
        auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
        for (uint8 waypointIndex = 0; waypointIndex < path.size(); ++waypointIndex)
        {
            Position const& start = path[waypointIndex];
            Position const& end = path[(waypointIndex + 1) % path.size()];
            float segmentDistance = GetDistanceToSegment2d(pointX, pointY, start, end);
            if (segmentDistance < bestDistance)
                bestDistance = segmentDistance;
        }

        return bestDistance;
    }

    uint8 GetNearestFelmystDemonicVaporPathIndex(float pointX, float pointY)
    {
        uint8 bestPathIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();

        for (uint8 pathIndex = 0; pathIndex < FELMYST_DEMONIC_VAPOR_KITE_PATHS.size(); ++pathIndex)
        {
            float pathDistance = GetDistanceToFelmystDemonicVaporPath(pointX, pointY, pathIndex);
            if (pathDistance < bestDistance)
            {
                bestDistance = pathDistance;
                bestPathIndex = pathIndex;
            }
        }

        return bestPathIndex;
    }

    uint8 GetNearestFelmystDemonicVaporWaypointIndex(Player* bot, uint8 pathIndex)
    {
        uint8 bestIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();
        auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];

        for (uint8 waypointIndex = 0; waypointIndex < path.size(); ++waypointIndex)
        {
            Position const& waypoint = path[waypointIndex];
            float distance = bot->GetExactDist2d(waypoint.GetPositionX(), waypoint.GetPositionY());
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestIndex = waypointIndex;
            }
        }

        return bestIndex;
    }

    uint8 GetNextFelmystDemonicVaporWaypointIndex(Player* bot, uint8 pathIndex, uint8 currentWaypointIndex)
    {
        auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
        uint8 waypointIndex = currentWaypointIndex % path.size();
        uint8 nextWaypointIndex = (waypointIndex + 1) % path.size();

        constexpr float waypointReachedDistance = 4.0f;
        Position const& currentWaypoint = path[waypointIndex];
        if (bot->GetExactDist2d(currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY()) <=
            waypointReachedDistance)
        {
            return nextWaypointIndex;
        }

        float currentDistance = bot->GetExactDist2d(
            currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY());
        Position const& nextWaypoint = path[nextWaypointIndex];
        float nextDistance = bot->GetExactDist2d(
            nextWaypoint.GetPositionX(), nextWaypoint.GetPositionY());
        if (nextDistance + 1.0f < currentDistance)
            return nextWaypointIndex;

        return waypointIndex;
    }

    std::array<uint32, 2> GetFelmystDemonicVaporPathOccupancyCounts(Player* bot)
    {
        std::array<uint32, 2> occupancyCounts = { 0, 0 };
        uint32 instanceId = bot->GetInstanceId();

        auto pathInstanceItr = felmystDemonicVaporPathIndices.find(instanceId);
        if (pathInstanceItr != felmystDemonicVaporPathIndices.end())
        {
            Group* group = bot->GetGroup();
            if (group)
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (!member || member == bot)
                    {
                        continue;
                    }

                    auto memberPathItr = pathInstanceItr->second.find(member->GetGUID());
                    if (memberPathItr == pathInstanceItr->second.end())
                        continue;

                    if (!GetFelmystDemonicVaporSummonedByBot(member))
                        continue;

                    if (memberPathItr->second < occupancyCounts.size())
                        ++occupancyCounts[memberPathItr->second];
                }
            }
        }

        auto addHazardsOnPath = [&](uint32 entry)
        {
            constexpr float searchRadius = 150.0f;
            constexpr float pathOccupationDistance = 18.0f;
            std::list<Creature*> creatures;
            bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
            for (Creature* creature : creatures)
            {
                if (!creature || !creature->IsAlive())
                    continue;

                if (entry == static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR) &&
                    creature->GetSummonerGUID() == bot->GetGUID())
                {
                    continue;
                }

                uint8 pathIndex = GetNearestFelmystDemonicVaporPathIndex(
                    creature->GetPositionX(), creature->GetPositionY());
                float pathDistance = GetDistanceToFelmystDemonicVaporPath(
                    creature->GetPositionX(), creature->GetPositionY(), pathIndex);
                if (pathDistance <= pathOccupationDistance)
                    ++occupancyCounts[pathIndex];
            }
        };

        addHazardsOnPath(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR));
        addHazardsOnPath(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL));

        return occupancyCounts;
    }

    uint8 SelectFelmystDemonicVaporPath(Player* bot)
    {
        std::array<uint32, 2> occupancyCounts = GetFelmystDemonicVaporPathOccupancyCounts(bot);
        uint8 bestPathIndex = 0;
        uint32 bestOccupancy = std::numeric_limits<uint32>::max();
        float bestDistance = std::numeric_limits<float>::max();

        for (uint8 pathIndex = 0; pathIndex < FELMYST_DEMONIC_VAPOR_KITE_PATHS.size(); ++pathIndex)
        {
            float pathDistance = GetDistanceToFelmystDemonicVaporPath(
                bot->GetPositionX(), bot->GetPositionY(), pathIndex);
            if (occupancyCounts[pathIndex] < bestOccupancy ||
                (occupancyCounts[pathIndex] == bestOccupancy && pathDistance < bestDistance))
            {
                bestPathIndex = pathIndex;
                bestOccupancy = occupancyCounts[pathIndex];
                bestDistance = pathDistance;
            }
        }

        return bestPathIndex;
    }

    bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination)
    {
        uint32 instanceId = bot->GetInstanceId();
        ObjectGuid guid = bot->GetGUID();

        if (!GetFelmystDemonicVaporSummonedByBot(bot))
        {
            ClearFelmystDemonicVaporKiteState(bot);
            return false;
        }

        auto& pathIndices = felmystDemonicVaporPathIndices[instanceId];
        auto& waypointIndices = felmystDemonicVaporWaypointIndices[instanceId];
        auto pathItr = pathIndices.find(guid);
        auto waypointItr = waypointIndices.find(guid);
        uint8 pathIndex = 0;
        uint8 waypointIndex = 0;

        if (pathItr == pathIndices.end())
        {
            pathIndex = SelectFelmystDemonicVaporPath(bot);
            pathIndices[guid] = pathIndex;
        }
        else
        {
            pathIndex = pathItr->second;
        }

        auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
        if (waypointItr == waypointIndices.end())
        {
            waypointIndex = 0;
            waypointIndices[guid] = waypointIndex;
        }
        else
        {
            waypointIndex = GetNextFelmystDemonicVaporWaypointIndex(bot, pathIndex, waypointItr->second);
            waypointIndices[guid] = waypointIndex;
        }

        Position const& waypoint = path[waypointIndex % path.size()];
        destination = Position{ waypoint.GetPositionX(), waypoint.GetPositionY(),
                                waypoint.GetPositionZ(), bot->GetOrientation() };
        return true;
    }

    bool TryGetFelmystFogOfCorruptionStageState(
        Unit* felmyst, FelmystFogOfCorruptionState& state)
    {
        state = FelmystFogOfCorruptionState();
        uint32 now = getMSTime();

        if (!felmyst || !felmyst->IsFlying())
        {
            if (felmyst)
                felmystFogOfCorruptionStates.erase(felmyst->GetInstanceId());
            return false;
        }

        FelmystFogOfCorruptionState& tracker = felmystFogOfCorruptionStates[felmyst->GetInstanceId()];
        bool hasTracker = tracker.phase != FelmystFogPhase::None;
        FelmystFogLocation currentLocation = GetFelmystCurrentFogLocation(felmyst);
        FelmystFogLocation destinationLocation = GetFelmystDestinationFogLocation(felmyst);
        FelmystFogLane currentLane = GetFelmystFogLaneFromLocation(currentLocation);
        FelmystFogLane destinationLane = GetFelmystFogLaneFromLocation(destinationLocation);
        bool isSweeping = felmyst->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FELMYST_SPEED_BURST));

        if (currentLane != FelmystFogLane::None)
        {
            constexpr uint32 fogWindupGraceMs = 7000;
            tracker.lane = currentLane;
            tracker.phase = FelmystFogPhase::Windup;
            tracker.expireMs = now + fogWindupGraceMs;
            state = tracker;
            return true;
        }

        if (isSweeping)
        {
            FelmystFogLane selectedLane = currentLane != FelmystFogLane::None ? currentLane : tracker.lane;
            if (selectedLane == FelmystFogLane::None)
                return false;

            constexpr uint32 fogRecoveryGraceMs = 2500;
            tracker.lane = selectedLane;
            tracker.phase = FelmystFogPhase::Sweep;
            tracker.expireMs = now + fogRecoveryGraceMs;
            state = tracker;
            return true;
        }

        if (hasTracker && tracker.expireMs > now && tracker.lane != FelmystFogLane::None &&
            tracker.phase == FelmystFogPhase::Windup &&
            !IsFelmystFogSideLocation(currentLocation) &&
            !IsFelmystFogSideLocation(destinationLocation))
        {
            state = tracker;
            return true;
        }

        if (hasTracker && tracker.expireMs > now && tracker.lane != FelmystFogLane::None &&
            (tracker.phase == FelmystFogPhase::Sweep ||
             tracker.phase == FelmystFogPhase::Recovery ||
             IsFelmystFogSideLocation(currentLocation) ||
             IsFelmystFogSideLocation(destinationLocation)))
        {
            tracker.phase = FelmystFogPhase::Recovery;
            state = tracker;
            return true;
        }

        felmystFogOfCorruptionStates.erase(felmyst->GetInstanceId());
        return false;
    }

    bool TryGetActiveFelmystFogOfCorruptionState(
        Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state)
    {
        if (!TryGetFelmystFogOfCorruptionStageState(felmyst, state))
            return false;

        if (state.phase == FelmystFogPhase::Recovery)
            return false;

        float safeSpotDistance = std::numeric_limits<float>::max();
        if (IsNearFelmystFogSafeSpot(bot, state.lane, safeSpotDistance))
            return false;

        return state.lane != FelmystFogLane::None;
    }

    bool TryGetFelmystFogSafeDestinations(
        Player* bot, FelmystFogLane dangerLane, std::array<Position, 3>& destinations,
        uint8& destinationCount)
    {
        destinationCount = 0;
        if (dangerLane == FelmystFogLane::None)
            return false;

        uint8 laneIndex = static_cast<uint8>(dangerLane);
        if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
            return false;

        auto const& safeSpots = FELMYST_FOG_SAFE_SPOTS[laneIndex];
        std::array<uint8, 3> candidateOrder = { 0, 1, 2 };
        std::list<Creature*> vaporHazards;
        auto addVaporHazards = [&](uint32 entry)
        {
            constexpr float searchRadius = 150.0f;
            std::list<Creature*> creatures;
            bot->GetCreatureListWithEntryInGrid(creatures, entry, searchRadius);
            for (Creature* creature : creatures)
            {
                if (creature && creature->IsAlive())
                    vaporHazards.push_back(creature);
            }
        };

        addVaporHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR));
        addVaporHazards(static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL));

        auto isSafeSpotBlockedByVapor = [&](Position const& safeSpot)
        {
            constexpr float safeDistanceFromVapor = 10.0f;
            for (Creature* hazard : vaporHazards)
            {
                if (!hazard)
                    continue;

                if (hazard->GetExactDist2d(
                        safeSpot.GetPositionX(), safeSpot.GetPositionY()) < safeDistanceFromVapor)
                {
                    return true;
                }
            }

            return false;
        };

        std::sort(candidateOrder.begin(), candidateOrder.end(),
            [&](uint8 leftIndex, uint8 rightIndex)
            {
                Position const& left = safeSpots[leftIndex];
                Position const& right = safeSpots[rightIndex];
                return bot->GetExactDist2d(left.GetPositionX(), left.GetPositionY()) <
                    bot->GetExactDist2d(right.GetPositionX(), right.GetPositionY());
            });

        for (uint8 candidateIndex : candidateOrder)
        {
            Position const& safeSpot = safeSpots[candidateIndex];
            if (isSafeSpotBlockedByVapor(safeSpot))
                continue;

            float destinationX = safeSpot.GetPositionX();
            float destinationY = safeSpot.GetPositionY();
            float destinationZ = safeSpot.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    destinationX, destinationY, destinationZ, false))
            {
                continue;
            }

            destinations[destinationCount++] = Position{ destinationX, destinationY, destinationZ };
        }

        return destinationCount > 0;
    }

    bool TryGetFelmystRangedPosition(PlayerbotAI* botAI, Player* bot, Unit* felmyst, Position& position)
    {
        if (!felmyst)
            return false;

        EnsureFelmystRangedAssignments(botAI, bot);

        auto instanceItr = felmystRangedAssignments.find(bot->GetInstanceId());
        if (instanceItr == felmystRangedAssignments.end())
            return false;

        auto assignmentItr = instanceItr->second.find(bot->GetGUID());
        if (assignmentItr == instanceItr->second.end())
            return false;

        constexpr float sideDistance = 22.0f;
        float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);
        float sideAngle = frontAngle + (assignmentItr->second == 0 ? M_PI_2 : -M_PI_2);
        float destinationX = felmyst->GetPositionX() + std::cos(sideAngle) * sideDistance;
        float destinationY = felmyst->GetPositionY() + std::sin(sideAngle) * sideDistance;
        float destinationZ = bot->GetMapWaterOrGroundLevel(destinationX, destinationY, bot->GetPositionZ());
        if (destinationZ <= INVALID_HEIGHT)
            destinationZ = bot->GetPositionZ();

        bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, false);

        position = Position{ destinationX, destinationY, destinationZ };
        return true;
    }

    Player* GetFelmystEncapsulateTarget(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* closestTarget = nullptr;
        float closestDistance = 0.0f;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() ||
                !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE)))
            {
                continue;
            }

            float distance = bot->GetDistance2d(member);
            if (!closestTarget || distance < closestDistance)
            {
                closestTarget = member;
                closestDistance = distance;
            }
        }

        return closestTarget;
    }

    Player* GetFelmystGasNovaDispelTarget(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Player* closestTarget = nullptr;
        float closestDistance = 0.0f;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member ||
                !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_GAS_NOVA)))
            {
                continue;
            }

            float distance = bot->GetDistance(member);
            if (!closestTarget || distance < closestDistance)
            {
                closestTarget = member;
                closestDistance = distance;
            }
        }

        return closestTarget;
    }

    Unit* GetNearestFelmystFogOfCorruptionCharmedTarget(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        Unit* closestTarget = nullptr;
        float closestDistance = 0.0f;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member ||
                !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
            {
                continue;
            }

            float distance = bot->GetDistance(member);
            if (!closestTarget || distance < closestDistance)
            {
                closestTarget = member;
                closestDistance = distance;
            }
        }

        return closestTarget;
    }

    Unit* GetNearestFelmystDemonicVaporHazard(Player* bot)
    {
        constexpr float searchRadius = 20.0f;
        Unit* nearestTrail = bot->FindNearestCreature(
            static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR_TRAIL), searchRadius, true);
        Unit* nearestVapor = bot->FindNearestCreature(
            static_cast<uint32>(SunwellNpcs::NPC_DEMONIC_VAPOR), searchRadius, true);

        if (!nearestTrail)
            return nearestVapor;

        if (!nearestVapor)
            return nearestTrail;

        return bot->GetDistance2d(nearestTrail) <= bot->GetDistance2d(nearestVapor) ?
            nearestTrail : nearestVapor;
    }

    // Eredar Twins

    const Position SACROLASH_TANK_POSITION  = { 1804.255f, 630.193f, 33.404f };
    const std::array<Position, 5> ALYTHESS_TANK_POSITIONS = {{
        { 1816.830f, 620.792f, 33.404f },
        { 1824.211f, 625.169f, 33.404f },
        { 1818.701f, 631.196f, 33.404f },
        { 1829.375f, 631.110f, 33.404f },
        { 1830.007f, 620.924f, 33.404f }
    }};
    const Position EREDAR_TWINS_P1_RANGED_POSITION =       { 1808.076f, 603.460f, 51.684f };
    const Position EREDAR_TWINS_P2_MELEE_STACK_POSITION =  { 1814.327f, 625.645f, 33.404f };
    const Position EREDAR_TWINS_P2_RANGED_STACK_POSITION = { 1805.587f, 625.653f, 33.404f };
    const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION =  { 1801.133f, 584.456f, 50.696f };
    // const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION =   { 1814.654f, 612.291f, 33.404f };
    const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION =   { 1814.337f, 607.771f, 33.404f };

    std::unordered_map<ObjectGuid, uint8> alythessTankStep;
    std::unordered_map<ObjectGuid, ObjectGuid> alythessTankLastBlazeGuid;

    bool IsSacrolashTank(PlayerbotAI* botAI, Player* bot)
    {
        // If the 2nd assist tank dies, further tanks can fill in
        return botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 1, true);
    }

    bool IsAlythessTank(PlayerbotAI* botAI, Player* bot)
    {
        // The 1st assist tank is pinned to this role and cannot be replaced
        return botAI->IsAssistTankOfIndex(bot, 0, false);
    }

    bool ShouldHoldSacrolashThreat(
        PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash)
    {
        if (!alythess || !sacrolash || IsSacrolashTank(botAI, bot) ||
            IsAlythessTank(botAI, bot))
            return false;

        uint8 playerThreatEntries = 0;

        auto const threatList = sacrolash->GetThreatMgr().GetSortedThreatList();
        for (auto itr = threatList.begin();
             itr != threatList.end() && playerThreatEntries < 2; ++itr)
        {
            ThreatReference const* threatRef = *itr;
            if (!threatRef || !threatRef->IsAvailable())
                continue;

            Player* threatPlayer = threatRef->GetVictim()->ToPlayer();
            if (!threatPlayer || !threatPlayer->IsAlive())
                continue;

            ++playerThreatEntries;
            if (threatPlayer == bot)
                return true;
        }

        return false;
    }

    bool IsAlythessTankPositionSafe(Player* bot, Position const& position)
    {
        constexpr float blazeDangerRadius = 4.5f;
        constexpr float blazeSearchRadius = 30.0f;

        std::list<GameObject*> targets;
        AnyGameObjectInObjectRangeCheck u_check(bot, blazeSearchRadius);
        Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, blazeSearchRadius);

        for (GameObject* go : targets)
        {
            if (!go || go->GetEntry() != static_cast<uint32>(SunwellObjects::GO_BLAZE))
                continue;

            if (go->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) <= blazeDangerRadius)
                return false;
        }

        return true;
    }

    bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot)
    {
        if (!alythess)
            return false;

        ObjectGuid const botGuid = bot->GetGUID();
        constexpr float blazeObjectRadius = 5.0f;
        GameObject* blazeObject = bot->FindNearestGameObject(
            static_cast<uint32>(SunwellObjects::GO_BLAZE),
            blazeObjectRadius);

        if (!blazeObject)
        {
            alythessTankLastBlazeGuid.erase(botGuid);
            return false;
        }

        ObjectGuid const blazeGuid = blazeObject->GetGUID();
        auto lastBlaze = alythessTankLastBlazeGuid.find(botGuid);
        if (lastBlaze != alythessTankLastBlazeGuid.end() && lastBlaze->second == blazeGuid)
            return false;

        alythessTankLastBlazeGuid[botGuid] = blazeGuid;
        return true;
    }

    bool IsEredarTwinsConflagrationTarget(Unit* alythess, Player* bot)
    {
        if (!alythess)
            return false;

        Spell* currentSpell = alythess->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        return currentSpell && currentSpell->m_spellInfo &&
               currentSpell->m_spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION) &&
               currentSpell->m_targets.GetUnitTarget() == bot;
    }

    // M'uru

    const Position MURU_STACK_POSITION =                { 1836.532f, 608.957f, 71.222f };
    const Position MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
    const Position MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };
    const Position MURU_CENTER_POSITION =               { 1816.250f, 625.484f, 69.604f };
    const Position MURU_ENTRANCE_POSITION =             { 1840.567f, 605.769f, 71.250f };

    std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;
    std::unordered_map<uint32, std::unordered_set<ObjectGuid>> muruEntropiusInitialRangedPositionsReached;

    Creature* GetNearestMuruSingularity(Player* bot, float searchRadius)
    {
        Creature* nearestSingularity = nullptr;
        float nearestDistance = std::numeric_limits<float>::max();
        std::list<Creature*> singularities;
        bot->GetCreatureListWithEntryInGrid(
            singularities, static_cast<uint32>(SunwellNpcs::NPC_SINGULARITY), searchRadius);

        for (Creature* singularity : singularities)
        {
            if (!singularity || !singularity->IsAlive())
                continue;

            float distance = bot->GetExactDist2d(singularity);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestSingularity = singularity;
            }
        }

        return nearestSingularity;
    }

    bool IsFirstAssistTankInSameGroup(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        uint8 shamanGroup = group->GetMemberGroup(bot->GetGUID());

        Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
        if (firstAssistTank &&
            group->GetMemberGroup(firstAssistTank->GetGUID()) == shamanGroup)
        {
            return true;
        }

        return false;
    }

    bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru)
    {
        if (!muru)
            return false;

        uint32 const instanceId = bot->GetInstanceId();
        uint32 const now = getMSTime();
        MuruDarknessState& state = muruDarknessStates[instanceId];
        constexpr uint32 darknessPreEffectMs = 3000;
        constexpr uint32 darknessCastMs = 2000;
        constexpr uint32 darknessPostCastDangerMs = 20000;
        constexpr uint32 darknessTotalMs =
            darknessPreEffectMs + darknessCastMs + darknessPostCastDangerMs;

        if (Aura* darknessPreEffect = muru->GetAura(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_PRE_EFFECT)))
        {
            int32 remainingPreEffectMs = darknessPreEffect->GetDuration();
            if (remainingPreEffectMs < 0)
                remainingPreEffectMs = darknessPreEffectMs;

            uint32 const remainingPreEffect = static_cast<uint32>(remainingPreEffectMs);
            uint32 const elapsedPreEffectMs = remainingPreEffect < darknessPreEffectMs ?
                darknessPreEffectMs - remainingPreEffect : 0;
            uint32 const startMs = now > elapsedPreEffectMs ? now - elapsedPreEffectMs : 0;

            if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
                state.startMs = startMs;

            state.expireMs = std::max(state.expireMs,
                                      startMs + darknessTotalMs);
            return true;
        }

        if (muru->HasUnitState(UNIT_STATE_CASTING) &&
            muru->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS)))
        {
            uint32 const startMs = now > darknessPreEffectMs ? now - darknessPreEffectMs : 0;
            if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
                state.startMs = startMs;

            state.expireMs = std::max(state.expireMs, now + darknessCastMs + darknessPostCastDangerMs);
            return true;
        }

        if (state.expireMs > now)
            return true;

        muruDarknessStates.erase(instanceId);
        return false;
    }

    bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs)
    {
        if (!TryGetMuruDarknessActiveState(bot, muru))
            return false;

        auto stateItr = muruDarknessStates.find(bot->GetInstanceId());
        if (stateItr == muruDarknessStates.end())
            return false;

        uint32 const now = getMSTime();
        return stateItr->second.startMs < now &&
               now - stateItr->second.startMs < earlyWindowMs;
    }

    void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
    {
        auto const& units =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

        auto considerTarget = [&](Unit* unit)
        {
            if (!unit || !unit->IsAlive())
                return;

            switch (unit->GetEntry())
            {
                case static_cast<uint32>(SunwellNpcs::NPC_MURU):
                    targets.muru = unit;
                    break;

                case static_cast<uint32>(SunwellNpcs::NPC_ENTROPIUS):
                    targets.entropius = unit;
                    break;

                case static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL):
                    targets.voidSentinels.push_back(unit);
                    break;

                case static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN):
                    targets.voidSpawns.push_back(unit);
                    break;

                case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                    targets.furyMages.push_back(unit);
                    break;

                case static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER):
                    targets.berserkers.push_back(unit);
                    break;

                default:
                    break;
            }
        };

        for (ObjectGuid const& guid : units)
        {
            Unit* unit = botAI->GetUnit(guid);
            considerTarget(unit);
        }
    }

    Creature* FindAvailableVoidSpawnForEnslave(
        PlayerbotAI* botAI, Player* bot, Unit* muru, Unit* entropius)
    {
        if (!muru && !entropius)
            return nullptr;

        Creature* bestSpawn = nullptr;
        float closestDistance = std::numeric_limits<float>::max();
        auto const& units =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

        for (ObjectGuid const& guid : units)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() ||
                unit->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN) ||
                unit->IsCharmed() || unit->GetCharmer())
            {
                continue;
            }

            float distance = bot->GetExactDist2d(unit);
            if (distance >= closestDistance)
                continue;

            Creature* creature = unit->ToCreature();
            if (!creature)
                continue;

            bestSpawn = creature;
            closestDistance = distance;
        }

        return bestSpawn;
    }
    // Kil'jaeden <The Deceiver>

    // Combat reach is 15 yards
    const Position KILJAEDEN_CENTER_POSITION =  { 1698.450f, 628.030f, 28.199f }; // Starting position for KJ
    const Position KILJAEDEN_TANK_POSITION =    { 1704.729f, 634.891f, 27.787f };
    const Position KILJAEDEN_S_MELEE_POSITION = { 1689.487f, 632.119f, 27.823f };
    const Position KILJAEDEN_E_MELEE_POSITION = { 1700.542f, 619.589f, 27.786f };
    const Position KILJAEDEN_STACK_POSITION =   { 1709.768f, 642.241f, 27.706f };

    std::unordered_map<uint32, std::vector<KiljaedenArmageddon>> kiljaedenArmageddons;
    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedAssignments;
    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedArmageddonAssignments;

    void PruneExpiredKiljaedenArmageddons(uint32 instanceId)
    {
        auto instanceItr = kiljaedenArmageddons.find(instanceId);
        if (instanceItr == kiljaedenArmageddons.end())
            return;

        uint32 now = getMSTime();
        std::vector<KiljaedenArmageddon>& armageddons = instanceItr->second;
        armageddons.erase(std::remove_if(armageddons.begin(), armageddons.end(),
            [now](KiljaedenArmageddon const& armageddon) {
                return !armageddon.expireMs || armageddon.expireMs <= now;
            }), armageddons.end());

        if (armageddons.empty())
            kiljaedenArmageddons.erase(instanceItr);
    }

    void AddKiljaedenArmageddon(
        uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance)
    {
        if (!durationMs || safeDistance <= 0.0f)
            return;

        uint32 now = getMSTime();
        PruneExpiredKiljaedenArmageddons(instanceId);

        KiljaedenArmageddon armageddon;
        armageddon.destination = destination;
        armageddon.expireMs = now + durationMs;
        armageddon.safeDistance = safeDistance;
        kiljaedenArmageddons[instanceId].push_back(armageddon);
    }

    bool HasActiveKiljaedenArmageddon(uint32 instanceId)
    {
        PruneExpiredKiljaedenArmageddons(instanceId);
        auto instanceItr = kiljaedenArmageddons.find(instanceId);
        return instanceItr != kiljaedenArmageddons.end() && !instanceItr->second.empty();
    }

    bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon)
    {
        PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
        auto instanceItr = kiljaedenArmageddons.find(bot->GetInstanceId());
        if (instanceItr == kiljaedenArmageddons.end())
            return false;

        bool foundArmageddon = false;
        float bestDistance = 0.0f;

        for (KiljaedenArmageddon const& candidate : instanceItr->second)
        {
            float distance = bot->GetExactDist2d(
                candidate.destination.GetPositionX(), candidate.destination.GetPositionY());
            if (distance >= candidate.safeDistance)
                continue;

            if (!foundArmageddon || distance < bestDistance)
            {
                armageddon = candidate;
                bestDistance = distance;
                foundArmageddon = true;
            }
        }

        return foundArmageddon;
    }

    bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden)
    {
        return kiljaeden && kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
               kiljaeden->FindCurrentSpellBySpellId(
                   static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
    }

    bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position)
    {
        if (slotIndex >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
            return false;

        float radius = KILJAEDEN_OUTER_RANGED_RADIUS;
        uint8 localSlotIndex = slotIndex;
        uint8 slotCount = KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

        if (slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT)
        {
            radius = KILJAEDEN_INNER_RANGED_RADIUS;
            slotCount = KILJAEDEN_INNER_RANGED_SLOT_COUNT;
        }
        else
        {
            localSlotIndex -= KILJAEDEN_INNER_RANGED_SLOT_COUNT;
        }

        float angleOffset = GetCenteredArcSlotAngleOffset(localSlotIndex, slotCount, M_PI);
        float angle = Position::NormalizeOrientation(
            KILJAEDEN_RANGED_ARC_ORIENTATION + angleOffset);
        float positionX = KILJAEDEN_CENTER_POSITION.GetPositionX() + std::cos(angle) * radius;
        float positionY = KILJAEDEN_CENTER_POSITION.GetPositionY() + std::sin(angle) * radius;

        position = Position{ positionX, positionY, KILJAEDEN_CENTER_POSITION.GetPositionZ() };
        return true;
    }

    float GetKiljaedenRangedSlotAngle(uint8 slotIndex)
    {
        Position position;
        if (!TryGetKiljaedenRangedSlotPosition(slotIndex, position))
            return 0.0f;

        return Position::NormalizeOrientation(
            std::atan2(position.GetPositionY() - KILJAEDEN_CENTER_POSITION.GetPositionY(),
                       position.GetPositionX() - KILJAEDEN_CENTER_POSITION.GetPositionX()));
    }

    bool IsKiljaedenRangedSlotSafe(
        Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
    {
        for (KiljaedenArmageddon const& armageddon : armageddons)
        {
            float deltaX = position.GetPositionX() - armageddon.destination.GetPositionX();
            float deltaY = position.GetPositionY() - armageddon.destination.GetPositionY();
            if (std::sqrt(deltaX * deltaX + deltaY * deltaY) < armageddon.safeDistance)
            {
                return false;
            }
        }

        return true;
    }

    float GetKiljaedenNearestArmageddonDistance(
        Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
    {
        float nearestDistance = std::numeric_limits<float>::max();

        for (KiljaedenArmageddon const& armageddon : armageddons)
        {
            float deltaX = position.GetPositionX() - armageddon.destination.GetPositionX();
            float deltaY = position.GetPositionY() - armageddon.destination.GetPositionY();
            nearestDistance = std::min(
                nearestDistance, std::sqrt(deltaX * deltaX + deltaY * deltaY));
        }

        return nearestDistance;
    }

    void EnsureKiljaedenRangedAssignments(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        auto& assignments = kiljaedenRangedAssignments[bot->GetInstanceId()];

        std::vector<ObjectGuid> invalidAssignments;
        for (auto const& assignment : assignments)
        {
            bool found = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member->GetGUID() != assignment.first)
                    continue;

                found = member->GetMapId() == SUNWELL_MAP_ID &&
                        GET_PLAYERBOT_AI(member) && botAI->IsRanged(member);
                break;
            }

            if (!found)
                invalidAssignments.push_back(assignment.first);
        }

        for (ObjectGuid const& guid : invalidAssignments)
            assignments.erase(guid);

        std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> usedSlots = {};
        for (auto const& assignment : assignments)
        {
            if (assignment.second < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
                usedSlots[assignment.second] = true;
        }

        auto assignNextOpenSlot = [&](Player* member)
        {
            for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++slotIndex)
            {
                if (usedSlots[slotIndex])
                    continue;

                assignments[member->GetGUID()] = slotIndex;
                usedSlots[slotIndex] = true;
                return;
            }
        };

        std::vector<Player*> healers;
        std::vector<Player*> rangedDamage;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID ||
                !GET_PLAYERBOT_AI(member) || !botAI->IsRanged(member))
            {
                continue;
            }

            if (assignments.find(member->GetGUID()) != assignments.end())
                continue;

            if (botAI->IsHeal(member))
                healers.push_back(member);
            else
                rangedDamage.push_back(member);
        }

        auto sortByGuid = [](std::vector<Player*>& members)
        {
            std::sort(members.begin(), members.end(),
                [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
        };

        sortByGuid(healers);
        sortByGuid(rangedDamage);

        for (Player* member : healers)
            assignNextOpenSlot(member);

        for (Player* member : rangedDamage)
            assignNextOpenSlot(member);
    }

    void EnsureKiljaedenRangedArmageddonAssignments(PlayerbotAI* botAI, Player* bot)
    {
        uint32 instanceId = bot->GetInstanceId();
        PruneExpiredKiljaedenArmageddons(instanceId);

        auto armageddonItr = kiljaedenArmageddons.find(instanceId);
        if (armageddonItr == kiljaedenArmageddons.end() || armageddonItr->second.empty())
        {
            kiljaedenRangedArmageddonAssignments.erase(instanceId);
            return;
        }

        Group* group = bot->GetGroup();
        if (!group || !botAI->IsRanged(bot))
        {
            kiljaedenRangedArmageddonAssignments.erase(instanceId);
            return;
        }

        EnsureKiljaedenRangedAssignments(botAI, bot);
        auto canonicalItr = kiljaedenRangedAssignments.find(instanceId);
        if (canonicalItr == kiljaedenRangedAssignments.end())
        {
            kiljaedenRangedArmageddonAssignments.erase(instanceId);
            return;
        }

        std::vector<KiljaedenRangedBotAssignment> rangedBots;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID ||
                !GET_PLAYERBOT_AI(member) || !botAI->IsRanged(member))
            {
                continue;
            }

            auto assignmentItr = canonicalItr->second.find(member->GetGUID());
            if (assignmentItr == canonicalItr->second.end() ||
                assignmentItr->second >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
            {
                continue;
            }

            rangedBots.push_back({ member->GetGUID(), assignmentItr->second });
        }

        std::sort(rangedBots.begin(), rangedBots.end(),
            [](KiljaedenRangedBotAssignment const& left, KiljaedenRangedBotAssignment const& right)
            {
                if (left.slotIndex != right.slotIndex)
                    return left.slotIndex < right.slotIndex;

                return left.guid < right.guid;
            });

        std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> safeSlots = {};
        std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> slotAngles = {};
        std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> nearestArmageddonDistances = {};

        for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++slotIndex)
        {
            Position slotPosition;
            if (!TryGetKiljaedenRangedSlotPosition(slotIndex, slotPosition))
                continue;

            safeSlots[slotIndex] = IsKiljaedenRangedSlotSafe(slotPosition, armageddonItr->second);
            slotAngles[slotIndex] = GetKiljaedenRangedSlotAngle(slotIndex);
            nearestArmageddonDistances[slotIndex] =
                GetKiljaedenNearestArmageddonDistance(slotPosition, armageddonItr->second);
        }

        std::array<uint8, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> plannedOccupancy = {};
        auto& tempAssignments = kiljaedenRangedArmageddonAssignments[instanceId];
        tempAssignments.clear();

        std::vector<KiljaedenRangedBotAssignment> displacedBots;
        for (KiljaedenRangedBotAssignment const& rangedBot : rangedBots)
        {
            if (!safeSlots[rangedBot.slotIndex])
            {
                displacedBots.push_back(rangedBot);
                continue;
            }

            tempAssignments[rangedBot.guid] = rangedBot.slotIndex;
            ++plannedOccupancy[rangedBot.slotIndex];
        }

        constexpr float distanceEpsilon = 0.001f;
        for (KiljaedenRangedBotAssignment const& rangedBot : displacedBots)
        {
            bool bestFound = false;
            uint8 bestSlotIndex = rangedBot.slotIndex;
            bool bestSameRow = false;
            float bestAngleDistance = 0.0f;
            uint8 bestOccupancy = 0;
            float bestArmageddonDistance = 0.0f;

            for (uint8 candidateSlotIndex = 0;
                 candidateSlotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++candidateSlotIndex)
            {
                if (!safeSlots[candidateSlotIndex] || plannedOccupancy[candidateSlotIndex] >= 2)
                    continue;

                bool candidateSameRow =
                    (candidateSlotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT) ==
                    (rangedBot.slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT);
                float candidateAngleDistance = std::fabs(NormalizeSignedAngle(
                    slotAngles[candidateSlotIndex] - slotAngles[rangedBot.slotIndex]));
                uint8 candidateOccupancy = plannedOccupancy[candidateSlotIndex];
                float candidateArmageddonDistance = nearestArmageddonDistances[candidateSlotIndex];

                bool takeCandidate = false;
                if (!bestFound)
                {
                    takeCandidate = true;
                }
                else if (candidateSameRow != bestSameRow)
                {
                    takeCandidate = candidateSameRow;
                }
                else if (candidateAngleDistance + distanceEpsilon < bestAngleDistance)
                {
                    takeCandidate = true;
                }
                else if (std::fabs(candidateAngleDistance - bestAngleDistance) <= distanceEpsilon)
                {
                    if (candidateOccupancy < bestOccupancy)
                    {
                        takeCandidate = true;
                    }
                    else if (candidateOccupancy == bestOccupancy)
                    {
                        if (candidateArmageddonDistance > bestArmageddonDistance + distanceEpsilon)
                        {
                            takeCandidate = true;
                        }
                        else if (std::fabs(candidateArmageddonDistance - bestArmageddonDistance) <=
                                     distanceEpsilon &&
                                 candidateSlotIndex < bestSlotIndex)
                        {
                            takeCandidate = true;
                        }
                    }
                }

                if (!takeCandidate)
                    continue;

                bestFound = true;
                bestSlotIndex = candidateSlotIndex;
                bestSameRow = candidateSameRow;
                bestAngleDistance = candidateAngleDistance;
                bestOccupancy = candidateOccupancy;
                bestArmageddonDistance = candidateArmageddonDistance;
            }

            tempAssignments[rangedBot.guid] = bestSlotIndex;
            if (bestFound)
                ++plannedOccupancy[bestSlotIndex];
        }

        if (tempAssignments.empty())
            kiljaedenRangedArmageddonAssignments.erase(instanceId);
    }
}
