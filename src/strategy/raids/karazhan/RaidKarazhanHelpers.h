#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

enum class KarazhanSpells
{
    // Maiden of Virtue
    REPENTANCE                 = 29511,

    // Opera Event
    LITTLE_RED_RIDING_HOOD     = 30756,

    // Shade of Aran
    FLAME_WREATH_CAST          = 30004,
    FLAME_WREATH_AURA          = 29946,
    ARCANE_EXPLOSION           = 29973,
    WARLOCK_BANISH             = 18647,

    // Netherspite
    RED_BEAM_DEBUFF            = 30421,
    GREEN_BEAM_DEBUFF          = 30422,
    BLUE_BEAM_DEBUFF           = 30423,
    NETHER_EXHAUSTION_RED      = 38637,
    NETHER_EXHAUSTION_GREEN    = 38638,
    NETHER_EXHAUSTION_BLUE     = 38639,
    NETHERSPITE_BANISHED       = 39833,

    // Prince Malchezaar
    ENFEEBLE                   = 30843,

    // Nightbane
    CHARRED_EARTH              = 30129,
    // RAIN_OF_BONES              = 37098,
};

enum class KarazhanNpcs
{
    // Attumen the Huntsman
    ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Terestian Illhoof
    KILREK                       = 17229,
    DEMON_CHAINS                 = 17248,

    // Shade of Aran
    CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    VOID_ZONE                    = 16697,
    RED_PORTAL                   = 17369,
    BLUE_PORTAL                  = 17368,
    GREEN_PORTAL                 = 17367,

    // Prince Malchezaar
    NETHERSPITE_INFERNAL         = 17646,
};

extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION;
extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8];
extern const Position KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION;
extern const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4];
extern const Position KARAZHAN_THE_CURATOR_BOSS_POSITION;
extern const Position KARAZHAN_NIGHTBANE_TRANSITION_BOSS_POSITION;
extern const Position KARAZHAN_NIGHTBANE_FINAL_BOSS_POSITION;
extern const Position KARAZHAN_NIGHTBANE_RANGED_POSITION_1;
extern const Position KARAZHAN_NIGHTBANE_RANGED_POSITION_2;
extern const Position KARAZHAN_NIGHTBANE_RANGED_POSITION_3;
extern const Position KARAZHAN_NIGHTBANE_FLIGHT_POSITION;

class RaidKarazhanHelpers : public AiObject
{
public:
    explicit RaidKarazhanHelpers(PlayerbotAI* botAI) : AiObject(botAI) {}

    void MarkTargetWithIcon(Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Unit* target);
    void MarkTargetWithSquare(Unit* target);
    void MarkTargetWithMoon(Unit* target);
    Unit* GetFirstAliveUnit(const std::vector<Unit*>& units);
    Unit* GetFirstAliveUnitByEntry(uint32 entry);
    Unit* GetNearestPlayerInRadius(float radius = 5.0f);
    bool IsFlameWreathActive();
    Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss);
    std::vector<Player*> GetRedBlockers();
    std::vector<Player*> GetBlueBlockers();
    std::vector<Player*> GetGreenBlockers();
    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers();
    std::vector<Unit*> GetAllVoidZones();
    bool IsSafePosition (float x, float y, float z, const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetSpawnedInfernals() const;
    bool IsStraightPathSafe(const Position& start, const Position& target, 
         const std::vector<Unit*>& hazards, float hazardRadius, float stepSize);
};

#endif
