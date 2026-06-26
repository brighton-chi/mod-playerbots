#include "MagActions.h"
#include "MagHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace MagtheridonHelpers;

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event /*event*/)
{
    Creature* channelerTarget = nullptr;
    if (Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER))
    {
        channelerTarget = channelerSquare;
        MarkTargetWithSquare(bot, channelerSquare);
        SetRtiTarget(botAI, "square", channelerSquare);
    }
    else if (Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER))
    {
        channelerTarget = channelerStar;
        MarkTargetWithStar(bot, channelerStar);
        SetRtiTarget(botAI, "star", channelerStar);
    }
    else if (Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER))
    {
        channelerTarget = channelerCircle;
        MarkTargetWithCircle(bot, channelerCircle);
        SetRtiTarget(botAI, "circle", channelerCircle);
    }

    if (channelerTarget && AI_VALUE(Unit*, "current target") != channelerTarget)
        return Attack(channelerTarget);

    // After first three channelers are dead, wait for Magtheridon to activate
    if (!channelerTarget)
    {
        const Position& position = WAITING_FOR_MAGTHERIDON_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(MAGTHERIDON_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        bot->SetFacingTo(position.GetOrientation());
        return true;
    }

    return false;
}

bool MagtheridonFirstAssistTankAttackNWChannelerAction::Execute(Event /*event*/)
{
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (!channelerDiamond)
        return false;

    MarkTargetWithDiamond(bot, channelerDiamond);
    SetRtiTarget(botAI, "diamond", channelerDiamond);

    if (AI_VALUE(Unit*, "current target") != channelerDiamond)
        return Attack(channelerDiamond);

    if (channelerDiamond->GetVictim() == bot)
    {
        const Position& position = NW_CHANNELER_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(distanceToPosition, 10.0f);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MagtheridonSecondAssistTankAttackNEChannelerAction::Execute(Event /*event*/)
{
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (!channelerTriangle)
        return false;

    MarkTargetWithTriangle(bot, channelerTriangle);
    SetRtiTarget(botAI, "triangle", channelerTriangle);

    if (AI_VALUE(Unit*, "current target") != channelerTriangle)
        return Attack(channelerTriangle);

    if (channelerTriangle->GetVictim() == bot)
    {
        const Position& position = NE_CHANNELER_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(distanceToPosition, 10.0f);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonMisdirectHellfireChannelersToMainTankAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Player* mainTank = GetGroupMainTank(botAI, bot);

    Creature* targetChanneler = nullptr;
    if (hunterIndex == 0)
        targetChanneler = GetChanneler(bot, WEST_CHANNELER);
    else if (hunterIndex == 1)
        targetChanneler = GetChanneler(bot, EAST_CHANNELER);

    if (mainTank && targetChanneler)
    {
        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (!bot->HasAura(static_cast<uint32>(MagtheridonSpells::SPELL_MISDIRECTION)))
            return false;

        if (botAI->CanCastSpell("steady shot", targetChanneler))
            return botAI->CastSpell("steady shot", targetChanneler);
    }

    return false;
}

bool MagtheridonAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Listed in order of priority
    if (Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER))
    {
        MarkTargetWithSquare(bot, channelerSquare);
        SetRtiTarget(botAI, "square", channelerSquare);

        if (AI_VALUE(Unit*, "current target") != channelerSquare)
            return Attack(channelerSquare);
    }
    else if (Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER))
    {
        MarkTargetWithStar(bot, channelerStar);
        SetRtiTarget(botAI, "star", channelerStar);

        if (AI_VALUE(Unit*, "current target") != channelerStar)
            return Attack(channelerStar);
    }
    else if (Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER))
    {
        MarkTargetWithCircle(bot, channelerCircle);
        SetRtiTarget(botAI, "circle", channelerCircle);

        if (AI_VALUE(Unit*, "current target") != channelerCircle)
            return Attack(channelerCircle);
    }
    else if (Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER))
    {
        MarkTargetWithDiamond(bot, channelerDiamond);
        SetRtiTarget(botAI, "diamond", channelerDiamond);

        if (AI_VALUE(Unit*, "current target") != channelerDiamond)
            return Attack(channelerDiamond);
    }
    else if (Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER))
    {
        MarkTargetWithTriangle(bot, channelerTriangle);
        SetRtiTarget(botAI, "triangle", channelerTriangle);

        if (AI_VALUE(Unit*, "current target") != channelerTriangle)
            return Attack(channelerTriangle);
    }
    else if (Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon"))
    {
        SetRtiTarget(botAI, "cross", magtheridon);

        if (AI_VALUE(Unit*, "current target") != magtheridon)
            return Attack(magtheridon);
    }

    return false;
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonWarlockCcBurningAbyssalAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> abyssals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(MagtheridonNpcs::NPC_BURNING_ABYSSAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            abyssals.push_back(creature);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK &&
            GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex >= 0 && warlockIndex < abyssals.size())
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!botAI->HasAura("banish", assignedAbyssal) &&
            botAI->CanCastSpell("banish", assignedAbyssal))
        {
            return botAI->CastSpell("banish", assignedAbyssal);
        }
    }

    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) &&
            !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
        {
            return botAI->CastSpell("fear", excessAbyssal);
        }
    }

    return false;
}

