#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;
using namespace TempestKeepLocations;

// Al'ar <Phoenix God>

/*
bool AlarPhase1PositionTanksAction::Execute(Event event)
{
    // Check if the event is a timer or boss movement event
    if (event.GetName() == "alar_platform_rotate")
    {
        // Rotate tanks to their next platform
        for (Player* tank : GetAlarTanks())
        {
            int currentIndex = tankPlatformIndex[tank->GetGUID()];
            int nextIndex = (currentIndex + 2) % 4; // Alternate between assigned platforms
            tankPlatformIndex[tank->GetGUID()] = nextIndex;

            Location target = alarPlatforms[nextIndex];
            if (tank->GetExactDist2d(target.x, target.y) > 2.0f)
            {
                MoveTo(tank->GetMapId(), target.x, target.y, target.z, false, false, false, false,
                       MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        return true;
    }

    // Otherwise, do nothing
    return false;
}
*/

/*
// Define platform positions (example coordinates)
std::vector<Location> alarPlatforms = {
    Location(x1, y1, z1), // Platform 1
    Location(x2, y2, z2), // Platform 2
    Location(x3, y3, z3), // Platform 3
    Location(x4, y4, z4)  // Platform 4
};

// Tank platform assignment
std::unordered_map<uint64, int> tankPlatformIndex; // bot GUID -> current platform index
std::unordered_map<uint64, time_t> lastPlatformSwitchTime; // bot GUID -> last switch time

void UpdateTankPlatform(Player* tank, int tankType) // tankType: 0 for main tank, 1 for assist tank
{
    uint64 guid = tank->GetGUID();
    time_t now = time(nullptr);

    // Initialize at start
    if (tankPlatformIndex.find(guid) == tankPlatformIndex.end())
    {
        tankPlatformIndex[guid] = tankType; // 0 or 1
        lastPlatformSwitchTime[guid] = now;
    }

    // Switch every 30 seconds
    if (now - lastPlatformSwitchTime[guid] >= 30)
    {
        // Main tank alternates between 0 and 2, assist between 1 and 3
        int current = tankPlatformIndex[guid];
        int next = (current + 2) % 4;
        tankPlatformIndex[guid] = next;
        lastPlatformSwitchTime[guid] = now;
    }

    // Move tank to assigned platform
    Location target = alarPlatforms[tankPlatformIndex[guid]];
    if (tank->GetExactDist2d(target.x, target.y) > 2.0f)
    {
        MoveTo(tank->GetMapId(), target.x, target.y, target.z, false, false, false, false,
               MovementPriority::MOVEMENT_COMBAT, true, false);
    }
} */

bool AlarJumpFromPlatformAction::Execute(Event event)
{
    // Only jump if bot is on a platform (Z > 15.0f)
    if (bot->GetPositionZ() > 15.0f)
    {
        // List of platform and ground pairs
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

        // Jump to corresponding ground location
        const Location& ground = platformGroundPairs[nearestIndex].second;
        return JumpTo(bot->GetMapId(), ground.x, ground.y, ground.z, MovementPriority::MOVEMENT_FORCED);
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
    {
        lastRebirthState[mapId] = false;
        // Optionally reset any other phase 2 timers/flags here
    }

    bool rebirthActive = alar->HasUnitState(UNIT_STATE_CASTING) &&
                         alar->FindCurrentSpellBySpellId(SPELL_REBIRTH_PHASE2);
    bool lastRebirth = lastRebirthState[mapId];

    // Detect transition: finished casting Rebirth (phase 2 begins)
    if (lastRebirth && !rebirthActive)
    {
        // Do your phase change logic here
        // e.g., phase2StartTime[mapId] = time(nullptr);
    }

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
