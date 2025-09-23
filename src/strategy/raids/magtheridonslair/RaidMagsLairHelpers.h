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
    SPELL_AURA_DEBRIS              = 30632,

    // Warlock
    SPELL_BANISH                   = 18647,
    SPELL_CURSE_OF_TONGUES         = 11719,
    SPELL_FEAR                     = 6215,

    // Hunter
    SPELL_AURA_MISDIRECTION        = 34477,
};

enum MagsLairNPCs
{
    NPC_BURNING_ABYSSAL = 17454,
};

constexpr uint32 SOUTH_CHANNELER     = 43160;
constexpr uint32 WEST_CHANNELER      = 43158;
constexpr uint32 NORTHWEST_CHANNELER = 43161;
constexpr uint32 EAST_CHANNELER      = 43159;
constexpr uint32 NORTHEAST_CHANNELER = 43157;

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
Creature* GetChanneler(Player* bot, uint32 dbGuid);

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
