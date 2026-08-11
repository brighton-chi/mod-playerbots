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

    constexpr float suppressionRadius = DEATH_AND_DECAY_SAFE_RADIUS + 10.0f;
    if (!IsNearDeathAndDecay(bot, suppressionRadius))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    if (winterchill->GetVictim() == bot || PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    return 0.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "anetheron"))
        return 0.0f;

    return 1.0f;
}

float AnetheronControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAvoidAoe = PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action);

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

float KazrogalLowManaBotStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

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

    if (dynamic_cast<KazrogalLowManaBotTakeDefensiveMeasuresAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    if (isBelowManaThreshold.count(bot->GetGUID()))
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

    if (bot->GetPower(POWER_MANA) < 4000)
        return 0.0f;

    return 1.0f;
}

float KazrogalControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    bool const isReachTarget = dynamic_cast<ReachTargetAction*>(action);

    if (!isReachTarget && !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (isReachTarget && PlayerbotAI::IsRanged(bot))
        return 0.0f;

    return 0.0f;
}

// Azgalor

float AzgalorDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTankFace = dynamic_cast<TankFaceAction*>(action);

    if (!isTankFace &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    if (isTankFace)
        return 0.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 0.0f;

    if (AI_VALUE2(Unit*, "find target", "lesser doomguard"))
        return 0.0f;

    if (AnyGroupMemberHasDoom(bot))
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

    // Getting out of Rain of Fire is exempt as well, so waiting on the tank never means standing
    // in fire to do it
    if (dynamic_cast<AzgalorWaitAtSafePositionAction*>(action) ||
        dynamic_cast<AzgalorMeleeGetOutOfFireAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    // Walks the group, so it comes after everything cheaper. A human main tank does not drive the
    // bot tank state machine, so there is nothing to wait on
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
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    // AvoidAoeAction is a MovementAction, so the first check already covers it
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AzgalorMeleeGetOutOfFireAction*>(action))
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
