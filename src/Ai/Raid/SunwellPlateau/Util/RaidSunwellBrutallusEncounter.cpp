/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <algorithm>
#include <cmath>
#include <vector>

#include "Playerbots.h"
#include "RaidSunwellBrutallusEncounter.h"

namespace SunwellHelpers
{

static float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
static float NormalizeSignedAngle(float angle);

const Position BRUTALLUS_MAIN_TANK_POSITION = { 1484.779f, 582.691f, 23.460f };

std::unordered_map<uint32, std::unordered_map<ObjectGuid, uint8>> brutallusRangedAssignments;
std::unordered_map<ObjectGuid, BrutallusRangedBurnState> brutallusRangedBurnStates;

Position GetBrutallusTankPosition(Unit* brutallus, bool isMainTank, float z)
{
    if (isMainTank)
    {
        return { BRUTALLUS_MAIN_TANK_POSITION.GetPositionX(),
                 BRUTALLUS_MAIN_TANK_POSITION.GetPositionY(), z };
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

    const float meleeRadius = std::max(1.0f, bot->GetMeleeRange(brutallus) - 2.0f);
    const float meleeAngleStep = 2.0f * std::asin(meleeSpacing / (2.0f * meleeRadius));
    const uint8 maxSideSlots = static_cast<uint8>(std::floor((arcAngle / 2.0f) / meleeAngleStep));
    const uint8 maxMeleeSlots = 1 + 2 * maxSideSlots;
    if (meleeIndex >= maxMeleeSlots)
        return false;

    const float arcCenterOffset = M_PI + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f;
    const float baseAngle = Position::NormalizeOrientation(
        GetBrutallusMainTankAngle(brutallus) + arcCenterOffset);
    const float arcWidth = maxSideSlots * 2.0f * meleeAngleStep;
    const float angleOffset = GetCenteredArcSlotAngleOffset(meleeIndex, maxMeleeSlots, arcWidth);

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
    const float centerX = brutallus ? brutallus->GetPositionX() :
        BRUTALLUS_MAIN_TANK_POSITION.GetPositionX();
    const float centerY = brutallus ? brutallus->GetPositionY() :
        BRUTALLUS_MAIN_TANK_POSITION.GetPositionY();
    const float x = centerX + std::cos(angle) * radius;
    const float y = centerY + std::sin(angle) * radius;
    return { x, y, z };
}

static float GetCenteredArcSlotAngleOffset(
    uint8 slotIndex, uint8 slotCount, float arcWidth)
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

static float NormalizeSignedAngle(float angle)
{
    angle = Position::NormalizeOrientation(angle);
    if (angle > M_PI)
        angle -= 2.0f * M_PI;

    return angle;
}

float GetBrutallusRangedSlotAngle(
    Unit* brutallus, const BrutallusRangedSlotInfo& slotInfo)
{
    constexpr float rangedSpacing = 6.0f;

    const float frontCenterAngle = Position::NormalizeOrientation(
        GetBrutallusMainTankAngle(brutallus) +
        BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);

    float tankAngle = GetBrutallusMainTankAngle(brutallus);
    if (!slotInfo.isMainTankGroup)
    {
        tankAngle = Position::NormalizeOrientation(
            tankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);
    }

    const float angleTowardCenter = NormalizeSignedAngle(
        frontCenterAngle - tankAngle);
    const float towardCenterSign = angleTowardCenter < 0.0f ? -1.0f : 1.0f;
    const float stepRatio = std::clamp(
        rangedSpacing / (2.0f * BRUTALLUS_NORMAL_RANGED_RADIUS), 0.0f, 1.0f);
    const float angleStep = 2.0f * std::asin(stepRatio);
    const float arcHalfWidth = angleStep * static_cast<float>(
        BRUTALLUS_RANGED_POSITIONS_PER_GROUP - 1) / 2.0f;
    const float outerEdgeAngle = Position::NormalizeOrientation(
        tankAngle - towardCenterSign * arcHalfWidth);

    return Position::NormalizeOrientation(
        outerEdgeAngle + towardCenterSign * angleStep * slotInfo.arcPositionIndex);
}

bool TryGetBrutallusRangedStepPosition(
    Unit* brutallus, uint8 rangedIndex, bool useMirrorAngle,
    float radius, float z, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    const BrutallusRangedSlotInfo slotInfo = {
        rangedIndex % 2 == 0,
        static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP)
    };

    float angle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
    if (useMirrorAngle)
    {
        angle = Position::NormalizeOrientation(
            angle + (slotInfo.isMainTankGroup ? M_PI_2 : -M_PI_2));
    }

    position = GetBrutallusPositionAtAngle(brutallus, angle, radius, z);
    return true;
}

bool TryGetBrutallusRangedArcPosition(
    Unit* brutallus, uint8 rangedIndex, float radius, bool moveTowardMirror,
    float currentX, float currentY, float z, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    const BrutallusRangedSlotInfo slotInfo = {
        rangedIndex % 2 == 0,
        static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP)
    };

    const float normalAngle = GetBrutallusRangedSlotAngle(brutallus, slotInfo);
    float targetAngle = normalAngle;
    if (moveTowardMirror)
    {
        targetAngle = Position::NormalizeOrientation(
            normalAngle + (slotInfo.isMainTankGroup ? M_PI_2 : -M_PI_2));
    }

    const float currentAngle = Position::NormalizeOrientation(
        std::atan2(currentY - brutallus->GetPositionY(), currentX - brutallus->GetPositionX()));
    const float remainingAngle = NormalizeSignedAngle(targetAngle - currentAngle);

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

}
