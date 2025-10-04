#include <unordered_map>
#include <ctime>

#include "RaidMagsLairMultipliers.h"
#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "WarlockActions.h"

using namespace MagsLairHelpers;
static std::unordered_map<uint32, time_t> magtheridonAggroWaitTimer;
static std::unordered_map<uint32, bool> lastAggroShadowCageState;

float MagtheridonMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)))
    {
        if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
            magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagsLairSpells::BLAST_NOVA)))
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

    if (channeler && channeler->IsAlive() && 
       (dynamic_cast<CastCurseOfAgonyAction*>(action) ||
        dynamic_cast<CastCurseOfAgonyOnAttackerAction*>(action) ||
        dynamic_cast<CastCurseOfDoomAction*>(action) ||
        dynamic_cast<CastCurseOfWeaknessAction*>(action) ||
        dynamic_cast<CastCurseOfTheElementsAction*>(action) ||
        dynamic_cast<CastCurseOfExhaustionAction*>(action)))
    {
        return 0.0f;
    }

    if ((botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1)) && 
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    // 1. Detect Shadow Cage just ended and start timer
    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagsLairSpells::SHADOW_CAGE)),
                              lastAggroShadowCageState, magtheridonAggroWaitTimer);

        // 2. Block DPS on Magtheridon for X seconds after Shadow Cage ends
        const int aggroWaitSeconds = 10;
        auto it = magtheridonAggroWaitTimer.find(bot->GetMapId());
        if (it != magtheridonAggroWaitTimer.end())
        {
            time_t since = time(nullptr) - it->second;
            if (since < aggroWaitSeconds)
            {
                if (!botAI->IsMainTank(bot) && dynamic_cast<AttackAction*>(action))
                {
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}
