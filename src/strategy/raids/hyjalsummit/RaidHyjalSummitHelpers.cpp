#include "RaidHyjalSummitHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

namespace HyjalSummitHelpers
{
    // Rage Winterchill
    const Position RAGE_WINTERCHILL_TANK_POSITION = { 0.0f, 0.0f, 0.0f };
    std::unordered_map<ObjectGuid, Position> winterchillRangedPositions;

    // Anetheron
    const Position ANETHERON_MAIN_TANK_POSITION = { 0.0f, 0.0f, 0.0f };
    const Position ANETHERON_INFERNAL_TANK_POSITION = { 0.0f, 0.0f, 0.0f };

    // Kaz'rogal
    const Position KAZROGAL_TANK_POSITION = { 0.0f, 0.0f, 0.0f };

    // Azgalor
    const Position AZGALOR_TANK_POSITION = { 0.0f, 0.0f, 0.0f };

    // Archimonde
    const Position ARCHIMONDE_TANK_POSITION = { 0.0f, 0.0f, 0.0f };
}
