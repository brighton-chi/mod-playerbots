/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulActions.h"
#include "CreatureAI.h"
#include "GruulHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <limits>
#include <vector>

using namespace GruulHelpers;

// General

bool GruulsLairResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;

    if (!AI_VALUE2(Unit*, "find target", "high king maulgar") &&
        ClearTargetIcon(bot, RtiTargetValue::skullIndex))
    {
        reset = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "gruul the dragonkiller"))
    {
        Action* action = context->GetAction("gruul the dragonkiller spread ranged");
        if (action &&
            static_cast<GruulTheDragonkillerSpreadRangedAction*>(action)->ResetInitialPosition())
        {
            reset = true;
        }
    }

    return reset;
}

// High King Maulgar

bool HighKingMaulgarMeleeTanksPositionBossesAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position const* position = nullptr;
    if (IsMaulgarTank(bot))
    {
        if (Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar"))
        {
            target = maulgar;
            position = &MAULGAR_TANK_POSITION;
        }
    }
    else if (IsOlmTank(bot))
    {
        if (Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner"))
        {
            target = olm;
            position = &OLM_TANK_POSITION;
        }
    }
    else if (IsBlindeyeTank(bot))
    {
        if (Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer"))
        {
            target = blindeye;
            position = &BLINDEYE_TANK_POSITION;
        }
    }

    if (!target || !position)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot)
        return false;

    float const distToPosition = bot->GetExactDist2d(*position);
    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position->GetPositionX() - botX;
    float const toPosY = position->GetPositionY() - botY;

    float const toBossX = target->GetPositionX() - botX;
    float const toBossY = target->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    if (AttackAndCast(krosh))
        return true;

    if (krosh->GetVictim() != bot)
        return false;

    return MoveToDesiredDistance(krosh);
}

bool HighKingMaulgarMageTankAttackKroshAction::AttackAndCast(Unit* krosh)
{
    if (krosh->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh))
    {
        return botAI->CastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh);
    }

    if (AI_VALUE(Unit*, "current target") != krosh)
        return Attack(krosh);

    if (!bot->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell("fire ward", bot))
    {
        return botAI->CastSpell("fire ward", bot);
    }

    return false;
}

