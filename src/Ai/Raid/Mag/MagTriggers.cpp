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
    if (IsEncounterInProgress(bot, MAG_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, MAG_MAP_ID);
}

bool MagtheridonMainTankShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonAssistTanksShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    if (GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return true;
    }

    return GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return GetChanneler(bot, WEST_CHANNELER_DB_GUID) || GetChanneler(bot, EAST_CHANNELER_DB_GUID);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    if (!GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return true;
    }

    if (!GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        return true;
    }

    return !PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK &&
        AI_VALUE2(Unit*, "find target", "burning abyssal"); // This should probably be a value
}

bool MagtheridonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return magtheridon->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonShouldSpreadRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon) || magtheridon->GetVictim() == bot)
        return false;

    /* constexpr uint32 dpsWaitMs = 6 * IN_MILLISECONDS;
    auto it = dpsWaitTimer.find(magtheridon->GetInstanceId());
    if (it == dpsWaitTimer.end() || getMSTimeDiff(it->second, getMSTime()) < dpsWaitMs)
        return false; */

    if (!IsCubeClicker(bot))
        return true;

    auto timerIt = blastNovaTimer.find(magtheridon->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return true;

    return getMSTimeDiff(timerIt->second, getMSTime()) < BLAST_NOVA_INTERIM_MS;
}

bool MagtheridonStandingInDebrisTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return IsPositionInActiveDebris(bot, bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && IsMagtheridonActive(magtheridon) && IsCubeClicker(bot);
}

bool MagtheridonShouldManageTimersAndAssignmentsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, MAG_MAP_ID))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && IsMagtheridonActive(magtheridon);
}
