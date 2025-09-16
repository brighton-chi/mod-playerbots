#include "RaidGruulsLairMultipliers.h"
#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "WarriorActions.h"

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float HighKingMaulgarMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (maulgar && maulgar->HasAura(SPELL_AURA_WHIRLWIND) && IsChargeAction(action))
    {
        return 0.0f;
    }
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* target = AI_VALUE(Unit*, "current target");
    if (krosh && target && target->GetGUID() == krosh->GetGUID())
    {
        if (dynamic_cast<CastArcaneShotAction*>(action))
        {
            return 0.0f;
        }
        if (dynamic_cast<CastEvocationAction*>(action))
        {
            if (IsMageTank(botAI, bot) && krosh->GetVictim() && krosh->GetVictim()->GetGUID() == bot->GetGUID())
                return 0.0f;
        }
    }
    return 1.0f;
}

float GruulTheDragonkillerMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul) 
    {
        return 1.0f;
    }

    if (bot->HasAura(SPELL_AURA_GROUND_SLAM_1) || bot->HasAura(SPELL_AURA_GROUND_SLAM_2))
    {
        if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action)) ||
            IsChargeAction(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
