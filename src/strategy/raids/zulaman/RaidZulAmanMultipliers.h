#ifndef _PLAYERBOT_RAIDZULAMANMULTIPLIERS_H
#define _PLAYERBOT_RAIDZULAMANMULTIPLIERS_H

#include "Multiplier.h"

class AkilzonStayInEyeOfTheStormMultiplier : public Multiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "akilzon stay in eye of the storm") {}
    virtual float GetValue(Action* action);
};

class NalorakkDisableTankActionsMultiplier : public Multiplier
{
public:
    NalorakkDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "nalorakk disable tank actions") {}
    virtual float GetValue(Action* action);
};

class NalorakkControlMisdirectionMultiplier : public Multiplier
{
public:
    NalorakkControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "nalorakk control misdirection") {}
    virtual float GetValue(Action* action);
};

class JanalaiDisableTankAssistMultiplier : public Multiplier
{
public:
    JanalaiDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "janalai disable tank assist") {}
    virtual float GetValue(Action* action);
};

class JanalaiStayAwayFromFireBombsMultiplier : public Multiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "janalai stay away from fire bombs") {}
    virtual float GetValue(Action* action);
};

class JanalaiDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    JanalaiDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "janalai delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class HalazziDisableTankActionsMultiplier : public Multiplier
{
public:
    HalazziDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "halazzi disable tank actions") {}
    virtual float GetValue(Action* action);
};

class HalazziControlMisdirectionMultiplier : public Multiplier
{
public:
    HalazziControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "halazzi control misdirection") {}
    virtual float GetValue(Action* action);
};

class HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier : public Multiplier
{
public:
    HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "hex lord malacrass do not dispel unstable affliction") {}
    virtual float GetValue(Action* action);
};

class ZuljinAvoidWhirlwindMultiplier : public Multiplier
{
public:
    ZuljinAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zuljin avoid whirlwind") {}
    virtual float GetValue(Action* action);
};

class ZuljinDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    ZuljinDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zuljin delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class ZuljinStayCloseToLynxFormMultiplier : public Multiplier
{
public:
    ZuljinStayCloseToLynxFormMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zuljin stay close to lynx form") {}
    virtual float GetValue(Action* action);
};

#endif
