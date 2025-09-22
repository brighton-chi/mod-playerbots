#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "ChatHelper.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool MagtheridonHellfireChannelerSouthTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channeler = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43160); // South channeler example
    if (!channeler || !channeler->IsAlive())
        return false;

    // Mark with square icon
    ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channeler->GetGUID())
        group->SetTargetIcon(squareIcon, bot->GetGUID(), channeler->GetGUID());

    // Set RTI value and target
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channeler)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channeler);
    }

    if (bot->GetVictim() != channeler)
    {
        Attack(channeler);

        if (!bot->IsWithinMeleeRange(channeler))
            return MoveTo(channeler->GetMapId(), channeler->GetPositionX(), channeler->GetPositionY(), channeler->GetPositionZ());
    }

    return false;
}

bool MagtheridonHellfireChannelerSouthTankAction::isUseful()
{
    if (!IsSouthTank(botAI, bot))
        return false;

    Creature* channeler = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43160); // South

    return channeler && channeler->IsAlive();
}

bool MagtheridonHellfireChannelerWestTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerStar = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43158);      // West
    Creature* channelerCircle = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43161);    // Northwest

    if ((!channelerStar || !channelerStar->IsAlive()) && (!channelerCircle || !channelerCircle->IsAlive()))
        return false;

    // Target selection logic (swap every 5 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerStar || !channelerStar->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerCircle || !channelerCircle->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 5)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
    if (channelerStar && channelerStar->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerStar->GetGUID())
            group->SetTargetIcon(starIcon, bot->GetGUID(), channelerStar->GetGUID());
    }
    if (channelerCircle && channelerCircle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerCircle->GetGUID())
            group->SetTargetIcon(circleIcon, bot->GetGUID(), channelerCircle->GetGUID());
    }

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerStar : channelerCircle;
    std::string rtiName = (currentTargetIndex == 0) ? "star" : "circle";
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerWestTankAction::isUseful()
{
    if (!IsWestTank(botAI, bot))
        return false;

    Creature* channelerStar = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43158);      // West
    Creature* channelerCircle = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43161);    // Northwest

    // Check if at least one assigned Channeler is alive
    if ((channelerStar && channelerStar->IsAlive()) || (channelerCircle && channelerCircle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerEastTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Creature* channelerDiamond = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43159);      // East
    Creature* channelerTriangle = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43157);    // Northeast

    if ((!channelerDiamond || !channelerDiamond->IsAlive()) && (!channelerTriangle || !channelerTriangle->IsAlive()))
        return false;

    // Target selection logic (swap every 5 seconds, or always target the alive one if the other is dead)
    static uint8 currentTargetIndex = 0;
    static time_t lastSwapTime = 0;
    time_t now = time(nullptr);

    if (!channelerDiamond || !channelerDiamond->IsAlive())
        currentTargetIndex = 1;
    else if (!channelerTriangle || !channelerTriangle->IsAlive())
        currentTargetIndex = 0;
    else if (now - lastSwapTime >= 5)
    {
        currentTargetIndex = (currentTargetIndex + 1) % 2;
        lastSwapTime = now;
    }

    // Mark both channelers
    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
            group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }
    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
            group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());
    }

    // Set RTI and target
    Creature* currentTarget = (currentTargetIndex == 0) ? channelerDiamond : channelerTriangle;
    std::string rtiName = (currentTargetIndex == 0) ? "diamond" : "triangle";
    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerEastTankAction::isUseful()
{
    if (!IsEastTank(botAI, bot))
        return false;

    Creature* channelerDiamond = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43159);      // East
    Creature* channelerTriangle = ObjectAccessor::GetSpawnedCreatureByDBGUID(bot->GetMapId(), 43157);    // Northeast

    // Check if at least one assigned Channeler is alive
    if ((channelerDiamond && channelerDiamond->IsAlive()) || (channelerTriangle && channelerTriangle->IsAlive()))
        return true;

    return false;
}

bool MagtheridonHellfireChannelerSouthWarlockAction::Execute(Event event)
{
    // Implementation for South Warlock action
    return false;
}

