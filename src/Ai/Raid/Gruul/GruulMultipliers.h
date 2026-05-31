#ifndef _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H

#include "Multiplier.h"

class HighKingMaulgarDisableTankAssistMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable tank assist multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public Multiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar avoid whirlwind multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableArcaneShotOnKroshMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableArcaneShotOnKroshMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable arcane shot on krosh multiplier") {}
    float GetValue(Action* action) override;
};

class HighKingMaulgarDisableMageTankAoeMultiplier : public Multiplier
{
public:
    HighKingMaulgarDisableMageTankAoeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar disable mage tank aoe multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerControlTankMovementMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerControlTankMovementMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller control tank movement multiplier") {}
    float GetValue(Action* action) override;
};

class GruulTheDragonkillerStaySpreadForShatterMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerStaySpreadForShatterMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller stay spread for shatter multiplier") {}
    float GetValue(Action* action) override;
};

#endif
