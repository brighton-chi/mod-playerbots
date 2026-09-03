/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ZAHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <vector>

using namespace ZaHelpers;
using namespace EncounterHelpers;

// General

bool ZulAmanResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;
    reset |= akilzonStormTimer.erase(bot->GetInstanceId()) > 0;

    if (!AI_VALUE2(bool, "combat", "self target"))
    {
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);
        reset |= ClearTargetIcon(bot, RtiTargetValue::moonIndex);
    }

    return reset;
}

bool ZulAmanMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(ZaSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", boss) && botAI->CastSpell("steady shot", boss);
}

bool ZulAmanTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss && PlayerbotAI::IsMainTank(bot))
        return Attack(boss);

    if (boss->GetVictim() != bot || !bot->IsWithinMeleeRange(boss))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, _position, arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        ZA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool ZulAmanSpreadRangedAction::Execute(Event /*event*/)
{
    float minDistance = _minDistance;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistance);
}

bool ZulAmanRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    float const currentDistance = bot->GetExactDist2d(boss);
    if (currentDistance >= ZA_WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(boss, ZA_WHIRLWIND_SAFE_DISTANCE - currentDistance);
}

// Trash

bool AmanishiMedicineManMarkWardAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;

    Creature* protectiveWard = bot->FindNearestCreature(
        Id(ZaNpcs::NPC_AMANI_PROTECTIVE_WARD), searchRadius);
    if (protectiveWard)
        return MarkTargetWithSkull(bot, protectiveWard);

    Creature* healingWard = bot->FindNearestCreature(
        Id(ZaNpcs::NPC_AMANI_HEALING_WARD), searchRadius);
    return healingWard && MarkTargetWithSkull(bot, healingWard);
}

