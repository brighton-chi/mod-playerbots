/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPState.h"
#include "Player.h"
#include <mutex>
#include <unordered_set>

namespace SwpHelpers
{

namespace
{

std::mutex swpStateMutex;
std::unordered_map<uint32, SwpInstanceState> swpStates;
std::unordered_map<ObjectGuid::LowType, uint32> kiljaedenDragonOrbUseTimes;
std::unordered_set<ObjectGuid> kiljaedenTrackedArmageddonTargets;

}

SwpInstanceState& SwpState(uint32 instanceId)
{
    std::lock_guard lock(swpStateMutex);
    return swpStates[instanceId];
}

bool SwpResetInstance(uint32 instanceId)
{
    std::lock_guard lock(swpStateMutex);
    auto it = swpStates.find(instanceId);
    if (it == swpStates.end())
        return false;

    SwpInstanceState const& state = it->second;
    bool const wasSet = state.kalecgosEncounterState || state.brutallusEncounterState ||
        state.felmystEncounterState || state.eredarTwinsIncomingConflagrationState ||
        state.eredarTwinsBlazeTargetState || state.eredarTwinsDpsHoldStartMs ||
        state.eredarTwinsTankAssignment || state.muruDarknessState ||
        state.muruVoidSentinelTankAssignments || state.kiljaedenEncounterState ||
        state.kiljaedenHandControlClaims;

    swpStates.erase(it);
    return wasSet;
}

void RecordKiljaedenDragonOrbUse(Player* bot)
{
    std::lock_guard lock(swpStateMutex);
    kiljaedenDragonOrbUseTimes[bot->GetGUID().GetCounter()] = getMSTime();
}

bool HasUsedKiljaedenDragonOrb(Player* bot)
{
    std::lock_guard lock(swpStateMutex);
    return kiljaedenDragonOrbUseTimes.contains(bot->GetGUID().GetCounter());
}

bool ResetKiljaedenDragonOrbUse(Player* bot)
{
    std::lock_guard lock(swpStateMutex);
    return kiljaedenDragonOrbUseTimes.erase(bot->GetGUID().GetCounter()) > 0;
}

bool IsKiljaedenArmageddonTargetTracked(ObjectGuid guid)
{
    std::lock_guard lock(swpStateMutex);
    return kiljaedenTrackedArmageddonTargets.contains(guid);
}

void TrackKiljaedenArmageddonTarget(ObjectGuid guid)
{
    std::lock_guard lock(swpStateMutex);
    kiljaedenTrackedArmageddonTargets.insert(guid);
}

void UntrackKiljaedenArmageddonTarget(ObjectGuid guid)
{
    std::lock_guard lock(swpStateMutex);
    kiljaedenTrackedArmageddonTargets.erase(guid);
}

}
