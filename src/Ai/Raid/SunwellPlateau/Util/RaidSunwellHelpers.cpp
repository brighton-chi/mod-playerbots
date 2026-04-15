/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <list>
#include <sstream>
#include <vector>

#include "RaidSunwellHelpers.h"
#include "RaidBossHelpers.h"
#include "Playerbots.h"

namespace SunwellHelpers
{
    Position InterpolateLanePoint(Position const& start, Position const& end, float factor, float z);

    namespace
    {
        std::unordered_map<ObjectGuid, std::string> felmystFogDebugMessages;

        char const* GetFelmystFogLaneName(FelmystFogLane lane)
        {
            switch (lane)
            {
                case FelmystFogLane::Top:
                    return "top";
                case FelmystFogLane::Middle:
                    return "middle";
                case FelmystFogLane::Bottom:
                    return "bottom";
                default:
                    return "none";
            }
        }

        char const* GetFelmystFogPhaseName(FelmystFogPhase phase)
        {
            switch (phase)
            {
                case FelmystFogPhase::Windup:
                    return "windup";
                case FelmystFogPhase::Sweep:
                    return "sweep";
                case FelmystFogPhase::Recovery:
                    return "recovery";
                default:
                    return "none";
            }
        }

        bool ShouldLogFelmystFogDebug(Player* bot)
        {
            if (PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr)
            {
                return botAI->HasStrategy("debug", BOT_STATE_NON_COMBAT) ||
                    botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT);
            }

            return false;
        }

