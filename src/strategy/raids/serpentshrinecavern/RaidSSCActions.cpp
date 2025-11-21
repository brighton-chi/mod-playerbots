#include <unordered_map>
#include <ctime>

#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

using namespace SerpentShrineCavernHelpers;
using namespace SerpentShrineCavernPositions;

// Trash Mobs

bool FollowMasterOutOfToxicPoolAction::Execute(Event event)
{
    /* Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (bot->GetExactDist2d(master) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(master->GetMapId(), master->GetPositionX(),
                      master->GetPositionY(), master->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false; */
    LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: entered for bot={}", bot->GetName());

    Aura* aura = bot->GetAura(SPELL_TOXIC_POOL);
    if (!aura)
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: no toxic pool aura present for bot={}", bot->GetName());
        return false;
    }

    // Find the dynamic object that created the pool
    DynamicObject* dynObj = aura->GetDynobjOwner();
    if (!dynObj)
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: aura present but dynobj owner is null for bot={}", bot->GetName());
        return false;
    }

    // Gather info for logs and behavior
    ObjectGuid dynGuid = dynObj->GetGUID();
    uint32 dynSpellId = dynObj->GetSpellId();
    const SpellInfo* sInfo = sSpellMgr->GetSpellInfo(dynSpellId);

    // Try using the dyn object's radius, fall back to spell info if needed
    float radius = 0.0f;
    if (dynObj->GetRadius() > 0.0f)
        radius = dynObj->GetRadius();
    else if (sInfo)
    {
        for (int e = 0; e < MAX_SPELL_EFFECTS; ++e)
        {
            if (sInfo->Effects[e].Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                (sInfo->Effects[e].Effect == SPELL_EFFECT_APPLY_AURA &&
                 sInfo->Effects[e].ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE))
            {
                radius = sInfo->Effects[e].CalcRadius();
                break;
            }
        }
    }

    LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: dynGuid={} spellId={} radius={} botPos=({}, {}) dynPos=({}, {}) for bot={}",
              dynGuid.ToString(), dynSpellId, radius,
              bot->GetPositionX(), bot->GetPositionY(),
              dynObj->GetPositionX(), dynObj->GetPositionY(), bot->GetName());

    if (radius <= 0.0f)
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: cannot determine radius for dynGuid={}, abort fallback for bot={}", dynGuid.ToString(), bot->GetName());
        return false;
    }

    // Compute XY vector to edge point
    const float buffer = 3.0f;          // safety buffer to be placed outside pool
    const float centerThreshold = 1.0f; // treat dist <= radius + this as "in pool"
    float dx = bot->GetPositionX() - dynObj->GetPositionX();
    float dy = bot->GetPositionY() - dynObj->GetPositionY();
    float distSq = dx * dx + dy * dy;
    float insideThresh = radius + centerThreshold;

    // If not inside or near the radius, nothing to do
    if (distSq > insideThresh * insideThresh)
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: bot is outside threshold (dist2={} thresh2={}): no move. bot={}", distSq, insideThresh * insideThresh, bot->GetName());
        return false;
    }

    float dist = std::sqrt(distSq);
    float safeDist = radius + buffer;
    float moveX, moveY;

    if (dist < 0.001f)
    {
        // at center -> random outward direction
        float angle = frand(0.0f, static_cast<float>(M_PI * 2.0));
        moveX = dynObj->GetPositionX() + cosf(angle) * safeDist;
        moveY = dynObj->GetPositionY() + sinf(angle) * safeDist;
    }
    else
    {
        moveX = dynObj->GetPositionX() + (dx / dist) * safeDist;
        moveY = dynObj->GetPositionY() + (dy / dist) * safeDist;
    }

    LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: computed move target (x={}, y={}) safeDist={} for bot={} (dist={})",
              moveX, moveY, safeDist, bot->GetName(), dist);

    // Attempt to stop any combat and move out
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    bool moved = MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED, true, false);

    if (moved)
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: MoveTo issued for bot={} to ({}, {})", bot->GetName(), moveX, moveY);
        return true;
    }

    // Fallback to FleePosition if MoveTo fails
    Position centerPos(dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ());
    if (FleePosition(centerPos, radius))
    {
        LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: FleePosition fallback used for bot={}", bot->GetName());
        return true;
    }

    LOG_DEBUG("playerbots", "AvoidToxicPoolOnAuraAction: movement/fallback failed for bot={}", bot->GetName());
    return false;
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event event)
{
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);
    if (!totem)
        return false;

    MarkTargetWithSkull(bot, totem);
    return false;
}

bool RancidMushroomMoveAwayFromMushroomSporeCloudAction::Execute(Event event)
{
    Unit* mushroom = GetFirstAliveUnitByEntry(botAI, NPC_RANCID_MUSHROOM);
    if (!mushroom)
        return false;

    float currentDistance = bot->GetExactDist2d(mushroom);
    const float safeDistance = 10.0f;
    if (currentDistance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveAway(mushroom, safeDistance - currentDistance + 2.0f, false);
    }

    return false;
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstablePositionFrostTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfHydrossAt100Percent(bot))
    {
        MarkTargetWithSquare(bot, hydross);
        SetRtiTarget(botAI, "square", hydross);

        if (bot->GetVictim() != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            const Position& position = HydrossFrostTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfHydrossAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        const uint32 mapId = hydross->GetMapId();
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToNaturePhaseTimer.find(mapId);

        if (it != hydrossChangeToNaturePhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossNatureTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return true;
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HydrossFrostTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return true;
        }
    }

    return false;
}

bool HydrossTheUnstablePositionNatureTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfCorruptionAt100Percent(bot))
    {
        MarkTargetWithTriangle(bot, hydross);
        SetRtiTarget(botAI, "triangle", hydross);

        if (bot->GetVictim() != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            const Position& position = HydrossNatureTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfCorruptionAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        const uint32 mapId = hydross->GetMapId();
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToFrostPhaseTimer.find(mapId);

        if (it != hydrossChangeToFrostPhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossFrostTankPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return true;
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HydrossNatureTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return true;
        }
    }

    return false;
}

bool HydrossTheUnstablePrioritizeElementalAddsAction::Execute(Event event)
{
    Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS);
    if (waterElemental)
    {
        if (IsMapIDTimerManager(botAI, bot))
            MarkTargetWithSkull(bot, waterElemental);

        SetRtiTarget(botAI, "skull", waterElemental);

        if (bot->GetTarget() != waterElemental->GetGUID())
        {
            bot->SetTarget(waterElemental->GetGUID());
            return Attack(waterElemental);
        }
    }
    else if (Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS))
    {
        if (IsMapIDTimerManager(botAI, bot))
            MarkTargetWithSkull(bot, natureElemental);

        SetRtiTarget(botAI, "skull", natureElemental);

        if (bot->GetTarget() != natureElemental->GetGUID())
        {
            bot->SetTarget(natureElemental->GetGUID());
            return Attack(natureElemental);
        }
    }

    return false;
}

bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    Group* group = bot->GetGroup();
    if (!hydross || !group)
        return false;

    const uint32 minInterval = 500;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (bot->GetExactDist2d(member) < 6.0f)
            return FleePosition(member->GetPosition(), 8.0f, minInterval);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    Group* group = bot->GetGroup();
    if (!hydross || !group)
        return false;

    LOG_DEBUG("playerbots", "HydrossMisdirect: entered for bot={} hydross={}", bot->GetName(), hydross->GetGUID().ToString());

    if (TryMisdirectToFrostTank(hydross, group))
        return true;

    if (TryMisdirectToNatureTank(hydross, group))
        return true;

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToFrostTank(Unit* hydross, Group* group)
{
    Player* frostTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            frostTank = member;
            break;
        }
    }

    if (HasNoMarkOfHydross(bot) && !hydross->HasAura(SPELL_CORRUPTION) && frostTank)
    {
        if (botAI->CanCastSpell("misdirection", frostTank))
            return botAI->CastSpell("misdirection", frostTank);

        if (!bot->HasAura(SPELL_MISDIRECTION))
            return false;

        if (botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToNatureTank(Unit* hydross, Group* group)
{
    Player* natureTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0))
        {
            natureTank = member;
            break;
        }
    }

    if (HasNoMarkOfCorruption(bot) && hydross->HasAura(SPELL_CORRUPTION) && natureTank)
    {
        if (botAI->CanCastSpell("misdirection", natureTank))
            return botAI->CastSpell("misdirection", natureTank);

        if (!bot->HasAura(SPELL_MISDIRECTION))
            return false;

        if (botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 mapId = hydross->GetMapId();
    const time_t now = std::time(nullptr);
    const int phaseEndStopSeconds = 6;
    const int phaseStartStopSeconds = 5;

    bool shouldStopDps = false;

    // 6 seconds after marks hit 100% in nature phase, stop DPS until transition into frost phase
    auto itNature = hydrossChangeToNaturePhaseTimer.find(mapId);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() && (now - itNature->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into frost phase
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(mapId);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() && (now - itFrostDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    // 6 seconds after marks hit 100% in frost phase, stop DPS until transition into nature phase
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(mapId);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() && (now - itFrost->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into nature phase
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(mapId);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() && (now - itNatureDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    if (shouldStopDps)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return true;
    }

    return false;
}

bool HydrossTheUnstableManageTimersAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 mapId = hydross->GetMapId();
    const time_t now = std::time(nullptr);

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        hydrossFrostDpsWaitTimer.erase(mapId);
        hydrossNatureDpsWaitTimer.erase(mapId);
        hydrossChangeToFrostPhaseTimer.erase(mapId);
        hydrossChangeToNaturePhaseTimer.erase(mapId);
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        hydrossFrostDpsWaitTimer.try_emplace(mapId, now);
        hydrossNatureDpsWaitTimer.erase(mapId);
        hydrossChangeToFrostPhaseTimer.erase(mapId);

        if (HasMarkOfHydrossAt100Percent(bot))
            hydrossChangeToNaturePhaseTimer.try_emplace(mapId, now);
    }
    else
    {
        hydrossNatureDpsWaitTimer.try_emplace(mapId, now);
        hydrossFrostDpsWaitTimer.erase(mapId);
        hydrossChangeToNaturePhaseTimer.erase(mapId);

        if (HasMarkOfCorruptionAt100Percent(bot))
            hydrossChangeToFrostPhaseTimer.try_emplace(mapId, now);
    }

    return false;
}


// The Lurker Below

// Limit to melee if movement into water issue is resolved
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    float bossFacing = lurker->GetOrientation();

    // Pick an angle behind Lurker (120-degree arc)
    float behindAngle = bossFacing + M_PI + ((rand() % 100) / 100.0f - 0.5f) * (M_PI / 3);

    // Random radius between 23 and 27 yards
    float radius = 23.0f + ((rand() % 500) / 100.0f);

    // Calculate target position on the circle
    float targetX = lurker->GetPositionX() + radius * cos(behindAngle);
    float targetY = lurker->GetPositionY() + radius * sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(lurker->GetMapId(), targetX, targetY, lurker->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowPositionMainTankAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (bot->GetVictim() != lurker)
        return Attack(lurker);

    const Position& position = LurkerMainTankPosition;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(bot->GetMapId(), position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowSpreadRangedAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    Group* group = bot->GetGroup();
    if (!lurker || !group)
        return false;

    const float minRadius = 25.0f;
    const float maxRadius = 27.0f;
    const float returnThreshold = 2.0f;
    const float referenceOrientation = 2.262f;

    if (lurker->GetHealth() == lurker->GetMaxHealth())
        lurkerRangedPositions.clear();

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            continue;
        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    // assign angle by index (even spacing) and randomize radius
    auto it = lurkerRangedPositions.find(guid);
    if (it == lurkerRangedPositions.end())
    {
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
        size_t count = rangedMembers.size();
        if (count == 0)
            return false;

        // spread 180° arc centered on referenceOrientation
        const float arcSpan = M_PI; // 180°
        float startAngle = referenceOrientation - arcSpan / 2.0f;

        float angle;
        if (count == 1)
            angle = referenceOrientation;
        else
            angle = startAngle + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        float radius = minRadius + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (maxRadius - minRadius);

        float tx = lurker->GetPositionX() + radius * cos(angle);
        float ty = lurker->GetPositionY() + radius * sin(angle);
        float tz = lurker->GetPositionZ();

        lurkerRangedPositions.emplace(guid, Position(tx, ty, tz));
        it = lurkerRangedPositions.find(guid);
    }

    if (it == lurkerRangedPositions.end())
        return false;

    const Position& target = it->second;

    if (!bot->IsWithinDist2d(target.GetPositionX(), target.GetPositionY(), returnThreshold))
    {
        return MoveTo(bot->GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Below for implementation later if movement into water issue is resolved
/* bool TheLurkerBelowPositionOtherMeleeAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    Group* group = bot->GetGroup();
    if (!lurker || !group)
        return false;

    auto groups = GetMeleeBotGroups(group);

    int8 myGroup = -1;
    for (int8 i = 0; i < 2; ++i)
    {
        for (Player* member : groups[i])
        {
            if (member == bot)
            {
                myGroup = i;
                break;
            }
        }
        if (myGroup != -1) break;
    }
    if (myGroup == -1)
        return false;

    // Assign E/W melee positions based on group index
    const Position* meleePositions[2] = { &LurkerEMeleePosition, &LurkerWMeleePosition };
    const Position* target = meleePositions[myGroup];

    // Move if not close enough
    if (bot->GetExactDist2d(target->GetPositionX(), target->GetPositionY()) > 0.2f)
    {
        return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowPositionRangedDpsAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    Group* group = bot->GetGroup();
    if (!lurker || !group)
        return false;

    auto groups = GetRangedDpsBotGroups(group);

    int8 myGroup = -1;
    for (int8 i = 0; i < 3 && myGroup == -1; ++i)
    {
        for (Player* member : groups[i])
        {
            if (member == bot)
            {
                myGroup = i;
                break;
            }
        }
    }
    if (myGroup == -1)
        return false;

    const Position* landPositions[3] =
        { &LurkerERangedDpsPosition, &LurkerNWRangedDpsPosition, &LurkerNERangedDpsPosition };
    const Position* target = landPositions[myGroup];

    if (bot->GetExactDist2d(target->GetPositionX(), target->GetPositionY()) > 0.2f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowPositionHealerAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    Group* group = bot->GetGroup();
    if (!lurker || !group)
        return false;

    auto groups = GetHealerBotGroups(group);

    int8 myGroup = -1;
    for (int8 i = 0; i < 3 && myGroup == -1; ++i)
    {
        for (Player* member : groups[i])
        {
            if (member == bot)
            {
                myGroup = i;
                break;
            }
        }
    }
    if (myGroup == -1)
        return false;

    const Position* landPositions[3] =
        { &LurkerSEHealerLandPosition, &LurkerSWHealerLandPosition, &LurkerNHealerLandPosition };
    const Position* target = landPositions[myGroup];

    if (bot->GetExactDist2d(target->GetPositionX(), target->GetPositionY()) > 0.2f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
} */

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 mapId = lurker->GetMapId();
    const time_t now = std::time(nullptr);

    if (lurker->GetHealth() == lurker->GetMaxHealth())
    {
        lurkerSpoutTimer.erase(mapId);
        return false;
    }

    auto it = lurkerSpoutTimer.find(mapId);
    if (it != lurkerSpoutTimer.end() && it->second <= now)
    {
        lurkerSpoutTimer.erase(it);
        it = lurkerSpoutTimer.end();
    }

    if (IsLurkerCastingSpout(lurker) && it == lurkerSpoutTimer.end())
        lurkerSpoutTimer.emplace(mapId, now + 20); // 20s channel

    return false;
}

// Leotheras the Blind

bool LeotherasTheBlindTargetSpellbindersAction::Execute(Event event)
{
    Unit* spellbinder = GetFirstAliveUnitByEntry(botAI, NPC_GREYHEART_SPELLBINDER);
    if (!spellbinder || !spellbinder->IsInCombat())
        return false;

    MarkTargetWithSkull(bot, spellbinder);

    return false;
}

bool LeotherasTheBlindDemonFormTankAttackBossAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasDemon);
    SetRtiTarget(botAI, "square", leotherasDemon);

    if (bot->GetVictim() != leotherasDemon)
    {
        bot->SetTarget(leotherasDemon->GetGUID());
        return Attack(leotherasDemon);
    }

    if (botAI->IsMainTank(bot) && botAI->IsMelee(bot) && leotherasDemon->GetVictim() == bot)
    {
        float maxMeleeRange = bot->GetMeleeRange(leotherasDemon);
        const float meleeRangeBuffer = 0.02f;
        float angle = atan2(bot->GetPositionY() - leotherasDemon->GetPositionY(),
                            bot->GetPositionX() - leotherasDemon->GetPositionX());

        float targetX = leotherasDemon->GetPositionX() + (maxMeleeRange - meleeRangeBuffer) * cos(angle);
        float targetY = leotherasDemon->GetPositionY() + (maxMeleeRange - meleeRangeBuffer) * sin(angle);

        if (fabs(bot->GetExactDist2d(leotherasDemon) - (maxMeleeRange - meleeRangeBuffer)) > 0.1f)
        {
            return MoveTo(leotherasDemon->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool LeotherasTheBlindPositionRangedAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    Group* group = bot->GetGroup();
    if (!leotheras || !demonFormTank || !group)
        return false;

    const uint32 minInterval = 500;
    if (leotheras && bot->GetExactDist2d(leotheras) < 10.0f)
        return FleePosition(leotheras->GetPosition(), 12.0f, minInterval);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (demonFormTank == member && leotherasDemon && bot->GetExactDist2d(member) < 10.0f)
            return FleePosition(member->GetPosition(), 12.0f, minInterval);

        if (bot->GetExactDist2d(member) < 5.0f)
            return FleePosition(member->GetPosition(), 6.0f, minInterval);
    }

    return false;
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event event)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    if (leotherasPhase3Demon && demonFormTank == bot)
        return false;

    if (leotherasHuman)
    {
        float currentDistance = bot->GetExactDist2d(leotherasHuman);
        const float safeDistance = 15.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(leotherasHuman, safeDistance - currentDistance + 10.0f);
        }
    }

    return false;
}

// Applies only if there is no Warlock tank
// Try to keep maximum melee distance to avoid Chaos Blast
bool LeotherasTheBlindDemonFormPositionMeleeAction::Execute(Event event)
{
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasPhase2Demon && !leotherasPhase3Demon)
        return false;

    if (!botAI->IsTank(bot) && leotherasPhase2Demon && leotherasPhase2Demon->GetVictim() != bot)
    {
        float maxMeleeRange = bot->GetMeleeRange(leotherasPhase2Demon);
        const float meleeRangeBuffer = 0.02f;
        float behindAngle = Position::NormalizeOrientation(leotherasPhase2Demon->GetOrientation() + M_PI);

        float targetX = leotherasPhase2Demon->GetPositionX() + (maxMeleeRange - meleeRangeBuffer) * cos(behindAngle);
        float targetY = leotherasPhase2Demon->GetPositionY() + (maxMeleeRange - meleeRangeBuffer) * sin(behindAngle);

        if (fabs(bot->GetExactDist2d(targetX, targetY) - (maxMeleeRange - meleeRangeBuffer)) > 0.1f)
        {
            return MoveTo(leotherasPhase2Demon->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    if (!botAI->IsTank(bot) && leotherasPhase3Demon && leotherasPhase3Demon->GetVictim() != bot)
    {
        float currentDistance = bot->GetExactDist2d(leotherasPhase3Demon);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(leotherasPhase3Demon, safeDistance - currentDistance + 5.0f);
        }
    }

    return false;
}

bool LeotherasTheBlindInnerDemonCheatAction::Execute(Event event)
{
    Unit* innerDemon = GetFirstAliveUnitByEntry(botAI, NPC_INNER_DEMON);
    if (innerDemon)
    {
        uint8 tab = AiFactory::GetPlayerSpecTab(bot);
        /* Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 20, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        return true; */
        if (botAI->IsHeal(bot) || botAI->IsTank(bot) || bot->getClass() == CLASS_HUNTER || (bot->getClass() == CLASS_WARLOCK && tab == 0))
        {
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 20, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
            return true;
        }
        /* else if (innerDemon->GetHealthPct() >= 60.0f)
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 2, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true); */ // test if other DPS can handle it
    }

    return false;
}

bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event event)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasHuman || !leotherasDemon)
        return false;

    MarkTargetWithStar(bot, leotherasHuman);
    SetRtiTarget(botAI, "star", leotherasHuman);

    if (bot->GetVictim() != leotherasHuman)
    {
        bot->SetTarget(leotherasHuman->GetGUID());
        return Attack(leotherasHuman);
    }

    if (botAI->IsTank(bot) && leotherasHuman->GetVictim() == bot)
    {
        if (leotherasHuman->GetExactDist2d(leotherasDemon) < 25.0f)
        {
            float angle = atan2(bot->GetPositionY() - leotherasDemon->GetPositionY(),
                                bot->GetPositionX() - leotherasDemon->GetPositionX());
            float targetX = bot->GetPositionX() + 27.0f * cos(angle);
            float targetY = bot->GetPositionY() + 27.0f * sin(angle);

            return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                            MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else if (botAI->IsTank(bot) && !bot->IsWithinMeleeRange(leotherasHuman))
        {
            return MoveTo(leotherasHuman->GetMapId(), leotherasHuman->GetPositionX(),
                          leotherasHuman->GetPositionY(), leotherasHuman->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool LeotherasTheBlindMisdirectBossToDemonFormTankAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!leotherasDemon || !demonFormTank)
        return false;

    if (botAI->CanCastSpell("misdirection", demonFormTank))
        return botAI->CastSpell("misdirection", demonFormTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", leotherasDemon))
        return botAI->CastSpell("steady shot", leotherasDemon);

    return false;
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool LeotherasTheBlindManageTimersAndTrackersAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    const uint32 mapId = leotheras->GetMapId();
    const time_t now = std::time(nullptr);

    // Encounter start/reset: clear all timers
    if (leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        leotherasHumanFormDpsWaitTimer.erase(mapId);
        leotherasDemonFormDpsWaitTimer.erase(mapId);
        leotherasFinalPhaseDpsWaitTimer.erase(mapId);
        return false;
    }

    // Human Phase
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasPhase3Demon)
    {
        leotherasHumanFormDpsWaitTimer.try_emplace(mapId, now);
        leotherasDemonFormDpsWaitTimer.erase(mapId);
    }
    // Demon Phase
    else if (Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI))
    {
        leotherasDemonFormDpsWaitTimer.try_emplace(mapId, now);
        leotherasHumanFormDpsWaitTimer.erase(mapId);
    }
    // Final Phase (<15% HP)
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        leotherasFinalPhaseDpsWaitTimer.try_emplace(mapId, now);
        leotherasHumanFormDpsWaitTimer.erase(mapId);
        leotherasDemonFormDpsWaitTimer.erase(mapId);
    }

    return false;
}

// Fathom-Lord Karathress

bool FathomLordKarathressMainTankPositionBossAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    MarkTargetWithTriangle(bot, karathress);
    SetRtiTarget(botAI, "triangle", karathress);

    if (bot->GetVictim() != karathress)
        return Attack(karathress);

    if (karathress->GetVictim() == bot && bot->IsWithinMeleeRange(karathress))
    {
        const Position& position = KarathressTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool FathomLordKarathressFirstAssistTankPositionSharkkisAction::Execute(Event event)
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (!sharkkis)
        return false;

    MarkTargetWithStar(bot, sharkkis);
    SetRtiTarget(botAI, "star", sharkkis);

    if (bot->GetVictim() != sharkkis)
        return Attack(sharkkis);

    if (sharkkis->GetVictim() == bot && bot->IsWithinMeleeRange(sharkkis))
    {
        const Position& position = SharkkisTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool FathomLordKarathressSecondAssistTankPositionTidalvessAction::Execute(Event event)
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (!tidalvess)
        return false;

    MarkTargetWithCircle(bot, tidalvess);
    SetRtiTarget(botAI, "circle", tidalvess);

    if (bot->GetVictim() != tidalvess)
        return Attack(tidalvess);

    if (tidalvess->GetVictim() == bot && bot->IsWithinMeleeRange(tidalvess))
    {
        const Position& position = TidalvessTankPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool FathomLordKarathressThirdAssistTankPositionCaribdisAction::Execute(Event event)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    MarkTargetWithDiamond(bot, caribdis);
    SetRtiTarget(botAI, "diamond", caribdis);

    if (bot->GetVictim() != caribdis)
        return Attack(caribdis);

    if (caribdis->GetVictim() == bot)
    {
        const Position& position = CaribdisTankPosition;
        // Distance to tank position check first for Caribdis due to need to move her ASAP
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(caribdis))
        {
            return MoveTo(caribdis->GetMapId(), caribdis->GetPositionX(),
                          caribdis->GetPositionY(), caribdis->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event event)
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

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 2))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member)->IsAssistTankOfIndex(member, 1))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
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

    if (!bossTarget || !tankTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event event)
{
    // Target priority 1: Spitfire Totems for melee
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
    if (totem && botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
        {
            bot->SetTarget(totem->GetGUID());
            return Attack(totem);
        }

        return false;
    }

    // Target priority 2: Tidalvess for all dps
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetTarget() != tidalvess->GetGUID())
        {
            bot->SetTarget(tidalvess->GetGUID());
            return Attack(tidalvess);
        }

        return false;
    }

    // Target priority 3: Caribdis for ranged
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (botAI->IsRangedDps(bot) && caribdis && caribdis->IsAlive())
    {
        SetRtiTarget(botAI, "diamond", caribdis);

        const Position& position = CaribdisRangedDpsPosition;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        if (bot->GetTarget() != caribdis->GetGUID())
        {
            bot->SetTarget(caribdis->GetGUID());
            return Attack(caribdis);
        }

        return false;
    }

    // Target priority 4: Sharkkis for melee (and ranged if Caribdis down first)
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetTarget() != sharkkis->GetGUID())
        {
            bot->SetTarget(sharkkis->GetGUID());
            return Attack(sharkkis);
        }

        return false;
    }

    // Target priority 5: Sharkkis pets for all dps
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && fathomSporebat->IsAlive() && botAI->IsMelee(bot))
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetTarget() != fathomSporebat->GetGUID())
        {
            bot->SetTarget(fathomSporebat->GetGUID());
            return Attack(fathomSporebat);
        }

        return false;
    }

    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && fathomLurker->IsAlive() && botAI->IsMelee(bot))
    {
        MarkTargetWithSquare(bot, fathomLurker);
        SetRtiTarget(botAI, "square", fathomLurker);

        if (bot->GetTarget() != fathomLurker->GetGUID())
        {
            bot->SetTarget(fathomLurker->GetGUID());
            return Attack(fathomLurker);
        }

        return false;
    }

    // Target priority 6: Karathress for all dps
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetTarget() != karathress->GetGUID())
        {
            bot->SetTarget(karathress->GetGUID());
            return Attack(karathress);
        }
    }

    return false;
}

bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event event)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    const Position& position = CaribdisHealerPosition;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(7.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, true,
                    MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    const uint32 mapId = karathress->GetMapId();
    const time_t now = std::time(nullptr);

    if (karathress->GetHealth() == karathress->GetMaxHealth())
        karathressDpsWaitTimer.insert_or_assign(mapId, now);

    return false;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    Group* group = bot->GetGroup();
    if (!tidewalker || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", tidewalker))
        return botAI->CastSpell("steady shot", tidewalker);

    return false;
}

bool MorogrimTidewalkerMoveBossToTankPositionAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (bot->GetVictim() != tidewalker)
        return Attack(tidewalker);

    if (tidewalker->GetVictim() == bot && bot->IsWithinMeleeRange(tidewalker))
    {
        if (tidewalker->GetHealthPct() > 26.0f)
            return MoveToPhase1TankPosition(tidewalker);
        else
            return MoveToPhase2TankPosition(tidewalker);
    }

    return false;
}

bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase1TankPosition(Unit* tidewalker)
{
    const Position& phase1 = TidewalkerPhase1TankPosition;
    if (bot->GetExactDist2d(phase1.GetPositionX(), phase1.GetPositionY()) > 1.0f)
    {
        float dX = phase1.GetPositionX() - bot->GetPositionX();
        float dY = phase1.GetPositionY() - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(4.5f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                        MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition(Unit* tidewalker)
{
    const Position& phase2 = TidewalkerPhase2TankPosition;
    const Position& transition = TidewalkerPhaseTransitionWaypoint;

    const ObjectGuid botGuid = bot->GetGUID();
    auto itStep = tidewalkerTankStep.find(botGuid);
    uint8 step = (itStep != tidewalkerTankStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY()) > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else
            tidewalkerTankStep.emplace(botGuid, 1);
    }

    if (step == 1)
    {
        if (bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY()) > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool MorogrimTidewalkerPhase2RepositionRangedAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const Position& phase2 = TidewalkerPhase2RangedPosition;
    const Position& transition = TidewalkerPhaseTransitionWaypoint;

    const ObjectGuid botGuid = bot->GetGUID();
    auto itStep = tidewalkerRangedStep.find(botGuid);
    uint8 step = (itStep != tidewalkerRangedStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY()) > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            tidewalkerRangedStep.emplace(botGuid, 1);
            step = 1;
        }
    }

    if (step == 1)
    {
        if (bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY()) > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MorogrimTidewalkerResetPhaseTransitionStepsAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();

    if (tidewalker->GetHealth() == tidewalker->GetMaxHealth())
    {
        tidewalkerTankStep.erase(botGuid);
        tidewalkerRangedStep.erase(botGuid);
    }

    return false;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjMainTankPositionBossAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (bot->GetVictim() != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() == bot && bot->IsWithinMeleeRange(vashj))
    {
        if (IsLadyVashjInPhase1(botAI))
        {
            const Position& position = VashjPlatformCenterPosition;
            if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(4.5f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.GetPositionZ(), false, false, false, false,
                            MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }

        if (IsLadyVashjInPhase3(botAI))
        {
            Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
            if (enchanted)
            {
                float currentDistance = bot->GetExactDist2d(enchanted);
                const float safeDistance = 10.0f;
                if (currentDistance < safeDistance)
                    return MoveAway(enchanted, safeDistance - currentDistance + 5.0f);
            }
        }
    }

    return false;
}

bool LadyVashjPhase1PositionRangedAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const Position& center = VashjPlatformCenterPosition;
    const float minSpreadRadius = 20.0f;
    const float maxSpreadRadius = 30.0f;

    std::vector<Player*> spreadMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member))
        {
            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI->IsRanged(member))
                spreadMembers.push_back(member);
        }
    }

    const ObjectGuid guid = bot->GetGUID();

    auto itPos = vashjRangedPositions.find(guid);
    auto itReached = vashjHasReachedRangedPosition.find(guid);
    if (itPos == vashjRangedPositions.end())
    {
        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        size_t botIndex = (it != spreadMembers.end()) ? std::distance(spreadMembers.begin(), it) : 0;
        size_t count = spreadMembers.size();

        const float referenceAngle = M_PI / 2.0f; // north
        const float arcSpan = M_PI; // 180°
        const float startAngle = referenceAngle - arcSpan / 2.0f;
        float angle = (count <= 1) ? referenceAngle : startAngle + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        uint32 botSeed = guid.GetCounter();
        float radius = minSpreadRadius + (botSeed % 1000) / 1000.0f * (maxSpreadRadius - minSpreadRadius);
        float targetX = center.GetPositionX() + radius * cos(angle);
        float targetY = center.GetPositionY() + radius * sin(angle);
        float tz = center.GetPositionZ();

        auto res = vashjRangedPositions.emplace(guid, Position(targetX, targetY, tz));
        itPos = res.first;
        vashjHasReachedRangedPosition.emplace(guid, false);
        itReached = vashjHasReachedRangedPosition.find(guid);
    }

    if (itPos == vashjRangedPositions.end())
        return false;

    Position targetPosition = itPos->second;
    if (itReached == vashjHasReachedRangedPosition.end() || !(itReached->second))
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            return MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        if (itReached != vashjHasReachedRangedPosition.end())
            itReached->second = true;
    }

    return false;
 }

bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    float dist = bot->GetExactDist2d(mainTank);
    if (dist >= 27.0f)
    {
        float angle = atan2(bot->GetPositionY() - mainTank->GetPositionY(),
                      bot->GetPositionX() - mainTank->GetPositionX());
        float targetX = mainTank->GetPositionX() + 25.0f * cos(angle);
        float targetY = mainTank->GetPositionY() + 25.0f * sin(angle);

        return MoveTo(mainTank->GetMapId(), targetX, targetY, mainTank->GetPositionZ(),
                        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (!botAI->HasStrategy("grounding totem", BotState::BOT_STATE_COMBAT))
        botAI->ChangeStrategy("+grounding totem", BotState::BOT_STATE_COMBAT);

    if (!bot->HasAura(SPELL_GROUNDING_TOTEM_EFFECT) && botAI->CanCastSpell("grounding totem", bot))
        return botAI->CastSpell("grounding totem", bot);

    return false;
}

bool LadyVashjMisdirectBossToMainTankAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!vashj || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", vashj))
        return botAI->CastSpell("steady shot", vashj);

    return false;
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // If the main tank has Static Charge, other group members should move away
    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member) &&
            member->HasAura(SPELL_STATIC_CHARGE))
        {
            mainTank = member;
            break;
        }
    }

    if (mainTank && bot != mainTank)
    {
        float currentDistance = bot->GetExactDist2d(mainTank);
        const float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
            // return MoveAway(mainTank, safeDistance - currentDistance + 0.5f);
            return MoveFromGroup(safeDistance + 0.5f);
    }

    // If any other bot has static charge, it should move away from other group members
    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float currentDistance = bot->GetExactDist2d(member);
            const float safeDistance = 10.0f;
            if (currentDistance < safeDistance)
                // return MoveAway(member, safeDistance - currentDistance + 0.5f);
                return MoveFromGroup(safeDistance + 0.5f);
        }
    }

    return false;
}

