/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */


#include <array>

#include "SethActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace
{
constexpr uint32 SETHEKK_HALLS_MAP_ID       = 556;
constexpr uint32 SPELL_TREMOR_TOTEM         = 8143;
constexpr uint32 SPELL_BANISH_ANZU          = 42354;
constexpr uint32 SPELL_REJUVENATION_RANK_1  = 774;
constexpr uint32 REJUVENATION_SPELL_ICON_ID = 64;
constexpr uint32 NPC_CHARMING_TOTEM         = 20343;
constexpr uint32 NPC_HAWK_SPIRIT            = 23134;
constexpr uint32 NPC_FALCON_SPIRIT          = 23135;
constexpr uint32 NPC_EAGLE_SPIRIT           = 23136;

Position const PILLAR_CENTER   = { 23.730f, 309.230f };
Position const PILLAR_POSITION = { 35.538f, 309.573f, 25.086f };
}

bool TimeLostControllerMarkCharmingTotemWithSkullAction::Execute(Event /*event*/)
{
    if (Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_CHARMING_TOTEM))
        return MarkTargetWithSkull(bot, totem);

    return false;
}

bool SethekkProphetDropTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(SPELL_TREMOR_TOTEM, bot) &&
        botAI->CastSpell(SPELL_TREMOR_TOTEM, bot);
}

bool DarkweaverSythMarkElementalsWithSkullAction::Execute(Event /*event*/)
{
    std::array<const char*, 4> const elementals =
    {
        "syth frost elemental",
        "syth shadow elemental",
        "syth arcane elemental",
        "syth fire elemental"
    };

    for (auto const& name : elementals)
    {
        if (Unit* elemental = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool AnzuAlternateMarksOnBossAction::Execute(Event /*event*/)
{
    Unit* anzu = AI_VALUE2(Unit*, "find target", "anzu");
    if (!anzu)
        return false;

    if (anzu->HasAura(SPELL_BANISH_ANZU))
        return MarkTargetWithMoon(bot, anzu);

    return MarkTargetWithSkull(bot, anzu);
}

// Priority: Falcon (haste) > Eagle during Banish (damage all enemies) > Hawk (damage reduction)
bool AnzuCastHealOverTimeSpellOnBirdSpiritAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 60.0f;
    Creature* targetSpirit = nullptr;

    std::array<uint32, 3> const spiritEntries =
        { NPC_FALCON_SPIRIT, NPC_HAWK_SPIRIT, NPC_EAGLE_SPIRIT };

    for (uint32 entry : spiritEntries)
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

bool TalonKingIkissTankMoveBossToPillarPositionAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    if (ikiss->GetHealthPct() > 95.0f)
        _hasReachedPillarPosition = false;

    if (_hasReachedPillarPosition == true)
        return false;

    Position const position = PILLAR_POSITION;
    float const distToPosition = bot->GetExactDist2d(
        position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 2.0f)
    {
        if (bot->IsWithinMeleeRange(ikiss))
        {
            float const dX = position.GetPositionX() - bot->GetPositionX();
            float const dY = position.GetPositionY() - bot->GetPositionY();
            float const moveDist = std::min(2.0f, distToPosition);
            float const moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float const moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

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

bool TalonKingIkissRangedStayNearVictimOfBossAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss || !ikiss->GetVictim())
        return false;

    Player* victim = ikiss->GetVictim()->ToPlayer();
    if (!victim)
        return false;

    constexpr float targetDistance = 10.0f;
    constexpr float tolerance = 5.0f;
    float const distanceToVictim = bot->GetExactDist2d(victim);
    if (distanceToVictim <= targetDistance + tolerance)
        return false;

    float const angle = victim->GetAngle(bot);
    float const destX = victim->GetPositionX() + std::cos(angle) * targetDistance;
    float const destY = victim->GetPositionY() + std::sin(angle) * targetDistance;

    return MoveTo(
        SETHEKK_HALLS_MAP_ID, destX, destY, victim->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool TalonKingIkissLosArcaneExplosionAction::Execute(Event event)
{
    Position const pillarCenter = PILLAR_CENTER;
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

    constexpr float circleRadiusMin = 10.0f;
    constexpr float circleRadiusMax = 12.0f;
    constexpr float angularStep = M_PI / 16.0f;

    float const radius = std::clamp(
        bot->GetExactDist2d(pillarCenter), circleRadiusMin, circleRadiusMax);
    float const destAngle = pillarCenter.GetAngle(ikiss) + M_PI;

    float const destX = pillarCenter.GetPositionX() + radius * cos(destAngle);
    float const destY = pillarCenter.GetPositionY() + radius * sin(destAngle);

    if (bot->GetExactDist2d(destX, destY) < 2.0f)
        return false;

    float const delta = Position::NormalizeOrientation(destAngle - botAngle);
    float const direction = (delta > 0.0f && delta < M_PI) ? 1.0f : -1.0f;
    float const targetAngle = botAngle + direction * angularStep;

    float const moveX = pillarCenter.GetPositionX() + radius * cos(targetAngle);
    float const moveY = pillarCenter.GetPositionY() + radius * sin(targetAngle);

    botAI->InterruptSpell();
    return MoveTo(
        SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TalonKingIkissMoveToWithinLosAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    Position const pillarCenter = PILLAR_CENTER;
    constexpr float angularStep = M_PI / 16.0f;

    float const botAngle = pillarCenter.GetAngle(bot);
    float const targetAngle = pillarCenter.GetAngle(ikiss);
    float const radius = bot->GetExactDist2d(pillarCenter);

    float const delta = Position::NormalizeOrientation(targetAngle - botAngle);

    if (fabs(delta) < angularStep)
        return false;

    float const direction = (delta > 0.0f && delta < M_PI) ? 1.0f : -1.0f;
    float const stepAngle = botAngle + direction * angularStep;

    float const moveX = pillarCenter.GetPositionX() + radius * cos(stepAngle);
    float const moveY = pillarCenter.GetPositionY() + radius * sin(stepAngle);

    return MoveTo(
        SETHEKK_HALLS_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}
