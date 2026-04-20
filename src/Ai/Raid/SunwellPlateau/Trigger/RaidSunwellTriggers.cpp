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

    if (ShouldMoveForBrutallusBurn(bot))
        return false;

    return !botAI->IsMelee(bot);
}

bool BrutallusBotIsBurningTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return false;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    if (ShouldMoveForBrutallusBurn(bot))
        return true;

    return false;
}

// Felmyst

bool FelmystPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    if (felmyst->GetHealthPct() > 95.0f)
        return true;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && felmyst->GetVictim() != mainTank)
        return true;

    return false;
}

bool FelmystBossEngagedByMainTankOnGroundTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return botAI->IsMainTank(bot);
}

bool FelmystBossEngagedByRangedOnGroundTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return !GetFelmystEncapsulateTarget(bot);
}

bool FelmystBossEngagedByMeleeOnGroundTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    return !GetFelmystEncapsulateTarget(bot);
}

bool FelmystBotIsEncapsulatedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE));
}

bool FelmystBotNearEncapsulatedPlayerTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying() || botAI->IsMainTank(bot))
        return false;

    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    return bot->GetDistance2d(
        encapsulateTarget) <= FELMYST_ENCAPSULATE_SAFE_DISTANCE;
}

bool FelmystPlayerHasGasNovaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return false;

    return GetFelmystGasNovaDispelTarget(bot) != nullptr;
}

bool FelmystBossSummonsDemonicVaporTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    if (GetFelmystDemonicVaporSummonedByBot(bot))
        return false;

    FelmystFogOfCorruptionState fogState;
    return !TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState);
}

bool FelmystBotIsDemonicVaporTargetTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    FelmystFogOfCorruptionState fogState;
    if (TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState))
        return false;

    return GetFelmystDemonicVaporSummonedByBot(bot);
}

bool FelmystFogOfCorruptionIsActiveTrigger::IsActive()
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return false;

    FelmystFogOfCorruptionState fogState;
    return TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState);
}

// Eredar Twins (Alythess & Sacrolash)

bool EredarTwinsEncounterJustStartedTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || bot->GetPositionZ() < EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") ||
           AI_VALUE2(Unit*, "find target", "lady sacrolash");
}

bool EredarTwinsPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    return alythess && alythess->GetHealthPct() > 90.0f;
}

bool EredarTwinsSacrolashEngagedByTwoTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
           IsSacrolashTank(botAI, bot);
}

bool EredarTwinsAlythessEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") &&
           IsAlythessTank(botAI, bot);
}

bool EredarTwinsBossesEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess && !AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return false;

    if (alythess && IsEredarTwinsConflagrationTarget(alythess, bot))
        return false;

    return true;
}

bool EredarTwinsOnlyOneBossRemainsTrigger::IsActive()
{
    if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return false;

    if (AI_VALUE2(Unit*, "find target", "lady sacrolash") ||
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return false;

    return !IsAlythessTank(botAI, bot);
}

bool EredarTwinsBotHasTooManyFlameTouchedStacksTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return false;

    Aura* flameSear =
        bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_SEAR));
    if (!flameSear || flameSear->GetDuration() > 2000)
        return false;

    Aura* flameTouched =
        bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED));
    return flameTouched && flameTouched->GetStackAmount() >= 5;
}

bool EredarTwinsDeterminingDpsPriorityTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess") &&
        !AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return false;

    return !IsSacrolashTank(botAI, bot) && !IsAlythessTank(botAI, bot);
}

bool EredarTwinsBotHasConflagrationTrigger::IsActive()
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    return IsEredarTwinsConflagrationTarget(alythess, bot);
}

// M'uru & Entropius

bool MuruVoidSentinelOrEntropiusHasAppearedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() > 80.0f)
        return true;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    return voidSentinel && voidSentinel->GetHealthPct() > 80.0f;
}

bool MuruBossesEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "m'uru") ||
           AI_VALUE2(Unit*, "find target", "entropius");
}

bool MuruDeterminingDpsPriorityTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "m'uru") ||
           AI_VALUE2(Unit*, "find target", "entropius");
}

bool MuruVoidSentinelNeedsTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "void sentinel");
}

bool MuruVoidSentinelCastsVoidBlastOnTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "void sentinel"))
        return false;

    if (bot->HasAura(static_cast<uint32>(
        SunwellSpells::SPELL_GROUNDING_TOTEM_EFFECT)))
    {
        return false;
    }

    return IsFirstAssistTankInSameGroup(botAI, bot);
}

bool MuruDarkFiendsSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_SHAMAN)
        return false;

    return AI_VALUE2(Unit*, "find target", "dark fiend");
}

bool MuruDarknessIsComingTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    if (botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    return TryGetMuruDarknessActiveState(bot, muru);
}

bool MuruBerserkerIsBuffedWithFlurryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARLOCK &&
        bot->getClass() != CLASS_WARRIOR)
        return false;

    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    return berserker &&
           berserker->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FLURRY));
}

bool MuruFuryMageCastingFelFireballTrigger::IsActive()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN ||
        bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_WARLOCK)
        return false;

    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage && furyMage->HasUnitState(UNIT_STATE_CASTING) &&
           furyMage->FindCurrentSpellBySpellId(
               static_cast<uint32>(SunwellSpells::SPELL_FEL_FIREBALL));
}

bool MuruFuryMageIsBuffedWithSpellFuryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* furyMage = AI_VALUE2(Unit*, "find target", "shadowsword fury mage");
    return furyMage &&
           furyMage->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPELL_FURY));
}

bool MuruVoidSpawnAvailableForEnslaveTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK || bot->GetCharm())
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    return FindAvailableVoidSpawnForEnslave(botAI, bot, muru, entropius) != nullptr;
}

bool MuruWarlockHasEnslavedVoidSpawnTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* charm = bot->GetCharm();
    if (!charm || !charm->IsAlive() ||
        charm->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN))
    {
        return false;
    }

    return AI_VALUE2(Unit*, "find target", "m'uru") ||
           AI_VALUE2(Unit*, "find target", "entropius");
}

// Kil'jaeden <The Deceiver>

bool KiljaedenTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "kil'jaeden");
}
