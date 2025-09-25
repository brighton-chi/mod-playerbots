#include <unordered_map>
#include <ctime>

#include "RaidMagsLairMultipliers.h"
#include "RaidMagsLairActions.h"
#include "RaidMagsLairHelpers.h"
#include "AttackAction.h"
#include "DruidBearActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

static std::unordered_map<uint32, time_t> magtheridonAggroWaitTimers;

float MagtheridonMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (magtheridon && magtheridon->IsAlive() && !magtheridon->HasAura(SPELL_SHADOW_CAGE))
    {
        if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
            magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
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

    // 1. Detect Shadow Cage just ended and start timer
    static bool lastShadowCage = false;
    bool shadowCage = magtheridon && magtheridon->HasAura(SPELL_SHADOW_CAGE);
    if (lastShadowCage && !shadowCage)
    {
        magtheridonAggroWaitTimers[bot->GetMapId()] = time(nullptr);
    }
    lastShadowCage = shadowCage;

    // 2. Block DPS on Magtheridon for X seconds after Shadow Cage ends
    const int aggroWaitSeconds = 10;
    auto it = magtheridonAggroWaitTimers.find(bot->GetMapId());
    if (it != magtheridonAggroWaitTimers.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < aggroWaitSeconds)
        {
            // Only allow tanks to attack
            if (!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}
