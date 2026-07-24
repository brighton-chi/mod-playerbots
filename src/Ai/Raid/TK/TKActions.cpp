/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKActions.h"
#include "AiFactory.h"
#include "EquipAction.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"

using namespace TkHelpers;

// General

bool TempestKeepResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    bool reset = false;

    if (!AI_VALUE2(Unit*, "find target", "alar"))
    {
        if (isAlarInPhase2.erase(instanceId) > 0)
            reset = true;

        if (lastRebirthState.erase(instanceId) > 0)
            reset = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "void reaver"))
    {
        if (voidReaverArcaneOrbs.erase(instanceId) > 0)
            reset = true;

        Action* action = botAI->GetAiObjectContext()->GetAction("void reaver spread ranged");
        if (action &&
            static_cast<VoidReaverSpreadRangedInCircleAction*>(action)->ResetReachedRangedPosition())
        {
            reset = true;
        }
    }

    return reset;
}

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> centurions;
    bot->GetCreatureListWithEntryInGrid(
        centurions, static_cast<uint32>(TkNpcs::NPC_CRIMSON_HAND_CENTURION), searchRadius);

    for (Creature* centurion : centurions)
    {
        if (!centurion || !centurion->IsAlive() ||
            !centurion->HasAura(static_cast<uint32>(TkSpells::SPELL_ARCANE_FLURRY)) ||
            !botAI->HasAura("polymorph", centurion))
        {
            continue;
        }

        if (!target || centurion->GetGUID() < target->GetGUID())
            target = centurion;
    }

    if (!target)
        return false;

    return botAI->CanCastSpell("polymorph", target) && botAI->CastSpell("polymorph", target);
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", alar))
    {
        return botAI->CastSpell("steady shot", alar);
    }

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    bool isMainTank = botAI->IsMainTank(bot);
    bool isFirstAssistTank = botAI->IsAssistTankOfIndex(bot, 0, true);
    if (!isMainTank && !isFirstAssistTank)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    constexpr uint8 TANK_PLATFORM_W  = 0;
    constexpr uint8 TANK_PLATFORM_NW = 1;
    constexpr uint8 TANK_PLATFORM_NE = 2;
    constexpr uint8 TANK_PLATFORM_E  = 3;

    int8 platformIdx;
    if (isMainTank)
    {
        platformIdx = (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_3_IDX) ?
            TANK_PLATFORM_W : TANK_PLATFORM_NE;
    }
    else if (isFirstAssistTank)
    {
        platformIdx = (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_1_IDX) ?
            TANK_PLATFORM_NW : TANK_PLATFORM_E;
    }

    Position const target = PLATFORM_POSITIONS[platformIdx];
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!botAI->IsMelee(bot) || botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0, true) || botAI->IsAssistTankOfIndex(bot, 1, false))
    {
        return false;
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    Position const target = PLATFORM_POSITIONS[locationIndex];
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event /*event*/)
{
    bool isRanged = botAI->IsRanged(bot);
    bool isEmberTank = botAI->IsAssistTankOfIndex(bot, 1, false);
    if (!isRanged && !isEmberTank)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    Position const position = GROUND_POSITIONS[locationIndex];
    if (isRanged)
    {
        constexpr float distFromTarget = 8.0f;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > distFromTarget)
        {
            return MoveInside(
                TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), distFromTarget, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    else if (isEmberTank && !AI_VALUE2(Unit*, "find target", "ember of al'ar"))
    {
        constexpr float distFromTarget = 20.0f;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > distFromTarget)
        {
            return MoveInside(
                TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), distFromTarget, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarAssistTanksPickUpEmbersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return HandlePhase1Embers(alar);

    return HandlePhase2Embers(event);
}

// Embers will be tanked by only the second assist tank in Phase 1
bool AlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, false))
        return false;

    Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (ember->GetVictim() != bot)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    Position const position = GROUND_POSITIONS[locationIndex];
    Position const center = ALAR_POINT_MIDDLE;
    float dx = center.GetPositionX() - position.GetPositionX();
    float dy = center.GetPositionY() - position.GetPositionY();
    float distToCenter = position.GetExactDist2d(center.GetPositionX(), center.GetPositionY());

    constexpr float moveDist = 25.0f;
    float targetX = position.GetPositionX() + (dx / distToCenter) * moveDist;
    float targetY = position.GetPositionY() + (dy / distToCenter) * moveDist;

    if (bot->GetExactDist2d(targetX, targetY) < 1.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar)
