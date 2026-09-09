/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WipeAction.h"

using namespace MagHelpers;
using namespace EncounterHelpers;

// When a cube clicker is in the handling phase (waiting near cube or moving to use), suppress
// movement actions that would pull them away from the cube.
float MagtheridonUseManticronCubeMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action))
    {
        return 1.0;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (!IsCubeClicker(bot))
        return 1.0f;

    auto timerIt = blastNovaTimer.find(bot->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return 1.0f;

    return getMSTimeDiff(timerIt->second, getMSTime()) >= BLAST_NOVA_INTERIM_MS ? 0.0f : 1.0f;
}

// Wait for 6 seconds after Magtheridon becomes attackable before engaging.
float MagtheridonWaitToAttackMultiplier::GetValueInEncounter(Action* action)
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
    auto it = dpsWaitTimer.find(magtheridon->GetInstanceId());
    if (it == dpsWaitTimer.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) <= dpsWaitMs ? 0.0f : 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    bool const isReachTargetSpell =
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isAvoidAoe && !isReachTargetSpell && !IsTauntAction(bot, action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (isAvoidAoe && magtheridon->GetVictim() != bot)
        return 1.0f;

    // Block the main tank from charging the assist tanks' Channelers when moving to the waiting
    // position.
    if (isReachTargetSpell && PlayerbotAI::IsMainTank(bot))
        return IsMagtheridonActive(magtheridon) ? 1.0f : 0.0f;

    return 0.0f;
}

float MagtheridonDebrisDangerMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<MagtheridonUseManticronCubeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    constexpr float debrisSuppressionZone = 15.0f;
    return IsPositionInActiveDebris(
        bot, bot->GetPositionX(), bot->GetPositionY(), debrisSuppressionZone) ? 0.0f : 1.0f;
}
