/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethTriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace
{
constexpr uint32 SETHEKK_HALLS_MAP_ID = 556;
constexpr uint32 SPELL_ARCANE_BUBBLE = 9438;
}

bool TimeLostControllerDropsCharmingTotemTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, SETHEKK_HALLS_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "time-lost controller");
}

bool SethekkProphetCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    return !AI_VALUE2(bool, "has totem", "tremor totem") &&
        AI_VALUE2(Unit*, "find target", "sethekk prophet");
}

bool DarkweaverSythBossSummonsElementalsTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, SETHEKK_HALLS_MAP_ID))
        return false;

    Unit* syth = AI_VALUE2(Unit*, "find target", "darkweaver syth");
    return syth && syth->GetHealthPct() > 10.0f;
}

bool AnzuEncounterHasTwoPhasesTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "anzu");
}

bool AnzuBirdSpiritsProvideBuffsTrigger::IsActive()
{
    return bot->getClass() == CLASS_DRUID && botAI->IsCaster(bot) &&
        AI_VALUE2(Unit*, "find target", "anzu");
}

bool TalonKingIkissNeedToPositionForArcaneExplosionTrigger::IsActive()
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->GetVictim() == bot;
}

bool TalonKingIkissBossPreparingToCastArcaneExplosionTrigger::IsActive()
{
    // Arcane Bubble is put up 1s before casting Arcane Explosion
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    return ikiss && ikiss->HasAura(SPELL_ARCANE_BUBBLE) && bot->IsWithinLOSInMap(ikiss);
}
