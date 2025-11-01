#include <ctime>
#include <sstream>

#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;
using namespace TempestKeepLocations;

// Al'ar <Phoenix God>

// To-Do:
// Tanks need to taunt Al'ar at their positions
// Ranged need to stay within a radius of the room center
// How to end flame quill action? Is it necessary? Flying to quills and at next platform is 20s
// Build out state tracker for rebirth to cleanly split off phase 1 logic
// Bots other than tanks need to move away from embers before they blow up
// Need to avoid Al'ar rebirth explosion
// Phase 2 melt armor taunt logic
// Phase 2: spread for dive bomb, anything else?

// Multipliers:
// No Tank Assist for MT or AT0 during phase 1 (keep for other ATs)
// For anybody doing the flame quill jump, everything else should be disabled

bool AlarLogDebugInfoAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    std::ostringstream auraList;
    for (auto const& pair : alar->GetAppliedAuras())
    {
        if (pair.second && pair.second->GetBase())
            auraList << pair.second->GetBase()->GetId() << " ";
    }

    std::time_t now = std::time(nullptr);
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    LOG_DEBUG("playerbots", "Time: {} | Al'ar coords: ({}, {}, {}) | Health: {} ({:.1f}%) | Auras: {}",
        timeStr, alar->GetPositionX(), alar->GetPositionY(), alar->GetPositionZ(),
        alar->GetHealth(), alar->GetHealthPct(), auraList.str());

    return false;
}

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

    uint32 mapId = alar->GetMapId();

    // List of platforms
    std::vector<Location> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };

    // Update last platform snapshot if needed
    UpdateAlarLastPlatform(alar, mapId, platforms);
    int8 alarPlatform = lastAlarPlatform[mapId];

    SetRtiTarget(botAI, "star", alar);
    bool mtAction = PositionMainTank(botAI->IsMainTank(bot) ? bot : nullptr, alar, alarPlatform, platforms);
    bool atAction = PositionAssistTank(botAI->IsAssistTankOfIndex(bot, 0) ? bot : nullptr, alar, alarPlatform, platforms);

    return mtAction || atAction;
}

