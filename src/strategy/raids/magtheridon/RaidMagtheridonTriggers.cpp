#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonFirstThreeChannelersEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonNWChannelerEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    return channelerDiamond && channelerDiamond->IsAlive();
}

bool MagtheridonNEChannelerEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    return channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return (channelerStar && channelerStar->IsAlive()) ||
           (channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonDeterminingKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot))
        return false;

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
        if (channelerDiamond && channelerDiamond->IsAlive())
            return false;
    }

    if (botAI->IsAssistTankOfIndex(bot, 1))
    {
        Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
        if (channelerTriangle && channelerTriangle->IsAlive())
            return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon != nullptr;
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* abyssal = AI_VALUE2(Unit*, "find target", "burning abyssal");
    return abyssal != nullptr;
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    return !(channeler && channeler->IsAlive());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool needsReassign = botToCubeAssignment.empty();
    if (!needsReassign)
    {
        for (auto const& pair : botToCubeAssignment)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
            {
                needsReassign = true;
                break;
            }
        }
    }

    if (needsReassign)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
    }

    return botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon != nullptr;
}