bool HighKingMaulgarMageTankAttackKroshAction::MoveToDesiredDistance(Unit* krosh)
{
    Position const& position = KROSH_TANK_POSITION;
    float const distanceKroshToPosition = krosh->GetExactDist2d(position);
    constexpr float minDistance = 17.0f;
    constexpr float maxDistance = 30.0f;

    if (distanceKroshToPosition > minDistance && distanceKroshToPosition < maxDistance &&
        bot->GetExactDist2d(position) > 1.0f)
    {
        return MoveTo(
            GRUUL_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float safeDistance = 15.0f;
    float const currentDistance = bot->GetDistance2d(krosh);

    if (currentDistance >= safeDistance)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveAway(krosh, safeDistance - currentDistance);
}

bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event /*event*/)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return false;

    if (AI_VALUE(Unit*, "current target") != kiggler)
        return Attack(kiggler);

    if (kiggler->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 28.5f;
    float const currentDistance = bot->GetDistance2d(kiggler);

    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(kiggler, safeDistance - currentDistance);
}

bool HighKingMaulgarAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Priority: (1) Blindeye, (2) Olm, (3) Krosh (ranged only), (4) Kiggler, and (5) Maulgar
    Unit* target = nullptr;
    Unit* krosh = nullptr;
    if (Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer"))
    {
        target = blindeye;
    }
    else if (Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner"))
    {
        target = olm;
    }
    else if ((krosh = AI_VALUE2(Unit*, "find target", "krosh firehand")) &&
        PlayerbotAI::IsRanged(bot))
    {
        target = krosh;
    }
    else if (Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed"))
    {
        target = kiggler;
    }
    else if (Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar"))
    {
        target = maulgar;
    }

    if (!target)
        return false;

    if (target == krosh)
    {
        if (MarkTargetWithCross(bot, target))
            return true;
    }
    else if (MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    float const currentDistance = bot->GetDistance2d(maulgar);
    if (currentDistance >= WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveAway(maulgar, WHIRLWIND_SAFE_DISTANCE - currentDistance);
}

bool HighKingMaulgarFleeFromBlastNovaDangerAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetDistance2d(krosh);

    if (currentDistance >= safeDistance)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return FleePosition(krosh->GetPosition(), safeDistance);
}

bool HighKingMaulgarBanishFelStalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> felStalkers;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 50.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(GruulNpcs::NPC_WILD_FEL_STALKER), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            felStalkers.push_back(creature);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == GRUUL_MAP_ID &&
            member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex < 0 || warlockIndex >= felStalkers.size())
        return false;

    Unit* assignedFelStalker = felStalkers[warlockIndex];
    if (!botAI->HasAura("banish", assignedFelStalker) &&
        botAI->CanCastSpell("banish", assignedFelStalker))
    {
        return botAI->CastSpell("banish", assignedFelStalker);
    }

    return false;
}

// Misdirect order: Blindeye, Olm, Kiggler, Krosh
bool HighKingMaulgarMisdirectOgresToTanksAction::Execute(Event /*event*/)
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

        if (hunters.size() >= 4)
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

    Unit* ogreTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "blindeye the seer");
        tankTarget = GetGroupAssistTank(botAI, bot, 1);
    }
    else if (hunterIndex == 1)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "olm the summoner");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupAssistTank(botAI, bot, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetKigglerMoonkinTank(bot))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 3)
    {
        ogreTarget = AI_VALUE2(Unit*, "find target", "krosh firehand");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetKroshMageTank(bot))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!ogreTarget || !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(Id(GruulSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", ogreTarget))
    {
        return botAI->CastSpell("steady shot", ogreTarget);
    }

    return false;
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    if (AI_VALUE(Unit*, "current target") != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() != bot)
        return false;

    Position const& position = GRUUL_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= 3.0f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    float const toBossX = gruul->GetPositionX() - botX;
    float const toBossY = gruul->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Ranged will take initial positions around the middle of the room, 25-40 yards from center
// Ranged should spread out 10 yards from each other
bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> members;
    Player* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        members.push_back(member);

        if (member != bot)
        {
            float distToMember = bot->GetExactDist2d(member);
            if (distToMember < closestDist)
            {
                closestDist = distToMember;
                closestMember = member;
            }
        }
    }

    Position const& position = GRUUL_TANK_POSITION;

    if (_initialPosition.GetPositionX() == 0.0f && _initialPosition.GetPositionY() == 0.0f)
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        constexpr float minRadius = 25.0f;
        constexpr float maxRadius = 40.0f;
        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) /
            static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float targetX = position.GetPositionX() + radius * cos(angle);
        float targetY = position.GetPositionY() + radius * sin(angle);

        _initialPosition = Position(targetX, targetY, position.GetPositionZ());
    }

    if (!_hasReachedInitialPosition)
    {
        float const distToTarget = bot->GetExactDist2d(_initialPosition);
        if (distToTarget > 2.0f)
        {
            // The stored Z is the ring centre's, which says nothing about the ground 25-40 yards
            // out. Seeding from the bot keeps the step inside the height probes MoveTo makes
            constexpr float maxMoveDist = 10.0f;
            float const moveDist = std::min(maxMoveDist, distToTarget);
            float const botX = bot->GetPositionX();
            float const botY = bot->GetPositionY();
            float const moveX =
                botX + ((_initialPosition.GetPositionX() - botX) / distToTarget) * moveDist;
            float const moveY =
                botY + ((_initialPosition.GetPositionY() - botY) / distToTarget) * moveDist;

            return MoveTo(
                GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            _hasReachedInitialPosition = true;
        }
    }

    constexpr float minSpreadDistance = 10.0f;
    if (closestMember && closestDist < minSpreadDistance)
        return FleePosition(closestMember->GetPosition(), minSpreadDistance);

    return false;
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    constexpr uint32 minInterval = 500;
    return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
}
