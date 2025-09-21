#include "RaidMagsLairMultipliers.h"
#include "RaidMagsLairActions.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"

float MagtheridonMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
    {
        return 1.0f;
    }
    // Shadow Grasp (Channeling Manticron Cube)
    if (bot->HasAura(30410) && 
        (dynamic_cast<MovementAction*>(action) || dynamic_cast<AttackAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}
