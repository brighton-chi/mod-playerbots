#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AiObjectContext.h"
#include "Position.h"
#include <algorithm>
#include <map>

#include "Log.h"

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

            if (!member || !member->IsAlive() || member == bot)
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

std::vector<Player*> RaidKarazhanHelpers::GetBlueBlockers()
{
    std::vector<Player*> blueBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            // Is DPS, not warrior, not rogue, no Nether Exhaustion Blue and <= 30 stacks of Blue Beam debuff
            bool isDps = botAI->IsDps(member);
            bool isWarrior = member->getClass() == CLASS_WARRIOR;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool eligible = isDps && !isWarrior && !isRogue && !member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE);
            Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
            
            if (eligible && (!blueBuff || blueBuff->GetStackAmount() <= 30))
            {
                blueBlockers.push_back(member);
            }
        }
    }

    return blueBlockers;
}

std::vector<Player*> RaidKarazhanHelpers::GetGreenBlockers()
{
    std::vector<Player*> greenBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
            // (1) Rogue or non-tank Warrior, no Nether Exhaustion Green
            // (2) Healer, no Nether Exhaustion Green and less than 30 stacks of Green Beam debuff
            bool eligibleGreen = 
                ((member->getClass() == CLASS_ROGUE || (member->getClass() == CLASS_WARRIOR && botAI->IsDps(member)))
                || (botAI->IsHeal(member) && (!greenBuff || greenBuff->GetStackAmount() < 30))) 
                && !member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN);

            if (eligibleGreen)
            {
                greenBlockers.push_back(member);
            }
        }
    }

    return greenBlockers;
}

Position RaidKarazhanHelpers::GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss)
{
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float bz = boss->GetPositionZ();
    float px = portal->GetPositionX();
    float py = portal->GetPositionY();

    float dx = px - bx;
    float dy = py - by;
    float length = sqrt(dx*dx + dy*dy);
    if (length == 0.0f)
        return Position(bx, by, bz);

    dx /= length;
    dy /= length;

    float targetX = bx + dx * distanceFromBoss;
    float targetY = by + dy * distanceFromBoss;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
}

std::tuple<Player*, Player*, Player*> RaidKarazhanHelpers::GetCurrentBeamBlockers()
{
    Player* redBlocker = nullptr;
    Player* greenBlocker = nullptr;
    Player* blueBlocker = nullptr;

    // Red: first eligible tank
    std::vector<Player*> redBlockers;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;
            PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
            if (!memberAI || !memberAI->IsTank(member))
                continue;
            if (member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
                continue;
            redBlockers.push_back(member);
        }
    }

    if (!redBlockers.empty())
        redBlocker = redBlockers.front();

    // Green: first eligible healer, rogue, or dps warrior
    std::vector<Player*> greenBlockers = GetGreenBlockers();
    if (!greenBlockers.empty())
        greenBlocker = greenBlockers.front();

    // Blue: first eligible dps (not warrior/rogue)
    std::vector<Player*> blueBlockers = GetBlueBlockers();
    if (!blueBlockers.empty())
        blueBlocker = blueBlockers.front();

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

Position RaidKarazhanHelpers::GetAvoidBeamPosition(Unit* boss, Unit* portal, float minDistance, float maxDistance)
{
    // Ensure bot is at least minDistance yards perpendicular from the beam
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float bz = boss->GetPositionZ();
    float px = portal->GetPositionX();
    float py = portal->GetPositionY();
    float botx = bot->GetPositionX();
    float boty = bot->GetPositionY();

    float dx = px - bx;
    float dy = py - by;
    float length = sqrt(dx*dx + dy*dy);

    if (length == 0.0f)
        return Position(botx, boty, bz);

    dx /= length;
    dy /= length;

    // Vector from boss to bot
    float botdx = botx - bx;
    float botdy = boty - by;
    // Perpendicular distance from bot to beam
    float perpDist = fabs(-dy * botdx + dx * botdy);

    // Distance from boss
    float bossDist = sqrt(botdx * botdx + botdy * botdy);

    // If already far enough from beam and boss, stay
    if (perpDist >= minDistance && bossDist >= 18.0f)
        return Position(botx, boty, bz);

    // Move bot to minDistance away from beam
    float perpDx = -dy;
    float perpDy = dx;
    // Determine which side to move to (away from beam)
    float sign = ((-dy * botdx + dx * botdy) >= 0) ? 1.0f : -1.0f;
    float targetX = botx + perpDx * (minDistance - perpDist) * sign;
    float targetY = boty + perpDy * (minDistance - perpDist) * sign;
    float targetZ = bz;

    // Ensure target position is at least 18 yards from boss
    float tdx = targetX - bx;
    float tdy = targetY - by;
    float targetBossDist = sqrt(tdx * tdx + tdy * tdy);

    if (targetBossDist < 18.0f)
    {
        // Move further away from boss along the direction from boss to target
        float dirX = tdx / (targetBossDist > 0.0f ? targetBossDist : 1.0f);
        float dirY = tdy / (targetBossDist > 0.0f ? targetBossDist : 1.0f);
        targetX = bx + dirX * 18.0f;
        targetY = by + dirY * 18.0f;
        // After boss distance adjustment, re-check perpendicular distance from beam
        float newBotdx = targetX - bx;
        float newBotdy = targetY - by;
        float newPerpDist = fabs(-dy * newBotdx + dx * newBotdy);

        if (newPerpDist < minDistance)
        {
            // Move further away from beam along perpendicular direction
            float adjust = minDistance - newPerpDist;
            targetX += perpDx * adjust * sign;
            targetY += perpDy * adjust * sign;
        }
    }

    return Position(targetX, targetY, targetZ);
}

std::vector<Unit*> RaidKarazhanHelpers::GetSpawnedInfernals() const
{
    std::vector<Unit*> infernals;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
            infernals.push_back(unit);
    }

    return infernals;
}
