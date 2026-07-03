/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SethActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace
{
constexpr uint32 SETHEKK_HALLS_MAP_ID = 556;
constexpr uint32 SPELL_REJUVENATION_RANK_1 = 774;
constexpr uint32 REJUVENATION_SPELL_ICON_ID = 64;
constexpr uint32 SPELL_BANISH_ANZU = 42354;
constexpr uint32 SPELL_TREMOR_TOTEM = 8143;
constexpr uint32 SPELL_TREMOR_TOTEM_PASSIVE = 8145;
constexpr uint32 NPC_CHARMING_TOTEM = 20343;
constexpr uint32 NPC_HAWK_SPIRIT = 23134;
constexpr uint32 NPC_FALCON_SPIRIT = 23135;
constexpr uint32 NPC_EAGLE_SPIRIT = 23136;
}

bool TimeLostControllerMarkCharmingTotemWithSkullAction::Execute(Event event)
{
    if (Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_CHARMING_TOTEM))
        MarkTargetWithSkull(bot, totem);

    return false;
}

bool SethekkProphetDropTremorTotemAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_TREMOR_TOTEM_PASSIVE))
        return false;

    if (botAI->CanCastSpell(SPELL_TREMOR_TOTEM, bot))
        return botAI->CastSpell(SPELL_TREMOR_TOTEM, bot);

    return false;
}

bool DarkweaverSythMarkElementalsWithSkullAction::Execute(Event event)
{
    if (Unit* frostElemental = AI_VALUE2(Unit*, "find target", "syth frost elemental"))
        MarkTargetWithSkull(bot, frostElemental);
    else if (Unit* shadowElemental = AI_VALUE2(Unit*, "find target", "syth shadow elemental"))
        MarkTargetWithSkull(bot, shadowElemental);
    else if (Unit* arcaneElemental = AI_VALUE2(Unit*, "find target", "syth arcane elemental"))
        MarkTargetWithSkull(bot, arcaneElemental);
    else if (Unit* fireElemental = AI_VALUE2(Unit*, "find target", "syth fire elemental"))
        MarkTargetWithSkull(bot, fireElemental);

    return false;
}

bool AnzuAlternateMarksOnBossAction::Execute(Event event)
{
    Unit* anzu = AI_VALUE2(Unit*, "find target", "anzu");
    if (!anzu)
        return false;

    if (anzu->HasAura(SPELL_BANISH_ANZU))
        MarkTargetWithMoon(bot, anzu);
    else
        MarkTargetWithSkull(bot, anzu);

    return false;
}

bool AnzuCastHealOverTimeSpellOnBirdSpiritAction::Execute(Event event)
{
    // Priority: falcon > eagle (banish only) > hawk
    if (Unit* spirit = GetFirstAliveUnitByEntry(botAI, NPC_FALCON_SPIRIT);
        spirit && !spirit->GetAuraEffect(
            SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, REJUVENATION_SPELL_ICON_ID, 0))
    {
        if (botAI->CanCastSpell(SPELL_REJUVENATION_RANK_1, spirit) &&
            botAI->CastSpell(SPELL_REJUVENATION_RANK_1, spirit))
        {
            return true;
        }
    }

    if (Unit* anzu = AI_VALUE2(Unit*, "find target", "anzu");
        anzu && anzu->HasAura(SPELL_BANISH_ANZU))
    {
        if (Unit* spirit = GetFirstAliveUnitByEntry(botAI, NPC_EAGLE_SPIRIT);
            spirit && !spirit->GetAuraEffect(
                SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, REJUVENATION_SPELL_ICON_ID, 0))
        {
            if (botAI->CanCastSpell(SPELL_REJUVENATION_RANK_1, spirit) &&
                botAI->CastSpell(SPELL_REJUVENATION_RANK_1, spirit))
            {
                return true;
            }
        }
    }

    if (Unit* spirit = GetFirstAliveUnitByEntry(botAI, NPC_HAWK_SPIRIT);
        spirit && !spirit->GetAuraEffect(
            SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, REJUVENATION_SPELL_ICON_ID, 0))
    {
        if (botAI->CanCastSpell(SPELL_REJUVENATION_RANK_1, spirit) &&
            botAI->CastSpell(SPELL_REJUVENATION_RANK_1, spirit))
        {
            return true;
        }
    }

    return false;
}

bool TalonKingIkissMoveToPillarPositionAction::Execute(Event event)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    if (ikiss->GetHealthPct() > 95.0f)
        _hasReachedPillarPosition = false;

    if (_hasReachedPillarPosition == true)
        return false;

    const Position position = { 35.538f, 309.573f, 25.086f };
    const float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (botAI->IsTank(bot))
    {
        const float dX = position.GetPositionX() - bot->GetPositionX();
        const float dY = position.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(2.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;
    }
    else
    {
        float moveX = position.GetPositionX();
        float moveY = position.GetPositionY();
    }

    if (distToPosition > 2.0f)
    {
        return MoveTo(SETHEKK_HALLS_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }
    else
    {
        _hasReachedPillarPosition = true;
    }

    return false;
}

bool TalonKingIkissLosArcaneExplosionAction::Execute(Event event)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    // TODO: Replace with actual pillar center coordinates
    const Position pillarCenter = { 23.730f, 309.230f };
    constexpr float orbitIncrement = 3.5f;

    // Angle from pillar to each unit
    float const angleToBot = pillarCenter.GetAngle(bot);
    float const angleToIkiss = pillarCenter.GetAngle(ikiss);

    // Move away from ikiss: if bot is clockwise of ikiss, orbit further CW
    float const delta = Position::NormalizeOrientation(angleToBot - angleToIkiss);
    float const direction = (delta > 0.0f && delta < M_PI) ? 1.0f : -1.0f;

    // Tangent to the pillar circle (perpendicular to pillar→bot)
    float const tangentAngle = angleToBot + direction * (M_PI / 16.0f);

    float const moveX = bot->GetPositionX() + cos(tangentAngle) * orbitIncrement;
    float const moveY = bot->GetPositionY() + sin(tangentAngle) * orbitIncrement;

    botAI->Reset();
    return MoveTo(SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}