bool LadyVashjMisdirectStriderToFirstAssistTankAction::Execute(Event event)
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    Group* group = bot->GetGroup();
    if (!strider || !group)
        return false;

    Player* firstAssistTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0))
        {
            firstAssistTank = member;
            break;
        }
    }

    if (!firstAssistTank || strider->GetVictim() == firstAssistTank)
        return false;

    if (botAI->CanCastSpell("misdirection", firstAssistTank))
        return botAI->CastSpell("misdirection", firstAssistTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", strider))
        return botAI->CastSpell("steady shot", strider);

    return false;
}

bool LadyVashjTankAttackAndMoveAwayStriderAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    Group* group = bot->GetGroup();
    if (!vashj || !strider || !group)
        return false;

    if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsTank(bot))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD_CHEAT))
            bot->AddAura(SPELL_FEAR_WARD_CHEAT, bot);

        if (botAI->IsAssistTankOfIndex(bot, 0) && bot->GetVictim() != strider)
            return Attack(strider);

        if (strider->GetVictim() == bot)
        {
            float currentDistance = bot->GetExactDist2d(vashj);
            const float safeDistance = 20.0f;

            if (currentDistance < safeDistance)
                return MoveAway(vashj, safeDistance - currentDistance + 5.0f);

            Player* firstCorePasser  = GetFirstTaintedCorePasser(group, botAI);
            Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);

            // Move the Strider away from the first two passers; the third and fourth passers
            // are rarely needed so they are ignored to avoid too many restrictions on movement
            for (Player* passer : { firstCorePasser, secondCorePasser })
            {
                if (passer && passer != bot)
                {
                    float currentDistFromPasser = strider->GetExactDist2d(passer);
                    const float safeDistFromPasser = 15.0f;
                    if (currentDistFromPasser < safeDistFromPasser)
                        return MoveAway(strider, safeDistFromPasser - currentDistFromPasser + 5.0f);
                }
            }
        }

        return false;
    }

    // Don't move away if raid cheats are enabled, or regardless if the bot is a tank
    if (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsTank(bot))
    {
        float currentDistance = bot->GetExactDist2d(strider);
        const float safeDistance = 15.0f;
        if (currentDistance < safeDistance)
            return MoveAway(strider, safeDistance - currentDistance + 5.0f);
    }

    if (!botAI->HasCheat(BotCheatMask::raid))
    {
        if (!strider->HasAura(SPELL_HEAVY_NETHERWEAVE_NET))
        {
            Item* net = bot->GetItemByEntry(ITEM_HEAVY_NETHERWEAVE_NET);
            if (net && botAI->HasItemInInventory(ITEM_HEAVY_NETHERWEAVE_NET) &&
                botAI->CanCastSpell("heavy netherweave net", strider))
                return botAI->CastSpell("heavy netherweave net", strider);
        }

        if (!strider->HasAura(SPELL_FROST_SHOCK) && bot->getClass() == CLASS_SHAMAN &&
            botAI->CanCastSpell("frost shock", strider))
            return botAI->CastSpell("frost shock", strider);

        if (!strider->HasAura(SPELL_CURSE_OF_EXHAUSTION) && bot->getClass() == CLASS_WARLOCK &&
            botAI->CanCastSpell("curse of exhaustion", strider))
            return botAI->CastSpell("curse of exhaustion", strider);

        if (!strider->HasAura(SPELL_SLOW) && bot->getClass() == CLASS_MAGE &&
            botAI->CanCastSpell("slow", strider))
            return botAI->CastSpell("slow", strider);
    }

    return false;
}

