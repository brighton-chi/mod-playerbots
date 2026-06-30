/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "SWPEncounter_Brut.h"
#include "Playerbots.h"

namespace SunwellHelpers
{

const Position BRUTALLUS_MAIN_TANK_POSITION = { 1483.528f, 595.346f, 23.552f };

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedAssignments;

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>>
    brutallusRangedBurnPadAssignments;

std::unordered_map<ObjectGuid, BrutallusRangedBurnState>
    brutallusRangedBurnStates;

float GetBrutallusMainTankAngle(Unit* brutallus)
{
    if (!brutallus)
        return 0.0f;

    return Position::NormalizeOrientation(
        std::atan2(BRUTALLUS_MAIN_TANK_POSITION.GetPositionY() - brutallus->GetPositionY(),
                   BRUTALLUS_MAIN_TANK_POSITION.GetPositionX() - brutallus->GetPositionX()));
}

Position GetBrutallusPositionAtAngle(Unit* brutallus, float angle, float radius, float z)
{
    if (!brutallus)
        return { 0.0f, 0.0f, z };

    const float x = brutallus->GetPositionX() + std::cos(angle) * radius;
    const float y = brutallus->GetPositionY() + std::sin(angle) * radius;
    return { x, y, z };
}

namespace
{

float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
{
    if (slotCount <= 1)
        return 0.0f;

    const float angleStep = arcWidth / static_cast<float>(slotCount - 1);
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

    const float halfStep = angleStep / 2.0f;
    const uint8 pairIndex = slotIndex / 2;
    float angleOffset = halfStep + angleStep * pairIndex;
    if (slotIndex % 2 == 1)
        angleOffset = -angleOffset;

    return angleOffset;
}

float GetBrutallusTankAngle(Unit* brutallus, Player* tank, float fallbackAngle)
{
    if (!brutallus || !tank)
        return Position::NormalizeOrientation(fallbackAngle);

    return Position::NormalizeOrientation(
        std::atan2(tank->GetPositionY() - brutallus->GetPositionY(),
                   tank->GetPositionX() - brutallus->GetPositionX()));
}

float GetBrutallusMidpointAngle(Unit* brutallus, Player* mainTank, Player* assistTank)
{
    const float mainTankAngle =
        GetBrutallusTankAngle(brutallus, mainTank, GetBrutallusMainTankAngle(brutallus));

    if (!brutallus || !mainTank || !assistTank)
    {
        return Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);
    }

    const float assistTankAngle = GetBrutallusTankAngle(
        brutallus, assistTank,
        Position::NormalizeOrientation(mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET));

    const float midpointX =
        (mainTank->GetPositionX() + assistTank->GetPositionX()) / 2.0f;
    const float midpointY =
        (mainTank->GetPositionY() + assistTank->GetPositionY()) / 2.0f;

    if (brutallus->GetExactDist2d(midpointX, midpointY) < 0.1f)
    {
        float assistAngleDelta =
            Position::NormalizeOrientation(assistTankAngle - mainTankAngle);
        if (assistAngleDelta > static_cast<float>(M_PI))
            assistAngleDelta -= 2.0f * static_cast<float>(M_PI);

        return Position::NormalizeOrientation(mainTankAngle + assistAngleDelta / 2.0f);
    }

