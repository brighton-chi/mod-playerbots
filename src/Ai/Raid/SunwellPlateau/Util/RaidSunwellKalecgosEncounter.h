/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLKALECGOSENCOUNTER_H
#define _PLAYERBOT_RAIDSUNWELLKALECGOSENCOUNTER_H

#include <array>
#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "ObjectGuid.h"
#include "Position.h"
#include "RaidSunwellData.h"

class Player;
class PlayerbotAI;

namespace SunwellHelpers
{

constexpr uint8 KALECGOS_TANK_COUNT = 3;
constexpr uint8 KALECGOS_GROUP_COUNT = 4;
constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
constexpr float KALECGOS_SPECTRAL_REALM_Z = -74.5f;

struct KalecgosRealmState
{
    uint32 lastEnterMs = 0;
    uint32 lastExitMs = 0;
    bool inSpectralRealm = false;
};

struct KalecgosEncounterState
{
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid firstEntrantGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty
    };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

extern const Position KALECGOS_TANK_POSITION;
extern const Position KALECGOS_INITIAL_RANGED_POSITION;

extern std::unordered_map<uint32, KalecgosEncounterState> kalecgosEncounterStates;
extern std::unordered_map<ObjectGuid, KalecgosRealmState> kalecgosRealmStates;
extern std::unordered_set<ObjectGuid> hasReachedKalecgosInitialRangedPosition;

bool IsKalecgosDecurser(PlayerbotAI* botAI, Player* bot);
void EnsureKalecgosGroupAssignments(PlayerbotAI* botAI, Player* bot);
Player* GetKalecgosCurrentTank(PlayerbotAI* botAI, Player* bot);
void SetKalecgosInitialRangedPositionReached(Player* bot, bool reached);
bool ShouldEnterKalecgosSpectralRift(PlayerbotAI* botAI, Player* bot);
bool IsInKalecgosSpectralRealm(Player* bot);
bool IsKalecgosRealmTransitionGraceActive(Player* bot);
void RecordKalecgosSpectralBlastTarget(PlayerbotAI* botAI, Player* bot);
void RecordKalecgosSpectralRealmEnter(PlayerbotAI* botAI, Player* bot);
void RecordKalecgosNormalRealmEnter(Player* bot);

}

#endif
