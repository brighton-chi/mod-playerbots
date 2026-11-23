/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagTriggers.h"
#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Playerbots.h"
#include <optional>

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

    Unit* magtheridon = GetMagtheridon(bot);
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonAssistTanksShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        return true;
    }

    return GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool MagtheridonHunterShouldMisdirectChannelersTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    return GetChanneler(bot, WEST_CHANNELER_DB_GUID) || GetChanneler(bot, EAST_CHANNELER_DB_GUID);
}

bool MagtheridonShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    return !PlayerbotAI::IsTank(bot);
}

bool MagtheridonDpsHoldIsActiveTrigger::IsActiveInEncounter()
{
    Unit* magtheridon = GetMagtheridon(bot);
    return magtheridon && IsMagtheridonDpsHoldActive(bot, magtheridon) &&
        !IsAnyChannelerAlive(bot);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK && !GetBurningAbyssals(bot).empty();
}

bool MagtheridonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* magtheridon = GetMagtheridon(bot);
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return magtheridon->GetVictim() == bot || PlayerbotAI::IsMainTank(bot);
}

bool MagtheridonRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* magtheridon = GetMagtheridon(bot);
    if (!magtheridon || !IsMagtheridonActive(magtheridon) || magtheridon->GetVictim() == bot)
        return false;

    if (!IsCubeClicker(bot))
        return true;

    std::optional<uint32> const& blastNovaTimer =
        MagState(magtheridon->GetInstanceId()).blastNovaTimer;
    if (!blastNovaTimer)
        return true;

    return getMSTimeDiff(*blastNovaTimer, getMSTime()) < BLAST_NOVA_INTERIM_MS;
}

bool MagtheridonStandingInDebrisTrigger::IsActiveInEncounter()
{
    return IsCeilingCollapsed(bot) &&
        IsPositionInActiveDebris(botAI, bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()
{
    if (!IsCubeClicker(bot))
        return false;

    Unit* magtheridon = GetMagtheridon(bot);
    return magtheridon && IsMagtheridonActive(magtheridon);
}

bool MagtheridonShouldManageTimersAndAssignmentsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, MAG_MAP_ID))
        return false;

    Unit* magtheridon = GetMagtheridon(bot);
    return magtheridon && IsMagtheridonActive(magtheridon);
}
