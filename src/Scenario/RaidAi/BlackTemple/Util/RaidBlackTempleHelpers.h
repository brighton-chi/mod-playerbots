#ifndef _PLAYERBOT_RAIDBLACKTEMPLEHELPERS_H_
#define _PLAYERBOT_RAIDBLACKTEMPLEHELPERS_H_

#include <unordered_map>

#include "RaidBlackTempleIllidanBossAI.h"
#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace BlackTempleHelpers
{
    enum BlackTempleSpells
    {
        // High Warlord Naj'entus
        SPELL_IMPALING_SPINE            = 39837,
        SPELL_TIDAL_SHIELD              = 39872,

        // Supremus
        SPELL_SNARE_SELF                = 41922,

        // Shade of Akama
        // N/A

        // Teron Gorefiend
        SPELL_SHADOW_OF_DEATH = 40251, // 55s duration
        SPELL_SPIRITUAL_VENGEANCE = 40268,

        SPELL_SPIRIT_LANCE = 40157,
        SPELL_SPIRIT_CHAINS = 40175,
        SPELL_SPIRIT_VOLLEY = 40314,
        SPELL_SPIRIT_SHIELD = 40322,
        SPELL_SPIRIT_STRIKE = 40325,

        // Gurtogg Bloodboil
        SPELL_BOSS_FEL_RAGE = 40594,
        SPELL_PLAYER_FEL_RAGE = 40604,
        SPELL_BLOODBOIL = 42005,

        // Reliquary of Souls
        SPELL_DEADEN = 41410,
        SPELL_RUNE_SHIELD               = 41431,

        // Mother Shahraz
        SPELL_FATAL_ATTRACTION              = 41001,

        // Gathios the Shatterer
        SPELL_BLESSING_OF_PROTECTION    = 41450,
        SPELL_BLESSING_OF_SPELL_WARDING = 41451,
        SPELL_JUDGEMENT                 = 41467,
        SPELL_SEAL_OF_COMMAND           = 41469,
        SPELL_CONSECRATION              = 41541,

        // Veras Darkshadow
        SPELL_VANISH                    = 41476,

        // High Nethermancer Zerevor
        SPELL_DAMPEN_MAGIC              = 41478,
        SPELL_FLAMESTRIKE               = 41481,
        SPELL_BLIZZARD                  = 41482,

        // Illidan Stormrage <The Betrayer>
        SPELL_DEMON_FIRE = 40030,
        SPELL_DEMON_FORM = 40506,
        SPELL_DARK_BARRAGE = 40585,
        SPELL_BLAZE = 40611,
        SPELL_SHADOW_PRISON             = 40647,
        SPELL_PARASITIC_SHADOWFIEND = 41917,

        // Hunter
        SPELL_MISDIRECTION              = 35079,

        // Priest
        // SPELL_FEAR_WARD              =  6346,
    };

    enum BlackTempleNPCs
    {
        // High Warlord Naj'entus
        // N/A

        // Supremus
        NPC_SUPREMUS_VOLCANO = 23085,

        // Shade of Akama
        // N/A

        // Teron Gorefiend
        NPC_SHADOWY_CONSTRUCT = 23111,

        // Gurtogg Bloodboil

        // Reliquary of Souls

        // Mother Shahraz

        // Illidari Council

        // Illidan Stormrage <The Betrayer>
        NPC_FLAME_OF_AZZINOTH = 22997,
        NPC_DEMON_FIRE = 23069,
        NPC_ILLIDAN_DB_TARGET = 23070, // For Eye Blast targeting
        NPC_BLAZE           = 23259,
        NPC_FLAME_CRASH = 23336,
        NPC_SHADOW_DEMON = 23375,
        NPC_PARASITIC_SHADOWFIEND = 23498,
    };

    enum BlackTempleItems
    {
        // High Warlord Naj'entus
        ITEM_NAJENTUS_SPINE             = 32408,
    };

    enum BlackTempleObjects
    {
        // High Warlord Naj'entus
        GO_NAJENTUS_SPINE = 185584,

        // Illidan Stormrage <The Betrayer>
        GO_CAGE_TRAP                        = 185916,
    };

    // General
    const uint32 BLACK_TEMPLE_MAP_ID = 564;
    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
    void MarkTargetWithSkull(Player* bot, Unit* target);
    void MarkTargetWithSquare(Player* bot, Unit* target);
    void MarkTargetWithStar(Player* bot, Unit* target);
    void MarkTargetWithCircle(Player* bot, Unit* target);
    void MarkTargetWithTriangle(Player* bot, Unit* target);
    void MarkTargetWithDiamond(Player* bot, Unit* target);
    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target);
    Unit* GetNearestPlayerInRadius(Player* bot, float radius);
    Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry);
    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot);

    // High Warlord Naj'entus
    extern const Position NAJENTUS_TANK_POSITION;

    // Supremus
    extern const Position SUPREMUS_TANK_POSITION;
    extern std::unordered_map<ObjectGuid, Position> supremusRangedPositions;
    extern std::unordered_map<uint32, time_t> supremusPhaseTimer;
    bool HasSupremusVolcanoNearby(PlayerbotAI* botAI, Player* bot);

    // Shade of Akama
    // N/A

    // Teron Gorefiend
    extern const Position GOREFIEND_TANK_POSITION;
    extern const Position GOREFIEND_DIE_POSITION;
    extern std::unordered_map<ObjectGuid, Position> gorefiendRangedPositions;

    // Gurtogg Bloodboil
    extern const Position GURTOGG_TANK_POSITION;
    extern const Position GURTOGG_ABSORB_BLOODBOIL_POSITION;
    extern std::unordered_map<uint32, time_t> gurtoggPhaseTimer;
    std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot);
    int GetGurtoggActiveRotationGroup(Unit* gurtogg);

    // Reliquary of Souls
    extern std::unordered_map<uint32, time_t> reliquaryDpsWaitTimer;

    // Mother Shahraz
    extern const Position SHAHRAZ_TANK_POSITION;
    extern const Position SHAHRAZ_TRANSITION_POSITION;
    extern const Position SHAHRAZ_RANGED_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> shahrazTankStep;

    // Illidari Council
    extern const Position GATHIOS_TANK_POSITION_1;
    extern const Position GATHIOS_TANK_POSITION_2;
    extern const Position GATHIOS_TANK_POSITION_3;
    extern const Position GATHIOS_TANK_POSITION_4;
    extern const Position MALANDE_TANK_POSITION;
    extern const Position ZEREVOR_TANK_POSITION;
    extern const Position ZEREVOR_HEALER_POSITION_1;
    extern const Position ZEREVOR_HEALER_POSITION_2;
    extern const Position DARKSHADOW_TANK_POSITION;
    extern std::unordered_map<uint32, time_t> councilDpsWaitTimer;
    extern std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
    extern std::unordered_map<ObjectGuid, uint8> zerevorHealStep;
    Player* GetZerevorMageTank(PlayerbotAI* botAI, Player* bot);
    bool HasDangerousCouncilAura(Unit* unit);

    // Illidan Stormrage <The Betrayer>
    extern const Position ILLIDAN_LANDING_POSITION;
    extern const Position ILLIDAN_MAIN_TANK_POSITION_1;
    extern const Position ILLIDAN_MAIN_TANK_POSITION_2;
    extern const Position ILLIDAN_MAIN_TANK_POSITION_3;
    extern const Position ILLIDAN_MAIN_TANK_POSITION_4;
    extern const Position ILLIDAN_MAIN_TANK_POSITION_5;
    extern const Position MAIN_TANK_POSITIONS[5];
    extern const Position ILLIDAN_C_GRATE_POSITION;
    extern const Position ILLIDAN_N_GRATE_POSITION;
    extern const Position ILLIDAN_S_GRATE_POSITION;
    extern const Position ILLIDAN_SW_GRATE_POSITION;
    extern const Position ILLIDAN_SE_GRATE_POSITION;
    extern const Position GRATE_POSITIONS[2];
    extern const Position ILLIDAN_E_GLAIVE_WAITING_POSITION;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_1;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_2;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_3;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_4;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_5;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_6;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_7;
    /* extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_8;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_9;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_10;
    extern const Position ILLIDAN_E_GLAIVE_TANK_POSITION_11; */
    extern const Position E_GLAIVE_TANK_POSITIONS[7];
    extern const Position ILLIDAN_W_GLAIVE_WAITING_POSITION;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_1;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_2;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_3;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_4;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_5;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_6;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_7;
    /* extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_8;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_9;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_10;
    extern const Position ILLIDAN_W_GLAIVE_TANK_POSITION_11; */
    extern const Position W_GLAIVE_TANK_POSITIONS[7];
    extern const Position ILLIDAN_P4_WARLOCK_TANK_POSITION;
    extern const Position ILLIDAN_P4_RANGED_POSITION_1;
    extern const Position ILLIDAN_P4_RANGED_POSITION_2;
    extern const Position ILLIDAN_P4_RANGED_POSITION_3;
    extern const Position ILLIDAN_P4_RANGED_POSITION_4;
    extern const Position ILLIDAN_P4_RANGED_POSITION_5;
    extern const Position ILLIDAN_P4_RANGED_POSITION_6;
    extern const Position ILLIDAN_P4_RANGED_POSITION_7;
    extern const Position ILLIDAN_P4_RANGED_POSITION_8;
    extern const Position ILLIDAN_P4_RANGED_POSITION_9;
    extern const Position ILLIDAN_P4_RANGED_POSITION_10;
    extern const Position P4_RANGED_POSITIONS[10];
    extern const Position ILLIDAN_P4_MELEE_POSITION_1;
    extern const Position ILLIDAN_P4_MELEE_POSITION_2;
    extern const Position ILLIDAN_P4_MELEE_POSITION_3;
    extern const Position ILLIDAN_P4_MELEE_POSITION_4;
    extern const Position ILLIDAN_P4_MELEE_POSITION_5;
    extern const Position P4_MELEE_POSITIONS[5];
    extern const Position eyeBeamPos[MAX_EYE_BEAM_POS * 2];
    extern std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
    extern std::unordered_map<uint32, time_t> illidanBossDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> illidanFlameDpsWaitTimer;
    extern std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
    extern std::unordered_map<uint32, ObjectGuid> westFlameGuid;
    extern std::unordered_map<ObjectGuid, uint8> illidanGrateStep;
    int GetIllidanPhase(Unit* illidan);
    std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(PlayerbotAI* botAI, Player* bot);
    Player* GetIllidanWarlockTank(PlayerbotAI* botAI, Player* bot);
    Player* GetIllidanTrapperHunter(PlayerbotAI* botAI, Player* bot);
    Player* HasParasiticShadowfiend(PlayerbotAI* botAI, Player* bot);
    struct EyeBlastDangerArea
    {
        Position start;
        Position end;
        float width;
    };
    EyeBlastDangerArea GetEyeBlastDangerArea(PlayerbotAI* botAI, Unit* illidan);
    bool IsPositionInEyeBlastDangerArea(const Position& pos, const EyeBlastDangerArea& area);
}

#endif
