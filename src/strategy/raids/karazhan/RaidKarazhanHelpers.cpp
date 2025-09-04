#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AiObjectContext.h"
#include "Position.h"
#include <algorithm>

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

/* std::vector<Player*> RaidKarazhanHelpers::GetRedBlockers()
{
    std::vector<Player*> redBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            // Tank, no nether exhaustion red (38637), and <= 6 stacks of nether portal red (30421)
            if (botAI->IsTank(member) && !member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
            {
                Aura* redBuff = member->GetAura(SPELL_NETHER_PORTAL_RED);
                if (!redBuff || redBuff->GetStackAmount() <= 6)
                {
                    redBlockers.push_back(member);
                }
            }
        }
    }
    return redBlockers;
}*/

/* std::vector<Player*> RaidKarazhanHelpers::GetBlueBlockers()
{
    std::vector<Player*> blueBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            // Is DPS, not warrior, not rogue, no nether exhaustion blue (38639), <= 30 stacks of nether portal blue (30423)
            bool isDps = botAI->IsDps(member);
            bool isWarrior = member->getClass() == CLASS_WARRIOR;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            if (isDps && !isWarrior && !isRogue && !member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE))
            {
                Aura* blueBuff = member->GetAura(SPELL_NETHER_PORTAL_BLUE);
                if (!blueBuff || blueBuff->GetStackAmount() <= 30)
                {
                    blueBlockers.push_back(member);
                }
            }
        }
    }
    return blueBlockers;
} */

/*std::vector<Player*> RaidKarazhanHelpers::GetGreenBlockers()
{
    std::vector<Player*> greenBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            // (1) Rogue or non-tank Warrior, no aura 38638
            if ((member->getClass() == CLASS_ROGUE || (member->getClass() == CLASS_WARRIOR && botAI->IsDps(member))) &&
                !member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN))
            {
                greenBlockers.push_back(member);
                continue;
            }

            // (2) Healer, no aura 38638, and less than 30 stacks of aura 30422
            if (botAI->IsHeal(member) && !member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN))
            {
                Aura* greenBuff = member->GetAura(SPELL_NETHER_PORTAL_GREEN);
                if (!greenBuff || greenBuff->GetStackAmount() < 30)
                {
                    greenBlockers.push_back(member);
                }
            }
        }
    }
    return greenBlockers;
} */

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

/* std::tuple<Player*, Player*, Player*> RaidKarazhanHelpers::GetCurrentBeamBlockers()
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
    std::vector<Player*> greenBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;
            PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
            if (!memberAI)
                continue;
            bool isHealer = memberAI->IsHeal(member);
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool isDpsWarrior = member->getClass() == CLASS_WARRIOR && !memberAI->IsTank(member);
            if (!(isHealer || isRogue || isDpsWarrior))
                continue;
            if (member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN))
                continue;
            greenBlockers.push_back(member);
        }
    }
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
    // Get position along the beam at minDistance, then offset perpendicular by (maxDistance-minDistance)/2
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

    // Perpendicular direction
    float perpDx = -dy;
    float perpDy = dx;
    float offset = (maxDistance - minDistance) / 2.0f;

    float targetX = bx + dx * minDistance + perpDx * offset;
    float targetY = by + dy * minDistance + perpDy * offset;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
} */
