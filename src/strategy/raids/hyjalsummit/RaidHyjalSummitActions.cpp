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
            radius = 20.0f;
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
        const float arcCenter = 0.165f;           // your chosen orientation
        const float arcStart = arcCenter - arcSpan / 2.0f; // start of arc

        if (botAI->IsHeal(bot))
        {
            size_t count = healers.size();
            if (count == 0)
                return false;

            auto findIt = std::find(healers.begin(), healers.end(), bot);
            size_t botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 25.0f;
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
            radius = 20.0f;
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
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(HYJAL_SUMMIT_MAP_ID, tankPosition.GetPositionX(), tankPosition.GetPositionY(),
                  tankPosition.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
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

// Azgalor

// Archimonde