bool AlarAssistTanksPickUpEmbersAction::HandlePhase2Embers(Event const& event)
{
    auto [firstEmber, secondEmber] = GetTargetUnitPair(
        botAI, static_cast<uint32>(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (botAI->IsAssistTankOfIndex(bot, 1, false))
        ember = firstEmber;
    else if (GetSecondEmberTank(bot) == bot)
        ember = secondEmber;

    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (ember->GetVictim() != bot)
    {
        return botAI->DoSpecificAction("taunt spell", event, true);
    }
    else
    {
        constexpr float safeDistance = 17.0f;
        if (GetNearestNonTankPlayerInRadius(bot, safeDistance))
            return MoveFromGroup(safeDistance);
    }

    return false;
}

bool AlarRangedDpsPrioritizeEmbersAction::Execute(Event /*event*/)
{
    auto [firstEmber, secondEmber] = GetTargetUnitPair(
        botAI, static_cast<uint32>(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (firstEmber)
        ember = firstEmber;
    else if (secondEmber)
        ember = secondEmber;

    if (ember)
    {
        constexpr float safeDistance = 15.0f;
        const float currentDistance = bot->GetDistance2d(ember);
        if (currentDistance < safeDistance)
        {
            botAI->InterruptSpell();
            return MoveAway(ember, safeDistance - currentDistance);
        }

        if (AI_VALUE(Unit*, "current target") != ember)
            return Attack(ember);
    }
    else if (Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar"))
    {
        if (AI_VALUE(Unit*, "current target") != alar)
            return Attack(alar);
    }

    return false;
}

// Jump from platform during Flame Quills and wait at assigned position after landing
bool AlarJumpFromPlatformAction::Execute(Event /*event*/)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        botAI->InterruptSpell();
        return JumpTo(
            TK_MAP_ID, ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(),
            MovementPriority::MOVEMENT_FORCED);
    }

    constexpr float distAlarTankFromPos = 5.0f;
    constexpr float distEmberTankFromPos = 25.0f;
    constexpr float distMeleeDpsFromPos = 5.0f;
    constexpr float distRangedFromPos = 10.0f;

    if (botAI->IsMainTank(bot) && bot->GetExactDist2d(
        ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY()) > distAlarTankFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
            ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true) && bot->GetExactDist2d(
        ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY()) > distAlarTankFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
            ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 1, false) && bot->GetExactDist2d(
        ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > distEmberTankFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), distEmberTankFromPos,
            MovementPriority::MOVEMENT_FORCED);
    }
    else if (botAI->IsMelee(bot) && bot->GetExactDist2d(
        ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY()) > distMeleeDpsFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
            ALAR_ROOM_S_CENTER.GetPositionZ(), distMeleeDpsFromPos,
            MovementPriority::MOVEMENT_FORCED);
    }
    else if (bot->GetExactDist2d( // Ranged
        ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > distRangedFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), distRangedFromPos,
            MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (botAI->IsRanged(bot) || botAI->IsTank(bot))
    {
        Creature* alarCreature = alar->ToCreature();
        if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
            return false;
    }

    // Per above, ranged/tanks wait until Al'ar actually "dies"; melee dps jumps off at 5% health
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position position;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, position);

        botAI->InterruptSpell();
        return JumpTo(
            TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        float currentDistance = bot->GetDistance2d(alar);
        constexpr float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
            return MoveAway(alar, safeDistance - currentDistance);
    }

    return false;
}

