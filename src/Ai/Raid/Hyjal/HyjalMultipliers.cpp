/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalMultipliers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "HunterActions.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "PaladinActions.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

using namespace HyjalHelpers;

float HyjalSummitDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->GetMapId() != HYJAL_MAP_ID) // Needed in case strategy isn't cleared outside
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action)) // This includes Bloodlust & Heroism
        return 1.0f;

    // Last boss first, so an instance that has progressed answers on its first lookup
    Unit* boss = nullptr;
    for (char const* name :
         { "archimonde", "azgalor", "kaz'rogal", "anetheron", "rage winterchill" })
    {
        boss = AI_VALUE2(Unit*, "find target", name);
        if (boss)
            break;
    }

    // Suppress Bloodlust/Heroism when no boss is present (trash waves). Asked only on this branch,
    // since it is the only one the answer differs on--everything else here is a dps cooldown too
    if (!boss)
    {
        return bot->getClass() == CLASS_SHAMAN &&
            (dynamic_cast<CastBloodlustAction*>(action) ||
             dynamic_cast<CastHeroismAction*>(action)) ? 0.0f : 1.0f;
    }

    // Suppress all dps cooldowns when boss is above 90% health
    return boss->GetHealthPct() > 90.0f ? 0.0f : 1.0f;
}

// Rage Winterchill

float RageWinterchillDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "rage winterchill") ? 0.0f : 1.0f;
}

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsRanged(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);

    if (!isAvoidAoe &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return 1.0f;

    // Shares its radius with the trigger that runs the maneuver action, so that action owns melee
    // movement across exactly the area this clears for it and there is no band between them
    if (!IsNearDeathAndDecay(bot, DEATH_AND_DECAY_MELEE_CONTROL_RADIUS))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    return winterchill->GetVictim() == bot || PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

// Stock avoid-aoe discards Death and Decay outright: it drops any hazard whose own radius exceeds
// AiPlayerbot.MaxAoeAvoidRadius, and at the default 15 a 20 yard pool never qualifies. Where it
// does run it flees to the raw radius, which still sits inside the aura once the target's combat
// reach is added. The hardcoded action handles both, so keep the two from fighting over the bot
float RageWinterchillRangedControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<RageWinterchillRangedGetOutOfDeathAndDecayAction*>(action))
        return 1.0f;

    // Acquiring a target is not movement. It only reads as such because AttackAction derives from
    // MovementAction, and Attack itself paths nowhere--it sets selection, faces the target, and if
    // anything stops movement. Unlike Azgalor there is no hardcoded targeting action here to spare,
    // so what this would otherwise suppress is stock "dps assist", which every dps bot runs and
    // which nothing else at this fight disables: ranged near a pool could not pick up a target at
    // all until it expired
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    // The spread settles once a bot reaches its point on the circle and then stops asking, but a
    // bot pushed off course before it ever arrives never settles and keeps trying for the rest of
    // the fight--including back into the pool it was just moved out of
    constexpr float suppressionRadius = DEATH_AND_DECAY_RADIUS + 10.0f;
    return IsNearDeathAndDecay(bot, suppressionRadius) ? 0.0f : 1.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action) != nullptr;
    if (!isTankAssist && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (isTankAssist && IsInfernalTank(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

float AnetheronAvoidAccidentalInfernalAggroMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsAoeThreatAction(bot, action))
        return 1.0f;

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return infernal->GetExactDist2d(bot) <= 20.0f ? 0.0f : 1.0f;
}

float AnetheronInfernalTargetRunToPositionMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AnetheronBringInfernalToInfernalTankAction*>(action))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron || anetheron->GetVictim() == bot)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI, bot) ? 0.0f : 1.0f;
}

float AnetheronControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAvoidAoe =
        PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action);

    if (!isTankAvoidAoe && !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

// Kaz'rogal

float KazrogalDisableDisperseAndTankFaceMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "kaz'rogal") ? 0.0f : 1.0f;
}

