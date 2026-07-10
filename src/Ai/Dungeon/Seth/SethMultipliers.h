/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_SETHMULTIPLIERS_H
#define PLAYERBOTS_SETHMULTIPLIERS_H

#include "Multiplier.h"

class SethekkProphetUseTremorTotemMultiplier : public Multiplier
{
public:
    SethekkProphetUseTremorTotemMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "sethekk prophet use tremor totem") {}
    virtual float GetValue(Action* action);
};

class AnzuControlSpellCastingWithSpellBombMultiplier : public Multiplier
{
public:
    AnzuControlSpellCastingWithSpellBombMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anzu control spell casting with spell bomb") {}
    virtual float GetValue(Action* action);
};

class TalonKingIkissDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    TalonKingIkissDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "talon king ikiss delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class TalonKingIkissControlMovementMultiplier : public Multiplier
{
public:
    TalonKingIkissControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "talon king ikiss control movement") {}
    virtual float GetValue(Action* action);
};

#endif
