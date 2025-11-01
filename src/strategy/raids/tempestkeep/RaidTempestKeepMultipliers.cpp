#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "WarriorActions.h"

float AlarPhase1StickToTheScriptMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetPositionZ() <= 17.0f)
        return 1.0f;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (bot->IsInCombat() && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) && 
            !(dynamic_cast<AlarBossTanksMoveBetweenPlatformsAction*>(action) || 
              dynamic_cast<AlarJumpFromPlatformAction*>(action)))
           return 0.0f;
    }

    /* if (dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f; */

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    if (alar && alar->GetHealth() == 0.0f &&
        (dynamic_cast<MovementAction*>(action) && 
         !dynamic_cast<AlarMoveAwayFromRebirthAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float AlarPhase2NoTankingIfArmorMeltedMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (botAI->IsTank(bot) && bot->HasAura(SPELL_MELT_ARMOR))
    {
        if (bot->IsInCombat() && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action))
           return 0.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return 1.0f;

    if (botAI->IsTank(bot) && dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot) && (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float HighAstromancerSolarianStayStackedMultiplier::GetValue(Action* action)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) || 
        dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER) && 
        !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
        return 0.0f;

    return 1.0f;
}
