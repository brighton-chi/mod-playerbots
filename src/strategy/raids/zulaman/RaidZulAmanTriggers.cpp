#include "RaidZulAmanTriggers.h"
#include "RaidZulAmanHelpers.h"
#include "RaidZulAmanActions.h"
#include "Playerbots.h"

using namespace ZulAmanHelpers;

// Akil'zon <Eagle Avatar>

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akilzon");
    if (!akilzon)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_ELECTRICAL_STORM))
            return false;
    }

    return true;
}

bool AkilzonElectricalStormHasFormed::IsActive()
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akilzon");
    if (!akilzon)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_ELECTRICAL_STORM))
                return true;
        }
    }

    return false;
}

// Nalorakk <Bear Avatar>

// Jan'alai <Dragonhawk Avatar>

// Halazzi <Lynx Avatar>

// Hex Lord Malacrass

// Zul'jin
