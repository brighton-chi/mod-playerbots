#ifndef _PLAYERBOT_RAIDBLACKTEMPLEILLIDANBOSSAI_H_
#define _PLAYERBOT_RAIDBLACKTEMPLEILLIDANBOSSAI_H_

#include "ScriptedCreature.h"

// I don't think I'll need to access the boss script right now, but this template is set up just in case

enum Says
{
    SAY_ILLIDAN_MINION         = 0,
    SAY_ILLIDAN_KILL           = 1,
    SAY_ILLIDAN_TAKEOFF        = 2,
    SAY_ILLIDAN_SUMMONFLAMES   = 3,
    SAY_ILLIDAN_EYE_BLAST      = 4,
    SAY_ILLIDAN_MORPH          = 5,
    SAY_ILLIDAN_ENRAGE         = 6,
    SAY_ILLIDAN_TAUNT          = 7,
    SAY_ILLIDAN_DUPLICITY      = 8,
    SAY_ILLIDAN_UNCONVINCED    = 9,
    SAY_ILLIDAN_PREPARED       = 10,
    SAY_ILLIDAN_SHADOW_PRISON  = 11,
    SAY_ILLIDAN_CONFRONT_MAIEV = 12,
    SAY_ILLIDAN_FRENZY         = 13,
    SAY_ILLIDAN_DEFEATED       = 14,

    EMOTE_AZZINOTH_GAZE        = 0
};

enum Spells
{
    SPELL_ILLIDAN_KNEEL_INTRO           = 39656, // Aura removal does not play the full animation, using StandState instead
    SPELL_DUAL_WIELD                    = 42459,
    SPELL_BERSERK                       = 45078,
    SPELL_EMOTE_TALK_QUESTION           = 41616,
    SPELL_CLEAR_ALL_DEBUFFS             = 34098,
    SPELL_HIT_CHANCE                    = 43689,

    // Phase 1
    SPELL_FLAME_CRASH                   = 40832,
    SPELL_DRAW_SOUL                     = 40904,
    SPELL_DRAW_SOUL_HEAL                = 40903,
    SPELL_PARASITIC_SHADOWFIEND         = 41917,
    SPELL_PARASITIC_SHADOWFIEND_TRIGGER = 41914,
    SPELL_SUMMON_PARASITIC_SHADOWFIENDS = 41915,

    // Phase 2
    SPELL_THROW_GLAIVE                  = 39635,
    SPELL_THROW_GLAIVE2                 = 39849,
    SPELL_GLAIVE_RETURNS                = 39873,
    SPELL_SUMMON_GLAIVE                 = 41466,
    SPELL_FIREBALL                      = 40598,
    SPELL_DARK_BARRAGE                  = 40585,
    SPELL_EYE_BLAST                     = 39908,

    // Phase 3
    SPELL_AGONIZING_FLAMES              = 40932,
    SPELL_SUMMON_MAIEV                  = 40403,
    SPELL_SHADOW_PRISON                 = 40647,

    // Phase 4
    SPELL_DEMON_TRANSFORM_1             = 40511,
    SPELL_DEMON_TRANSFORM_2             = 40398,
    SPELL_DEMON_TRANSFORM_3             = 40510,
    SPELL_DEMON_FORM                    = 40506,
    SPELL_SHADOW_BLAST                  = 41078,
    SPELL_FLAME_BURST                   = 41126,
    SPELL_FLAME_BURST_EFFECT            = 41131,
    SPELL_SUMMON_SHADOW_DEMON           = 41117,
    SPELL_CONSUME_SOUL                  = 41080,
    SPELL_FIND_TARGET                   = 41081,

    // Phase 5
    SPELL_FRENZY                        = 40683,
    SPELL_TELEPORT_MAIEV                = 41221,
    SPELL_DEATH                         = 41218,

    // Cage
    SPELL_CAGE_TRAP                     = 40693,
    SPELL_CAGE_TRAP_PERIODIC            = 40760,
    SPELL_CAGE_TRAP_DUMMY               = 40761,
    SPELL_CAGED_DEBUFF                  = 40695,
    SPELL_CAGED_SUMMON1                 = 40696,
    SPELL_CAGED_SUMMON8                 = 40703
};

enum Misc
{
    EQUIPMENT_UNARMED                   = 0,
    EQUIPMENT_GLAIVES                   = 1,

    // Illidan
    ACTION_START_EVENT                  = 1,
    ACTION_ILLIDAN_LIFTOFF              = 2,
    ACTION_ILLIDAN_CAGED                = 3,
    ACTION_SHADOW_PRISON                = 4,
    ACTION_ILLIDAN_DIE                  = 5,
    ACTION_ILLIDAN_DEMON_TRANSFORM      = 6,
    ACTION_ILLIDAN_DEMON_TRANSFORM_BACK = 7,

    // Akama
    ACTION_ILLIDARI_COUNCIL_DONE        = 0,
    ACTION_AKAMA_MINIONS                = 1,
    ACTION_AKAMA_ENDING                 = 2,
    ACTION_AKAMA_MAIEV_DESPAWN          = 3,

    // Summons
    ACTION_MAIEV_ENDING                 = 1,
    ACTION_RETURN_BLADE                 = 2, // Sent to 22996 (Blade of Azzinoth)

    MAX_EYE_BEAM_POS                    = 4,

    POINT_ILLIDAN_TAKEOFF               = 1,
    POINT_ILLIDAN_HOVER                 = 2,
    POINT_ILLIDAN_LAND                  = 3,

    GROUP_BERSERK                       = 1,
    GROUP_PHASE_FLYING                  = 2,
    GROUP_DEMON_FORM                    = 3,

    NPC_WORLD_TRIGGER                   = 22515,
    NPC_ILLIDAN_DB_TARGET               = 23070,
    NPC_MAIEV_SHADOWSONG                = 23197,

    GO_CAGE_TRAP                        = 185916,

    PHASE_INITIAL                       = 1,
    PHASE_FLYING                        = 2,
    PHASE_LANDING                       = 3,
    PHASE_DEMON                         = 4,
    PHASE_MAIEV                         = 5
};

struct boss_illidan_stormrage : public BossAI
{
    boss_illidan_stormrage(Creature* creature) : BossAI(creature, DATA_ILLIDAN_STORMRAGE), _canTalk(true), _dying(false), _inCutscene(false), beamPosId(0) { }

    void Reset() override;
    void DoAction(int32 param) override;
    void MovementInform(uint32 type, uint32 id) override;
    void ScheduleAbilities(uint8 phase);
    void JustEngagedWith(Unit* who) override;
    void EnterEvadeMode(EvadeReason why) override;
    void JustSummoned(Creature* summon) override;
    void KilledUnit(Unit* /*victim*/) override;
    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask) override;
    void JustDied(Unit* killer) override;
    bool CanAIAttack(Unit const* target) const override;

private:
    bool _canTalk;
    bool _dying;
    bool _inCutscene;
    uint8 beamPosId;

    void CycleBeamPos(uint8 &beamPosId)
    {
        uint8 newPos;
        do {
            newPos = urand(0, MAX_EYE_BEAM_POS - 1);
        } while (newPos == beamPosId);
        beamPosId = newPos;
    }
};

#endif
