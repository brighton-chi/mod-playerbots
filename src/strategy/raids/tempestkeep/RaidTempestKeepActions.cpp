#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepBossAI.h"
#include "AiFactory.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using namespace TempestKeepHelpers;

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event event)
{
    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");
    if (!centurion)
        return false;

    if (centurion->GetHealth() == centurion->GetMaxHealth() &&
        !centurion->HasAura(SPELL_POLYMORPH_SHEEP) &&
        !centurion->HasAura(SPELL_POLYMORPH_TURTLE) &&
        !centurion->HasAura(SPELL_POLYMORPH_PIG))
    {
        return botAI->CastSpell("polymorph", centurion);
    }
    else if (botAI->CanCastSpell("polymorph", centurion))
        return botAI->CastSpell("polymorph", centurion);

    return false;
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", alar))
        return botAI->CastSpell("steady shot", alar);

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event event)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    MarkTargetWithStar(bot, alar);
    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    bool mtAction = PositionMainTank(botAI->IsMainTank(bot)
        ? bot : nullptr, alar, locationIndex);
    bool atAction = PositionAssistTank(botAI->IsAssistTankOfIndex(bot, 0)
        ? bot : nullptr, alar, locationIndex);

    return mtAction || atAction;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(
    Player* mainTank, Unit* alar, int8 locationIndex)
{
    if (!mainTank)
        return false;

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_3_IDX)
                ? PLATFORM_POSITIONS[0] : PLATFORM_POSITIONS[2];

        if (mainTank->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPESTKEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        if (mainTank->GetVictim() != alar)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, alar))
                    return botAI->CastSpell(spellName, alar);
            }
            return Attack(alar);
        }
    }

    return false;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 locationIndex)
{
    if (!assistTank)
        return false;

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target =
            (locationIndex == PLATFORM_0_IDX || locationIndex == PLATFORM_1_IDX)
                ? PLATFORM_POSITIONS[1] : PLATFORM_POSITIONS[3];

        if (assistTank->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPESTKEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        if (assistTank->GetVictim() != alar)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, alar))
                    return botAI->CastSpell(spellName, alar);
            }
            return Attack(alar);
        }
    }

    return false;
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event event)
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    SetRtiTarget(botAI, "star", alar);

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& target = PLATFORM_POSITIONS[locationIndex];

        if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPESTKEEP_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetVictim() != alar)
            return Attack(alar);
    }

    return false;
}

