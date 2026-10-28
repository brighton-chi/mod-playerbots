/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPShared.h"
#include "EncounterHelpers.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include <array>

using namespace EncounterHelpers;

namespace SwpHelpers
{

ObjectGuid FindSwpVolatileFiendGuid(Player* bot)
{
    Creature* fiend = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_VOLATILE_FIEND), VOLATILE_FIEND_SEARCH_RADIUS);

    return fiend ? fiend->GetGUID() : ObjectGuid::Empty;
}

float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
{
    if (slotCount <= 1)
        return 0.0f;

    float const angleStep = arcWidth / static_cast<float>(slotCount - 1);
    if (slotCount % 2 == 1)
    {
        if (slotIndex == 0)
            return 0.0f;

        uint8 const stepIndex = (slotIndex + 1) / 2;
        float angleOffset = angleStep * stepIndex;
        if (slotIndex % 2 == 0)
            angleOffset = -angleOffset;

        return angleOffset;
    }

    float const halfStep = angleStep / 2.0f;
    uint8 const pairIndex = slotIndex / 2;
    float angleOffset = halfStep + angleStep * pairIndex;
    if (slotIndex % 2 == 1)
        angleOffset = -angleOffset;

    return angleOffset;
}

uint32 GetManualCastCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    uint32 cooldownMs = spellInfo->GetRecoveryTime();
    if (spellInfo->CategoryRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->CategoryRecoveryTime;
    if (spellInfo->StartRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->StartRecoveryTime;

    return cooldownMs ? cooldownMs : minGlobalCooldown;
}

uint32 GetManualCastGlobalCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    if (spellInfo->StartRecoveryTime)
        return spellInfo->StartRecoveryTime;

    // A charmed caster still gets MIN_GCD for a cooldownless spell.
    return spellInfo->RecoveryTime || spellInfo->CategoryRecoveryTime ? 0 : minGlobalCooldown;
}

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell(Id(SwpSpells::SPELL_MISDIRECTION), tank))
        return botAI->CastSpell(Id(SwpSpells::SPELL_MISDIRECTION), tank);

    if (!botAI->GetBot()->HasAura(Id(SwpSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

uint32 GetReadySpellLock(Creature* pet, Unit* target)
{
    if (!pet || !target || !pet->IsAlive() || pet->GetEntry() != NPC_FELHUNTER)
        return 0;

    if (!pet->IsWithinCombatRange(target, RANGED_ABILITY_REACH) || !pet->IsWithinLOSInMap(target))
        return 0;

    static constexpr std::array spellLockRanks = {
        Id(SwpSpells::SPELL_SPELL_LOCK_2),
        Id(SwpSpells::SPELL_SPELL_LOCK_1),
    };

    for (uint32 spellLock : spellLockRanks)
    {
        if (pet->HasSpell(spellLock))
            return pet->HasSpellCooldown(spellLock) ? 0 : spellLock;
    }

    return 0;
}

bool GetPathStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards)
{
    backwards = false;
    if (bot->GetExactDist2d(position) <= arrivalDist)
        return false;

    // Whether the step is backwards is known only once the path is, so a tank takes the backward
    // length either way rather than paying for a second path.
    bool const tanking = facing && facing->GetVictim() == bot && bot->IsWithinMeleeRange(facing);
    float const stepDistance = tanking ? PATH_BACKWARD_STEP_DISTANCE : PATH_STEP_DISTANCE;
    if (!GetPathStepTowardPoint(bot, position, arrivalDist, stepDistance, stepX, stepY))
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    backwards = tanking &&
        (stepX - botX) * (facing->GetPositionX() - botX) +
        (stepY - botY) * (facing->GetPositionY() - botY) < 0.0f;

    return true;
}

}
