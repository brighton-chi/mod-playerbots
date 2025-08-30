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
    //bool IsBetween(Unit* /*first*/, Unit* /*second*/);
    //Position GetPositionBetween(Unit* /*first*/, Unit* /*second*/);
    //Position GetPositionToRotateAroundTarget(Unit* /*target*/);
    //Position GetPositionToMoveCloserToTarget(Unit* /*target*/);
    //Position GetFlankPosition(Unit* /*target*/, float /*distance*/, bool /*rightFlank*/);
    //Unit* GetNearestUnitByEntryWithinRadius(uint32 /*entry*/, float /*radius*/);
};

#endif
