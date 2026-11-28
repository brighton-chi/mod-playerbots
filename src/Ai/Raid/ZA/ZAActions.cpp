/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAActions.h"
#include "EncounterHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ThreatHelpers.h"
#include "ZAHelpers.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

using namespace ZaHelpers;
using namespace EncounterHelpers;

// Shared

bool ZulAmanResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;
    reset |= ZaResetInstance(bot->GetInstanceId());
    reset |= ClearStrategyTargetIcon(bot, RtiTargetValue::skullIndex, {
        Id(ZaNpcs::NPC_LORD_RAADAN),
        Id(ZaNpcs::NPC_ALYSON_ANTILLE),
        Id(ZaNpcs::NPC_KORAGG),
        Id(ZaNpcs::NPC_DARKHEART),
        Id(ZaNpcs::NPC_FENSTALKER),
        Id(ZaNpcs::NPC_GAZAKROTH),
        Id(ZaNpcs::NPC_THURG),
        Id(ZaNpcs::NPC_SLITHER),
        Id(ZaNpcs::NPC_HEX_LORD_MALACRASS),
    });

    return reset;
}

bool ZulAmanMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = GetZaBoss(bot, _boss);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    if (botAI->CanCastSpell(Id(ZaSpells::SPELL_MISDIRECTION), mainTank))
        return botAI->CastSpell(Id(ZaSpells::SPELL_MISDIRECTION), mainTank);

    if (!bot->HasAura(Id(ZaSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", boss) && botAI->CastSpell("steady shot", boss);
}

bool ZulAmanTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = GetZaBoss(bot, _boss);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss && PlayerbotAI::IsMainTank(bot))
        return Attack(boss);

    if (boss->GetVictim() != bot || !bot->IsWithinMeleeRange(boss))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, _position, ZA_ARRIVAL_DISTANCE, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        ZA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool ZulAmanSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, _minDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), _minDistance);
}

bool ZulAmanRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* boss = GetZaBoss(bot, _boss);
    if (!boss)
        return false;

    float const currentDistance = bot->GetExactDist2d(boss);
    if (currentDistance >= ZA_WHIRLWIND_CLEARANCE)
        return false;

    bot->CastStop();
    return MoveAway(boss, ZA_WHIRLWIND_CLEARANCE - currentDistance);
}

// Trash

bool AmanishiMedicineManMarkWardAction::Execute(Event /*event*/)
{
    Creature* protectiveWard = bot->FindNearestCreature(
        Id(ZaNpcs::NPC_AMANI_PROTECTIVE_WARD), ZA_CREATURE_SEARCH_RADIUS);
    if (protectiveWard)
        return MarkTargetWithSkull(bot, protectiveWard);

    Creature* healingWard = bot->FindNearestCreature(
        Id(ZaNpcs::NPC_AMANI_HEALING_WARD), ZA_CREATURE_SEARCH_RADIUS);
    return healingWard && MarkTargetWithSkull(bot, healingWard);
}