// Main tank and first assist tank will swap tanking Al'ar when Melt Armor is applied
bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // secondEmberTank = whichever of MT or 1st AT doesn't have Melt Armor (1st AT if neither do)
    Player* secondEmberTank = GetSecondEmberTank(bot);
    if (!secondEmberTank || secondEmberTank != bot)
    {
        if (AI_VALUE(Unit*, "current target") != alar)
            return Attack(alar);

        if (alar->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", event, true);
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return AvoidFlamePatch() || HandleDiveBomb(alar);
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::AvoidFlamePatch()
{
    constexpr float searchRadius = 40.0f;
    constexpr float hazardRadius = 8.0f;

    std::vector<Unit*> flamePatches =
        GetAllHazardTriggers(bot, static_cast<uint32>(TkNpcs::NPC_FLAME_PATCH), searchRadius);

    for (Unit* flamePatch : flamePatches)
    {
        if (bot->GetExactDist2d(flamePatch) < hazardRadius)
        {
            Position safestPos = FindSafestNearbyPosition(bot, flamePatches, hazardRadius);
            botAI->InterruptSpell();
            return MoveTo(
                TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                safestPos.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    // After Dive Bomb, before reapperance
    if (alar->HasAura(static_cast<uint32>(TkSpells::SPELL_MODEL_INVISIBILITY)) ||
        (alar->HasUnitState(UNIT_STATE_CASTING) &&
         alar->FindCurrentSpellBySpellId(static_cast<uint32>(TkSpells::SPELL_REBIRTH_DIVE))))
    {
        float currentDistance = bot->GetDistance2d(alar);
        constexpr float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
        {
            botAI->InterruptSpell();
            return MoveAway(alar, safeDistance - currentDistance);
        }

        return false;
    }

    // During Dive Bomb sequence
    Position dest;
    if (GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar, dest) != POINT_QUILL_OR_DIVE_IDX)
    {
        return false;
    }

    constexpr float safeDistance = 10.0f;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
    {
        float const currentDistance = bot->GetDistance2d(nearestPlayer);
        return MoveAway(nearestPlayer, safeDistance - currentDistance);
    }

    return false;
}

// For Phase 2, ensure that bots don't get too far away and become inactive
bool AlarReturnToRoomCenterAction::Execute(Event /*event*/)
{
    constexpr float distFromCenter = 45.0f;
    Position const center = ALAR_ROOM_CENTER;
    if (AI_VALUE(Unit*, "current target") == nullptr &&
        bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > distFromCenter)
    {
        return MoveInside(
            TK_MAP_ID, center.GetPositionX(), center.GetPositionY(),
            center.GetPositionZ(), distFromCenter - 5.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AlarManagePhaseTrackerAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 const instanceId = alar->GetMap()->GetInstanceId();

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
        alar->FindCurrentSpellBySpellId(static_cast<uint32>(TkSpells::SPELL_REBIRTH_PHASE2));

    if (lastRebirthState[instanceId] && !rebirthActive)
    {
        isAlarInPhase2[instanceId] = true;
        return true;
    }

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver

bool VoidReaverTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    Position const position = { 423.845f, 371.733f, 14.897f };
    float const dX = position.GetPositionX() - bot->GetPositionX();
    float const dY = position.GetPositionY() - bot->GetPositionY();
    float const distanceToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distanceToPosition < 2.0f)
        return false;

    float const moveDist = std::min(3.5f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;
    bool backwards = voidReaver->GetExactDist2d(moveX, moveY) >=
        distanceToPosition ? true : false;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event /*event*/)
{
    static std::array<char const*, 7> const spells =
    {
        "divine shield",
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool VoidReaverSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (_hasReachedVoidReaverPosition)
    {
        int healerCount = 0, rangedDpsCount = 0;
        int healerIndex = GetHealerIndex(group, healerCount);
        int rangedDpsIndex = GetRangedDpsIndex(group, rangedDpsCount);

        // Void Reaver's CombatReach is 15 yards
        constexpr float radius = 45.0f;
        float targetX = 0.0f;
        float targetY = 0.0f;

        if (healerIndex != -1 && healerCount > 0)
        {
            float angle = 2 * M_PI * healerIndex / healerCount;
            targetX = voidReaver->GetPositionX() + radius * std::cos(angle);
            targetY = voidReaver->GetPositionY() + radius * std::sin(angle);
        }
        else if (rangedDpsIndex != -1 && rangedDpsCount > 0)
        {
            float angle = 2 * M_PI * rangedDpsIndex / rangedDpsCount;
            if (healerCount > 0)
                angle += M_PI / rangedDpsCount;

            targetX = voidReaver->GetPositionX() + radius * std::cos(angle);
            targetY = voidReaver->GetPositionY() + radius * std::sin(angle);
        }

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            return MoveTo(
                TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            _hasReachedVoidReaverPosition = true;
        }
    }
    else
    {
        auto const orbIt = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
        if (orbIt != voidReaverArcaneOrbs.end())
        {
            uint32 const currentTime = getMSTime();
            constexpr uint32 orbDuration = 7000;
            constexpr float orbSafeDistance = 30.0f;

            for (auto const& orb : orbIt->second)
            {
                if (getMSTimeDiff(orb.castTime, currentTime) <= orbDuration &&
                    bot->GetExactDist2d(
                        orb.destination.GetPositionX(),
                        orb.destination.GetPositionY()) < orbSafeDistance)
                {
                    return false;
                }
            }
        }

        // No incoming arcane orbs within 30 yards
        constexpr float safeDistance = 20.0f;
        constexpr uint32 minInterval = 1000;
        if (bot->GetDistance2d(voidReaver) < safeDistance)
            return FleePosition(voidReaver->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

int VoidReaverSpreadRangedInCircleAction::GetHealerIndex(Group* group, int& healerCount)
{
    std::vector<Player*> healers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsHeal(member))
            continue;

        healers.push_back(member);
    }

    healerCount = healers.size();
    auto it = std::find(healers.begin(), healers.end(), bot);
    return (it != healers.end()) ? std::distance(healers.begin(), it) : -1;
}

int VoidReaverSpreadRangedInCircleAction::GetRangedDpsIndex(Group* group, int& rangedDpsCount)
{
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRangedDps(member))
            continue;

        rangedDps.push_back(member);
    }

    rangedDpsCount = rangedDps.size();
    auto it = std::find(rangedDps.begin(), rangedDps.end(), bot);
    return (it != rangedDps.end()) ? std::distance(rangedDps.begin(), it) : -1;
}

bool VoidReaverAvoidArcaneOrbAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    auto it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 currentTime = getMSTime();
    constexpr uint32 orbDuration = 7000;
    constexpr float orbSafeDistance = 22.0f;

    // Collect active orbs and check if any are with 22 yards
    std::vector<Position> activeOrbs;
    bool inDanger = false;
    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, currentTime) <= orbDuration)
        {
            activeOrbs.push_back(orb.destination);
            if (!inDanger && bot->GetExactDist2d(
                    orb.destination.GetPositionX(), orb.destination.GetPositionY()) <
                orbSafeDistance)
            {
                inDanger = true;
            }
        }
    }

    // Clean expired orbs
    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
        [currentTime](ArcaneOrbData const& orb) {
            return getMSTimeDiff(orb.castTime, currentTime) > orbDuration;
        }), it->second.end());

    if (!inDanger)
        return false;

    // Search radially from bot for the shortest move that satisfies:
    // 1. >= orbSafeDistance from all active orbs
    // 2. >= minDistFromBoss (20 yd) GetDistance2d from Void Reaver
    // 3. <= maxDistFromBoss (28 yd) GetDistance2d from Void Reaver
    constexpr float searchStep = M_PI / 12.0f;
    constexpr float minSearchDist = 2.0f;
    constexpr float maxSearchDist = 40.0f;
    constexpr float searchDistStep = 2.0f;
    constexpr float minDistFromBoss = 20.0f;
    constexpr float maxDistFromBoss = 28.0f;

    std::vector<Position> bestCandidates;
    float bestMoveDist = std::numeric_limits<float>::max();

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (float dist = minSearchDist; dist <= maxSearchDist; dist += searchDistStep)
    {
        if (dist > bestMoveDist)
            break;

        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float const x = botX + dist * std::cos(angle);
            float const y = botY + dist * std::sin(angle);

            float const distFromBoss = voidReaver->GetDistance2d(x, y);
            if (distFromBoss < minDistFromBoss || distFromBoss > maxDistFromBoss)
                continue;

            bool safeFromOrbs = true;
            for (auto const& orbPos : activeOrbs)
            {
                float const dx = x - orbPos.GetPositionX();
                float const dy = y - orbPos.GetPositionY();
                if (std::sqrt(dx * dx + dy * dy) < orbSafeDistance)
                {
                    safeFromOrbs = false;
                    break;
                }
            }

            if (safeFromOrbs && dist <= bestMoveDist)
            {
                if (dist < bestMoveDist)
                {
                    bestCandidates.clear();
                    bestMoveDist = dist;
                }
                bestCandidates.push_back(Position(x, y, bot->GetPositionZ()));
            }
        }
    }

    botAI->InterruptSpell();

    if (!bestCandidates.empty())
    {
        Position const& chosen = bestCandidates[urand(0, bestCandidates.size() - 1)];
        return MoveTo(
            TK_MAP_ID, chosen.GetPositionX(), chosen.GetPositionY(), chosen.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Fallback: flee from closest orb
    return FleePosition(activeOrbs[0], orbSafeDistance, 0);
}

/*
// Previous simple-flee version
bool VoidReaverAvoidArcaneOrbAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    auto it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 currentTime = getMSTime();
    constexpr uint32 orbDuration = 7000;
    constexpr float safeDistance = 22.0f;
    bool shouldFlee = false;
    Position fleeDest;

    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, currentTime) <= orbDuration)
        {
            if (bot->GetExactDist2d(
                    orb.destination.GetPositionX(), orb.destination.GetPositionY()) < safeDistance)
            {
                shouldFlee = true;
                fleeDest = orb.destination;
                break;
            }
        }
    }

    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
        [currentTime](ArcaneOrbData const& orb) {
            return getMSTimeDiff(orb.castTime, currentTime) > orbDuration;
        }), it->second.end());

    if (!shouldFlee)
        return false;

    constexpr uint32 minInterval = 0;
    botAI->InterruptSpell();
    return FleePosition(fleeDest, safeDistance, minInterval);
}
*/

// High Astromancer Solarian

bool HighAstromancerSolarianPositionRangedAction::Execute(Event /*event*/)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    float currentDistance = bot->GetExactDist2d(astromancer);
    constexpr float minDistance = 20.0f;
    if (currentDistance < minDistance)
        return MoveAway(astromancer, minDistance - currentDistance);

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (!GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    botAI->InterruptSpell();
    return MoveFromGroup(safeDistance);
}

bool HighAstromancerSolarianStackForAoeAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* stackTarget = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsRanged(member))
        {
            stackTarget = member;
            break;
        }
    }

    if (!stackTarget || bot == stackTarget || bot->GetExactDist2d(stackTarget) < 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, stackTarget->GetPositionX(), stackTarget->GetPositionY(),
        stackTarget->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Split melee into two groups, one on each Solarium Priest
bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event /*event*/)
{
    auto priestsPair = GetTargetUnitPair(botAI, static_cast<uint32>(TkNpcs::NPC_SOLARIUM_PRIEST));
    if (!priestsPair.first || !priestsPair.second)
        return false;

    Unit* targetPriest = AssignSolariumPriestsToBots(priestsPair, GetMeleeBots());
    if (!targetPriest)
        return false;

    if (AI_VALUE(Unit*, "current target") != targetPriest)
        return Attack(targetPriest);

    return false;
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots()
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && GET_PLAYERBOT_AI(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    std::pair<Unit*, Unit*> const& priestsPair, std::vector<Player*> const& meleeMembers)
{
    if (!priestsPair.first || !priestsPair.second || meleeMembers.empty())
        return nullptr;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (totalMelee == 1)
        return priestsPair.first;

    size_t split = totalMelee / 2;

    if (botIndex < split)
        return priestsPair.first;
    else
        return priestsPair.second;
}

bool HighAstromancerSolarianCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || mainTank->HasAura(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD)))
        return false;

    if (!botAI->CanCastSpell(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD), mainTank))
        return false;

    return botAI->CastSpell(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD), mainTank);
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event /*event*/)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    constexpr float safeDistance = 15.0f;
    float const currentDistance = bot->GetDistance2d(thaladred);
    if (currentDistance < safeDistance)
        return MoveAway(thaladred, safeDistance - currentDistance);

    return false;
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again for good measure)
bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* advisorTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0 || hunterIndex == 2)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(bot);
    }
    else if (hunterIndex == 1)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        tankTarget = GetGroupAssistTank(botAI, bot, 0);
    }

    if (!advisorTarget || advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || IsFeigningDeath(advisorTarget))
    {
        return false;
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", advisorTarget))
    {
        return botAI->CastSpell("steady shot", advisorTarget);
    }

    return false;
}

