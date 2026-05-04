/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellMultipliers.h"
#include "RaidSunwellActions.h"
#include "RaidSunwellBrutallusEncounter.h"
#include "RaidSunwellEredarTwinsEncounter.h"
#include "RaidSunwellFelmystEncounter.h"
#include "RaidSunwellKalecgosEncounter.h"
#include "RaidSunwellKiljaedenEncounter.h"
#include "RaidSunwellMuruEncounter.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "TargetValue.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PartyMemberToDispel.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

using namespace SunwellHelpers;

static bool IsDpsCooldownAction(Action* action)
{
    return dynamic_cast<CastHeroismAction*>(action) ||
           dynamic_cast<CastBloodlustAction*>(action) ||
           dynamic_cast<CastMetamorphosisAction*>(action) ||
           dynamic_cast<CastAdrenalineRushAction*>(action) ||
           dynamic_cast<CastBladeFlurryAction*>(action) ||
           dynamic_cast<CastIcyVeinsAction*>(action) ||
           dynamic_cast<CastColdSnapAction*>(action) ||
           dynamic_cast<CastArcanePowerAction*>(action) ||
           dynamic_cast<CastPresenceOfMindAction*>(action) ||
           dynamic_cast<CastCombustionAction*>(action) ||
           dynamic_cast<CastRapidFireAction*>(action) ||
           dynamic_cast<CastReadinessAction*>(action) ||
           dynamic_cast<CastAvengingWrathAction*>(action) ||
           dynamic_cast<CastElementalMasteryAction*>(action) ||
           dynamic_cast<CastFeralSpiritAction*>(action) ||
           dynamic_cast<CastFireElementalTotemAction*>(action) ||
           dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
           dynamic_cast<CastForceOfNatureAction*>(action) ||
           dynamic_cast<CastArmyOfTheDeadAction*>(action) ||
           dynamic_cast<CastSummonGargoyleAction*>(action) ||
           dynamic_cast<CastBerserkingAction*>(action) ||
           dynamic_cast<CastBloodFuryAction*>(action);
}

// Kalecgos

float KalecgosControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        return 1.0f;
    }

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float KalecgosWaitToDecurseMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_SHAMAN)
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        return 1.0f;
    }

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_CURSE);
    if (!target)
        return 1.0f;

    Aura* aura = target->GetAura(
        static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!aura || aura->GetDuration() < 10000)
        return 1.0f;

    if (dynamic_cast<CastRemoveCurseAction*>(action) ||
        dynamic_cast<CastCleanseSpiritAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) ||
        dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        return 1.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        (dynamic_cast<CombatFormationMoveAction*>(action) &&
         !dynamic_cast<SetBehindTargetAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosRestrictTauntMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)) ||
        !AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        return 1.0f;
    }

    if (GetKalecgosCurrentTank(botAI, bot) == bot)
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosDelayCooldownsForSathrovarrMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)))
        return 1.0f;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Brutallus

float BrutallusControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "brutallus"))
    {
        return 1.0f;
    }

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float BrutallusControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    if (bot->getClass() == CLASS_ROGUE &&
        bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)) &&
        dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float BrutallusNoTankingWithTooManyMeteorStacksMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) || !AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
    {
        return 0.0f;
    }

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float BrutallusDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus || brutallus->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    if (!GetFelmystEncapsulateTarget(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FelmystFogOfCorruptionState fogState;
    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, fogState))
        return 1.0f;

    FelmystFogOfCorruptionState activeFogState;
    bool isActiveFog = TryGetActiveFelmystFogOfCorruptionState(
        bot, felmyst, activeFogState);

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    std::array<Position, 3> destinations;
    uint8 destinationCount = 0;
    bool needsShift = TryGetFelmystFogSafeDestinations(
        bot, isActiveFog ? activeFogState.lane :
        fogState.lane, destinations, destinationCount);

    if (isActiveFog && needsShift && dynamic_cast<CastSpellAction*>(action) &&
        !dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action))
    {
        if (dynamic_cast<FelmystAvoidFogOfCorruptionAction*>(action))
            return needsShift ? 1.0f : 0.0f;

        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeDemonicVaporKiteMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FelmystFogOfCorruptionState fogState;
    if (TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState))
        return 1.0f;

    if (!GetFelmystDemonicVaporSummonedByBot(bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<FelmystKiteDemonicVaporAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Eredar Twins

float EredarTwinsMeleeJumpDownFromBalconyMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMelee(bot) || bot->GetPositionZ() < EREDAR_TWINS_BALCONY_Z ||
        !AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        return 1.0f;
    }

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<EredarTwinsMeleeJumpDownFromBalconyAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        return 1.0f;
    }

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValue(Action* action)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    bool shouldSuppressThreat = sacrolash &&
         ShouldHoldSacrolashThreat(botAI, bot, alythess, sacrolash);

    if (!shouldSuppressThreat)
        return 1.0f;

    Unit* actionTarget = action->GetTarget();
    if (dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<EredarTwinsDpsPrioritizeLadySacrolashAction*>(action) &&
        (actionTarget == sacrolash || bot->GetVictim() == sacrolash))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastSpellAction*>(action) && actionTarget == sacrolash)
        return 0.0f;

    return 1.0f;
}

float EredarTwinsDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        return 1.0f;
    }

    if (dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsRoguesStayStackedMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE ||
        !AI_VALUE2(Unit*, "find target", "grand warlock alythess") ||
        AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValue(Action* action)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess && !AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    if (IsEredarTwinsConflagrationTarget(alythess, bot) &&
        (dynamic_cast<CastReachTargetSpellAction*>(action) ||
         (dynamic_cast<MovementAction*>(action) &&
          !dynamic_cast<EredarTwinsConflagratedBotMoveFromGroupAction*>(action))))
    {
        return 0.0f;
    }

    if ((botAI->IsRanged(bot) || IsAlythessTank(botAI, bot)) &&
        (dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!alythess || alythess->GetHealthPct() < 80.0f ||
        !sacrolash || sacrolash->GetHealthPct() < 80.0f)
    {
        return 1.0f;
    }

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru && !AI_VALUE2(Unit*, "find target", "entropius"))
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    constexpr float searchRadius = 40.0f;
    Unit* voidSpawn = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), searchRadius);
    if (voidSpawn &&
        (bot->GetTarget() == voidSpawn->GetGUID() || bot->GetVictim() == voidSpawn) &&
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 0.0f;
    }

    if (muru &&
        (bot->GetTarget() == muru->GetGUID() || bot->GetVictim() == muru))
    {
        context->GetValue<bool>("neglect threat")->Set(true);
    }

    if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel") &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MuruControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "m'uru"))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MuruControlMovementMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru && !AI_VALUE2(Unit*, "find target", "entropius"))
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        if (muru && TryGetMuruDarknessEarlyState(bot, muru) &&
            (dynamic_cast<CastReachTargetSpellAction*>(action) ||
             dynamic_cast<ReachTargetAction*>(action)))
        {
            return 0.0f;
        }

        return 1.0f;
    }

    if (muru && TryGetMuruDarknessActiveState(bot, muru) &&
        (dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MuruUseOnlyGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN ||
        !AI_VALUE2(Unit*, "find target", "void sentinel") ||
        !IsFirstAssistTankInSameGroup(botAI, bot))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastWindfuryTotemAction*>(action) ||
        dynamic_cast<SetWindfuryTotemAction*>(action) ||
        dynamic_cast<CastWrathOfAirTotemAction*>(action) ||
        dynamic_cast<SetWrathOfAirTotemAction*>(action) ||
        dynamic_cast<CastNatureResistanceTotemAction*>(action) ||
        dynamic_cast<SetNatureResistanceTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MuruDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() == 1)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (muru->GetHealthPct() < 98.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if ((!kiljaeden && !AI_VALUE2(Unit*, "find target", "hand of the deceiver")) ||
        (kiljaeden && kiljaeden->GetHealthPct() < 25.0f)) // Phase 5
    {
        return 1.0f;
    }

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (kiljaeden && kiljaeden->GetHealthPct() < 85.0f) // Phase 3
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot) && bot->GetVictim() != nullptr &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KiljaedenPrioritizeArmageddonAvoidanceMultiplier::GetValue(Action* action)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
