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
    // Kalecgos & Sathrovarr the Corruptor

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
        Group* group, KalecgosEncounterState const& state)
    {
        std::array<Player*, KALECGOS_GROUP_COUNT> tanks = { nullptr, nullptr, nullptr, nullptr };

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
            tanks[groupIndex] =
                FindKalecgosGroupMember(group, state.groupTankGuids[groupIndex]);

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
        Group* group, KalecgosEncounterState const& state, Player* currentTank)
    {
        for (Player* tank : GetKalecgosAssignedTankOrder(group, state))
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
        Group* group, KalecgosEncounterState const& state, Player* tank)
    {
        return IsKalecgosAssignedTank(state, tank) &&
               !HasAnotherKalecgosAssignedSurfaceTank(group, state, tank);
    }

    bool CanKalecgosBotEnterRift(
        Group* group, Player* candidate, KalecgosEncounterState const& state)
    {
        if (!IsKalecgosPortalEligibleCandidate(candidate))
            return false;

        if (state.blastedPlayerGuid == candidate->GetGUID())
            return false;

        return !ShouldKalecgosTankStayOnSurface(group, state, candidate);
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
        std::array<ObjectGuid, KALECGOS_GROUP_COUNT> expectedTankGuids =
            GetExpectedKalecgosTankGuids(botAI, bot);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != SUNWELL_MAP_ID)
                continue;

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

        std::array<size_t, KALECGOS_GROUP_COUNT> groupSizes = {
            0, 0, 0, 0 };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasTank = {
            false, false, false, false };
        std::array<bool, KALECGOS_GROUP_COUNT> groupHasDecurser = {
            false, false, false, false };

        for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
        {
            Player* tank = FindKalecgosGroupMember(
                group, state.groupTankGuids[groupIndex]);
            if (!tank)
                continue;

            AssignPlayerToGroup(
                state, groupSizes, groupHasTank, groupHasDecurser, tank, groupIndex);
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
        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);

        for (Player* tank : GetKalecgosAssignedTankOrder(group, state))
        {
            if (!tank || !tank->IsAlive() || tank->GetMapId() != SUNWELL_MAP_ID)
                continue;

            if (IsInKalecgosSpectralRealm(tank))
                continue;

            if (state.activeRiftOpenedMs && IsKalecgosActiveRiftCandidate(tank, state) &&
                !ShouldKalecgosTankStayOnSurface(group, state, tank))
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
        if (!group)
            return false;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);
        if (!state.activeRiftOpenedMs)
            return false;

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
    }

    void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        KalecgosEncounterState& state = GetPreparedKalecgosEncounterState(botAI, bot);
        uint32 now = getMSTime();

        UpdateKalecgosRealmState(bot, true, now);

        if (!state.activeRiftOpenedMs)
        {
            state.activeRiftOpenedMs = now;
            state.blastedPlayerGuid = bot->GetGUID();
            state.firstEntrantGuid = ObjectGuid::Empty;
        }

        if (state.firstEntrantGuid == ObjectGuid::Empty)
            state.firstEntrantGuid = bot->GetGUID();

        if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
            state.activeRiftGroup = ResolveKalecgosActiveRiftGroup(group, state);
    }

    void RecordKalecgosNormalRealmEnter(Player* bot)
    {
        if (bot->GetMapId() != SUNWELL_MAP_ID)
            return;

        UpdateKalecgosRealmState(bot, false, getMSTime());
    }

    // Brutallus

    const Position BRUTALLUS_MAIN_TANK_POSITION = { 1484.779f, 582.691f, 23.460f };

    struct BrutallusRangedSlotInfo
    {
        bool isMainTankGroup = false;
        uint8 arcPositionIndex = 0;
    };

    constexpr float BRUTALLUS_RANGED_LANE_OFFSET = 5.0f;

    float GetBrutallusMainTankAngle(Unit* brutallus);
    Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z);
    float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
    bool TryGetBrutallusRangedSlotInfo(uint8 rangedIndex, BrutallusRangedSlotInfo& slotInfo);
    float NormalizeSignedAngle(float angle);
    float GetBrutallusRangedSlotAngleStep(float radius);
    float GetBrutallusRangedSlotAngle(Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo);
    float GetBrutallusMirroredRangedAngle(float normalAngle, BrutallusRangedSlotInfo const& slotInfo);
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

    bool TryGetBrutallusMeleePosition(
        Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        constexpr float meleeSpacing = 5.0f;
        constexpr float arcAngle = 2.0f * M_PI / 3.0f;

        float meleeRadius = std::max(1.0f, bot->GetMeleeRange(brutallus) - 2.0f); // Tested with hardcoded 18.0f
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

    bool TryGetBrutallusRangedSlotInfo
    (uint8 rangedIndex, BrutallusRangedSlotInfo& slotInfo)
    {
        if (rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
            return false;

        slotInfo.isMainTankGroup = rangedIndex % 2 == 0;
        slotInfo.arcPositionIndex =
            (rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP;

        return true;
    }

    float NormalizeSignedAngle(float angle)
    {
        angle = Position::NormalizeOrientation(angle);
        if (angle > M_PI)
            angle -= 2.0f * M_PI;

        return angle;
    }

    float GetBrutallusNormalRangedRadius()
    {
        return BRUTALLUS_TANK_POSITION_RADIUS + BRUTALLUS_RANGED_TANK_OFFSET;
    }

    float GetBrutallusBurnRangedRadius()
    {
        return GetBrutallusNormalRangedRadius() - BRUTALLUS_RANGED_LANE_OFFSET;
    }

    float GetBrutallusReturnRangedRadius()
    {
        return GetBrutallusNormalRangedRadius() + BRUTALLUS_RANGED_LANE_OFFSET;
    }

    float GetBrutallusRangedSlotAngleStep(float radius)
    {
        constexpr float rangedSpacing = 6.0f;
        float stepRatio = rangedSpacing / (2.0f * radius);
        stepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
        return 2.0f * std::asin(stepRatio);
    }

    float GetBrutallusRangedSlotAngle(
        Unit* brutallus, BrutallusRangedSlotInfo const& slotInfo)
    {
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
        float angleStep = GetBrutallusRangedSlotAngleStep(
            GetBrutallusNormalRangedRadius());
        float arcHalfWidth = angleStep * static_cast<float>(
            BRUTALLUS_RANGED_POSITIONS_PER_GROUP - 1) / 2.0f;
        float outerEdgeAngle = Position::NormalizeOrientation(
            tankAngle - towardCenterSign * arcHalfWidth);

        return Position::NormalizeOrientation(
            outerEdgeAngle + towardCenterSign * angleStep * slotInfo.arcPositionIndex);
    }

    float GetBrutallusMirroredRangedAngle(
        float normalAngle, BrutallusRangedSlotInfo const& slotInfo)
    {
        float mirrorAngleOffset = M_PI_2;
        if (!slotInfo.isMainTankGroup)
            mirrorAngleOffset = -mirrorAngleOffset;

        return Position::NormalizeOrientation(normalAngle + mirrorAngleOffset);
    }

    bool TryGetBrutallusRangedStepPosition(
        Unit* brutallus, uint8 rangedIndex, bool useMirrorAngle,
        float radius, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        float angle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        if (useMirrorAngle)
            angle = GetBrutallusMirroredRangedAngle(angle, slotInfo);

        position = GetBrutallusPositionAtAngle(brutallus, angle, radius, z);
        return true;
    }

    bool TryGetBrutallusRangedArcPosition(
        Unit* brutallus, uint8 rangedIndex, float radius, bool moveTowardMirror,
        float currentX, float currentY, float z, Position& position)
    {
        if (!brutallus)
            return false;

        BrutallusRangedSlotInfo slotInfo;
        if (!TryGetBrutallusRangedSlotInfo(rangedIndex, slotInfo))
            return false;

        float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
        float targetAngle = normalAngle;
        if (moveTowardMirror)
            targetAngle = GetBrutallusMirroredRangedAngle(normalAngle, slotInfo);

        Position center = BRUTALLUS_MAIN_TANK_POSITION;
        center.Relocate(
            brutallus->GetPositionX(), brutallus->GetPositionY(), brutallus->GetPositionZ());

        float currentAngle = Position::NormalizeOrientation(
            std::atan2(currentY - center.GetPositionY(), currentX - center.GetPositionX()));
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
        { 1494.745f, 704.0001f, 50.084652f, 4.7472f },
        { 1469.923f, 703.23914f, 50.08592f, 4.7472f },
        { 1446.5154f, 701.5184f, 50.085438f, 4.7472f },
    }};

    const std::array<Position, 3> FELMYST_FOG_RIGHT_LANES = {{
        { 1492.82f, 515.668f, 50.0833f, 1.4486f },
        { 1466.7322f, 515.5953f, 50.571518f, 1.4486f },
        { 1441.64f, 520.52f, 50.0833f, 1.4486f },
    }};

    const std::array<std::array<Position, 3>, 3> FELMYST_FOG_SAFE_SPOTS = {{
        {{
            { 1466.4141f, 598.4603f, 22.69093f },
            { 1468.8397f, 614.43774f, 22.460419f },
            { 1469.7253f, 628.24084f, 21.587616f },
        }},
        {{
            { 1500.2583f, 613.3685f, 26.30991f },
            { 1499.5287f, 598.9852f, 25.925274f },
            { 1501.3601f, 630.6556f, 25.481695f },
        }},
        {{
            { 1479.5696f, 603.14514f, 23.73047f },
            { 1479.8798f, 583.8124f, 23.262527f },
            { 1477.0249f, 616.52545f, 23.247686f },
        }}
    }};

    const Position FELMYST_FOG_LEFT_SIDE = { 1469.0642f, 729.5854f, 59.823853f, 4.6774f };
    const Position FELMYST_FOG_RIGHT_SIDE = { 1458.5555f, 502.1995f, 59.899513f, 1.605702f };

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
        if (!bot || dangerLane == FelmystFogLane::None)
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

        destination.Relocate(destinationX, destinationY, destinationZ);
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
        destination.Relocate(
            waypoint.GetPositionX(), waypoint.GetPositionY(), waypoint.GetPositionZ(), bot->GetOrientation());
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
        if (!bot || dangerLane == FelmystFogLane::None)
            return false;

        uint8 laneIndex = static_cast<uint8>(dangerLane);
        if (laneIndex >= FELMYST_FOG_SAFE_SPOTS.size())
            return false;

        auto const& safeSpots = FELMYST_FOG_SAFE_SPOTS[laneIndex];
        std::array<uint8, 3> candidateOrder = { 0, 1, 2 };
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
            float destinationX = safeSpot.GetPositionX();
            float destinationY = safeSpot.GetPositionY();
            float destinationZ = safeSpot.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    destinationX, destinationY, destinationZ, false))
                continue;

            Position destination;
            destination.Relocate(destinationX, destinationY, destinationZ);
            destinations[destinationCount++] = destination;
        }

        return destinationCount > 0;
    }

    Position GetFelmystPositionAtAngle(Unit* felmyst, Player* bot, float angle, float radius)
    {
        Position position;
        position.Relocate(felmyst->GetPositionX() + std::cos(angle) * radius,
                          felmyst->GetPositionY() + std::sin(angle) * radius,
                          bot->GetPositionZ());
        return position;
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
        position = GetFelmystPositionAtAngle(felmyst, bot, sideAngle, sideDistance);
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

    // Eredar Twins (Grand Warlock Alythess and Lady Sacrolash)

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

    // M'uru & Entropius

    const Position MURU_STACK_POSITION = { 1836.738f, 608.646f, 71.222f };
    const Position MURU_VOID_SENTINEL_N_TANK_POSITION = { 1840.448f, 630.605f, 70.567f };
    const Position MURU_VOID_SENTINEL_E_TANK_POSITION = { 1814.960f, 601.646f, 70.547f };

    std::unordered_map<uint32, MuruDarknessState> muruDarknessStates;

    Unit* GetNearestNonTankPlayerInRadius(PlayerbotAI* botAI, Player* bot, float radius)
    {
        Unit* nearestPlayer = nullptr;
        float nearestDistance = radius;

        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot || botAI->IsTank(member))
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestPlayer = member;
            }
        }

        return nearestPlayer;
    }

    const Position* GetClosestVoidSentinelTankPosition(Unit* voidSentinel, Player* bot)
    {
        if (!voidSentinel)
            return nullptr;

        const Position& north = MURU_VOID_SENTINEL_N_TANK_POSITION;
        const Position& east = MURU_VOID_SENTINEL_E_TANK_POSITION;
        return (voidSentinel->GetExactDist2d(north.GetPositionX(), north.GetPositionY()) <=
                voidSentinel->GetExactDist2d(east.GetPositionX(), east.GetPositionY())) ? &north : &east;
    }

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

        if (Aura* darknessPreEffect = muru->GetAura(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_PRE_EFFECT)))
        {
            int32 remainingPreEffectMs = darknessPreEffect->GetDuration();
            if (remainingPreEffectMs < 0)
                remainingPreEffectMs = darknessPreEffectMs;

            state.expireMs = std::max(state.expireMs,
                                      now + static_cast<uint32>(remainingPreEffectMs) +
                                      darknessCastMs + darknessPostCastDangerMs);
            return true;
        }

        if (muru->HasUnitState(UNIT_STATE_CASTING) &&
            muru->FindCurrentSpellBySpellId(static_cast<uint32>(SunwellSpells::SPELL_DARKNESS)))
        {
            state.expireMs = std::max(state.expireMs, now + darknessCastMs + darknessPostCastDangerMs);
            return true;
        }

        if (state.expireMs > now)
            return true;

        muruDarknessStates.erase(instanceId);
        return false;
    }

    bool DoesMuruUnitHaveTankAggro(PlayerbotAI* botAI, Unit* unit)
    {
        if (!unit || !unit->IsAlive())
            return false;

        Player* victim = unit->GetVictim() ? unit->GetVictim()->ToPlayer() : nullptr;
        return victim && botAI->IsTank(victim);
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

    Unit* GetVoidSpawnVolleyPriorityTarget(
        PlayerbotAI* botAI, Player* bot, Unit* muru, Unit* entropius)
    {
        MuruEncounterTargets targets;
        targets.muru = muru;
        targets.entropius = entropius;
        GatherMuruEncounterTargets(botAI, targets);

        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        constexpr float targetSwitchDistance = 10.0f;
        auto chooseNearestTarget = [&](Unit*& current, Unit* candidate)
        {
            if (!candidate)
                return;

            if (!current)
            {
                current = candidate;
                return;
            }

            if (current == candidate)
                return;

            float currentDistance = bot->GetExactDist2d(current);
            float candidateDistance = bot->GetExactDist2d(candidate);
            if (candidateDistance + targetSwitchDistance < currentDistance)
                current = candidate;
        };

        auto selectEncounterTarget = [&](uint32 entry, std::vector<Unit*> const& candidates)
        {
            Unit* selected = nullptr;
            if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
                selected = currentTarget;

            for (Unit* candidate : candidates)
                chooseNearestTarget(selected, candidate);

            return selected;
        };

        Unit* furyMage = selectEncounterTarget(
            static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages);
        Unit* berserker = selectEncounterTarget(
            static_cast<uint32>(SunwellNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers);
        Unit* voidSentinel = selectEncounterTarget(
            static_cast<uint32>(SunwellNpcs::NPC_VOID_SENTINEL), targets.voidSentinels);

        Unit* validMuru = targets.muru;
        if (!validMuru || validMuru->GetHealth() <= 1 ||
            TryGetMuruDarknessActiveState(bot, validMuru))
        {
            validMuru = nullptr;
        }

        std::array<Unit*, 5> priorities = {
            furyMage,
            berserker,
            voidSentinel,
            validMuru,
            targets.entropius
        };

        for (Unit* target : priorities)
        {
            if (target && target->IsAlive())
                return target;
        }

        return nullptr;
    }

    bool CommandControlledCreatureToAttack(Unit* controlled, Unit* target)
    {
        if (!controlled || !controlled->IsAlive() ||
            !target || !target->IsAlive() ||
            controlled->GetVictim() == target)
        {
            return false;
        }

        controlled->ClearUnitState(UNIT_STATE_FOLLOW);
        controlled->AttackStop();
        controlled->SetTarget(target->GetGUID());

        if (CharmInfo* charmInfo = controlled->GetCharmInfo())
        {
            charmInfo->SetIsCommandAttack(true);
            charmInfo->SetIsAtStay(false);
            charmInfo->SetIsFollowing(false);
            charmInfo->SetIsCommandFollow(false);
            charmInfo->SetIsReturning(false);
        }

        if (!controlled->IsPlayer() && controlled->IsCreature() &&
            controlled->ToCreature()->IsAIEnabled)
        {
            controlled->ToCreature()->AI()->AttackStart(target);
        }
        else
        {
            controlled->Attack(target, true);
        }

        return true;
    }

    // Kil'jaeden <The Deceiver>

    // Combat reach is 15 yards
    const Position KILJAEDEN_CENTER_POSITION = { 1698.450f, 628.030f, 28.199f }; // Starting position for KJ
    // const Position KILJAEDEN_TANK_POSITION = { 1709.474f, 641.598f, 27.582f };
    const Position KILJAEDEN_TANK_POSITION = { 1704.729f, 634.891f, 27.787f };
    // const Position KILJAEDEN_S_MELEE_POSITION = { 1681.527f, 628.600f, 27.610f };
    const Position KILJAEDEN_S_MELEE_POSITION = { 1689.487f, 632.119f, 27.823f };
    // const Position KILJAEDEN_E_MELEE_POSITION = { 1702.280f, 611.716f, 27.553f };
    const Position KILJAEDEN_E_MELEE_POSITION = { 1700.542f, 619.589f, 27.786f };
    const Position KILJAEDEN_STACK_POSITION = { 1709.768f, 642.241f, 27.706f };

    std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> kiljaedenRangedAssignments;
    std::unordered_map<uint32, std::vector<KiljaedenHazard>> kiljaedenHazards;

    void PruneExpiredKiljaedenHazards(uint32 instanceId)
    {
        auto instanceItr = kiljaedenHazards.find(instanceId);
        if (instanceItr == kiljaedenHazards.end())
            return;

        uint32 now = getMSTime();
        std::vector<KiljaedenHazard>& hazards = instanceItr->second;
        hazards.erase(std::remove_if(hazards.begin(), hazards.end(),
            [now](KiljaedenHazard const& hazard) {
                return !hazard.expireMs || hazard.expireMs <= now;
            }), hazards.end());

        if (hazards.empty())
            kiljaedenHazards.erase(instanceItr);
    }

    void AddKiljaedenHazard(
        uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance)
    {
        if (!durationMs || safeDistance <= 0.0f)
            return;

        uint32 now = getMSTime();
        PruneExpiredKiljaedenHazards(instanceId);

        KiljaedenHazard hazard;
        hazard.destination = destination;
        hazard.expireMs = now + durationMs;
        hazard.safeDistance = safeDistance;
        kiljaedenHazards[instanceId].push_back(hazard);
    }

    bool HasActiveKiljaedenHazard(uint32 instanceId)
    {
        PruneExpiredKiljaedenHazards(instanceId);
        auto instanceItr = kiljaedenHazards.find(instanceId);
        return instanceItr != kiljaedenHazards.end() && !instanceItr->second.empty();
    }

    bool TryGetKiljaedenNearestHazard(Player* bot, KiljaedenHazard& hazard)
    {
        PruneExpiredKiljaedenHazards(bot->GetInstanceId());
        auto instanceItr = kiljaedenHazards.find(bot->GetInstanceId());
        if (instanceItr == kiljaedenHazards.end())
            return false;

        bool foundHazard = false;
        float bestDistance = 0.0f;

        for (KiljaedenHazard const& candidate : instanceItr->second)
        {
            float distance = bot->GetExactDist2d(
                candidate.destination.GetPositionX(), candidate.destination.GetPositionY());
            if (distance >= candidate.safeDistance)
                continue;

            if (!foundHazard || distance < bestDistance)
            {
                hazard = candidate;
                bestDistance = distance;
                foundHazard = true;
            }
        }

        return foundHazard;
    }

    bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden)
    {
        return kiljaeden && kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
               kiljaeden->FindCurrentSpellBySpellId(
                   static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
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

    bool TryGetKiljaedenRangedPosition(PlayerbotAI* botAI, Player* bot, Position& position)
    {
        Group* group = bot->GetGroup();
        if (!group || !botAI->IsRanged(bot))
            return false;

        EnsureKiljaedenRangedAssignments(botAI, bot);

        auto instanceItr = kiljaedenRangedAssignments.find(bot->GetInstanceId());
        if (instanceItr == kiljaedenRangedAssignments.end())
            return false;

        auto assignmentItr = instanceItr->second.find(bot->GetGUID());
        if (assignmentItr == instanceItr->second.end())
            return false;

        uint8 slotIndex = assignmentItr->second;
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

        position.Relocate(positionX, positionY, KILJAEDEN_CENTER_POSITION.GetPositionZ());
        return true;
    }
}
