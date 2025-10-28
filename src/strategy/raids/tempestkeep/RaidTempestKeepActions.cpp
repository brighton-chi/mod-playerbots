#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

// Al'ar



// Void Reaver

// Position in center of room
bool VoidReaverPositionBossAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    
    if (bot->GetVictim() != voidReaver)
        return Attack(voidReaver);
    
    if (voidReaver->GetVictim() == bot)
    {
        const Location& tankPosition = TempestKeepLocations::VoidReaverTankPosition;
        const float maxDistance = 3.0f;

        float dX = tankPosition.x - bot->GetPositionX();
        float dY = tankPosition.y - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.x, tankPosition.y);

        if (distanceToTankPosition > maxDistance)
        {
            float step = std::min(maxDistance, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * maxDistance;
            const float moveZ = tankPosition.z;
            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        float orientation = atan2(voidReaver->GetPositionY() - bot->GetPositionY(), 
                                  voidReaver->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(voidReaver))
        return MoveTo(voidReaver->GetMapId(), voidReaver->GetPositionX(), voidReaver->GetPositionY(), voidReaver->GetPositionZ(), 
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);

    return false;
}

bool VoidReaverSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (voidReaver && voidReaver->IsAlive() && voidReaver->GetHealth() == voidReaver->GetMaxHealth())
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const Location& tankPosition = TempestKeepLocations::VoidReaverTankPosition;
    const float centerX = tankPosition.x;
    const float centerY = tankPosition.y;
    float centerZ = bot->GetPositionZ();
    const float minRadius = 25.0f;
    const float maxRadius = 40.0f;

    std::vector<Player*> members;
    Player* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            members.push_back(member);
            if (member != bot)
            {
                float dist = bot->GetExactDist2d(member);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestMember = member;
                }
            }
        }
    }

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) / 
                       static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return false;

            return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float minSpreadDistance = 10.0f;
    const float movementThreshold = 2.0f;

    if (closestMember && closestDist < minSpreadDistance - movementThreshold)
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(), 
                            closestMember->GetPositionZ()), minSpreadDistance, 0);

    return false;
}

bool VoidReaverArcaneOrbMoveAwayAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    auto arcaneOrbTargets = GetAllArcaneOrbTargets(botAI, bot);
    if (!voidReaver || arcaneOrbTargets.empty())
        return false;

    const float safeDistance = 25.0f;
    if (!IsInArcaneOrbRadius(arcaneOrbTargets, safeDistance))
        return false;

    // Find a safe position that's away from ALL orbs AND maintains proper distance from Void Reaver
    const float stepSize = 2.0f;
    const float maxSearchDist = 40.0f;
    const uint8 numAngles = 32;

    // Combat range constraints for Void Reaver
    const float minBossDistance = 25.0f;  // Stay at least 25 yards from boss
    const float maxBossDistance = 45.0f;

    Position bestPosition;
    float bestMoveDist = std::numeric_limits<float>::max();
    bool foundSafeSpot = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2 * M_PI * i) / numAngles;
        for (float dist = stepSize; dist <= maxSearchDist; dist += stepSize)
        {
            float testX = bot->GetPositionX() + cos(angle) * dist;
            float testY = bot->GetPositionY() + sin(angle) * dist;
            float testZ = bot->GetPositionZ();

            // Check distance from Void Reaver
            float distToBoss = sqrt(pow(testX - voidReaver->GetPositionX(), 2) + 
                                    pow(testY - voidReaver->GetPositionY(), 2));
            if (distToBoss < minBossDistance || distToBoss > maxBossDistance)
                continue;

            // Check if this position is safe from ALL orbs
            bool safeFromAllOrbs = true;
            for (Unit* orbTarget : arcaneOrbTargets)
            {
                float distToOrb = sqrt(pow(testX - orbTarget->GetPositionX(), 2) + 
                                       pow(testY - orbTarget->GetPositionY(), 2));
                if (distToOrb < safeDistance)
                {
                    safeFromAllOrbs = false;
                    break;
                }
            }

            if (safeFromAllOrbs)
            {
                float moveDist = sqrt(pow(testX - bot->GetPositionX(), 2) + 
                                      pow(testY - bot->GetPositionY(), 2));
                
                if (!foundSafeSpot || moveDist < bestMoveDist)
                {
                    bestPosition = Position(testX, testY, testZ);
                    bestMoveDist = moveDist;
                    foundSafeSpot = true;
                }
            }
        }
    }

    if (foundSafeSpot)
    {
        LOG_DEBUG("playerbots", "VoidReaverArcaneOrbMoveAwayAction: Bot {} moving to safe position away from all orbs and maintaining boss distance", 
                  bot->GetName());

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), bestPosition.GetPositionX(), bestPosition.GetPositionY(), 
                      bestPosition.GetPositionZ(), false, false, false, false, 
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool VoidReaverArcaneOrbMoveAwayAction::IsInArcaneOrbRadius(const std::vector<Unit*>& arcaneOrbTargets, float safeDistance = 25.0f)
{
    for (Unit* orbTarget : arcaneOrbTargets)
    {
        float distanceToOrb = bot->GetExactDist2d(orbTarget);
        if (distanceToOrb < safeDistance)
            return true;
    }

    return false;
}

