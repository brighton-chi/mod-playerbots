/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidSunwellMultipliers.h"
#include "RaidSunwellActions.h"
#include "RaidSunwellHelpers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
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

// Kalecgos & Sathrovarr the Corruptor

float KalecgosControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return 1.0f;

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float KalecgosWaitToDecurseMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_DRUID &&
        bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_CURSE);
    if (!target)
        return 1.0f;

    Aura* aura = target->GetAura(static_cast<uint32>(
        SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!aura || aura->GetDuration() < 10000)
        return 1.0f;

    if (dynamic_cast<CastRemoveCurseAction*>(action) ||
        dynamic_cast<CastCleanseSpiritAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) ||
        dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KalecgosDelayCooldownsForSathrovarrMultiplier::GetValue(Action* action)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos || kalecgos->GetHealthPct() < 40.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action))
        return 0.0f;

    if (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Brutallus

float BrutallusControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

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
        return 0.0f;

    return 1.0f;
}

float BrutallusNoTankingWithTooManyMeteorStacksMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) || !AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
        return 0.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float BrutallusDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus || brutallus->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action))
        return 0.0f;

    if (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Felmyst

float FelmystMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
        return 1.0f;

    return 1.0f;
}

// Eredar Twins (Alythess & Sacrolash)

float EredarTwinsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    return 1.0f;
}

// M'uru & Entropius

float MuruMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return 1.0f;

    return 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    return 1.0f;
}
