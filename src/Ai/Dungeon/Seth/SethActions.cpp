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
constexpr uint32 SPELL_TREMOR_TOTEM = 8143;
constexpr uint32 SPELL_BANISH_ANZU = 42354;
constexpr uint32 SPELL_REJUVENATION_RANK_1 = 774;
constexpr uint32 REJUVENATION_SPELL_ICON_ID = 64;
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

// Priority: Falcon (haste) > Eagle during Banish (damage all enemies) > Hawk (damage reduction)
bool AnzuCastHealOverTimeSpellOnBirdSpiritAction::Execute(Event event)
{
    constexpr float searchRadius = 60.0f;
    Creature* targetSpirit = nullptr;

    for (uint32 entry : { NPC_FALCON_SPIRIT, NPC_HAWK_SPIRIT, NPC_EAGLE_SPIRIT })
    {
        Creature* spirit = bot->FindNearestCreature(entry, searchRadius, true);
        if (spirit && !spirit->GetAuraEffect(
                SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, REJUVENATION_SPELL_ICON_ID, 0))
        {
            targetSpirit = spirit;
            break;
        }
    }

    if (!targetSpirit)
        return false;

    return botAI->CanCastSpell(SPELL_REJUVENATION_RANK_1, targetSpirit) &&
        botAI->CastSpell(SPELL_REJUVENATION_RANK_1, targetSpirit);
}

bool TalonKingIkissTankMoveBossToPillarPositionAction::Execute(Event event)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    if (ikiss->GetHealthPct() > 95.0f)
        _hasReachedPillarPosition = false;

    if (_hasReachedPillarPosition == true)
        return false;

    const Position position = { 35.538f, 309.573f, 25.086f };
    const float distToPosition = bot->GetExactDist2d(
        position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 2.0f)
    {
        if (bot->IsWithinMeleeRange(ikiss))
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(2.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(
                SETHEKK_HALLS_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        _hasReachedPillarPosition = true;
    }

    return false;
}

bool TalonKingIkissRangedStayNearVictimOfBossAction::Execute(Event event)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss || !ikiss->GetVictim())
        return false;

    Player* victim = ikiss->GetVictim()->ToPlayer();
    if (!victim)
        return false;

    constexpr float maxDistance = 10.0f;
    constexpr float tolerance = 5.0f;
    if (bot->GetExactDist2d(victim) <= maxDistance + tolerance)
        return false;

    return MoveTo(victim, maxDistance, MovementPriority::MOVEMENT_COMBAT);
}

bool TalonKingIkissLosArcaneExplosionAction::Execute(Event event)
{
    const Position pillarCenter = { 23.730f, 309.230f };
    float const botAngle = pillarCenter.GetAngle(bot);

    return MoveToPillar(pillarCenter, botAngle) || MoveAroundPillar(pillarCenter, botAngle);
}

bool TalonKingIkissLosArcaneExplosionAction::MoveToPillar(
    Position const& pillarCenter, float botAngle)
{
    constexpr float circleRadiusMin = 10.0f;
    constexpr float circleRadiusMax = 12.0f;
    float const distToPillar = bot->GetExactDist2d(pillarCenter);

    if (distToPillar >= circleRadiusMin && distToPillar <= circleRadiusMax)
        return false;

    float const targetRadius =
        distToPillar < circleRadiusMin ? circleRadiusMin : circleRadiusMax;
    float const moveX = pillarCenter.GetPositionX() + targetRadius * cos(botAngle);
    float const moveY = pillarCenter.GetPositionY() + targetRadius * sin(botAngle);

    botAI->InterruptSpell();
    return MoveTo(
        SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TalonKingIkissLosArcaneExplosionAction::MoveAroundPillar(
    Position const& pillarCenter, float botAngle)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    constexpr float circleRadius = 11.0f;
    constexpr float arcStep = 4.0f;

    float const destAngle = pillarCenter.GetAngle(ikiss) + M_PI;
    float const destX = pillarCenter.GetPositionX() + circleRadius * cos(destAngle);
    float const destY = pillarCenter.GetPositionY() + circleRadius * sin(destAngle);

    if (bot->GetExactDist2d(destX, destY) < 2.0f)
        return false;

    float const delta = Position::NormalizeOrientation(destAngle - botAngle);
    float const direction = (delta > 0.0f && delta < M_PI) ? 1.0f : -1.0f;
    float const tangentAngle = botAngle + direction * static_cast<float>(M_PI_2);

    float const moveX = bot->GetPositionX() + cos(tangentAngle) * arcStep;
    float const moveY = bot->GetPositionY() + sin(tangentAngle) * arcStep;

    botAI->InterruptSpell();
    return MoveTo(
        SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TalonKingIkissMoveToWithinLosAction::Execute(Event event)
{
    constexpr Position pillarCenter = { 23.730f, 309.230f };
    constexpr float circleRadius = 11.0f;
    constexpr float arcStep = 4.0f;

    float const botAngle = pillarCenter.GetAngle(bot);

    // Orbit clockwise to regain LOS
    float const tangentAngle = botAngle - static_cast<float>(M_PI_2);

    float const moveX = bot->GetPositionX() + cos(tangentAngle) * arcStep;
    float const moveY = bot->GetPositionY() + sin(tangentAngle) * arcStep;

    botAI->InterruptSpell();
    return MoveTo(
        SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}
