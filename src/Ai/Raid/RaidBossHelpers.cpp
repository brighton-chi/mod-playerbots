#include "RaidBossHelpers.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// Functions to mark targets with raid target icons
// Note that these functions do not allow the player to change the icon during the encounter
void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
{
    if (!target)
        return;

    if (Group* group = bot->GetGroup())
    {
        ObjectGuid currentGuid = group->GetTargetIcon(iconId);
        if (currentGuid != target->GetGUID())
            group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
    }
}

void MarkTargetWithSkull(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
}

void MarkTargetWithSquare(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
}

void MarkTargetWithStar(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
}

void MarkTargetWithCircle(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
}

void MarkTargetWithDiamond(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
}

void MarkTargetWithTriangle(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
}

void MarkTargetWithCross(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
}

void MarkTargetWithMoon(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
}

// For bots to set their raid target icon to the specified icon on the specified target
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
{
    if (!target)
        return;

    std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

    if (currentRti != rtiName || currentTarget != target)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
    }
}

// Return the first alive DPS bot in the specified instance map, excluding any specified bot
// Intended for purposes of storing and erasing timers and trackers in associative containers
bool IsMechanicTrackerBot(PlayerbotAI* botAI, Player* bot, uint32 mapId, Player* exclude)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != mapId)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI || !memberAI->IsDps(member))
                continue;

            if (member != exclude)
                return member == bot;
        }
    }

    return false;
}

// Requires the main tank to be alive (IsMainTank() will return the player with
// the main tank flag, even if dead)
Player* GetGroupMainTank(PlayerbotAI* botAI, Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI && memberAI->IsMainTank(member))
                return member;
        }
    }

    return nullptr;
}

// The below functions require the assist tanks to be alive (the 3rd parameter of
// IsAssistTankOfIndex() otherwise by default does not require them to be alive)
Player* GetGroupFirstAssistTank(PlayerbotAI* botAI, Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI && memberAI->IsAssistTankOfIndex(member, 0, false))
                return member;
        }
    }

    return nullptr;
}

Player* GetGroupSecondAssistTank(PlayerbotAI* botAI, Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI && memberAI->IsAssistTankOfIndex(member, 1, false))
                return member;
        }
    }

    return nullptr;
}

Player* GetGroupThirdAssistTank(PlayerbotAI* botAI, Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI && memberAI->IsAssistTankOfIndex(member, 2, false))
                return member;
        }
    }

    return nullptr;
}

// Return the first matching alive unit from a CreatureListSearcher
Unit* GetFirstAliveUnitByEntry(
    PlayerbotAI* botAI, uint32 entry, bool requireInCombat, float searchRadius)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    // If no searchRadius is provided, default to the bot's configured sight distance
    if (searchRadius <= 0.0f)
        searchRadius = sPlayerbotAIConfig.sightDistance;

    std::list<Creature*> targets;
    Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, searchRadius);
    Acore::CreatureListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, searchRadius);

    for (Creature* creature : targets)
    {
        if (creature && creature->IsAlive() && creature->GetEntry() == entry)
        {
            // Note that some units are never considered in combat (e.g., totems)
            if (!requireInCombat || creature->IsInCombat())
                return creature;
        }
    }

    return nullptr;
}

// Return the nearest alive player (human or bot) within the specified radius
Unit* GetNearestPlayerInRadius(Player* bot, float searchRadius)
{
    Unit* nearestPlayer = nullptr;
    float nearestDistance = searchRadius;

    if (Group* group = bot->GetGroup())
    {
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
    }

    return nearestPlayer;
}
