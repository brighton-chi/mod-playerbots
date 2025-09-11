#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "WarriorActions.h"
#include "AiObjectContext.h"

float KarazhanShadeOfAranMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!boss)
        return 1.0f;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION)) 
    {
        if (dynamic_cast<MovementAction*>(action) ||
            dynamic_cast<CastChargeAction*>(action) ||
            dynamic_cast<CastInterceptAction*>(action) ||
            dynamic_cast<CastFeralChargeBearAction*>(action) ||
            dynamic_cast<CastFeralChargeCatAction*>(action))
        {
            return 0.0f;
        }
    }
	return 1.0f;
}

float KarazhanPrinceMalchezaarMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!boss)
        return 1.0f;

    if (boss && bot->HasAura(SPELL_ENFEEBLE))
    {
        if (dynamic_cast<CastChargeAction*>(action) ||
            dynamic_cast<CastInterceptAction*>(action) ||
            dynamic_cast<CastFeralChargeBearAction*>(action) ||
            dynamic_cast<CastFeralChargeCatAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
