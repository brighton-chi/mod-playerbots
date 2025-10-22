#ifndef _PLAYERBOT_RAIDSSCHELPERS_H_
#define _PLAYERBOT_RAIDSSCHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

enum SerpentShrineCavernSpells
{
    // Hydross the Unstable <Duke of Currents>

    SPELL_MARK_OF_HYDROSS_10 = 38215,
    SPELL_MARK_OF_HYDROSS_25 = 38216,
    SPELL_MARK_OF_HYDROSS_50 = 38217,
    SPELL_MARK_OF_HYDROSS_100 = 38218,
    SPELL_MARK_OF_HYDROSS_250 = 38231,
    SPELL_MARK_OF_HYDROSS_500 = 40584,
    SPELL_MARK_OF_CORRUPTION_10 = 38219,
    SPELL_MARK_OF_CORRUPTION_25 = 38220,
    SPELL_MARK_OF_CORRUPTION_50 = 38221,
    SPELL_MARK_OF_CORRUPTION_100 = 38222,
    SPELL_MARK_OF_CORRUPTION_250 = 38230,
    SPELL_MARK_OF_CORRUPTION_500 = 40583,
    SPELL_CORRUPTION = 37961, // TBD if better to use this aura or NPC ID to identify nature form

    // The Lurker Below

    SPELL_SPOUT = 37433,

    // Leotheras the Blind

    SPELL_WHIRLWIND = 37640,
    SPELL_WHIRLWIND_CHANNEL = 37641,

    // Morogrim Tidewalker

    SPELL_WATERY_GRAVE = 38049,

}

enum SerpentShrineCavernNPCs
{
    // Hydross the Unstable
    NPC_HYDROSS_FROST_FORM = 21216,
    NPC_HYDROSS_NATURE_FORM = 21232,
    NPC_PURE_SPAWN_OF_HYDROSS = 22035,
    NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

    // The Lurker Below
    NPC_COILFANG_AMBUSHER = 21865,
    NPC_COILFANG_GUARDIAN = 21873,

    // Leotheras the Blind
    NPC_INNER_DEMON = 21857,

    // Fathom-Lord Karathress
    NPC_SPITFIRE_TOTEM = 22091,
    NPC_FATHOM_LURKER = 22119,
    NPC_FATHOM_SPOREBAT = 22120,

    // Morogrim Tidewalker
    NPC_TIDEWALKER_LURKER = 21290,

};
namespace SerpentShrineCavernHelpers
{
    struct Location 
    {
        float x, y, z;
    };

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithCross(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot)
}


/* enum KarazhanSpells
{
    // Maiden of Virtue
    SPELL_REPENTANCE              = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD  = 30756,

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST       = 30004,
    SPELL_FLAME_WREATH_AURA       = 29946,
    SPELL_ARCANE_EXPLOSION        = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF         = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF       = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF        = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_GREEN_BEAM_HEAL         = 30467, // "Nether Portal - Serenity" (Netherspite aura)
    SPELL_NETHER_EXHAUSTION_RED   = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE  = 38639,
    SPELL_NETHERSPITE_BANISHED    = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH           = 30129,
    SPELL_BELLOWING_ROAR          = 36922,
    SPELL_RAIN_OF_BONES           = 37091,

    // Warlock
    SPELL_WARLOCK_BANISH          = 18647,

    // Priest
    SPELL_FEAR_WARD               =  6346,
};

enum KarazhanNPCs
{
    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN         = 15550,
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Terestian Illhoof
    NPC_KILREK                       = 17229,
    NPC_DEMON_CHAINS                 = 17248,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    NPC_VOID_ZONE                    = 16697,
    NPC_GREEN_PORTAL                 = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL                  = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                   = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL         = 17646,
};

// Attumen the Huntsman
extern std::unordered_map<uint32, time_t> attumenDPSWaitTimer;
// Big Bad Wolf
extern std::unordered_map<ObjectGuid, uint8> bigBadWolfRunIndex;
// Netherspite
extern std::unordered_map<uint32, time_t> netherspiteDPSWaitTimer;
extern std::unordered_map<ObjectGuid, time_t> redBeamMoveTimer;
extern std::unordered_map<ObjectGuid, bool> lastBeamMoveSideways;
// Nightbane
extern std::unordered_map<uint32, time_t> nightbaneDPSWaitTimer;
extern std::unordered_map<ObjectGuid, uint8> nightbaneTankStep;
extern std::unordered_map<ObjectGuid, uint8> nightbaneRangedStep;
extern std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;
extern std::unordered_map<ObjectGuid, bool> nightbaneRainOfBonesHit;

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
extern const Position KARAZHAN_NIGHTBANE_FLIGHT_STACK_POSITION;
extern const Position KARAZHAN_NIGHTBANE_RAIN_OF_BONES_POSITION;

namespace KarazhanHelpers
{

void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
void MarkTargetWithSkull(Player* bot, Unit* target);
void MarkTargetWithSquare(Player* bot, Unit* target);
void MarkTargetWithMoon(Player* bot, Unit* target);
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
bool IsMapIDTimerManager(Player* bot);
Unit* GetFirstAliveUnit(const std::vector<Unit*>& units);
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
Unit* GetNearestPlayerInRadius(Player* bot, float radius = 5.0f);
bool IsFlameWreathActive(PlayerbotAI* botAI, Player* bot);
Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss);
std::vector<Player*> GetRedBlockers(PlayerbotAI* botAI, Player* bot);
std::vector<Player*> GetBlueBlockers(PlayerbotAI* botAI, Player* bot);
std::vector<Player*> GetGreenBlockers(PlayerbotAI* botAI, Player* bot);
std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(PlayerbotAI* botAI, Player* bot);
std::vector<Unit*> GetAllVoidZones(PlayerbotAI *botAI, Player* bot);
bool IsSafePosition (float x, float y, float z, const std::vector<Unit*>& hazards, float hazardRadius);
std::vector<Unit*> GetSpawnedInfernals(PlayerbotAI* botAI);
bool IsStraightPathSafe(const Position& start, const Position& target, 
                        const std::vector<Unit*>& hazards, float hazardRadius, float stepSize);

} */

#endif
