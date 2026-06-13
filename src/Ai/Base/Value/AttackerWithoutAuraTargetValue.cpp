/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AttackerWithoutAuraTargetValue.h"

#include "Playerbots.h"
#include "Strategy.h"
#include "TargetValue.h"

namespace
{
bool IsExcludedAttackerTarget(PlayerbotAI* botAI, GuidSet const& dynamicExclusions, ObjectGuid const& guid)
{
    return botAI->IsTargetValueExcluded(TargetValueExclusionType::Attacker, guid) ||
        dynamicExclusions.find(guid) != dynamicExclusions.end();
}
}

Unit* AttackerWithoutAuraTargetValue::Calculate()
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    GuidSet const dynamicExclusions = GatherStrategyTargetExclusions(botAI, TargetValueExclusionType::Attacker);
    // Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    uint32 max_health = 0;
    Unit* result = nullptr;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || IsExcludedAttackerTarget(botAI, dynamicExclusions, guid))
            continue;

        if (!bot->IsWithinCombatRange(unit, botAI->GetRange(range)))
            continue;

        if (unit->GetHealth() < max_health)
        {
            continue;
        }

        if (!botAI->HasAura(qualifier, unit, false, true))
        {
            max_health = unit->GetHealth();
            result = unit;
        }
    }

    return result;
}

Unit* MeleeAttackerWithoutAuraTargetValue::Calculate()
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    GuidSet const dynamicExclusions = GatherStrategyTargetExclusions(botAI, TargetValueExclusionType::Attacker);
    // Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    uint32 max_health = 0;
    Unit* result = nullptr;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || IsExcludedAttackerTarget(botAI, dynamicExclusions, guid))
            continue;

        if (!bot->IsWithinMeleeRange(unit))
            continue;

        if (checkArc && !bot->HasInArc(CAST_ANGLE_IN_FRONT, unit))
            continue;

        if (unit->GetHealth() < max_health)
        {
            continue;
        }

        if (!botAI->HasAura(qualifier, unit, false, true))
        {
            max_health = unit->GetHealth();
            result = unit;
        }
    }

    return result;
}