/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Location>& platforms)
{
    if (mainTank && alar && alar->GetPositionZ() < 42.0f)
    {
        Location mtTarget;
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

        if (mainTank->GetExactDist2d(mtTarget.x, mtTarget.y) > 10.0f && mainTank->GetPositionZ() >= 17.0f)
        {
            // Teleport directly to platform
            uint32 mapId = alar->GetMapId();
            bot->TeleportTo(mapId, mtTarget.x, mtTarget.y, mtTarget.z, mainTank->GetOrientation());
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
bool AlarBossTanksMoveBetweenPlatformsAction::PositionMainTank(Player* mainTank, Unit* alar, int8 alarPlatform, const std::vector<Location>& platforms)
{
    if (!mainTank || !alar)
        return false;

    if (mainTank->GetPositionZ() < 13.0f)
        return false;

    // Main tank only ever moves between platform 1 and 3
    Location mtTarget;
    std::vector<Location> midpoints;

    int8 fromPlatform = lastMainTankPlatform[mainTank->GetGUID()];

    // Determine target and midpoints based on Al'ar's platform
    switch (alarPlatform)
    {
        case 1: // Al'ar at platform 2
            mtTarget = platforms[2]; // Move to platform 3
            midpoints = std::vector<Location>(midpoints_1_to_6.begin(), midpoints_1_to_6.begin() + 4); // Forward
            break;
        case 2: // Al'ar at platform 3
            mtTarget = platforms[2]; // Stay at platform 3
            midpoints.clear(); // No movement
            break;
        case 3: // Al'ar at platform 4
            mtTarget = platforms[0]; // Move back to platform 1
            midpoints = std::vector<Location>(midpoints_6_to_1.begin(), midpoints_6_to_1.begin() + 4); // Backward
            break;
        case 0: // Al'ar at platform 1
        default:
            mtTarget = platforms[0]; // Stay at platform 1
            midpoints.clear(); // No movement
            break;
    }

    // If already at target, update state and attack
    if (mainTank->GetExactDist2d(mtTarget.x, mtTarget.y) < 2.0f)
    {
        lastMainTankPlatform[mainTank->GetGUID()] = (mtTarget.x == platforms[0].x) ? 0 : 2;
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
                const Location& wp = midpoints[i];
                if (mainTank->GetExactDist2d(wp.x, wp.y) >= 2.0f)
                {
                    return MoveTo(bot->GetMapId(), wp.x, wp.y, wp.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    mtBalconyMidpointVisited[mainTank->GetGUID()][i] = true;
                break;
            }
        }
    }

    // After all midpoints are visited, move to final target
    if (mainTank->GetExactDist2d(mtTarget.x, mtTarget.y) >= 2.0f)
    {
        return MoveTo(bot->GetMapId(), mtTarget.x, mtTarget.y, mtTarget.z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

/* bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Location>& platforms)
{
    if (assistTank && alar && alar->GetPositionZ() < 42.0f)
    {
        Location atTarget;
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

        if (assistTank->GetExactDist2d(atTarget.x, atTarget.y) > 10.0f && assistTank->GetPositionZ() >= 17.0f)
        {
            uint32 mapId = alar->GetMapId();
            bot->TeleportTo(mapId, atTarget.x, atTarget.y, atTarget.z, assistTank->GetOrientation());
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
bool AlarBossTanksMoveBetweenPlatformsAction::PositionAssistTank(Player* assistTank, Unit* alar, int8 alarPlatform, const std::vector<Location>& platforms)
{
    if (assistTank && alar)
    {
        int8 fromPlatform = lastAssistTankPlatform[assistTank->GetGUID()];
        int8 toPlatform = alarPlatform;

        // Special case: if last platform was 2 and Al'ar HP >= 70%, move directly to platform 2
        if (fromPlatform == 1 && alar->GetHealthPct() >= 70.0f)
        {
            const Location& directTarget = platforms[1]; // Platform 2
            if (assistTank->GetExactDist2d(directTarget.x, directTarget.y) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), directTarget.x, directTarget.y, directTarget.z, false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }
            else
            {
                lastAssistTankPlatform[assistTank->GetGUID()] = toPlatform;
                // Optionally clear visited waypoints if needed
                atBalconyMidpointVisited[assistTank->GetGUID()].clear();
            }
        }

        if (assistTank->GetPositionZ() >= 13.0f)
        {
            Location atTarget;
            std::vector<Location> midpoints;
            bool movingForward = false;

            switch (alarPlatform)
            {
                case 0: // Platform 1
                case 1: // Platform 2
                    atTarget = platforms[1]; // Platform 2
                    break;
                case 2: // Platform 3
                case 3: // Platform 4
                    atTarget = platforms[3]; // Platform 4
                    break;
                default:
                    return false;
            }

            if (assistTank->GetExactDist2d(atTarget.x, atTarget.y) < 2.0f)
            {
                lastAssistTankPlatform[assistTank->GetGUID()] = toPlatform;
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
                return false;
            }

            // Select midpoints based on direction
            if (fromPlatform < toPlatform)
            {
                // Forward: use waypoints 3 through 6 (indices 2–5)
                midpoints = std::vector<Location>(midpoints_1_to_6.begin() + 2, midpoints_1_to_6.end());
                movingForward = true;
            }
            else if (fromPlatform > toPlatform)
            {
                // Backward: use waypoints 6 through 3 (indices 2–5, reversed)
                midpoints = std::vector<Location>(midpoints_6_to_1.begin() + 2, midpoints_6_to_1.end());
                movingForward = false;
            }

            // Initialize visited vector if empty
            if (atBalconyMidpointVisited[assistTank->GetGUID()].size() != midpoints.size())
                atBalconyMidpointVisited[assistTank->GetGUID()] = std::vector<bool>(midpoints.size(), false);

            // Move through midpoints
            for (size_t i = 0; i < midpoints.size(); ++i)
            {
                if (!atBalconyMidpointVisited[assistTank->GetGUID()][i])
                {
                    const Location& wp = midpoints[i];
                    if (assistTank->GetExactDist2d(wp.x, wp.y) >= 2.0f)
                    {
                        return MoveTo(bot->GetMapId(), wp.x, wp.y, wp.z, false, false, false, true,
                                    MovementPriority::MOVEMENT_FORCED, true, false);
                    }
                    else
                        atBalconyMidpointVisited[assistTank->GetGUID()][i] = true;
                    break;
                }
            }

            // After all midpoints are visited, move to final target
            if (assistTank->GetExactDist2d(atTarget.x, atTarget.y) >= 2.0f)
            {
                return MoveTo(bot->GetMapId(), atTarget.x, atTarget.y, atTarget.z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED, true, false);
            }

            if (assistTank->GetExactDist2d(atTarget.x, atTarget.y) < 2.0f)
            {
                lastAssistTankPlatform[assistTank->GetGUID()] = toPlatform;
            }
        }
    }

    return false;
}

bool AlarMeleeDpsFocusOnBossAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    MarkTargetWithStar(bot, alar);
    SetRtiTarget(botAI, "star", alar);

    if (bot->GetVictim() != alar)
        return Attack(alar);

    uint32 mapId = alar->GetMapId();
    int8 alarPlatform = lastAlarPlatform[mapId];
    int8 fromPlatform = lastMeleeTargetPlatform[bot->GetGUID()];

    auto& visited = meleeMidpointVisited[bot->GetGUID()];
    if (visited.size() < 9)
        visited.resize(9, false);

    // Ramp logic if Z < -2.0f
    if (bot->GetPositionZ() < -2.0f && alar->GetPositionZ() < 30.0f && !isPhase2[mapId])
    {
        if (alarPlatform == 3) // Al'ar at platform 4
        {
            // Move to SE ramp base, then directly to platform 4 (skip midpoints)
            if (!meleeMidpointVisited[bot->GetGUID()][7])
            {
                if (bot->GetExactDist2d(AlarSERampBase.x, AlarSERampBase.y) >= 2.0f)
                {
                    return MoveTo(mapId, AlarSERampBase.x, AlarSERampBase.y, AlarSERampBase.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    meleeMidpointVisited[bot->GetGUID()][7] = true;
            }
            // After ramp base, move directly to platform 4
            if (bot->GetExactDist2d(AlarMelee4.x, AlarMelee4.y) >= 2.0f)
            {
                return MoveTo(mapId, AlarMelee4.x, AlarMelee4.y, AlarMelee4.z, false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED, true, false);
            }

            if (bot->GetExactDist2d(AlarMelee4.x, AlarMelee4.y) < 2.0f)
            {
                meleeMidpointVisited[bot->GetGUID()].clear();
                return false;
            }
        }
        else if (alarPlatform == 0) // Al'ar at platform 1
        {
            if (!meleeMidpointVisited[bot->GetGUID()][8]) // Unique index for SW ramp
            {
                if (bot->GetExactDist2d(AlarSWRampBase.x, AlarSWRampBase.y) >= 2.0f)
                {
                    return MoveTo(mapId, AlarSWRampBase.x, AlarSWRampBase.y, AlarSWRampBase.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    meleeMidpointVisited[bot->GetGUID()][8] = true;

                if (bot->GetExactDist2d(AlarSWRampBase.x, AlarSWRampBase.y) < 2.0f && alarPlatform == 0)
                {
                    meleeMidpointVisited[bot->GetGUID()].clear();
                    return false;
                }
            }
        }
    }

    if (bot->GetPositionZ() >= 17.0f && !isPhase2[mapId])
    {
        std::vector<Location> meleePositions = { AlarMelee1, AlarMelee2, AlarMelee3, AlarMelee4 };
        const Location& meleeTarget = meleePositions[alarPlatform];

        // If arrived, clear state
        if (bot->GetExactDist2d(meleeTarget.x, meleeTarget.y) < 2.0f)
        {
            lastMeleeTargetPlatform[bot->GetGUID()] = alarPlatform;
            meleeMidpointVisited[bot->GetGUID()].clear();
            return false;
        }

        // Only reset visited vector if target platform changed
        if (lastMeleeTargetPlatform[bot->GetGUID()] != alarPlatform)
        {
            lastMeleeTargetPlatform[bot->GetGUID()] = alarPlatform;
            meleeMidpointVisited[bot->GetGUID()].clear();
        }

        // Determine which midpoints to use based on movement
        std::vector<Location> midpointsNeeded;
        if (alarPlatform == 1)
            midpointsNeeded = std::vector<Location>(midpoints_1_to_6.begin(), midpoints_1_to_6.begin() + 2);
        else if (alarPlatform == 2)
            midpointsNeeded = std::vector<Location>(midpoints_1_to_6.begin() + 2, midpoints_1_to_6.begin() + 4);
        else if (alarPlatform == 3)
            midpointsNeeded = std::vector<Location>(midpoints_1_to_6.begin() + 4, midpoints_1_to_6.begin() + 6);

        if (meleeMidpointVisited[bot->GetGUID()].size() != midpointsNeeded.size())
            meleeMidpointVisited[bot->GetGUID()] = std::vector<bool>(midpointsNeeded.size(), false);

        // Move through needed midpoints
        for (size_t i = 0; i < midpointsNeeded.size(); ++i)
        {
            if (!meleeMidpointVisited[bot->GetGUID()][i])
            {
                const Location& wp = midpointsNeeded[i];
                if (bot->GetExactDist2d(wp.x, wp.y) >= 2.0f)
                {
                    return MoveTo(mapId, wp.x, wp.y, wp.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    meleeMidpointVisited[bot->GetGUID()][i] = true;
                break;
            }
        }

        // After all midpoints are visited, move to melee target
        if (bot->GetExactDist2d(meleeTarget.x, meleeTarget.y) >= 2.0f)
        {
            return MoveTo(mapId, meleeTarget.x, meleeTarget.y, meleeTarget.z, false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool AlarRangedDpsPrioritizeAddsAction::Execute(Event event)
{
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember && ember->IsAlive())
    {
        SetRtiTarget(botAI, "square", ember);

        if (bot->GetTarget() != ember->GetGUID())
        {
            bot->SetSelection(ember->GetGUID());
            return Attack(ember);
        }
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (alar && (!ember || !ember->IsAlive()))
    {
        SetRtiTarget(botAI, "star", alar);

        if (bot->GetTarget() != alar->GetGUID())
        {
            bot->SetSelection(alar->GetGUID());
            return Attack(alar);
        }
    }

    uint32 mapId = alar->GetMapId();
    if (!isPhase2[mapId])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];

        // List of ground locations matching platforms
        std::vector<Location> groundLocations = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Location& groundTarget = groundLocations[alarPlatform];

        // Only move if not already within 10 yards
        if (bot->GetExactDist2d(groundTarget.x, groundTarget.y) > 10.0f)
        {
            // Move near the ground target (e.g., 5 yards away for some spread)
            return MoveNear(bot->GetMapId(), groundTarget.x, groundTarget.y, groundTarget.z, 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Stay within 45 yards of the ranged center
    const Location& center = AlarRangedCenter;
    float dist = bot->GetExactDist2d(center.x, center.y);
    if (isPhase2[mapId] && dist > 45.0f)
    {
        // Calculate direction vector from center to bot
        float dx = bot->GetPositionX() - center.x;
        float dy = bot->GetPositionY() - center.y;
        float scale = 40.0f / dist; // scale to 40 yards

        // New target position at 40 yards from center, in the direction of the bot
        float targetX = center.x + dx * scale;
        float targetY = center.y + dy * scale;

        return MoveNear(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), 5.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AlarPositionHealerAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();
    if (!isPhase2[mapId])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];

        // List of ground locations matching platforms
        std::vector<Location> groundLocations = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Location& groundTarget = groundLocations[alarPlatform];

        // Only move if not already within 10 yards
        if (bot->GetExactDist2d(groundTarget.x, groundTarget.y) > 10.0f)
        {
            // Move near the ground target (e.g., 5 yards away for some spread)
            return MoveNear(bot->GetMapId(), groundTarget.x, groundTarget.y, groundTarget.z, 5.0f, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Stay within 45 yards of the ranged center
    const Location& center = AlarRangedCenter;
    float dist = bot->GetExactDist2d(center.x, center.y);
    if (isPhase2[mapId] && dist > 45.0f)
    {
        // Calculate direction vector from center to bot
        float dx = bot->GetPositionX() - center.x;
        float dy = bot->GetPositionY() - center.y;
        float scale = 40.0f / dist; // scale to 40 yards

        // New target position at 40 yards from center, in the direction of the bot
        float targetX = center.x + dx * scale;
        float targetY = center.y + dy * scale;

        return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool AlarAddTankPickUpEmbersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        const Location& center = AlarRangedCenter;
        if (ember->GetVictim() == bot && bot->GetExactDist2d(center.x, center.y) > 5.0f)
        {
            float dX = center.x - bot->GetPositionX();
            float dY = center.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetVictim() != ember)
            return Attack(ember);
    }
    else if (!isPhase2[mapId])
    {
        int8 alarPlatform = lastAlarPlatform[mapId];

        // List of ground locations matching platforms
        std::vector<Location> groundLocations = { AlarGround1, AlarGround2, AlarGround3, AlarGround4 };
        const Location& groundTarget = groundLocations[alarPlatform];

        // Only move if not already within 30 yards
        if (bot->GetExactDist2d(groundTarget.x, groundTarget.y) > 30.0f)
        {
            // Move near the ground target (e.g., 5 yards away for some spread)
            return MoveNear(bot->GetMapId(), groundTarget.x, groundTarget.y, groundTarget.z, 5.0f, MovementPriority::MOVEMENT_COMBAT);
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
        std::vector<std::pair<Location, Location>> platformGroundPairs =
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
            float dist = bot->GetExactDist2d(platformGroundPairs[i].first.x, platformGroundPairs[i].first.y);
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }

        // Jump to corresponding landing location
        const Location& ground = platformGroundPairs[nearestIndex].second;
        return JumpTo(bot->GetMapId(), ground.x, ground.y, ground.z, MovementPriority::MOVEMENT_FORCED);
    }

    const Location& atMidpoint = AlarMidpointToSERamp;
    const Location& atTarget = AlarSERampBase;
    const Location& mtMidpoint = AlarMidpointToSWRamp;
    const Location& mtTarget = AlarSWRampBase;
    const Location& meleeMidpoint = AlarRoomSouthCenter;
    if (botAI->IsMainTank(bot) && bot->GetPositionZ() < -2.0f)
    {
        // Only use midpoint if landing at groundposition3
        if (bot->GetExactDist2d(AlarGround3.x, AlarGround3.y) < 5.0f)
        {
            if (!mtGroundMidpointVisited[bot->GetGUID()])
            {
                if (bot->GetExactDist2d(mtMidpoint.x, mtMidpoint.y) >= 1.0f)
                {
                    return MoveTo(bot->GetMapId(), mtMidpoint.x, mtMidpoint.y, mtMidpoint.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    mtGroundMidpointVisited[bot->GetGUID()] = true;
            }
        }
        // MT heads to SW ramp
        return MoveTo(bot->GetMapId(), mtTarget.x, mtTarget.y, mtTarget.z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0) && bot->GetPositionZ() < -2.0f)
    {
        // Only use midpoint if landing at groundposition2
        if (bot->GetExactDist2d(AlarGround2.x, AlarGround2.y) < 5.0f)
        {
            if (!atGroundMidpointVisited[bot->GetGUID()])
            {
                if (bot->GetExactDist2d(atMidpoint.x, atMidpoint.y) >= 1.0f)
                {
                    return MoveTo(bot->GetMapId(), atMidpoint.x, atMidpoint.y, atMidpoint.z, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED, true, false);
                }
                else
                    atGroundMidpointVisited[bot->GetGUID()] = true;
            }
        }
        // AT0 heads to SE ramp
        return MoveTo(bot->GetMapId(), atTarget.x, atTarget.y, atTarget.z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsMelee(bot) && botAI->IsDps(bot) && bot->GetPositionZ() < -2.0f && alar->GetPositionZ() >= 30.0f)
    {
        return MoveTo(bot->GetMapId(), meleeMidpoint.x, meleeMidpoint.y, meleeMidpoint.z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
    if (bot->GetPositionZ() >= 17.0f)
    {
        // List of jump and landing pairs
        std::vector<std::pair<Location, Location>> platformGroundPairs =
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
            float dist = bot->GetExactDist2d(platformGroundPairs[i].first.x, platformGroundPairs[i].first.y);
            if (dist < minDist)
            {
                minDist = dist;
                nearestIndex = i;
            }
        }

        // Jump to corresponding landing location
        const Location& ground = platformGroundPairs[nearestIndex].second;
        return JumpTo(bot->GetMapId(), ground.x, ground.y, ground.z, MovementPriority::MOVEMENT_FORCED);
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

    if (bot->GetVictim() != alar)
        return Attack(alar);

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

        const float dist = bot->GetExactDist2d(unit);
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

    uint32 mapId = alar->GetMapId();

    // Reset tracker at the start of the fight (Al'ar at max health)
    if (IsMapIDTimerManager(botAI, bot) && alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= 17.0f)
    {
        lastRebirthState[mapId] = false;
        lastAlarPlatform[mapId] = -1;
        isPhase2[mapId] = false;
        meleeMidpointVisited[bot->GetGUID()].clear();
    }

    if ((alar->GetHealthPct() > 99.5f && alar->GetPositionZ() >= 17.0f) ||
        alar->GetPositionZ() >= 22.0f)
    {
        if (botAI->IsMainTank(bot))
        {
            mtBalconyMidpointVisited[bot->GetGUID()].clear();
            mtGroundMidpointVisited[bot->GetGUID()] = false;
            lastMainTankPlatform[bot->GetGUID()] = 0; // Platform 1
        }
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            atBalconyMidpointVisited[bot->GetGUID()].clear();
            atGroundMidpointVisited[bot->GetGUID()] = false;
            lastAssistTankPlatform[bot->GetGUID()] = 1; // Platform 2
        }
        if (botAI->IsMelee(bot) && botAI->IsDps(bot))
        {
            meleeMidpointVisited[bot->GetGUID()].clear();
            lastMeleeTargetPlatform[bot->GetGUID()] = 0; // Platform 1
        }
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
        const Location& tankPosition = VoidReaverTankPosition;

        float dX = tankPosition.x - bot->GetPositionX();
        float dY = tankPosition.y - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.x, tankPosition.y);

        if (distanceToTankPosition > 2.0f)
        {
            float moveDist = std::min(4.5f, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * moveDist;
            const float moveZ = tankPosition.z;
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

        const Location& tankPosition = VoidReaverTankPosition;
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

            float targetX = tankPosition.x + ringRadius * cos(angle);
            float targetY = tankPosition.y + ringRadius * sin(angle);

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

Position VoidReaverSpreadRangedAction::GetRangedBotPosition(const TempestKeepHelpers::Location& center,
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

    float targetX = center.x + radius * cos(angle);
    float targetY = center.y + radius * sin(angle);

    return Position(targetX, targetY, botZ);
}

// High Astromancer Solarian

/* bool HighAstromancerSolarianStackBotsAction::Execute(Event event)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    const Location& position = AstromancerStackPosition;

    // Phase 1 & 2 - Ranged
    if (botAI->IsRanged(bot) && bot->GetExactDist2d(position.x, position.y) >= 4.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Phase 2 - Melee move to Ranged
    Unit* solariumAgent = AI_VALUE2(Unit*, "find target", "solarium agent");
    if (solariumAgent && botAI->IsMelee(bot) &&
        bot->GetExactDist2d(position.x, position.y) >= 6.0f)
    {
        bot->AttackStop();
        return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
} */

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
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();

            if (!member || !member->IsAlive() || member == bot)
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < 12.0f)
                return FleePosition(Position(member->GetPositionX(), member->GetPositionY(),
                                    member->GetPositionZ()), 15.0f, 0);
        }
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
    for (const auto& npcGuid : npcs)
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
