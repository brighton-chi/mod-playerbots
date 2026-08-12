/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALMULTIPLIERS_H
#define PLAYERBOTS_HYJALMULTIPLIERS_H

#include "Multiplier.h"

class HyjalSummitDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    HyjalSummitDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hyjal summit delay dps cooldowns") {}
    float GetValue(Action* action) override;
};

// Rage Winterchill

class RageWinterchillDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    RageWinterchillDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "rage winterchill disable combat formation move") {}
    float GetValue(Action* action) override;
};

class RageWinterchillMeleeControlAvoidanceMultiplier : public Multiplier
{
public:
    RageWinterchillMeleeControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "rage winterchill melee control avoidance") {}
    float GetValue(Action* action) override;
};

class RageWinterchillRangedControlAvoidanceMultiplier : public Multiplier
{
public:
    RageWinterchillRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "rage winterchill ranged control avoidance") {}
    float GetValue(Action* action) override;
};

// Anetheron

class AnetheronDisableAssistTargetingMultiplier : public Multiplier
{
public:
    AnetheronDisableAssistTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anetheron disable assist targeting") {}
    float GetValue(Action* action) override;
};

class AnetheronAvoidAccidentalInfernalAggroMultiplier : public Multiplier
{
public:
    AnetheronAvoidAccidentalInfernalAggroMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anetheron avoid accidental infernal aggro") {}
    float GetValue(Action* action) override;
};

class AnetheronControlMovementMultiplier : public Multiplier
{
public:
    AnetheronControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anetheron control movement") {}
    float GetValue(Action* action) override;
};

class AnetheronControlMisdirectionMultiplier : public Multiplier
{
public:
    AnetheronControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anetheron control misdirection") {}
    float GetValue(Action* action) override;
};

// Kaz'rogal

class KazrogalLowManaBotStayAwayFromGroupMultiplier : public Multiplier
{
public:
    KazrogalLowManaBotStayAwayFromGroupMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal low mana bot stay away from group") {}
    float GetValue(Action* action) override;
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public Multiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal keep aspect of the viper active") {}
    float GetValue(Action* action) override;
};

class KazrogalControlMovementMultiplier : public Multiplier
{
public:
    KazrogalControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal control movement") {}
    float GetValue(Action* action) override;
};

// Azgalor

class AzgalorDisableTankActionsMultiplier : public Multiplier
{
public:
    AzgalorDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor disable tank actions") {}
    float GetValue(Action* action) override;
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public Multiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor doomed bot prioritize positioning") {}
    float GetValue(Action* action) override;
};

class AzgalorMeleeWaitForTankPositioningMultiplier : public Multiplier
{
public:
    AzgalorMeleeWaitForTankPositioningMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor melee wait for tank positioning") {}
    float GetValue(Action* action) override;
};

class AzgalorMeleeDpsControlAvoidanceMultiplier : public Multiplier
{
public:
    AzgalorMeleeDpsControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor melee dps control avoidance") {}
    float GetValue(Action* action) override;
};

class AzgalorRangedControlAvoidanceMultiplier : public Multiplier
{
public:
    AzgalorRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor ranged control avoidance") {}
    float GetValue(Action* action) override;
};

// Archimonde

class ArchimondeDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    ArchimondeDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "archimonde disable combat formation move") {}
    float GetValue(Action* action) override;
};

class ArchimondeSetTremorTotemMultiplier : public Multiplier
{
public:
    ArchimondeSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "archimonde set tremor totem") {}
    float GetValue(Action* action) override;
};

#endif
