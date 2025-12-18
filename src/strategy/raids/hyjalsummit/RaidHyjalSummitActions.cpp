#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "Playerbots.h"

using namespace HyjalSummitHelpers;

// Rage Winterchill

bool RageWinterchillMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", winterchill))
        return botAI->CastSpell("steady shot", winterchill);

    return false;
}

bool RageWinterchillMainTankPositionBossAction::Execute(Event event)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (bot->GetVictim() != winterchill)
        return Attack(winterchill);

    if (winterchill->GetVictim() == bot)
    {
        const Position& position = RAGE_WINTERCHILL_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Right now, this is just a one-time deal--no returning to positions
bool RageWinterchillSpreadRangedInCircleAction::Execute(Event event)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (winterchill->GetHealth() == winterchill->GetMaxHealth())
    {
        winterchillRangedPositions.clear();
        hasReachedWinterchillPosition.clear();
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = winterchillRangedPositions.find(guid);
    if (it == winterchillRangedPositions.end())
    {
        float targetX = 0.0f, targetY = 0.0f;
        float radius = 0.0f;
        float angle = 0.0f;

        if (botAI->IsHeal(bot))
        {
            size_t count = healers.size();
            if (count == 0)
                return false;

            auto findIt = std::find(healers.begin(), healers.end(), bot);
            size_t botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 30.0f;
            angle = (count == 1) ? 0.0f : (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));
        }
        else
        {
            size_t count = rangedDps.size();
            if (count == 0)
                return false;

            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            size_t botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 25.0f;
            angle = (count == 1) ? 0.0f : (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));
        }

        targetX = RAGE_WINTERCHILL_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
        targetY = RAGE_WINTERCHILL_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

        winterchillRangedPositions.try_emplace(guid, Position(
            targetX, targetY, RAGE_WINTERCHILL_TANK_POSITION.GetPositionZ()));
        hasReachedWinterchillPosition[guid] = false;
        it = winterchillRangedPositions.find(guid);
    }

    if (it == winterchillRangedPositions.end())
        return false;

    if (!hasReachedWinterchillPosition[guid])
    {
        const Position& target = it->second;
        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                          target.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
            hasReachedWinterchillPosition[guid] = true;
    }

    return false;
}

// Anetheron

bool AnetheronMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", anetheron))
        return botAI->CastSpell("steady shot", anetheron);

    return false;
}

