#ifndef _PLAYERBOT_RAIDZULAMANZULJINBOSSAI_H_
#define _PLAYERBOT_RAIDZULAMANZULJINBOSSAI_H_

#include "ScriptedCreature.h"

constexpr int DATA_ZULJIN = 5;

struct boss_zuljin : public BossAI
{
    boss_zuljin(Creature* creature) : BossAI(creature, DATA_ZULJIN) { };

    void Reset() override;
    void JustEngagedWith(Unit* /*who*/) override;
    void EnterEvadeMode(EvadeReason /*why*/) override;
    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override;
    void KilledUnit(Unit* /*victim*/) override;
    ObjectGuid GetGUID(int32 index) const override;
    void JustDied(Unit* /*killer*/) override;
    void SpawnAdds();
    void MovementInform(uint32 type, uint32 id) override;
    void EnterPhase(uint32 NextPhase);

    ObjectGuid GetChargeTarget() const { return _chargeTargetGUID; } // This is the only addition to the existing class

private:
    ObjectGuid _chargeTargetGUID;
    uint8 _nextPhase;
};

#endif
