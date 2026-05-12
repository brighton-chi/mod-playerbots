/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellTriggers.h"
#include "RaidSunwellBrutallusEncounter.h"
#include "RaidSunwellEredarTwinsEncounter.h"
#include "RaidSunwellFelmystEncounter.h"
#include "RaidSunwellKalecgosEncounter.h"
#include "RaidSunwellKiljaedenEncounter.h"
#include "RaidSunwellMuruEncounter.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

#include <sstream>

using namespace SunwellHelpers;

namespace
{
std::string FormatAuraDebugList(Unit* unit)
{
    if (!unit)
        return "none";

    std::ostringstream auraStream;
    bool firstAura = true;

    Unit::AuraApplicationMap const& auras = unit->GetAppliedAuras();
    for (Unit::AuraApplicationMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aura = itr->second->GetBase();
        if (!aura)
            continue;

        if (!firstAura)
            auraStream << ", ";

        auraStream << aura->GetSpellInfo()->Id << ':' << aura->GetSpellInfo()->SpellName[0];
        firstAura = false;
    }

    return firstAura ? "none" : auraStream.str();
}
}

// General

bool SunwellPlateauBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat() && bot->GetMapId() == SUNWELL_MAP_ID;
}

// Trash

bool VolatileFiendSelfDestructsWhenNearTrigger::IsActive()
{
    if (AI_VALUE2(Unit*, "find target", "kil'jaeden") ||
        AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
    {
        return false;
    }

    constexpr float searchRadius = 30.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_VOLATILE_FIEND), searchRadius, true);
}

bool ApocalypseGuardProtectedByInfernalDefenseTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST &&
           AI_VALUE2(Unit*, "find target", "apocalypse guard");
}

// Kalecgos

bool KalecgosBossEngagedByTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "kalecgos") &&
           GetKalecgosCurrentTank(botAI, bot) == bot;
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

bool KalecgosBotHasTooManyArcaneBuffetStacksTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return false;

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    Aura* arcaneBuffet =
        bot->GetAura(static_cast<uint32>(SunwellSpells::SPELL_ARCANE_BUFFET));
    return arcaneBuffet && arcaneBuffet->GetStackAmount() >= 10;
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

bool KalecgosBotsDontObserveGravityTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)) &&
           bot->GetPositionZ() > KALECGOS_SPECTRAL_REALM_Z + 3.0f &&
           bot->GetMapId() == SUNWELL_MAP_ID;
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
    return AI_VALUE2(Unit*, "find target", "brutallus") &&
           (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true));
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

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
        return false;

    return !botAI->IsMelee(bot);
}

bool BrutallusBotIsBurningTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus") ||
        botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        return false;
    }

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
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
    if (!felmyst || felmyst->IsFlying() || felmyst->GetVictim() == bot)
        return false;

    return !GetFelmystEncapsulateTarget(bot);
}

bool FelmystBossEngagedByMeleeOnGroundTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying() || felmyst->GetVictim() == bot)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    return !GetFelmystEncapsulateTarget(bot);
}

bool FelmystBotIsEncapsulatedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying() || botAI->IsMainTank(bot))
        return false;

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

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

    const FelmystGroundStack botStack =
        GetClosestFelmystGroundStack(botAI, bot, felmyst, bot);
    const FelmystGroundStack targetStack =
        GetClosestFelmystGroundStack(botAI, bot, felmyst, encapsulateTarget);

    return botStack != FelmystGroundStack::None && botStack == targetStack;
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

// Eredar Twins

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

    if (IsEredarTwinsConflagrationTarget(alythess, bot))
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

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (IsEredarTwinsConflagrationTarget(alythess, bot))
        return false;

    return !IsAlythessTank(botAI, bot);
}

bool EredarTwinsBotHasTooManyFlameTouchedStacksTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    if (botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash") &&
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        return false;
    }

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

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
    return IsEredarTwinsConflagrationTarget(alythess, bot);
}

// M'uru

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

bool MuruEntropiusHasAppearedTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "entropius") && botAI->IsMainTank(bot);
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
    if (!botAI->IsDps(bot) /* && !botAI->IsTank(bot) */)
        return false;

    return AI_VALUE2(Unit*, "find target", "m'uru") ||
           AI_VALUE2(Unit*, "find target", "entropius");
}

bool MuruVoidSentinelPulsesShadowTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (AI_VALUE2(Unit*, "find target", "void sentinel"))
        return true;

    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    return muru && muru->GetHealth() > 1;
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

bool MuruAddsSpawnAtEntranceTrigger::IsActive()
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() == 1)
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (voidSentinel && voidSentinel->GetVictim() == bot)
        return false;

    return !AI_VALUE2(Unit*, "find target", "shadowsword fury mage") &&
           !AI_VALUE2(Unit*, "find target", "shadowsword berserker");
}

bool MuruDarkFiendsSpawnedTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    return AI_VALUE2(Unit*, "find target", "dark fiend");
}

bool MuruEntropiusTurnedOutTheLightsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "entropius"))
        return false;

    constexpr float searchDistance = 15.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_DARKNESS), searchDistance, true);
}

bool MuruDarknessIsComingTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() == 1)
        return false;

    return TryGetMuruDarknessActiveState(bot, muru);
}