bool AlarRangedMoveUnderPlatformsAction::Execute(Event event)
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        float currentDistance = bot->GetDistance2d(ember);
        const float safeDistance = 15.0f;

        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, safeDistance - currentDistance);
        }

        if (botAI->IsDps(bot))
        {
            SetRtiTarget(botAI, "square", ember);

            if (bot->GetTarget() != ember->GetGUID())
            {
                bot->SetTarget(ember->GetGUID());
                return Attack(ember);
            }
            else if (!ember)
                SetRtiTarget(botAI, "star", alar);
        }
    }

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position groundPositions[] = {ALAR_GROUND_0, ALAR_GROUND_1, ALAR_GROUND_2, ALAR_GROUND_3};
        const Position& groundTarget = groundPositions[locationIndex];

        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 8.0f)
        {
            return MoveInside(TEMPESTKEEP_MAP_ID, groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                              groundTarget.GetPositionZ(), 8.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarAddTankPickUpEmbersAction::Execute(Event event)
{
    if (!IsAlarAddTank(botAI, bot))
        return false;

    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPESTKEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        MarkTargetWithSquare(bot, ember);

        boss_alar* alarAI = dynamic_cast<boss_alar*>(alar->GetAI());
        if (alarAI && !alarAI->HasPretendedToDie())
            SetRtiTarget(botAI, "square", ember);
        else if (alarAI && alarAI->HasPretendedToDie())
            SetRtiTarget(botAI, "skull", nullptr);

        if (bot->GetVictim() != ember)
            return Attack(ember);

        if (ember->GetVictim() == bot)
        {
            if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
            {
                const Position& groundTarget = GROUND_POSITIONS[locationIndex];
                return MoveNear(TEMPESTKEEP_MAP_ID, groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                                groundTarget.GetPositionZ(), 20.0f, MovementPriority::MOVEMENT_COMBAT);
            }
            else
                return MoveFromGroup(20.0f);
        }
    }
    else if (locationIndex >= PLATFORM_0_IDX && locationIndex <= PLATFORM_3_IDX)
    {
        const Position& groundTarget = GROUND_POSITIONS[locationIndex];
        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 20.0f)
        {
            return MoveInside(TEMPESTKEEP_MAP_ID, groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                              groundTarget.GetPositionZ(), 20.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarJumpFromPlatformAction::Execute(Event event)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPESTKEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }
    else if (botAI->IsMainTank(bot))
    {
        if (bot->GetExactDist2d(ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPESTKEEP_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
                          ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (bot->GetExactDist2d(ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY()) > 5.0f)
        {
            return MoveTo(TEMPESTKEEP_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
                          ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (IsAlarAddTank(botAI, bot))
    {
        if (bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > 20.0f)
        {
            return MoveInside(TEMPESTKEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
                              ALAR_POINT_MIDDLE.GetPositionZ(), 30.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (botAI->IsMelee(bot))
    {
        if (bot->GetExactDist2d(ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY()) > 5.0f)
        {
            return MoveInside(TEMPESTKEEP_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
                              ALAR_ROOM_S_CENTER.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (botAI->IsRanged(bot))
    {
        if (bot->GetExactDist2d(ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY()) > 10.0f)
        {
            return MoveInside(TEMPESTKEEP_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
                              ALAR_POINT_MIDDLE.GetPositionZ(), 10.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        int8 closestPlatform;
        Position ground;
        GetClosestPlatformAndGround(bot->GetPosition(), closestPlatform, ground);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return JumpTo(TEMPESTKEEP_MAP_ID, ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    float currentDistance = bot->GetDistance2d(alar);
    const float safeDistance = 20.0f;
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(alar, safeDistance - currentDistance);
    }

    return false;
}

bool AlarRangedDpsPrioritizeEmbersInPhase2Action::Execute(Event event)
{
    if (!botAI->IsRangedDps(bot))
        return false;

    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        float currentDistance = bot->GetDistance2d(ember);
        const float safeDistance = 15.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, safeDistance - currentDistance);
        }

        SetRtiTarget(botAI, "square", ember);
        if (bot->GetTarget() != ember->GetGUID())
        {
            bot->SetTarget(ember->GetGUID());
            return Attack(ember);
        }
    }
    else if (Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar"))
    {
        SetRtiTarget(botAI, "star", alar);
        if (bot->GetTarget() != alar->GetGUID())
        {
            bot->SetTarget(alar->GetGUID());
            return Attack(alar);
        }
    }

    return false;
}

// Main tank and first assist tank will swap tanking Al'ar when Melt Armor is applied
bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = nullptr;
    Player* assistTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;

            if (botAI->IsMainTank(member))
                mainTank = member;

            if (botAI->IsAssistTankOfIndex(member, 0))
                assistTank = member;
        }
    }

    if (mainTank && assistTank && alar->GetVictim() == mainTank &&
        mainTank->HasAura(SPELL_MELT_ARMOR) && bot == assistTank)
    {
        const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
        for (const char* spellName : taunts)
        {
            if (botAI->CanCastSpell(spellName, alar))
                return botAI->CastSpell(spellName, alar);
        }
    }

    if (mainTank && assistTank && alar->GetVictim() == assistTank &&
        assistTank->HasAura(SPELL_MELT_ARMOR) && bot == mainTank)
    {
        const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
        for (const char* spellName : taunts)
        {
            if (botAI->CanCastSpell(spellName, alar))
                return botAI->CastSpell(spellName, alar);
        }
    }

    boss_alar* alarAI = dynamic_cast<boss_alar*>(alar->GetAI());
    if (bot->GetVictim() != alar && alarAI && !alarAI->IsNoMelee())
    {
        SetRtiTarget(botAI, "star", alar);
        return Attack(alar);
    }

    return false;
}

bool AlarDiveBombSpreadAndStayBackAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 currentIndex = GetAlarCurrentLocationIndex(alar);
    Position dest;
    int8 destIndex = GetAlarDestinationLocationIndex(alar, dest);

    if (currentIndex == POINT_QUILL_OR_DIVE_IDX || destIndex == POINT_QUILL_OR_DIVE_IDX ||
        alar->HasAura(SPELL_DIVE_BOMB_VISUAL))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        GuidVector members = AI_VALUE(GuidVector, "group members");
        Unit* closestMember = nullptr;
        float closestDist = std::numeric_limits<float>::max();

        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || bot->GetGUID() == member)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestMember = unit;
            }
        }

        if (closestMember)
        {
            const uint32 minInterval = 1000;
            return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                                         closestMember->GetPositionZ()), 11.0f, minInterval);
        }
    }
    else
    {
        float currentDistance = bot->GetDistance2d(alar);
        const float safeDistance = 16.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(alar, safeDistance - currentDistance);
        }
    }

    return false;
}

bool AlarReturnToRoomCenterAction::Execute(Event event)
{
    const Position& center = ALAR_ROOM_CENTER;
    if (bot->GetVictim() == nullptr &&
        bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 35.0f)
    {
        return MoveInside(TEMPESTKEEP_MAP_ID, center.GetPositionX(), center.GetPositionY(),
                          center.GetPositionZ(), 30.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

// Void Reaver

// Position in center of room
bool VoidReaverPositionBossAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    if (bot->GetVictim() != voidReaver)
        return Attack(voidReaver);

    if (voidReaver->GetVictim() == bot)
    {
        const Position& tankPosition = VOID_REAVER_TANK_POSITION;

        float dX = tankPosition.GetPositionX() - bot->GetPositionX();
        float dY = tankPosition.GetPositionY() - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.GetPositionX(),
                                                           tankPosition.GetPositionY());

        if (distanceToTankPosition > 2.0f)
        {
            float moveDist = std::min(5.0f, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * moveDist;
            return MoveTo(TEMPESTKEEP_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool VoidReaverSpreadRangedAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    if (voidReaver->GetHealth() == voidReaver->GetMaxHealth())
    {
        initialVoidReaverPositions.clear();
        hasReachedInitialVoidReaverPosition.clear();
    }

    if (initialVoidReaverPositions.empty())
    {
        std::vector<Player*> healers;
        std::vector<Player*> rangedDps;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !botAI->IsRanged(member))
                    continue;
                if (botAI->IsHeal(member))
                    healers.push_back(member);
                else
                    rangedDps.push_back(member);
            }
        }

        const Position& tankPosition = VOID_REAVER_TANK_POSITION;
        const float radius = 30.0f;
        const float offsetArc = 1.0f;
        const uint8 botsPerRing = 8;

        std::vector<Player*> rangedBots = healers;
        rangedBots.insert(rangedBots.end(), rangedDps.begin(), rangedDps.end());

        for (size_t i = 0; i < rangedBots.size(); ++i)
        {
            Player* ranged = rangedBots[i];
            uint8 ringIndex = i / botsPerRing;
            uint8 posInRing = i % botsPerRing;
            float ringRadius = radius + (ringIndex * offsetArc);
            float angle = 2 * M_PI * posInRing / botsPerRing;

            float targetX = tankPosition.GetPositionX() + ringRadius * cos(angle);
            float targetY = tankPosition.GetPositionY() + ringRadius * sin(angle);

            Position pos(targetX, targetY, ranged->GetPositionZ());
            initialVoidReaverPositions[ranged->GetGUID()] = pos;
            hasReachedInitialVoidReaverPosition[ranged->GetGUID()] = false;
        }
    }

    Position targetPosition = initialVoidReaverPositions[bot->GetGUID()];
    float destX = targetPosition.GetPositionX();
    float destY = targetPosition.GetPositionY();

    if (bot->GetExactDist2d(destX, destY) > 1.0f)
    {
        return MoveTo(TEMPESTKEEP_MAP_ID, destX, destY, targetPosition.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

Position VoidReaverSpreadRangedAction::GetRangedBotPosition(const Position& center,
    float radius, uint8 botsPerRing, float offsetArc, uint8 botIndex, float botZ)
{
    float angleOffset = (offsetArc / radius);
    uint8 ringIndex = botIndex / botsPerRing;
    uint8 posInRing = botIndex % botsPerRing;
    float baseAngle = 2 * M_PI * posInRing / botsPerRing;
    float angle = baseAngle + (ringIndex == 1 ? angleOffset : 0);

    if (ringIndex > 1)
    {
        angle = 2 * M_PI * (botIndex % botsPerRing) / botsPerRing;
        ringIndex = 0;
    }

    float targetX = center.GetPositionX() + radius * cos(angle);
    float targetY = center.GetPositionY() + radius * sin(angle);

    return Position(targetX, targetY, botZ);
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    static const std::array<const char*, 6> spells =
    {
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot))
            return botAI->CastSpell(spell, bot);
    }

    return false;
}

// High Astromancer Solarian

bool HighAstromancerSolarianRangedLeaveSpaceForMeleeAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    float currentDistance = bot->GetExactDist2d(astromancer);
    const float minDistance = 20.0f;
    if (currentDistance < minDistance)
        return MoveAway(astromancer, minDistance - currentDistance + 2.0f);

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event event)
{
    const float safeDistance = 15.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveFromGroup(safeDistance + 1.0f);
    }

    return false;
}

// If any agent is present, all bots stack on first alive ranged group member
bool HighAstromancerSolarianStackForAoeAction::Execute(Event event)
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

    if (stackTarget && bot != stackTarget && bot->GetExactDist2d(stackTarget) >= 5.0f)
    {
        return MoveTo(TEMPESTKEEP_MAP_ID, stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                      stackTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event event)
{
    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");
    if (!solariumPriest)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto solariumPriests = GetSolariumPriests();
    auto meleeMembers = GetMeleeBots(group);
    Unit* targetPriest = AssignSolariumPriestsToBots(solariumPriests, meleeMembers);
    if (!targetPriest)
        return false;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();
    if (botIndex < totalMelee / 2)
    {
        MarkTargetWithSquare(bot, targetPriest);
        SetRtiTarget(botAI, "square", targetPriest);
    }
    else
    {
        MarkTargetWithStar(bot, targetPriest);
        SetRtiTarget(botAI, "star", targetPriest);
    }

    if (bot->GetVictim() != targetPriest)
        return Attack(targetPriest);

    return false;
}

std::vector<Unit*> HighAstromancerSolarianTargetSolariumPriestsAction::GetSolariumPriests()
{
    std::vector<Unit*> solariumPriests;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SOLARIUM_PRIEST && unit->IsAlive())
            solariumPriests.push_back(unit);
    }

    return solariumPriests;
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots(Group* group)
{
    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMelee(member) && GET_PLAYERBOT_AI(member))
            meleeMembers.push_back(member);
    }

    std::sort(meleeMembers.begin(), meleeMembers.end(),
              [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    const std::vector<Unit*>& solariumPriests, const std::vector<Player*>& meleeMembers)
{
    if (solariumPriests.size() < 2 || meleeMembers.empty())
        return nullptr;

    std::vector<Unit*> sortedPriests = solariumPriests;
    std::sort(sortedPriests.begin(), sortedPriests.end(),
              [](Unit* left, Unit* right) { return left->GetGUID() < right->GetGUID(); });

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (botIndex < totalMelee / 2)
        return sortedPriests[0];
    else
        return sortedPriests[1];
}

bool HighAstromancerSolarianTankVoidwalkerAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (astromancer->GetVictim() != bot)
    {
        if (botAI->CanCastSpell("taunt", astromancer))
            return botAI->CastSpell("taunt", astromancer);

        if (botAI->CanCastSpell("growl", astromancer))
            return botAI->CastSpell("growl", astromancer);

        if (botAI->CanCastSpell("hand of reckoning", astromancer))
            return botAI->CastSpell("hand of reckoning", astromancer);
    }

    return false;
}

bool HighAstromancerSolarianCastFearWardOnMainTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event event)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    float currentDistance = bot->GetExactDist2d(thaladred);
    const float safeDistance = 25.0f;
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(thaladred, safeDistance - currentDistance + 5.0f);
    }

    return false;
}

