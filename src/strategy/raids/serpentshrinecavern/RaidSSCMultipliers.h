#ifndef _PLAYERBOT_RAIDSSCMULTIPLIERS_H
#define _PLAYERBOT_RAIDSSCMULTIPLIERS_H

#include "Multiplier.h"

class HydrossTheUnstableDisableTankAssistMultiplier : public Multiplier
{
public:
    HydrossTheUnstableDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable disable tank assist") {}
    virtual float GetValue(Action* action);
};

/* class HydrossTheUnstableWaitForDPSMultiplier : public Multiplier
{
public:
    HydrossTheUnstableWaitForDPSMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable wait for dps") {}
    virtual float GetValue(Action* action);
}; */

class TheLurkerBelowStayBehindBossDuringSpoutMultiplier : public Multiplier
{
public:
    TheLurkerBelowStayBehindBossDuringSpoutMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below stay behind boss during spout") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindAvoidWhirlwindMultiplier : public Multiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind avoid whirlwind") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDisableTankActionsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind disable tank actions") {}
    virtual float GetValue(Action* action);
};

/* class LeotherasTheBlindMainTankMaintainDemonFormPositionMultiplier : public Multiplier
{
public:
    LeotherasTheBlindMainTankMaintainDemonFormPositionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind main tank maintain demon form position") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind demon form disable melee actions") {}
    virtual float GetValue(Action* action);
}; */

class LeotherasTheBlindWaitForDPSMultiplier : public Multiplier
{
public:
    LeotherasTheBlindWaitForDPSMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind wait for dps") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindWaitForBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    LeotherasTheBlindWaitForBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind wait for bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressDisableTankAssistMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable tank assist") {}
    virtual float GetValue(Action* action);
};

#endif
