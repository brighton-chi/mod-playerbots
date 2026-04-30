/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <vector>

#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "RaidSunwellKalecgosEncounter.h"
#include "Timer.h"

namespace SunwellHelpers
{

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
    auto const assignment = state.playerToGroup.find(playerGuid);
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
    {
        return false;
    }

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
        const ObjectGuid guid =
            state.tankPortalRotationGuids[(startIndex + offset) % KALECGOS_TANK_COUNT];
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
    Player* currentVictim = kalecgos && kalecgos->GetVictim() ?
        kalecgos->GetVictim()->ToPlayer() : nullptr;

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
    if (!state.activeRiftOpenedMs ||
        HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid) ||
        CountKalecgosSurfaceAssignedTanks(group, state) <= 2)
    {
        return nullptr;
    }

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
    return !IsKalecgosAssignedTank(state, candidate) &&
           IsKalecgosPortalEligibleCandidate(candidate) &&
           state.blastedPlayerGuid != candidate->GetGUID();
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
        const uint8 blastedGroup = GetKalecgosAssignedGroup(state, state.blastedPlayerGuid);
        if (blastedGroup != KALECGOS_INVALID_GROUP)
            return blastedGroup;

        return GetNextAvailableKalecgosGroup(group, state);
    }

    if (state.firstEntrantGuid != ObjectGuid::Empty)
    {
        const uint8 entrantGroup = GetKalecgosAssignedGroup(state, state.firstEntrantGuid);
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

    KalecgosEncounterState& state = kalecgosEncounterStates[bot->GetInstanceId()];
    std::vector<Player*> botMembers;
    const std::array<ObjectGuid, KALECGOS_TANK_COUNT> expectedTankAssignmentGuids =
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

    auto const realmStateItr = kalecgosRealmStates.find(bot->GetGUID());
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

    auto const realmStateItr = kalecgosRealmStates.find(bot->GetGUID());
    if (realmStateItr == kalecgosRealmStates.end())
        return false;

    const uint32 now = getMSTime();
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
    const uint32 now = getMSTime();

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
    const ObjectGuid guid = bot->GetGUID();
    const uint32 now = getMSTime();

    UpdateKalecgosRealmState(bot, true, now);

    if (state.activeRiftOpenedMs)
    {
        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = guid;

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
    }

    if (IsKalecgosAssignedTank(state, bot))
    {
        AdvanceKalecgosTankPortalRotation(state, guid);

        if (state.activeRiftOutgoingTankGuid == guid)
            state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

        if (state.currentTankGuid == guid)
        {
            if (Player* nextTank = GetFirstKalecgosSurfaceAssignedTank(group, state, guid))
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

}
