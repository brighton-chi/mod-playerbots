#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "WarriorActions.h"

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float AlarPhase1StickToTheScriptMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    uint32 mapId = alar->GetMapId();
    if (isPhase2[mapId])
        return 1.0f;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (bot->IsInCombat() && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        /* if (dynamic_cast<MovementAction*>(action) &&
            !(dynamic_cast<AlarBossTanksMoveBetweenPlatformsAction*>(action) ||
              dynamic_cast<AlarJumpFromPlatformAction*>(action)))
           return 0.0f; */
    }

    /* if (botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        if (IsChargeAction(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action))
            return 0.0f;
    } */

    /* if (botAI->IsMelee(bot) && botAI->IsDps(bot) && (dynamic_cast<SetBehindTargetAction*>(action) || dynamic_cast<ReachTargetAction*>(action)))
        return 0.0f; */

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (alar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
        (dynamic_cast<MovementAction*>(action) &&
         !(dynamic_cast<AlarMoveAwayFromRebirthAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))))
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

    return 1.0f;
}

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return 1.0f;

    if (botAI->IsTank(bot))
    {
        dynamic_cast<TankFaceAction*>(action);
        return 0.0f;
    }

    if (botAI->IsRanged(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;
    }

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

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
