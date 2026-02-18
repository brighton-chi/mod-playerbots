#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitEraseTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();

    bool erased = false;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill") &&
        hasReachedWinterchillPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "anetheron") &&
        hasReachedAnetheronPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal") &&
        hasReachedKazrogalPosition.erase(guid) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "azgalor") &&
        azgalorTankStep.erase(guid) > 0)
    {
        erased = true;
    }

    return erased;
}

// Rage Winterchill

bool RageWinterchillMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", winterchill))
        return botAI->CastSpell("steady shot", winterchill);

    return false;
}

bool RageWinterchillMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    if (bot->GetTarget() != winterchill->GetGUID())
        return Attack(winterchill);

    if (winterchill->GetVictim() == bot)
    {
        const Position& position = RAGE_WINTERCHILL_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool RageWinterchillSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedWinterchillPosition[guid])
    {
        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        float radius = 0.0f;
        float angle = 0.0f;

        constexpr float arcSpan = 2.0f * M_PI;
        constexpr float arcCenter = 0.0f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 25.0f;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 35.0f;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = RAGE_WINTERCHILL_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
        float targetY = RAGE_WINTERCHILL_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
            hasReachedWinterchillPosition[guid] = true;
    }

    return false;
}

// Anetheron

bool AnetheronMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", anetheron))
        return botAI->CastSpell("steady shot", anetheron);

    return false;
}

bool AnetheronMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    MarkTargetWithSquare(bot, anetheron);
    SetRtiTarget(botAI, "square", anetheron);

    if (bot->GetTarget() != anetheron->GetGUID())
        return Attack(anetheron);

    if (anetheron->GetVictim() == bot)
    {
        const Position& position = ANETHERON_MAIN_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AnetheronSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        if (botAI->IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    if (healers.empty() && rangedDps.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedAnetheronPosition[guid])
    {
        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        float radius = 0.0f;
        float angle = 0.0f;

        constexpr float arcSpan = 3.0f * M_PI / 2.0f;
        constexpr float arcCenter = 0.165f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 27.0f;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 34.0f;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = ANETHERON_MAIN_TANK_POSITION.GetPositionX() + radius * std::sin(angle);
        float targetY = ANETHERON_MAIN_TANK_POSITION.GetPositionY() + radius * std::cos(angle);

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            hasReachedAnetheronPosition[guid] = true;

            constexpr float safeDistFromPlayer = 6.0f;
            constexpr float minInterval = 2000.0f;
            if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
                return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
        }
    }

    return false;
}

bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    const Position& position = GetClosestInfernalTankPosition(bot);
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        botAI->Reset();
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AnetheronAssistTankPickUpInfernalsAction::Execute(Event /*event*/)
{
    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
        return false;

    MarkTargetWithDiamond(bot, infernal);
    SetRtiTarget(botAI, "diamond", infernal);

    if (bot->GetTarget() != infernal->GetGUID())
        return Attack(infernal);

    if (infernal->GetVictim() == bot)
    {
        const Position& position = GetClosestInfernalTankPosition(bot);
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (botAI->IsMelee(bot))
    {
        SetRtiTarget(botAI, "square", anetheron);

        if (bot->GetTarget() != anetheron->GetGUID())
            return Attack(anetheron);

        return false;
    }
    else if (botAI->IsRanged(bot))
    {
        if (Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal"))
        {
            Player* victim = dynamic_cast<Player*>(infernal->GetVictim());
            if (victim && botAI->IsTank(victim) && bot->GetDistance2d(infernal) < 40.0f)
            {
                SetRtiTarget(botAI, "diamond", infernal);

                if (bot->GetTarget() != infernal->GetGUID())
                    return Attack(infernal);
            }
        }
        else
        {
            SetRtiTarget(botAI, "square", anetheron);
            if (bot->GetTarget() != anetheron->GetGUID())
                return Attack(anetheron);
        }
    }

    return false;
}

// Kaz'rogal

bool KazrogalMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", kazrogal))
        return botAI->CastSpell("steady shot", kazrogal);

    return false;
}

bool KazrogalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->GetTarget() != kazrogal->GetGUID())
        return Attack(kazrogal);

    if (kazrogal->GetVictim() == bot && bot->IsWithinMeleeRange(kazrogal))
    {
        const Position& position = KAZROGAL_TANK_POSITION;
        float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, true);
        }
    }

    return false;
}

