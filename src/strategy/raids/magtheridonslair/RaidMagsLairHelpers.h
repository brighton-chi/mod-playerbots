#ifndef _PLAYERBOT_RAIDMAGSLAIRHELPERS_H
#define _PLAYERBOT_RAIDMAGSLAIRHELPERS_H

#include <map>

#include "ObjectGuid.h"
#include "Group.h"
#include "PlayerbotAI.h"

enum MagsLairSpells
{
	// Magtheridon
    SPELL_BLAST_NOVA               = 30616,
    SPELL_AURA_SHADOW_GRASP        = 30410,

};

struct TankSpot
{
    float x, y, z, orientation;
};

extern const TankSpot MagtheridonTankSpot;

extern std::map<ObjectGuid, int> cubeAssignments;
extern std::map<ObjectGuid, time_t> cubeTimers;

struct CubePosition 
{
    float x, y, z;
};

extern const CubePosition manticronCubes[5];

void AssignRangedDpsToCubes(Group* group, PlayerbotAI* botAI);

#endif
