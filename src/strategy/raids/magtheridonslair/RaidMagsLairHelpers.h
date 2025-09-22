#ifndef _PLAYERBOT_RAIDMAGSLAIRHELPERS_H
#define _PLAYERBOT_RAIDMAGSLAIRHELPERS_H

#include <map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

enum MagsLairSpells
{
	// Magtheridon
    SPELL_BLAST_NOVA               = 30616,
    SPELL_AURA_SHADOW_GRASP        = 30410,
    SPELL_AURA_DEBRIS           = 30632,

};

inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;
inline constexpr int8 crossIcon = RtiTargetValue::crossIndex;

bool IsSouthTank(PlayerbotAI* botAI, Player* bot);
bool IsWestTank(PlayerbotAI* botAI, Player* bot);
bool IsEastTank(PlayerbotAI* botAI, Player* bot);
bool IsSouthWarlock(PlayerbotAI* botAI, Player* bot);
bool IsWestWarlock(PlayerbotAI* botAI, Player* bot);
bool IsEastWarlock(PlayerbotAI* botAI, Player* bot);
bool IsWestHealer(PlayerbotAI* botAI, Player* bot);
bool IsEastHealer(PlayerbotAI* botAI, Player* bot);
bool IsWestHunter(PlayerbotAI* botAI, Player* bot);
bool IsEastHunter(PlayerbotAI* botAI, Player* bot);

extern std::map<ObjectGuid, int> cubeAssignments;
extern std::map<ObjectGuid, time_t> cubeTimers;

struct TankSpot
{
    float x, y, z, orientation;
};

extern const TankSpot MagtheridonTankSpot;

struct CubePosition 
{
    float x, y, z;
};

extern const CubePosition manticronCubes[5];

void AssignRangedDpsToCubes(Group* group, PlayerbotAI* botAI);

#endif
