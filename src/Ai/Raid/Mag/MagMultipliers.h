/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGMULTIPLIERS_H
#define PLAYERBOTS_MAGMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Multiplier.h"
#include <string>

class MagtheridonEncounterMultiplier : public Multiplier
{
public:
    MagtheridonEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, MagHelpers::MAG_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class MagtheridonUseManticronCubeMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonUseManticronCubeMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon use manticron cube multiplier") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonWaitToAttackMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonWaitToAttackMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon wait to attack multiplier") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonControlTankActionsMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonControlTankActionsMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon control tank actions multiplier") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonDebrisDangerMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonDebrisDangerMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon debris danger multiplier") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
