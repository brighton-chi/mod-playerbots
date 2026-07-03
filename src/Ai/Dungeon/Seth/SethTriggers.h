/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHTRIGGERS_H
#define PLAYERBOTS_SETHTRIGGERS_H

#include "Trigger.h"

class TimeLostControllerDropsCharmingTotemTrigger : public Trigger
{
public:
    TimeLostControllerDropsCharmingTotemTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "time-lost controller drops charming totem") {}
    bool IsActive() override;
};

class SethekkProphetCastsFearTrigger : public Trigger
{
public:
    SethekkProphetCastsFearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "sethekk prophet casts fear") {}
    bool IsActive() override;
};

class DarkweaverSythSummonsElementalsTrigger : public Trigger
{
public:
    DarkweaverSythSummonsElementalsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "darkweaver syth summons elementals") {}
    bool IsActive() override;
};

class AnzuEncounterHasTwoPhasesTrigger : public Trigger
{
public:
    AnzuEncounterHasTwoPhasesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anzu encounter has two phases") {}
    bool IsActive() override;
};

class AnzuBirdSpiritsProvideBuffsTrigger : public Trigger
{
public:
    AnzuBirdSpiritsProvideBuffsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "anzu bird spirits provide buffs") {}
    bool IsActive() override;
};

class TalonKingIkissNeedToPositionForArcaneExplosionTrigger : public Trigger
{
public:
    TalonKingIkissNeedToPositionForArcaneExplosionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "talon king ikiss need to position for arcane explosion") {}
    bool IsActive() override;
};

class TalonKingIkissBossPreparingToCastArcaneExplosionTrigger : public Trigger
{
public:
    TalonKingIkissBossPreparingToCastArcaneExplosionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "talon king ikiss boss preparing to cast arcane explosion") {}
    bool IsActive() override;
};

#endif