bool KazrogalAssistTanksMoveInFrontOfBossAction::Execute(Event /*event*/)
{
    const Position& position = KAZROGAL_TANK_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distToPosition > 4.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool KazrogalSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    if (!hasReachedKazrogalPosition[guid])
    {
        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;

        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 4.65f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        constexpr float radius = 20.0f;
        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = KAZROGAL_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
        float targetY = KAZROGAL_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            return MoveTo(HYJAL_SUMMIT_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
            hasReachedKazrogalPosition[guid] = true;
    }

    return false;
}

bool KazrogalLowManaBotMoveFromGroupAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        if (!botAI->HasAura("aspect of the viper", bot) &&
            botAI->CanCastSpell("aspect of the viper", bot))
        {
            return botAI->CastSpell("aspect of the viper", bot);
        }
    }
    else
    {
        if (bot->HasAura(SPELL_MARK_OF_KAZROGAL))
        {
            if (bot->getClass() == CLASS_MAGE &&
                botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot))
            {
                return true;
            }
            else if (bot->getClass() == CLASS_PALADIN &&
                     botAI->CanCastSpell("divine shield", bot) &&
                     botAI->CastSpell("divine shield", bot))
            {
                return true;
            }
        }

        constexpr float safeDistance = 16.0f;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveFromGroup(safeDistance);
        }
    }

    return false;
}

// Azgalor

bool AzgalorMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", azgalor))
        return botAI->CastSpell("steady shot", azgalor);

    return false;
}

bool AzgalorMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    MarkTargetWithStar(bot, azgalor);
    SetRtiTarget(botAI, "star", azgalor);

    if (bot->GetTarget() != azgalor->GetGUID())
        return Attack(azgalor);

    if (azgalor->GetVictim() == bot)
    {
        const ObjectGuid guid = bot->GetGUID();
        uint8 step = azgalorTankStep.count(guid) ? azgalorTankStep[guid] : 0;

        const Position positions[2] =
        {
            AZGALOR_MAIN_TANK_TRANSITION_POSITION,
            AZGALOR_MAIN_TANK_FINAL_POSITION
        };
        constexpr float maxDistance = 2.0f;
        const Position& position = positions[step];
        float distToPosition = bot->GetExactDist2d(position);

        if ((distToPosition > maxDistance) && bot->IsWithinMeleeRange(azgalor))
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, true);
        }

        if (step == 0 && distToPosition <= maxDistance)
        {
            azgalorTankStep[guid] = 1;
        }
        else if (step == 1 && distToPosition <= maxDistance)
        {
            float orientation = atan2(azgalor->GetPositionY() - bot->GetPositionY(),
                                      azgalor->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
    }

    return false;
}

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    constexpr uint32 minInterval = 1000;

    // Azgalor's hitbox is 8.8 yards
    constexpr float safeDistFromBoss = 29.0f;
    if (bot->GetExactDist2d(azgalor) < safeDistFromBoss)
        return FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval);

    // Lesser Doomguard's hitbox is 3.75 yards
    constexpr float safeDistFromDoomguard = 14.0f;
    if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
        doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard, minInterval);

    constexpr float safeDistFromPlayer = 6.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

bool AzgalorMoveToDoomguardTankAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 5.0f)
    {
        return MoveTo(HYJAL_SUMMIT_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AzgalorFirstAssistTankPositionDoomguardAction::Execute(Event /*event*/)
{
    const Position& position = AZGALOR_DOOMGUARD_TANK_POSITION;
    float distToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    float moveDist = 0.0f;
    bool shouldMove = false;
    bool moveBackwards = false;

    if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard"))
    {
        MarkTargetWithCircle(bot, doomguard);
        SetRtiTarget(botAI, "circle", doomguard);

        if (bot->GetTarget() != doomguard->GetGUID())
            return Attack(doomguard);

        if (doomguard->GetVictim() == bot && bot->IsWithinMeleeRange(doomguard) &&
            distToPosition > 3.0f)
        {
            moveDist = std::min(5.0f, distToPosition);
            shouldMove = true;
            moveBackwards = true;
        }
    }
    else if (distToPosition > 3.0f)
    {
        moveDist = std::min(10.0f, distToPosition);
        shouldMove = true;
        moveBackwards = false;
    }

    if (shouldMove)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                      true, moveBackwards);
    }

    return false;
}

bool AzgalorAssignDpsPriorityAction::Execute(Event /*event*/)
{
    if (botAI->IsRanged(bot))
    {
        if (Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
            doomguard && bot->GetDistance2d(doomguard) < 40.0f)
        {
            SetRtiTarget(botAI, "circle", doomguard);

            if (bot->GetTarget() != doomguard->GetGUID())
                return Attack(doomguard);
        }
    }
    else if (Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor"))
    {
        SetRtiTarget(botAI, "star", azgalor);

        if (bot->GetTarget() != azgalor->GetGUID())
            return Attack(azgalor);
    }

    return false;
}

// Archimonde

bool ArchimondeMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", archimonde))
        return botAI->CastSpell("steady shot", archimonde);

    return false;
}

