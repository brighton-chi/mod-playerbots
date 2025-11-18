#include <ctime>
#include <sstream>

#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "Playerbots.h"

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

/* bool AlarMeleeDpsPrioritizeAddsAction::Execute(Event event)
{
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember && ember->IsAlive())
    {
        if (ember->GetHealthPct() < 15.0f && bot->GetExactDist2d(ember) < 20.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, 17.0f, false);
        }

        SetRtiTarget(botAI, "square", ember);

        if (bot->GetVictim() != ember)
            return Attack(ember);
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    const uint32 mapId = alar->GetMapId();
    if (alar && isPhase2[mapId])
    {
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetVictim() != alar)
            return Attack(alar);
    }

    // Stay within 40 yards of the ranged center
    const Position& center = AlarRangedCenter;
    float dist = bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
    if (!isPhase2[mapId] && dist > 40.0f)
    {
        // Calculate direction vector from center to bot
        float dx = bot->GetPositionX() - center.GetPositionX();
        float dy = bot->GetPositionY() - center.GetPositionY();
        float scale = 35.0f / dist; // scale to 35 yards

        // New target position at 35 yards from center, in the direction of the bot
        float targetX = center.GetPositionX() + dx * scale;
        float targetY = center.GetPositionY() + dy * scale;

        return MoveNear(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}*/

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

    if (bot->GetExactDist2d(alar) < 20.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(alar, 25.0f, false);
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
    if (IsMapIDTimerManager(botAI, bot) && alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= 17.0f)
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

    if (bot->getClass() == CLASS_WARLOCK)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        if (botAI->CanCastSpell("soulshatter", bot))
            return botAI->CastSpell("soulshatter", bot);
    }
    else if (bot->getClass() == CLASS_MAGE)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        if (botAI->CanCastSpell("ice block", bot))
            return botAI->CastSpell("ice block", bot);
    }
    else if (bot->getClass() == CLASS_PRIEST)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        if (botAI->CanCastSpell("fade", bot))
            return botAI->CastSpell("fade", bot);
    }
    else if (bot->getClass() == CLASS_HUNTER)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        if (botAI->CanCastSpell("feign death", bot))
            return botAI->CastSpell("feign death", bot);
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        if (botAI->CanCastSpell("divine protection", bot))
            return botAI->CastSpell("divine protection", bot);
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

        float distance = bot->GetExactDist2d(member);
        if (distance < 10.0f)
            return MoveAway(member, 10.5f, false);
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
    Group* group = bot->GetGroup();
    if (!thaladred || !group)
        return false;

    const uint32 mapId = thaladred->GetMapId();

    // Reset phase tracker at phase 1 or 3 start (Thaladred at max health)
    if (thaladred->GetHealth() == thaladred->GetMaxHealth())
    {
        thaladredRelayPhase[mapId] = 0; // Start at phase 0
        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Relay point reached reset");
    }

    // Update phase based on Thaladred's position (10-yard buffer)
    const float phaseBuffer = 10.0f;
    uint8& relayPhase = thaladredRelayPhase[mapId];

    const Position& relayPoint1 = ThaladredRelayPoint1;
    const Position& relayPoint2 = ThaladredRelayPoint2;
    const Position& finalPosition = ThaladredFinalPosition;

    if (relayPhase == 0 && thaladred->GetExactDist2d(relayPoint1.GetPositionX(), relayPoint1.GetPositionY()) <= phaseBuffer)
    {
        relayPhase = 1;
        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Thaladred reached Relay Point 1, advancing from Relay Phase 0 to 1");
    }
    else if (relayPhase == 1 && thaladred->GetExactDist2d(relayPoint2.GetPositionX(), relayPoint2.GetPositionY()) <= phaseBuffer)
    {
        relayPhase = 2;
        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Thaladred reached Relay Point 2, advancing from Relay Phase 1 to 2");
    }
    else if (relayPhase == 2 && thaladred->GetExactDist2d(finalPosition.GetPositionX(), finalPosition.GetPositionY()) <= phaseBuffer)
    {
        relayPhase = 3;
        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Thaladred reached Final Position, advancing from Relay Phase 2 to 3");
    }

    // If bot is fixated, run to target position based on current phase
    if (thaladred->GetVictim() == bot)
    {
        Position targetPos = GetTargetPosition(relayPhase);

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);

        LOG_DEBUG("playerbots", "KaelthasSunstriderKiteThaladredAction: Bot {} fixated, moving to phase {} target ({}, {}, {})",
                    bot->GetName(), int(relayPhase), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());

        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