bool MuruTheSingularityIsNearTrigger::IsActive()
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = GetNearestMuruSingularity(bot);
    if (!singularity)
        return false;

    float safeDistance = entropius->GetVictim() == bot ? 15.0f : 10.0f;
    return bot->GetExactDist2d(singularity) < safeDistance;
}

bool MuruBerserkerIsBuffedWithFlurryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARLOCK &&
        bot->getClass() != CLASS_WARRIOR)
    {
        return false;
    }

    Unit* berserker = AI_VALUE2(Unit*, "find target", "shadowsword berserker");
    return berserker &&
           berserker->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FLURRY));
}

bool MuruFuryMageCastingFelFireballTrigger::IsActive()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_PALADIN ||
        bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_WARLOCK)
    {
        return false;
    }

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
    return FindAvailableVoidSpawnForEnslave(botAI, bot, muru, entropius);
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

bool KiljaedenHandsOfTheDeceiverAreActiveTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "hand of the deceiver");
}

bool KiljaedenItsRainingMeteorsTrigger::IsActive()
{
    if (botAI->IsTank(bot) || botAI->IsRanged(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    if (IsKiljaedenDragonController(bot, kiljaeden))
    {
        return false;
    }

    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
    auto armageddonItr = kiljaedenArmageddons.find(bot->GetInstanceId());
    if (armageddonItr == kiljaedenArmageddons.end() || armageddonItr->second.empty())
        return false;

    auto isSafePosition = [&](Position const& position)
    {
        for (KiljaedenArmageddon const& armageddon : armageddonItr->second)
        {
            if (position.GetExactDist2d(armageddon.destination.GetPositionX(),
                                        armageddon.destination.GetPositionY()) <
                armageddon.safeDistance)
            {
                return false;
            }
        }

        return true;
    };

    if (isSafePosition(KILJAEDEN_S_MELEE_POSITION) ||
        isSafePosition(KILJAEDEN_E_MELEE_POSITION))
    {
        return false;
    }

    KiljaedenArmageddon armageddon;
    return TryGetKiljaedenNearestArmageddon(bot, armageddon);
}

bool KiljaedenSaysChaosDestructionOblivionTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return false;

    Player* orbUser = GetKiljaedenDragonOrbUser(bot);
    bool const hasVengeanceOfTheBlueFlight =
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
    bool const hasPossessDrakeImmunity =
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_POSSESS_DRAKE_IMMUNITY));
    bool const isDragonController = IsKiljaedenDragonController(bot, kiljaeden);

    if (orbUser == bot && IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
    {
        Unit* controlledDragon = bot->GetCharm();
        LOG_DEBUG(
            "playerbots",
            "Kiljaeden darkness trigger: bot={} orbUser={} aura45839={} aura45838={} controller={} playerAuras=[{}] drakeAuras=[{}]",
            bot->GetName(), true, hasVengeanceOfTheBlueFlight, hasPossessDrakeImmunity,
            isDragonController, FormatAuraDebugList(bot), FormatAuraDebugList(controlledDragon));
    }

    if (isDragonController)
    {
        return false;
    }

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden);
}

bool KiljaedenBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    if (IsKiljaedenDragonController(bot, kiljaeden))
    {
        return false;
    }

    if (botAI->IsAssistTank(bot) &&
        AI_VALUE2(Unit*, "find target", "sinister reflection"))
    {
        return false;
    }

    return true;
}

bool KiljaedenBossEngagedByMeleeTrigger::IsActive()
{
    if (botAI->IsRanged(bot) || botAI->IsTank(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    if (IsKiljaedenDragonController(bot, kiljaeden))
    {
        return false;
    }

    return true;
}

bool KiljaedenBotHasFireBloomTrigger::IsActive()
{
    if (bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > 55.0f || botAI->IsMainTank(bot))
        return false;

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    return bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FIRE_BLOOM));
}

bool KiljaedenBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return false;

    if (IsKiljaedenDragonController(bot, kiljaeden))
    {
        return false;
    }

    return true;
}

bool KiljaedenDragonOrbIsActiveTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > 85.0f)
        return false;

    if (IsKiljaedenDragonController(bot, kiljaeden))
    {
        return false;
    }

    return GetKiljaedenDragonOrbUser(bot) == bot;
}

bool KiljaedenBotControlsDragonTrigger::IsActive()
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    Player* orbUser = GetKiljaedenDragonOrbUser(bot);
    bool const hasVengeanceOfTheBlueFlight =
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
    bool const hasPossessDrakeImmunity =
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_POSSESS_DRAKE_IMMUNITY));
    bool const isDragonController = IsKiljaedenDragonController(bot, kiljaeden);

    if (orbUser == bot)
    {
        Unit* controlledDragon = bot->GetCharm();
        LOG_DEBUG(
            "playerbots",
            "Kiljaeden control trigger: bot={} orbUser={} aura45839={} aura45838={} controller={} playerAuras=[{}] drakeAuras=[{}]",
            bot->GetName(), true, hasVengeanceOfTheBlueFlight, hasPossessDrakeImmunity,
            isDragonController, FormatAuraDebugList(bot), FormatAuraDebugList(controlledDragon));
    }

    return isDragonController;
}
