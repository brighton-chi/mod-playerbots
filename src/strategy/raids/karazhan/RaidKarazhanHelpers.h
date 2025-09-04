#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include "AiObject.h"
#include "Playerbots.h"

class RaidKarazhanHelpers : public AiObject
{
public:
    explicit RaidKarazhanHelpers(PlayerbotAI* botAI) : AiObject(botAI) {}

    void MarkTargetWithSkull(Unit* /*target*/);
    Unit* GetFirstAliveUnit(const std::vector<Unit*>& /*units*/);
    Unit* GetFirstAliveUnitByEntry(uint32 /*entry*/);
    Unit* GetNearestPlayerInRadius(float /*radius*/ = 5.0f);
    bool IsFlameWreathActive();
    Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss);
    std::vector<Unit*> GetSpawnedInfernals() const;
};

#endif
