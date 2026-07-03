/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethMultipliers.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ShamanActions.h"

float SethekkProphetUseTremorTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "sethekk prophet"))
        return 1.0f;

    if (dynamic_cast<CastStrengthOfEarthTotemAction*>(action) ||
        dynamic_cast<CastStoneskinTotemAction*>(action) ||
        dynamic_cast<CastStoneclawTotemAction*>(action) ||
        dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float AnzuControlSpellCastingWithSpellBombMultiplier::GetValue(Action* action)
{
    if (bot->getPowerType() != POWER_MANA || botAI->IsTank(bot))
        return 1.0f;

    constexpr uint32 SPELL_SPELL_BOMB = 40303;
    if (!bot->HasAura(SPELL_SPELL_BOMB))
        return 1.0f;

    if (botAI->IsDps(bot) && dynamic_cast<CastSpellAction*>(action))
        return 0.0f;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && mainTank->GetHealthPct() > 50.0f &&
        dynamic_cast<CastSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float TalonKingIkissDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastHeroismAction*>(action) &&
        !dynamic_cast<CastBloodlustAction*>(action))
    {
        return 1.0f;
    }

    if (Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
        ikiss && ikiss->GetHealthPct() > 95.0f)
    {
        return 0.0f;
    }

    return 1.0f;
}
