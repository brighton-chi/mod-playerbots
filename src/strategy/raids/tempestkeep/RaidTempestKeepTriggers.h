#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "Trigger.h"

class VoidReaverPositionBossTrigger : public Trigger
{
public:
    VoidReaverPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver position boss") {}
    bool IsActive() override;
};

class VoidReaverSpreadRangedTrigger : public Trigger
{
public:
    VoidReaverSpreadRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver spread ranged") {}
    bool IsActive() override;
};

class VoidReaverArcaneOrbMoveAwayTrigger : public Trigger
{
public:
    VoidReaverArcaneOrbMoveAwayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver arcane orb move away") {}
    bool IsActive() override;
};

class HighAstromancerSolarianStackTrigger : public Trigger
{
public:
    HighAstromancerSolarianStackTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian stack") {}
    bool IsActive() override;
};

class HighAstromancerSolarianMoveAwayFromGroupTrigger : public Trigger
{
public:
    HighAstromancerSolarianMoveAwayFromGroupTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian move away from group") {}
    bool IsActive() override;
};

class HighAstromancerSolarianTargetPriestAddsTrigger : public Trigger
{
public:
    HighAstromancerSolarianTargetPriestAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian target priest adds") {}
    bool IsActive() override;
};

class HighAstromancerSolarianTankVoidwalkerTrigger : public Trigger
{
public:
    HighAstromancerSolarianTankVoidwalkerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian tank voidwalker") {}
    bool IsActive() override;
};

class HighAstromancerSolarianCastFearWardOnMainTankTrigger : public Trigger
{
public:
    HighAstromancerSolarianCastFearWardOnMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high astromancer solarian cast fear ward on main tank") {}
    bool IsActive() override;
};

#endif
