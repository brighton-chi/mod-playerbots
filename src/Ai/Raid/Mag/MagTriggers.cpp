/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagTriggers.h"
#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Playerbots.h"

using namespace MagHelpers;
using namespace EncounterHelpers;

bool MagtheridonNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != MAG_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

bool MagtheridonFirstThreeChannelersEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonLastTwoChannelersEngagedByAssistTanksTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    if (GetChanneler(bot, NORTHWEST_CHANNELER) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return true;
    }

    return GetChanneler(bot, NORTHEAST_CHANNELER) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
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
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    if (!GetChanneler(bot, NORTHWEST_CHANNELER) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return true;
    }

    if (!GetChanneler(bot, NORTHEAST_CHANNELER) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        return true;
    }

    return !PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK &&
        AI_VALUE2(Unit*, "find target", "burning abyssal");
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return magtheridon->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    constexpr uint32 dpsWaitMs = 6 * IN_MILLISECONDS;
    auto it = dpsWaitTimer.find(magtheridon->GetInstanceId());
    if (it == dpsWaitTimer.end() || getMSTimeDiff(it->second, getMSTime()) < dpsWaitMs)
        return false;

    return magtheridon->GetVictim() != bot;
}

bool MagtheridonStandingInDebrisTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return IsPositionInActiveDebris(bot, bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && IsMagtheridonActive(magtheridon) && IsCubeClicker(bot);
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, MAG_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "magtheridon");
}