        std::string FormatFelmystFogPoint(Position const& position)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(2)
                << '(' << position.GetPositionX() << ", "
                << position.GetPositionY() << ", "
                << position.GetPositionZ() << ')';
            return out.str();
        }

        void LogFelmystFogDebug(Player* bot, std::string const& message)
        {
            if (!ShouldLogFelmystFogDebug(bot))
                return;

            std::string& lastMessage = felmystFogDebugMessages[bot->GetGUID()];
            if (lastMessage == message)
                return;

            lastMessage = message;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                botAI->TellMasterNoFacing("[felmyst fog] " + message);

            LOG_DEBUG("playerbots", "[FelmystFog] {} {}", bot->GetName(), message);
        }

        float GetLaneProjectionFactor(float positionX, float positionY, Position const& start, Position const& end)
        {
            float deltaX = end.GetPositionX() - start.GetPositionX();
            float deltaY = end.GetPositionY() - start.GetPositionY();
            float segmentLengthSquared = deltaX * deltaX + deltaY * deltaY;
            if (segmentLengthSquared <= 0.0f)
                return 0.0f;

            float projection = ((positionX - start.GetPositionX()) * deltaX +
                (positionY - start.GetPositionY()) * deltaY) / segmentLengthSquared;
            return std::clamp(projection, 0.0f, 1.0f);
        }

        float GetLaneProjectionFactor(Player* bot, Position const& start, Position const& end)
        {
            return GetLaneProjectionFactor(bot->GetPositionX(), bot->GetPositionY(), start, end);
        }
    }

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

    // I was using a hardcoded radius to calculate distance for melee; trying GetMeleeRange, need to test
    bool TryGetBrutallusMeleePosition(
        Player* bot, Unit* brutallus, uint8 meleeIndex, float z, Position& position)
    {
        if (!brutallus)
            return false;

        float meleeRadius = std::max(1.0f, bot->GetMeleeRange(brutallus) - 2.0f);

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
        position = GetBrutallusPositionAtAngle(brutallus, angle, meleeRadius, z);
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

    const Position FELMYST_TANK_POSITION = { 1476.624f, 620.888f, 22.490f };
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

    bool TryGetFelmystFogLaneEndpoints(FelmystFogLane lane, Position& left, Position& right)
    {
        uint8 laneIndex = static_cast<uint8>(lane);
        if (lane == FelmystFogLane::None || laneIndex >= FELMYST_FOG_LEFT_LANES.size())
            return false;

        left = FELMYST_FOG_LEFT_LANES[laneIndex];
        right = FELMYST_FOG_RIGHT_LANES[laneIndex];
        return true;
    }

    bool TryGetFelmystFogLaneSpacing(FelmystFogLane firstLane, FelmystFogLane secondLane, float projection, float& spacing)
    {
        Position firstLeft;
        Position firstRight;
        Position secondLeft;
        Position secondRight;
        if (!TryGetFelmystFogLaneEndpoints(firstLane, firstLeft, firstRight) ||
            !TryGetFelmystFogLaneEndpoints(secondLane, secondLeft, secondRight))
        {
            return false;
        }

        Position firstPoint = InterpolateLanePoint(firstLeft, firstRight, projection, 0.0f);
        Position secondPoint = InterpolateLanePoint(secondLeft, secondRight, projection, 0.0f);
        spacing = std::hypot(
            secondPoint.GetPositionX() - firstPoint.GetPositionX(),
            secondPoint.GetPositionY() - firstPoint.GetPositionY());
        return true;
    }

    float GetFelmystFogLaneHalfWidth(FelmystFogLane lane, float projection)
    {
        float spacing = 0.0f;
        switch (lane)
        {
            case FelmystFogLane::Top:
                return TryGetFelmystFogLaneSpacing(FelmystFogLane::Top, FelmystFogLane::Middle, projection, spacing) ?
                    spacing * 0.5f : 0.0f;
            case FelmystFogLane::Middle:
            {
                float topSpacing = 0.0f;
                float bottomSpacing = 0.0f;
                bool hasTopSpacing = TryGetFelmystFogLaneSpacing(
                    FelmystFogLane::Middle, FelmystFogLane::Top, projection, topSpacing);
                bool hasBottomSpacing = TryGetFelmystFogLaneSpacing(
                    FelmystFogLane::Middle, FelmystFogLane::Bottom, projection, bottomSpacing);
                if (hasTopSpacing && hasBottomSpacing)
                    return std::min(topSpacing, bottomSpacing) * 0.5f;
                if (hasTopSpacing)
                    return topSpacing * 0.5f;
                if (hasBottomSpacing)
                    return bottomSpacing * 0.5f;
                return 0.0f;
            }
            case FelmystFogLane::Bottom:
                return TryGetFelmystFogLaneSpacing(FelmystFogLane::Bottom, FelmystFogLane::Middle, projection, spacing) ?
                    spacing * 0.5f : 0.0f;
            default:
                return 0.0f;
        }
    }

    float GetFelmystFogLanePointBoundaryRadius(FelmystFogLane lane, bool useLeftPoint)
    {
        uint8 laneIndex = static_cast<uint8>(lane);
        Position const& currentPoint = useLeftPoint ?
            FELMYST_FOG_LEFT_LANES[laneIndex] : FELMYST_FOG_RIGHT_LANES[laneIndex];
        float bestSpacing = std::numeric_limits<float>::max();

        auto considerAdjacentLane = [&](FelmystFogLane adjacentLane)
        {
            uint8 adjacentIndex = static_cast<uint8>(adjacentLane);
            Position const& adjacentPoint = useLeftPoint ?
                FELMYST_FOG_LEFT_LANES[adjacentIndex] : FELMYST_FOG_RIGHT_LANES[adjacentIndex];
            float spacing = std::hypot(
                adjacentPoint.GetPositionX() - currentPoint.GetPositionX(),
                adjacentPoint.GetPositionY() - currentPoint.GetPositionY());
            if (spacing < bestSpacing)
                bestSpacing = spacing;
        };

        if (lane == FelmystFogLane::Top || lane == FelmystFogLane::Middle)
            considerAdjacentLane(lane == FelmystFogLane::Top ? FelmystFogLane::Middle : FelmystFogLane::Top);

        if (lane == FelmystFogLane::Bottom || lane == FelmystFogLane::Middle)
            considerAdjacentLane(lane == FelmystFogLane::Bottom ? FelmystFogLane::Middle : FelmystFogLane::Bottom);

        return bestSpacing == std::numeric_limits<float>::max() ? 0.0f : bestSpacing * 0.5f;
    }

    const std::array<std::array<Position, 6>, 2> FELMYST_DEMONIC_VAPOR_KITE_PATHS = {{
        {{
            { 1457.426f, 617.279f, 20.350f },
            { 1451.897f, 597.726f, 20.697f },
            { 1459.353f, 574.804f, 21.972f },
            { 1484.936f, 572.094f, 23.407f },
            { 1499.060f, 588.701f, 24.920f },
            { 1495.719f, 607.127f, 25.607f },
        }},
        {{
            { 1496.546f, 624.383f, 25.052f },
            { 1495.697f, 642.868f, 23.572f },
            { 1485.558f, 661.212f, 21.159f },
            { 1465.801f, 657.336f, 19.982f },
            { 1449.653f, 639.291f, 17.918f },
            { 1458.564f, 615.200f, 20.582f },
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
            mainTank->GetMapId() == felmyst->GetMapId() &&
            mainTank->GetInstanceId() == felmyst->GetInstanceId())
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

    Creature* GetDemonicVaporSummonedByBot(PlayerbotAI* botAI, Player* carrier)
    {
        if (!carrier)
            return nullptr;

        auto const& potentialTargets =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
        for (ObjectGuid const& targetGuid : potentialTargets)
        {
            Unit* unit = botAI->GetUnit(targetGuid);
            Creature* creature = unit ? unit->ToCreature() : nullptr;
            if (!creature || !creature->IsAlive() ||
                creature->GetEntry() != static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR))
            {
                continue;
            }

            if (creature->GetMapId() != carrier->GetMapId() ||
                creature->GetInstanceId() != carrier->GetInstanceId())
            {
                continue;
            }

            if (creature->GetSummonerGUID() == carrier->GetGUID())
                return creature;
        }

        return nullptr;
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

        Position const& currentWaypoint = path[waypointIndex];
        if (bot->GetExactDist2d(currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY()) <=
            FELMYST_DEMONIC_VAPOR_WAYPOINT_REACHED_DISTANCE)
        {
            return nextWaypointIndex;
        }

        uint8 nearestWaypointIndex = GetNearestFelmystDemonicVaporWaypointIndex(bot, pathIndex);
        if (nearestWaypointIndex == nextWaypointIndex)
        {
            Position const& nextWaypoint = path[nextWaypointIndex];
            float currentDistance = bot->GetExactDist2d(
                currentWaypoint.GetPositionX(), currentWaypoint.GetPositionY());
            float nextDistance = bot->GetExactDist2d(
                nextWaypoint.GetPositionX(), nextWaypoint.GetPositionY());
            if (nextDistance + 1.0f < currentDistance)
                return nextWaypointIndex;
        }

        return waypointIndex;
    }

    bool TryGetFelmystFogLaneFromPosition(float positionX, float positionY, FelmystFogLane& lane)
    {
        float bestDistance = std::numeric_limits<float>::max();
        float bestProjection = 0.0f;
        FelmystFogLane bestLane = FelmystFogLane::None;

        for (uint8 laneIndex = 0; laneIndex < FELMYST_FOG_LEFT_LANES.size(); ++laneIndex)
        {
            Position const& left = FELMYST_FOG_LEFT_LANES[laneIndex];
            Position const& right = FELMYST_FOG_RIGHT_LANES[laneIndex];
            float projection = GetLaneProjectionFactor(positionX, positionY, left, right);
            Position lanePoint = InterpolateLanePoint(left, right, projection, 0.0f);
            float segmentDistance = std::hypot(
                positionX - lanePoint.GetPositionX(),
                positionY - lanePoint.GetPositionY());
            if (segmentDistance < bestDistance)
            {
                bestDistance = segmentDistance;
                bestProjection = projection;
                bestLane = static_cast<FelmystFogLane>(laneIndex);
            }
        }

        if (bestLane == FelmystFogLane::None)
            return false;

        float halfWidth = GetFelmystFogLaneHalfWidth(bestLane, bestProjection);
        if (halfWidth <= 0.0f || bestDistance > halfWidth)
            return false;

        lane = bestLane;
        return true;
    }

    std::array<uint32, 2> GetFelmystDemonicVaporPathOccupancyCounts(PlayerbotAI* botAI, Player* bot)
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

                    if (!GetDemonicVaporSummonedByBot(botAI, member))
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
                {
                    continue;
                }

                if (entry == static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR) &&
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

        addHazardsOnPath(static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR));
        addHazardsOnPath(static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR_TRAIL));

        return occupancyCounts;
    }

    uint8 SelectFelmystDemonicVaporPath(PlayerbotAI* botAI, Player* bot)
    {
        std::array<uint32, 2> occupancyCounts = GetFelmystDemonicVaporPathOccupancyCounts(botAI, bot);
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

    bool TryGetFelmystDemonicVaporKiteDestination(
        PlayerbotAI* botAI, Player* bot, Position& destination)
    {
        uint32 instanceId = bot->GetInstanceId();
        ObjectGuid guid = bot->GetGUID();

        if (!GetDemonicVaporSummonedByBot(botAI, bot))
        {
            auto pathInstanceItr = felmystDemonicVaporPathIndices.find(instanceId);
            if (pathInstanceItr != felmystDemonicVaporPathIndices.end())
            {
                pathInstanceItr->second.erase(guid);
                if (pathInstanceItr->second.empty())
                    felmystDemonicVaporPathIndices.erase(pathInstanceItr);
            }

            auto instanceItr = felmystDemonicVaporWaypointIndices.find(instanceId);
            if (instanceItr != felmystDemonicVaporWaypointIndices.end())
            {
                instanceItr->second.erase(guid);
                if (instanceItr->second.empty())
                    felmystDemonicVaporWaypointIndices.erase(instanceItr);
            }

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
            pathIndex = SelectFelmystDemonicVaporPath(botAI, bot);
            pathIndices[guid] = pathIndex;
        }
        else
        {
            pathIndex = pathItr->second;
        }

        auto const& path = FELMYST_DEMONIC_VAPOR_KITE_PATHS[pathIndex];
        if (waypointItr == waypointIndices.end())
        {
            waypointIndex = GetNearestFelmystDemonicVaporWaypointIndex(bot, pathIndex);
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

    bool TryGetFelmystFogLaneFromLanePoints(Unit* felmyst, FelmystFogLane& lane)
    {
        if (!felmyst)
            return false;

        float bestDistance = std::numeric_limits<float>::max();
        FelmystFogLane bestLane = FelmystFogLane::None;
        bool bestPointWasLeft = true;

        for (uint8 laneIndex = 0; laneIndex < FELMYST_FOG_LEFT_LANES.size(); ++laneIndex)
        {
            float leftDistance = felmyst->GetExactDist2d(
                FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionX(),
                FELMYST_FOG_LEFT_LANES[laneIndex].GetPositionY());
            if (leftDistance < bestDistance)
            {
                bestDistance = leftDistance;
                bestLane = static_cast<FelmystFogLane>(laneIndex);
                bestPointWasLeft = true;
            }

            float rightDistance = felmyst->GetExactDist2d(
                FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionX(),
                FELMYST_FOG_RIGHT_LANES[laneIndex].GetPositionY());
            if (rightDistance < bestDistance)
            {
                bestDistance = rightDistance;
                bestLane = static_cast<FelmystFogLane>(laneIndex);
                bestPointWasLeft = false;
            }
        }

        if (bestLane == FelmystFogLane::None)
            return false;

        float boundaryRadius = GetFelmystFogLanePointBoundaryRadius(bestLane, bestPointWasLeft);
        if (boundaryRadius <= 0.0f || bestDistance > boundaryRadius)
            return false;

        lane = bestLane;
        return true;
    }

    bool TryGetFelmystFogLaneFromLaneSegments(Unit* felmyst, FelmystFogLane& lane)
    {
        if (!felmyst)
            return false;

        return TryGetFelmystFogLaneFromPosition(
            felmyst->GetPositionX(), felmyst->GetPositionY(), lane);
    }

    Position InterpolateLanePoint(Position const& start, Position const& end, float factor, float z)
    {
        Position point;
        point.Relocate(
            start.GetPositionX() + (end.GetPositionX() - start.GetPositionX()) * factor,
            start.GetPositionY() + (end.GetPositionY() - start.GetPositionY()) * factor,
            z);
        return point;
    }

    bool TryGetFelmystFogLaneAnchorDestination(
        Player* bot, FelmystFogLane safeLane, float projectionHint, Position& destination)
    {
        Position safeLeft;
        Position safeRight;
        if (!TryGetFelmystFogLaneEndpoints(safeLane, safeLeft, safeRight))
            return false;

        std::array<float, 8> projectionCandidates = {{
            std::clamp(projectionHint, 0.0f, 1.0f),
            std::clamp(projectionHint - 0.15f, 0.0f, 1.0f),
            std::clamp(projectionHint + 0.15f, 0.0f, 1.0f),
            std::clamp(projectionHint - 0.30f, 0.0f, 1.0f),
            std::clamp(projectionHint + 0.30f, 0.0f, 1.0f),
            0.5f,
            0.0f,
            1.0f,
        }};

        for (float projection : projectionCandidates)
        {
            Position lanePoint = InterpolateLanePoint(safeLeft, safeRight, projection, bot->GetPositionZ());
            float destinationX = lanePoint.GetPositionX();
            float destinationY = lanePoint.GetPositionY();
            float destinationZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    destinationX, destinationY, destinationZ, false))
            {
                continue;
            }

            destination.Relocate(destinationX, destinationY, destinationZ);
            std::ostringstream out;
            out << std::fixed << std::setprecision(2)
                << "secondary anchor safe=" << GetFelmystFogLaneName(safeLane)
                << " projection=" << projection
                << " destination=" << FormatFelmystFogPoint(destination);
            LogFelmystFogDebug(bot, out.str());
            return true;
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "secondary anchor failed safe=" << GetFelmystFogLaneName(safeLane)
            << " projectionHint=" << projectionHint;
        LogFelmystFogDebug(bot, out.str());
        return false;
    }

    bool TryGetFelmystFogShiftDestinationForLane(
        Player* bot, FelmystFogLane dangerLane, FelmystFogLane safeLane,
        Position& destination, float& requiredDistance)
    {
        Position dangerLeft;
        Position dangerRight;
        Position safeLeft;
        Position safeRight;
        if (!TryGetFelmystFogLaneEndpoints(dangerLane, dangerLeft, dangerRight) ||
            !TryGetFelmystFogLaneEndpoints(safeLane, safeLeft, safeRight))
        {
            return false;
        }

        float projection = GetLaneProjectionFactor(bot, dangerLeft, dangerRight);
        Position dangerPoint = InterpolateLanePoint(dangerLeft, dangerRight, projection, bot->GetPositionZ());
        Position safePoint = InterpolateLanePoint(safeLeft, safeRight, projection, bot->GetPositionZ());

        float shiftX = safePoint.GetPositionX() - dangerPoint.GetPositionX();
        float shiftY = safePoint.GetPositionY() - dangerPoint.GetPositionY();
        float laneSpacing = std::hypot(shiftX, shiftY);
        if (laneSpacing <= 0.01f)
        {
            std::ostringstream out;
            out << "primary shift aborted danger=" << GetFelmystFogLaneName(dangerLane)
                << " safe=" << GetFelmystFogLaneName(safeLane)
                << " reason=zero-lane-spacing";
            LogFelmystFogDebug(bot, out.str());
            return false;
        }

        float unitX = shiftX / laneSpacing;
        float unitY = shiftY / laneSpacing;
        float currentOffset =
            (bot->GetPositionX() - dangerPoint.GetPositionX()) * unitX +
            (bot->GetPositionY() - dangerPoint.GetPositionY()) * unitY;
        float boundaryOffset = laneSpacing * 0.5f;
        float targetOffset = boundaryOffset + FELMYST_FOG_BOUNDARY_MARGIN;
        requiredDistance = targetOffset - currentOffset;
        if (requiredDistance <= 0.0f)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(2)
                << "primary shift skipped danger=" << GetFelmystFogLaneName(dangerLane)
                << " safe=" << GetFelmystFogLaneName(safeLane)
                << " currentOffset=" << currentOffset
                << " boundaryOffset=" << boundaryOffset
                << " targetOffset=" << targetOffset;
            LogFelmystFogDebug(bot, out.str());
            return false;
        }

        float desiredDistance = std::clamp(
            requiredDistance, FELMYST_FOG_SHIFT_MIN_STEP, FELMYST_FOG_SHIFT_MAX_STEP);
        float lastValidX = bot->GetPositionX();
        float lastValidY = bot->GetPositionY();
        float lastValidZ = bot->GetPositionZ();
        bool foundValidPoint = false;

        for (float currentStep = 1.0f; currentStep <= desiredDistance; currentStep += 1.0f)
        {
            float testX = bot->GetPositionX() + unitX * currentStep;
            float testY = bot->GetPositionY() + unitY * currentStep;
            float testZ = bot->GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    testX, testY, testZ, false))
            {
                break;
            }

            foundValidPoint = true;
            lastValidX = testX;
            lastValidY = testY;
            lastValidZ = testZ;
        }

        if (!foundValidPoint)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(2)
                << "primary shift blocked danger=" << GetFelmystFogLaneName(dangerLane)
                << " safe=" << GetFelmystFogLaneName(safeLane)
                << " projection=" << projection
                << " laneSpacing=" << laneSpacing
                << " required=" << requiredDistance
                << " desired=" << desiredDistance
                << " fallback=anchor";
            LogFelmystFogDebug(bot, out.str());
            return TryGetFelmystFogLaneAnchorDestination(bot, safeLane, projection, destination);
        }

        destination.Relocate(lastValidX, lastValidY, lastValidZ);
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "primary shift danger=" << GetFelmystFogLaneName(dangerLane)
            << " safe=" << GetFelmystFogLaneName(safeLane)
            << " projection=" << projection
            << " laneSpacing=" << laneSpacing
            << " currentOffset=" << currentOffset
            << " boundaryOffset=" << boundaryOffset
            << " targetOffset=" << targetOffset
            << " required=" << requiredDistance
            << " desired=" << desiredDistance
            << " destination=" << FormatFelmystFogPoint(destination);
        LogFelmystFogDebug(bot, out.str());
        return true;
    }

    bool TryGetFelmystFogLaneFromAirPosition(Unit* felmyst, FelmystFogLane& lane)
    {
        if (!felmyst || !felmyst->IsFlying())
            return false;

        if (TryGetFelmystFogLaneFromLanePoints(felmyst, lane))
            return true;

        return TryGetFelmystFogLaneFromLaneSegments(felmyst, lane);
    }

    bool GetActiveFelmystFogOfCorruptionState(
        Player* bot, Unit* felmyst, FelmystFogOfCorruptionState& state)
    {
        state = FelmystFogOfCorruptionState();

        auto instanceItr = felmystFogOfCorruptionStates.find(bot->GetInstanceId());
        uint32 now = getMSTime();

        if (!felmyst || !felmyst->IsFlying())
        {
            if (instanceItr != felmystFogOfCorruptionStates.end())
                felmystFogOfCorruptionStates.erase(instanceItr);

            return false;
        }

        FelmystFogOfCorruptionState& tracker = felmystFogOfCorruptionStates[bot->GetInstanceId()];
        bool hasTracker = tracker.phase != FelmystFogPhase::None;
        FelmystFogLane observedLane = FelmystFogLane::None;
        bool nearLanePoint = TryGetFelmystFogLaneFromLanePoints(felmyst, observedLane);
        bool isSweeping = felmyst->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FELMYST_SPEED_BURST));
        bool nearLaneSegment = false;

        if (!nearLanePoint && (isSweeping || hasTracker))
            nearLaneSegment = TryGetFelmystFogLaneFromLaneSegments(felmyst, observedLane);

        if (nearLanePoint)
        {
            if (tracker.lane != observedLane)
                tracker.firstObservedMs = now;

            tracker.lane = observedLane;
            tracker.phase = FelmystFogPhase::Windup;
            tracker.lastObservedMs = now;
            tracker.expireMs = now + FELMYST_FOG_WINDUP_GRACE_MS;
            if (!tracker.firstObservedMs)
                tracker.firstObservedMs = now;

            state = tracker;
            LogFelmystFogDebug(bot,
                std::string("state lane=") + GetFelmystFogLaneName(state.lane) +
                " phase=" + GetFelmystFogPhaseName(state.phase) +
                " source=lane-point");
            return true;
        }

        if (isSweeping || nearLaneSegment)
        {
            if (tracker.lane != observedLane && observedLane != FelmystFogLane::None)
                tracker.firstObservedMs = now;

            if (observedLane != FelmystFogLane::None)
                tracker.lane = observedLane;

            tracker.phase = FelmystFogPhase::Sweep;
            tracker.lastObservedMs = now;
            tracker.expireMs = now + FELMYST_FOG_RECOVERY_GRACE_MS;
            if (!tracker.firstObservedMs)
                tracker.firstObservedMs = now;

            state = tracker;
            LogFelmystFogDebug(bot,
                std::string("state lane=") + GetFelmystFogLaneName(state.lane) +
                " phase=" + GetFelmystFogPhaseName(state.phase) +
                " source=" + (nearLaneSegment ? "lane-segment" : "speed-burst"));
            return tracker.lane != FelmystFogLane::None;
        }

        if (hasTracker && tracker.expireMs > now && tracker.lane != FelmystFogLane::None)
        {
            tracker.phase = FelmystFogPhase::Recovery;
            state = tracker;
            LogFelmystFogDebug(bot,
                std::string("state lane=") + GetFelmystFogLaneName(state.lane) +
                " phase=" + GetFelmystFogPhaseName(state.phase) +
                " source=grace-window");
            return true;
        }

        felmystFogOfCorruptionStates.erase(bot->GetInstanceId());
        LogFelmystFogDebug(bot, "state cleared");
        return false;
    }

    bool TryGetFelmystFogSidewaysShiftDestination(
        Player* bot, FelmystFogLane dangerLane, Position& destination)
    {
        if (dangerLane == FelmystFogLane::None)
            return false;

        FelmystFogLane currentLane = FelmystFogLane::None;
        if (TryGetFelmystFogLaneFromPosition(
                bot->GetPositionX(), bot->GetPositionY(), currentLane) &&
            currentLane != dangerLane)
        {
            LogFelmystFogDebug(bot,
                std::string("no shift current=") + GetFelmystFogLaneName(currentLane) +
                " danger=" + GetFelmystFogLaneName(dangerLane));
            return false;
        }

        if (dangerLane == FelmystFogLane::Top)
        {
            float requiredDistance = 0.0f;
            return TryGetFelmystFogShiftDestinationForLane(
                bot, dangerLane, FelmystFogLane::Middle, destination, requiredDistance);
        }

        if (dangerLane == FelmystFogLane::Bottom)
        {
            float requiredDistance = 0.0f;
            return TryGetFelmystFogShiftDestinationForLane(
                bot, dangerLane, FelmystFogLane::Middle, destination, requiredDistance);
        }

        Position topDestination;
        Position bottomDestination;
        float topRequiredDistance = 0.0f;
        float bottomRequiredDistance = 0.0f;
        bool canShiftTop = TryGetFelmystFogShiftDestinationForLane(
            bot, dangerLane, FelmystFogLane::Top, topDestination, topRequiredDistance);
        bool canShiftBottom = TryGetFelmystFogShiftDestinationForLane(
            bot, dangerLane, FelmystFogLane::Bottom, bottomDestination, bottomRequiredDistance);

        if (!canShiftTop && !canShiftBottom)
        {
            LogFelmystFogDebug(bot,
                std::string("no shift options danger=") + GetFelmystFogLaneName(dangerLane));
            return false;
        }

        if (!canShiftTop)
        {
            destination = bottomDestination;
            LogFelmystFogDebug(bot,
                std::string("choose secondary lane bottom only destination=") +
                FormatFelmystFogPoint(destination));
            return true;
        }

        if (!canShiftBottom)
        {
            destination = topDestination;
            LogFelmystFogDebug(bot,
                std::string("choose secondary lane top only destination=") +
                FormatFelmystFogPoint(destination));
            return true;
        }

        destination = topRequiredDistance <= bottomRequiredDistance ? topDestination : bottomDestination;
        std::ostringstream out;
        out << std::fixed << std::setprecision(2)
            << "choose secondary lane topRequired=" << topRequiredDistance
            << " bottomRequired=" << bottomRequiredDistance
            << " chosen=" << (topRequiredDistance <= bottomRequiredDistance ? "top" : "bottom")
            << " destination=" << FormatFelmystFogPoint(destination);
        LogFelmystFogDebug(bot, out.str());
        return true;
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

        float frontAngle = GetFelmystFrontAngle(botAI, bot, felmyst);
        float sideAngle = frontAngle + (assignmentItr->second == 0 ? M_PI_2 : -M_PI_2);
        position = GetFelmystPositionAtAngle(felmyst, bot, sideAngle, FELMYST_RANGED_SIDE_DISTANCE);
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
                !member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE_CHANNEL)))
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
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->GetMapId() != bot->GetMapId() ||
                member->GetInstanceId() != bot->GetInstanceId() ||
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

    Unit* GetNearestFelmystDemonicVaporHazard(Player* bot, float searchRadius)
    {
        Unit* nearestTrail = bot->FindNearestCreature(
            static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR_TRAIL), searchRadius, true);
        Unit* nearestVapor = bot->FindNearestCreature(
            static_cast<uint32>(SunwellNPCs::NPC_DEMONIC_VAPOR), searchRadius, true);

        if (!nearestTrail)
            return nearestVapor;

        if (!nearestVapor)
            return nearestTrail;

        return bot->GetDistance2d(nearestTrail) <= bot->GetDistance2d(nearestVapor) ?
            nearestTrail : nearestVapor;
    }
}
