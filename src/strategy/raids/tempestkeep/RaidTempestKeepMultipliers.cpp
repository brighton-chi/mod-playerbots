#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"

float HighAstromancerSolarianStayStackedMultiplier::GetValue(Action* action)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    if (!astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && botAI->IsRanged(bot) &&
        (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action)))
        return 0.0f;

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER) && 
        (dynamic_cast<MovementAction*>(action) && !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action)))
        return 0.0f;

    return 1.0f;
}