bool MagtheridonHellfireChannelerSouthWarlockAction::isUseful()
{
    return IsSouthWarlock(botAI, bot);
}

bool MagtheridonHellfireChannelerWestWarlockAction::Execute(Event event)
{
    // Implementation for West Warlock action
    return false;
}

bool MagtheridonHellfireChannelerWestWarlockAction::isUseful()
{
    return IsWestWarlock(botAI, bot);
}

bool MagtheridonHellfireChannelerEastWarlockAction::Execute(Event event)
{
    // Implementation for East Warlock action
    return false;
}

bool MagtheridonHellfireChannelerEastWarlockAction::isUseful()
{
    return IsEastWarlock(botAI, bot);
}

bool MagtheridonHellfireChannelerWestHealerAction::Execute(Event event)
{
    // Implementation for West Healer action
    return false;
}

bool MagtheridonHellfireChannelerWestHealerAction::isUseful()
{
    return IsWestHealer(botAI, bot);
}

bool MagtheridonHellfireChannelerEastHealerAction::Execute(Event event)
{
    // Implementation for East Healer action
    return false;
}

bool MagtheridonHellfireChannelerEastHealerAction::isUseful()
{
    return IsEastHealer(botAI, bot);
}

bool MagtheridonHellfireChannelerWestHunterAction::Execute(Event event)
{
    // Implementation for West Hunter action
    return false;
}

bool MagtheridonHellfireChannelerWestHunterAction::isUseful()
{
    return IsWestHunter(botAI, bot);
}

bool MagtheridonHellfireChannelerEastHunterAction::Execute(Event event)
{
    // Implementation for East Hunter action
    return false;
}

bool MagtheridonHellfireChannelerEastHunterAction::isUseful()
{
    return IsEastHunter(botAI, bot);
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    // Implementation for DPS Priority action
    return false;
}

bool MagtheridonHellfireChannelerDPSPriorityAction::isUseful()
{
    return botAI->IsDps(bot);
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    static const TankSpot& tankSpot = MagtheridonTankSpot;

    const float maxDistance = 3.0f;
    float distanceToTankSpot = magtheridon->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxDistance)
    {
        float dX = tankSpot.x - magtheridon->GetPositionX();
        float dY = tankSpot.y - magtheridon->GetPositionY();
        float moveX = tankSpot.x + (dX / distanceToTankSpot) * maxDistance;
        float moveY = tankSpot.y + (dY / distanceToTankSpot) * maxDistance;

        float moveDistance = bot->GetExactDist2d(moveX, moveY);
        if (moveDistance < 0.5f)
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), moveX, moveY, tankSpot.z, 
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    float currentOrientation = bot->GetOrientation();
    float delta = tankSpot.orientation - currentOrientation;
    while (delta > M_PI)
        delta -= 2 * M_PI;
    while (delta < -M_PI)
        delta += 2 * M_PI;
    float orientationDifference = fabs(static_cast<double>(delta));

    const float orientationLeeway = 15.0f * M_PI / 180.0f;
    if (orientationDifference > orientationLeeway)
    {
        bot->SetFacingTo(tankSpot.orientation);
    }

    return false; 
}

bool MagtheridonPositionBossAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon && magtheridon->IsAlive() && botAI->IsTank(bot) && 
           botAI->HasAggro(magtheridon) && magtheridon->GetVictim() == bot;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->IsInCombat() && magtheridon->GetHealth() == magtheridon->GetMaxHealth())
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const TankSpot& tankSpot = MagtheridonTankSpot;

    // Gather all ranged bots in the group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        rangedBots.push_back(member);
    }

    // Assign initial positions if not already done
    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        // Find this bot's index among ranged bots
        size_t botIndex = 0;
        for (size_t i = 0; i < rangedBots.size(); ++i)
        {
            if (rangedBots[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        // Fan parameters
        float arcDegrees = 100.0f; // 50 degrees either side
        float arcRadians = arcDegrees * M_PI / 180.0f;
        float centerAngle = tankSpot.orientation;
        float startAngle = centerAngle - arcRadians / 2.0f;
        float angleStep = rangedBots.size() > 1 ? arcRadians / (rangedBots.size() - 1) : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        // Random radius between 25 and 45 yards
        float minRadius = 25.0f;
        float maxRadius = 45.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = tankSpot.x + radius * cos(angle);
        float targetY = tankSpot.y + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            return MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ());
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float magtheridonRangedRadius = 25.0f;
    float minSpreadDistance = 10.0f;
    float movementThreshold = 2.0f;
    Unit* closestMember = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot || !botAI->IsRanged(member))
        {
            continue;
        }

        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
        {
            closestMember = member;
        }
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float distanceToMagtheridon = bot->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToMagtheridon < magtheridonRangedRadius - 3.0f - movementThreshold)
    {
        return MoveTo(bot->GetMapId(), tankSpot.x, tankSpot.y, tankSpot.z);
    }

    return false;
}

bool MagtheridonSpreadRangedAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon && magtheridon->IsInCombat() && botAI->IsRanged(bot);
}

// Movement action for clickers to stay near cubes
bool MagtheridonManticronCubeClickerPositionAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsAlive() || !magtheridon->IsInCombat())
    {
        cubeTimers.clear();
        return false;
    }

    // Assign ranged DPS to cubes if not already assigned
    Group* group = bot->GetGroup();
    if (cubeAssignments.empty())
        AssignRangedDpsToCubes(group, botAI);

    // Make sure this bot is assigned to a cube
    auto it = cubeAssignments.find(bot->GetGUID());
    if (it == cubeAssignments.end())
        return false;

    int cubeIndex = it->second;
    static const CubePosition& cube = manticronCubes[cubeIndex];

    // Timer logic
    time_t now = time(nullptr);

    // If Magtheridon is casting Blast Nova, reset timer for this bot
    if (magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        cubeTimers[bot->GetGUID()] = now;
        return false;
    }

    // If timer hasn't started, start it now
    if (cubeTimers[bot->GetGUID()] == 0)
        cubeTimers[bot->GetGUID()] = now;

    // Wait 45 seconds after the start of the last Blast Nova cast
    if (now - cubeTimers[bot->GetGUID()] < 45)
        return false;

    // Move to 5 yards away from cube
    float offsetAngle = urand(0, 359) * M_PI / 180.0f;
    float targetX = cube.x + cos(offsetAngle) * 5.0f;
    float targetY = cube.y + sin(offsetAngle) * 5.0f;
    float targetZ = cube.z;

    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

// Action for clickers to use cube
bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsInCombat())
        return false;

    // Only act if Magtheridon is casting Blast Nova
    if (!(magtheridon->HasUnitState(UNIT_STATE_CASTING) && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA)))
        return false;

    // Make sure this bot is assigned to a cube
    auto it = cubeAssignments.find(bot->GetGUID());
    if (it == cubeAssignments.end())
        return false;

    int cubeIndex = it->second;
    static const CubePosition& cube = manticronCubes[cubeIndex];

    // Move to within interaction distance (e.g. 1.5 yards)
    float interactionDistance = 1.5f;
    float offsetAngle = urand(0, 359) * M_PI / 180.0f;
    float targetX = cube.x + cos(offsetAngle) * interactionDistance;
    float targetY = cube.y + sin(offsetAngle) * interactionDistance;
    float targetZ = cube.z;

    // If not close enough, move closer
    if (bot->GetExactDist2d(targetX, targetY) > interactionDistance)
        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED);

    // Find the actual game object for the cube (by position)
    GuidVector nearbyCubes = chat->parseGameobjects("manticron cube");
    GameObject* go = nullptr;
    for (ObjectGuid guid : nearbyCubes)
    {
        GameObject* candidate = botAI->GetGameObject(guid);
        if (candidate &&
            fabs(candidate->GetPositionX() - cube.x) < 1.0f &&
            fabs(candidate->GetPositionY() - cube.y) < 1.0f)
        {
            go = candidate;
            break;
        }
    }
    if (!go)
        return false;

    // Use the cube
    go->Use(bot);

    return true;
}
