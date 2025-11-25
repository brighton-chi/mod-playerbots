#ifndef _PLAYERBOT_RAIDSSCMULTIPLIERS_H
#define _PLAYERBOT_RAIDSSCMULTIPLIERS_H

#include "Multiplier.h"

class UnderbogColossusEscapeToxicPoolMultiplier : public Multiplier
{
public:
    UnderbogColossusEscapeToxicPoolMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "underbog colossus escape toxic pool") {}
    virtual float GetValue(Action* action);
};

class HydrossTheUnstableDisableTankActionsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable disable tank actions") {}
    virtual float GetValue(Action* action);
};

class HydrossTheUnstableWaitForDpsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableWaitForDpsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable wait for dps") {}
    virtual float GetValue(Action* action);
};

class HydrossTheUnstableControlMisdirectionMultiplier : public Multiplier
{
public:
    HydrossTheUnstableControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable control misdirection") {}
    virtual float GetValue(Action* action);
};

class TheLurkerBelowStayAwayFromSpoutMultiplier : public Multiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below stay away from spout") {}
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

class LeotherasTheBlindMeleeTankMaintainDemonFormPositionMultiplier : public Multiplier
{
public:
    LeotherasTheBlindMeleeTankMaintainDemonFormPositionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind melee tank maintain demon form position") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind demon form disable melee actions") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindWaitForDpsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind wait for dps") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressDisableTankAssistMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable tank assist") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressDisableAoeMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable aoe") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressControlMisdirectionMultiplier : public Multiplier
{
public:
    FathomLordKarathressControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress control misdirection") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressWaitForDpsMultiplier : public Multiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress wait for dps") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier : public Multiplier
{
public:
    FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress caribdis tank healer maintain position") {}
    virtual float GetValue(Action* action);
};

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class MorogrimTidewalkerDisablePhase2FleeActionMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDisablePhase2FleeActionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker disable phase2 flee action") {}
    virtual float GetValue(Action* action);
};

class LadyVashjDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    LadyVashjDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public Multiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj static charge stay away from group") {}
    virtual float GetValue(Action* action);
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public Multiplier
{
public:
    LadyVashjDoNotLootTheTaintedCoreMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj do not loot the tainted core") {}
    virtual float GetValue(Action* action);
};

class LadyVashjDisableAutomaticTargetingAndMovementModifier : public Multiplier
{
public:
    LadyVashjDisableAutomaticTargetingAndMovementModifier(PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj disable automatic targeting and movement") {}
    virtual float GetValue(Action* action);
};

#endif
