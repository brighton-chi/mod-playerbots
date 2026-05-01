/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <vector>

#include "RaidSunwellActions.h"
#include "RaidSunwellEredarTwinsEncounter.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace SunwellHelpers;

bool EredarTwinsMeleeJumpDownFromBalconyAction::Execute(Event /*event*/)
{
    const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
    const Position& landingPos = EREDAR_TWINS_P2_MELEE_STACK_POSITION;

    constexpr float arrivalDistance = 2.0f;
    const float distanceToJumpPos =
        bot->GetExactDist2d(jumpPos.GetPositionX(), jumpPos.GetPositionY());

    if (distanceToJumpPos > arrivalDistance)
    {
        return MoveTo(SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
                      jumpPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        return JumpTo(SUNWELL_MAP_ID, landingPos.GetPositionX(),
                      landingPos.GetPositionY(), landingPos.GetPositionZ(),
                      MovementPriority::MOVEMENT_FORCED);
    }
}

bool EredarTwinsMisdirectBossesToTanksAction::Execute(Event /*event*/)
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

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        tankTarget = GetGroupAssistTank(botAI, bot, 0);
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupMainTank(botAI, bot);
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tankTarget = GetGroupAssistTank(botAI, bot, 1);
    }

    if (!tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

bool EredarTwinsMainAndSecondAssistTanksPositionSacrolashAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    MarkTargetWithStar(bot, sacrolash);
    SetRtiTarget(botAI, "star", sacrolash);

    if (bot->GetVictim() != sacrolash)
        return Attack(sacrolash);

    if (sacrolash->GetVictim() == bot && bot->IsWithinMeleeRange(sacrolash))
    {
        const Position& position = SACROLASH_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool EredarTwinsFirstAssistTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    MarkTargetWithCircle(bot, alythess);
    SetRtiTarget(botAI, "circle", alythess);

    if (bot->GetVictim() != alythess)
        return Attack(alythess);

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = alythessTankStep.count(guid) ? alythessTankStep[guid] : 0;
    if (index >= ALYTHESS_TANK_POSITIONS.size())
        index = 0;

    auto const findSafeAlythessTankIndex = [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        const size_t offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITIONS.size(); ++offset)
        {
            const uint8 candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITIONS.size());
            if (IsAlythessTankPositionSafe(bot, ALYTHESS_TANK_POSITIONS[candidateIndex]))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    if (!IsAlythessTankPositionSafe(bot, ALYTHESS_TANK_POSITIONS[index]))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        alythessTankStep[guid] = index;
    }

    const Position& position = ALYTHESS_TANK_POSITIONS[index];

    constexpr float maxDistance = 1.0f;
    const float distToPosition = bot->GetExactDist2d(position);

    if (alythess->GetVictim() == bot)
    {
        if (distToPosition <= maxDistance &&
            ShouldAdvanceAlythessTankPosition(alythess, bot))
        {
            uint8 safeIndex = index;
            if (!findSafeAlythessTankIndex(index, false, safeIndex))
                return false;

            index = safeIndex;
            alythessTankStep[guid] = index;
            const Position& newPosition = ALYTHESS_TANK_POSITIONS[index];
            const float newDistToPosition = bot->GetExactDist2d(newPosition);
            if (newDistToPosition > maxDistance)
            {
                const float dX = newPosition.GetPositionX() - bot->GetPositionX();
                const float dY = newPosition.GetPositionY() - bot->GetPositionY();
                const float moveDist = std::min(5.0f, newDistToPosition);
                const float moveX = bot->GetPositionX() + (dX / newDistToPosition) * moveDist;
                const float moveY = bot->GetPositionY() + (dY / newDistToPosition) * moveDist;

                return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                              true, false);
            }
        }
        else if (distToPosition > maxDistance)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SUNWELL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool EredarTwinsPositionRangedAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash)
    {
        const Position& position = EREDAR_TWINS_P1_RANGED_POSITION;

        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
        {
            return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }
    else if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        const Position& jumpPos = EREDAR_TWINS_P1_RANGED_POSITION;
        const Position& landingPos = EREDAR_TWINS_P2_RANGED_STACK_POSITION;

        constexpr float arrivalDistance = 2.0f;
        const float distanceToJumpPos =
            bot->GetExactDist2d(jumpPos.GetPositionX(), jumpPos.GetPositionY());

        if (distanceToJumpPos > arrivalDistance)
        {
            return MoveTo(SUNWELL_MAP_ID, jumpPos.GetPositionX(), jumpPos.GetPositionY(),
                          jumpPos.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            return JumpTo(SUNWELL_MAP_ID, landingPos.GetPositionX(),
                          landingPos.GetPositionY(), landingPos.GetPositionZ(),
                          MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    const Position& position = botAI->IsRanged(bot) ?
        EREDAR_TWINS_P2_RANGED_STACK_POSITION : EREDAR_TWINS_P2_MELEE_STACK_POSITION;

    const float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 0.5f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (botAI->IsTank(bot))
    {
        Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        if (alythess && bot->GetVictim() != alythess)
            return Attack(alythess);
    }

    return false;
}

bool EredarTwinsRemoveFlameSearAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool EredarTwinsDpsPrioritizeLadySacrolashAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    if (sacrolash)
        SetRtiTarget(botAI, "star", sacrolash);

    if (sacrolash && ShouldHoldSacrolashThreat(botAI, bot, alythess, sacrolash))
    {
        if (bot->GetVictim() == sacrolash || bot->GetTarget() == sacrolash->GetGUID())
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
            return true;
        }

        return false;
    }

    if (sacrolash)
    {
        if (bot->GetTarget() != sacrolash->GetGUID())
            return Attack(sacrolash);

        return false;
    }

    if (alythess)
    {
        SetRtiTarget(botAI, "circle", alythess);

        if (bot->GetTarget() != alythess->GetGUID())
            return Attack(alythess);

        return false;
    }

    return false;
}

bool EredarTwinsConflagratedBotMoveFromGroupAction::Execute(Event /*event*/)
{
    const Position& position = botAI->IsRanged(bot) ?
        EREDAR_TWINS_RANGED_CONFLAG_POSITION : EREDAR_TWINS_MELEE_CONFLAG_POSITION;

    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(SUNWELL_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
