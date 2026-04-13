/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellTriggers.h"
#include "RaidSunwellHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

// General

bool SunwellPlateauBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
}

// Kalecgos & Sathrovarr the Corruptor

bool KalecgosBossEngagedByTankTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return false;

    return GetKalecgosCurrentTank(botAI, bot) == bot;
}

bool KalecgosSpectralRiftIsOpenTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return false;

    if (!ShouldEnterKalecgosSpectralRift(botAI, bot))
        return false;

    return bot->FindNearestGameObject(
        static_cast<uint32>(SunwellObjects::GO_SPECTRAL_RIFT), 50.0f, true);
}

bool KalecgosBotsTakeSplashDamageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return false;

    return !ShouldEnterKalecgosSpectralRift(botAI, bot);
}

bool KalecgosBothBossesMustBeDefeatedTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || bot->GetVictim())
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return false;

    if (ShouldEnterKalecgosSpectralRift(botAI, bot))
        return false;

    return GetKalecgosCurrentTank(botAI, bot) != bot;
}

// Brutallus

bool BrutallusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    return brutallus && brutallus->GetHealthPct() > 95.0f;
}

bool BrutallusBossEngagedByTanksTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    return botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusBossEngagedByMeleeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    return botAI->IsMelee(bot) && !botAI->IsMainTank(bot) &&
           !botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool BrutallusBossEngagedByRangedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN_DAMAGE)))
        return false;

    return !botAI->IsMelee(bot);
}

bool BrutallusBotIsBurningTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN_DAMAGE)))
    {
        LOG_DEBUG("playerbots", "bot {} has burn", bot->GetName());
        return true;
    }

    return false;
}

// Felmyst

bool FelmystTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "felmyst");
}

// Eredar Twins (Alythess & Sacrolash)

bool EredarTwinsTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "alythess") ||
           AI_VALUE2(Unit*, "find target", "sacrolash");
}

// M'uru & Entropius

bool MuruTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "m'uru");
}

// Kil'jaeden <The Deceiver>

bool KiljaedenTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "kil'jaeden");
}