    return Position::NormalizeOrientation(
        std::atan2(midpointY - brutallus->GetPositionY(),
                   midpointX - brutallus->GetPositionX()));
}

bool IsBrutallusMainTankGroup(uint8 rangedIndex)
{
    return rangedIndex % 2 == 0;
}

uint8 GetBrutallusArcPositionIndex(uint8 rangedIndex)
{
    return static_cast<uint8>(
        (rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP);
}

bool IsBrutallusBurnPadActive(ObjectGuid ownerGuid)
{
    auto const burnStateItr = brutallusRangedBurnStates.find(ownerGuid);
    return burnStateItr != brutallusRangedBurnStates.end() &&
           burnStateItr->second != BrutallusRangedBurnState::None;
}

bool TryGetBrutallusBurnPadIndex(Player* bot, uint8 rangedIndex, uint8& padIndex)
{
    auto& assignments = brutallusRangedBurnPadAssignments[bot->GetInstanceId()];
    for (auto itr = assignments.begin(); itr != assignments.end();)
    {
        if (itr->first != bot->GetGUID() && !IsBrutallusBurnPadActive(itr->first))
        {
            itr = assignments.erase(itr);
            continue;
        }

        ++itr;
    }

    auto const existingItr = assignments.find(bot->GetGUID());
    if (existingItr != assignments.end() &&
        existingItr->second < BRUTALLUS_TOTAL_BURN_PADS)
    {
        padIndex = existingItr->second;
        return true;
    }

    std::array<bool, BRUTALLUS_TOTAL_BURN_PADS> usedPads = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_BURN_PADS)
            usedPads[assignment.second] = true;
    }

    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupPriority = { 0, 1, 2, 3 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupOverflow = { 4, 5, 6, 7 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupPriority = { 7, 6, 5, 4 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupOverflow = { 3, 2, 1, 0 };

    auto const assignFromOrder = [&](std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP> const& order)
    {
        for (uint8 candidate : order)
        {
            if (usedPads[candidate])
                continue;

            assignments[bot->GetGUID()] = candidate;
            padIndex = candidate;
            return true;
        }

        return false;
    };

    if (IsBrutallusMainTankGroup(rangedIndex))
        return assignFromOrder(mainGroupPriority) || assignFromOrder(mainGroupOverflow);

    return assignFromOrder(assistGroupPriority) || assignFromOrder(assistGroupOverflow);
}

}

bool TryGetBrutallusMeleePosition(
    Unit* brutallus, Player* mainTank, Player* assistTank,
    uint8 meleeIndex, float z, Position& position)
{
    if (!brutallus)
        return false;

    struct BrutallusMeleeRingLayout
    {
        float radius;
        uint8 slotCount;
    };

    constexpr std::array<BrutallusMeleeRingLayout, 4> meleeRingLayouts = {{
        { BRUTALLUS_INNERMOST_MELEE_RADIUS, BRUTALLUS_INNERMOST_MELEE_POSITIONS },
        { BRUTALLUS_INNER_MELEE_RADIUS, BRUTALLUS_INNER_MELEE_POSITIONS },
        { BRUTALLUS_OUTER_MELEE_RADIUS, BRUTALLUS_OUTER_MELEE_POSITIONS },
        { BRUTALLUS_OUTERMOST_MELEE_RADIUS, BRUTALLUS_OUTERMOST_MELEE_POSITIONS }
    }};

    uint8 totalMeleeSlots = 0;
    for (auto const& meleeRingLayout : meleeRingLayouts)
        totalMeleeSlots += meleeRingLayout.slotCount;

    if (meleeIndex >= totalMeleeSlots)
        return false;

    float meleeRadius = 0.0f;
    uint8 localMeleeIndex = meleeIndex;
    uint8 maxMeleeSlots = 0;
    for (auto const& meleeRingLayout : meleeRingLayouts)
    {
        if (localMeleeIndex < meleeRingLayout.slotCount)
        {
            meleeRadius = meleeRingLayout.radius;
            maxMeleeSlots = meleeRingLayout.slotCount;
            break;
        }

        localMeleeIndex -= meleeRingLayout.slotCount;
    }

    if (!maxMeleeSlots)
        return false;

    const float midpointAngle = GetBrutallusMidpointAngle(brutallus, mainTank, assistTank);
    const float baseAngle = Position::NormalizeOrientation(midpointAngle + M_PI);
    const float angleOffset =
        GetCenteredArcSlotAngleOffset(
            localMeleeIndex, maxMeleeSlots, BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH);

    const float angle = Position::NormalizeOrientation(baseAngle + angleOffset);
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

        auto const instanceItr = brutallusRangedAssignments.find(bot->GetInstanceId());
        if (instanceItr == brutallusRangedAssignments.end())
            return false;

        auto const assignmentItr = instanceItr->second.find(bot->GetGUID());
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

        const bool isMelee = botAI->IsMelee(member);
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

float GetBrutallusRangedSlotAngle(
    Unit* brutallus, Player* mainTank, Player* assistTank,
    const BrutallusRangedSlotInfo& slotInfo)
{
    const float mainTankAngle =
        GetBrutallusTankAngle(brutallus, mainTank, GetBrutallusMainTankAngle(brutallus));
    const float assistTankAngle = GetBrutallusTankAngle(
        brutallus, assistTank,
        Position::NormalizeOrientation(mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET));
    const float tankAngle = slotInfo.isMainTankGroup ? mainTankAngle : assistTankAngle;
    const float angleOffset = GetCenteredArcSlotAngleOffset(
        slotInfo.arcPositionIndex, BRUTALLUS_RANGED_POSITIONS_PER_GROUP,
        BRUTALLUS_RANGED_GROUP_ARC_WIDTH);

    return Position::NormalizeOrientation(tankAngle + angleOffset);
}

bool TryGetBrutallusRangedPosition(
    Unit* brutallus, Player* mainTank, Player* assistTank,
    uint8 rangedIndex, float radius, float z, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    const BrutallusRangedSlotInfo slotInfo = {
        IsBrutallusMainTankGroup(rangedIndex),
        GetBrutallusArcPositionIndex(rangedIndex)
    };

    const float angle = GetBrutallusRangedSlotAngle(brutallus, mainTank, assistTank, slotInfo);

    position = GetBrutallusPositionAtAngle(brutallus, angle, radius, z);
    return true;
}

bool TryGetBrutallusBurnPadPosition(
    Player* bot, Unit* brutallus, Player* mainTank,
    uint8 rangedIndex, float radius, float z, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    uint8 padIndex = 0;
    if (!TryGetBrutallusBurnPadIndex(bot, rangedIndex, padIndex))
        return false;

    static constexpr float degreeToRadian = M_PI / 180.0f;
    static constexpr std::array<float, BRUTALLUS_TOTAL_BURN_PADS>
        burnPadAngleOffsets = {
            70.0f * degreeToRadian,
            83.3f * degreeToRadian,
            96.7f * degreeToRadian,
            110.0f * degreeToRadian,
            130.0f * degreeToRadian,
            143.3f * degreeToRadian,
            156.7f * degreeToRadian,
            170.0f * degreeToRadian
        };

    const float mainTankAngle =
        GetBrutallusTankAngle(brutallus, mainTank, GetBrutallusMainTankAngle(brutallus));
    const float angle =
        Position::NormalizeOrientation(mainTankAngle + burnPadAngleOffsets[padIndex]);

    position = GetBrutallusPositionAtAngle(brutallus, angle, radius, z);
    return true;
}

bool TryGetBrutallusLaneTraversalPosition(
    Unit* brutallus, float targetX, float targetY, float radius,
    float currentX, float currentY, float z, Position& position)
{
    if (!brutallus)
        return false;

    const float targetAngle = Position::NormalizeOrientation(
        std::atan2(targetY - brutallus->GetPositionY(),
                   targetX - brutallus->GetPositionX()));

    const float currentAngle = Position::NormalizeOrientation(
        std::atan2(currentY - brutallus->GetPositionY(), currentX - brutallus->GetPositionX()));

    float remainingAngle = Position::NormalizeOrientation(targetAngle - currentAngle);
    if (remainingAngle > static_cast<float>(M_PI))
        remainingAngle -= 2.0f * static_cast<float>(M_PI);

    constexpr float stepDistance = 3.0f;
    const float stepRatio = stepDistance / (2.0f * radius);
    const float clampedStepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
    const float stepAngle = 2.0f * std::asin(clampedStepRatio);
    float nextAngle = targetAngle;

    if (std::fabs(remainingAngle) > stepAngle)
    {
        nextAngle = Position::NormalizeOrientation(
            currentAngle + std::copysign(stepAngle, remainingAngle));
    }

    position = GetBrutallusPositionAtAngle(brutallus, nextAngle, radius, z);
    return true;
}

bool ReleaseBrutallusBurnPad(Player* bot)
{
    auto instanceItr = brutallusRangedBurnPadAssignments.find(bot->GetInstanceId());
    if (instanceItr == brutallusRangedBurnPadAssignments.end())
        return false;

    const bool erased = instanceItr->second.erase(bot->GetGUID()) > 0;
    if (instanceItr->second.empty())
        brutallusRangedBurnPadAssignments.erase(instanceItr);

    return erased;
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

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_RANGED_POSITIONS; ++slotIndex)
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
        if (!member || member->GetMapId() != SUNWELL_MAP_ID || !botAI->IsRanged(member))
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

}
