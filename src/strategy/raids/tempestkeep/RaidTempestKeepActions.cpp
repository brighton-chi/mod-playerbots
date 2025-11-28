#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "AiFactory.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using namespace TempestKeepHelpers;
using namespace TempestKeepPositions;

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
        return botAI->CastSpell("polymorph", centurion);
    else if (botAI->CanCastSpell("polymorph", centurion))
        return botAI->CastSpell("polymorph", centurion);

    return false;
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    Group* group = bot->GetGroup();
    if (!alar || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
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
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();
    std::vector<Position> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };

    UpdateAlarLastPlatform(alar, mapId, platforms);
    int8 alarPlatform = lastAlarPlatform[mapId];

    SetRtiTarget(botAI, "star", alar);
    bool mtAction = PositionMainTank(botAI->IsMainTank(bot) ? bot : nullptr, alar, alarPlatform, platforms);
    bool atAction = PositionAssistTank(botAI->IsAssistTankOfIndex(bot, 0) ?
                    bot : nullptr, alar, alarPlatform, platforms);

    return mtAction || atAction;
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar,
    int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!mainTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SW ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && mainTank->GetPositionZ() < -2.0f)
    {
        if (mainTank->GetExactDist2d(AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY(),
                          AlarSWRampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true;
    }

    Position mtTarget;

    // Determine target based on Al'ar's platform
    if (alarPlatform == 0 || alarPlatform == 3)
    {
        mtTarget = platforms[0]; // Move to platform 1
    }
    else if (alarPlatform == 1 || alarPlatform == 2)
    {
        mtTarget = platforms[1]; // Move to platform 2
    }
    else
        return false;

    if (mainTank->GetExactDist2d(mtTarget.GetPositionX(), mtTarget.GetPositionY()) < 2.0f)
    {
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
        return false;
    }

    return MoveTo(bot->GetMapId(), mtTarget.GetPositionX(), mtTarget.GetPositionY(),
                  mtTarget.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar,
    int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!assistTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SE ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && assistTank->GetPositionZ() < -2.0f)
    {
        if (assistTank->GetExactDist2d(AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY(),
                          AlarSERampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true;
    }

    Position atTarget;

    // Determine target based on Al'ar's platform
    if (alarPlatform == 0 || alarPlatform == 3)
    {
        atTarget = platforms[3]; // Move to platform 4
    }
    else if (alarPlatform == 1 || alarPlatform == 2)
    {
        atTarget = platforms[2]; // Move to platform 3
    }
    else
        return false;

    if (assistTank->GetExactDist2d(atTarget.GetPositionX(), atTarget.GetPositionY()) < 2.0f)
    {
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
        return false;
    }

    return MoveTo(bot->GetMapId(), atTarget.GetPositionX(), atTarget.GetPositionY(),
                  atTarget.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AlarMeleeDpsPrioritizeBossAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    if (isPhase2[mapId])
    {
        Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
        if (ember)
        {
            float currentDistance = bot->GetExactDist2d(ember);
            const float safeDistance = 18.0f;
            if (ember->GetHealthPct() < 50.0f &&
                currentDistance < safeDistance)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return MoveAway(ember, safeDistance - currentDistance + 2.0f);
            }
        }
        else if (alar && !ember)
        {
            MarkTargetWithStar(bot, alar);
            SetRtiTarget(botAI, "star", alar);

            if (bot->GetVictim() != alar)
                return Attack(alar);
        }
    }

    if (!isPhase2[mapId])
    {
        // If Al'ar is flying (Flame Quills), move to S of room and hold position
        if (alar->GetPositionZ() >= 42.0f && bot->GetPositionZ() < -2.0f)
        {
            if (bot->GetExactDist2d(AlarRoomSouthCenter.GetPositionX(), AlarRoomSouthCenter.GetPositionY()) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), AlarRoomSouthCenter.GetPositionX(), AlarRoomSouthCenter.GetPositionY(),
                            AlarRoomSouthCenter.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            return true;
        }

        int8 alarPlatform = lastAlarPlatform[mapId];
        std::vector<Position> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };
        const Position& platformTarget = platforms[alarPlatform];

        std::vector<Position> waypoints;
        if (alarPlatform == 1)
            waypoints = { AlarPlatform1To2MidpointA, AlarPlatform1To2MidpointB };
        else if (alarPlatform == 2)
            waypoints = { AlarPlatform2To3MidpointA, AlarPlatform2To3MidpointB };

        if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) < 2.0f)
            meleeDpsWaypointVisited[bot->GetGUID()].clear();

        if (!waypoints.empty())
        {
            if (meleeDpsWaypointVisited[bot->GetGUID()].empty() &&
                bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
            {
                meleeDpsWaypointVisited[bot->GetGUID()] = std::vector<bool>(waypoints.size(), false);
            }
            for (size_t i = 0; i < waypoints.size(); ++i)
            {
                if (!meleeDpsWaypointVisited[bot->GetGUID()][i])
                {
                    const Position& wp = waypoints[i];
                    if (bot->GetExactDist2d(wp.GetPositionX(), wp.GetPositionY()) >= 2.0f)
                    {
                        return MoveTo(bot->GetMapId(), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ(), false, false, false, true,
                                    MovementPriority::MOVEMENT_COMBAT, true, false);
                    }
                    else
                        meleeDpsWaypointVisited[bot->GetGUID()][i] = true;
                    break;
                }
            }
            if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        else
        {
            if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        if (bot->GetVictim() != alar)
            return Attack(alar);
    }

    return false;
}

bool AlarRangedDpsPrioritizeAddsAction::Execute(Event event)
{
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        float currentDistance = bot->GetExactDist2d(ember);
        const float safeDistance = 18.0f;
        if (ember->GetHealthPct() < 50.0f &&
            currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, safeDistance - currentDistance + 2.0f);
        }

        SetRtiTarget(botAI, "square", ember);

        if (bot->GetTarget() != ember->GetGUID())
        {
            bot->SetSelection(ember->GetGUID());
            return Attack(ember);
        }
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (alar && !ember)
    {
        MarkTargetWithStar(bot, alar);
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetTarget() != alar->GetGUID())
        {
            bot->SetSelection(alar->GetGUID());
            return Attack(alar);
        }
    }

    const uint32 mapId = alar->GetMapId();
    if (!isPhase2[mapId])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];
        std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Position& groundTarget = groundPositions[alarPlatform];

        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 15.0f)
        {
            return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                            groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarPositionHealerAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        float currentDistance = bot->GetExactDist2d(ember);
        const float safeDistance = 18.0f;

        if (ember->GetHealthPct() < 50.0f &&
            currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, safeDistance - currentDistance + 2.0f);
        }
    }

    const uint32 mapId = alar->GetMapId();
    if (!isPhase2[alar->GetMapId()])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];
        std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Position& groundTarget = groundPositions[alarPlatform];

        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 15.0f)
        {
            return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                            groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarAddTankPickUpEmbersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (!ember)
        return false;

    if (!isPhase2[mapId])
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetVictim() == ember)
            return Attack(ember);

        const Position& center = AlarRangedCenter;
        if (ember->GetVictim() == bot && bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 5.0f)
        {
            float dX = center.GetPositionX() - bot->GetPositionX();
            float dY = center.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, center.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            int8 alarPlatform = lastAlarPlatform[mapId];
            std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
            const Position& groundTarget = groundPositions[alarPlatform];

            if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 30.0f)
            {
                return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(),
                                groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    else if (isPhase2[mapId])
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetVictim() == ember)
            return Attack(ember);

        const float safeDistance = 16.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (nearestPlayer)
            return MoveFromGroup(safeDistance + 2.0f);
    }

    return false;
}

bool AlarJumpFromPlatformAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (bot->GetPositionZ() >= 17.0f)
    {
        std::vector<std::pair<Position, Position>> platformGroundPairs =
        {
            {AlarPlatform1, AlarGround1},
            {AlarPlatform2, AlarGround2},
            {AlarPlatform3, AlarGround3},
            {AlarPlatform4, AlarGround4}
        };

        float minDist = std::numeric_limits<float>::max();
        size_t nearestIndex = 0;
        for (size_t i = 0; i < platformGroundPairs.size(); ++i)
        {
            float dist = bot->GetExactDist2d(platformGroundPairs[i].first.GetPositionX(),
                                             platformGroundPairs[i].first.GetPositionY());
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }
        const Position& ground = platformGroundPairs[nearestIndex].second;
        return JumpTo(bot->GetMapId(), ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
    if (bot->GetPositionZ() >= 17.0f)
    {
        std::vector<std::pair<Position, Position>> platformGroundPairs =
        {
            {AlarPlatform1, AlarGround1},
            {AlarPlatform2, AlarGround2},
            {AlarPlatform3, AlarGround3},
            {AlarPlatform4, AlarGround4}
        };

        float minDist = std::numeric_limits<float>::max();
        size_t nearestIndex = 0;
        for (size_t i = 0; i < platformGroundPairs.size(); ++i)
        {
            float dist = bot->GetExactDist2d(platformGroundPairs[i].first.GetPositionX(),
                                             platformGroundPairs[i].first.GetPositionY());
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }
        const Position& ground = platformGroundPairs[nearestIndex].second;
        return JumpTo(bot->GetMapId(), ground.GetPositionX(), ground.GetPositionY(),
                      ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    float currentDistance = bot->GetExactDist2d(alar);
    const float safeDistance = 20.0f;
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(alar, safeDistance - currentDistance + 5.0f);
    }

    return false;
}

bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    Group* group = bot->GetGroup();
    if (!alar || !group)
        return false;

    Player* mainTank = nullptr;
    Player* assistTank = nullptr;
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

    // If main tank is tanking Al'ar and has Melt Armor, assist tank should taunt
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

    // If assist tank is tanking Al'ar and has Melt Armor, main tank should taunt
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

    if (bot->GetVictim() != alar)
    {
        SetRtiTarget(botAI, "star", alar);
        return Attack(alar);
    }

    return false;
}

bool AlarReturnToRoomCenterAction::Execute(Event event)
{
    const Position& center = AlarRoomCenter;
    if (bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 35.0f)
    {
        return MoveInside(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(),
                          center.GetPositionZ(), 25.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AlarDiveBombSpreadAction::Execute(Event event)
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
        const uint32 minInterval = 200;
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                                     closestMember->GetPositionZ()), 11.0f, minInterval);
    }

    return false;
}

bool AlarManageTimersAndTrackersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    if (IsAlarMapIDTimerManager(botAI, bot) && alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= 17.0f)
    {
        lastRebirthState[mapId] = false;
        lastAlarPlatform[mapId] = -1;
        isPhase2[mapId] = false;
    }

    if ((alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= 17.0f) ||
        alar->GetPositionZ() >= 22.0f)
    {
        if (botAI->IsMainTank(bot))
        {
            mtBalconyMidpointVisited[bot->GetGUID()].clear();
            mainTankAtPlatform2[bot->GetGUID()] = false;
        }
        else if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            atBalconyMidpointVisited[bot->GetGUID()].clear();
            assistTankAtPlatform3[bot->GetGUID()] = false;
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            meleeDpsWaypointVisited[bot->GetGUID()].clear();
    }

    // Manual override: if Flame Quills is active, set lastAlarPlatform to platform 4 (index 3)
    if (alar->GetPositionZ() >= 22.0f && alar->GetHealthPct() < 95.0f)
        lastAlarPlatform[mapId] = 3;

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
                         alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_PHASE2);
    bool lastRebirth = lastRebirthState[mapId];

    // Detect transition: finished casting Rebirth (phase 2 begins)
    if (lastRebirth && !rebirthActive)
        isPhase2[mapId] = true;

    lastRebirthState[mapId] = rebirthActive;

    return false;
}

// Void Reaver

