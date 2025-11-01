#ifndef _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H

#include "Multiplier.h"

class AlarPhase1StickToTheScriptMultiplier : public Multiplier
{
public:
    AlarPhase1StickToTheScriptMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar phase 1 stick to the script multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarStayAwayFromRebirthMultiplier : public Multiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar stay away from rebirth multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarPhase2NoTankingIfArmorMeltedMultiplier : public Multiplier
{
public:
    AlarPhase2NoTankingIfArmorMeltedMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar phase 2 no tanking if armor melted multiplier") {}
    virtual float GetValue(Action* action);
};

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "void reaver maintain positions multiplier") {}
    virtual float GetValue(Action* action);
};

class HighAstromancerSolarianStayStackedMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianStayStackedMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian stay stacked multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
