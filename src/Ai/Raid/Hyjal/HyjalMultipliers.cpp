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

    bool const isLust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

    if (!IsDpsCooldownAction(bot, action)) // This includes Bloodlust & Heroism
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "rage winterchill");

    // Suppress Bloodlust/Heroism when no boss is present (trash waves)
    if (isLust && !boss)
        return 0.0f;

    // Suppress all dps cooldowns when boss is above 90% health
    if (boss && boss->GetHealthPct() > 90.0f)
        return 0.0f;

    return 1.0f;
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

    if (AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return 0.0f;

    return 1.0f;
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

    constexpr float suppressionRadius = DEATH_AND_DECAY_RADIUS + 10.0f;
    if (!IsNearDeathAndDecay(bot, suppressionRadius))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    if (winterchill->GetVictim() == bot || PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    return 0.0f;
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

    if (AI_VALUE2(Unit*, "find target", "anetheron"))
        return 0.0f;

    return 1.0f;
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

    if (infernal->GetExactDist2d(bot) <= 20.0f)
        return 0.0f;

    return 1.0f;
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

    if (GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI, bot))
        return 0.0f;

    return 1.0f;
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

    if (AI_VALUE2(Unit*, "find target", "anetheron"))
        return 0.0f;

    return 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "anetheron"))
        return 0.0f;

    return 1.0f;
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

    if (AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 0.0f;

    return 1.0f;
}

float KazrogalControlLowManaMovementMultiplier::GetValue(Action* action)
{
    if (bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_HUNTER)
    {
        return 1.0f;
    }

    if (bot->getClass() == CLASS_DRUID &&
        (botAI->HasStrategy("bear", BOT_STATE_COMBAT) ||
         botAI->HasStrategy("cat", BOT_STATE_COMBAT)))
    {
        return 1.0f;
    }

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

    if (botsBelowManaThreshold.contains(bot->GetGUID()))
        return 0.0f;

    return 1.0f;
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

    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
        return 0.0f;

    return 1.0f;
}

// Azgalor

float AzgalorDisableAutoTargetingAndPositioningMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    // Still disabled in RoF, below
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "azgalor"))
        return 0.0f;

    return 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action))
    {
        return 1.0f;
    }

    return 0.0f;
}

// Hold melee at the safe spot until the tank has Azgalor turned away from the raid
float AzgalorMeleeWaitForTankPositioningMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AzgalorWaitAtSafePositionAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    // Don't wait if the tank is a human
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !GET_PLAYERBOT_AI(mainTank))
        return 1.0f;

    TankPositionState const tankState = GetAzgalorTankPositionState(botAI, bot);
    if (tankState == TankPositionState::Unknown ||
        tankState == TankPositionState::MovingToTransition)
    {
        return 0.0f;
    }

    return 1.0f;
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
    if (bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManueverThroughFireAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    // Has to reach further than the radius that fires the escape action, by at least the one step
    // that action takes, so a bot which has only just cleared the fire is still held back instead
    // of being walked into Azgalor's front again
    constexpr float suppressionRadius = RAIN_OF_FIRE_RADIUS + 10.0f;
    if (IsNearRainOfFire(bot, suppressionRadius))
        return 0.0f;

    return 1.0f;
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
    if (bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

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

    if (AI_VALUE2(Unit*, "find target", "archimonde"))
        return 0.0f;

    return 1.0f;
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
    if (archimonde && archimonde->GetHealthPct() < 90.0f)
        return 0.0f;

    return 1.0f;
}
