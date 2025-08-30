#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"

void RaidKarazhanHelpers::MarkTargetWithSkull(Unit* target)
{
    if (!target)
        return;

    if (Group* group = bot->GetGroup())
    {
        constexpr uint8_t skullIconId = 7;
        ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);

        if (skullGuid != target->GetGUID())
            group->SetTargetIcon(skullIconId, bot->GetGUID(), target->GetGUID());
    }
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnit(const std::vector<Unit*>& units)
{
    for (Unit* unit : units)
        if (unit && unit->IsAlive())
            return unit;

    return nullptr;
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnitByEntry(uint32 entry)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

Unit* RaidKarazhanHelpers::GetNearestPlayerInRadius(float radius)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member == bot || member->IsGameMaster())
                continue;

            if (bot->GetExactDist2d(member) < radius)
                return member;
        }
    }

    return nullptr;
}

bool RaidKarazhanHelpers::IsFlameWreathActive()
{
    if (bot->HasAura(SPELL_FLAME_WREATH))
        return true;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member == bot)
                continue;

            if (member->HasAura(SPELL_FLAME_WREATH))
                return true;
        }
    }

    return false;
}

/*bool RaidKarazhanHelpers::IsBetween(Unit* first, Unit* second)
{
    float bx = first->GetPositionX();
    float by = first->GetPositionY();
    float px = second->GetPositionX();
    float py = second->GetPositionY();
    float botx = bot->GetPositionX();
    float boty = bot->GetPositionY();

    // Vector from boss to portal
    float dx = px - bx;
    float dy = py - by;

    // Vector from boss to bot
    float botdx = botx - bx;
    float botdy = boty - by;

    // Project bot position onto the line segment [boss, portal]
    float lenSq = dx * dx + dy * dy;
    float t = (lenSq > 0) ? ((botdx * dx + botdy * dy) / lenSq) : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);

    // Closest point on the line segment
    float closestX = bx + t * dx;
    float closestY = by + t * dy;

    // Distance from bot to the closest point on the line
    float dist = std::sqrt((botx - closestX) * (botx - closestX) + (boty - closestY) * (boty - closestY));

    return dist < 1.0f;
}

Position RaidKarazhanHelpers::GetPositionBetween(Unit* first, Unit* second)
{
    float bx = first->GetPositionX();
    float by = first->GetPositionY();
    float px = second->GetPositionX();
    float py = second->GetPositionY();
    float botx = bot->GetPositionX();
    float boty = bot->GetPositionY();

    float dx = px - bx;
    float dy = py - by;

    float botdx = botx - bx;
    float botdy = boty - by;

    float lenSq = dx * dx + dy * dy;
    float t = (lenSq > 0) ? ((botdx * dx + botdy * dy) / lenSq) : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);

    float closestX = bx + t * dx;
    float closestY = by + t * dy;
    float closestZ = bot->GetPositionZ();

    return Position(closestX, closestY, closestZ);
}

Position RaidKarazhanHelpers::GetPositionToRotateAroundTarget(Unit* target)
{
    float bx = target->GetPositionX();
    float by = target->GetPositionY();
    float botx = bot->GetPositionX();
    float boty = bot->GetPositionY();
    float radius = std::sqrt((botx - bx) * (botx - bx) + (boty - by) * (boty - by));
    float angle = std::atan2(boty - by, botx - bx);

    constexpr float angleStep = M_PI / 18.0f;
    float newAngle = angle + angleStep;

    float newX = bx + radius * std::cos(newAngle);
    float newY = by + radius * std::sin(newAngle);
    float newZ = bot->GetPositionZ();

    return Position(newX, newY, newZ);
}

Position RaidKarazhanHelpers::GetPositionToMoveCloserToTarget(Unit* target)
{
    constexpr float stopDistance = 10.0f;
    constexpr float stepDistance = 3.0f;

    float portalX = target->GetPositionX();
    float portalY = target->GetPositionY();
    float portalZ = target->GetPositionZ();

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();

    float dx = portalX - botX;
    float dy = portalY - botY;
    float dz = portalZ - botZ;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (distance > stopDistance)
    {
        float moveDist = std::min(stepDistance, distance - stopDistance);
        float norm = std::sqrt(dx * dx + dy * dy + dz * dz);
        float stepX = botX + (dx / norm) * moveDist;
        float stepY = botY + (dy / norm) * moveDist;
        float stepZ = botZ + (dz / norm) * moveDist;

        return Position(stepX, stepY, stepZ);
    }

    return Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
}

Position RaidKarazhanHelpers::GetFlankPosition(Unit* target, float distance, bool rightFlank)
{
    float angle = target->GetOrientation();
    // 90 degrees to the left or right
    angle += rightFlank ? M_PI_2 : -M_PI_2;

    float x = target->GetPositionX() + std::cos(angle) * distance;
    float y = target->GetPositionY() + std::sin(angle) * distance;
    float z = target->GetPositionZ();

    return Position(x, y, z);
}

Unit* RaidKarazhanHelpers::GetNearestUnitByEntryWithinRadius(uint32 entry, float radius)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && !unit->IsAlive() && unit->GetEntry() == entry && bot->GetExactDist2d(unit) < radius)
            return unit;
    }

    return nullptr;
}*/
