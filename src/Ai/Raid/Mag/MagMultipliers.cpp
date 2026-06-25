#include <unordered_map>
#include <ctime>

#include "MagMultipliers.h"
#include "MagActions.h"
#include "MagHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "WipeAction.h"

using namespace MagtheridonHelpers;

// Don't do anything other than clicking cubes when Magtheridon is casting Blast Nova
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (!magtheridon->HasUnitState(UNIT_STATE_CASTING) ||
        !magtheridon->FindCurrentSpellBySpellId(
            static_cast<uint32>(MagtheridonSpells::SPELL_BLAST_NOVA)))
    {
        return 1.0f;
    }

    if (IsCubeClicker(bot) &&
        !dynamic_cast<MagtheridonUseManticronCubeAction*>(action) &&
        !dynamic_cast<MagtheridonManageTimersAndAssignmentsAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Bots will wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return 1.0f;

    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot))
        return 1.0f;

    constexpr time_t dpsWaitSeconds = 6;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it != dpsWaitTimer.end() && time(nullptr) - it->second > dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<CastSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) || bot->GetVictim() == nullptr)
        return 1.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;

        if (!IsMagtheridonActive(magtheridon) &&
            dynamic_cast<CastReachTargetSpellAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