bool AnetheronMainTankPositionBossAction::Execute(Event event)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (bot->GetVictim() != anetheron)
        return Attack(anetheron);

    if (anetheron->GetVictim() == bot)
    {
        const Position& position = ANETHERON_MAIN_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AnetheronSpreadRangedInArcAction::Execute(Event event)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (anetheron->GetHealth() == anetheron->GetMaxHealth())
        anetheronRangedPositions.clear();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = anetheronRangedPositions.find(guid);
    if (it == anetheronRangedPositions.end())
    {
        float targetX = 0.0f, targetY = 0.0f;
        float radius = 0.0f;
        float angle = 0.0f;

        // Arc settings
        const float arcSpan = 3.0f * M_PI / 2.0f; // 270 degrees in radians
        const float arcCenter = 0.165f;           // chosen orientation
        const float arcStart = arcCenter - arcSpan / 2.0f; // start of arc

        if (botAI->IsHeal(bot))
        {
            size_t count = healers.size();
            if (count == 0)
                return false;

            auto findIt = std::find(healers.begin(), healers.end(), bot);
            size_t botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 30.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }
        else
        {
            size_t count = rangedDps.size();
            if (count == 0)
                return false;

            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            size_t botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 25.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }

        targetX = ANETHERON_MAIN_TANK_POSITION.GetPositionX() + radius * std::sin(angle); // sin for X (east-west)
        targetY = ANETHERON_MAIN_TANK_POSITION.GetPositionY() + radius * std::cos(angle); // cos for Y (north-south)

        anetheronRangedPositions.try_emplace(guid, Position(
            targetX, targetY, ANETHERON_MAIN_TANK_POSITION.GetPositionZ()));
        it = anetheronRangedPositions.find(guid);
    }

    if (it == anetheronRangedPositions.end())
        return false;

    const Position& target = it->second;
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool AnetheronBringInfernalToInfernalTankAction::Execute(Event event)
{
    const Position& tankPosition = ANETHERON_INFERNAL_TANK_POSITION;
    if (bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY()) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(HYJAL_SUMMIT_MAP_ID, tankPosition.GetPositionX(), tankPosition.GetPositionY(),
                      tankPosition.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool AnetheronFirstAssistTankPickUpInfernalsAction::Execute(Event event)
{
    const Position& tankPosition = ANETHERON_INFERNAL_TANK_POSITION;
    if (bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY()) > 5.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, tankPosition.GetPositionX(), tankPosition.GetPositionY(),
                      tankPosition.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Find all infernals within 15 yards of the tank position
    std::vector<Unit*> infernals;
    for (auto const& guid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        if (unit->GetEntry() != NPC_TOWERING_INFERNAL)
            continue;
        if (unit->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY()) > 15.0f)
            continue;
        infernals.push_back(unit);
    }

    for (Unit* infernal : infernals)
    {
        if (infernal->GetVictim() != bot)
        {
            if (bot->GetVictim() != infernal)
            {
                return Attack(infernal);
            }
            else
            {
                const char* taunts[] = { "taunt", "growl", "hand of reckoning", "dark command" };
                for (const char* spellName : taunts)
                {
                    if (botAI->CanCastSpell(spellName, infernal))
                        return botAI->CastSpell(spellName, infernal);
                }
            }
        }
    }

    return false;
}

bool AnetheronFocusAllDpsOnBossAction::Execute(Event event)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (IsInstanceTimerManager(botAI, bot))
        MarkTargetWithSquare(bot, anetheron);

    SetRtiTarget(botAI, "square", anetheron);

    if (bot->GetTarget() != anetheron->GetGUID())
        return Attack(anetheron);

    return false;
}

// Kaz'rogal

bool KazrogalMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", kazrogal))
        return botAI->CastSpell("steady shot", kazrogal);

    return false;
}

bool KazrogalMainTankPositionBossAction::Execute(Event event)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->GetVictim() != kazrogal)
        return Attack(kazrogal);

    if (kazrogal->GetVictim() == bot)
    {
        const Position& position = KAZROGAL_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event event)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    float bossX = kazrogal->GetPositionX();
    float bossY = kazrogal->GetPositionY();
    float bossZ = kazrogal->GetPositionZ();
    float bossO = kazrogal->GetOrientation();

    float frontDist = 3.0f;
    float targetX = bossX + std::cos(bossO) * frontDist;
    float targetY = bossY + std::sin(bossO) * frontDist;

    float dist = bot->GetExactDist2d(targetX, targetY);
    if (dist > 2.0f)
    {
        float dX = targetX - bot->GetPositionX();
        float dY = targetY - bot->GetPositionY();
        float moveDist = std::min(10.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bossZ, false, false, false,
                      false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }
    else if (bot->GetVictim() != kazrogal)
    {
        return Attack(kazrogal);
    }

    return false;
}

bool KazrogalSpreadRangedInArcAction::Execute(Event event)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (kazrogal->GetHealth() == kazrogal->GetMaxHealth())
        kazrogalRangedPositions.clear();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = kazrogalRangedPositions.find(guid);
    if (it == kazrogalRangedPositions.end())
    {
        float targetX = 0.0f, targetY = 0.0f;
        float radius = 0.0f;
        float angle = 0.0f;

        // Arc settings
        const float arcSpan = 2.0f * M_PI / 3.0f; // 120 degrees in radians
        const float arcCenter = 1.342f;           // chosen orientation
        const float arcStart = arcCenter - arcSpan / 2.0f; // start of arc

        if (botAI->IsHeal(bot))
        {
            size_t count = healers.size();
            if (count == 0)
                return false;

            auto findIt = std::find(healers.begin(), healers.end(), bot);
            size_t botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 30.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }
        else
        {
            size_t count = rangedDps.size();
            if (count == 0)
                return false;

            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            size_t botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 25.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }

        targetX = KAZROGAL_TANK_POSITION.GetPositionX() + radius * std::sin(angle); // sin for X (east-west)
        targetY = KAZROGAL_TANK_POSITION.GetPositionY() + radius * std::cos(angle); // cos for Y (north-south)

        kazrogalRangedPositions.try_emplace(guid, Position(
            targetX, targetY, KAZROGAL_TANK_POSITION.GetPositionZ()));
        it = kazrogalRangedPositions.find(guid);
    }

    if (it == kazrogalRangedPositions.end())
        return false;

    const Position& target = it->second;
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool KazrogalLowManaBotMoveFromGroupAction::Execute(Event event)
{
    const float safeDistance = 15.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveFromGroup(safeDistance + 1.0f);
    }

    return false;
}

