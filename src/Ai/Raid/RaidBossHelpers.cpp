/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidBossHelpers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <list>

// Projects a short step from the bot towards a destination and snaps it to walkable ground.
// The step is deliberately small: GetMapWaterOrGroundLevel only finds ground at or below its
// seed, so a short hop keeps the bot's own Z usable as that seed. Pass destinationZ only when
// the caller knows it is correct at the destination--a Z inherited from a circle centre or from
// the bot itself is not, and seeding from the bot is the better guess in that case.
//
// Returns false when the bot is already there, or when no walkable step could be validated. The
// latter lets a caller that enumerates candidate destinations fall through to the next one.
bool GetGroundedStepPosition(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ, float const* destinationZ)
{
    // A step shorter than this is not worth the map queries needed to validate it
    constexpr float minStepDistance = 0.5f;

    float const distance = bot->GetExactDist2d(destinationX, destinationY);
    if (distance < minStepDistance)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();
    float const deltaX = destinationX - botX;
    float const deltaY = destinationY - botY;

    // Reject a step onto ground the bot cannot climb. Set to false if raid terrain turns out to be
    // rejected too aggressively and bots stop repositioning
    constexpr bool failOnSlopes = true;
    // How many times the step is halved after the map rejects it before giving up
    constexpr uint8 maxAttempts = 3;

    float stepDistance = std::min(moveDist, distance);

    for (uint8 attempt = 0; attempt < maxAttempts; ++attempt)
    {
        if (stepDistance < minStepDistance)
            return false;

        float const ratio = stepDistance / distance;
        float candidateX = botX + deltaX * ratio;
        float candidateY = botY + deltaY * ratio;

        // Interpolating the seed towards a trustworthy destination Z lets the search find ground
        // above the bot, which a seed taken from the bot alone cannot do
        float const seedZ = destinationZ ? botZ + (*destinationZ - botZ) * ratio : botZ;
        float candidateZ = bot->GetMapWaterOrGroundLevel(candidateX, candidateY, seedZ);

        // No ground in that column. Keep the bot's Z as the candidate and let the reach check
        // rule on it rather than silently moving to an ungrounded position
        if (candidateZ <= INVALID_HEIGHT)
            candidateZ = botZ;

        // failOnCollision stays false so a blocked step is shortened to the contact point instead
        // of being discarded; the slope check is what reports a step the bot cannot actually take
        if (bot->GetMap()->CanReachPositionAndGetValidCoords(
                bot, botX, botY, botZ, candidateX, candidateY, candidateZ, false, failOnSlopes))
        {
            stepX = candidateX;
            stepY = candidateY;
            stepZ = candidateZ;
            return true;
        }

        stepDistance *= 0.5f;
    }

    return false;
}

// Overload for a destination whose Z is known good, such as a hand-placed encounter position
bool GetGroundedStepPosition(
    Player* bot, Position const& destination, float moveDist, Position& step)
{
    float const destinationZ = destination.GetPositionZ();
    float stepX = 0.0f, stepY = 0.0f, stepZ = 0.0f;

    if (!GetGroundedStepPosition(
            bot, destination.GetPositionX(), destination.GetPositionY(),
            moveDist, stepX, stepY, stepZ, &destinationZ))
    {
        return false;
    }

    step.Relocate(stepX, stepY, stepZ);
    return true;
}

// Functions to mark targets with raid target icons
// Note that these functions do not allow the player to change the icon during the encounter
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
{
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentGuid = group->GetTargetIcon(iconId);
    if (currentGuid != target->GetGUID())
    {
        group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
        return true;
    }

    return false;
}

bool MarkTargetWithSkull(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
}

bool MarkTargetWithSquare(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
}

bool MarkTargetWithStar(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
}

bool MarkTargetWithCircle(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
}

bool MarkTargetWithDiamond(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
}

bool MarkTargetWithTriangle(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
}

bool MarkTargetWithCross(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
}

bool MarkTargetWithMoon(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
}

bool ClearTargetIcon(Player* bot, uint8 iconId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentGuid = group->GetTargetIcon(iconId);
    if (currentGuid != ObjectGuid::Empty)
    {
        group->SetTargetIcon(iconId, bot->GetGUID(), ObjectGuid::Empty);
        return true;
    }

    return false;
}

// For bots to set their raid target icon to the specified icon on the specified target
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName, Unit* target)
{
    if (!target)
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    context->GetValue<std::string>("rti")->Set(rtiName);
    context->GetValue<Unit*>("rti target")->Set(target);
}

// Return the first alive bot in the specified instance map for purposes of assigning
// a single bot to manage associative containers, mark targets, etc.
bool IsMechanicTrackerBot(Player* bot, uint32 mapId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != mapId ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        return member == bot;
    }

    return false;
}

// Requires the main tank to be alive
Player* GetGroupMainTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    ObjectGuid const mainTankGuid = botAI->GetMainTankGuid(group);
    if (mainTankGuid.IsEmpty())
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetGUID() == mainTankGuid)
            return member;
    }

    return nullptr;
}

// Returns the alive assist tank of the specified index (0 = first, 1 = second, etc.)
Player* GetGroupAssistTank(PlayerbotAI* botAI, Player* bot, uint8 index)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    ObjectGuid const mainTankGuid = botAI->GetMainTankGuid(group);
    if (mainTankGuid.IsEmpty())
        return nullptr;

    uint8 assistantCount = 0;
    std::vector<Player*> nonAssistantTanks;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsTank(member) ||
            member->GetGUID() == mainTankGuid)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
        {
            if (assistantCount == index)
                return member;

            assistantCount++;
        }
        else
        {
            nonAssistantTanks.push_back(member);
        }
    }

    uint8 nonAssistantIndex = index - assistantCount;
    if (nonAssistantIndex < nonAssistantTanks.size())
        return nonAssistantTanks[nonAssistantIndex];

    return nullptr;
}

// Return the first matching alive unit from PossibleTargetsValue within sightDistance from config
// Note that PossibleTargetsValue picks up only hostile units
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
{
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& unitGuid : units)
    {
        Unit* unit = botAI->GetUnit(unitGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

// Return the nearest alive player (human or bot) within the specified radius. Distance is
// measured by GetExactDist2d(), which does not take into account player hitboxes (1.5y).
Player* GetNearestPlayerInRadius(Player* bot, float radius)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearestPlayer = nullptr;
    float nearestDistance = radius;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float distance = bot->GetExactDist2d(member);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestPlayer = member;
        }
    }

    return nearestPlayer;
}

// Grid search for dynamic objects for methods to avoid dynobj-based AoE hazards
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId)
{
    std::list<WorldObject*> objs;
    Acore::AllWorldObjectsInRange check(bot, searchRadius);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objs, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, searchRadius);

    std::vector<Position> dynObjs;
    for (WorldObject* obj : objs)
    {
        if (obj->GetTypeId() != TYPEID_DYNAMICOBJECT)
            continue;

        DynamicObject* dynObj = static_cast<DynamicObject*>(obj);
        if (dynObj->GetSpellId() == spellId)
        {
            dynObjs.emplace_back(
                dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ());
        }
    }

    return dynObjs;
}
