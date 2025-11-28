#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"

using namespace ZulAmanHelpers;
using namespace ZulAmanPositions;

// Akil'zon <Eagle Avatar>

bool AkilzonSpreadRangedAction::Execute(Event event)
{
    const float minDistance = 15.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* stormBot = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_ELECTRICAL_STORM))
            return member == stormBot;
    }

    float distanceFromStorm = bot->GetExactDist2d(bot, stormBot);
    if (distanceFromStorm > 4.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), stormBot->GetPositionX(),
                      stormBot->GetPositionY(), stormBot->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Nalorakk <Bear Avatar>

// Jan'alai <Dragonhawk Avatar>

// Halazzi <Lynx Avatar>

// Hex Lord Malacrass

// Zul'jin