// Position in center of room
bool VoidReaverPositionBossAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    if (bot->GetVictim() != voidReaver)
        return Attack(voidReaver);

    if (voidReaver->GetVictim() == bot)
    {
        const Position& tankPosition = VoidReaverTankPosition;

        float dX = tankPosition.GetPositionX() - bot->GetPositionX();
        float dY = tankPosition.GetPositionY() - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.GetPositionX(),
                                                           tankPosition.GetPositionY());

        if (distanceToTankPosition > 2.0f)
        {
            float moveDist = std::min(4.5f, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * moveDist;
            const float moveZ = tankPosition.GetPositionZ();
            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool VoidReaverSpreadRangedAction::Execute(Event event)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    Group* group = bot->GetGroup();
    if (!voidReaver || !group)
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

        const Position& tankPosition = VoidReaverTankPosition;
        const float radius = 30.0f;
        const float offsetArc = 1.0f;
        const uint8 botsPerRing = 8;

        // Assign all ranged (healers first, then dps) to rings
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
    float destZ = targetPosition.GetPositionZ();

    if (!bot->IsWithinDist2d(destX, destY, 1.0f))
    {
        return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
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

    // If overflow, wrap around
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

bool HighAstromancerSolarianStackBotsAction::Execute(Event event)
{
    Unit* solariumAgent = AI_VALUE2(Unit*, "find target", "solarium agent");
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Phase 2: If any agent is present, all bots stack on first alive group member
    if (solariumAgent)
    {
        Player* stackTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive())
            {
                stackTarget = member;
                break;
            }
        }

        if (stackTarget && bot != stackTarget && bot->GetExactDist2d(stackTarget) >= 5.0f)
        {
            return MoveTo(bot->GetMapId(), stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                          stackTarget->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        return false;
    }

    // Phase 1: All ranged stack 25 yards from boss
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (astromancer && botAI->IsRanged(bot))
    {
        const float stackDistance = 25.0f;
        const float stackAngle = 5.0f * M_PI / 4.0f; // Southwest direction
        float stackX = astromancer->GetPositionX() + stackDistance * cos(stackAngle);
        float stackY = astromancer->GetPositionY() + stackDistance * sin(stackAngle);
        float stackZ = astromancer->GetPositionZ();

        if (bot->GetExactDist2d(stackX, stackY) >= 3.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), stackX, stackY, stackZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event event)
{
    const float safeDistance = 15.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (nearestPlayer)
        return MoveFromGroup(safeDistance + 1.0f);

    return false;
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event event)
{
    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");
    Group* group = bot->GetGroup();
    if (!solariumPriest || !group)
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

    if (!bot->IsWithinMeleeRange(targetPriest))
        return MoveTo(bot->GetMapId(), targetPriest->GetPositionX(), targetPriest->GetPositionY(),
                      targetPriest->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);

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
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToBots(
    const std::vector<Unit*>& solariumPriests, const std::vector<Player*>& meleeMembers)
{
    if (solariumPriests.size() < 2 || meleeMembers.empty())
        return nullptr;

    std::vector<Unit*> sortedPriests = solariumPriests;
    std::sort(sortedPriests.begin(), sortedPriests.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

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

    if (sanguinar->GetVictim() == bot)
    {
        const Position& position = SanguinarTankPosition;
        if (!bot->IsWithinMeleeRange(sanguinar))
        {
            return MoveTo(sanguinar->GetMapId(), sanguinar->GetPositionX(),
                          sanguinar->GetPositionY(), sanguinar->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderCastFearWardOnSanguinarTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
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

    bool currentlyTank = botAI->HasStrategy("tank", BotState::BOT_STATE_COMBAT);

    // Phase 1: Single advisor phase - switch to tank after Sanguinar is dead
    if (IsKaelthasInPhase1(botAI))
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
    if (IsKaelthasInPhase2(botAI))
    {
        if (currentlyTank)
            botAI->ResetStrategies(false);
        return false;
    }

    // Phase 2→3 Transition: Weapons dead, waiting for advisors - switch to tank
    if (IsKaelthasInPhase2To3Transition(botAI))
    {
        if (!currentlyTank)
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        return false;
    }

    // Phase 3: All advisors phase
    if (IsKaelthasInPhase3(botAI))
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");

        // If Capernian is alive, add tank strategy (failsafe for if not all weapons are down)
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
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    MarkTargetWithCircle(bot, capernian);
    SetRtiTarget(botAI, "circle", capernian);

    if (!botAI->HasStrategy("curse of doom", BOT_STATE_COMBAT))
        botAI->ChangeStrategy("+curse of doom", BOT_STATE_COMBAT);

    if (bot->GetTarget() != capernian->GetGUID())
    {
        if (botAI->CanCastSpell("curse of doom", capernian))
            return botAI->CastSpell("curse of doom", capernian);

        bot->SetTarget(capernian->GetGUID());
        return Attack(capernian);
    }

    if (capernian->GetVictim() == bot && IsKaelthasInPhase1(botAI))
    {
        const float minDistance = 31.0f;
        const float maxDistance = 34.0f;

        float currentDist = bot->GetExactDist2d(capernian);
        if (currentDist < minDistance || currentDist > maxDistance)
        {
            float desiredDist = (currentDist < minDistance) ? minDistance : maxDistance;

            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();

            if (currentDist == 0.0f)
                return false;

            dx /= currentDist;
            dy /= currentDist;
            float targetX = capernian->GetPositionX() + dx * desiredDist;
            float targetY = capernian->GetPositionY() + dy * desiredDist;

            if (!bot->IsWithinDist2d(targetX, targetY, 1.0f))
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return MoveTo(bot->GetMapId(), targetX, targetY, capernian->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
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

    // Tanks purposely stay in range to bait Conflagration in Phase 1
    if (botAI->IsTank(bot) && IsKaelthasInPhase1(botAI))
    {
        const float desiredDist = 15.0f;
        const float tolerance = 2.0f;

        if (fabs(bot->GetExactDist2d(capernian) - desiredDist) > tolerance)
        {
            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist == 0.0f)
                return false;

            float nx = dx / dist;
            float ny = dy / dist;

            float targetX = capernian->GetPositionX() + nx * desiredDist;
            float targetY = capernian->GetPositionY() + ny * desiredDist;

            return MoveTo(capernian->GetMapId(), targetX, targetY, capernian->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    // Determine safe distance based on role
    float safeDistance;
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (botAI->IsMelee(bot) && botAI->IsDps(bot))
        safeDistance = 45.0f;
    else if (botAI->IsRangedDps(bot))
        safeDistance = 30.5f;
    else if (botAI->IsHeal(bot))
        safeDistance = 40.0f;

    float currentDistance = bot->GetExactDist2d(capernian);
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(capernian, safeDistance - currentDistance + 1.0f);
    }

    // In Phase 1, melee DPS should stay awand and do nothing while Capernian is active
    if (botAI->IsMelee(bot) && botAI->IsDps(bot) && IsKaelthasInPhase1(botAI))
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

    if (telonicus->GetVictim() == bot)
    {
        const Position& position = TelonicusTankPosition;
        if (!bot->IsWithinMeleeRange(telonicus))
        {
            return MoveTo(telonicus->GetMapId(), telonicus->GetPositionX(),
                          telonicus->GetPositionY(), telonicus->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(5.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
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

    if (!advisorTarget || advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || advisorTarget->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        advisorTarget->HasAura(SPELL_PERMANENT_FEIGN_DEATH) || !tankTarget || !tankTarget->IsAlive())
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

        if (advisor->GetHealth() == advisor->GetMaxHealth() && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            const time_t now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(advisor->GetMapId(), now);
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

    // Disable disperse during the weapons phase
    float currentDisperse = AI_VALUE(float, "disperse distance");
    if (currentDisperse > 0.0f)
        RESET_AI_VALUE(float, "disperse distance");

    // Clear targets for assist tanks at start of weapon phase
    if (botAI->IsAssistTank(bot))
        SetRtiTarget(botAI, "moon", nullptr);

    if (botAI->IsDps(bot))
    {
        // Priority 0: Stay away from Devastation
        // Applies to DPS at all times and tanks if they need to pull away the mace, dagger, or sword
        if (axe)
        {
            if (botAI->IsDps(bot) ||
                (botAI->IsTank(bot) &&
                 (mace->GetVictim() == bot || dagger->GetVictim() == bot || sword->GetVictim() == bot)))
            {
                const float safeDistance = 8.0f;
                float currentDistance = bot->GetExactDist2d(axe);
                if (currentDistance < safeDistance)
                    return MoveAway(axe, safeDistance - currentDistance + 1.0f);
            }
        }
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

            if (bot->GetTarget() != staff->GetGUID())
            {
                bot->SetTarget(staff->GetGUID());
                return Attack(staff);
            }
            return false;
        }
        // Priority 2: Cosmic Infuser (Skull)
        if (mace)
        {
            MarkTargetWithSkull(bot, mace);
            SetRtiTarget(botAI, "skull", mace);

            if (bot->GetTarget() != mace->GetGUID())
            {
                bot->SetTarget(mace->GetGUID());
                return Attack(mace);
            }
            return false;
        }
        // Priority 3: Warp Slicer (Triangle)
        if (sword)
        {
            MarkTargetWithSkull(bot, sword);
            SetRtiTarget(botAI, "skull", sword);

            if (bot->GetTarget() != sword->GetGUID())
            {
                bot->SetTarget(sword->GetGUID());
                return Attack(sword);
            }
            return false;
        }
        // Priority 4: Infinity Blades (Star)
        if (dagger)
        {
            MarkTargetWithSkull(bot, dagger);
            SetRtiTarget(botAI, "skull", dagger);

            if (bot->GetTarget() != dagger->GetGUID())
            {
                bot->SetTarget(dagger->GetGUID());
                return Attack(dagger);
            }
            return false;
        }
        // Priority 5: Netherstrand Longbow (Cross)
        if (longbow)
        {
            SetRtiTarget(botAI, "cross", longbow);

            if (bot->GetTarget() != longbow->GetGUID())
            {
                bot->SetTarget(longbow->GetGUID());
                return Attack(longbow);
            }
            return false;
        }
        // Priority 6: Devastation - Ranged DPS only (Diamond)
        if (axe && botAI->IsRangedDps(bot))
        {
            SetRtiTarget(botAI, "diamond", axe);

            if (bot->GetTarget() != axe->GetGUID())
            {
                bot->SetTarget(axe->GetGUID());
                return Attack(axe);
            }
            return false;
        }
        // Priority 7: Phaseshift Bulwark (Skull)
        if (shield)
        {
            MarkTargetWithSkull(bot, shield);
            SetRtiTarget(botAI, "skull", shield);

            if (bot->GetTarget() != shield->GetGUID())
            {
                bot->SetTarget(shield->GetGUID());
                return Attack(shield);
            }
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
        const float safeDistance = 8.0f;
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

    for (const auto& weapon : weapons)
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
    // Get equipped ranged weapon
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
    // Enable disperse at the start of phases 1 and 3 for ranged DPS except Capernian tank
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (botAI->IsRangedDps(bot) && bot != capernianTank)
    {
        const float desiredDisperse = 6.0f;
        float currentDisperse = AI_VALUE(float, "disperse distance");
        if (currentDisperse < 0.0f || currentDisperse < 0.0f != desiredDisperse)
            SET_AI_VALUE(float, "disperse distance", desiredDisperse);
    }

    // Target priority 1: Thaladred for all dps except Capernian tank
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    bool isCapernianTank = (capernianTank && bot == capernianTank);

    if (thaladred && thaladred->IsAlive() && !isCapernianTank &&
        !thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        MarkTargetWithSquare(bot, thaladred);
        SetRtiTarget(botAI, "square", thaladred);

        if (bot->GetTarget() != thaladred->GetGUID())
        {
            bot->SetTarget(thaladred->GetGUID());
            return Attack(thaladred);
        }

        return false;
    }

    // Target priority 2: Capernian for ranged
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (capernian && capernian->IsAlive() && botAI->IsRangedDps(bot) &&
        !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "circle", capernian);

        if (bot->GetTarget() != capernian->GetGUID())
        {
            bot->SetTarget(capernian->GetGUID());
            return Attack(capernian);
        }

        return false;
    }

    // Target priority 3: Sanguinar for all dps
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (sanguinar && sanguinar->IsAlive() &&
        !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "star", sanguinar);

        if (bot->GetTarget() != sanguinar->GetGUID())
        {
            bot->SetTarget(sanguinar->GetGUID());
            return Attack(sanguinar);
        }

        return false;
    }

    // Target priority 4: Telonicus for all dps
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (telonicus && telonicus->IsAlive() &&
        !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
    {
        SetRtiTarget(botAI, "triangle", telonicus);

        if (bot->GetTarget() != telonicus->GetGUID())
        {
            bot->SetTarget(telonicus->GetGUID());
            return Attack(telonicus);
        }

        // Melee DPS positioning: stay at max-ish melee range behind Telonicus (bomb safety)
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

                if (!bot->IsWithinDist2d(targetX, targetY, 1.0f))
                {
                    return MoveTo(telonicus->GetMapId(), targetX, targetY, telonicus->GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
            }
        }
    }

    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event event)
{
    // Disable disperse in Phase 4
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
    return MoveTo(bot->GetMapId(), safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
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
            float z = bot->GetPositionZ();

            Position testPos(x, y, z);

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

// Need to revise something with eggs--bots are not attacking them in time. Consider ignoring them maybe?
bool KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction::Execute(Event event)
{
    // Handle phoenix tanking for assist tanks
    if (botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1))
    {
        LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: entered assist-tank branch for bot={} guid={}", bot->GetName(), bot->GetGUID().ToString());

        // Get all phoenixes
        std::vector<Unit*> phoenixes;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_PHOENIX && unit->IsAlive())
                phoenixes.push_back(unit);
        }

        LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: found {} phoenix candidates for bot={}", phoenixes.size(), bot->GetName());

        if (phoenixes.empty())
        {
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: no phoenixes found, returning false for bot={}", bot->GetName());
            return false;
        }

        // Sort phoenixes by GUID for consistent targeting
        std::sort(phoenixes.begin(), phoenixes.end(),
                  [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

        Unit* targetPhoenix = nullptr;
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            // Assist Tank 0: Take first phoenix (Square)
            targetPhoenix = phoenixes[0];
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: assistTankIndex=0 assigning phoenix guid={} entry={} to bot={}", targetPhoenix->GetGUID().ToString(), targetPhoenix->GetEntry(), bot->GetName());
            MarkTargetWithSquare(bot, targetPhoenix);
            SetRtiTarget(botAI, "square", targetPhoenix);
        }
        else if (botAI->IsAssistTankOfIndex(bot, 1) && phoenixes.size() >= 2)
        {
            // Assist Tank 1: Take second phoenix (Circle)
            targetPhoenix = phoenixes[1];
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: assistTankIndex=1 assigning phoenix guid={} entry={} to bot={}", targetPhoenix->GetGUID().ToString(), targetPhoenix->GetEntry(), bot->GetName());
            MarkTargetWithCircle(bot, targetPhoenix);
            SetRtiTarget(botAI, "circle", targetPhoenix);
        }
        else
        {
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: assist tank index not matched or not enough phoenixes for bot={}", bot->GetName());
        }

        if (!targetPhoenix)
        {
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: no targetPhoenix assigned, returning false for bot={}", bot->GetName());
            return false;
        }

        // Attack the assigned phoenix
        if (bot->GetTarget() != targetPhoenix->GetGUID())
        {
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: bot={} setting target to phoenix guid={} and attacking", bot->GetName(), targetPhoenix->GetGUID().ToString());
            bot->SetTarget(targetPhoenix->GetGUID());
            return Attack(targetPhoenix);
        }
        else
        {
            LOG_DEBUG("playerbots", "KaelthasRoundUpPhoenixes: bot={} already targeting phoenix guid={}", bot->GetName(), targetPhoenix->GetGUID().ToString());
        }

        // If tanking the phoenix, kite it away from other players
        if (targetPhoenix->GetVictim() == bot)
        {
            const float safeDistance = 10.0f;

            // Find nearest non-phoenix-tank player within danger radius
            Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);

            // Filter out other phoenix tanks
            Group* group = bot->GetGroup();
            if (nearestPlayer && group)
            {
                PlayerbotAI* nearestAI = GET_PLAYERBOT_AI(nearestPlayer->ToPlayer());
                if (nearestAI && (nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 0) ||
                                   nearestAI->IsAssistTankOfIndex(nearestPlayer->ToPlayer(), 1)))
                {
                    nearestPlayer = nullptr; // Ignore other phoenix tanks
                }
            }

            // If too close to a player, kite away
            if (nearestPlayer)
            {
                float closestDist = bot->GetExactDist2d(nearestPlayer);
                if (closestDist < safeDistance)
                {
                    LOG_DEBUG("playerbots", "KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction: {} kiting phoenix away from raid",
                              bot->GetName());
                    // return MoveAway(nearestPlayer, safeDistance - closestDist + 2.0f);
                    return MoveFromGroup(safeDistance + 2.0f);
                }
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

    if (botAI->IsRangedDps(bot) ||
        (botAI->IsMelee(bot) && botAI->IsDps(bot) && IsKaelthasInPhase4To5Transition(botAI)))
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
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

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

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(bot->GetMapId(), mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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

    if (IsKaelthasMapIDTimerManager(botAI, bot))
        MarkTargetWithSkull(bot, kaelthas);

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
        SetRtiTarget(botAI, "skull", kaelthas);
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
        float z = bot->GetPositionZ();

        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