// Akil'zon <Eagle Avatar>

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event /*event*/)
{
    Player* target = GetElectricalStormTarget(bot);
    if (!target && !PlayerbotAI::IsMainTank(bot))
        target = GetGroupMainTank(bot);

    if (!target || bot->GetExactDist2d(target) <= ZA_ARRIVAL_DISTANCE)
        return false;

    bot->CastStop();
    return MoveTo(
        ZA_MAP_ID, target->GetPositionX(), target->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AkilzonStartElectricalStormTimerAction::Execute(Event /*event*/)
{
    return EmplaceIfUnset(ZaState(bot->GetInstanceId()).akilzonStormTimer, getMSTime());
}

// Nalorakk <Bear Avatar>

bool NalorakkTanksPositionBossAction::Execute(Event event)
{
    Unit* nalorakk = GetZaBoss(bot, ZaBosses::DATA_NALORAKK);
    if (!nalorakk)
        return false;

    if (AI_VALUE(Unit*, "current target") != nalorakk)
        return Attack(nalorakk);

    Player* nalorakkTank = nullptr;
    if (IsNalorakkInBearForm(nalorakk))
        nalorakkTank = GetGroupMainTank(bot);
    else
        nalorakkTank = GetGroupAssistTank(bot, 0);

    if (nalorakkTank && nalorakkTank == bot)
    {
        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", event, true);

        if (!bot->IsWithinMeleeRange(nalorakk))
            return false;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, NALORAKK_TANK_POSITION, ZA_ARRIVAL_DISTANCE, nalorakk, moveX, moveY, backwards))
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
        if (!member || !bot->IsInMap(member) || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex =
        (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
    size_t count = rangedMembers.size();
    float angle = (count == 1) ? 0.0f
        : (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));

    float targetX =
        JANALAI_TANK_POSITION.GetPositionX() + JANALAI_RANGED_SPREAD_RADIUS * std::cos(angle);
    float targetY =
        JANALAI_TANK_POSITION.GetPositionY() + JANALAI_RANGED_SPREAD_RADIUS * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) <= ZA_ARRIVAL_DISTANCE)
        return false;

    return MoveTo(
        ZA_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool JanalaiAvoidFireBombsAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    std::vector<Unit*> const bombs = GetFireBombs(bot);

    if (bombs.empty())
        return false;

    bool inDanger = false;
    for (Unit* bomb : bombs)
    {
        if (bot->GetExactDist2d(bomb) < JANALAI_FIRE_BOMB_CLEARANCE)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxFloorDeviation = 5.0f;
    if (std::fabs(bot->GetPositionZ() - JANALAI_PLATFORM_Z) > maxFloorDeviation)
        return false;

    constexpr float moveDist = 3.5f;
    float stepX;
    float stepY;
    float stepZ;
    if (!FindSafeStepInJanalaiZone(
            bot, bombs, JANALAI_SAFE_ZONE, JANALAI_FIRE_BOMB_SAFE_POSITION_SEARCH_RADIUS,
            JANALAI_FIRE_BOMB_CLEARANCE, moveDist, stepX, stepY, stepZ))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        ZA_MAP_ID, stepX, stepY, stepZ, false, false, false, false, priority, true, false);
}

bool JanalaiMarkAmanishiHatchersAction::Execute(Event /*event*/)
{
    auto [hatcherLow, hatcherHigh] = GetAmanishiHatcherPair(bot);
    if (!hatcherLow || !hatcherHigh || hatcherHigh == hatcherLow)
        return false;

    return MarkTargetWithMoon(bot, hatcherHigh) || MarkTargetWithSkull(bot, hatcherLow);
}

// Halazzi <Lynx Avatar>

bool HalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event event)
{
    if (Unit* lynx = FindThreatTarget(bot, "spirit of the lynx"))
    {
        if (AI_VALUE(Unit*, "current target") != lynx)
            return Attack(lynx);

        if (lynx->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", event, true);
    }

    Position const& position = HALAZZI_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= ZA_ARRIVAL_DISTANCE)
        return false;

    return MoveTo(
        ZA_MAP_ID, position.GetPositionX(), position.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool HalazziDpsAttackTotemAndBossAction::Execute(Event /*event*/)
{
    // Target priority 1: Corrupted Lightning Totems
    if (Creature* totem = GetNearestCorruptedLightningTotem(bot))
    {
        if (MarkTargetWithSkull(bot, totem))
            return true;

        return AI_VALUE(Unit*, "current target") != totem && Attack(totem);
    }

    // Target priority 2: Halazzi
    if (Unit* halazzi = GetZaBoss(bot, ZaBosses::DATA_HALAZZI))
        return AI_VALUE(Unit*, "current target") != halazzi && Attack(halazzi);

    // Don't attack the Lynx.
    return false;
}

// Hex Lord Malacrass

bool HexLordMalacrassMarkTargetAction::Execute(Event /*event*/)
{
    GuidVector const& adds = GetInstanceCreatureList(
        ZaState(bot->GetInstanceId()).hexLordAdds, HEX_LORD_ADD_CACHE_INTERVAL_MS, bot,
        FindHexLordAddGuids);

    for (uint32 entry : HEX_LORD_ADD_ENTRIES)
    {
        for (ObjectGuid const& guid : adds)
        {
            if (guid.GetEntry() != entry)
                continue;

            Creature* add = ObjectAccessor::GetCreature(*bot, guid);
            if (add && add->IsAlive())
                return MarkTargetWithSkull(bot, add);
        }
    }

    return false;
}

bool HexLordMalacrassMoveAwayFromFreezingTrapAction::Execute(Event /*event*/)
{
    GameObject* trap = GetNearbyFreezingTrap(botAI);
    if (!trap)
        return false;

    float const currentDistance = bot->GetExactDist2d(trap);
    if (currentDistance >= HEX_LORD_FREEZING_TRAP_CLEARANCE)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(trap->GetPosition(), HEX_LORD_FREEZING_TRAP_CLEARANCE, minInterval);
}

// Zul'jin

bool ZuljinHealGrievousThrowTargetToFullAction::Execute(Event /*event*/)
{
    Player* target = GetZuljinGrievousThrowHealTarget(bot);
    if (!target)
        return false;

    char const* heal = nullptr;
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            heal = "greater heal";
            break;
        case CLASS_PALADIN:
            heal = "holy light";
            break;
        case CLASS_SHAMAN:
            heal = "healing wave";
            break;
        case CLASS_DRUID:
            heal = "healing touch";
            break;
        default:
            return false;
    }

    return botAI->CanCastSpell(heal, target) && botAI->CastSpell(heal, target);
}

bool ZuljinMassDispelCreepingParalysisAction::Execute(Event /*event*/)
{
    Player* paralysisTarget = GetZuljinCreepingParalysisDispelTarget(bot);
    return paralysisTarget &&
        botAI->CanCastSpell(Id(ZaSpells::SPELL_MASS_DISPEL), paralysisTarget) &&
        botAI->CastSpell(Id(ZaSpells::SPELL_MASS_DISPEL), paralysisTarget);
}

bool ZuljinPositionRangedForCyclonesAction::Execute(Event /*event*/)
{
    size_t slotIndex;
    if (!GetZuljinSpreadSlotIndex(bot, ZULJIN_SPREAD_POSITIONS.size(), slotIndex))
        return false;

    Position const& position = ZULJIN_SPREAD_POSITIONS[slotIndex];

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, ZA_ARRIVAL_DISTANCE, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        ZA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}
