/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAMULTIPLIERS_H
#define PLAYERBOTS_ZAMULTIPLIERS_H

#include "Multiplier.h"

// General

class ZulAmanDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    ZulAmanDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "zul'aman delay dps cooldowns") {}
    float GetValue(Action* action) override;
};

class ZulAmanDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    ZulAmanDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "zul'aman disable combat formation move") {}
    float GetValue(Action* action) override;
};

class ZulAmanAvoidWhirlwindMultiplier : public Multiplier
{
public:
    ZulAmanAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "zul'aman avoid whirlwind") {}
    float GetValue(Action* action) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonStayInEyeOfTheStormMultiplier : public Multiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "akil'zon stay in eye of the storm") {}
    float GetValue(Action* action) override;
};

// Nalorakk <Bear Avatar>

class NalorakkDisableTankActionsMultiplier : public Multiplier
{
public:
    NalorakkDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nalorakk disable tank actions") {}
    float GetValue(Action* action) override;
};

class NalorakkControlMisdirectionMultiplier : public Multiplier
{
public:
    NalorakkControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nalorakk control misdirection") {}
    float GetValue(Action* action) override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiDisableTankActionsMultiplier : public Multiplier
{
public:
    JanalaiDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "jan'alai disable tank actions") {}
    float GetValue(Action* action) override;
};

class JanalaiStayAwayFromFireBombsMultiplier : public Multiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "jan'alai stay away from fire bombs") {}
    float GetValue(Action* action) override;
};

class JanalaiDoNotCrowdControlHatchersMultiplier : public Multiplier
{
public:
    JanalaiDoNotCrowdControlHatchersMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "jan'alai do not crowd control hatchers") {}
    float GetValue(Action* action) override;
};

// Halazzi <Lynx Avatar>

class HalazziDisableTankActionsMultiplier : public Multiplier
{
public:
    HalazziDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "halazzi disable tank actions") {}
    float GetValue(Action* action) override;
};

class HalazziControlMisdirectionMultiplier : public Multiplier
{
public:
    HalazziControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "halazzi control misdirection") {}
    float GetValue(Action* action) override;
};

class HalazziDisableAutoDpsTargetingMultiplier : public Multiplier
{
public:
    HalazziDisableAutoDpsTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "halazzi disable auto dps targeting") {}
    float GetValue(Action* action) override;
};

// Hex Lord Malacrass

class HexLordMalacrassUnstableAfflictionMultiplier : public Multiplier
{
public:
    HexLordMalacrassUnstableAfflictionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hex lord malacrass unstable affliction") {}
    float GetValue(Action* action) override;
};

class HexLordMalacrassSpellReflectionMultiplier : public Multiplier
{
public:
    HexLordMalacrassSpellReflectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hex lord malacrass spell reflection") {}
    float GetValue(Action* action) override;
};

// Zul'jin

class ZuljinDisableTankFaceMultiplier : public Multiplier
{
public:
    ZuljinDisableTankFaceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "zul'jin disable tank face") {}
    float GetValue(Action* action) override;
};

class ZuljinEagleDisableAvoidAoeMultiplier : public Multiplier
{
public:
    ZuljinEagleDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "zul'jin eagle disable avoid aoe") {}
    float GetValue(Action* action) override;
};

#endif