Position KaelthasSunstriderKiteThaladredAction::GetTargetPosition(uint8 relayPhase)
{
    // Return next relay point based on current phase
    switch (relayPhase)
    {
        case 0: return ThaladredRelayPoint1; // Phase 0: move to relay 1
        case 1: return ThaladredRelayPoint2; // Phase 1: move to relay 2
        case 2:
        case 3:
        default:
            return ThaladredFinalPosition;       // Phase 2+: move to death zone
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

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event event)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    MarkTargetWithCircle(bot, capernian);
    SetRtiTarget(botAI, "circle", capernian);

    if (bot->GetTarget() != capernian->GetGUID())
    {
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

    if (bot->GetExactDist2d(capernian) < 30.5f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(capernian, 31.0f, false);
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

    Creature* advisorCreature = advisorTarget ? advisorTarget->ToCreature() : nullptr;
    if (!advisorCreature || !advisorCreature->IsAlive() || advisorCreature->GetReactState() != REACT_AGGRESSIVE ||
        advisorCreature->HasAura(SPELL_PERMANENT_FEIGN_DEATH) || !tankTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", advisorCreature))
        return botAI->CastSpell("steady shot", advisorCreature);

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

        Creature* advisorCreature = advisor->ToCreature();
        // If any advisor is at 100% HP and aggressive, set timer
        if (advisorCreature->GetHealth() == advisorCreature->GetMaxHealth() &&
            advisorCreature->GetReactState() == REACT_AGGRESSIVE)
        {
            const uint32 mapId = advisorCreature->GetMapId();
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
    const Position& position = KaelthasWeaponStackPosition;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 5.0f)
    {
        return MoveTo(bot->GetMapId(), position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    Unit* staff = AI_VALUE2(Unit*, "find target", "staff of disintegration");
    if (staff && staff->HasUnitState(UNIT_STATE_CASTING) &&
        staff->FindCurrentSpellBySpellId(SPELL_STAFF_FROSTBOLT))
    {
        if (botAI->CanCastSpell("counterspell", staff))
            return botAI->CastSpell("counterspell", staff);
        else if (botAI->CanCastSpell("earth shock", staff))
            return botAI->CastSpell("earth shock", staff);
    }

    return false;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event event)
{
    Unit* devastation = AI_VALUE2(Unit*, "find target", "devastation");
    if (!devastation || !devastation->IsAlive())
        return false;

    MarkTargetWithStar(bot, devastation);
    SetRtiTarget(botAI, "star", devastation);

    if (bot->GetVictim() != devastation)
        return Attack(devastation);

    if (devastation->GetVictim() == bot)
    {
        const Position& position = KaelthasAxeTankPosition;
        if (!bot->IsWithinMeleeRange(devastation))
        {
            return MoveTo(devastation->GetMapId(), devastation->GetPositionX(),
                        devastation->GetPositionY(), devastation->GetPositionZ(),
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

bool KaelthasSunstriderHunterTurnAwayNetherstrandLongbowAction::Execute(Event event)
{
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    if (!longbow || !longbow->IsAlive())
        return false;

    MarkTargetWithCircle(bot, longbow);
    SetRtiTarget(botAI, "circle", longbow);

    if (bot->GetVictim() != longbow)
        return Attack(longbow);

    if (longbow->GetVictim() == bot)
    {
        const Position& position = KaelthasBowTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
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

bool KaelthasSunstriderCheatToTestAction::Execute(Event event)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    /* static std::unordered_map<uint32, std::chrono::steady_clock::time_point> kaelthasFightStart;
    uint32 mapId = vashj->GetMapId();
    auto now = std::chrono::steady_clock::now();
    if (kaelthas.find(mapId) == kaelthas.end())
    {
        kaelthas[mapId] = now;
        LOG_DEBUG("playerbots", "KaelthasSunstriderCheatToTestAction: Kael'thas fight START on map={} at steady_clock", mapId);
    }
    else
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - kaelthasFightStart[mapId]).count();
        LOG_DEBUG("playerbots", "KaelthasSunstriderCheatToTestAction: Kael'thas fight elapsed_ms={} on map={}", elapsed, mapId);
    } */

    bot->SetFullHealth();

    return false;
}