bool KaelthasSunstriderMainTankPositionSanguinarAction::Execute(Event event)
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    MarkTargetWithStar(bot, sanguinar);
    SetRtiTarget(botAI, "star", sanguinar);

    if (bot->GetVictim() != sanguinar)
        return Attack(sanguinar);

    if (sanguinar->GetVictim() == bot && bot->IsWithinMeleeRange(sanguinar))
    {
        const Position& position = SANGUINAR_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(TEMPESTKEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event event)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

bool KaelthasSunstriderManageWarlockTankStrategyAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    bool currentlyTank = botAI->HasStrategy("tank", BotState::BOT_STATE_COMBAT);

    // Phase 1: Single advisor phase - switch to tank after Sanguinar is dead
    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        if (!currentlyTank)
        {
            Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
            if (sanguinar && sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        }
        return false;
    }

    // Phase 2: Weapons phase - reset to DPS
    if (kaelAI->GetPhase() == PHASE_WEAPONS)
    {
        if (currentlyTank)
            botAI->ResetStrategies(false);
        return false;
    }

    // Phase 2→3 Transition: Weapons dead, waiting for advisors - switch to tank
    if (kaelAI->GetPhase() == PHASE_TRANSITION)
    {
        if (!currentlyTank)
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        return false;
    }

    // Phase 3: All advisors phase
    if (kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");

        // If Capernian is alive, add tank strategy (applicable only if not all weapons are down)
        if (capernian && !currentlyTank)
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        // If Capernian is dead, reset to DPS
        else if (!capernian && currentlyTank)
            botAI->ResetStrategies(false);
    }

    return false;
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    MarkTargetWithCircle(bot, capernian);
    SetRtiTarget(botAI, "circle", capernian);

    if (bot->GetVictim() != capernian)
        return Attack(capernian);

    if (capernian->GetVictim() == bot && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR)
    {
        float currentDist = bot->GetDistance2d(capernian);
        if (currentDist == 0.0f)
            return false;

        const float minDistance = 31.0f;
        if (currentDist < minDistance)
        {
            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();

            dx /= currentDist;
            dy /= currentDist;
            float targetX = capernian->GetPositionX() + dx * minDistance;
            float targetY = capernian->GetPositionY() + dy * minDistance;

            if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return MoveTo(TEMPESTKEEP_MAP_ID, targetX, targetY, capernian->GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        float orientation = atan2(capernian->GetPositionY() - bot->GetPositionY(),
                                  capernian->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }

    return false;
}

bool KaelthasSunstriderMoveAwayFromCapernianAction::Execute(Event event)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    // Main tank purposely stays in range to bait Conflagration in Phase 1
    if (botAI->IsMainTank(bot))
    {
        const float desiredDist = 15.0f;
        const float tolerance = 2.0f;

        if (fabs(bot->GetExactDist2d(capernian) - desiredDist) > tolerance)
        {
            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();
            float dist = bot->GetExactDist2d(capernian);
            if (dist == 0.0f)
                return false;

            float nx = dx / dist;
            float ny = dy / dist;

            float targetX = capernian->GetPositionX() + nx * desiredDist;
            float targetY = capernian->GetPositionY() + ny * desiredDist;

            return MoveTo(TEMPESTKEEP_MAP_ID, targetX, targetY, capernian->GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    float safeDistance;
    if (botAI->IsMelee(bot) && !botAI->IsMainTank(bot))
        safeDistance = 45.0f;
    else if (botAI->IsRangedDps(bot))
        safeDistance = 25.0f;
    else if (botAI->IsHeal(bot))
        safeDistance = 40.0f;

    float currentDistance = bot->GetExactDist2d(capernian);
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(capernian, safeDistance - currentDistance + 1.0f);
    }

    if (botAI->IsMelee(bot) && !botAI->IsMainTank(bot))
    {
        bot->SetTarget(ObjectGuid::Empty);
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return true;
    }

    return false;
}

bool KaelthasSunstriderFirstAssistTankPositionTelonicusAction::Execute(Event event)
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    MarkTargetWithTriangle(bot, telonicus);
    SetRtiTarget(botAI, "triangle", telonicus);

    if (bot->GetVictim() != telonicus)
        return Attack(telonicus);

    if (telonicus->GetVictim() == bot && bot->IsWithinMeleeRange(telonicus))
    {
        const Position& position = TELONICUS_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(TEMPESTKEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderPositionPhase3TankHealerAction::Execute(Event event)
{
    const Position& position = ADVISOR_HEAL_POSITION;
    float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (dist > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(TEMPESTKEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again)
bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
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
    if (hunterIndex == 0)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(botAI, bot);
    }
    else if (hunterIndex == 1)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        advisorTarget = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tankTarget = GetCapernianTank(botAI, bot);
    }

    if (!advisorTarget || advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        advisorTarget->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        advisorTarget->HasAura(SPELL_PERMANENT_FEIGN_DEATH) ||
        !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", advisorTarget))
        return botAI->CastSpell("steady shot", advisorTarget);

    return false;
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event event)
{
    const char* advisorNames[] =
    {
        "grand astromancer capernian",
        "master engineer telonicus",
        "lord sanguinar"
    };

    for (const char* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            const time_t now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(TEMPESTKEEP_MAP_ID, now);
            return false;
        }
    }

    return false;
}

bool KaelthasSunstriderGroupUpLegendaryWeaponsAction::Execute(Event event)
{
    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    Unit* dagger = AI_VALUE2(Unit*, "find target", "infinity blades");
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    Unit* mace = AI_VALUE2(Unit*, "find target", "cosmic infuser");
    Unit* shield = AI_VALUE2(Unit*, "find target", "phaseshift bulwark");
    Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration");
    Unit* sword = AI_VALUE2(Unit*, "find target", "warp slicer");

    float currentDisperse = AI_VALUE(float, "disperse distance");
    if (currentDisperse > 0.0f)
        RESET_AI_VALUE(float, "disperse distance");

    // Clear targets for assist tanks at start of weapon phase (better to pick up adds?)
    if (botAI->IsAssistTank(bot))
        SetRtiTarget(botAI, "moon", nullptr);

    // Priority 0: Stay away from Devastation
    // Applies to DPS and healers and tanks if they need to pull away the mace, dagger, or sword
    if (axe)
    {
        if (botAI->IsDps(bot) || botAI->IsHeal(bot) ||
            (botAI->IsAssistTank(bot) && (mace && mace->GetVictim() == bot ||
             dagger && dagger->GetVictim() == bot || sword && sword->GetVictim() == bot)))
        {
            const float safeDistance = 10.0f;
            float currentDistance = bot->GetExactDist2d(axe);
            if (currentDistance < safeDistance)
                return MoveAway(axe, safeDistance - currentDistance + 1.0f);
        }
    }

    if (botAI->IsDps(bot))
    {
        // Priority 1: Staff of Disintegration (Skull)
        if (staff)
        {
            MarkTargetWithSkull(bot, staff);
            SetRtiTarget(botAI, "skull", staff);

            if (staff->HasUnitState(UNIT_STATE_CASTING) &&
                staff->FindCurrentSpellBySpellId(SPELL_STAFF_FROSTBOLT))
            {
                if (bot->getClass() == CLASS_MAGE && botAI->CanCastSpell("counterspell", staff))
                    return botAI->CastSpell("counterspell", staff);
                else if (bot->getClass() == CLASS_SHAMAN && botAI->CanCastSpell("wind shear", staff))
                    return botAI->CastSpell("wind shear", staff);
            }

            if (bot->GetVictim() != staff)
                return Attack(staff);

            return false;
        }
        // Priority 2: Cosmic Infuser (Skull)
        if (mace)
        {
            MarkTargetWithSkull(bot, mace);
            SetRtiTarget(botAI, "skull", mace);

            if (bot->GetVictim() != mace)
                return Attack(mace);

            return false;
        }
        // Priority 3: Warp Slicer (Skull)
        if (sword)
        {
            MarkTargetWithSkull(bot, sword);
            SetRtiTarget(botAI, "skull", sword);

            if (bot->GetVictim() != sword)
                return Attack(sword);

            return false;
        }
        // Priority 4: Infinity Blades (Skull)
        if (dagger)
        {
            MarkTargetWithSkull(bot, dagger);
            SetRtiTarget(botAI, "skull", dagger);

            if (bot->GetVictim() != dagger)
                return Attack(dagger);

            return false;
        }
        // Priority 5: Netherstrand Longbow (Cross)
        if (longbow)
        {
            SetRtiTarget(botAI, "cross", longbow);

            if (bot->GetVictim() != longbow)
                return Attack(longbow);

            return false;
        }
        // Priority 6: Devastation - Ranged DPS only (Diamond)
        if (axe && botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "diamond", axe);

            if (bot->GetVictim() != axe)
                return Attack(axe);

            return false;
        }
        // Priority 7: Phaseshift Bulwark (Skull)
        if (shield)
        {
            MarkTargetWithSkull(bot, shield);
            SetRtiTarget(botAI, "skull", shield);

            if (bot->GetVictim() != shield)
                return Attack(shield);
        }
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event event)
{
    Unit* devastation = AI_VALUE2(Unit*, "find target", "devastation");
    if (!devastation)
        return false;

    MarkTargetWithDiamond(bot, devastation);
    SetRtiTarget(botAI, "diamond", devastation);

    if (bot->GetVictim() != devastation)
        return Attack(devastation);

    if (devastation->GetVictim() == bot)
    {
        const float safeDistance = 10.0f;
        Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, safeDistance);
        if (nearestPlayer)
        {
            float currentDistance = bot->GetExactDist2d(nearestPlayer);
            return MoveFromGroup(safeDistance + 1.0f);
        }
    }

    return false;
}

bool KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction::Execute(Event event)
{
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    if (!longbow || !longbow->IsAlive())
        return false;

    MarkTargetWithCross(bot, longbow);
    SetRtiTarget(botAI, "cross", longbow);

    if (bot->GetVictim() != longbow)
        return Attack(longbow);

    if (longbow->GetVictim() == bot)
    {
        const float dangerZone = 15.0f;
        Unit* nearestPlayer = GetNearestNonTankPlayerInRadius(bot, dangerZone);
        if (!nearestPlayer)
            return false;

        float currentDistance = bot->GetExactDist2d(nearestPlayer);
        if (currentDistance < dangerZone)
            return MoveFromGroup(dangerZone);
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event event)
{
    struct WeaponInfo
    {
        uint32 npcEntry;
        uint32 itemId;
        const char* name;
    };

    const WeaponInfo weapons[] =
    {
        { NPC_NETHERSTRAND_LONGBOW, ITEM_NETHERSTRAND_LONGBOW, "netherstrand longbow" },
        { NPC_COSMIC_INFUSER, ITEM_COSMIC_INFUSER, "cosmic infuser" },
        { NPC_DEVASTATION, ITEM_DEVASTATION, "devastation" },
        { NPC_INFINITY_BLADES, ITEM_INFINITY_BLADE, "infinity blade" },
        { NPC_WARP_SLICER, ITEM_WARP_SLICER, "warp slicer" },
        { NPC_STAFF_OF_DISINTEGRATION, ITEM_STAFF_OF_DISINTEGRATION, "staff of disintegration" },
        { NPC_PHASESHIFT_BULWARK, ITEM_PHASESHIFT_BULWARK, "phaseshift bulwark" }
    };

    for (auto const& weapon : weapons)
    {
        if (ShouldBotLootWeapon(weapon.npcEntry))
        {
            if (bot->HasItemCount(weapon.itemId, 1, false))
                continue;

            return LootWeapon(weapon.npcEntry, weapon.itemId, weapon.name);
        }
    }

    return false;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(uint32 weaponEntry)
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    switch (weaponEntry)
    {
        case NPC_NETHERSTRAND_LONGBOW:
            return bot->getClass() == CLASS_HUNTER;

        case NPC_COSMIC_INFUSER:
            return botAI->IsHeal(bot);

        case NPC_DEVASTATION:
            return (bot->getClass() == CLASS_WARRIOR && tab == 0) ||
                   (botAI->IsDps(bot) && (bot->getClass() == CLASS_PALADIN ||
                    bot->getClass() == CLASS_DEATH_KNIGHT));

        case NPC_INFINITY_BLADES:
            return bot->getClass() == CLASS_ROGUE ||
                   bot->getClass() == CLASS_HUNTER ||
                   (bot->getClass() == CLASS_WARRIOR && tab != 0) ||
                   (bot->getClass() == CLASS_SHAMAN && tab == 1);

        case NPC_WARP_SLICER:
            return (bot->getClass() == CLASS_ROGUE && tab != 0) ||
                   (bot->getClass() == CLASS_WARRIOR && tab == 1);

        case NPC_STAFF_OF_DISINTEGRATION:
            return (botAI->IsRangedDps(bot) && bot->getClass() != CLASS_HUNTER) ||
                   (bot->getClass() == CLASS_DRUID && tab == 1);

        case NPC_PHASESHIFT_BULWARK:
            return botAI->IsTank(bot) &&
                   (bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_WARRIOR ||
                    bot->getClass() == CLASS_DEATH_KNIGHT);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId, const char* weaponName)
{
    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig->lootDistance;

    for (auto const& guid : corpses)
    {
        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
            continue;

        WorldObject* object = loot.GetWorldObject(bot);
        if (!object)
            continue;

        Creature* creature = object->ToCreature();
        if (!creature)
            continue;

        if (creature->GetEntry() != weaponEntry || creature->IsAlive())
            continue;

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(object);

        if (dist > maxLootRange)
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);

        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());

        if (!opened)
            return opened;

        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        const uint8 weaponIndex = 0;

        botAI->AddTimedEvent([this, botGuid, corpseGuid, weaponIndex, itemId, weaponName]()
        {
            Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver || !receiver->IsInWorld())
                return;

            if (receiver->HasItemCount(itemId, 1, false))
                return;

            receiver->SetLootGUID(corpseGuid);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << weaponIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        return true;
    }

    return false;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event event)
{
    if (UsePhaseshiftBulwark())
        return true;

    if (UseStaffOfDisintegration())
        return true;

    if (UseNetherstrandLongbow())
        return true;

    return false;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != ITEM_PHASESHIFT_BULWARK)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(SPELL_SHOCK_BARRIER))
        return false;

    if (bot->HasAura(SPELL_ARCANE_BARRIER))
        return false;

    if (bot->CanUseItem(offHand) != EQUIP_ERR_OK)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != ITEM_STAFF_OF_DISINTEGRATION)
        return false;

    if (bot->HasAura(SPELL_MENTAL_PROTECTION_FIELD))
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != ITEM_NETHERSTRAND_LONGBOW)
        return false;

    if (bot->HasItemCount(ITEM_NETHER_SPIKES, 201, false))
        return false;

    return UseEquippedItemWithPacket(ranged);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseEquippedItemWithPacket(Item* item)
{
    if (!item)
        return false;

    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(true))
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

bool KaelthasSunstriderReequipGearAction::Execute(Event event)
{
    return botAI->DoSpecificAction("equip upgrades", Event(), true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event event)
{
    // Enable disperse at the start of phases 1 and 3 for ranged, except Capernian tank
    // The purpose is so they don't all get melted by Conflagration
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (botAI->IsRanged(bot) && bot != capernianTank)
    {
        const float desiredDisperse = 7.0f;
        float currentDisperse = AI_VALUE(float, "disperse distance");
        if (currentDisperse < 0.0f || currentDisperse != desiredDisperse)
            SET_AI_VALUE(float, "disperse distance", desiredDisperse);
    }

    if (botAI->IsHeal(bot))
        return false;

    // Target priority 1: Thaladred for all dps except Capernian tank
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    bool isCapernianTank = (capernianTank && bot == capernianTank);

    if (thaladred && thaladred->IsAlive() && !isCapernianTank &&
        !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        MarkTargetWithSquare(bot, thaladred);
        SetRtiTarget(botAI, "square", thaladred);

        if (bot->GetVictim() != thaladred)
            return Attack(thaladred);

        return false;
    }

    // Target priority 2: Capernian for ranged
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (capernian && capernian->IsAlive() && botAI->IsRangedDps(bot) &&
        !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "circle", capernian);

        if (bot->GetVictim() != capernian)
            return Attack(capernian);

        return false;
    }

    // Target priority 3: Sanguinar for all dps
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (sanguinar && sanguinar->IsAlive() &&
        !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "star", sanguinar);

        if (bot->GetVictim() != sanguinar)
            return Attack(sanguinar);

        return false;
    }

    // Target priority 4: Telonicus for all dps
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (telonicus && telonicus->IsAlive() &&
        !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "triangle", telonicus);

        if (bot->GetVictim() != telonicus)
            return Attack(telonicus);

        // Melee DPS positioning: stay at max-ish melee range behind Telonicus (god damn bombs)
        if (botAI->IsMelee(bot) && botAI->IsDps(bot) && telonicus->GetVictim() != bot)
        {
            float maxMeleeRange = bot->GetMeleeRange(telonicus);

            const float meleeRangeBuffer = 0.5f;
            const float tolerance = 0.75f;

            float desiredDist = std::max(2.0f, maxMeleeRange - meleeRangeBuffer);
            float currentDist = bot->GetExactDist2d(telonicus);

            if (fabs(currentDist - desiredDist) > tolerance)
            {
                float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
                float targetX = telonicus->GetPositionX() + desiredDist * cos(behindAngle);
                float targetY = telonicus->GetPositionY() + desiredDist * sin(behindAngle);

                if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
                {
                    return MoveTo(TEMPESTKEEP_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false,
                                  false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
                }
            }
        }
    }

    return false;
}

bool KaelthasSunstriderMainTankPositionBossAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    MarkTargetWithStar(bot, kaelthas);
    SetRtiTarget(botAI, "star", kaelthas);

    if (bot->GetVictim() != kaelthas)
        return Attack(kaelthas);

    if (kaelthas->GetVictim() == bot && bot->IsWithinMeleeRange(kaelthas))
    {
        const Position& position = KAELTHAS_TANK_POSITION;
        float dist = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        if (dist > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(TEMPESTKEEP_MAP_ID, moveX, moveY, position.GetPositionZ(), false,
                          false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event event)
{
    // Disable disperse in Phase 4--could insert this somewhere else instead
    float currentDisperse = AI_VALUE(float, "disperse distance");
    if (currentDisperse > 0.0f)
        RESET_AI_VALUE(float, "disperse distance");

    std::vector<Unit*> flameStrikes = GetAllFlameStrikeTriggers(botAI, bot);
    if (flameStrikes.empty())
        return false;

    const float hazardRadius = 12.0f;
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

    Position safestPos = FindSafestNearbyPosition(flameStrikes, hazardRadius);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(TEMPESTKEEP_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position KaelthasSunstriderAvoidFlameStrikeAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& flameStrikes, float hazardRadius)
{
    const float searchStep = M_PI / 8.0f;
    const float minDistance = 2.0f;
    const float maxDistance = 30.0f;
    const float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = 1000.0f;
    bool foundSafe = false;

    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * cos(angle);
            float y = bot->GetPositionY() + distance * sin(angle);

            Position testPos(x, y, bot->GetPositionZ());

            bool isSafe = true;
            for (Unit* flameStrike : flameStrikes)
            {
                if (flameStrike->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }
            if (!isSafe)
                continue;

            bool pathSafe = IsPathSafeFromFlameStrikes(bot->GetPosition(), testPos, flameStrikes, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::IsPathSafeFromFlameStrikes(
    const Position& start, const Position& end, const std::vector<Unit*>& flameStrikes, float hazardRadius)
{
    const int numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (int i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* flameStrike : flameStrikes)
        {
            float distToFlameStrike = flameStrike->GetExactDist2d(checkX, checkY);
            if (distToFlameStrike < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> KaelthasSunstriderAvoidFlameStrikeAction::GetAllFlameStrikeTriggers(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> flameStrikeTriggers;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 45.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_FLAME_STRIKE_TRIGGER && bot->GetExactDist2d(unit) < maxSearchRadius)
            flameStrikeTriggers.push_back(unit);
    }

    return flameStrikeTriggers;
}

bool KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction::Execute(Event event)
{
    if (botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1))
    {
        std::vector<Unit*> phoenixes;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_PHOENIX && unit->IsAlive())
                phoenixes.push_back(unit);
        }

        if (phoenixes.empty())
            return false;

        std::sort(phoenixes.begin(), phoenixes.end(),
                  [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

        Unit* targetPhoenix = nullptr;
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            // Assist Tank 0: Take first phoenix (Square)
            targetPhoenix = phoenixes[0];
            MarkTargetWithSquare(bot, targetPhoenix);
            SetRtiTarget(botAI, "square", targetPhoenix);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1) && phoenixes.size() >= 2)
        {
            // Assist Tank 1: Take second phoenix (Circle)
            targetPhoenix = phoenixes[1];
            MarkTargetWithCircle(bot, targetPhoenix);
            SetRtiTarget(botAI, "circle", targetPhoenix);
        }

        if (!targetPhoenix)
            return false;

        if (bot->GetVictim() != targetPhoenix)
            return Attack(targetPhoenix);

        if (targetPhoenix->GetVictim() == bot)
        {
            const float safeDistance = 10.0f;

            Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
            if (Group* group = bot->GetGroup())
            {
                if (nearestPlayer && group)
                {
                    PlayerbotAI* nearestAI = GET_PLAYERBOT_AI(nearestPlayer->ToPlayer());
                    if (nearestAI && (nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 0) ||
                                       nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 1)))
                    {
                        nearestPlayer = nullptr;
                    }
                }
            }

            if (nearestPlayer)
            {
                float closestDist = bot->GetExactDist2d(nearestPlayer);
                if (closestDist < safeDistance)
                    return MoveFromGroup(safeDistance + 2.0f);
            }
        }

        return false;
    }

    if (botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix");
        if (!phoenix)
            return false;

        float currentDistance = bot->GetExactDist2d(phoenix);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
            return MoveAway(phoenix, safeDistance - currentDistance + 2.0f);
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (botAI->IsRangedDps(bot) ||
        (botAI->IsMelee(bot) && botAI->IsDps(bot) && kaelthas &&
         kaelthas->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && kaelthas->GetHealthPct() <= 50.0f))
    {
        Unit* phoenixEgg = AI_VALUE2(Unit*, "find target", "phoenix egg");
        if (!phoenixEgg)
            return false;

        MarkTargetWithDiamond(bot, phoenixEgg);
        SetRtiTarget(botAI, "diamond", phoenixEgg);

        if (bot->GetTarget() != phoenixEgg->GetGUID())
        {
            bot->SetTarget(phoenixEgg->GetGUID());
            return Attack (phoenixEgg);
        }
    }

    return false;
}

bool KaelthasSunstriderBreakMindControlWithInfinityBladeAction::Execute(Event event)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
            {
                float dist = bot->GetExactDist2d(member);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    mcTarget = member;
                }
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(TEMPESTKEEP_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(),
                      mcTarget->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    static const std::array<const char*, 4> spells =
    {
        "hamstring",
        "wing clip",
        "shiv",
        "stormstrike"
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, mcTarget))
            return botAI->CastSpell(spell, mcTarget);
    }

    return false;
}

// Focus on Kael'thas when Shock Barrier is up
// Cast interrupt as soon as Shock Barrier is broken
bool KaelthasSunstriderBreakThroughShockBarrierAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!kaelthas->HasAura(SPELL_SHOCK_BARRIER))
    {
        static const std::array<const char*, 8> spells =
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

        for (const char* spell : spells)
        {
            if (botAI->CanCastSpell(spell, kaelthas))
                return botAI->CastSpell(spell, kaelthas);
        }
    }
    else if (bot->GetTarget() != kaelthas->GetGUID())
    {
        SetRtiTarget(botAI, "star", kaelthas);
        bot->SetTarget(kaelthas->GetGUID());
        return Attack(kaelthas);
    }

    return false;
}

bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const float minSpreadDistance = 12.0f;

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
        float dist = bot->GetExactDist(player);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestPlayer = player;
        }
    }

    if (closestPlayer && closestDist < minSpreadDistance)
    {
        float angle = bot->GetAngle(closestPlayer) + M_PI;
        float distance = minSpreadDistance - closestDist + 2.0f;

        float x = bot->GetPositionX() + cos(angle) * distance;
        float y = bot->GetPositionY() + sin(angle) * distance;

        return MoveTo(TEMPESTKEEP_MAP_ID, x, y, bot->GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
