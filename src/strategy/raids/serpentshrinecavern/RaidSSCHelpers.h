#ifndef _PLAYERBOT_RAIDSSCHELPERS_H_
#define _PLAYERBOT_RAIDSSCHELPERS_H_

#include <ctime>
#include <unordered_map>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace SerpentShrineCavernHelpers
{
    enum SerpentShrineCavernSpells
    {
        // Trash Mob
        SPELL_TOXIC_POOL             = 38718,

        // Hydross the Unstable <Duke of Currents>
        SPELL_MARK_OF_HYDROSS_10     = 38215,
        SPELL_MARK_OF_HYDROSS_25     = 38216,
        SPELL_MARK_OF_HYDROSS_50     = 38217,
        SPELL_MARK_OF_HYDROSS_100    = 38218,
        SPELL_MARK_OF_HYDROSS_250    = 38231,
        SPELL_MARK_OF_HYDROSS_500    = 40584,
        SPELL_MARK_OF_CORRUPTION_10  = 38219,
        SPELL_MARK_OF_CORRUPTION_25  = 38220,
        SPELL_MARK_OF_CORRUPTION_50  = 38221,
        SPELL_MARK_OF_CORRUPTION_100 = 38222,
        SPELL_MARK_OF_CORRUPTION_250 = 38230,
        SPELL_MARK_OF_CORRUPTION_500 = 40583,
        SPELL_CORRUPTION             = 37961,

        // The Lurker Below
        SPELL_SPOUT_VISUAL = 37431,

        // Leotheras the Blind
        SPELL_LEOTHERAS_BANISHED     = 37546,
        SPELL_WHIRLWIND              = 37640,
        SPELL_WHIRLWIND_CHANNEL      = 37641,
        SPELL_METAMORPHOSIS          = 37673,
        SPELL_INSIDIOUS_WHISPER      = 37676,

        // Lady Vashj <Coilfang Matron>
        SPELL_FEAR_WARD              = 6346,
        SPELL_PARALYZE               = 38132,
        SPELL_POISON_BOLT            = 38253,
        SPELL_STATIC_CHARGE          = 38280,
        SPELL_ENTANGLE               = 38316,

        // Hunter
        SPELL_MISDIRECTION           = 35079,

        // Mage
        SPELL_SLOW                   = 31589,

        // Shaman
        SPELL_GROUNDING_TOTEM_EFFECT = 8178,
        SPELL_FROST_SHOCK            = 25464,

        // Warlock
        SPELL_CURSE_OF_EXHAUSTION    = 18223,

        // Item
        SPELL_HEAVY_NETHERWEAVE_NET  = 31368,
    };

    enum SerpentShrineCavernNPCs
    {
        // Trash Mobs
        NPC_WATER_ELEMENTAL_TOTEM    = 22236,
        NPC_RANCID_MUSHROOM          = 22250,

        // Hydross the Unstable <Duke of Currents>
        NPC_HYDROSS_FROST_FORM       = 21216,
        NPC_HYDROSS_NATURE_FORM      = 21232,
        NPC_PURE_SPAWN_OF_HYDROSS    = 22035,
        NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

        // Leotheras the Blind
        NPC_LEOTHERAS_THE_BLIND      = 21215,
        NPC_GREYHEART_SPELLBINDER    = 21806,
        NPC_SHADOW_OF_LEOTHERAS      = 21875,
        NPC_INNER_DEMON              = 21857,

        // Fathom-Lord Karathress
        NPC_SPITFIRE_TOTEM           = 22091,

        // Lady Vashj <Coilfang Matron>
        NPC_WORLD_INVISIBLE_TRIGGER  = 12999,
        NPC_LADY_VASHJ               = 21212,
        NPC_ENCHANTED_ELEMENTAL      = 21958,
        NPC_TAINTED_ELEMENTAL        = 22009,
        NPC_COILFANG_ELITE           = 22055,
        NPC_COILFANG_STRIDER         = 22056,
        NPC_TOXIC_SPOREBAT           = 22140,
        NPC_SPORE_DROP_TRIGGER       = 22207,
    };

    enum SerpentShrineCavernItems
    {
        // Lady Vashj <Coilfang Matron>
        ITEM_TAINTED_CORE            = 31088,

        // Tailoring
        ITEM_HEAVY_NETHERWEAVE_NET   = 24269,
    };

    extern std::unordered_map<uint32, time_t> hydrossFrostDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> hydrossNatureDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> hydrossChangeToFrostPhaseTimer;
    extern std::unordered_map<uint32, time_t> hydrossChangeToNaturePhaseTimer;

    extern std::unordered_map<uint32, time_t> lurkerSpoutTimer;
    extern std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;

    extern std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;

    extern std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    extern std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    extern std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    extern std::unordered_map<ObjectGuid, Position> vashjRangedPositions;
    extern std::unordered_map<ObjectGuid, bool> vashjHasReachedRangedPosition;
    extern std::unordered_map<ObjectGuid, Position> intendedLineup;
    extern std::unordered_map<ObjectGuid, time_t> lastImbueAttempt;
    extern std::unordered_map<uint32, time_t> lastParalyzeTime;

    namespace SerpentShrineCavernPositions
    {
        extern const Position HydrossFrostTankPosition;
        extern const Position HydrossNatureTankPosition;

        extern const Position LurkerMainTankPosition;

        extern const Position KarathressTankPosition;
        extern const Position TidalvessTankPosition;
        extern const Position SharkkisTankPosition;
        extern const Position CaribdisTankPosition;
        extern const Position CaribdisHealerPosition;
        extern const Position CaribdisRangedDpsPosition;

        extern const Position TidewalkerPhase1TankPosition;
        extern const Position TidewalkerPhaseTransitionWaypoint;
        extern const Position TidewalkerPhase2TankPosition;
        extern const Position TidewalkerPhase2RangedPosition;

        extern const Position VashjPlatformCenterPosition;
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
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    bool IsMapIDTimerManager(PlayerbotAI* botAI, Player* bot);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);

    bool HasMarkOfHydrossAt100Percent(Player* bot);
    bool HasNoMarkOfHydross(Player* bot);
    bool HasMarkOfCorruptionAt100Percent(Player* bot);
    bool HasNoMarkOfCorruption(Player* bot);

    bool IsLurkerCastingSpout(Unit* lurker);

    Unit* GetLeotherasHuman(PlayerbotAI* botAI);
    Unit* GetPhase2LeotherasDemon(PlayerbotAI* botAI);
    Unit* GetPhase3LeotherasDemon(PlayerbotAI* botAI);
    Unit* GetActiveLeotherasDemon(PlayerbotAI* botAI);
    Player* GetLeotherasDemonFormTank(PlayerbotAI* botAI, Player* bot);

    bool IsMainTankInSameSubgroup(Player* bot);
    bool IsLadyVashjInPhase1(PlayerbotAI* botAI);
    bool IsLadyVashjInPhase2(PlayerbotAI* botAI);
    bool IsLadyVashjInPhase3(PlayerbotAI* botAI);
    bool IsValidPhase2CombatNpc(Unit* unit, PlayerbotAI* botAI);
    bool AnyRecentParalyze(Group* group, uint32 mapId, uint32 graceSeconds = 3);
    Player* GetDesignatedCoreLooter(Group* group, Player* master, PlayerbotAI* botAI);
    Player* GetFirstTaintedCorePasser(Group* group, PlayerbotAI* botAI);
    Player* GetSecondTaintedCorePasser(Group* group, PlayerbotAI* botAI);
    Player* GetThirdTaintedCorePasser(Group* group, PlayerbotAI* botAI);
    Player* GetFourthTaintedCorePasser(Group* group, PlayerbotAI* botAI);

    struct GeneratorInfo
    {
        ObjectGuid guid;
        float x, y, z;
    };

    extern const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS;
    std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(Map* map, const std::vector<uint32>& generatorDbGuids);
    Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Player* bot, Unit* reference);
    const GeneratorInfo* GetNearestGeneratorToBot(Player* bot, const std::vector<GeneratorInfo>& generators);
}

#endif