float KazrogalControlLowManaMovementMultiplier::GetValue(Action* action)
{
    // Hunters are excluded alongside the classes the Mark cannot reach: it reaches them, but their
    // whole answer to it is Viper, so there is no escape here to clear the way for
    if (!IsKazrogalManaUser(botAI, bot) || bot->getClass() == CLASS_HUNTER)
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

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return 1.0f;

    return botsBelowManaThreshold.contains(bot->GetGUID()) ? 0.0f : 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

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

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA ? 0.0f : 1.0f;
}

// Azgalor

float AzgalorDisableAutoTargetingAndPositioningMultiplier::GetValue(Action* action)
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

    // Still disabled in RoF, below
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "azgalor") ? 0.0f : 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (!IsDoomed(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action) ? 1.0f : 0.0f;
}

// Leave the escape action as the only thing that moves melee while Rain of Fire is a threat
float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    // Doom outranks standing in fire, and it has its own positioning to do
    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManeuverThroughFireAction*>(action))
        return 1.0f;

    // Acquiring a target is not movement. It only reads as such because AttackAction derives from
    // MovementAction, and Attack itself paths nowhere--it sets selection, faces the target, and if
    // anything stops movement. Suppressing it would leave a melee bot that entered the fire without
    // a live target unable to pick one up until the pool expired
    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    // Shares its radius with the trigger that runs the maneuver action, so that action owns melee
    // movement across exactly the area this clears for it and there is no band between them
    return IsNearRainOfFire(bot, RAIN_OF_FIRE_MELEE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Rain of Fire is 15 yards, so unlike Death and Decay it does scrape past the default
// MaxAoeAvoidRadius and stock avoid-aoe does handle it--but only out to the raw radius, which
// leaves the bot inside the aura. The hardcoded action owns this instead.
//
// The dispersal action is the other thing that moves ranged here, and it runs for the whole fight
// rather than settling like the spreads at the other bosses do. It only loses to the escape on the
// ticks the escape actually returns true, so on any tick FleePosition declines an angle it would
// be free to walk the bot back into the fire it has just left
float AzgalorRangedControlAvoidanceMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    // Doom outranks standing in fire, and it has its own positioning to do
    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action))
        return 1.0f;

    // Spared for the same reason as on the melee side: acquiring a target is not movement, it only
    // reads as such because AttackAction derives from MovementAction. Suppressing it would leave a
    // ranged bot near a pool stuck on whatever it was already hitting--unable to switch onto a
    // Doomguard as one spawns, or back onto Azgalor once it dies
    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    // Wider than the escape trigger's own radius on purpose. The band between them is where a bot
    // that has just left a pool waits it out: the dispersal action is what would otherwise move it,
    // and letting that resume at the pool's edge is the tug-of-war this exists to prevent. Nothing
    // is lost by holding it there--the dispersal trigger stops firing across the same band anyway
    constexpr float suppressionRadius = RAIN_OF_FIRE_RADIUS + 10.0f;
    return IsNearRainOfFire(bot, suppressionRadius) ? 0.0f : 1.0f;
}

// Archimonde

float ArchimondeDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}

// Leave the Doomfire avoidance as the only thing that moves a bot near a trail. Its push tapers to
// nothing at DOOMFIRE_DANGER_RADIUS, so without this anything that wants the bot elsewhere--closing
// to spell range, stock avoid-aoe on the same patches, the ranged spread--takes over the instant it
// stops being pushed, drags it back inside, and the two swap the bot every tick
float ArchimondeControlDoomfireAvoidanceMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    // Air Burst outranks Doomfire: a knockback lands the bot somewhere unpredictable anyway, and
    // its own action already reaches further than this suppression does
    if (dynamic_cast<ArchimondeAvoidDoomfireAction*>(action) ||
        dynamic_cast<ArchimondeSpreadToAvoidAirBurstAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        return 1.0f;

    // Stock avoid-aoe goes for the whole fight, not merely near a trail. Between them the hardcoded
    // actions cover both hazards here, and stock flees each trail patch to its own 6y radius--a
    // different figure, reached by a different route, pulling against the repulsion the moment the
    // two disagree about which patch matters
    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    // Wider than the radius the avoidance reacts at, so a bot pushed to the edge is still held
    return IsNearDoomfire(bot, DOOMFIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float ArchimondeSetTremorTotemMultiplier::GetValue(Action* action)
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

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() > 90.0f)
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}
