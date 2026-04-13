/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "RaidSunwellHelpers.h"
#include "RaidBossHelpers.h"
#include "Playerbots.h"

namespace SunwellHelpers
{
    // Kalecgos & Sathrovarr the Corruptor

    const Position KALECGOS_TANK_POSITION = { 1703.584f, 895.626f, 53.076f };
    const Position KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

    std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
    std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;
    std::unordered_map<ObjectGuid, bool> hasReachedKalecgosInitialRangedPosition;

    KalecgosEncounterState& GetKalecgosEncounterState(uint32 instanceId)
    {
        return kalecgosEncounterStates[instanceId];
    }

    KalecgosRealmState& GetKalecgosRealmState(Player* bot)
    {
        return kalecgosRealmStates[bot->GetGUID()];
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

    Player* FindKalecgosGroupMember(Group* group, uint32 instanceId, ObjectGuid playerGuid)
    {
        if (playerGuid == ObjectGuid::Empty || !group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != playerGuid)
                continue;

            if (!member->IsInWorld() || member->GetMapId() != SUNWELL_MAP_ID ||
                member->GetInstanceId() != instanceId)
            {
                return nullptr;
            }

            return member;
        }

        return nullptr;
    }

    KalecgosEncounterState& PrepareKalecgosEncounterState(PlayerbotAI* botAI, Player* bot)
    {
        KalecgosEncounterState& state = GetKalecgosEncounterState(bot->GetInstanceId());
        ResetExpiredKalecgosRift(state, getMSTime());
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

        return GetAssignedGroup(state, candidate->GetGUID()) == state.activeRiftGroup;
    }

