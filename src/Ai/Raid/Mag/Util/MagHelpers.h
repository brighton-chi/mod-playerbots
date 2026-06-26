#ifndef PLAYERBOTS_MAGHELPERS_H
#define PLAYERBOTS_MAGHELPERS_H

#include <ctime>
#include <unordered_map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"

namespace MagtheridonHelpers
{

enum class MagtheridonSpells : uint32
{
    // Magtheridon
    SPELL_SHADOW_CAGE   = 30205,
    SPELL_SHADOW_GRASP  = 30410,
    SPELL_BLAST_NOVA    = 30616,
    SPELL_DEBRIS_SPAWN  = 30630,
    SPELL_QUAKE         = 30657,

    // Hunter
    SPELL_MISDIRECTION  = 35079,
};

enum class MagtheridonNpcs : uint32
{
    NPC_BURNING_ABYSSAL = 17454,
};

enum class MagtheridonObjects : uint32
{
    GO_BLAZE            = 181832,
};

constexpr uint32 MAGTHERIDON_MAP_ID         = 544;
constexpr uint32 SOUTH_CHANNELER            = 90978;
constexpr uint32 WEST_CHANNELER             = 90979;
constexpr uint32 NORTHWEST_CHANNELER        = 90980;
constexpr uint32 EAST_CHANNELER             = 90982;
constexpr uint32 NORTHEAST_CHANNELER        = 90981;
constexpr uint8 BLAST_NOVA_INTERIM_SECONDS = 49;

extern const Position WAITING_FOR_MAGTHERIDON_POSITION;
extern const Position MAGTHERIDON_TANK_POSITION;
extern const Position NW_CHANNELER_TANK_POSITION;
extern const Position NE_CHANNELER_TANK_POSITION;
extern const Position RANGED_SPREAD_POSITION;
extern const Position HEALER_SPREAD_POSITION;

extern std::unordered_map<uint32, time_t> dpsWaitTimer;
extern std::unordered_map<uint32, time_t> blastNovaTimer;

struct CubeInfo
{
    ObjectGuid guid;
    float x, y, z;
};
extern const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS;
std::vector<CubeInfo> GetAllCubeInfosByDbGuids(
    Map* map, const std::vector<uint32>& cubeDbGuids);

Creature* GetChanneler(Player* bot, uint32 dbGuid);
bool IsMagtheridonActive(Unit* magtheridon);
CubeInfo const* GetAssignedCube(Player* bot);
bool IsCubeClicker(Player* bot);
void AssignCubeClickers(Group* group, Map* map, PlayerbotAI* botAI);
void UnassignCubeClicker(Player* bot);
bool NeedsCubeReassignment(uint32 instanceId);

struct DebrisData
{
    Position position;
    uint32 spawnTime;
};
extern std::unordered_map<uint32, std::vector<DebrisData>> activeDebrisPositions;
bool IsPositionInActiveDebris(uint32 instanceId, float x, float y);
bool IsPositionInActiveConflagration(PlayerbotAI* botAI, Player* bot, float x, float y);

}

#endif