// Main tank will back up to the Northern point of the room
bool MagtheridonMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    MarkTargetWithCross(bot, magtheridon);
    SetRtiTarget(botAI, "cross", magtheridon);

    if (AI_VALUE(Unit*, "current target") != magtheridon)
        return Attack(magtheridon);

    if (magtheridon->GetVictim() == bot && bot->GetHealthPct() > 50.0f)
    {
        const Position& position = MAGTHERIDON_TANK_POSITION;
        const float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distanceToPosition);
            const float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Just stay away from boss and other players
// Magtheridon's CombatReach is 12 yards and BoundingRadius is 4 yards
bool MagtheridonSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    if (IsCubeClicker(bot))
    {
        const time_t now = time(nullptr);
        auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
        if (timerIt != blastNovaTimer.end())
        {
            const time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= BLAST_NOVA_INTERIM_SECONDS)
                return false;
        }
    }

    constexpr float safeDistFromBoss = 15.0f;
    const float currentDistance = bot->GetDistance2d(magtheridon);
    if (currentDistance < safeDistFromBoss)
        return MoveAway(magtheridon, safeDistFromBoss - currentDistance);

    constexpr float safeDistFromPlayer = 6.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

// For bots that are assigned to click cubes
// Magtheridon casts Blast Nova every 54.35 to 55.40s, with a 2s cast time
bool MagtheridonUseManticronCubeAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    CubeInfo const* cubeInfo = GetAssignedCube(bot);
    if (!cubeInfo)
        return false;

    GameObject* cube = botAI->GetGameObject(cubeInfo->guid);
    if (!cube)
        return false;

    // Release cubes after Blast Nova is interrupted
    if (HandleCubeRelease(magtheridon))
        return true;

    // Check if cube logic should be active (49+ second rule)
    if (!ShouldActivateCubeLogic(magtheridon))
        return false;

    // Handle active cube logic based on Blast Nova casting state
    const bool blastNovaActive =
        magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA));

    if (!blastNovaActive)
        // After 49 seconds, wait at safe distance from cube
        return HandleWaitingPhase(*cubeInfo);
    else
        // Blast Nova is casting - move to and click cube
        return HandleCubeInteraction(*cubeInfo, cube);

    return false;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (bot->HasAura(static_cast<uint32>(MagtheridonSpells::SPELL_SHADOW_GRASP)) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
          magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA))))
    {
        uint32 delay = urand(200, 3000);
        botAI->AddTimedEvent(
            [this]
            {
                botAI->Reset();
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::ShouldActivateCubeLogic(Unit* magtheridon)
{
    auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return false;

    const time_t now = time(nullptr);
    const time_t lastBlastNova = timerIt->second;

    return (now - lastBlastNova >= BLAST_NOVA_INTERIM_SECONDS);
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    constexpr float safeWaitDistance = 8.0f;
    const float cubeDist = bot->GetDistance2d(cubeInfo.x, cubeInfo.y);

    if (fabs(cubeDist - safeWaitDistance) <= 1.0f)
        return true;

    Position safePos;
    if (FindSafePositionNearCube(cubeInfo, safeWaitDistance, safePos))
    {
        botAI->InterruptSpell();
        return MoveTo(MAGTHERIDON_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // No safe position found; fall back to a random angle at the preferred distance
    const float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    const float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
    const float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;

    return MoveTo(MAGTHERIDON_MAP_ID, fallbackX, fallbackY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MagtheridonUseManticronCubeAction::FindSafePositionNearCube(
    const CubeInfo& cubeInfo, float preferredDistance, Position& outPos)
{
    constexpr float maxSearchRadius = 15.0f;
    constexpr float distanceStep = 1.0f;
    constexpr float angleStep = M_PI / 12.0f;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = 2.0f; distance <= maxSearchRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
        {
            const float x = bot->GetPositionX() + distance * std::cos(angle);
            const float y = bot->GetPositionY() + distance * std::sin(angle);

            const float cubeDist = std::sqrt(
                (x - cubeInfo.x) * (x - cubeInfo.x) + (y - cubeInfo.y) * (y - cubeInfo.y));
            if (std::fabs(cubeDist - preferredDistance) > 2.0f)
                continue;

            if (!IsSafeFromMagtheridonHazards(botAI, bot, x, y))
                continue;

            constexpr float debrisHazardRadius = 9.0f;
            constexpr uint32 debrisMaxAgeMs = 8000;
            if (IsPositionInActiveDebris(
                    bot->GetMap()->GetInstanceId(), x, y, debrisHazardRadius, debrisMaxAgeMs))
                continue;

            const float moveDistance = bot->GetExactDist2d(x, y);
            Position candidate(x, y, bot->GetPositionZ());
            const bool pathSafe = IsPathSafeFromHazards(bot->GetPosition(), candidate);

            if (pathSafe || !foundSafe)
            {
                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    outPos = candidate;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    outPos = candidate;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return foundSafe;
}

bool MagtheridonUseManticronCubeAction::IsPathSafeFromHazards(
    const Position& start, const Position& end)
{
    constexpr uint8 numChecks = 10;
    constexpr float debrisHazardRadius = 9.0f;
    constexpr uint32 debrisMaxAgeMs = 8000;
    const float dx = end.GetPositionX() - start.GetPositionX();
    const float dy = end.GetPositionY() - start.GetPositionY();
    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        const float ratio = static_cast<float>(i) / numChecks;
        const float checkX = start.GetPositionX() + dx * ratio;
        const float checkY = start.GetPositionY() + dy * ratio;

        if (!IsSafeFromMagtheridonHazards(botAI, bot, checkX, checkY))
            return false;

        if (IsPositionInActiveDebris(instanceId, checkX, checkY, debrisHazardRadius, debrisMaxAgeMs))
            return false;
    }

    return true;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(
    const CubeInfo& cubeInfo, GameObject* cube)
{
    constexpr float interactDistance = 1.0f;
    const float cubeDist = bot->GetDistance2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist < interactDistance + 1.0f)
    {
        uint32 delay = urand(200, 1500);
        botAI->AddTimedEvent(
            [this, cube]
            {
                bot->StopMoving();
                cube->Use(bot);
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    botAI->InterruptSpell();
    return MoveTo(cube, interactDistance, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonMoveOutOfDebrisAction::Execute(Event /*event*/)
{
    Position safePos;
    if (FindSafePosition(safePos))
    {
        botAI->InterruptSpell();
        return MoveTo(MAGTHERIDON_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MagtheridonMoveOutOfDebrisAction::FindSafePosition(Position& outPos)
{
    constexpr float maxSearchRadius = 15.0f;
    constexpr float distanceStep = 1.0f;
    constexpr float angleStep = M_PI / 12.0f;
    constexpr float hazardRadius = 9.0f;
    constexpr uint32 maxAgeMs = 8000;

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;
    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    for (float distance = 2.0f; distance <= maxSearchRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
        {
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (IsPositionInActiveDebris(instanceId, x, y, hazardRadius, maxAgeMs))
                continue;

            if (!IsSafeFromMagtheridonHazards(botAI, bot, x, y))
                continue;

            float const moveDistance = bot->GetExactDist2d(x, y);

            if (!foundSafe || moveDistance < minMoveDistance)
            {
                outPos = Position(x, y, bot->GetPositionZ());
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }
    }

    return foundSafe;
}

// Known issue: The Blast Nova timer resets when Magtheridon stops casting it, which is needed to
// ensure that bots use cubes. However, Blast Nova's cooldown runs from when he starts casting it.
// This means that if a Blast Nova is not interrupted or takes too long to interrupt,
// the timer will be thrown off for the rest of the encounter.
bool MagtheridonManageTimersAndAssignmentsAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();
    const time_t now = time(nullptr);

    const bool blastNovaActive =
        magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA));

    if (lastBlastNovaState && !blastNovaActive)
        blastNovaTimer[instanceId] = now;

    lastBlastNovaState = blastNovaActive;

    if (IsMagtheridonActive(magtheridon) &&
        IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
    {
        blastNovaTimer.try_emplace(instanceId, now);
        dpsWaitTimer.try_emplace(instanceId, now);

        if (NeedsCubeReassignment(instanceId))
            AssignCubeClickers(bot->GetGroup(), bot->GetMap(), botAI);
    }
    else
    {
        if (!IsMagtheridonActive(magtheridon))
            RemoveCubeClicker(bot);

        if (IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
        {
            blastNovaTimer.erase(instanceId);
            dpsWaitTimer.erase(instanceId);
        }
    }

    return false;
}