    bool IsKalecgosPortalEligibleCandidate(Player* candidate)
    {
        if (!candidate || !GET_PLAYERBOT_AI(candidate))
            return false;

        if (!candidate->IsAlive() || candidate->GetMapId() != SUNWELL_MAP_ID)
            return false;

        if (candidate->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_EXHAUSTION)))
            return false;

        return !IsInKalecgosSpectralRealm(candidate);
    }

    std::array<ObjectGuid, KALECGOS_GROUP_COUNT> GetExpectedKalecgosTankGuids(
        PlayerbotAI* botAI, Player* bot)
    {
        std::array<ObjectGuid, KALECGOS_GROUP_COUNT> tankGuids = {
            ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
        };

        std::array<Player*, KALECGOS_GROUP_COUNT> tanks = {
            GetGroupMainTank(botAI, bot),
            GetGroupAssistTank(botAI, bot, 0),
            GetGroupAssistTank(botAI, bot, 1),
            GetGroupAssistTank(botAI, bot, 2)
        };

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
        {
            Player* tank = tanks[groupIndex];
            if (GET_PLAYERBOT_AI(tank))
                tankGuids[groupIndex] = tank->GetGUID();
        }

        return tankGuids;
    }

    std::array<Player*, KALECGOS_GROUP_COUNT> GetKalecgosAssignedTankOrder(
        Group* group, uint32 instanceId, KalecgosEncounterState const& state)
    {
        std::array<Player*, KALECGOS_GROUP_COUNT> tanks = { nullptr, nullptr, nullptr, nullptr };

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
            tanks[groupIndex] =
                FindKalecgosGroupMember(group, instanceId, state.groupTankGuids[groupIndex]);

        return tanks;
    }

    bool IsKalecgosAssignedTank(KalecgosEncounterState const& state, Player* candidate)
    {
        if (!candidate)
            return false;

        return std::find(state.groupTankGuids.begin(), state.groupTankGuids.end(), candidate->GetGUID()) !=
               state.groupTankGuids.end();
    }

    bool HasAnotherKalecgosAssignedSurfaceTank(
        Group* group, uint32 instanceId, KalecgosEncounterState const& state, Player* currentTank)
    {
        for (Player* tank : GetKalecgosAssignedTankOrder(group, instanceId, state))
        {
            if (!tank || tank == currentTank)
                continue;

            if (!tank->IsAlive() || tank->GetMapId() != SUNWELL_MAP_ID)
                continue;

            if (IsInKalecgosSpectralRealm(tank))
                continue;

            return true;
        }

        return false;
    }

    bool ShouldKalecgosTankStayOnSurface(
        Group* group, uint32 instanceId, KalecgosEncounterState const& state, Player* tank)
    {
        return IsKalecgosAssignedTank(state, tank) &&
               !HasAnotherKalecgosAssignedSurfaceTank(group, instanceId, state, tank);
    }

    bool CanKalecgosBotEnterRift(
        Group* group, uint32 instanceId, Player* candidate, KalecgosEncounterState const& state)
    {
        if (!IsKalecgosPortalEligibleCandidate(candidate))
            return false;

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return false;

        return !ShouldKalecgosTankStayOnSurface(group, instanceId, state, candidate);
    }

    uint8 GetNextAvailableKalecgosGroup(
        Group* group, uint32 instanceId, KalecgosEncounterState const& state)
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

                if (GetAssignedGroup(state, member->GetGUID()) != groupIndex)
                    continue;

                if (CanKalecgosBotEnterRift(group, instanceId, member, state))
                    return groupIndex;
            }
        }

        return KALECGOS_INVALID_GROUP;
    }

    uint8 ResolveKalecgosActiveRiftGroup(
        Group* group, uint32 instanceId, KalecgosEncounterState const& state)
    {
        if (state.blastedPlayerGuid != ObjectGuid::Empty)
        {
            uint8 blastedGroup = GetAssignedGroup(state, state.blastedPlayerGuid);
            if (blastedGroup != KALECGOS_INVALID_GROUP)
                return blastedGroup;

            return GetNextAvailableKalecgosGroup(group, instanceId, state);
        }

        if (state.firstEntrantGuid != ObjectGuid::Empty)
        {
            uint8 entrantGroup = GetAssignedGroup(state, state.firstEntrantGuid);
            if (entrantGroup != KALECGOS_INVALID_GROUP)
                return entrantGroup;

            return GetNextAvailableKalecgosGroup(group, instanceId, state);
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
        if (!bot->GetGroup() || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        Group* group = bot->GetGroup();
        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = GetKalecgosEncounterState(instanceId);
        std::vector<Player*> botMembers;
        std::array<ObjectGuid, KALECGOS_GROUP_COUNT> expectedTankGuids =
            GetExpectedKalecgosTankGuids(botAI, bot);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsInWorld() || member->GetMapId() != SUNWELL_MAP_ID ||
                member->GetInstanceId() != instanceId)
            {
                continue;
            }

            if (GET_PLAYERBOT_AI(member))
                botMembers.push_back(member);
        }

        bool needsRebuild = state.playerToGroup.size() != botMembers.size() ||
                            state.groupTankGuids != expectedTankGuids;
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
        state.groupTankGuids = expectedTankGuids;

        std::array<size_t, KALECGOS_GROUP_COUNT> groupSizes = { 0, 0, 0, 0 };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasTank = { false, false, false, false };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasDecurser = { false, false, false, false };

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
        {
            Player* tank = FindKalecgosGroupMember(group, instanceId, state.groupTankGuids[groupIndex]);
            if (!tank)
                continue;

            AssignPlayerToGroup(state, groupSizes, groupHasTank, groupHasDecurser, tank, groupIndex);
        }

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
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, instanceId, state);
    }

    bool HasReachedKalecgosInitialRangedPosition(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return false;

        auto trackerItr = hasReachedKalecgosInitialRangedPosition.find(bot->GetGUID());
        return trackerItr != hasReachedKalecgosInitialRangedPosition.end() && trackerItr->second;
    }

    void SetKalecgosInitialRangedPositionReached(Player* bot, bool reached)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        if (reached)
            hasReachedKalecgosInitialRangedPosition[bot->GetGUID()] = true;
        else
            hasReachedKalecgosInitialRangedPosition.erase(bot->GetGUID());
    }

    Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = PrepareKalecgosEncounterState(botAI, bot);

        for (Player* tank : GetKalecgosAssignedTankOrder(group, instanceId, state))
        {
            if (!tank || !tank->IsAlive() || tank->GetMapId() != SUNWELL_MAP_ID)
                continue;

            if (IsInKalecgosSpectralRealm(tank))
                continue;

            if (state.activeRiftOpenedMs && IsKalecgosActiveRiftCandidate(tank, state) &&
                !ShouldKalecgosTankStayOnSurface(group, instanceId, state, tank))
            {
                continue;
            }

            return tank;
        }

        return nullptr;
    }

    bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = PrepareKalecgosEncounterState(botAI, bot);
        if (!state.activeRiftOpenedMs)
            return false;

        if (!IsKalecgosActiveRiftCandidate(bot, state))
            return false;

        return CanKalecgosBotEnterRift(group, instanceId, bot, state);
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

    void RecordKalecgosSpectralBlastPortal(PlayerbotAI* botAI, Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        Group* group = bot->GetGroup();
        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = PrepareKalecgosEncounterState(botAI, bot);
        uint32 now = getMSTime();

        state.activeRiftOpenedMs = now;
        state.activeRiftSequence++;
        state.blastedPlayerGuid = bot->GetGUID();
        state.firstEntrantGuid = ObjectGuid::Empty;
        state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, instanceId, state);
    }

    void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        Group* group = bot->GetGroup();
        uint32 instanceId = bot->GetInstanceId();
        KalecgosEncounterState& state = PrepareKalecgosEncounterState(botAI, bot);
        uint32 now = getMSTime();

        KalecgosRealmState& realmState = GetKalecgosRealmState(bot);
        realmState.inSpectralRealm = true;
        realmState.lastEnterMs = now;
        SetKalecgosInitialRangedPositionReached(bot, false);

        if (!state.activeRiftOpenedMs)
        {
            state.activeRiftOpenedMs = now;
            state.activeRiftSequence++;
            state.blastedPlayerGuid = bot->GetGUID();
            state.firstEntrantGuid = ObjectGuid::Empty;
        }

        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = bot->GetGUID();

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, instanceId, state);
    }

    void RecordKalecgosNormalRealmEnter(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosRealmState& realmState = GetKalecgosRealmState(bot);
        realmState.inSpectralRealm = false;
        realmState.lastExitMs = getMSTime();
        SetKalecgosInitialRangedPositionReached(bot, false);
    }

    // Brutallus

    const Position BRUTALLUS_MAIN_TANK_POSITION = { 1484.779f, 582.691f, 23.460f };

    struct BrutallusRangedSlotInfo
    {
        bool isMainTankGroup = false;
        uint8 arcPositionIndex = 0;
    };

    float GetBrutallusMainTankAngle(Unit* brutallus);
    Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z);
    float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
    bool TryGetBrutallusRangedSlotInfo(uint8 rangedIndex, BrutallusRangedSlotInfo& slotInfo);
    float NormalizeSignedAngle(float angle);
    float GetBrutallusRangedRadius(bool isBurnMovement = false);
    float GetBrutallusRangedSlotAngleStep(float radius);
    float GetBrutallusRangedSlotAngle(Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo);
    float GetBrutallusMirroredRangedAngle(float normalAngle, BrutallusRangedSlotInfo const& slotInfo);
    bool IsEligibleBrutallusRangedMember(PlayerbotAI* botAI, Player* bot, Player* member);
    void EnsureBrutallusRangedAssignments(PlayerbotAI* botAI, Player* bot);

    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
    std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

    bool ShouldMoveForBrutallusBurn(Player* bot)
    {
        Aura* burnAura = bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_BURN));
        if (!burnAura)
            return false;

        return burnAura->GetDuration() < 45000;
    }

    Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z)
    {
        if (isMainTank)
        {
            Position position = BRUTALLUS_MAIN_TANK_POSITION;
            position.Relocate(position.GetPositionX(), position.GetPositionY(), z);
            return position;
        }

        float angle = GetBrutallusMainTankAngle(brutallus);
        angle = Position::NormalizeOrientation(angle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        return GetBrutallusPositionAtAngle(brutallus, angle, BRUTALLUS_TANK_POSITION_RADIUS, z);
    }

    bool TryGetBrutallusMeleePosition(Unit* brutallus, uint8 meleeIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        float meleeAngleStep = 2.0f * std::asin(BRUTALLUS_MELEE_SPACING / (2.0f * BRUTALLUS_MELEE_RADIUS));
        uint8 maxSideSlots = static_cast<uint8>(std::floor((BRUTALLUS_MELEE_ARC_ANGLE / 2.0f) / meleeAngleStep));
        uint8 maxMeleeSlots = 1 + 2 * maxSideSlots;
        if (meleeIndex >= maxMeleeSlots)
            return false;

        float baseAngle = Position::NormalizeOrientation(
            GetBrutallusMainTankAngle(brutallus) + BRUTALLUS_MELEE_ARC_CENTER_ANGLE_OFFSET);
        float arcWidth = maxSideSlots * 2.0f * meleeAngleStep;
        float angleOffset = GetCenteredArcSlotAngleOffset(meleeIndex, maxMeleeSlots, arcWidth);

        float angle = Position::NormalizeOrientation(baseAngle + angleOffset);
        position = GetBrutallusPositionAtAngle(brutallus, angle, BRUTALLUS_MELEE_RADIUS, z);
        return true;
    }

    bool TryGetBrutallusRangedPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        position = GetBrutallusPositionAtAngle(brutallus, GetBrutallusRangedSlotAngle(brutallus, slotInfo),
                                               GetBrutallusRangedRadius(), z);
        return true;
    }

    bool TryGetBrutallusRangedBurnStepPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        position = GetBrutallusPositionAtAngle(brutallus, GetBrutallusRangedSlotAngle(brutallus, slotInfo),
                                               GetBrutallusRangedRadius(true), z);
        return true;
    }

    bool TryGetBrutallusRangedBurnMirrorStepPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        position = GetBrutallusPositionAtAngle(brutallus,
                                               GetBrutallusMirroredRangedAngle(normalAngle, slotInfo),
                                               GetBrutallusRangedRadius(true), z);
        return true;
    }

    bool TryGetBrutallusRangedBurnArcPosition(
        Unit* brutallus, uint8 rangedIndex, bool moveTowardMirror,
        float currentX, float currentY, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        float burnRadius = GetBrutallusRangedRadius(true);
        float targetAngle = normalAngle;
        if (moveTowardMirror)
            targetAngle = GetBrutallusMirroredRangedAngle(normalAngle, slotInfo);

        Position center = BRUTALLUS_MAIN_TANK_POSITION;
        center.Relocate(brutallus->GetPositionX(), brutallus->GetPositionY(), brutallus->GetPositionZ());
        float currentAngle = Position::NormalizeOrientation(
            std::atan2(currentY - center.GetPositionY(), currentX - center.GetPositionX()));
        float remainingAngle = NormalizeSignedAngle(targetAngle - currentAngle);
        float stepAngle = 2.0f * std::asin(BRUTALLUS_BURN_ARC_STEP_DISTANCE / (2.0f * burnRadius));
        float nextAngle = targetAngle;

        if (std::fabs(remainingAngle) > stepAngle)
            nextAngle = Position::NormalizeOrientation(currentAngle + std::copysign(stepAngle, remainingAngle));

        position = GetBrutallusPositionAtAngle(brutallus, nextAngle, burnRadius, z);
        return true;
    }

    bool TryGetBrutallusRangedBurnPosition(Unit* brutallus, uint8 rangedIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);

        position = GetBrutallusPositionAtAngle(brutallus,
                                               GetBrutallusMirroredRangedAngle(normalAngle, slotInfo),
                                               GetBrutallusRangedRadius(), z);
        return true;
    }

    bool TryGetBrutallusPositionIndex(PlayerbotAI* botAI, Player* bot, bool wantRanged,
                                      uint8& positionIndex)
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
            if (!member || !member->IsInWorld() || member->GetMapId() != SUNWELL_MAP_ID ||
                member->GetInstanceId() != bot->GetInstanceId())
            {
                continue;
            }

            bool isMelee = botAI->IsMelee(member);
            if ((wantRanged && isMelee) || (!wantRanged && !isMelee) ||
                botAI->IsMainTank(member) || botAI->IsAssistTankOfIndex(member, 0, true))
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

    Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z)
    {
        Position center = BRUTALLUS_MAIN_TANK_POSITION;
        if (brutallus)
        {
            center.Relocate(brutallus->GetPositionX(), brutallus->GetPositionY(),
                            brutallus->GetPositionZ());
        }

        float x = center.GetPositionX() + std::cos(angle) * radius;
        float y = center.GetPositionY() + std::sin(angle) * radius;

        Position position = BRUTALLUS_MAIN_TANK_POSITION;
        position.Relocate(x, y, z);
        return position;
    }

    float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
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

    bool TryGetBrutallusRangedSlotInfo(uint8 rangedIndex, BrutallusRangedSlotInfo& slotInfo)
    {
        if (rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
            return false;

        slotInfo.isMainTankGroup = rangedIndex % 2 == 0;
        slotInfo.arcPositionIndex = (rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP;
        return true;
    }

    float NormalizeSignedAngle(float angle)
    {
        angle = Position::NormalizeOrientation(angle);
        if (angle > M_PI)
            angle -= 2.0f * M_PI;

        return angle;
    }

    float GetBrutallusRangedRadius(bool isBurnMovement)
    {
        float radius = BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
        if (isBurnMovement)
            radius -= BRUTALLUS_BURN_FORWARD_DISTANCE;

        return radius;
    }

    float GetBrutallusRangedSlotAngleStep(float radius)
    {
        float stepRatio = BRUTALLUS_RANGED_SPACING / (2.0f * radius);
        stepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
        return 2.0f * std::asin(stepRatio);
    }

    float GetBrutallusRangedSlotAngle(Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo)
    {
        float frontCenterAngle = Position::NormalizeOrientation(
            GetBrutallusMainTankAngle(brutallus) + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);
        float tankAngle = GetBrutallusMainTankAngle(brutallus);
        if (!slotInfo.isMainTankGroup)
            tankAngle = Position::NormalizeOrientation(tankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        float angleTowardCenter = NormalizeSignedAngle(frontCenterAngle - tankAngle);
        float towardCenterSign = angleTowardCenter < 0.0f ? -1.0f : 1.0f;
        float angleStep = GetBrutallusRangedSlotAngleStep(GetBrutallusRangedRadius());
        float arcHalfWidth = angleStep * static_cast<float>(BRUTALLUS_RANGED_POSITIONS_PER_GROUP - 1) / 2.0f;
        float outerEdgeAngle = Position::NormalizeOrientation(tankAngle - towardCenterSign * arcHalfWidth);

        return Position::NormalizeOrientation(
            outerEdgeAngle + towardCenterSign * angleStep * slotInfo.arcPositionIndex);
    }

    float GetBrutallusMirroredRangedAngle(float normalAngle, BrutallusRangedSlotInfo const& slotInfo)
    {
        float mirrorAngleOffset = BRUTALLUS_BURN_MIRROR_ANGLE_OFFSET;
        if (!slotInfo.isMainTankGroup)
            mirrorAngleOffset = -mirrorAngleOffset;

        return Position::NormalizeOrientation(normalAngle + mirrorAngleOffset);
    }

    bool IsEligibleBrutallusRangedMember(PlayerbotAI* botAI, Player* bot, Player* member)
    {
        if (!member || !member->IsInWorld() || member->GetMapId() != SUNWELL_MAP_ID ||
            member->GetInstanceId() != bot->GetInstanceId())
        {
            return false;
        }

        if (botAI->IsMelee(member) || botAI->IsMainTank(member) ||
            botAI->IsAssistTankOfIndex(member, 0, true))
        {
            return false;
        }

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
            for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_RANGED_POSITIONS; ++slotIndex)
            {
                if (usedPositions[slotIndex])
                    continue;

                assignments[member->GetGUID()] = slotIndex;
                usedPositions[slotIndex] = true;
                return true;
            }

            assignments[member->GetGUID()] = static_cast<uint8>(assignments.size() % BRUTALLUS_TOTAL_RANGED_POSITIONS);
            return true;

            return false;
        };

        std::vector<Player*> healers;
        std::vector<Player*> rangedDamage;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!IsEligibleBrutallusRangedMember(botAI, bot, member))
                continue;

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

    const Position FELMYST_TANK_POSITION = { 0f, 0f, 0f };
}