bool KazrogalRemoveMarkOfKazrogalAction::Execute(Event event)
{
    if (botAI->CanCastSpell("ice block", bot))
    {
        return botAI->CastSpell("ice block", bot);
    }
    else if (botAI->CanCastSpell("divine shield", bot))
    {
        return botAI->CastSpell("divine shield", bot);
    }

    return false;
}

// Azgalor

bool AzgalorMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", azgalor))
        return botAI->CastSpell("steady shot", azgalor);

    return false;
}

bool AzgalorMainTankPositionBossAction::Execute(Event event)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (bot->GetVictim() != azgalor)
        return Attack(azgalor);

    if (azgalor->GetVictim() == bot)
    {
        const Position& position = AZGALOR_MAIN_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool AzgalorSpreadRangedInArcAction::Execute(Event event)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (azgalor->GetHealth() == azgalor->GetMaxHealth())
        azgalorRangedPositions.clear();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = azgalorRangedPositions.find(guid);
    if (it == azgalorRangedPositions.end())
    {
        float targetX = 0.0f, targetY = 0.0f;
        float radius = 0.0f;
        float angle = 0.0f;

        // Arc settings
        const float arcSpan = 2.0f * M_PI / 3.0f; // 120 degrees in radians
        const float arcCenter = 6.034f;           // chosen orientation
        const float arcStart = arcCenter - arcSpan / 2.0f; // start of arc

        if (botAI->IsHeal(bot))
        {
            size_t count = healers.size();
            if (count == 0)
                return false;

            auto findIt = std::find(healers.begin(), healers.end(), bot);
            size_t botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 30.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }
        else
        {
            size_t count = rangedDps.size();
            if (count == 0)
                return false;

            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            size_t botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 25.0f;
            // Spread evenly along the arc
            angle = (count == 1) ? 0.0f : (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        }

        targetX = AZGALOR_MAIN_TANK_POSITION.GetPositionX() + radius * std::sin(angle); // sin for X (east-west)
        targetY = AZGALOR_MAIN_TANK_POSITION.GetPositionY() + radius * std::cos(angle); // cos for Y (north-south)

        azgalorRangedPositions.try_emplace(guid, Position(
            targetX, targetY, AZGALOR_MAIN_TANK_POSITION.GetPositionZ()));
        it = azgalorRangedPositions.find(guid);
    }

    if (it == azgalorRangedPositions.end())
        return false;

    const Position& target = it->second;
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 2.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool AzgalorMoveToDoomguardTankAction::Execute(Event event)
{
    const Position& tankPosition = AZGALOR_DOOMGUARD_TANK_POSITION;
    if (bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY()) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(HYJAL_SUMMIT_MAP_ID, tankPosition.GetPositionX(), tankPosition.GetPositionY(),
                      tankPosition.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool AzgalorFirstAssistTankPositionDoomguardAction::Execute(Event event)
{
    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    if (!doomguard)
        return false;

    if (bot->GetVictim() != doomguard)
        return Attack(doomguard);

    if (doomguard->GetVictim() == bot && bot->IsWithinMeleeRange(doomguard))
    {
        const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool AzgalorMarkDoomguardWithSkullAction::Execute(Event event)
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "doomguard");
    if (doomguard)
        MarkTargetWithSkull(bot, doomguard);

    return false;
}

// Archimonde

bool ArchimondeMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", archimonde))
        return botAI->CastSpell("steady shot", archimonde);

    return false;
}

bool ArchimondeMainTankPositionBossAction::Execute(Event event)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (bot->GetVictim() != archimonde)
        return Attack(archimonde);

    if (archimonde->GetVictim() == bot)
    {
        const Position& position = ARCHIMONDE_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool ArchimondeCastFearWardOnMainTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}
