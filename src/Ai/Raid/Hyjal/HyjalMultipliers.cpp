/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "HunterActions.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

float HyjalDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    // The map check is for a strategy that persists outside Hyjal, such as after a server reset.
    if (botAI->GetState() == BOT_STATE_NON_COMBAT || bot->GetMapId() != HYJAL_MAP_ID)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Suppress Bloodlust/Heroism during all trash waves. It's blown on CD otherwise.
    std::optional<uint32> const bossId = GetBossInProgress(bot, HYJAL_MAP_ID);
    if (!bossId)
    {
        return bot->getClass() == CLASS_SHAMAN &&
            (dynamic_cast<CastBloodlustAction*>(action) ||
             dynamic_cast<CastHeroismAction*>(action)) ? 0.0f : 1.0f;
    }

    Unit* boss = GetHyjalBoss(bot, static_cast<HyjalBosses>(*bossId));
    return !boss || boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

float HyjalDisableDisperseAndTankFaceMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    std::optional<uint32> const bossId = GetBossInProgress(bot, HYJAL_MAP_ID);
    if (!bossId)
        return 1.0f;

    HyjalBosses const boss = static_cast<HyjalBosses>(*bossId);
    return boss == HyjalBosses::DATA_ARCHIMONDE || boss == HyjalBosses::DATA_KAZROGAL ||
        boss == HyjalBosses::DATA_RAGE_WINTERCHILL ? 0.0f : 1.0f;
}

// Rage Winterchill

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);

    if (!isAvoidAoe &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    if (!IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    Unit* winterchill = GetEncounterBoss();
    if (!winterchill || winterchill->GetVictim() == bot)
        return 1.0f;

    return PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

float RageWinterchillRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<RageWinterchillRangedGetOutOfDeathAndDecayAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return !dynamic_cast<TankAssistAction*>(action) || IsInfernalTank(bot) ? 1.0f : 0.0f;
}

// Keep non-Infernal tanks from inadvertently grabbing aggro with Consecration, Thunder Clap, etc.
float AnetheronAvoidAccidentalInfernalAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsAoeThreatAction(bot, action))
        return 1.0f;

    constexpr float holdTankAoeRadius = 20.0f; // arbitrary, but > AoE threat ability radii
    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal || infernal->GetExactDist2d(bot) > holdTankAoeRadius)
        return 1.0f;

    return IsInfernalTank(bot) ? 1.0f : 0.0f;
}

float AnetheronInfernalTargetRunToPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AnetheronBringInfernalToInfernalTankAction*>(action))
        return 1.0f;

    Unit* anetheron = GetEncounterBoss();
    if (!anetheron || anetheron->GetVictim() == bot)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI) ? 0.0f : 1.0f;
}

float AnetheronControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return dynamic_cast<SetBehindTargetAction*>(action) ? 1.0f : 0.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    return dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ? 0.0f : 1.0f;
}

// Kaz'rogal

float KazrogalControlLowManaMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsKazrogalManaUser(botAI) || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KazrogalMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* kazrogal = GetEncounterBoss();
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return 1.0f;

    return AI_VALUE(bool, "kaz'rogal below mana threshold") ? 0.0f : 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastAspectOfTheHawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheWildAction*>(action) &&
        !dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheCheetahAction*>(action) &&
        !dynamic_cast<CastAspectOfThePackAction*>(action) &&
        !dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
    {
        return 1.0f;
    }

    return bot->GetPower(POWER_MANA) <= MARK_REJOIN_MANA ? 0.0f : 1.0f;
}

// Azgalor

float AzgalorDisableAssistTargetingAndPositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    // Set behind is still disabled in Rain of Fire (in AzgalorMeleeDpsControlAvoidanceMultiplier).
    return dynamic_cast<SetBehindTargetAction*>(action) ? 1.0f : 0.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDoomed(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    return dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action) ? 1.0f : 0.0f;
}

float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManeuverThroughFireAction*>(action) ||
        dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
    {
        return 1.0f;
    }

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float AzgalorRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action) ||
        dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
    {
        return 1.0f;
    }

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Archimonde

float ArchimondeControlDoomfireAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<ArchimondeAvoidDoomfireAction*>(action) ||
        dynamic_cast<ArchimondeKeepAirBurstAwayFromTankAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float ArchimondeKeepTremorTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* archimonde = GetEncounterBoss();
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}