// Akil'zon <Eagle Avatar>

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event /*event*/)
{
    Player* target = GetElectricalStormTarget(bot);
    if (!target && !PlayerbotAI::IsMainTank(bot))
        target = GetGroupMainTank(bot);

    if (!target || bot->GetExactDist2d(target) <= 3.0f)
        return false;

    bot->CastStop();
    return MoveTo(
        ZA_MAP_ID, target->GetPositionX(), target->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AkilzonManageElectricalStormTimerAction::Execute(Event /*event*/)
{
    return akilzonStormTimer.try_emplace(bot->GetInstanceId(), getMSTime()).second;
}

// Nalorakk <Bear Avatar>

bool NalorakkTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

    // Main tank takes bear, assist tank takes troll
    Player* nalorakkTank = nullptr;
    if (IsNalorakkInBearForm(nalorakk))
        nalorakkTank = GetGroupMainTank(bot);
    else
        nalorakkTank = GetGroupAssistTank(bot, 0);

    if (nalorakkTank && nalorakkTank == bot)
    {
        if (AI_VALUE(Unit*, "current target") != nalorakk)
            return Attack(nalorakk);

        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), true);

        if (!bot->IsWithinMeleeRange(nalorakk))
            return false;
    }

    // Both tanks walk to the position and back to it only when holding Nalorakk, so the
    // determination for direct of movement is left to GetStepToPosition().
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, NALORAKK_TANK_POSITION, arrivalDist, nalorakk, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        ZA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex =
        (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
    size_t count = rangedMembers.size();
    if (count == 0)
        return false;

    constexpr float radius = 15.0f;
    float angle = (count == 1) ? 0.0f
        : (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));

    float targetX = JANALAI_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float targetY = JANALAI_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) <= 2.0f)
        return false;

    return MoveTo(
        ZA_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool JanalaiAvoidFireBombsAction::Execute(Event /*event*/)
{
    auto const& bombs = GetNearbyFireBombs(botAI);

    if (bombs.empty())
        return false;

    bool inDanger = false;
    for (Unit* bomb : bombs)
    {
        if (bot->GetExactDist2d(bomb) < JANALAI_FIRE_BOMB_SAFE_DISTANCE)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    // The safe zone is flat, so a bot that has been knocked below the platform would be sent to a
    // spot under the floor. Leave those to normal movement.
    constexpr float maxFloorDeviation = 5.0f;
    if (std::fabs(bot->GetPositionZ() - JANALAI_PLATFORM_Z) > maxFloorDeviation)
        return false;

    constexpr float maxSearchDistance = 20.0f;
    // One tick of movement is all a step needs to cover; CanTakeStepTowards() clamps it to the
    // distance when the safe spot is nearer than this.
    constexpr float moveDist = 3.5f;

    float stepX, stepY, stepZ;
    if (!FindSafeStepInZone(
            bot, bombs, JANALAI_SAFE_ZONE, maxSearchDistance, JANALAI_FIRE_BOMB_SAFE_DISTANCE,
            moveDist, stepX, stepY, stepZ))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        ZA_MAP_ID, stepX, stepY, stepZ,
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool JanalaiMarkAmanishiHatchersAction::Execute(Event /*event*/)
{
    auto [hatcherLow, hatcherHigh] = GetAmanishiHatcherPair(botAI);

    if (!hatcherLow || !hatcherHigh || hatcherHigh == hatcherLow)
        return false;

    return MarkTargetWithMoon(bot, hatcherHigh) || MarkTargetWithSkull(bot, hatcherLow);
}

// Halazzi <Lynx Avatar>

bool HalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event /*event*/)
{
    Unit* lynx = AI_VALUE2(Unit*, "find target", "spirit of the lynx");
    if (lynx)
    {
        if (AI_VALUE(Unit*, "current target") != lynx)
            return Attack(lynx);

        if (lynx->GetVictim() != bot && botAI->DoSpecificAction("taunt spell", Event(), true))
            return true;
    }

    if (lynx && lynx->GetVictim() != bot)
        return false;

    Position const& position = HALAZZI_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    return MoveTo(
        ZA_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool HalazziDpsAttackTotemAndBossAction::Execute(Event /*event*/)
{
    // Target priority 1: Corrupted Lightning Totems
    constexpr float searchRadius = 40.0f;
    if (Creature* totem =
            bot->FindNearestCreature(Id(ZaNpcs::NPC_CORRUPTED_LIGHTNING_TOTEM), searchRadius))
    {
        if (MarkTargetWithSkull(bot, totem))
            return true;

        return AI_VALUE(Unit*, "current target") != totem && Attack(totem);
    }

    // Target priority 2: Halazzi
    if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
        return AI_VALUE(Unit*, "current target") != halazzi && Attack(halazzi);

    // Don't attack the Lynx
    return false;
}

// Hex Lord Malacrass

bool HexLordMalacrassAssignDpsPriorityAction::Execute(Event /*event*/)
{
    static constexpr std::array hexLordAdds = {
        Id(ZaNpcs::NPC_LORD_RAADAN),
        Id(ZaNpcs::NPC_ALYSON_ANTILLE),
        Id(ZaNpcs::NPC_KORAGG),
        Id(ZaNpcs::NPC_DARKHEART),
        Id(ZaNpcs::NPC_FENSTALKER),
        Id(ZaNpcs::NPC_GAZAKROTH),
        Id(ZaNpcs::NPC_THURG),
        Id(ZaNpcs::NPC_SLITHER),
        Id(ZaNpcs::NPC_HEX_LORD_MALACRASS)
    };

    auto const& targets = AI_VALUE(GuidVector, "possible targets no los");
    Unit* priorityTarget = nullptr;
    for (uint32 entry : hexLordAdds)
    {
        for (auto const& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            {
                priorityTarget = unit;
                break;
            }
        }

        if (priorityTarget)
            break;
    }

    if (priorityTarget)
    {
        if (MarkTargetWithSkull(bot, priorityTarget))
            return true;
    }

    return false;
}

bool HexLordMalacrassMoveAwayFromFreezingTrapAction::Execute(Event /*event*/)
{
    GameObject* trap = GetNearbyFreezingTrap(botAI);
    if (!trap)
        return false;

    float const currentDistance = bot->GetExactDist2d(trap);
    if (currentDistance >= ZA_FREEZING_TRAP_SAFE_DISTANCE)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(trap->GetPosition(), ZA_FREEZING_TRAP_SAFE_DISTANCE, minInterval);
}

// Zul'jin

bool ZuljinSpreadRaidForCyclonesAction::Execute(Event /*event*/)
{
    size_t slotIndex;
    if (!GetSpreadSlotIndex(bot, ZULJIN_SPREAD_POSITIONS.size(), slotIndex))
        return false;

    Position const& position = ZULJIN_SPREAD_POSITIONS[slotIndex];

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        ZA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}
