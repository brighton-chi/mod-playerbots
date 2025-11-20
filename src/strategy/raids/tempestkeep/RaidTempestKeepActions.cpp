#include <ctime>
#include <sstream>

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

    // List of platforms
    std::vector<Position> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };

    // Update last platform snapshot if needed
    UpdateAlarLastPlatform(alar, mapId, platforms);
    int8 alarPlatform = lastAlarPlatform[mapId];

    SetRtiTarget(botAI, "star", alar);
    bool mtAction = PositionMainTank(botAI->IsMainTank(bot) ? bot : nullptr, alar, alarPlatform, platforms);
    bool atAction = PositionAssistTank(botAI->IsAssistTankOfIndex(bot, 0) ? bot : nullptr, alar, alarPlatform, platforms);

    return mtAction || atAction;
}

/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (mainTank && alar && alar->GetPositionZ() < 42.0f)
    {
        Position mtTarget;
        switch (alarPlatform)
        {
            case 0: // Alar at Platform 1
            case 3: // Alar at Platform 4
                mtTarget = platforms[0]; // Platform 1
                break;
            case 1: // Alar at Platform 2
            case 2: // Alar at Platform 3
                mtTarget = platforms[2]; // Platform 3
                break;
            default:
                return false;
        }

        if (mainTank->GetExactDist2d(mtTarget.GetPositionX(), mtTarget.GetPositionY()) > 10.0f && mainTank->GetPositionZ() >= 17.0f)
        {
            // Teleport directly to platform
            const uint32 mapId = alar->GetMapId();
            bot->TeleportTo(mapId, mtTarget.GetPositionX(), mtTarget.GetPositionY(), mtTarget.GetPositionZ(), mainTank->GetOrientation());
            return true;
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
} */
/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!mainTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SW ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && mainTank->GetPositionZ() < -2.0f)
    {
        if (mainTank->GetExactDist2d(AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY(), AlarSWRampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true; // Hold position at ramp base
    }

    // Track if main tank is at platform 2
    if (mainTank->GetExactDist2d(platforms[1].GetPositionX(), platforms[1].GetPositionY()) < 2.0f)
        mainTankAtPlatform2[mainTank->GetGUID()] = true;
    else if (mainTank->GetExactDist2d(platforms[0].GetPositionX(), platforms[0].GetPositionY()) < 2.0f)
        mainTankAtPlatform2[mainTank->GetGUID()] = false;

    Position mtTarget;
    std::vector<Position> midpoints;

    // Determine target and midpoints based on Al'ar's platform
    if (alarPlatform == 0 || alarPlatform == 3)
    {
        mtTarget = platforms[0]; // Move to platform 1

        // Only use waypoints if main tank is at platform 2
        if (mainTankAtPlatform2[mainTank->GetGUID()])
            midpoints = std::vector<Position>(midpoints_2_to_1.begin(), midpoints_2_to_1.begin() + 2); // Backward (2->1)
        else
            midpoints.clear(); // Move directly to platform 1
    }
    else if (alarPlatform == 1 || alarPlatform == 2)
    {
        mtTarget = platforms[1]; // Move to platform 2
    midpoints = std::vector<Position>(midpoints_1_to_2.begin(), midpoints_1_to_2.begin() + 2); // Forward (1->2)
    }
    else
        return false;

    // If already at target, update state and attack
    if (mainTank->GetExactDist2d(mtTarget.GetPositionX(), mtTarget.GetPositionY()) < 2.0f)
    {
        mtBalconyMidpointVisited[mainTank->GetGUID()].clear();

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

    // If movement is needed, go through midpoints
    if (!midpoints.empty())
    {
        if (mtBalconyMidpointVisited[mainTank->GetGUID()].size() != midpoints.size())
            mtBalconyMidpointVisited[mainTank->GetGUID()] = std::vector<bool>(midpoints.size(), false);

        for (size_t i = 0; i < midpoints.size(); ++i)
        {
            if (!mtBalconyMidpointVisited[mainTank->GetGUID()][i])
            {
                const Position& wp = midpoints[i];
                if (mainTank->GetExactDist2d(wp.GetPositionX(), wp.GetPositionY()) >= 2.0f)
                {
                    return MoveTo(bot->GetMapId(), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ(), false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    mtBalconyMidpointVisited[mainTank->GetGUID()][i] = true;
                break;
            }
        }
    }

    // After all midpoints are visited, move to final target
    if (mainTank->GetExactDist2d(mtTarget.GetPositionX(), mtTarget.GetPositionY()) >= 2.0f)
    {
        return MoveTo(bot->GetMapId(), mtTarget.GetPositionX(), mtTarget.GetPositionY(), mtTarget.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
} */

bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!mainTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SW ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && mainTank->GetPositionZ() < -2.0f)
    {
        if (mainTank->GetExactDist2d(AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSWRampBase.GetPositionX(), AlarSWRampBase.GetPositionY(), AlarSWRampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true; // Hold position at ramp base
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

    // If already at target, update state and attack
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

    // Move directly to target platform
    return MoveTo(bot->GetMapId(), mtTarget.GetPositionX(), mtTarget.GetPositionY(), mtTarget.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (assistTank && alar && alar->GetPositionZ() < 42.0f)
    {
        Position atTarget;
        switch (alarPlatform)
        {
        case 0: // Al'ar at Platform 1
        case 1: // Al'ar at Platform 2
            atTarget = platforms[1]; // Platform 2
            break;
        case 2: // Al'ar at Platform 3
        case 3: // Al'ar at Platform 4
            atTarget = platforms[3]; // Platform 4
            break;
        default:
            return false;
        }

        if (assistTank->GetExactDist2d(atTarget.GetPositionX(), atTarget.GetPositionY()) > 10.0f && assistTank->GetPositionZ() >= 17.0f)
        {
            const uint32 mapId = alar->GetMapId();
            bot->TeleportTo(mapId, atTarget.GetPositionX(), atTarget.GetPositionY(), atTarget.GetPositionZ(), assistTank->GetOrientation());
            return true;
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
} */

/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!assistTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SE ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && assistTank->GetPositionZ() < -2.0f)
    {
        if (assistTank->GetExactDist2d(AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY(), AlarSERampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true; // Hold position at ramp base
    }

    // Track if assist tank is at platform 3
    if (assistTank->GetExactDist2d(platforms[2].GetPositionX(), platforms[2].GetPositionY()) < 2.0f)
        assistTankAtPlatform3[assistTank->GetGUID()] = true;
    else if (assistTank->GetExactDist2d(platforms[3].GetPositionX(), platforms[3].GetPositionY()) < 2.0f)
        assistTankAtPlatform3[assistTank->GetGUID()] = false;

    Position atTarget;
    std::vector<Position> midpoints;

    // Determine target and midpoints based on Al'ar's platform
    if (alarPlatform == 0 || alarPlatform == 3)
    {
        atTarget = platforms[3]; // Move to platform 4

        // Only use waypoints if assist tank is at platform 3
        if (assistTankAtPlatform3[assistTank->GetGUID()])
            midpoints = std::vector<Position>(midpoints_3_to_4.begin(), midpoints_3_to_4.begin() + 2); // Backward (3->4)
        else
            midpoints.clear(); // Move directly to platform 4
    }
    else if (alarPlatform == 1 || alarPlatform == 2)
    {
        atTarget = platforms[2]; // Move to platform 3
    midpoints = std::vector<Position>(midpoints_4_to_3.begin(), midpoints_4_to_3.begin() + 2); // Forward (4->3)
    }
    else
        return false;

    // If already at target, update state and attack
    if (assistTank->GetExactDist2d(atTarget.GetPositionX(), atTarget.GetPositionY()) < 2.0f)
    {
        atBalconyMidpointVisited[assistTank->GetGUID()].clear();

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

    // If movement is needed, go through midpoints
    if (!midpoints.empty())
    {
        if (atBalconyMidpointVisited[assistTank->GetGUID()].size() != midpoints.size())
            atBalconyMidpointVisited[assistTank->GetGUID()] = std::vector<bool>(midpoints.size(), false);

        for (size_t i = 0; i < midpoints.size(); ++i)
        {
            if (!atBalconyMidpointVisited[assistTank->GetGUID()][i])
            {
                const Position& wp = midpoints[i];
                if (assistTank->GetExactDist2d(wp.GetPositionX(), wp.GetPositionY()) >= 2.0f)
                {
                    return MoveTo(bot->GetMapId(), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ(), false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    atBalconyMidpointVisited[assistTank->GetGUID()][i] = true;
                break;
            }
        }
    }

    // After all midpoints are visited, move to final target
    if (assistTank->GetExactDist2d(atTarget.GetPositionX(), atTarget.GetPositionY()) >= 2.0f)
    {
        return MoveTo(bot->GetMapId(), atTarget.GetPositionX(), atTarget.GetPositionY(), atTarget.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
} */

bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Position>& platforms)
{
    if (!assistTank || !alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to SE ramp base and hold position
    if (alar->GetPositionZ() >= 42.0f && assistTank->GetPositionZ() < -2.0f)
    {
        if (assistTank->GetExactDist2d(AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY()) >= 2.0f)
        {
            return MoveTo(bot->GetMapId(), AlarSERampBase.GetPositionX(), AlarSERampBase.GetPositionY(), AlarSERampBase.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return true; // Hold position at ramp base
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

    // If already at target, update state and attack
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

    // Move directly to target platform
    return MoveTo(bot->GetMapId(), atTarget.GetPositionX(), atTarget.GetPositionY(), atTarget.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AlarMeleeDpsPrioritizeAddsAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // If Al'ar is flying (Flame Quills), move to S of room and hold position
    if (alar->GetPositionZ() >= 42.0f && bot->GetPositionZ() < -2.0f)
    {
        if (bot->GetExactDist2d(AlarRoomSouthCenter.GetPositionX(), AlarRoomSouthCenter.GetPositionY()) >= 2.0f)
        {
            LOG_DEBUG("playerbots", "Bot {} moving to South Center during Flame Quills", bot->GetName());
            return MoveTo(bot->GetMapId(), AlarRoomSouthCenter.GetPositionX(), AlarRoomSouthCenter.GetPositionY(), AlarRoomSouthCenter.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        LOG_DEBUG("playerbots", "Bot {} holding position at South Center during Flame Quills", bot->GetName());
        return true;
    }

    const uint32 mapId = alar->GetMapId();
    int8 alarPlatform = lastAlarPlatform[mapId];

    std::vector<Position> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };
    const Position& platformTarget = platforms[alarPlatform];

    std::vector<Position> waypoints;
    if (alarPlatform == 1)
        waypoints = { AlarPlatform1To2MidpointA, AlarPlatform1To2MidpointB };
    else if (alarPlatform == 2)
        waypoints = { AlarPlatform2To3MidpointA, AlarPlatform2To3MidpointB };

    // Clear waypoint tracker if at target platform (before any movement logic)
    if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) < 2.0f)
    {
        LOG_DEBUG("playerbots", "Bot {} reached platform {}. Clearing waypoint tracker.", bot->GetName(), int(alarPlatform + 1));
        meleeDpsWaypointVisited[bot->GetGUID()].clear();
    }

    if (!waypoints.empty())
    {
        if (meleeDpsWaypointVisited[bot->GetGUID()].empty() && bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
        {
            LOG_DEBUG("playerbots", "Bot {} initializing waypoint tracker for movement to platform {}", bot->GetName(), int(alarPlatform + 1));
            meleeDpsWaypointVisited[bot->GetGUID()] = std::vector<bool>(waypoints.size(), false);
        }

        for (size_t i = 0; i < waypoints.size(); ++i)
        {
            if (!meleeDpsWaypointVisited[bot->GetGUID()][i])
            {
                const Position& wp = waypoints[i];
                if (bot->GetExactDist2d(wp.GetPositionX(), wp.GetPositionY()) >= 2.0f)
                {
                    LOG_DEBUG("playerbots", "Bot {} moving to waypoint {} for platform {}: ({}, {}, {})", bot->GetName(), i + 1, int(alarPlatform + 1), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ());
                    return MoveTo(bot->GetMapId(), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ(), false, false, false, true,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                else
                {
                    LOG_DEBUG("playerbots", "Bot {} reached waypoint {} for platform {}: ({}, {}, {})", bot->GetName(), i + 1, int(alarPlatform + 1), wp.GetPositionX(), wp.GetPositionY(), wp.GetPositionZ());
                    meleeDpsWaypointVisited[bot->GetGUID()][i] = true;
                }
                break;
            }
        }
        if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
        {
            LOG_DEBUG("playerbots", "Bot {} moving to final platform {}: ({}, {}, {})", bot->GetName(), int(alarPlatform + 1), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ());
            return MoveTo(bot->GetMapId(), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else
    {
        if (bot->GetExactDist2d(platformTarget.GetPositionX(), platformTarget.GetPositionY()) >= 2.0f)
        {
            LOG_DEBUG("playerbots", "Bot {} moving directly to platform {}: ({}, {}, {})", bot->GetName(), int(alarPlatform + 1), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ());
            return MoveTo(bot->GetMapId(), platformTarget.GetPositionX(), platformTarget.GetPositionY(), platformTarget.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    if (bot->GetVictim() != alar)
    {
        LOG_DEBUG("playerbots", "Bot {} attacking Al'ar on platform {}", bot->GetName(), int(alarPlatform + 1));
        return Attack(alar);
    }

    return false;
}

bool AlarRangedDpsPrioritizeBossAction::Execute(Event event)
{
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember && ember->IsAlive() /*&& ember->GetHealthPct() < 15.0f*/)
    {
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetTarget() != ember->GetGUID())
        {
            bot->SetSelection(ember->GetGUID());
            return Attack(ember);
        }
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (alar && (!ember /*|| !ember->GetHealthPct() >= 15.0f*/))
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

        // List of ground positions matching platforms
        std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Position& groundTarget = groundPositions[alarPlatform];

        // Only move if not already within 15 yards
        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 15.0f)
        {
            // Move near the ground target (e.g., 5 yards away for some spread)
            return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(), groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarPositionHealerAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();
    int8 alarPlatform = lastAlarPlatform[mapId];

    // List of ground positions matching platforms
    std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
    const Position& groundTarget = groundPositions[alarPlatform];

    // Only move if not already within 15 yards
    if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 15.0f)
    {
        // Move near the ground target (e.g., 5 yards away for some spread)
        return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(), groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
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
    if (ember)
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

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
    }
    else if (!isPhase2[mapId])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];

        // List of ground positions matching platforms
        std::vector<Position> groundPositions = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Position& groundTarget = groundPositions[alarPlatform];

        // Only move if not already within 30 yards
        if (bot->GetExactDist2d(groundTarget.GetPositionX(), groundTarget.GetPositionY()) > 30.0f)
        {
            // Move near the ground target (e.g., 5 yards away for some spread)
            return MoveNear(bot->GetMapId(), groundTarget.GetPositionX(), groundTarget.GetPositionY(), groundTarget.GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AlarJumpFromPlatformAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // Only jump if bot is on a platform (Z >= 17.0f)
    if (bot->GetPositionZ() >= 17.0f)
    {
        // List of jump and landing pairs
        std::vector<std::pair<Position, Position>> platformGroundPairs =
        {
            {AlarPlatform1, AlarGround1},
            {AlarPlatform2, AlarGround2},
            {AlarPlatform3, AlarGround3},
            {AlarPlatform4, AlarGround4}
        };

        // Find nearest platform
        float minDist = std::numeric_limits<float>::max();
        size_t nearestIndex = 0;
        for (size_t i = 0; i < platformGroundPairs.size(); ++i)
        {
        float dist = bot->GetExactDist2d(platformGroundPairs[i].first.GetPositionX(), platformGroundPairs[i].first.GetPositionY());
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }

    // Jump to corresponding landing poisition
    const Position& ground = platformGroundPairs[nearestIndex].second;
    return JumpTo(bot->GetMapId(), ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    /* const Position& atMidpoint = AlarMidpointToSERamp;
    const Position& atTarget = AlarSERampBase;
    const Position& mtMidpoint = AlarMidpointToSWRamp;
    const Position& mtTarget = AlarSWRampBase;
    if (botAI->IsMainTank(bot) && bot->GetPositionZ() < -2.0f)
    {
        if (!mtGroundMidpointVisited[bot->GetGUID()])
        {
            if (bot->GetExactDist2d(mtMidpoint.GetPositionX(), mtMidpoint.GetPositionY()) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), mtMidpoint.GetPositionX(), mtMidpoint.GetPositionY(), mtMidpoint.GetPositionZ(), false, false, false, true,
                                MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            else
                mtGroundMidpointVisited[bot->GetGUID()] = true;
        }
        // MT heads to SW ramp
        return MoveTo(bot->GetMapId(), mtTarget.GetPositionX(), mtTarget.GetPositionY(), mtTarget.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0) && bot->GetPositionZ() < -2.0f)
    {
        // Only use midpoint if landing at groundposition2
        if (!atGroundMidpointVisited[bot->GetGUID()])
        {
            if (bot->GetExactDist2d(atMidpoint.GetPositionX(), atMidpoint.GetPositionY()) >= 1.0f)
            {
                return MoveTo(bot->GetMapId(), atMidpoint.GetPositionX(), atMidpoint.GetPositionY(), atMidpoint.GetPositionZ(), false, false, false, true,
                                MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            else
                atGroundMidpointVisited[bot->GetGUID()] = true;
        }
        // AT0 heads to SE ramp
        return MoveTo(bot->GetMapId(), atTarget.GetPositionX(), atTarget.GetPositionY(), atTarget.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    } */

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
    if (bot->GetPositionZ() >= 17.0f)
    {
        // List of jump and landing pairs
        std::vector<std::pair<Position, Position>> platformGroundPairs =
        {
            {AlarPlatform1, AlarGround1},
            {AlarPlatform2, AlarGround2},
            {AlarPlatform3, AlarGround3},
            {AlarPlatform4, AlarGround4}
        };

        // Find nearest platform
        float minDist = std::numeric_limits<float>::max();
        size_t nearestIndex = 0;
        for (size_t i = 0; i < platformGroundPairs.size(); ++i)
        {
        float dist = bot->GetExactDist2d(platformGroundPairs[i].first.GetPositionX(), platformGroundPairs[i].first.GetPositionY());
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }

    // Jump to corresponding landing position
    const Position& ground = platformGroundPairs[nearestIndex].second;
    return JumpTo(bot->GetMapId(), ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
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
    if (mainTank && assistTank && alar->GetVictim() == mainTank && mainTank->HasAura(SPELL_MELT_ARMOR) && bot == assistTank)
    {
        const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
        for (const char* spellName : taunts)
        {
            if (botAI->CanCastSpell(spellName, alar))
                return botAI->CastSpell(spellName, alar);
        }
    }

    // If assist tank is tanking Al'ar and has Melt Armor, main tank should taunt
    if (mainTank && assistTank && alar->GetVictim() == assistTank && assistTank->HasAura(SPELL_MELT_ARMOR) && bot == mainTank)
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
        return MoveInside(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(), 25.0f,
                          MovementPriority::MOVEMENT_COMBAT);
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

    // Reset tracker at the start of the fight (Al'ar at max health)
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
            // mtGroundMidpointVisited[bot->GetGUID()] = false;
            // lastMainTankPlatform[bot->GetGUID()] = 0; // Platform 1
        }
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            atBalconyMidpointVisited[bot->GetGUID()].clear();
            assistTankAtPlatform3[bot->GetGUID()] = false;
            // atGroundMidpointVisited[bot->GetGUID()] = false;
            // lastAssistTankPlatform[bot->GetGUID()] = 1; // Platform 2
        }
        if (botAI->IsMelee(bot) && botAI->IsDps(bot))
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
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY());

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

    // Only clear positions at the very start of the fight (boss at max health)
    if (voidReaver->GetHealth() == voidReaver->GetMaxHealth())
    {
        initialVoidReaverPositions.clear();
        hasReachedInitialVoidReaverPosition.clear();
        LOG_DEBUG("playerbots", "VoidReaverSpreadRangedAction: Cleared initial positions (Void Reaver at max health)");
    }

    // Assign positions only if not already assigned
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
            LOG_DEBUG("playerbots", "VoidReaverSpreadRangedAction: {} assigned ring {} position ({}, {}, {})",
                ranged->GetName(), ringIndex + 1, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        }
    }

    // Move bot to its assigned position
    Position targetPosition = initialVoidReaverPositions[bot->GetGUID()];
    float destX = targetPosition.GetPositionX();
    float destY = targetPosition.GetPositionY();
    float destZ = targetPosition.GetPositionZ();

    if (!bot->IsWithinDist2d(destX, destY, 1.0f))
    {
        LOG_DEBUG("playerbots", "VoidReaverSpreadRangedAction: Bot {} moving to initial position ({}, {}, {})", bot->GetName(), destX, destY, destZ);

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
            return MoveTo(bot->GetMapId(), stackTarget->GetPositionX(), stackTarget->GetPositionY(), stackTarget->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float currentDistance = bot->GetExactDist2d(member);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
            return MoveAway(member, safeDistance - currentDistance + 0.5f);
    }

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
    Unit* targetSolariumPriest = AssignSolariumPriestsToBots(solariumPriests, meleeMembers);
    if (!targetSolariumPriest)
        return false;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();
    if (botIndex < totalMelee / 2)
    {
        MarkTargetWithSquare(bot, targetSolariumPriest);
        SetRtiTarget(botAI, "square", targetSolariumPriest);
    }
    else
    {
        MarkTargetWithStar(bot, targetSolariumPriest);
        SetRtiTarget(botAI, "star", targetSolariumPriest);
    }

    if (bot->GetVictim() != targetSolariumPriest)
        return Attack(targetSolariumPriest);

    if (!bot->IsWithinMeleeRange(targetSolariumPriest))
        return MoveTo(bot->GetMapId(), targetSolariumPriest->GetPositionX(), targetSolariumPriest->GetPositionY(),
                      targetSolariumPriest->GetPositionZ(), false, false, false, false,
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

    // Sort priest adds by GUID for consistent targeting
    std::vector<Unit*> sortedSolariumPriests = solariumPriests;
    std::sort(sortedSolariumPriests.begin(), sortedSolariumPriests.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (botIndex < totalMelee / 2)
        return sortedSolariumPriests[0];
    else
        return sortedSolariumPriests[1];
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

bool KaelthasSunstriderLogForTestingAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    static std::unordered_map<uint32, std::chrono::steady_clock::time_point> kaelthasFightStart;
    const uint32 mapId = kaelthas->GetMapId();
    auto now = std::chrono::steady_clock::now();
    if (kaelthasFightStart.find(mapId) == kaelthasFightStart.end())
    {
        kaelthasFightStart[mapId] = now;
        LOG_DEBUG("playerbots", "KaelthasSunstriderLogForTestingAction: Kael'thas fight START on map={} at steady_clock", mapId);
    }
    else
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - kaelthasFightStart[mapId]).count();
        LOG_DEBUG("playerbots", "KaelthasSunstriderLogForTestingAction: Kael'thas fight elapsed_ms={} on map={}", elapsed, mapId);
    }

    return false;
}

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

        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Bot {} at final position, Thaladred too close, moving away",
                    bot->GetName());
        if (!bot->HasAura(SPELL_SPRINT))
            bot->AddAura(SPELL_SPRINT, bot);

        return MoveAway(thaladred, safeDistance - currentDistance + 5.0f);
    }

    return false;
}

Position KaelthasSunstriderKiteThaladredAction::GetTargetPosition(uint8 relayPhase)
{
    // This method is no longer needed but keep for potential future use
    switch (relayPhase)
    {
        case 0: return ThaladredRelayPoint;
        case 1:
        case 2:
        default:
            return ThaladredFinalPosition;
    }
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
            float moveDist = std::min(7.0f, dist);
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

    // Phase 1: Single advisor phase - warlock should be tank (for Capernian)
    if (IsKaelthasInPhase1(botAI))
    {
        if (!currentlyTank)
        {
            Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
            Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
            if (thaladred && !thaladred->IsAlive() && sanguinar && !sanguinar->IsAlive())
            {
                // Wait until first two advisors are dead before switching to tank
                LOG_DEBUG("playerbots", "KaelthasSunstriderManageWarlockTankStrategyAction: {} adding TANK strategy for Phase 1",
                        bot->GetName());
                botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
            }
        }
        return false;
    }

    // Phase 2→3 Transition: Weapons dead, waiting for advisors - warlock should be tank
    if (IsKaelthasInPhase2(botAI) && AreAllLegendaryWeaponsDead(botAI, bot))
    {
        if (!currentlyTank)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderManageWarlockTankStrategyAction: {} adding TANK strategy for Phase 2->3 transition",
                      bot->GetName());
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        }
        return false;
    }

    // Phase 2: Weapons phase - warlock should be DPS
    if (IsKaelthasInPhase2(botAI))
    {
        if (currentlyTank)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderManageWarlockTankStrategyAction: {} reset botAI for Phase 2",
                      bot->GetName());
            botAI->ResetStrategies(false);
        }
        return false;
    }

    // Phase 3: All advisors phase - handle Capernian
    if (IsKaelthasInPhase3(botAI))
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");

        // If Capernian is alive and warlock is not tanking, add tank strategy (failsafe)
        if (capernian && capernian->IsAlive() && !currentlyTank)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderManageWarlockTankStrategyAction: {} adding TANK strategy for Phase 3 (Capernian alive failsafe)",
                      bot->GetName());
            botAI->ChangeStrategy("+tank", BotState::BOT_STATE_COMBAT);
        }
        // If Capernian is dead and still tanking, remove tank strategy
        else if (capernian && !capernian->IsAlive() && currentlyTank)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderManageWarlockTankStrategyAction: {} reset botAI for Phase 3 (Capernian dead)",
                      bot->GetName());
            botAI->ResetStrategies(false);
        }
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

    if (capernian->GetVictim() == bot)
    {
        const float minDistance = 31.0f;
        const float maxDistance = 34.0f;

        // current 2D distance to boss
        float currentDist = bot->GetExactDist2d(capernian);
        if (currentDist < minDistance || currentDist > maxDistance)
        {
            // move to nearest boundary of allowed range
            float desiredDist = (currentDist < minDistance) ? minDistance : maxDistance;

            float dx = bot->GetPositionX() - capernian->GetPositionX();
            float dy = bot->GetPositionY() - capernian->GetPositionY();
            float distXY = bot->GetExactDist2d(capernian);

            if (distXY == 0.0f)
                return false;

            dx /= distXY;
            dy /= distXY;

            float targetX = capernian->GetPositionX() + dx * desiredDist;
            float targetY = capernian->GetPositionY() + dy * desiredDist;

            if (!bot->IsWithinDist2d(targetX, targetY, 1.0f))
                return MoveTo(bot->GetMapId(), targetX, targetY, capernian->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
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

    // Determine safe distance based on role
    float safeDistance;
    Player* capernianTank = GetCapernianTank(botAI, bot);

    if (botAI->IsMelee(bot))
    {
        if (botAI->IsTank(bot) && IsKaelthasInPhase1(botAI))
            safeDistance = 15.0f;  // Tank safe distance
        else
            safeDistance = 35.0f;  // Melee dps safe distance
    }
    else if (botAI->IsRangedDps(bot))
    {
        safeDistance = 30.5f;  // Ranged DPS safe distance
    }
    else if (botAI->IsHeal(bot))
    {
        safeDistance = 32.0f;  // Healer safe distance
    }
    else
    {
        return false;  // No action needed for other roles
    }

    // Check if bot is inside danger zone
    float currentDistance = bot->GetExactDist2d(capernian);
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(capernian, safeDistance - currentDistance + 1.0f);
    }

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
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find up to 3 hunters in the group
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
        if (hunters.size() >= 3)
            break;
    }

    // Find hunter index
    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    // Determine boss and tank targets based on hunter index
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
        advisorTarget = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member)->IsMainTank(member))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!advisorTarget || !advisorTarget->IsAlive() || advisorTarget->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
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

        // If any advisor is at 100% HP and aggressive, set timer
        if (advisor->GetHealth() == advisor->GetMaxHealth() && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            const uint32 mapId = advisor->GetMapId();
            const time_t now = std::time(nullptr);
            advisorDpsWaitTimer.insert_or_assign(mapId, now);

            LOG_DEBUG("playerbots", "KaelthasSunstriderManageAdvisorDpsTimerAction: Timer set for {} on map {}",
                      name, mapId);
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

    // Priority 0 (excluding tanks): Stay away from Devastation
    if (!botAI->IsTank(bot) && axe && axe->IsAlive())
    {
        const float safeDistance = 7.0f;
        float currentDistance = bot->GetExactDist2d(axe);
        if (currentDistance < safeDistance)
            return MoveAway(axe, safeDistance - currentDistance + 1.0f);
    }

    // Priority 1 (excluding tanks): Staff of Disintegration (Circle)
    if (staff && staff->IsAlive() && botAI->IsDps(bot))
    {
        MarkTargetWithCircle(bot, staff);
        SetRtiTarget(botAI, "circle", staff);

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

    // Priority 2 (excluding tanks): Cosmic Infuser (Star)
    if (mace && mace->IsAlive() && botAI->IsDps(bot))
    {
        MarkTargetWithStar(bot, mace);
        SetRtiTarget(botAI, "star", mace);

        if (bot->GetTarget() != mace->GetGUID())
        {
            bot->SetTarget(mace->GetGUID());
            return Attack(mace);
        }
        return false;
    }

    // Priority 3 (including first assist tank): Warp Slicer (Square)
    if (sword && sword->IsAlive() && (botAI->IsDps(bot) || botAI->IsAssistTankOfIndex(bot, 0)))
    {
        MarkTargetWithSquare(bot, sword);
        SetRtiTarget(botAI, "square", sword);

        if (bot->GetTarget() != sword->GetGUID())
        {
            bot->SetTarget(sword->GetGUID());
            return Attack(sword);
        }
        return false;
    }

    // Priority 4 (excluding tanks): Infinity Blades (Triangle)
    if (dagger && dagger->IsAlive() && botAI->IsDps(bot))
    {
        MarkTargetWithTriangle(bot, dagger);
        SetRtiTarget(botAI, "triangle", dagger);

        if (bot->GetTarget() != dagger->GetGUID())
        {
            bot->SetTarget(dagger->GetGUID());
            return Attack(dagger);
        }
        return false;
    }

    // Priority 5: Netherstrand Longbow (Cross)
    if (longbow && longbow->IsAlive())
    {
        MarkTargetWithCross(bot, longbow);
        SetRtiTarget(botAI, "cross", longbow);

        if (bot->GetTarget() != longbow->GetGUID())
        {
            bot->SetTarget(longbow->GetGUID());
            return Attack(longbow);
        }
        return false;
    }

    // Priority 6: Devastation - Ranged DPS only (Diamond)
    if (axe && axe->IsAlive() && botAI->IsRangedDps(bot))
    {
        MarkTargetWithDiamond(bot, axe);
        SetRtiTarget(botAI, "diamond", axe);

        if (bot->GetTarget() != axe->GetGUID())
        {
            bot->SetTarget(axe->GetGUID());
            return Attack(axe);
        }
        return false;
    }

    // Priority 7: Phaseshift Bulwark (Skull)
    if (shield && shield->IsAlive() && (botAI->IsDps(bot) || botAI->IsAssistTankOfIndex(bot, 1)))
    {
        MarkTargetWithSkull(bot, shield);
        SetRtiTarget(botAI, "skull", shield);

        if (bot->GetTarget() != shield->GetGUID())
        {
            bot->SetTarget(shield->GetGUID());
            return Attack(shield);
        }
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event event)
{
    Unit* devastation = AI_VALUE2(Unit*, "find target", "devastation");
    if (!devastation || !devastation->IsAlive())
        return false;

    MarkTargetWithDiamond(bot, devastation);
    SetRtiTarget(botAI, "diamond", devastation);

    if (bot->GetVictim() != devastation)
        return Attack(devastation);

    if (devastation->GetVictim() == bot)
    {
        const float safeDistance = 7.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);

        if (nearestPlayer)
        {
            float currentDistance = bot->GetExactDist2d(nearestPlayer);

            // If nearest player is inside danger zone, move away
            if (currentDistance < safeDistance)
                return MoveAway(nearestPlayer, safeDistance - currentDistance + 1.0f);
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
        // Find nearest raid member (danger zone)
        const float dangerZone = 20.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, dangerZone);

        if (nearestPlayer)
        {
            // Kite away from nearest player to turn bow away from raid
            float currentDistance = bot->GetExactDist2d(nearestPlayer);
            if (currentDistance < dangerZone)
                return MoveAway(nearestPlayer, dangerZone - currentDistance + 3.0f);
        }
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
            {
                LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: {} already has {} (itemId={}), skipping",
                          bot->GetName(), weapon.name, weapon.itemId);
                continue;
            }

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
                   (botAI->IsDps(bot) && (bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_DEATH_KNIGHT));

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
                   (bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_DEATH_KNIGHT);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId, const char* weaponName)
{
    LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: Execute start for bot {} (weaponEntry={})", bot->GetName(), weaponEntry);

    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig->lootDistance;

    for (auto const& guid : corpses)
    {
        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: inspecting guid {}", guid.ToString());

        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: loot not possible on guid {}", guid.ToString());
            continue;
        }

        WorldObject* object = loot.GetWorldObject(bot);
        if (!object)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: no world object for guid {}", guid.ToString());
            continue;
        }

        Creature* creature = object->ToCreature();
        if (!creature)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: world object for guid {} is not a creature, skipping", guid.ToString());
            continue;
        }

        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: found CREATURE guid={} entry={} hp={}/{} alive={}",
                  guid.ToString(), creature->GetEntry(), creature->GetHealth(), creature->GetMaxHealth(), creature->IsAlive());

        // Only consider the specific dead weapon
        if (creature->GetEntry() != weaponEntry || creature->IsAlive())
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: creature {} is not the target weapon or is alive, skipping", guid.ToString());
            continue;
        }

        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: found dead {} target {}", weaponName, guid.ToString());

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(object);
        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: target {} dist={}", guid.ToString(), dist);

        if (dist > maxLootRange)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: moving to target {} (dist={} > maxLootRange={})", guid.ToString(), dist, maxLootRange);
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);
        }

        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: in range of target {}, invoking OpenLootAction", guid.ToString());

        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());
        LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: OpenLootAction returned {}", opened);

        if (!opened)
            return opened;

        // Schedule autostore attempt
        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        const uint8 guessedIndex = 0;

        botAI->AddTimedEvent([this, botGuid, corpseGuid, guessedIndex, itemId, weaponName]()
        {
            Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver || !receiver->IsInWorld())
                return;

            // Double-check this bot doesn't already have the weapon
            if (receiver->HasItemCount(itemId, 1, false))
                return;

            receiver->SetLootGUID(corpseGuid);

            LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: sending CMSG_AUTOSTORE_LOOT_ITEM guessedIndex={} for bot={} corpse={} weapon={}",
                      guessedIndex, receiver->GetName(), corpseGuid.ToString(), weaponName);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << guessedIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        return true;
    }

    LOG_DEBUG("playerbots", "KaelthasSunstriderLootLegendaryWeaponsAction: no matching dead {} found, returning false", weaponName);
    return false;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event event)
{
    // Check and use Phaseshift Bulwark
    if (UsePhaseshiftBulwark())
        return true;

    // Check and use Staff of Disintegration
    if (UseStaffOfDisintegration())
        return true;

    // Check and use Netherstrand Longbow
    if (UseNetherstrandLongbow())
        return true;

    return false;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    // Get equipped off-hand (shield slot)
    Item* offHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offHand || offHand->GetEntry() != ITEM_PHASESHIFT_BULWARK)
        return false;

    // Find Kael'thas
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    // Only use if Kael'thas is casting Pyroblast
    if (!kaelthas->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* currentSpell = kaelthas->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || currentSpell->m_spellInfo->Id != SPELL_KAELTHAS_PYROBLAST)
        return false;

    // Only use if bot is the target of the Pyroblast
    Unit* pyroblastTarget = currentSpell->m_targets.GetUnitTarget();
    if (pyroblastTarget != bot)
        return false;

    LOG_DEBUG("playerbots", "KaelthasSunstriderUseLegendaryWeaponsAction: {} using Phaseshift Bulwark to block Pyroblast", bot->GetName());

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    // Get equipped main hand weapon
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mainHand || mainHand->GetEntry() != ITEM_STAFF_OF_DISINTEGRATION)
        return false;

    // Only use if bot doesn't have the protection aura
    if (bot->HasAura(SPELL_MENTAL_PROTECTION_FIELD))
        return false;

    LOG_DEBUG("playerbots", "KaelthasSunstriderUseLegendaryWeaponsAction: {} using equipped Staff of Disintegration", bot->GetName());

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    // Get equipped ranged weapon
    Item* ranged = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!ranged || ranged->GetEntry() != ITEM_NETHERSTRAND_LONGBOW)
        return false;

    if (bot->HasItemCount(ITEM_NETHER_SPIKES, 201, false))
    {
        LOG_DEBUG("playerbots", "KaelthasSunstriderUseLegendaryWeaponsAction: {} already has nether spikes, skipping bow use", bot->GetName());
        return false;
    }

    LOG_DEBUG("playerbots", "KaelthasSunstriderUseLegendaryWeaponsAction: {} using Netherstrand Longbow to create nether spikes", bot->GetName());

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

    // Find the on-use spell
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

/* bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event event)
{
    Player* capernianTank = GetCapernianTank(botAI, bot);
    bool isCapernianTank = (capernianTank && bot == capernianTank);

    // Target priority 1: Thaladred for all dps except Capernian tank
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (thaladred && thaladred->IsAlive() && !isCapernianTank)
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
    if (capernian && capernian->IsAlive() && (botAI->IsRangedDps(bot) || isCapernianTank))
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
    if (sanguinar && sanguinar->IsAlive())
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
    if (telonicus && telonicus->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", telonicus);

        if (bot->GetTarget() != telonicus->GetGUID())
        {
            bot->SetTarget(telonicus->GetGUID());
            return Attack(telonicus);
        }

        // Melee DPS positioning: stay at max-ish melee range behind Telonicus (bomb safety)
        if (botAI->IsMelee(bot) && !botAI->IsTank(bot) && telonicus->GetVictim() != bot)
        {
            float maxMeleeRange = bot->GetMeleeRange(telonicus);
            float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);

            float targetX = telonicus->GetPositionX() + (maxMeleeRange - 0.5f) * cos(behindAngle);
            float targetY = telonicus->GetPositionY() + (maxMeleeRange - 0.5f) * sin(behindAngle);

            if (bot->GetExactDist2d(targetX, targetY) > 0.5f)
            {
                return MoveTo(telonicus->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }

    return false;
} */

// logging version
bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event event)
{
    LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Enter bot={} guid={}", bot->GetName(), bot->GetGUID().ToString());

    // Target priority 1: Thaladred for all dps except Capernian tank
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (capernianTank)
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: capernianTank present name={} guid={}", capernianTank->GetName(), capernianTank->GetGUID().ToString());
    else
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: capernianTank none");

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: thaladred not found");
    }
    else
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: thaladred found entry={} guid={} alive={} non_attackable={}", thaladred->GetEntry(), thaladred->GetGUID().ToString(), thaladred->IsAlive(), thaladred->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE));
    }

    bool isCapernianTank = (capernianTank && bot == capernianTank);

    if (thaladred && thaladred->IsAlive() && !isCapernianTank)
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Selecting Thaladred (square) for bot={}", bot->GetName());
        MarkTargetWithSquare(bot, thaladred);
        SetRtiTarget(botAI, "square", thaladred);

        if (bot->GetTarget() != thaladred->GetGUID())
        {
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Setting target to Thaladred botTarget={} thaladredGuid={}", bot->GetTarget().ToString(), thaladred->GetGUID().ToString());
            bot->SetTarget(thaladred->GetGUID());
            return Attack(thaladred);
        }

        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Already targeting Thaladred targetGuid={}", bot->GetTarget().ToString());
        return false;
    }
    else
    {
        if (!thaladred)
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Thaladred reason=not found");
        else if (!thaladred->IsAlive())
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Thaladred reason=not alive", thaladred->GetHealth(), thaladred->GetMaxHealth());
        else if (isCapernianTank)
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Thaladred reason=bot is capernian tank");
    }

    // Target priority 2: Capernian for ranged
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: capernian not found");
    }
    else
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: capernian found entry={} guid={} alive={} non_attackable={} botIsRangedDps={}", capernian->GetEntry(), capernian->GetGUID().ToString(), capernian->IsAlive(), capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE), botAI->IsRangedDps(bot));
    }

    if (capernian && capernian->IsAlive() && botAI->IsRangedDps(bot))
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Selecting Capernian (circle) for ranged bot={}", bot->GetName());
        SetRtiTarget(botAI, "circle", capernian);

        if (bot->GetTarget() != capernian->GetGUID())
        {
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Setting target to Capernian botTarget={} capernianGuid={}", bot->GetTarget().ToString(), capernian->GetGUID().ToString());
            bot->SetTarget(capernian->GetGUID());
            return Attack(capernian);
        }

        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Already targeting Capernian targetGuid={}", bot->GetTarget().ToString());
        return false;
    }
    else
    {
        if (capernian && !botAI->IsRangedDps(bot))
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Capernian reason=bot_not_ranged");
    }

    // Target priority 3: Sanguinar for all dps
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: sanguinar not found");
    }
    else
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: sanguinar found entry={} guid={} alive={} non_attackable={}", sanguinar->GetEntry(), sanguinar->GetGUID().ToString(), sanguinar->IsAlive(), sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE));
    }

    if (sanguinar && sanguinar->IsAlive())
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Selecting Sanguinar (star) for bot={}", bot->GetName());
        SetRtiTarget(botAI, "star", sanguinar);

        if (bot->GetTarget() != sanguinar->GetGUID())
        {
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Setting target to Sanguinar botTarget={} sanguinarGuid={}", bot->GetTarget().ToString(), sanguinar->GetGUID().ToString());
            bot->SetTarget(sanguinar->GetGUID());
            return Attack(sanguinar);
        }

        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Already targeting Sanguinar targetGuid={}", bot->GetTarget().ToString());
        return false;
    }
    else
    {
        if (sanguinar && !sanguinar->IsAlive())
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Sanguinar reason=not_alive");
    }

    // Target priority 4: Telonicus for all dps
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: telonicus not found");
    }
    else
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: telonicus found entry={} guid={} alive={} non_attackable={} victimGuid={}", telonicus->GetEntry(), telonicus->GetGUID().ToString(), telonicus->IsAlive(), telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE), telonicus->GetVictim() ? telonicus->GetVictim()->GetGUID().ToString() : std::string("none"));
    }

    if (telonicus && telonicus->IsAlive())
    {
        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Selecting Telonicus (triangle) for bot={}", bot->GetName());
        SetRtiTarget(botAI, "triangle", telonicus);

        if (bot->GetTarget() != telonicus->GetGUID())
        {
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Setting target to Telonicus botTarget={} telonicusGuid={}", bot->GetTarget().ToString(), telonicus->GetGUID().ToString());
            bot->SetTarget(telonicus->GetGUID());
            return Attack(telonicus);
        }

        LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Already targeting Telonicus targetGuid={}", bot->GetTarget().ToString());

        // Melee DPS positioning: stay at max-ish melee range behind Telonicus (bomb safety)
        if (botAI->IsMelee(bot) && !botAI->IsTank(bot) && telonicus->GetVictim() != bot)
        {
            float maxMeleeRange = bot->GetMeleeRange(telonicus);
            float behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);

            float targetX = telonicus->GetPositionX() + (maxMeleeRange - 0.5f) * cos(behindAngle);
            float targetY = telonicus->GetPositionY() + (maxMeleeRange - 0.5f) * sin(behindAngle);

            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Melee positioning behind Telonicus maxRange={} target=({}, {}) botPos=({}, {})", maxMeleeRange, targetX, targetY, bot->GetPositionX(), bot->GetPositionY());

            if (bot->GetExactDist2d(targetX, targetY) > 0.5f)
            {
                LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Moving to melee safe spot behind Telonicus");
                return MoveTo(telonicus->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
            else
            {
                LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Already at melee safe spot behind Telonicus");
            }
        }
    }
    else
    {
        if (telonicus && !telonicus->IsAlive())
            LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Skipping Telonicus reason=not_alive");
    }

    LOG_DEBUG("playerbots", "AssignAdvisorDpsPriorityAction: Exit no action taken bot={}", bot->GetName());
    return false;
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event event)
{
    // Get all flame strike triggers
    std::vector<Unit*> flameStrikes = GetAllFlameStrikeTriggers(botAI, bot);
    if (flameStrikes.empty())
        return false;

    // Check if bot is in danger
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

    // Find safe position with minimal movement
    Position safestPos = FindSafestNearbyPosition(flameStrikes, hazardRadius);

    // Move to safe position
    return MoveTo(bot->GetMapId(), safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position KaelthasSunstriderAvoidFlameStrikeAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& flameStrikes, float hazardRadius)
{
    const float searchStep = M_PI / 8.0f; // 22.5 degree increments
    const float minDistance = 2.0f;
    const float maxDistance = 30.0f;
    const float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = 1000.0f;
    bool foundSafe = false;

    // Search in expanding circles for closest safe spot
    for (float distance = minDistance; distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * cos(angle);
            float y = bot->GetPositionY() + distance * sin(angle);
            float z = bot->GetPositionZ();

            Position testPos(x, y, z);

            // Check if position is safe from all flame strikes
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

                // Update best position if this is better
                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                // If no safe path found yet, accept any safe destination
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        // If we found a safe position with safe path at this distance, return it (minimal movement)
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

    // Check points along the path
    for (int i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        // Check if this point is too close to any flame strike
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
    // Handle phoenix tanking for assist tanks
    if (botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1))
    {
        // Get all phoenixes
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

        // Sort phoenixes by GUID for consistent targeting
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

        // Attack the assigned phoenix
        if (bot->GetTarget() != targetPhoenix->GetGUID())
        {
            bot->SetTarget(targetPhoenix->GetGUID());
            return Attack(targetPhoenix);
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
                    return MoveAway(nearestPlayer, safeDistance - closestDist + 2.0f);
                }
            }
        }

        return false;
    }

    if (botAI->IsRangedDps(bot))
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

    // Find nearest mind-controlled player
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

    LOG_DEBUG("playerbots", "KaelthasSunstriderBreakMindControlWithInfinityBladeAction: {} targeting MC'd player {} at {:.1f} yards",
              bot->GetName(), mcTarget->GetName(), closestDist);

    // Move to melee range if not in range
    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        LOG_DEBUG("playerbots", "KaelthasSunstriderBreakMindControlWithInfinityBladeAction: {} moving to melee range of {}",
                  bot->GetName(), mcTarget->GetName());
        return MoveTo(bot->GetMapId(), mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Try to cast MC-breaking abilities
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
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderBreakMindControlWithInfinityBladeAction: {} casting {} on {}",
                      bot->GetName(), spell, mcTarget->GetName());
            return botAI->CastSpell(spell, mcTarget);
        }
    }

    LOG_DEBUG("playerbots", "KaelthasSunstriderBreakMindControlWithInfinityBladeAction: {} in range but no spells available",
              bot->GetName());
    return false;
}

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

    const float minSpreadDistance = 10.0f;

    // Find all nearby raid members in 3D space
    std::vector<Player*> nearbyPlayers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // Check 3D distance using IsWithinDist3d
        if (bot->IsWithinDist3d(member, minSpreadDistance * 2.0f))
            nearbyPlayers.push_back(member);
    }

    if (nearbyPlayers.empty())
        return false;

    // Find the closest player in 3D
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

    // If we're too close to someone, move away in 2D plane
    if (closestPlayer && closestDist < minSpreadDistance)
    {
        LOG_DEBUG("playerbots", "KaelthasSunstriderSpreadOutInMidairAction: {} spreading from {} (3D dist: {:.1f})",
                  bot->GetName(), closestPlayer->GetName(), closestDist);

        // Calculate 2D direction away from closest player
        float angle = bot->GetAngle(closestPlayer) + M_PI; // Opposite direction
        float distance = minSpreadDistance - closestDist + 2.0f; // Extra buffer

        float x = bot->GetPositionX() + cos(angle) * distance;
        float y = bot->GetPositionY() + sin(angle) * distance;
        float z = bot->GetPositionZ(); // Keep current Z, let physics handle it

        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}
