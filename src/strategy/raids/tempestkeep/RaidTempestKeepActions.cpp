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

        if (mainTank->GetExactDist2d(mtTarget.x, mtTarget.y) >= 10.0f)
        {
            if (mainTank->GetPositionZ() < 13.0f)
                return MoveTo(bot->GetMapId(), mtTarget.x, mtTarget.y, mtTarget.z, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED, true, false);
            else
                return JumpTo(mainTank->GetMapId(), mtTarget.x, mtTarget.y, mtTarget.z, MovementPriority::MOVEMENT_FORCED);
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
    if (assistTank && alar && alar->GetPositionZ() < 42.0f)
    {
        Location atTarget;
        switch (alarPlatform)
        {
            case 0: // Alar at Platform 1
            case 1: // Alar at Platform 2
                atTarget = platforms[1]; // Platform 2
                break;
            case 2: // Alar at Platform 3
            case 3: // Alar at Platform 4
                atTarget = platforms[3]; // Platform 4
                break;
            default:
                return false;
        }

        if (assistTank->GetExactDist2d(atTarget.x, atTarget.y) >= 10.0f)
        {
            if (assistTank->GetPositionZ() < 13.0f)
                return MoveTo(bot->GetMapId(), atTarget.x, atTarget.y, atTarget.z, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED, true, false);
            else
                return JumpTo(assistTank->GetMapId(), atTarget.x, atTarget.y, atTarget.z, MovementPriority::MOVEMENT_FORCED);
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

bool AlarMeleeDpsFocusOnBossAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    MarkTargetWithStar(bot, alar);
    SetRtiTarget(botAI, "star", alar);

    if (alar->GetPositionZ() >= 17.0f && alar->GetPositionZ() < 22.0f && 
        !(alar->HasAura(SPELL_DIVE_BOMB) || alar->HasAura(SPELL_DIVE_BOMB_VISUAL)))
    {
        uint32 mapId = alar->GetMapId();
        int8 alarPlatform = lastAlarPlatform[mapId];

        std::vector<Location> platforms = { AlarPlatform1, AlarPlatform2, AlarPlatform3, AlarPlatform4 };
        const Location& currentPlatform = platforms[alarPlatform];

        if (bot->GetExactDist2d(currentPlatform.x, currentPlatform.y) > 5.0f)
        {
            /* if (alarPlatform == 2) // Platform 3; for some reason bots will not move from Platform 2 to Platform 3
                return JumpTo(mapId, currentPlatform.x, currentPlatform.y, currentPlatform.z, MovementPriority::MOVEMENT_FORCED);
            else */
                return MoveNear(currentPlatform.x, currentPlatform.y, currentPlatform.z, 5.0f, MovementPriority::MOVEMENT_FORCED);
        }
    }

    if (bot->GetVictim() != alar)
        return Attack(alar);

    return false;
}

bool AlarRangedDpsPrioritizeAddsAction::Execute(Event event)
{
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember && ember->IsAlive())
    {
        SetRtiTarget(botAI, "square", ember);

        float emberDist = bot->GetExactDist2d(ember->GetPositionX(), ember->GetPositionY());
        if (emberDist < 18.0f && ember->GetHealthPct() < 50.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, 20.0f, false);
        }

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

    // Stay within 45 yards of the ranged center
    const Location& center = AlarRangedCenter;
    float dist = bot->GetExactDist2d(center.x, center.y);

    if (dist > 45.0f)
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

bool AlarPositionHealerAction::Execute(Event event)
{
    // Get the nearest Ember of Al'ar
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (ember)
    {
        float emberDist = bot->GetExactDist2d(ember->GetPositionX(), ember->GetPositionY());
        if (emberDist < 18.0f && ember->GetHealthPct() < 50.0f)
        {
            // Move away from Ember if too close
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(ember, 20.0f, false);
        }
    }

    // Stay within 45 yards of the ranged center
    const Location& center = AlarRangedCenter;
    float dist = bot->GetExactDist2d(center.x, center.y);

    if (dist > 45.0f)
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
    Unit* ember = GetFirstAliveUnitByEntry(botAI, NPC_EMBER_OF_ALAR);
    if (!ember)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return true;
    }
    else
    {
        MarkTargetWithSquare(bot, ember);
        SetRtiTarget(botAI, "square", ember);

        if (ember->GetVictim() == bot && ember->GetHealthPct() < 20.0f)
        {
            const float minDistance = 18.0f;
            Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
            if (nearestPlayer)
                return MoveAway(nearestPlayer, 20.0f, false);
        }

        if (bot->GetVictim() != ember)
            return Attack(ember);
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

    const Location& rampBaseSW = AlarSWRampBase;
    const Location& rampBaseSE = AlarSERampBase;
    if (botAI->IsMainTank(bot))
    {
        // MT heads to SW ramp
        return MoveTo(bot->GetMapId(), rampBaseSW.x, rampBaseSW.y, rampBaseSW.z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        // AT0 heads to SE ramp
        return MoveTo(bot->GetMapId(), rampBaseSE.x, rampBaseSE.y, rampBaseSE.z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        const Location& target = AlarRoomSouthCenter;
        return MoveTo(bot->GetMapId(), target.x, target.y, target.z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AlarMoveAwayFromRebirthAction::Execute(Event event)
{
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
    if (alar->GetHealthPct() > 99.5f)
        lastAlarPlatform[mapId] = -1;

    // Manual override: if Flame Quills is active, set lastAlarPlatform to platform 4 (index 3)
    if (alar->GetPositionZ() > 22.0f && alar->GetHealthPct() < 95.0f)
        lastAlarPlatform[mapId] = 3;

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
            float stepSize = std::min(4.5f, distanceToTankPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * stepSize;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * stepSize;
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
        const uint8 botsPerRing = 8;
        const float offsetArc = 1.0f;

        // Assign healer positions first
        for (size_t i = 0; i < healers.size(); ++i)
        {
            Player* healer = healers[i];
            Position pos = GetRangedBotPosition(tankPosition, radius, healers.size(), 0.0f, i, healer->GetPositionZ());
            initialVoidReaverPositions[healer->GetGUID()] = pos;
            hasReachedInitialVoidReaverPosition[healer->GetGUID()] = false;
            LOG_DEBUG("playerbots", "VoidReaverSpreadRangedAction: Healer {} assigned initial position ({}, {}, {})", 
                healer->GetName(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        }

        // Assign ranged DPS positions next, offset from healers
        for (size_t i = 0; i < rangedDps.size(); ++i)
        {
            Player* dps = rangedDps[i];
            Position pos = GetRangedBotPosition(tankPosition, radius, rangedDps.size(), offsetArc, i, dps->GetPositionZ());
            initialVoidReaverPositions[dps->GetGUID()] = pos;
            hasReachedInitialVoidReaverPosition[dps->GetGUID()] = false;
            LOG_DEBUG("playerbots", "VoidReaverSpreadRangedAction: DPS {} assigned initial position ({}, {}, {})", 
                dps->GetName(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
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
    if (botAI->IsRanged(bot) && bot->GetExactDist2d(position.x, position.y) > 4.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Phase 2 - Melee move to Ranged
    Unit* solariumAgent = AI_VALUE2(Unit*, "find target", "solarium agent");
    if (solariumAgent && botAI->IsMelee(bot) &&
        bot->GetExactDist2d(position.x, position.y) > 6.0f)
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

    // Phase 2: If any agent is present, all bots stack on first alive group member
    if (solariumAgent)
    {
        Player* stackTarget = nullptr;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive())
                {
                    stackTarget = member;
                    break;
                }
            }
        }

        if (stackTarget && bot != stackTarget && bot->GetExactDist2d(stackTarget) > 5.0f)
        {;
            return MoveTo(bot->GetMapId(), stackTarget->GetPositionX(), stackTarget->GetPositionY(), stackTarget->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        return false;
    }

    // Phase 1: All ranged stack 25 yards from boss (e.g., north)
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (astromancer && botAI->IsRanged(bot))
    {
        const float stackDistance = 25.0f;
        const float stackAngle = 5.0f * M_PI / 4.0f; // Southwest direction
        float stackX = astromancer->GetPositionX() + stackDistance * cos(stackAngle);
        float stackY = astromancer->GetPositionY() + stackDistance * sin(stackAngle);
        float stackZ = astromancer->GetPositionZ();

        if (bot->GetExactDist2d(stackX, stackY) > 3.0f)
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
