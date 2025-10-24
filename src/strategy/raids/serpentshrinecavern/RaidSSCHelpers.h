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
    SPELL_SPOUT_VISUAL = 37431,
    SPELL_SPOUT_PERIODIC_1 = 37429,
    SPELL_SPOUT_PERIODIC_2 = 37430,

    // Leotheras the Blind
    SPELL_LEOTHERAS_BANISHED = 37546,
    SPELL_WHIRLWIND = 37640,
    SPELL_WHIRLWIND_CHANNEL = 37641,
    SPELL_METAMORPHOSIS = 37673,
    SPELL_CHAOS_BLAST = 37674,
    SPELL_INSIDIOUS_WHISPER = 37676,

    // Morogrim Tidewalker
    SPELL_WATERY_GRAVE = 38049,

    // Hunter
    SPELL_MISDIRECTION = 34477,
};

enum SerpentShrineCavernNPCs
{
    // Trash Mobs
    NPC_RANCID_MUSHROOM = 22250,

    // Hydross the Unstable
    NPC_HYDROSS_FROST_FORM = 21216,
    NPC_HYDROSS_NATURE_FORM = 21232,
    NPC_PURE_SPAWN_OF_HYDROSS = 22035,
    NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

    // The Lurker Below
    NPC_COILFANG_AMBUSHER = 21865,
    NPC_COILFANG_GUARDIAN = 21873,

    // Leotheras the Blind
    NPC_LEOTHERAS_THE_BLIND = 21215,
    NPC_GREYHEART_SPELLBINDER = 21806,
    NPC_SHADOW_OF_LEOTHERAS = 21875,
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
    // extern std::unordered_map<uint32, time_t> hydrossFrostDPSWaitTimer;
    // extern std::unordered_map<uint32, time_t> hydrossNatureDPSWaitTimer;
    extern std::unordered_map<ObjectGuid, time_t> hydrossChangeToFrostPhaseTimer;
    extern std::unordered_map<ObjectGuid, time_t> hydrossChangeToNaturePhaseTimer;
    extern std::unordered_map<uint32, time_t> leotherasHumanFormDPSWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasDemonFormDPSWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasFinalPhaseDPSWaitTimer;

    struct Location 
    {
        float x, y, z;
    };

    namespace SerpentShrineCavernLocations
    {
        extern const Location HydrossFrostTankPosition;
        extern const Location HydrossNatureTankPosition;
        // Hydross dps locations?

        // Lurker tank location?
        extern const Location LurkerCenterOfPoolPosition;

        extern const Location KarathressTankPosition;
        extern const Location TidalvessTankPosition;
        extern const Location SharkkisTankPosition;
        extern const Location CaribdisTankPosition;

        // extern const Location LeotherasHumanFormTankPosition;
        // extern const Location LeotherasDemonFormTankPosition;

        extern const Location TidewalkerPhase1TankPosition;
        extern const Location TidewalkerPhase2TankPosition;
        extern const Location TidewalkerPhase2DPSAndHealerPosition;
        // extern const Location TidewalkerGraveHealerPosition;
        // Tidewalker offtank position(s) for murlocs?
        // Tidewalker healer location for graves?
    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithCross(Player* bot, Unit* target);
    void MarkTargetWithMoon(Player* bot, Unit* target);
    std::string GetRtiName(uint8 index);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    Player* GetLeotherasDemonFormTank(PlayerbotAI* botAI, Player* bot);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);

    bool HasMarkOfHydrossAt100Percent(Player* bot);
    bool HasMarkOfCorruptionAt100Percent(Player* bot);

    Unit* GetLeotherasHuman(PlayerbotAI* botAI);
    Unit* GetPhase2LeotherasDemon(PlayerbotAI* botAI);
    Unit* GetPhase3LeotherasDemon(PlayerbotAI* botAI);
    Unit* GetActiveLeotherasDemon(PlayerbotAI* botAI);

    Player* GetCaribdisTankHealer(PlayerbotAI* botAI, Player* bot);
}

#endif
