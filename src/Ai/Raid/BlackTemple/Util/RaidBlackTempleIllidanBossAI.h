/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDBLACKTEMPLEILLIDANBOSSAI_H_
#define _PLAYERBOT_RAIDBLACKTEMPLEILLIDANBOSSAI_H_

#include "ScriptedCreature.h"

enum Misc
{
    MAX_EYE_BEAM_POS = 4,
};

constexpr int DATA_ILLIDAN_STORMRAGE = 9;

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

    uint8 GetBeamPosId() const { return beamPosId; }

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
