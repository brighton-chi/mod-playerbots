/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKActions.h"
#include "AiFactory.h"
#include "EquipAction.h"
#include "ItemPackets.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "StatsWeightCalculator.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"

using namespace TkHelpers;

// General

bool TempestKeepResetEncounterStatesAction::Execute(Event /*event*/)
{
    // Some weird bug with Solarian applies this aura to players
    // It doesn't seem to have any effect, but it should be removed anyway
    if (bot->HasAura(Id(TkSpells::SPELL_SELECT_TRUE_BEAM)))
        bot->RemoveAura(Id(TkSpells::SPELL_SELECT_TRUE_BEAM));

    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    bool reset = false;

    if (!AI_VALUE2(Unit*, "find target", "alar"))
    {
        if (isAlarInPhase2.erase(instanceId) > 0)
            reset = true;

        if (lastRebirthState.erase(instanceId) > 0)
            reset = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "void reaver") &&
        voidReaverArcaneOrbs.erase(instanceId) > 0)
    {
        reset = true;
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
        centurions, Id(TkNpcs::NPC_CRIMSON_HAND_CENTURION), searchRadius);

    for (Creature* centurion : centurions)
    {
        if (!centurion || !centurion->IsAlive() ||
            !centurion->HasAura(Id(TkSpells::SPELL_ARCANE_FLURRY)) ||
            botAI->HasAura("polymorph", centurion))
        {
            continue;
        }

        if (!target || centurion->GetGUID() < target->GetGUID())
            target = centurion;
    }

    if (!target)
        return false;

    if (!botAI->CanCastSpell("polymorph", target))
        return false;

    botAI->InterruptSpell();
    return botAI->CastSpell("polymorph", target);
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

    if (bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", alar))
    {
        return botAI->CastSpell("steady shot", alar);
    }

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    bool isMainTank = PlayerbotAI::IsMainTank(bot);
    bool isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
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

    Position const& target = PLATFORM_POSITIONS[platformIdx];
    if (bot->GetExactDist2d(target) < 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
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

    Position const& target = PLATFORM_POSITIONS[locationIndex];
    if (bot->GetExactDist2d(target) < 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event /*event*/)
{
    bool isRanged = PlayerbotAI::IsRanged(bot);
    bool isEmberTank = PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
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

    Position const& position = GROUND_POSITIONS[locationIndex];
    if (isRanged)
    {
        constexpr float distFromTarget = 8.0f;
        if (bot->GetExactDist2d(position) > distFromTarget)
        {
            return MoveInside(
                TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                position.GetPositionZ(), distFromTarget, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    else if (isEmberTank && !AI_VALUE2(Unit*, "find target", "ember of al'ar"))
    {
        constexpr float distFromTarget = 20.0f;
        if (bot->GetExactDist2d(position) > distFromTarget)
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
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return false;

    Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (!bot->IsWithinMeleeRange(ember))
    {
        return MoveTo(
            TK_MAP_ID, ember->GetPositionX(), ember->GetPositionY(), ember->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (ember->GetVictim() != bot)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    Position const& position = GROUND_POSITIONS[locationIndex];
    Position const& center = ALAR_POINT_MIDDLE;
    float dx = center.GetPositionX() - position.GetPositionX();
    float dy = center.GetPositionY() - position.GetPositionY();
    float distToCenter = position.GetExactDist2d(center);

    constexpr float moveDist = 26.0f;
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
    auto const& [firstEmber, secondEmber] =
        GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        ember = firstEmber;
    else if (GetSecondEmberTank(bot) == bot)
        ember = secondEmber;

    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (ember->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    constexpr float safeDistance = 17.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool AlarRangedDpsPrioritizeEmbersAction::Execute(Event /*event*/)
{
    auto const& [firstEmber, secondEmber] =
        GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (firstEmber)
        ember = firstEmber;
    else if (secondEmber)
        ember = secondEmber;

    if (ember)
    {
        constexpr float safeDistance = 15.0f;
        float const currentDistance = bot->GetDistance2d(ember);
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

    constexpr float distMeleeFromPos = 5.0f;
    constexpr float distRangedFromPos = 10.0f;

    if (PlayerbotAI::IsMainTank(bot) && bot->GetExactDist2d(
        ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY()) > distMeleeFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
            ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && bot->GetExactDist2d(
        ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY()) > distMeleeFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
            ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) && bot->GetExactDist2d(
        ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > distMeleeFromPos)
    {
        return MoveTo(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (PlayerbotAI::IsMelee(bot) && bot->GetExactDist2d(
        ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY()) > distMeleeFromPos)
    {
        return MoveInside(
            TK_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
            ALAR_ROOM_S_CENTER.GetPositionZ(), distMeleeFromPos,
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

    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
    {
        Creature* alarCreature = alar->ToCreature();
        if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
            return false;
    }

    // Per above, ranged/tanks wait until Al'ar actually "dies," while melee dps jumps off at 5% HP
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

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetDistance2d(alar);
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(alar, safeDistance - currentDistance);
}

// Main tank and first assist tank will swap tanking Al'ar when Melt Armor is applied
bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    if (!PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // secondEmberTank = whichever of MT or 1st AT doesn't have Melt Armor (1st AT if neither do)
    Player* secondEmberTank = GetSecondEmberTank(bot);
    if (secondEmberTank == bot)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    if (alar->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

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
        GetAllHazardTriggers(bot, Id(TkNpcs::NPC_FLAME_PATCH), searchRadius);

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
    if (alar->HasAura(Id(TkSpells::SPELL_MODEL_INVISIBILITY)) ||
        (alar->HasUnitState(UNIT_STATE_CASTING) &&
         alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_DIVE))))
    {
        constexpr float safeDistance = 20.0f;
        float const currentDistance = bot->GetDistance2d(alar);
        if (currentDistance >= safeDistance)
            return false;

        botAI->InterruptSpell();
        return MoveAway(alar, safeDistance - currentDistance);
    }

    // During Dive Bomb sequence
    Position dest;
    if (GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar, dest) != POINT_QUILL_OR_DIVE_IDX)
    {
        return false;
    }

    constexpr float safeDistance = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    float const currentDistance = bot->GetDistance2d(nearestPlayer);
    return MoveAway(nearestPlayer, safeDistance - currentDistance);
}

bool AlarManagePhaseTrackerAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 const instanceId = alar->GetMap()->GetInstanceId();

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
        alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_PHASE2));

    if (!isAlarInPhase2[instanceId] && lastRebirthState[instanceId] && !rebirthActive)
    {
        isAlarInPhase2[instanceId] = true;
        return true;
    }

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver
// CombatReach is 15 yards

bool VoidReaverTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    if (AI_VALUE(Unit*, "current target") != voidReaver)
        return Attack(voidReaver);

    if (voidReaver->GetVictim() != bot || !bot->IsWithinMeleeRange(voidReaver))
        return false;

    Position const& position = VOID_REAVER_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = voidReaver->GetPositionX() - botX;
    float const toBossY = voidReaver->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event /*event*/)
{
    static constexpr std::array spells = {
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

bool VoidReaverKeepRangedInGoldilocksZoneAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    constexpr float minDistFromBoss = 38.5f; // 22.0f GetDistance2d()
    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist2d(voidReaver) < minDistFromBoss)
        return FleePosition(voidReaver->GetPosition(), minDistFromBoss, minInterval);

    if (voidReaver->GetHealthPct() > 90.0f)
        return false;

    // Maintain small spread after pull to discourage clumping when avoiding orbs
    constexpr float minDistFromPlayer = 3.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistFromPlayer);
}

bool VoidReaverAvoidArcaneOrbAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    auto it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    constexpr uint32 orbDuration = 7000;
    constexpr float orbSafeDistance = 22.0f;
    uint32 const now = getMSTime();

    std::vector<Position> activeOrbs;
    bool inDanger = false;
    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, now) > orbDuration)
            continue;

        activeOrbs.push_back(orb.destination);
        if (!inDanger && bot->GetExactDist2d(
                orb.destination.GetPositionX(), orb.destination.GetPositionY()) < orbSafeDistance)
        {
            inDanger = true;
        }
    }

    if (!inDanger)
        return false;

    constexpr float searchStep = M_PI / 12.0f;
    constexpr float minSearchDist = 1.0f;
    constexpr float maxSearchDist = 40.0f;
    constexpr float searchDistStep = 1.0f;
    constexpr float minDistFromBoss = 20.5f;
    constexpr float maxDistFromBoss = 28.5f;

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

    constexpr uint32 minInterval = 0;
    return FleePosition(activeOrbs[0], orbSafeDistance, minInterval);
}

// High Astromancer Solarian

bool HighAstromancerSolarianMainTankPickUpBossAction::Execute(Event /*event*/)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return AI_VALUE(Unit*, "current target") != astromancer && Attack(astromancer);
}

bool HighAstromancerSolarianStackOnRangedLeaderAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* rangedLeader = GetRangedLeader(bot);
    if (!rangedLeader || bot == rangedLeader)
        return false;

    if (bot->GetExactDist2d(rangedLeader) <= 5.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, rangedLeader->GetPositionX(), rangedLeader->GetPositionY(),
        rangedLeader->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (!GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    botAI->InterruptSpell();
    return MoveFromGroup(safeDistance);
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event /*event*/)
{
    auto const& priestsPair = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_SOLARIUM_PRIEST));
    if (!priestsPair.first || !priestsPair.second)
        return false;

    if (PlayerbotAI::IsRanged(bot) && !AI_VALUE2(Unit*, "find target", "solarium agent"))
    {
        return AI_VALUE(Unit*, "current target") != priestsPair.first &&
            Attack(priestsPair.first);
    }

    // Split melee into two groups, one on each Solarium Priest
    Unit* targetPriest = AssignSolariumPriestsToMeleeBots(priestsPair, GetMeleeBots());
    if (!targetPriest)
        return false;

    return AI_VALUE(Unit*, "current target") != targetPriest && Attack(targetPriest);
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
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToMeleeBots(
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
    if (!mainTank || mainTank->HasAura(Id(TkSpells::SPELL_FEAR_WARD)))
        return false;

    if (!botAI->CanCastSpell(Id(TkSpells::SPELL_FEAR_WARD), mainTank))
        return false;

    return botAI->CastSpell(Id(TkSpells::SPELL_FEAR_WARD), mainTank);
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event /*event*/)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    constexpr float safeDistance = 15.0f;
    float const currentDistance = bot->GetDistance2d(thaladred);
    if (currentDistance > safeDistance)
        return false;

    botAI->InterruptSpell();
    return MoveAway(thaladred, safeDistance - currentDistance);
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

    if (bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)) &&
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

    if (AI_VALUE(Unit*, "current target") != sanguinar)
        return Attack(sanguinar);

    if (sanguinar->GetVictim() != bot || !bot->IsWithinMeleeRange(sanguinar))
        return false;

    Position const& position = SANGUINAR_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = sanguinar->GetPositionX() - botX;
    float const toBossY = sanguinar->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || mainTank->HasAura(Id(TkSpells::SPELL_FEAR_WARD)))
        return false;

    if (!botAI->CanCastSpell(Id(TkSpells::SPELL_FEAR_WARD), mainTank))
        return false;

    return botAI->CastSpell(Id(TkSpells::SPELL_FEAR_WARD), mainTank);
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    char const* const searingPain = "searing pain";

    if (AI_VALUE(Unit*, "current target") != capernian &&
        botAI->CanCastSpell(searingPain, capernian) &&
        botAI->CastSpell(searingPain, capernian))
    {
        return true;
    }

    if (capernian->GetVictim() == bot)
    {
        constexpr float minDistance = 28.0f;
        float const currentDist = bot->GetDistance2d(capernian);
        if (currentDist < minDistance)
            return MoveAway(capernian, minDistance - currentDist);
    }

    return botAI->CanCastSpell(searingPain, capernian) &&
        botAI->CastSpell(searingPain, capernian);
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

    if (PlayerbotAI::IsRanged(bot) && capernian->GetVictim() != bot &&
        RangedBotsDisperse(kaelAI, capernian))
    {
        return true;
    }

    if (PlayerbotAI::IsMelee(bot) && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR &&
        MeleeStayBackFromCapernian(capernian))
    {
        return true;
    }

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(
    boss_kaelthas* kaelAI, Unit* capernian)
{
    if (kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;

        constexpr float safeDistance = 6.0f;
        Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (!nearestPlayer)
            return false;

        return FleePosition(nearestPlayer->GetPosition(), safeDistance);
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
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
    float arcSpan = PlayerbotAI::IsHeal(bot) ? M_PI / 2.0f : 2.0f * M_PI / 3.0f;
    constexpr float arcCenter = 2.9f;
    float arcStart = arcCenter - arcSpan / 2.0f;

    // Capernian's CombatReach is 4.5y
    if (PlayerbotAI::IsHeal(bot))
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

    if (bot->GetExactDist2d(targetX, targetY) < 1.0f)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
        false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(
    Unit* capernian)
{
    // Main tank purposely stays in range to bait Conflagration in Phase 1
    if (PlayerbotAI::IsMainTank(bot))
    {
        constexpr float targetDist = 20.0f;
        float const angle = capernian->GetAngle(bot);
        float const destX = capernian->GetPositionX() + std::cos(angle) * targetDist;
        float const destY = capernian->GetPositionY() + std::sin(angle) * targetDist;

        return MoveTo(
            TK_MAP_ID, destX, destY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        constexpr float safeDistance = 42.0f;
        float const currentDistance = bot->GetDistance2d(capernian);
        if (currentDistance >= safeDistance)
            return true;

        botAI->InterruptSpell();
        return MoveAway(capernian, safeDistance - currentDistance);
    }
}

bool KaelthasSunstriderFirstAssistTankPositionTelonicusAction::Execute(Event /*event*/)
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    if (AI_VALUE(Unit*, "current target") != telonicus)
        return Attack(telonicus);

    if (telonicus->GetVictim() != bot || !bot->IsWithinMeleeRange(telonicus))
        return false;

    Position const& position = TELONICUS_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = telonicus->GetPositionX() - botX;
    float const toBossY = telonicus->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event /*event*/)
{
    Position position;
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, false))
        position = ADVISOR_HEAL_POSITION;
    else if (PlayerbotAI::IsMainTank(bot))
        position = SANGUINAR_WAITING_POSITION;
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        position = TELONICUS_WAITING_POSITION;
    else if (GetCapernianTank(bot) == bot)
        position = CAPERNIAN_WAITING_POSITION;
    else
        return false;

    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderReequipGearAction::Execute(Event event)
{
    return botAI->DoSpecificAction("equip upgrade", event, true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    bool const isPhase3 = kaelAI->GetPhase() == PHASE_ALL_ADVISORS;
    bool const isActiveCapernianTank = isPhase3 && GetCapernianTank(bot) == bot;

    Unit* target = nullptr;

    // Target priority 1: Thaladred, except Capernian tank during all advisors phase
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!isActiveCapernianTank && thaladred && !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(thaladred))
    {
        target = thaladred;
        if (isPhase3 && MarkTargetWithSkull(bot, thaladred))
            return true;
    }

    // Target priority 2: Capernian for ranged only (excluding debuff hunter)
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!target && capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian) && PlayerbotAI::IsRangedDps(bot) && !IsDebuffHunter(bot))
    {
        target = capernian;
        if (isPhase3 && MarkTargetWithCross(bot, capernian))
            return true;
    }

    // Target priority 3: Sanguinar (debuff hunter and melee move here after Thaladred)
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!target && sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(sanguinar))
    {
        target = sanguinar;
        if (isPhase3 && MarkTargetWithSkull(bot, sanguinar))
            return true;
    }

    // Target priority 4: Telonicus
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!target && telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(telonicus))
    {
        target = telonicus;
        if (isPhase3 && MarkTargetWithSkull(bot, telonicus))
            return true;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs
    if (target != telonicus || telonicus->GetVictim() == bot)
        return false;

    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    float const desiredDist = bot->GetMeleeRange(telonicus);
    float const behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
    float const targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
    float const targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) <= 0.25f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false,
        false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const instanceId = kaelthas->GetMap()->GetInstanceId();

    static constexpr std::array advisorNames = {
        "grand astromancer capernian",
        "master engineer telonicus",
        "lord sanguinar",
    };

    bool advisorAtFullHp = false;
    for (char const* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            advisorAtFullHp = true;
            break;
        }
    }

    if (advisorAtFullHp)
    {
        advisorDpsWaitTimer[instanceId] = -1;
        return false;
    }

    auto it = advisorDpsWaitTimer.find(instanceId);
    if (it != advisorDpsWaitTimer.end() && it->second == -1)
    {
        it->second = std::time(nullptr);
        return true;
    }

    return false;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event /*event*/)
{
    // Priority 0: Everybody other than the main tank needs to stay away from the axe
    // But for assist tanks, move away only after getting aggro on the mace, dagger, or sword
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");

    if (axe)
    {
        bool const hasAggroFromWeapon =
            (mace && mace->GetVictim() == bot) ||
            (dagger && dagger->GetVictim() == bot) ||
            (sword && sword->GetVictim() == bot);

        if (!PlayerbotAI::IsTank(bot) ||
            (PlayerbotAI::IsAssistTank(bot) && hasAggroFromWeapon))
        {
            float const safeDistance = PlayerbotAI::IsTank(bot) ? 17.0f : 13.0f;
            float const currentDistance = bot->GetExactDist2d(axe);
            if (currentDistance < safeDistance)
                return MoveAway(axe, safeDistance - currentDistance);
        }
    }

    bool const isMechanicTracker = IsMechanicTrackerBot(bot, TK_MAP_ID);
    Unit* target = nullptr;

    // Priority 1: Staff of Disintegration
    if (Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration"))
    {
        target = staff;
        if (isMechanicTracker && MarkTargetWithSkull(bot, staff))
            return true;
    }
    // Priority 2: Cosmic Infuser
    else if (mace)
    {
        target = mace;
        if (isMechanicTracker && MarkTargetWithSkull(bot, mace))
            return true;
    }
    // Priority 3: Netherstrand Longbow
    else if (Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow"))
    {
        target = longbow;
        if (isMechanicTracker && MarkTargetWithSkull(bot, longbow))
            return true;
    }
    // Priority 4: Warp Slicer
    else if (sword)
    {
        target = sword;
        if (isMechanicTracker && MarkTargetWithSkull(bot, sword))
            return true;
    }
    // Priority 5: Infinity Blades
    else if (dagger)
    {
        target = dagger;
        if (isMechanicTracker && MarkTargetWithSkull(bot, dagger))
            return true;
    }
    // Priority 6: Devastation - ranged only
    else if (axe && PlayerbotAI::IsRangedDps(bot))
    {
        target = axe;
        if (MarkTargetWithCross(bot, axe))
            return true;
    }
    // Priority 7: Phaseshift Bulwark
    else if (Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark"))
    {
        target = shield;
        if (MarkTargetWithSkull(bot, shield))
            return true;
    }

    if (!target)
        return false;

    if (!PlayerbotAI::IsDps(bot))
        return false;

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event /*event*/)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (!axe)
        return false;

    if (AI_VALUE(Unit*, "current target") != axe)
        return Attack(axe);

    if (MarkTargetWithCross(bot, axe))
        return true;

    if (axe->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 13.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event /*event*/)
{
    static constexpr std::array weapons = {
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
            if (bot->HasItemCount(Id(weapon.itemId), 1, false))
            {
                EquipLegendaryWeapon(Id(weapon.itemId));
                continue;
            }

            return LootWeapon(Id(weapon.npcEntry), Id(weapon.itemId));
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
            return PlayerbotAI::IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at 70
        // So they're better off looting only the dagger to break MCs
        // Plus dual wielding 1H is better dps than Titan Grip at 70 anyway
        case TkNpcs::NPC_DEVASTATION:
            return (bot->getClass() == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) ||
                (bot->getClass() == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
                (PlayerbotAI::IsDps(bot) && bot->getClass() == CLASS_DEATH_KNIGHT);

        case TkNpcs::NPC_INFINITY_BLADES:
            return bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_HUNTER ||
                (bot->getClass() == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT) ||
                (bot->getClass() == CLASS_WARRIOR && tab != WARRIOR_TAB_ARMS);

        // Sub will probably also want to use the Sword, but the spec is currently unimplemented
        case TkNpcs::NPC_WARP_SLICER:
            return (bot->getClass() == CLASS_ROGUE && tab == ROGUE_TAB_COMBAT) ||
                (PlayerbotAI::IsTank(bot) &&
                 (bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_PALADIN));

        case TkNpcs::NPC_STAFF_OF_DISINTEGRATION:
            return (PlayerbotAI::IsRangedDps(bot) && bot->getClass() != CLASS_HUNTER) ||
                (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL);

        case TkNpcs::NPC_PHASESHIFT_BULWARK:
            return PlayerbotAI::IsTank(bot) && bot->getClass() != CLASS_DRUID;

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId)
{
    constexpr float searchRadius = 150.0f;
    Creature* weapon = bot->FindNearestCreature(weaponEntry, searchRadius, false);
    if (!weapon)
        return false;

    LootObject loot(bot, weapon->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    if (bot->GetDistance2d(weapon) > INTERACTION_DISTANCE - 1.0f)
    {
        float const targetDist = INTERACTION_DISTANCE - 2.0f;
        float const angle = weapon->GetAngle(bot);
        float const destX = weapon->GetPositionX() + std::cos(angle) * targetDist;
        float const destY = weapon->GetPositionY() + std::sin(angle) * targetDist;

        return MoveTo(
            TK_MAP_ID, destX, destY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

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

bool KaelthasSunstriderLootLegendaryWeaponsAction::EquipLegendaryWeapon(uint32 itemId)
{
    // Find the legendary item — check equipped slots first (it may have been swapped
    // to a wrong slot by a previous EquipLegendaryWeapon call), then backpack and bags
    Item* legendaryItem = nullptr;
    auto const checkSlot = [&](uint8 bag, uint8 slot)
    {
        Item* item = bot->GetItemByPos(bag, slot);
        if (item && item->GetEntry() == itemId)
        {
            legendaryItem = item;
            return true;
        }
        return false;
    };

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
            break;
    }
    if (!legendaryItem)
    {
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        {
            if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
                break;
        }
    }
    if (!legendaryItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag const* pBag = static_cast<Bag*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag)))
            {
                for (uint32 slot = 0; slot < pBag->GetBagSize(); ++slot)
                {
                    if (checkSlot(bag, slot))
                        break;
                }
            }
            if (legendaryItem)
                break;
        }
    }

    if (!legendaryItem)
        return false;

    ItemTemplate const* proto = legendaryItem->GetTemplate();
    if (!proto)
        return false;

    if (proto->InventoryType == INVTYPE_NON_EQUIP)
        return false;

    // Determine the equip slot for this weapon type
    uint8 dstSlot = EQUIPMENT_SLOT_MAINHAND;

    if (proto->InventoryType == INVTYPE_RANGED)
    {
        dstSlot = EQUIPMENT_SLOT_RANGED;
    }
    else if (proto->InventoryType == INVTYPE_SHIELD ||
        proto->InventoryType == INVTYPE_WEAPONOFFHAND)
    {
        dstSlot = EQUIPMENT_SLOT_OFFHAND;
    }

    // Infinity Blade prefers OH when MH already holds a legendary
    // (combat rogues: Warp Slicer MH, Infinity Blade OH)
    if (dstSlot == EQUIPMENT_SLOT_MAINHAND && itemId == Id(TkItems::ITEM_INFINITY_BLADE) &&
        bot->CanDualWield())
    {
        if (Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            uint32 mhEntry = mhItem->GetEntry();
            if (mhEntry >= ITEM_LEGENDARY_WEAPON_MIN && mhEntry <= ITEM_LEGENDARY_WEAPON_MAX &&
                mhEntry != itemId)
            {
                dstSlot = EQUIPMENT_SLOT_OFFHAND;
            }
        }
    }

    // Check if the legendary is already equipped in the correct slot
    Item* alreadyEquipped = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);
    if (alreadyEquipped && alreadyEquipped->GetEntry() == itemId)
        return false;

    botAI->InterruptSpell();

    // If a 2H in MH is blocking the target OH slot, swap the 2H to inventory first
    bool ohCleared = false;
    if (dstSlot == EQUIPMENT_SLOT_OFFHAND)
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
        {
            uint16 const mhPos = (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_MAINHAND;
            uint16 const srcPos = (legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot();
            bot->SwapItem(mhPos, srcPos);
            ohCleared = true;
            return true;  // legendary is now in MH; next tick will route it to OH if applicable
        }
    }

    // Build src position — legendary may be in inventory or in the wrong equipped slot
    uint16 srcPos = (legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot();
    uint16 const dstPos = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;

    // If target slot is empty, just move the legendary there from wherever it is
    if (!alreadyEquipped)
    {
        bot->SwapItem(srcPos, dstPos);
        return true;
    }

    // Target slot has a different item — swap them
    bool const oldIs2H = alreadyEquipped->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
    bool const newIs2H = proto->InventoryType == INVTYPE_2HWEAPON;

    bot->SwapItem(srcPos, dstPos);

    // If a 2H→1H or 1H→2H swap also affects OH, move the OH item to backpack
    if (((oldIs2H && !newIs2H && proto->InventoryType != INVTYPE_SHIELD) ||
         (!oldIs2H && newIs2H)) && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
    {
        uint16 const ohPos = (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_OFFHAND;
        for (uint8 bpSlot = INVENTORY_SLOT_ITEM_START; bpSlot < INVENTORY_SLOT_ITEM_END; ++bpSlot)
        {
            if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bpSlot))
            {
                bot->SwapItem(ohPos, (INVENTORY_SLOT_BAG_0 << 8) | bpSlot);
                ohCleared = true;
                break;
            }
        }
    }

    // After a 2H→1H swap left OH empty, try to equip the best offhand from inventory.
    // Skip if MH is now a 2H (server rejects OH equip with IsTwoHandUsed).
    if (!ohCleared || bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        return true;

    Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
        return true;

    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    Item* bestOH = nullptr;
    float bestScore = 0.0f;

    auto const scanSlots = [&](uint8 bag, uint8 start, uint8 end)
    {
        for (uint8 slot = start; slot < end; ++slot)
        {
            Item* item = bot->GetItemByPos(bag, slot);
            if (!item || item == legendaryItem)
                continue;

            ItemTemplate const* itemProto = item->GetTemplate();
            if (!itemProto)
                continue;

            uint8 const invType = itemProto->InventoryType;
            if (invType != INVTYPE_WEAPONOFFHAND && invType != INVTYPE_SHIELD &&
                invType != INVTYPE_HOLDABLE && invType != INVTYPE_WEAPON)
            {
                continue;
            }

            if (invType == INVTYPE_WEAPONMAINHAND)
                continue;

            if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
                continue;

            float const score =
                calculator.CalculateItem(itemProto->ItemId, item->GetItemRandomPropertyId());
            if (score > bestScore)
            {
                bestScore = score;
                bestOH = item;
            }
        }
    };

    scanSlots(INVENTORY_SLOT_BAG_0, INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = static_cast<Bag*>(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag)))
            scanSlots(bag, 0, pBag->GetBagSize());
    }

    if (bestOH)
    {
        WorldPacket ohPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        ohPacket << bestOH->GetGUID() << uint8(EQUIPMENT_SLOT_OFFHAND);

        WorldPackets::Item::AutoEquipItemSlot ohNicePacket(std::move(ohPacket));
        ohNicePacket.Read();
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(ohNicePacket);
    }

    return true;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event /*event*/)
{
    return UsePhaseshiftBulwark() || UseStaffOfDisintegration() || UseNetherstrandLongbow();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != Id(TkItems::ITEM_PHASESHIFT_BULWARK))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
        return false;

    if (bot->HasAura(Id(TkSpells::SPELL_ARCANE_BARRIER)) ||
        bot->CanUseItem(offHand) != EQUIP_ERR_OK)
    {
        return false;
    }

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION))
        return false;

    if (bot->HasAura(Id(TkSpells::SPELL_MENTAL_PROTECTION_FIELD)) ||
        bot->CanUseItem(mainHand) != EQUIP_ERR_OK)
    {
        return false;
    }

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != Id(TkItems::ITEM_NETHERSTRAND_LONGBOW))
        return false;

    if (bot->HasItemCount(Id(TkItems::ITEM_NETHER_SPIKES), 1, false) ||
        bot->CanUseItem(ranged) != EQUIP_ERR_OK)
    {
        return false;
    }

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
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (AI_VALUE(Unit*, "current target") != kaelthas)
        return Attack(kaelthas);

    if (kaelthas->GetVictim() != bot || !bot->IsWithinMeleeRange(kaelthas))
        return false;

    Position const position = KAELTHAS_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 4.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = kaelthas->GetPositionX() - botX;
    float const toBossY = kaelthas->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;
    std::vector<Unit*> flameStrikes =
        GetAllHazardTriggers(bot, Id(TkNpcs::NPC_FLAME_STRIKE_TRIGGER), searchRadius);

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

    botAI->InterruptSpell();
    return MoveTo(
        TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(), safestPos.GetPositionZ(),
        false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        return AssistTanksPickUpPhoenixes();
    }

    return NonTanksDestroyEggsAndAvoidPhoenixes();
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::AssistTanksPickUpPhoenixes()
{
    std::vector<Unit*> phoenixes;
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& targetGuid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (target && target->GetEntry() == Id(TkNpcs::NPC_PHOENIX))
            phoenixes.push_back(target);
    }

    if (phoenixes.empty())
        return false;

    std::sort(phoenixes.begin(), phoenixes.end(),
        [](Unit* first, Unit* second) { return first->GetGUID() < second->GetGUID(); });

    Unit* targetPhoenix = nullptr;
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        targetPhoenix = phoenixes[0];
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) && phoenixes.size() >= 2)
        targetPhoenix = phoenixes[1];

    if (!targetPhoenix)
        return false;

    if (AI_VALUE(Unit*, "current target") != targetPhoenix)
        return Attack(targetPhoenix);

    if (targetPhoenix->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 12.0f;
    if (GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return MoveFromGroup(safeDistance);

    return false;
}

bool KaelthasSunstriderHandlePhoenixesAndEggsAction::NonTanksDestroyEggsAndAvoidPhoenixes()
{
    if (Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix"))
    {
        constexpr float safeDistance = 10.0f;
        float const currentDistance = bot->GetDistance2d(phoenix);
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance);
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
    {
        constexpr float searchRadius = 75.0f;
        Unit* phoenixEgg = bot->FindNearestCreature(
            Id(TkNpcs::NPC_PHOENIX_EGG), searchRadius, true);
        if (phoenixEgg && AI_VALUE(Unit*, "current target") != phoenixEgg)
            return Attack(phoenixEgg);
    }
    else if (AI_VALUE(Unit*, "current target") != kaelthas)
    {
        return Attack(kaelthas);
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

        if (member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
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
        return MoveTo(
            TK_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (bot->getClass() == CLASS_ROGUE && AiFactory::GetPlayerSpecTab(bot) != ROGUE_TAB_COMBAT &&
        botAI->CanCastSpell("sinister strike", mcTarget))
    {
        return botAI->CastSpell("sinister strike", mcTarget);
    }
    else
    {
        static std::array const spells = { "hamstring", "wing clip", "shiv", "stormstrike" };
        for (char const* spell : spells)
        {
            if (botAI->CanCastSpell(spell, mcTarget))
                return botAI->CastSpell(spell, mcTarget);
        }
    }

    return false;
}

bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event /*event*/)
{
    // Help bots that get stuck in midair after Gravity Lapse
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE)) &&
        bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY))
    {
        float const x = bot->GetPositionX();
        float const y = bot->GetPositionY();
        float groundZ = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, groundZ);

        bot->GetMotionMaster()->MoveFall();
        bot->SetFallInformation(0, bot->GetPositionZ());
        MovementInfo fallInfo = bot->m_movementInfo;
        fallInfo.pos.Relocate(x, y, groundZ);
        bot->HandleFall(fallInfo);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
        return true;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float minSpreadDistance = 17.0f;
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
        float const distToPlayer = bot->GetExactDist(player);
        if (distToPlayer < closestDist)
        {
            closestDist = distToPlayer;
            closestPlayer = player;
        }
    }

    if (!closestPlayer || closestDist >= minSpreadDistance)
        return false;

    float const angle = bot->GetAngle(closestPlayer) + M_PI;
    float const distance = minSpreadDistance - closestDist;
    float const x = bot->GetPositionX() + std::cos(angle) * distance;
    float const y = bot->GetPositionY() + std::sin(angle) * distance;

    return MoveTo(
        TK_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, true,
        MovementPriority::MOVEMENT_FORCED, true, false);
}