bool ArchimondeCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

bool ArchimondeSpreadToAvoidAirBurstAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    constexpr float safeDistFromVictim = 16.0f;
    constexpr float safeDistFromPlayer = 8.0f;
    constexpr uint32 minInterval = 1000;

    Unit* victim = archimonde->GetVictim();
    if (victim && victim != bot && bot->GetExactDist2d(victim) < safeDistFromVictim &&
        FleePosition(victim->GetPosition(), safeDistFromVictim, minInterval))
        return true;

    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (nearestPlayer && botAI->IsRanged(bot) &&
        FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval))
        return true;

    return false;
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    std::vector<Unit*> doomfires;
    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_DOOMFIRE)
            doomfires.push_back(unit);
    }

    if (doomfires.empty())
        return false;

    std::vector<DoomfireLine> hazardLines;
    for (Unit* doomfire : doomfires)
    {
        Position start = doomfire->GetPosition();
        float destX, destY, destZ;
        if (doomfire->GetMotionMaster()->GetDestination(destX, destY, destZ))
        {
            Position end(destX, destY, destZ);
            hazardLines.push_back({start, end});
        }
    }

    constexpr float hazardWidth = 15.0f;
    bool inDanger = false;
    for (auto const& line : hazardLines)
    {
        float dist = DistanceToDoomfireLine(bot->GetPosition(), line.start, line.end);
        if (dist < hazardWidth)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Position safePos = FindSafePositionFromDoomfires(bot, hazardLines, hazardWidth);

    float distToSafePos = bot->GetExactDist2d(safePos.GetPositionX(), safePos.GetPositionY());
    if (distToSafePos > 1.0f)
    {
        float dx = safePos.GetPositionX() - bot->GetPositionX();
        float dy = safePos.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToSafePos);
        float moveX = bot->GetPositionX() + (dx / distToSafePos) * moveDist;
        float moveY = bot->GetPositionY() + (dy / distToSafePos) * moveDist;

        Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
        bool backwards = (archimonde && archimonde->GetVictim() == bot);

        bot->InterruptNonMeleeSpells(true);
        return MoveTo(HYJAL_SUMMIT_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, backwards);
    }

    return false;
}

float ArchimondeAvoidDoomfireAction::DistanceToDoomfireLine(
    const Position& testPosition, const Position& lineStartPosition, const Position& lineEndPosition)
{
    float dx = lineEndPosition.GetPositionX() - lineStartPosition.GetPositionX();
    float dy = lineEndPosition.GetPositionY() - lineStartPosition.GetPositionY();
    float lengthSq = dx*dx + dy*dy;
    if (lengthSq == 0.0f)
        return testPosition.GetExactDist2d(lineStartPosition.GetPositionX(),
                                           lineStartPosition.GetPositionY());

    float projectionFactor = ((testPosition.GetPositionX() - lineStartPosition.GetPositionX()) * dx +
                              (testPosition.GetPositionY() - lineStartPosition.GetPositionY()) * dy) / lengthSq;
    projectionFactor = std::max(0.0f, std::min(1.0f, projectionFactor));
    float projX = lineStartPosition.GetPositionX() + projectionFactor * dx;
    float projY = lineStartPosition.GetPositionY() + projectionFactor * dy;

    return testPosition.GetExactDist2d(projX, projY);
}

Position ArchimondeAvoidDoomfireAction::FindSafePositionFromDoomfires(
    Player* bot, const std::vector<DoomfireLine>& lines, float hazardWidth)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 40.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos = bot->GetPosition();
    float minMoveDistance = std::numeric_limits<float>::max();

    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::sin(angle);
            float y = bot->GetPositionY() + distance * std::cos(angle);
            float z = bot->GetPositionZ();

            bool isSafe = true;
            for (auto const& line : lines)
            {
                if (DistanceToDoomfireLine(Position(x, y, z), line.start, line.end) < hazardWidth)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            float moveDistance = bot->GetExactDist2d(x, y);
            if (moveDistance < minMoveDistance)
            {
                bestPos = Position(x, y, z);
                minMoveDistance = moveDistance;
            }
        }
    }

    return bestPos;
}

bool ArchimondeRemoveDoomfireDotAction::Execute(Event /*event*/)
{
    if (bot->GetHealthPct() > 40.0f)
        return false;

    if (botAI->CanCastSpell("ice block", bot))
        return botAI->CastSpell("ice block", bot);
    else if (botAI->CanCastSpell("cloak of shadows", bot))
        return botAI->CastSpell("cloak of shadows", bot);
    else if (botAI->CanCastSpell("divine shield", bot))
        return botAI->CastSpell("divine shield", bot);

    return false;
}
