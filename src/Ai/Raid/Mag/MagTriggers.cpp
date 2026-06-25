#include "MagTriggers.h"
#include "MagHelpers.h"
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
    if (!botAI->IsAssistTankOfIndex(bot, 0, false))
        return false;

    return AI_VALUE2(Unit*, "find target", "magtheridon") &&
           GetChanneler(bot, NORTHWEST_CHANNELER);
}

bool MagtheridonNEChannelerEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, false))
        return false;

    return AI_VALUE2(Unit*, "find target", "magtheridon") &&
           GetChanneler(bot, NORTHEAST_CHANNELER);
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return GetChanneler(bot, WEST_CHANNELER) || GetChanneler(bot, EAST_CHANNELER);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return !botAI->IsMainTank(bot) &&
           !(botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond) &&
           !(botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK &&
           AI_VALUE2(Unit*, "find target", "burning abyssal");
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;

    return IsCubeClicker(bot);
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    return !botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "magtheridon");
}