// High Astromancer Solarian

bool HighAstromancerSolarianStackBotsAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    const Location& position = TempestKeepLocations::AstromancerStackPosition;

    // Phase 1 & 2 - Ranged
    if (botAI->IsRanged(bot) && bot->GetExactDist2d(position.x, position.y) > 4.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Phase 2 - Melee move to Ranged
    Unit* solariumAgent = AI_VALUE2(Unit*, "find target", "solarium agent");
    if (solariumAgent && botAI->IsMelee(bot) &&
        bot->GetExactDist2d(position.x, position.y) > 6.0f)
    {
        bot->AttackStop();
        return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;
    
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();

            if (!member || !member->IsAlive() || member == bot)
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < 10.0f)
                return FleePosition(Position(member->GetPositionX(), member->GetPositionY(), 
                                    member->GetPositionZ()), 12.0f, 0);
        }
    }

    return false;
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event event)
{
    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");
    Group* group = bot->GetGroup();
    if (!solariumPriest || !group)
        return false;

    auto solariumPriests = GetSolariumPriests();
    auto meleeMembers = GetMeleeBots(group);
    Unit* targetSolariumPriest = AssignSolariumPriestsToBots(solariumPriests, meleeMembers);
    if (!targetSolariumPriest)
        return false;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();
    if (botIndex < totalMelee / 2)
    {
        MarkTargetWithSquare(bot, targetSolariumPriest);
        SetRtiTarget(botAI, "square", targetSolariumPriest);
    }
    else
    {
        MarkTargetWithStar(bot, targetSolariumPriest);
        SetRtiTarget(botAI, "star", targetSolariumPriest);
    }

    if (bot->GetVictim() != targetSolariumPriest)
        return Attack(targetSolariumPriest);

    if (!bot->IsWithinMeleeRange(targetSolariumPriest))
        return MoveTo(bot->GetMapId(), targetSolariumPriest->GetPositionX(), targetSolariumPriest->GetPositionY(),
                      targetSolariumPriest->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);

    return false;
}

std::vector<Unit*> HighAstromancerSolarianTargetSolariumPriestsAction::GetSolariumPriests()
{
    std::vector<Unit*> solariumPriests;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SOLARIUM_PRIEST && unit->IsAlive())
            solariumPriests.push_back(unit);
    }

    return solariumPriests;
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots(Group* group)
{
    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && GET_PLAYERBOT_AI(member))
            meleeMembers.push_back(member);
    }

    std::sort(meleeMembers.begin(), meleeMembers.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    const std::vector<Unit*>& solariumPriests, const std::vector<Player*>& meleeMembers)
{
    if (solariumPriests.size() < 2 || meleeMembers.empty())
        return nullptr;

    // Sort priest adds by GUID for consistent targeting
    std::vector<Unit*> sortedSolariumPriests = solariumPriests;
    std::sort(sortedSolariumPriests.begin(), sortedSolariumPriests.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (botIndex < totalMelee / 2)
        return sortedSolariumPriests[0];
    else
        return sortedSolariumPriests[1];
}

bool HighAstromancerSolarianTankVoidwalkerAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    if (astromancer->GetVictim() != bot)
    {
        if (botAI->CanCastSpell("taunt", astromancer))
            return botAI->CastSpell("taunt", astromancer);

        if (botAI->CanCastSpell("growl", astromancer))
            return botAI->CastSpell("growl", astromancer);

        if (botAI->CanCastSpell("hand of reckoning", astromancer))
            return botAI->CastSpell("hand of reckoning", astromancer);
    }

    return false;
}

bool HighAstromancerSolarianCastFearWardOnMainTankAction::Execute(Event event)
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