bool KaelthasSunstriderMainTankPositionSanguinarAction::Execute(Event /*event*/)
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    if (MarkTargetWithStar(bot, sanguinar))
        return true;

    SetRtiTarget(botAI, "star", sanguinar);

    if (AI_VALUE(Unit*, "current target") != sanguinar)
        return Attack(sanguinar);

    if (sanguinar->GetVictim() == bot && bot->IsWithinMeleeRange(sanguinar))
    {
        Position const position = SANGUINAR_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || mainTank->HasAura(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD)))
        return false;

    if (!botAI->CanCastSpell(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD), mainTank))
        return false;

    return botAI->CastSpell(static_cast<uint32>(TkSpells::SPELL_FEAR_WARD), mainTank);
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    if (MarkTargetWithCircle(bot, capernian))
        return true;

    SetRtiTarget(botAI, "circle", capernian);

    if (AI_VALUE(Unit*, "current target") != capernian &&
        botAI->CanCastSpell("searing pain", capernian) &&
        botAI->CastSpell("searing pain", capernian))
        return true;

    if (capernian->GetVictim() == bot)
    {
        float currentDist = bot->GetDistance2d(capernian);
        constexpr float minDistance = 28.0f;
        if (currentDist < minDistance)
            return MoveAway(capernian, minDistance - currentDist);
    }

    if (botAI->CanCastSpell("searing pain", capernian))
        return botAI->CastSpell("searing pain", capernian);

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (botAI->IsRanged(bot) && capernian->GetVictim() != bot &&
        RangedBotsDisperse(kaelAI, capernian))
    {
        return true;
    }
    else if (botAI->IsMelee(bot) && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR &&
             MeleeStayBackFromCapernian(capernian))
    {
        return true;
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(
    boss_kaelthas* kaelAI, Unit* capernian)
{
    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (botAI->IsHeal(member))
                healers.push_back(member);
            else
                rangedDps.push_back(member);
        }

        if (healers.empty() && rangedDps.empty())
            return false;

        size_t count = healers.size() + rangedDps.size();
        size_t botIndex = 0;
        float radius = 0.0f;
        float angle = 0.0f;

        // Spread is 90-degree arc for healers and 120-degree arc for ranged DPS
        float arcSpan = botAI->IsHeal(bot) ? M_PI / 2.0f : 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.9f;
        float arcStart = arcCenter - arcSpan / 2.0f;

        // Capernian's hitbox is 4.5 yards (GetDistance2d of 6.0f)
        if (botAI->IsHeal(bot))
        {
            auto findIt = std::find(healers.begin(), healers.end(), bot);
            botIndex = (findIt != healers.end()) ? std::distance(healers.begin(), findIt) : 0;
            radius = 42.0f;
            count = healers.size();
        }
        else
        {
            auto findIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
            botIndex = (findIt != rangedDps.end()) ? std::distance(rangedDps.begin(), findIt) : 0;
            radius = 34.0f;
            count = rangedDps.size();
        }

        angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

        float targetX = capernian->GetPositionX() + radius * std::cos(angle);
        float targetY = capernian->GetPositionY() + radius * std::sin(angle);

        if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;

        float const safeDistance = 6.0f;
        constexpr uint32 minInterval = 1000;
        if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
            return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(Unit* capernian)
{
    // Main tank purposely stays in range to bait Conflagration in Phase 1
    if (botAI->IsMainTank(bot))
    {
        // MoveTo called for a WorldObj is a GetDistance() check so both hitboxes are accounted for
        constexpr float desiredDist = 15.0f;
        botAI->Reset();
        return MoveTo(capernian, desiredDist, MovementPriority::MOVEMENT_FORCED);
    }
    else
    {
        constexpr float safeDistance = 42.0f;
        float currentDistance = bot->GetDistance2d(capernian);
        if (currentDistance < safeDistance)
        {
            botAI->Reset();
            return MoveAway(capernian, safeDistance - currentDistance);
        }
        else
        {
            return true;
        }
    }
}

bool KaelthasSunstriderFirstAssistTankPositionTelonicusAction::Execute(Event /*event*/)
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    if (MarkTargetWithTriangle(bot, telonicus))
        return true;

    SetRtiTarget(botAI, "triangle", telonicus);

    if (AI_VALUE(Unit*, "current target") != telonicus)
        return Attack(telonicus);

    if (telonicus->GetVictim() == bot && bot->IsWithinMeleeRange(telonicus))
    {
        Position const position = TELONICUS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event /*event*/)
{
    const Position* movePosition = nullptr;
    if (botAI->IsAssistHealOfIndex(bot, 0, true))
    {
        movePosition = &ADVISOR_HEAL_POSITION;
    }
    else if (botAI->IsMainTank(bot))
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        if (sanguinar && sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &SANGUINAR_WAITING_POSITION;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        if (telonicus && telonicus->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &TELONICUS_WAITING_POSITION;
    }
    else if (GetCapernianTank(bot) == bot)
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        if (capernian && capernian->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            movePosition = &CAPERNIAN_WAITING_POSITION;
    }

    if (movePosition &&
        bot->GetExactDist2d(movePosition->GetPositionX(), movePosition->GetPositionY()) > 2.0f)
    {
        return MoveTo(TK_MAP_ID, movePosition->GetPositionX(), movePosition->GetPositionY(),
                        movePosition->GetPositionZ(), false, false, false, false,
                        MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool KaelthasSunstriderReequipGearAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("equip upgrade", Event(), true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Thaladred, except Capernian tank
    Player* capernianTank = GetCapernianTank(bot);
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");

    if ((!capernianTank || bot != capernianTank) &&
        thaladred && !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(thaladred))
    {
        if (MarkTargetWithSquare(bot, thaladred))
            return true;

        SetRtiTarget(botAI, "square", thaladred);

        if (AI_VALUE(Unit*, "current target") != thaladred)
            return Attack(thaladred);

        return false;
    }

    // Target priority 2: Capernian for ranged only (excluding longbow tank)
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");

    if (botAI->IsRangedDps(bot) && !IsDebuffHunter(bot) &&
        capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian))
    {
        SetRtiTarget(botAI, "circle", capernian);

        if (AI_VALUE(Unit*, "current target") != capernian)
            return Attack(capernian);

        return false;
    }

    // Target priority 3: Sanguinar (debuff hunter and melee move here after Thaladred)
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");

    if (sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(sanguinar))
    {
        SetRtiTarget(botAI, "star", sanguinar);

        if (AI_VALUE(Unit*, "current target") != sanguinar)
            return Attack(sanguinar);

        return false;
    }

    // Target priority 4: Telonicus
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    if (telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(telonicus))
    {
        SetRtiTarget(botAI, "triangle", telonicus);
        if (AI_VALUE(Unit*, "current target") != telonicus)
            return Attack(telonicus);

        // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs
        if (botAI->IsMelee(bot) && botAI->IsDps(bot) && telonicus->GetVictim() != bot)
        {
            float desiredDist = bot->GetMeleeRange(telonicus);
            float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
            float targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
            float targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

            if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
            {
                return MoveTo(TK_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false,
                              false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }

    return false;
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    static std::array<char const*, 3> const advisorNames =
    {
        "grand astromancer capernian",
        "master engineer telonicus",
        "lord sanguinar"
    };

    for (char const* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            time_t const now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(kaelthas->GetMap()->GetInstanceId(), now);
            return true;
        }
    }

    return false;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event /*event*/)
{
    if (botAI->IsAssistTank(bot))
        SetRtiTarget(botAI, "moon", nullptr);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe
    // But this applies to assist tanks only after they get aggro on the mace, dagger, or sword
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");

    if (axe)
    {
        bool hasAggroFromWeapon = (mace && mace->GetVictim() == bot) ||
                                  (dagger && dagger->GetVictim() == bot) ||
                                  (sword && sword->GetVictim() == bot);
        if (!botAI->IsTank(bot) ||
            (botAI->IsAssistTank(bot) && hasAggroFromWeapon))
        {
            float currentDistance = bot->GetExactDist2d(axe);
            float const safeDistance = botAI->IsAssistTank(bot) ? 17.0f : 13.0f;
            if (currentDistance < safeDistance)
                return MoveAway(axe, safeDistance - currentDistance);
        }
    }

    if (botAI->IsDps(bot))
    {
        // Priority 1: Staff of Disintegration (Skull)
        if (Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration"))
        {
            if (MarkTargetWithSkull(bot, staff))
                return true;

            SetRtiTarget(botAI, "skull", staff);

            if (AI_VALUE(Unit*, "current target") != staff)
                return Attack(staff);
        }
        // Priority 2: Cosmic Infuser (Skull)
        else if (mace)
        {
            if (MarkTargetWithSkull(bot, mace))
                return true;

            SetRtiTarget(botAI, "skull", mace);

            if (AI_VALUE(Unit*, "current target") != mace)
                return Attack(mace);
        }
        // Priority 3: Warp Slicer (Skull)
        else if (sword)
        {
            if (MarkTargetWithSkull(bot, sword))
                return true;

            SetRtiTarget(botAI, "skull", sword);

            if (AI_VALUE(Unit*, "current target") != sword)
                return Attack(sword);
        }
        // Priority 4: Infinity Blades (Skull)
        else if (dagger)
        {
            if (MarkTargetWithSkull(bot, dagger))
                return true;

            SetRtiTarget(botAI, "skull", dagger);

            if (AI_VALUE(Unit*, "current target") != dagger)
                return Attack(dagger);
        }
        // Priority 5: Devastation - ranged only (Diamond--marked in other method by main tank)
        else if (axe && botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "diamond", axe);

            if (AI_VALUE(Unit*, "current target") != axe)
                return Attack(axe);
        }
        // Priority 6: Netherstrand Longbow (Skull)
        else if (Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow"))
        {
            if (MarkTargetWithSkull(bot, longbow))
                return true;

            SetRtiTarget(botAI, "skull", longbow);

            if (AI_VALUE(Unit*, "current target") != longbow)
                return Attack(longbow);
        }
        // Priority 7: Phaseshift Bulwark (Skull)
        else if (Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark"))
        {
            if (MarkTargetWithSkull(bot, shield))
                return true;

            SetRtiTarget(botAI, "skull", shield);

            if (AI_VALUE(Unit*, "current target") != shield)
                return Attack(shield);
        }
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event /*event*/)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (!axe)
        return false;

    if (MarkTargetWithDiamond(bot, axe))
        return true;

    SetRtiTarget(botAI, "diamond", axe);

    if (AI_VALUE(Unit*, "current target") != axe)
        return Attack(axe);

    constexpr float safeDistance = 13.0f;
    if (axe->GetVictim() == bot && GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return MoveFromGroup(safeDistance);

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event /*event*/)
{
    static std::array<WeaponInfo, 7> const weapons =
    {
        WeaponInfo{ TkNpcs::NPC_NETHERSTRAND_LONGBOW, TkItems::ITEM_NETHERSTRAND_LONGBOW },
        WeaponInfo{ TkNpcs::NPC_COSMIC_INFUSER, TkItems::ITEM_COSMIC_INFUSER },
        WeaponInfo{ TkNpcs::NPC_DEVASTATION, TkItems::ITEM_DEVASTATION },
        WeaponInfo{ TkNpcs::NPC_INFINITY_BLADES, TkItems::ITEM_INFINITY_BLADE },
        WeaponInfo{ TkNpcs::NPC_WARP_SLICER, TkItems::ITEM_WARP_SLICER },
        WeaponInfo{ TkNpcs::NPC_STAFF_OF_DISINTEGRATION, TkItems::ITEM_STAFF_OF_DISINTEGRATION },
        WeaponInfo{ TkNpcs::NPC_PHASESHIFT_BULWARK, TkItems::ITEM_PHASESHIFT_BULWARK },
    };

    for (auto const& weapon : weapons)
    {
        if (ShouldBotLootWeapon(weapon.npcEntry))
        {
            if (bot->HasItemCount(static_cast<uint32>(weapon.itemId), 1, false))
            {
                EquipAction* equipAction =
                    dynamic_cast<EquipAction*>(botAI->GetAiObjectContext()->GetAction("equip"));
                if (equipAction)
                {
                    ItemIds ids;
                    ids.insert(static_cast<uint32>(weapon.itemId));
                    equipAction->EquipItems(ids);
                }
                continue;
            }
            return LootWeapon(static_cast<uint32>(weapon.npcEntry), static_cast<uint32>(weapon.itemId));
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(TkNpcs weaponEntry)
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    switch (weaponEntry)
    {
        case TkNpcs::NPC_NETHERSTRAND_LONGBOW:
            return bot->getClass() == CLASS_HUNTER;

        case TkNpcs::NPC_COSMIC_INFUSER:
            return botAI->IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at appropriate gear levels
        // So IMO they're better off looting only the dagger to MH it and break MCs
        case TkNpcs::NPC_DEVASTATION:
            return (bot->getClass() == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) ||
                   (bot->getClass() == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
                   (botAI->IsDps(bot) && bot->getClass() == CLASS_DEATH_KNIGHT);

        case TkNpcs::NPC_INFINITY_BLADES:
            return bot->getClass() == CLASS_ROGUE ||
                   bot->getClass() == CLASS_HUNTER ||
                   (bot->getClass() == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT) ||
                   (bot->getClass() == CLASS_WARRIOR && tab != WARRIOR_TAB_ARMS);

        case TkNpcs::NPC_WARP_SLICER:
            return (bot->getClass() == CLASS_ROGUE && tab != ROGUE_TAB_ASSASSINATION) ||
                   (botAI->IsTank(bot) &&
                    (bot->getClass() == CLASS_DEATH_KNIGHT ||
                     bot->getClass() == CLASS_PALADIN));

        case TkNpcs::NPC_STAFF_OF_DISINTEGRATION:
            return (botAI->IsRangedDps(bot) && bot->getClass() != CLASS_HUNTER) ||
                   (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL);

        case TkNpcs::NPC_PHASESHIFT_BULWARK:
            return botAI->IsTank(bot) &&
                   (bot->getClass() == CLASS_PALADIN ||
                    bot->getClass() == CLASS_WARRIOR ||
                    bot->getClass() == CLASS_DEATH_KNIGHT);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(
    uint32 weaponEntry, uint32 itemId)
{
    constexpr float searchRadius = 150.0f;
    Creature* weapon = bot->FindNearestCreature(weaponEntry, searchRadius, false);

    if (!weapon || weapon->IsAlive())
        return false;

    LootObject loot(bot, weapon->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    float const maxLootRange = sPlayerbotAIConfig.lootDistance;
    constexpr float distFromObject = 2.0f;

    if (bot->GetDistance(weapon) > maxLootRange)
        return MoveTo(weapon, distFromObject, MovementPriority::MOVEMENT_COMBAT);

    OpenLootAction open(botAI);
    bool opened = open.Execute(Event());
    if (!opened)
        return opened;

    if (bot->HasItemCount(itemId, 1, false))
        return false;

    bot->SetLootGUID(weapon->GetGUID());

    constexpr uint8 weaponIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << weaponIndex;
    bot->GetSession()->QueuePacket(packet);

    return true;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event /*event*/)
{
    return UsePhaseshiftBulwark() ||
           UseStaffOfDisintegration() ||
           UseNetherstrandLongbow();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != static_cast<uint32>(TkItems::ITEM_PHASESHIFT_BULWARK))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(static_cast<uint32>(TkSpells::SPELL_SHOCK_BARRIER)))
        return false;

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_ARCANE_BARRIER)) ||
        bot->CanUseItem(offHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand ||
        mainHand->GetEntry() != static_cast<uint32>(TkItems::ITEM_STAFF_OF_DISINTEGRATION))
        return false;

    if (bot->HasAura(static_cast<uint32>(TkSpells::SPELL_MENTAL_PROTECTION_FIELD)) ||
        bot->CanUseItem(mainHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != static_cast<uint32>(TkItems::ITEM_NETHERSTRAND_LONGBOW))
        return false;

    if (bot->HasItemCount(static_cast<uint32>(TkItems::ITEM_NETHER_SPIKES), 1, false) ||
        bot->CanUseItem(ranged) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(ranged);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseEquippedItemWithPacket(Item* item)
{
    if (!item || bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    uint32 targetFlag = TARGET_FLAG_UNIT;
    packet << targetFlag << bot->GetPackGUID();

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

bool KaelthasSunstriderMainTankPositionBossAction::Execute(Event /*event*/)
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (MarkTargetWithStar(bot, kaelthas))
        return true;

    SetRtiTarget(botAI, "star", kaelthas);

    if (AI_VALUE(Unit*, "current target") != kaelthas)
        return Attack(kaelthas);

    if (kaelthas->GetVictim() == bot && bot->IsWithinMeleeRange(kaelthas))
    {
        Position const position = KAELTHAS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 4.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;
    std::vector<Unit*> flameStrikes = GetAllHazardTriggers(
        bot, static_cast<uint32>(TkNpcs::NPC_FLAME_STRIKE_TRIGGER), searchRadius);

    if (flameStrikes.empty())
        return false;

    constexpr float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameStrike : flameStrikes)
    {
        if (bot->GetExactDist2d(flameStrike) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Position safestPos = FindSafestNearbyPosition(bot, flameStrikes, hazardRadius);

    botAI->Reset();
    return MoveTo(TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::Execute(Event /*event*/)
{
    if (botAI->IsAssistTankOfIndex(bot, 0, true) || botAI->IsAssistTankOfIndex(bot, 1, false))
        return AssistTanksPickUpPhoenixes();

    return NonTanksDestroyEggsAndAvoidPhoenixes();
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::AssistTanksPickUpPhoenixes()
{
    std::vector<Unit*> phoenixes;
    auto const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == static_cast<uint32>(TkNpcs::NPC_PHOENIX))
            phoenixes.push_back(unit);
    }

    if (phoenixes.empty())
        return false;

    std::sort(phoenixes.begin(), phoenixes.end(),
              [](Unit* first, Unit* second) { return first->GetGUID() < second->GetGUID(); });

    Unit* targetPhoenix = nullptr;
    if (botAI->IsAssistTankOfIndex(bot, 0, true))
    {
        targetPhoenix = phoenixes[0];

        if (MarkTargetWithSquare(bot, targetPhoenix))
            return true;

        SetRtiTarget(botAI, "square", targetPhoenix);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 1, false) && phoenixes.size() >= 2)
    {
        targetPhoenix = phoenixes[1];

        if (MarkTargetWithCircle(bot, targetPhoenix))
            return true;

        SetRtiTarget(botAI, "circle", targetPhoenix);
    }

    if (!targetPhoenix)
        return false;

    if (AI_VALUE(Unit*, "current target") != targetPhoenix)
        return Attack(targetPhoenix);

    constexpr float safeDistance = 12.0f;
    if (targetPhoenix->GetVictim() == bot &&
        GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return MoveFromGroup(safeDistance);

    return false;
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::NonTanksDestroyEggsAndAvoidPhoenixes()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (botAI->IsDps(bot) &&
        !kaelthas->HasAura(static_cast<uint32>(TkSpells::SPELL_SHOCK_BARRIER)))
    {
        if (Unit* phoenixEgg = GetFirstAliveUnitByEntry(botAI, static_cast<uint32>(TkNpcs::NPC_PHOENIX_EGG)))
        {
            if (MarkTargetWithDiamond(bot, phoenixEgg))
                return true;

            SetRtiTarget(botAI, "diamond", phoenixEgg);

            if (AI_VALUE(Unit*, "current target") != phoenixEgg)
                return Attack(phoenixEgg);
        }
    }
    else if (botAI->IsDps(bot))
        return false;

    if (Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix"))
    {
        float currentDistance = bot->GetExactDist2d(phoenix);
        constexpr float safeDistance = 12.0f;
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance);
    }

    return false;
}

bool KaelthasSunstriderBreakMindControlAction::Execute(Event /*event*/)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(static_cast<uint32>(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < closestDist)
            {
                closestDist = dist;
                mcTarget = member;
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(TK_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(),
                      mcTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (bot->getClass() == CLASS_ROGUE &&
        AiFactory::GetPlayerSpecTab(bot) != ROGUE_TAB_COMBAT &&
        botAI->CanCastSpell("sinister strike", mcTarget))
    {
        return botAI->CastSpell("sinister strike", mcTarget);
    }
    else
    {
        static std::array<char const*, 4> const spells =
        {
            "hamstring",
            "wing clip",
            "shiv",
            "stormstrike"
        };
        for (char const* spell : spells)
        {
            if (botAI->CanCastSpell(spell, mcTarget))
                return botAI->CastSpell(spell, mcTarget);
        }
    }

    return false;
}

// Shock Barrier needs to be #1 focus, even if there is a Phoenix Egg up
bool KaelthasSunstriderBreakThroughShockBarrierAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(static_cast<uint32>(TkSpells::SPELL_SHOCK_BARRIER)))
    {
        static std::array<char const*, 8> const spells =
        {
            "bash",
            "counterspell",
            "kick",
            "mind freeze",
            "pummel",
            "shield bash",
            "silencing shot",
            "wind shear",
        };
        for (char const* spell : spells)
        {
            if (botAI->CanCastSpell(spell, kaelthas))
                return botAI->CastSpell(spell, kaelthas);
        }
    }
    else if (AI_VALUE(Unit*, "current target") != kaelthas)
    {
        SetRtiTarget(botAI, "star", kaelthas);
        return Attack(kaelthas);
    }

    return false;
}

bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float minSpreadDistance = 16.0f;

    std::vector<Player*> nearbyPlayers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (bot->IsWithinDist3d(member, minSpreadDistance * 1.0f))
            nearbyPlayers.push_back(member);
    }

    if (nearbyPlayers.empty())
        return false;

    Player* closestPlayer = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (Player* player : nearbyPlayers)
    {
        float distToPlayer = bot->GetExactDist(player);
        if (distToPlayer < closestDist)
        {
            closestDist = distToPlayer;
            closestPlayer = player;
        }
    }

    if (closestPlayer && closestDist < minSpreadDistance)
    {
        float angle = bot->GetAngle(closestPlayer) + M_PI;
        float distance = minSpreadDistance - closestDist;

        float x = bot->GetPositionX() + std::cos(angle) * distance;
        float y = bot->GetPositionY() + std::sin(angle) * distance;

        return MoveTo(TK_MAP_ID, x, y, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
