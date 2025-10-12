#include <unordered_map>
#include <ctime>

#include "RaidMagtheridonMultipliers.h"
#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "WarlockActions.h"

using namespace MagtheridonHelpers;

float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (magtheridon && !magtheridon->HasAura(SHADOW_CAGE))
    {
        if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
            magtheridon->FindCurrentSpellBySpellId(BLAST_NOVA))
        {
            auto it = botToCubeAssignment.find(bot->GetGUID());
            if (it != botToCubeAssignment.end())
            {
                if (dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
                {
                    return 1.0f;
                }

                return 0.0f;
            }
        }
    }

    return 1.0f;
}

float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        const int aggroWaitSeconds = 10;
        auto it = magtheridonAggroWaitTimer.find(bot->GetMapId());
        if (it != magtheridonAggroWaitTimer.end())
        {
            time_t since = time(nullptr) - it->second;
            if (since < aggroWaitSeconds)
            {
                if (!botAI->IsMainTank(bot) && 
                    (dynamic_cast<AttackAction*>(action) || 
                     (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                {
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}

float MagtheridonDisableOffTankAssistMultiplier::GetValue(Action* action)
{
    if ((botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1)) && 
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
