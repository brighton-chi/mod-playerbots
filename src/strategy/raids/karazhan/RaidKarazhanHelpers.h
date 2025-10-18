#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

enum KarazhanSpells
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

}

#endif
