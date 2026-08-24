/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACMultipliers.h"
#include "ACActions.h"
#include "ACTriggers.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

// Shirrak the Dead Watcher

// Flee from Focus Fire and dont run back in
float ShirrakFleeFocusFireMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "shirrak the dead watcher"))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    bool const isMovementAction = dynamic_cast<MovementAction*>(action);
    bool const isReachTargetSpell = dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!isMovementAction && !isReachTargetSpell)
        return 1.0f;

    if (dynamic_cast<ShirrakFleeFocusFireAction*>(action))
        return 1.0f;

    Creature* flare = bot->FindNearestCreature(
        static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f, true);

    if (flare)
    {
        if (isReachTargetSpell)
            return 0.0f;

        float currentDistance = bot->GetDistance2d(flare);
        constexpr float safeDistance = 12.0f;
        constexpr float buffer = 5.0f;

        if (isMovementAction && currentDistance < safeDistance + buffer)
            return 0.0f;
    }

    return 1.0f;
}