bool LadyVashjAssignPhase2DpsPriorityAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    Unit* target = nullptr;
    Unit* tainted = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    if (bot->GetVictim() == vashj && (IsLadyVashjInPhase2(botAI) || (IsLadyVashjInPhase3(botAI) &&
        (enchanted && enchanted->IsAlive() || elite && elite->IsAlive() || strider && strider->IsAlive()))))
    {
        // Stop attacking Vashj in Phase 2
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    const Position& center = VashjPlatformCenterPosition;
    const float maxSearchRange = botAI->IsRangedDps(bot) ? 60.0f : (botAI->IsMelee(bot) ? 55.0f : 40.0f);
    const float maxPursueRange = maxSearchRange - 5.0f;

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidPhase2CombatNpc(unit, botAI))
            continue;

        // Skip NPCs outside the search radius
        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (IsLadyVashjInPhase2(botAI) && distFromCenter > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
            case NPC_TAINTED_ELEMENTAL:
                if (!tainted || bot->GetExactDist2d(unit) < bot->GetExactDist2d(tainted))
                    tainted = unit;
                break;

            case NPC_ENCHANTED_ELEMENTAL:
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;
                break;

            case NPC_COILFANG_ELITE:
                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case NPC_COILFANG_STRIDER:
                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case NPC_TOXIC_SPOREBAT:
                if (!sporebat || unit->GetHealthPct() < sporebat->GetHealthPct())
                    sporebat = unit;
                break;

            case NPC_LADY_VASHJ:
                vashj = unit;
                break;

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (IsLadyVashjInPhase2(botAI))
    {
        if (botAI->IsRanged(bot))
        {
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { tainted, enchanted, strider, elite };
            else
                targets = { tainted, strider, elite, enchanted };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { tainted, enchanted, elite };
        else if (botAI->IsTank(bot))
        {
            // With raid cheats enabled, the first assist tank will tank the strider
            if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsAssistTankOfIndex(bot, 0))
                targets = { strider, enchanted, tainted };
            else
                targets = { elite, enchanted, tainted };
        }
        else
            targets = { tainted, enchanted, elite, strider };
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (botAI->IsTank(bot))
        {
            if (botAI->IsMainTank(bot))
            {
                MarkTargetWithDiamond(bot, vashj);
                SetRtiTarget(botAI, "diamond", vashj);
                targets = { vashj };
            }
            else if (botAI->IsAssistTankOfIndex(bot, 0))
            {
                if (botAI->HasCheat(BotCheatMask::raid))
                    targets = { strider, enchanted, vashj };
                else
                    targets = { elite, enchanted, vashj };
            }
            else
                targets = { elite, enchanted, vashj };
        }
        if (botAI->IsRanged(bot))
        {
            if (bot->getClass() == CLASS_HUNTER)
                targets = { enchanted, sporebat, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    for (Unit* t : targets)
    {
        if (t && t->IsAlive())
        {
            target = t;
            break;
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    if (target && currentTarget == target && IsValidPhase2CombatNpc(currentTarget, botAI))
        return false;

    if (target && bot->GetExactDist2d(target) <= maxPursueRange &&
        bot->GetTarget() != target->GetGUID())
    {
        bot->SetTarget(target->GetGUID());
        return Attack(target);
    }

    // Clear invalid current target to avoid null==null early-return issues
    if (currentTarget && (!currentTarget->IsAlive() || !IsValidPhase2CombatNpc(currentTarget, botAI)))
    {
        LOG_DEBUG("playerbots", "Clearing invalid current target for {}: {}", bot->GetName(),
                  currentTarget ? currentTarget->GetName() : std::string("null"));
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    if (!bot->GetVictim())
    {
        Player* master = botAI->GetMaster();
        Player* designatedLooter = GetDesignatedCoreLooter(bot->GetGroup(), master, botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(bot->GetGroup(), botAI);
        // A bot will not move back to the middle if:
        // (1) The designated looter is within 10 yards of a tainted elemental, and the bot is
        //     either the designated looter or the first core passer, or
        // (2) It has the Paralyze aura
        if (designatedLooter && tainted && designatedLooter->GetExactDist2d(tainted) < 5.0f &&
            (designatedLooter == bot || (firstCorePasser && firstCorePasser == bot)) ||
            bot->HasAura(SPELL_PARALYZE))
            return false;

        const Position& center = VashjPlatformCenterPosition;
        if (bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 35.0f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);

            return MoveInside(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(), 30.0f,
                              MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

// If cheats are enabled, the first returned melee DPS bot will teleport to tainted elementals
// Such bot will recover HP and remove Poison Bolt debuff while attacking the elemental
bool LadyVashjTeleportToTaintedElementalAction::Execute(Event event)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    lastTaintedGuid = tainted->GetGUID();
    if (bot->GetExactDist2d(tainted) >= 10.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->TeleportTo(tainted->GetMapId(), tainted->GetPositionX(), tainted->GetPositionY(), tainted->GetPositionZ(), tainted->GetOrientation());
    }

    if (bot->GetVictim() != tainted)
    {
        MarkTargetWithStar(bot, tainted);
        SetRtiTarget(botAI, "star", tainted);

        return Attack(tainted);
    }

    if (bot->GetExactDist2d(tainted) < 5.0f)
    {
        bot->SetFullHealth();
        bot->RemoveAura(SPELL_POISON_BOLT);
    }

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event)
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

        if (creature->GetEntry() != NPC_TAINTED_ELEMENTAL || creature->IsAlive())
            continue;

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(object);

        if (dist > maxLootRange)
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);

        // Invoke OpenLootAction to request the server's StoreLoot packet for this corpse.
        // Attempt a forced autostore from SSC (without modifying LootAction) by
        // scheduling a short-timer to send CMSG_AUTOSTORE_LOOT_ITEM (index 0) once the
        // server has had time to send the StoreLoot packet.
        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());

        if (!opened)
            return opened;

        // If anyone in the group already has the core, skip creating a duplicate
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                    return true;
            }
        }

        // Schedule autostore attempt + reconcile fallback
        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        const uint8 guessedIndex = 0; // best-effort guess (most single-item corpses use index 0)

        botAI->AddTimedEvent([botGuid, corpseGuid, guessedIndex]()
        {
        Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver)
                return;

            // Double-check someone else didn't obtain the core in the meantime using receiver's group
            if (Group* group = receiver->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                        return;
                }
            }

            // Set the loot GUID so server treats the following autostore as targeted to this corpse
            receiver->SetLootGUID(corpseGuid);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << guessedIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event event)
{
    Player* master = botAI->GetMaster();
    Group* group = bot->GetGroup();
    if (!master || !group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);
    if (!designatedLooter)
        return false;

    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);
    Unit* closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(bot, designatedLooter);

    if (!firstCorePasser || !secondCorePasser || !thirdCorePasser || !fourthCorePasser || !closestTrigger)
        return false;

    // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
    if (bot == firstCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        if (LineUpFirstCorePasser(designatedLooter, closestTrigger))
            return true;
    }
    else if (bot == secondCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        if (LineUpSecondCorePasser(firstCorePasser, closestTrigger))
            return true;
    }
    else if (bot == thirdCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        if (LineUpThirdCorePasser(secondCorePasser, closestTrigger))
            return true;
    }
    else if (bot == fourthCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        if (LineUpFourthCorePasser(thirdCorePasser, closestTrigger))
            return true;
    }

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (item && botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Designated core looter logic--applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter)
        {
            if (IsFirstCorePasserInIntendedPosition(designatedLooter, firstCorePasser, closestTrigger))
            {
                const ObjectGuid giverGuid = bot->GetGUID();
                const time_t now = std::time(nullptr);

                // Prevent duplicate imbue requests while a previous imbue is still pending/fallback
                if (imbuePending.count(giverGuid))
                {
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: imbue already pending for giver {}, skipping", bot->GetName());
                }
                else
                {
                   auto it = lastImbueAttempt.find(giverGuid);
                   bool imbueQueued = false;

                    if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                    {
                        if (IsFirstCorePasserInIntendedPosition(designatedLooter, firstCorePasser, closestTrigger))
                        {
                            botAI->ImbueItem(item, firstCorePasser);
                            LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: Imbue visual sent from {} -> {}", bot->GetName(), firstCorePasser->GetName());
                            imbueQueued = true;
                        }
                    }

                    // guaranteed fallback to create/store the core after delay
                    ScheduleStoreCoreAfterImbue(botAI, bot, firstCorePasser);
                    if (imbueQueued)
                        return true;
                }
            }
        }
        else if (bot == firstCorePasser)
        {
            if (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger))
            {
                const ObjectGuid giverGuid = bot->GetGUID();
                const time_t now = std::time(nullptr);

                if (imbuePending.count(giverGuid))
                {
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: imbue already pending for giver {}, skipping", bot->GetName());
                }
                else
                {
                    auto it = lastImbueAttempt.find(giverGuid);
                    bool imbueQueued = false;

                    if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                    {
                        if (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger))
                        {
                            botAI->ImbueItem(item, secondCorePasser);
                            imbueQueued = true;
                        }
                    }

                    ScheduleStoreCoreAfterImbue(botAI, bot, secondCorePasser);
                    if (imbueQueued)
                        return true;
                }
            }
        }
        // Second core passer logic (use core or pass to third)
        else if (bot == secondCorePasser)
        {
            UseCoreOnNearestGenerator();
        }
        else if (IsThirdCorePasserInIntendedPosition(secondCorePasser, thirdCorePasser, closestTrigger))
        {
            const ObjectGuid giverGuid = bot->GetGUID();
            const time_t now = std::time(nullptr);

            if (imbuePending.count(giverGuid))
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: imbue already pending for giver {}, skipping", bot->GetName());
            }
            else
            {
                auto it = lastImbueAttempt.find(giverGuid);
                bool imbueQueued = false;

                if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                {
                    if (IsThirdCorePasserInIntendedPosition(secondCorePasser, thirdCorePasser, closestTrigger))
                    {
                        botAI->ImbueItem(item, thirdCorePasser);
                        imbueQueued = true;
                    }
                }

                ScheduleStoreCoreAfterImbue(botAI, bot, thirdCorePasser);
                if (imbueQueued)
                    return true;
            }
        }
        else if (bot == thirdCorePasser)
        {
            UseCoreOnNearestGenerator();
        }
        else if (IsFourthCorePasserInIntendedPosition(thirdCorePasser, fourthCorePasser, closestTrigger))
        {
            const ObjectGuid giverGuid = bot->GetGUID();
            const time_t now = std::time(nullptr);

            if (imbuePending.count(giverGuid))
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: imbue already pending for giver {}, skipping", bot->GetName());
            }
            else
            {
                auto it = lastImbueAttempt.find(giverGuid);
                bool imbueQueued = false;

                if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                {
                    if (IsFourthCorePasserInIntendedPosition(thirdCorePasser, fourthCorePasser, closestTrigger))
                    {
                        botAI->ImbueItem(item, fourthCorePasser);
                        imbueQueued = true;
                    }
                }

                ScheduleStoreCoreAfterImbue(botAI, bot, fourthCorePasser);
                if (imbueQueued)
                    return true;
            }
        }
        else if (bot == fourthCorePasser)
            UseCoreOnNearestGenerator();
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* designatedLooter, Unit* closestTrigger)
{
    const float centerX = VashjPlatformCenterPosition.GetPositionX();
    const float centerY = VashjPlatformCenterPosition.GetPositionY();
    const float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * cos(angle);
    float targetY = centerY + radius * sin(angle);
    const float targetZ = 41.097f;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger)
{
    float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float targetX, targetY, targetZ; // Target is on a line between firstCorePasser and closestTrigger
    const float thresholdDist = 42.0f; // if firstCorePasser is within this distance of the closestTrigger, go to nearTriggerDist short of the closestTrigger
    const float nearTriggerDist = 2.0f;
    const float farDistance = 38.0f;  // if firstCorePasser is not thresholdDist yards from the closestTrigger, go to farDistance from the firstCorePasser

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = fx + dx * moveDist;
        targetY = fy + dy * moveDist;
        targetZ = 42.985f;
    }
    else
    {
        targetX = fx + dx * farDistance;
        targetY = fy + dy * farDistance;
        targetZ = 42.985f;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger)
{
    // Don't move too early; wait until the second passer has the core
    if (!secondCorePasser->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        return false;

    if (secondCorePasser->GetExactDist2d(closestTrigger) <= 2.0f)
        return false;

    float sx = secondCorePasser->GetPositionX();
    float sy = secondCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - sx;
    float dy = closestTrigger->GetPositionY() - sy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float targetX, targetY, targetZ;
    const float thresholdDist = 42.0f;
    const float nearTriggerDist = 2.0f;
    const float farDistance = 38.0f;

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = sx + dx * moveDist;
        targetY = sy + dy * moveDist;
        targetZ = 42.985f;
    }
    else
    {
        targetX = sx + dx * farDistance;
        targetY = sy + dy * farDistance;
        targetZ = 42.985f;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(Player* thirdCorePasser, Unit* closestTrigger)
{
    // Don't move too early; wait until the second passer has the core
    // This also means that the fourth passer often does not need to move at all because usually the
    // second passer is close enough to use the generator
    if (!thirdCorePasser->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        return false;

    if (thirdCorePasser->GetExactDist2d(closestTrigger) <= 2.0f)
        return false;

    float sx = thirdCorePasser->GetPositionX();
    float sy = thirdCorePasser->GetPositionY();

    float tx = closestTrigger->GetPositionX();
    float ty = closestTrigger->GetPositionY();

    float dx = tx - sx;
    float dy = ty - sy;
    float length = std::sqrt(dx*dx + dy*dy);

    if (length == 0.0f)
        return false;

    dx /= length; dy /= length;

    float targetX = tx - dx * 2.0f;
    float targetY = ty - dy * 2.0f;
    const float targetZ = 42.985f;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInIntendedPosition(Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(firstCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = firstCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    // Fallback
    /* const float centerX = VashjPlatformCenterPosition.GetPositionX();
    const float centerY = VashjPlatformCenterPosition.GetPositionY();
    const float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * cos(angle);
    float targetY = centerY + radius * sin(angle);

    float dist = firstCorePasser->GetExactDist2d(Position(targetX, targetY));

    return dist <= 2.0f; */

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInIntendedPosition(Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(secondCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = secondCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    // Fallback
    /* float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float moveDist = std::max(distToTrigger - 2.0f, 0.0f);
    float pos1X = fx + dx * moveDist;
    float pos1Y = fy + dy * moveDist;

    float pos2X = fx + dx * 38.0f;
    float pos2Y = fy + dy * 38.0f;

    float dist1 = secondCorePasser->GetExactDist2d(Position(pos1X, pos1Y));
    float dist2 = secondCorePasser->GetExactDist2d(Position(pos2X, pos2Y));

    return dist1 <= 2.0f || dist2 <= 2.0f; */

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInIntendedPosition(Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(thirdCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = thirdCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    // Fallback
    /* float sx = secondCorePasser->GetPositionX();
    float sy = secondCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - sx;
    float dy = closestTrigger->GetPositionY() - sy;
    float distToTrigger = std::sqrt(dx*dx + dy*dy);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float moveDist = std::max(distToTrigger - 2.0f, 0.0f);
    float pos1X = sx + dx * moveDist;
    float pos1Y = sy + dy * moveDist;

    float pos2X = sx + dx * 38.0f;
    float pos2Y = sy + dy * 38.0f;

    float dist1 = thirdCorePasser->GetExactDist2d(Position(pos1X, pos1Y));
    float dist2 = thirdCorePasser->GetExactDist2d(Position(pos2X, pos2Y));

    return dist1 <= 2.0f || dist2 <= 2.0f; */

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInIntendedPosition(Player* thirdCorePasser, Player* fourthCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(fourthCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = fourthCorePasser->GetExactDist2d(itSnap->second.GetPositionX(), itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    // Fallback: simple proximity to trigger
    /* float distToTrigger2d = fourthCorePasser->GetExactDist2d(closestTrigger);
    return distToTrigger2d <= 2.0f; */

    return false;
}

void LadyVashjPassTheTaintedCoreAction::ScheduleStoreCoreAfterImbue(PlayerbotAI* botAI, Player* giver, Player* receiver)
{
    if (!receiver)
        return;

    // Give the client-side visual time to play and avoid races with other bots
    const uint32 delayMs = 1500;

    const ObjectGuid giverGuid    = giver ? giver->GetGUID() : ObjectGuid::Empty;
    const ObjectGuid receiverGuid = receiver->GetGUID();

    // Reserve pending state now so other bots skip issuing duplicate visuals while fallback is scheduled
    if (!giverGuid.IsEmpty())
    {
        if (!imbuePending.count(giverGuid))
            imbuePending.insert(giverGuid);
    }

    botAI->AddTimedEvent([botAI, giverGuid, receiverGuid]()
    {
        Player* receiverPlayer = receiverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(receiverGuid);
        Player* giverPlayer    = giverGuid.IsEmpty()    ? nullptr : ObjectAccessor::FindPlayer(giverGuid);

        if (!receiverPlayer)
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
            imbuePending.erase(giverGuid);
            return;
        }

        // Detect if anyone already has the core
        if (Group* group = receiverPlayer->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();

                if (!member)
                    continue;

                if (member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                {
                    intendedLineup.erase(receiverGuid);
                    intendedLineup.erase(giverGuid);
                    imbuePending.erase(giverGuid);
                    return;
                }
            }
        }

        if (receiverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
            imbuePending.erase(giverGuid);
            return;
        }

        // Store a new core into receiver inventory (sends client/db update)
        ItemPosCountVec dest;
        uint32 count = 1;
        int canStore = receiverPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);

        if (canStore == EQUIP_ERR_OK)
        {
            Item* created = receiverPlayer->StoreNewItem(dest, ITEM_TAINTED_CORE, true, Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            if (created)
            {
                // start cooldown only after actual store succeeded
                time_t now = std::time(nullptr);
                lastImbueAttempt[giverGuid] = now;
                intendedLineup.erase(receiverGuid);
                intendedLineup.erase(giverGuid);
                imbuePending.erase(giverGuid);
            }
        }
        else
        {
            intendedLineup.erase(receiverGuid);
            intendedLineup.erase(giverGuid);
            imbuePending.erase(giverGuid);
        }
    }, delayMs);
}

void LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
        return;

    float dist = bot->GetExactDist2d(generator);
    if (dist > 3.0f)
        return;

    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        const uint8 bagIndex = core->GetBagSlot();
        const uint8 slot = core->GetSlot();
        const uint8 cast_count = 0;
        uint32 spellId = 0;

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (core->GetTemplate()->Spells[i].SpellId > 0)
            {
                spellId = core->GetTemplate()->Spells[i].SpellId;
                break;
            }
        }

        const ObjectGuid item_guid = core->GetGUID();
        const uint32 glyphIndex = 0;
        const uint8 castFlags = 0;

        WorldPacket packet(CMSG_USE_ITEM);
        packet << bagIndex;
        packet << slot;
        packet << cast_count;
        packet << spellId;
        packet << item_guid;
        packet << glyphIndex;
        packet << castFlags;
        packet << (uint32)TARGET_FLAG_GAMEOBJECT;
        packet << generator->GetGUID().WriteAsPacked();

        // Process server-side immediately so GO script runs with item context
        bot->GetSession()->HandleUseItemOpcode(packet);
    }
}

bool LadyVashjDestroyTaintedCoreAction::Execute(Event event)
{
    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        bot->DestroyItem(core->GetBagSlot(), core->GetSlot(), true);
        return true;
    }

    return false;
}

bool LadyVashjAvoidToxicSporesAction::Execute(Event event)
{
    std::vector<Unit*> spores = GetAllSporeDropTriggers(botAI, bot);
    if (spores.empty())
        return false;

    const float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (bot->GetExactDist2d(spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& vashjCenter = VashjPlatformCenterPosition;
    const float maxRadius = 70.0f;

    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);

    return MoveTo(bot->GetMapId(), safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(const std::vector<Unit*>& spores,
    const Position& vashjCenter, float maxRadius, float hazardRadius)
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

            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            Position testPos(x, y, z);

            bool isSafe = true;
            for (Unit* spore : spores)
            {
                if (spore->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            bool pathSafe = IsPathSafeFromSpores(bot->GetPosition(), testPos, spores, hazardRadius);
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

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(const Position& start,
    const Position& end, const std::vector<Unit*>& spores, float hazardRadius)
{
    const int numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (int i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* spore : spores)
        {
            float distToSpore = spore->GetExactDist2d(checkX, checkY);
            if (distToSpore < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        const float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SPORE_DROP_TRIGGER && bot->GetExactDist2d(unit) < maxSearchRadius)
            sporeDropTriggers.push_back(unit);
    }

    return sporeDropTriggers;
}

bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: enter for bot={}", bot->GetName());

    if (bot->HasAura(SPELL_ENTANGLE) &&
        (bot->HasAura(SPELL_TOXIC_SPORES) || bot->HasAura(SPELL_STATIC_CHARGE)))
    {
        LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: self entangled with spores/static for bot={} (toxic_spores={} static_charge={})",
                  bot->GetName(), bot->HasAura(SPELL_TOXIC_SPORES), bot->HasAura(SPELL_STATIC_CHARGE));
        if (bot->getClass() == CLASS_ROGUE && botAI->CanCastSpell("cloak of shadows", bot))
        {
            LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: attempting Cloak of Shadows for bot={}", bot->GetName());
            bool castRes = botAI->CastSpell("cloak of shadows", bot);
            LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: Cloak of Shadows result for bot={} -> {}", bot->GetName(), castRes);
            return castRes;
        }
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!member->HasAura(SPELL_ENTANGLE))
            continue;

        LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: found entangled member={} for bot={}", member->GetName(), bot->GetName());

        // Priority 1: member entangled + static charge -> freedom
        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: member {} has STATIC_CHARGE; checking hand of freedom for bot={}", member->GetName(), bot->GetName());
            if (bot->getClass() == CLASS_PALADIN && botAI->CanCastSpell("hand of freedom", member))
            {
                LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: attempting Hand of Freedom on member={} by bot={}", member->GetName(), bot->GetName());
                bool castRes = botAI->CastSpell("hand of freedom", member);
                LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: Hand of Freedom result on member={} by bot={} -> {}", member->GetName(), bot->GetName(), castRes);
                return castRes;
            }
            continue;
        }

        // Priority 2: main tank entangled + toxic spores -> freedom
        if (member->HasAura(SPELL_TOXIC_SPORES) && botAI->IsMainTank(member))
        {
            LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: member {} is main tank and has TOXIC_SPORES; checking hand of freedom for bot={}", member->GetName(), bot->GetName());
            if (bot->getClass() == CLASS_PALADIN && botAI->CanCastSpell("hand of freedom", member))
            {
                LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: attempting Hand of Freedom on main tank={} by bot={}", member->GetName(), bot->GetName());
                bool castRes = botAI->CastSpell("hand of freedom", member);
                LOG_DEBUG("playerbots", "LadyVashjUseFreeActionAbilitiesAction: Hand of Freedom result on main tank={} by bot={} -> {}", member->GetName(), bot->GetName(), castRes);
                return castRes;
            }
        }
    }

    return false;
}

bool LadyVashjManageTrackersAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    vashjRangedPositions.clear();
    vashjHasReachedRangedPosition.clear();
    lastImbueAttempt.clear();
    imbuePending.clear();
    intendedLineup.clear();

    return false;
}
