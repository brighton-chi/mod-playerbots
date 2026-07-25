/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKMULTIPLIERS_H
#define PLAYERBOTS_TKMULTIPLIERS_H

#include "Multiplier.h"

// Al'ar <Phoenix God>

class AlarMoveBetweenPlatformsMultiplier : public Multiplier
{
public:
    AlarMoveBetweenPlatformsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar move between platforms") {}
    virtual float GetValue(Action* action);
};

class AlarDisableDisperseMultiplier : public Multiplier
{
public:
    AlarDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable disperse") {}
    virtual float GetValue(Action* action);
};

class AlarDisableAutomaticTargetingMultiplier : public Multiplier
{
public:
    AlarDisableAutomaticTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar disable automatic targeting") {}
    virtual float GetValue(Action* action);
};

class AlarStayAwayFromRebirthMultiplier : public Multiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar stay away from rebirth") {}
    virtual float GetValue(Action* action);
};

class AlarDontTauntBossIfArmorMeltedMultiplier : public Multiplier
{
public:
    AlarDontTauntBossIfArmorMeltedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "al'ar don't taunt boss if armor melted") {}
    virtual float GetValue(Action* action);
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public Multiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "void reaver maintain positions") {}
    virtual float GetValue(Action* action);
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableMeleeTargetingMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianDisableMeleeTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian disable melee targeting") {}
    virtual float GetValue(Action* action);
};

class HighAstromancerSolarianMaintainPositionMultiplier : public Multiplier
{
public:
    HighAstromancerSolarianMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high astromancer solarian maintain position") {}
    virtual float GetValue(Action* action);
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderWaitForDpsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider wait for dps") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderKiteThaladredMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider kite thaladred") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderControlMisdirectionMultiplier : public Multiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider control misdirection") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderKeepDistanceFromCapernianMultiplier : public Multiplier
{
public:
    KaelthasSunstriderKeepDistanceFromCapernianMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider keep distance from capernian") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderManageWeaponTankingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageWeaponTankingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider manage weapon tanking") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderSuppressEquipUpgradeMultiplier : public Multiplier
{
public:
    KaelthasSunstriderSuppressEquipUpgradeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider suppress equip upgrade") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderManageAutomaticTargetingMultiplier : public Multiplier
{
public:
    KaelthasSunstriderManageAutomaticTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider manage automatic targeting") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDisableDisperseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider disable disperse") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderDelayCooldownsMultiplier : public Multiplier
{
public:
    KaelthasSunstriderDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider delay cooldowns") {}
    virtual float GetValue(Action* action);
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier : public Multiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kael'thas sunstrider stay spread during gravity lapse") {}
    virtual float GetValue(Action* action);
};

#endif
