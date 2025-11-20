#ifndef _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H

#include "Multiplier.h"

class AlarDisableTankAssistMultiplier : public Multiplier
{
public:
    AlarDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar disable tank assist multiplier") {}
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

class KaelthasSunstriderWaitForDpsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderControlMisdirectionMultiplier : public Multiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider control misdirection multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDisableTankAssistMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier : public Multiplier
{
public:
    KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider try nonfatal breaking of mind control multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier : public Multiplier
{
public:
    KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas sunstrider all dps on boss during pyroblast multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
