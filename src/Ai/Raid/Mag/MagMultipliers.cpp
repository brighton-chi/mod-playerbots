/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace MagHelpers;

// When a cube clicker is in the handling phase (waiting near cube or moving
// to use), suppress movement actions that would pull them away from the cube
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action))
    {
        return 1.0;
    }

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return 1.0f;

    if (!IsCubeClicker(bot))
        return 1.0f;

    auto timerIt = blastNovaTimer.find(bot->GetMap()->GetInstanceId());
    if (timerIt != blastNovaTimer.end() &&
        getMSTimeDiff(timerIt->second, getMSTime()) >= BLAST_NOVA_INTERIM_MS)
    {
        return 0.0f;
    }

    return 1.0f;
}

// Wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    constexpr uint32 dpsWaitMs = 6 * IN_MILLISECONDS;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it == dpsWaitTimer.end() || getMSTimeDiff(it->second, getMSTime()) <= dpsWaitMs)
        return 0.0f;

    return 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isTaunt = IsTauntAction(bot, action);
    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    bool const isReachTargetSpell =
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isTaunt && !isAvoidAoe && !isReachTargetSpell &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action))
    {
        return 1.0f;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (isAvoidAoe && magtheridon->GetVictim() != bot)
        return 1.0f;

    if (isReachTargetSpell && !PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (GetChanneler(bot, NORTHWEST_CHANNELER) ||
        GetChanneler(bot, NORTHEAST_CHANNELER))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MagtheridonDebrisDangerMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return 1.0f;

    constexpr float debrisSuppressionZone = 15.0f;
    if (IsPositionInActiveDebris(
            bot, bot->GetPositionX(), bot->GetPositionY(), debrisSuppressionZone))
    {
        return 0.0f;
    }

    return 1.0f;
}
