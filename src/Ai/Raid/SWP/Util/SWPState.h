/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPSTATE_H
#define PLAYERBOTS_SWPSTATE_H

#include "ObjectGuid.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include <optional>
#include <unordered_map>

class Player;

namespace SwpHelpers
{

using MuruVoidSentinelTankAssignments = std::unordered_map<ObjectGuid, uint8>;
using KiljaedenHandControlClaims = std::unordered_map<ObjectGuid, uint32>;

struct SwpInstanceState
{
    std::optional<KalecgosEncounterState> kalecgosEncounterState;
    std::optional<BrutallusEncounterState> brutallusEncounterState;
    std::optional<FelmystEncounterState> felmystEncounterState;
    std::optional<EredarTwinsIncomingConflagrationState> eredarTwinsIncomingConflagrationState;
    std::optional<EredarTwinsBlazeTargetState> eredarTwinsBlazeTargetState;
    std::optional<uint32> eredarTwinsDpsHoldStartMs;
    std::optional<EredarTwinsTankAssignment> eredarTwinsTankAssignment;
    std::optional<MuruDarknessState> muruDarknessState;
    std::optional<MuruVoidSentinelTankAssignments> muruVoidSentinelTankAssignments;
    std::optional<KiljaedenEncounterState> kiljaedenEncounterState;
    std::optional<KiljaedenHandControlClaims> kiljaedenHandControlClaims;
};

SwpInstanceState& SwpState(uint32 instanceId);
// Only the mechanic tracker runs this. Every bot clears its own Brutallus entries field by field.
bool SwpResetInstance(uint32 instanceId);

// Keyed by bot or creature rather than by instance, so SwpResetInstance() leaves them alone.
void RecordKiljaedenDragonOrbUse(Player* bot);
bool HasUsedKiljaedenDragonOrb(Player* bot);
bool ResetKiljaedenDragonOrbUse(Player* bot);
bool IsKiljaedenArmageddonTargetTracked(ObjectGuid guid);
void TrackKiljaedenArmageddonTarget(ObjectGuid guid);
void UntrackKiljaedenArmageddonTarget(ObjectGuid guid);

}

#endif
