#ifndef _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H

#include "Multiplier.h"

// Al'ar <Phoenix God>

class AlarMoveBetweenPlatformsMultiplier : public Multiplier
{
public:
    AlarMoveBetweenPlatformsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar move between platforms multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarDisableDisperseMultiplier : public Multiplier
{
public:
    AlarDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable disperse multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarDisableTankAssistMultiplier : public Multiplier
{
public:
    AlarDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarStayAwayFromRebirthMultiplier : public Multiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar stay away from rebirth multiplier") {}
    virtual float GetValue(Action* action);
};

class AlarPhase2NoTankingIfArmorMeltedMultiplier : public Multiplier
{
public:
    AlarPhase2NoTankingIfArmorMeltedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar phase 2 no tanking if armor melted multiplier") {}
    virtual float GetValue(Action* action);
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "void reaver maintain positions multiplier") {}
    virtual float GetValue(Action* action);
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableTankAssistMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class HighAstromancerSolarianMaintainPositionMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian maintain position multiplier") {}
    virtual float GetValue(Action* action);
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderKiteThaladredMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider kite thaladred multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderWaitForDpsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderControlMisdirectionMultiplier : public Multiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider control misdirection multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDisableShadowWardMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableShadowWardMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider disable shadow ward multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderManageTankTargetingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageTankTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider manage tank targeting multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDisableDisperseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider disable disperse multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDelayCooldownsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider delay cooldowns multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier : public Multiplier
{
public:
    KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider try nonfatal breaking of mind control multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier : public Multiplier
{
public:
    KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider all dps on boss during pyroblast multiplier") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider stay spread during gravity lapse multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
