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

class AnetheronInfernalTargetRunToPositionMultiplier : public Multiplier
{
public:
    AnetheronInfernalTargetRunToPositionMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anetheron infernal target run to position") {}
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

class KazrogalDisableDisperseAndTankFaceMultiplier : public Multiplier
{
public:
    KazrogalDisableDisperseAndTankFaceMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal disable disperse and tank face") {}
    float GetValue(Action* action) override;
};

class KazrogalControlLowManaMovementMultiplier : public Multiplier
{
public:
    KazrogalControlLowManaMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal control low mana movement") {}
    float GetValue(Action* action) override;
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public Multiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "kaz'rogal keep aspect of the viper active") {}
    float GetValue(Action* action) override;
};

// Azgalor

class AzgalorDisableAutoTargetingAndPositioningMultiplier : public Multiplier
{
public:
    AzgalorDisableAutoTargetingAndPositioningMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor disable auto targeting and positioning") {}
    float GetValue(Action* action) override;
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public Multiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "azgalor doomed bot prioritize positioning") {}
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
