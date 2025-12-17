#ifndef _PLAYERBOT_RAIDZULAMANMULTIPLIERS_H
#define _PLAYERBOT_RAIDZULAMANMULTIPLIERS_H

#include "Multiplier.h"

// Akil'zon <Eagle Avatar>

class AkilzonDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AkilzonDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "akil'zon disable combat formation move") {}
    virtual float GetValue(Action* action);
};

class AkilzonStayInEyeOfTheStormMultiplier : public Multiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "akil'zon stay in eye of the storm") {}
    virtual float GetValue(Action* action);
};

// Nalorakk <Bear Avatar>

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

// Jan'alai <Dragonhawk Avatar>

class JanalaiDisableTankActionsMultiplier : public Multiplier
{
public:
    JanalaiDisableTankActionsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai disable tank actions") {}
    virtual float GetValue(Action* action);
};

class JanalaiDisableDispersalMultiplier : public Multiplier
{
public:
    JanalaiDisableDispersalMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai disable dispersal") {}
    virtual float GetValue(Action* action);
};

class JanalaiStayAwayFromFireBombsMultiplier : public Multiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai stay away from fire bombs") {}
    virtual float GetValue(Action* action);
};

class JanalaiDoNotCrowdControlHatchersMultiplier : public Multiplier
{
public:
    JanalaiDoNotCrowdControlHatchersMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai do not crowd control hatchers") {}
    virtual float GetValue(Action* action);
};

class JanalaiDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    JanalaiDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "jan'alai delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

// Halazzi <Lynx Avatar>

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

// Hex Lord Malacrass

class HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier : public Multiplier
{
public:
    HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "hex lord malacrass do not dispel unstable affliction") {}
    virtual float GetValue(Action* action);
};

// Zul'jin

class ZuljinDisableTankFaceMultiplier : public Multiplier
{
public:
    ZuljinDisableTankFaceMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin disable tank face") {}
    virtual float GetValue(Action* action);
};

class ZuljinAvoidWhirlwindMultiplier : public Multiplier
{
public:
    ZuljinAvoidWhirlwindMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin avoid whirlwind") {}
    virtual float GetValue(Action* action);
};

class ZuljinDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    ZuljinDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class ZuljinDoNotAvoidCyclonesMultiplier : public Multiplier
{
public:
    ZuljinDoNotAvoidCyclonesMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin do not avoid cyclones") {}
    virtual float GetValue(Action* action);
};

class ZuljinStayCloseToLynxFormMultiplier : public Multiplier
{
public:
    ZuljinStayCloseToLynxFormMultiplier(PlayerbotAI* botAI) : Multiplier(
        botAI, "zul'jin stay close to lynx form") {}
    virtual float GetValue(Action* action);
};

#endif
