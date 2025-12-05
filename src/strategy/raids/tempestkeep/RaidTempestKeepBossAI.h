#ifndef _PLAYERBOT_RAIDTEMPESTKEEPBOSSAI_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPBOSSAI_H_

#include "ScriptedCreature.h"

// Al'ar <Phoenix God>

enum Misc
{
    POINT_PLATFORM = 0,
    POINT_QUILL = 6,
    POINT_MIDDLE = 7,
    POINT_DIVE = 8
};

struct boss_alar : public BossAI
{
    boss_alar(Creature* creature);

    void Reset() override;
    void JustReachedHome() override;
    void JustEngagedWith(Unit* who) override;
    bool CanAIAttack(Unit const* victim) const override;
    void EnterEvadeMode(EvadeReason why) override;
    void JustDied(Unit* killer) override;
    void MoveInLineOfSight(Unit* who) override;
    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageEffectType, SpellSchoolMask spellSchoolMask) override;
    void PretendToDie(Creature* creature);
    void ScheduleAbilities();
    void SpawnPhoenixes(uint8 count, Unit* targetToSpawnAt);
    void DoDiveBomb();
    void MovementInform(uint32 type, uint32 id) override;
    void ScheduleMainSpellAttack(std::chrono::seconds timer);
    void ConstructWaypointsAndMove();
    void UpdateAI(uint32 diff) override;
    Position DeterminePhoenixPosition(Position playerPosition);

    bool HasPretendedToDie() const { return _hasPretendedToDie; }

private:
    bool _hasPretendedToDie;
    bool _canAttackCooldown;
    bool _baseAttackOverride;
    bool _spawnPhoenixes;
    bool _noMelee;
    uint8 _platform;
    uint8 _platformRoll;
    uint8 _noQuillTimes;
    std::chrono::seconds _platformMoveRepeatTimer;
    TaskScheduler _transitionScheduler;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

enum KTYells
{
};

enum KTPhases
{
    PHASE_NONE                          = 0,
    PHASE_SINGLE_ADVISOR                = 1,
    PHASE_WEAPONS                       = 2,
    PHASE_TRANSITION                    = 3,
    PHASE_ALL_ADVISORS                  = 4,
    PHASE_FINAL                         = 5
};

enum KTActions
{
};

class boss_kaelthas : public BossAI
{
public:
    boss_kaelthas(Creature* creature);

    void PrepareAdvisors();
    void SetRoomState(GOState state);
    void Reset() override;
    void AttackStart(Unit* who) override;
    void MoveInLineOfSight(Unit* who) override;
    void KilledUnit(Unit* victim) override;
    void JustSummoned(Creature* summon) override;
    void SpellHit(Unit* caster, SpellInfo const* spell) override;
    void MovementInform(uint32 type, uint32 point) override;
    void ExecuteMiddleEvent();
    void IntroduceNewAdvisor(KTYells talkIntroduction, KTActions kaelAction);
    void PhaseEnchantedWeaponsExecute();
    void PhaseAllAdvisorsExecute();
    void PhaseKaelExecute();
    void UpdateAI(uint32 diff) override;
    bool CheckEvadeIfOutOfCombatArea() const override;
    void JustDied(Unit* killer) override;

    uint32 GetPhase() const { return _phase; }

private:
    uint32 _phase;
    uint8 _advisorsAlive;
    bool _transitionSceneReached = false;
};

#endif
