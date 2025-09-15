#include "RaidGruulsLairMultipliers.h"
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
    if (maulgar && maulgar->HasUnitState(UNIT_STATE_CASTING) && maulgar->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        if (IsChargeAction(action))
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target && target->GetGUID() == maulgar->GetGUID())
                return 0.0f;
        }
        if (dynamic_cast<MovementAction*>(action))
        {
            if (bot->GetDistance(maulgar) >= 45.0f)
                return 0.0f;
        }
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
