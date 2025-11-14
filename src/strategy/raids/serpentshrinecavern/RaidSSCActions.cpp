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

#include <chrono> // For testing purposes

using namespace SerpentShrineCavernHelpers;
using namespace SerpentShrineCavernPositions;

// Trash Mobs

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

    if (bot->GetExactDist2d(mushroom) < 10.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveAway(mushroom, 12.0f, false);
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

        if (hydross->GetVictim() == bot)
        {
            const Position& position = HydrossFrostTankPosition;
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
            else if (!bot->IsWithinMeleeRange(hydross))
            {
                return MoveTo(hydross->GetMapId(), hydross->GetPositionX(),
                              hydross->GetPositionY(), hydross->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfHydrossAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        uint32 mapId = hydross->GetMapId();
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToNaturePhaseTimer.find(mapId);

        if (it != hydrossChangeToNaturePhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossNatureTankPosition;
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

        if (hydross->GetVictim() == bot)
        {
            const Position& position = HydrossNatureTankPosition;
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
            else if (!bot->IsWithinMeleeRange(hydross))
            {
                return MoveTo(hydross->GetMapId(), hydross->GetPositionX(),
                            hydross->GetPositionY(), hydross->GetPositionZ(),
                            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfCorruptionAt100Percent(bot) && hydross->GetVictim() == bot)
    {
        uint32 mapId = hydross->GetMapId();
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToFrostPhaseTimer.find(mapId);

        if (it != hydrossChangeToFrostPhaseTimer.end() && (now - it->second) >= 5)
        {
            const Position& position = HydrossFrostTankPosition;
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

bool HydrossTheUnstableMarkElementalAddsAction::Execute(Event event)
{
    Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS);
    if (waterElemental)
        MarkTargetWithSkull(bot, waterElemental);

    Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS);
    if (natureElemental)
        MarkTargetWithSkull(bot, natureElemental);

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

    Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS);
    if (waterElemental && !hydross->HasAura(SPELL_CORRUPTION) && frostTank)
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

    Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS);
    if (natureElemental && hydross->HasAura(SPELL_CORRUPTION) && natureTank)
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

    uint32 mapId = hydross->GetMapId();
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

    uint32 mapId = hydross->GetMapId();
    const time_t now = std::time(nullptr);

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        if (hydrossFrostDpsWaitTimer.count(mapId))
            hydrossFrostDpsWaitTimer.erase(mapId);

        if (hydrossNatureDpsWaitTimer.count(mapId))
            hydrossNatureDpsWaitTimer.erase(mapId);

        if (hydrossChangeToFrostPhaseTimer.count(mapId))
            hydrossChangeToFrostPhaseTimer.erase(mapId);

        if (hydrossChangeToNaturePhaseTimer.count(mapId))
            hydrossChangeToNaturePhaseTimer.erase(mapId);
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossFrostDpsWaitTimer.count(mapId) == 0)
            hydrossFrostDpsWaitTimer[mapId] = now;

        if (hydrossNatureDpsWaitTimer.count(mapId))
            hydrossNatureDpsWaitTimer.erase(mapId);

        if (hydrossChangeToFrostPhaseTimer.count(mapId))
            hydrossChangeToFrostPhaseTimer.erase(mapId);

        if (HasMarkOfHydrossAt100Percent(bot) &&
            hydrossChangeToNaturePhaseTimer.count(mapId) == 0)
            hydrossChangeToNaturePhaseTimer[mapId] = now;
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossNatureDpsWaitTimer.count(mapId) == 0)
            hydrossNatureDpsWaitTimer[mapId] = now;

        if (hydrossFrostDpsWaitTimer.count(mapId))
            hydrossFrostDpsWaitTimer.erase(mapId);

        if (hydrossChangeToNaturePhaseTimer.count(mapId))
            hydrossChangeToNaturePhaseTimer.erase(mapId);

        if (HasMarkOfCorruptionAt100Percent(bot) &&
            hydrossChangeToFrostPhaseTimer.count(mapId) == 0)
            hydrossChangeToFrostPhaseTimer[mapId] = now;
    }

    return false;
}

// The Lurker Below

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

bool TheLurkerBelowPositionOtherMeleeAction::Execute(Event event)
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

bool TheLurkerBelowMeleeRunAroundBehindBossAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    // Get Lurker's facing direction (orientation)
    float bossFacing = lurker->GetOrientation();

    // Pick an angle behind Lurker (120-degree arc)
    float behindAngle = bossFacing + M_PI + ((rand() % 100) / 100.0f - 0.5f) * (M_PI / 3);

    // Random radius between 20 and 25 yards
    float radius = 20.0f + ((rand() % 500) / 100.0f);

    // Calculate target position on the circle
    float targetX = lurker->GetPositionX() + radius * cos(behindAngle);
    float targetY = lurker->GetPositionY() + radius * sin(behindAngle);
    float targetZ = lurker->GetPositionZ();

    // Only move if not close enough to the circle position
    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, lurker->GetPositionX(), lurker->GetPositionY(),
                                                            lurker->GetPositionZ(), targetX, targetY, targetZ))
            return false;

        return MoveTo(lurker->GetMapId(), targetX, targetY, targetZ, false, false, false, false,
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
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    const time_t now = std::time(nullptr);

    // Log current timer value
    LOG_DEBUG("playerbots", "SpoutTimerAction: lurkerSpoutTimer[{}]={}, now={}", mapId, lurkerSpoutTimer.count(mapId) ? lurkerSpoutTimer[mapId] : -1, now);

    // Set timer if Spout starts
    if (IsLurkerCastingSpout(lurker) && (lurkerSpoutTimer.count(mapId) == 0 || lurkerSpoutTimer[mapId] <= now)) {
        lurkerSpoutTimer[mapId] = now + 20; // 20s channel for Spout
        LOG_DEBUG("playerbots", "SpoutTimerAction: Set lurkerSpoutTimer[{}] to {}", mapId, lurkerSpoutTimer[mapId]);
    }

    // Erase timer if expired
    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] <= now) {
        LOG_DEBUG("playerbots", "SpoutTimerAction: Erasing expired lurkerSpoutTimer[{}]", mapId);
        lurkerSpoutTimer.erase(mapId);
    }

    // Erase timer if boss is at full health
    if (lurker->GetHealth() == lurker->GetMaxHealth() && lurkerSpoutTimer.count(mapId))
        lurkerSpoutTimer.erase(mapId);

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

        if (fabs(bot->GetExactDist2d(leotherasDemon) - (maxMeleeRange - meleeRangeBuffer)))
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

    if (leotherasHuman && bot->GetExactDist2d(leotherasHuman) < 14.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(leotherasHuman, 15.0f, false);
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

        if (fabs(bot->GetExactDist2d(targetX, targetY) - (maxMeleeRange - meleeRangeBuffer)))
        {
            return MoveTo(leotherasPhase2Demon->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    if (!botAI->IsTank(bot) && leotherasPhase3Demon && leotherasPhase3Demon->GetVictim() != bot &&
        bot->GetExactDist2d(leotherasPhase3Demon) < 8.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(leotherasPhase3Demon, 10.0f, false);
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

// No whirlwind timer logic--too complicated to deal with right now
bool LeotherasTheBlindManageTimersAndTrackersAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    uint32 mapId = leotheras->GetMapId();
    const time_t now = std::time(nullptr);

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);

    if (leotheras && leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        if (leotherasHumanFormDpsWaitTimer.count(mapId))
            leotherasHumanFormDpsWaitTimer.erase(mapId);

        if (leotherasDemonFormDpsWaitTimer.count(mapId))
            leotherasDemonFormDpsWaitTimer.erase(mapId);

        if (leotherasFinalPhaseDpsWaitTimer.count(mapId))
            leotherasFinalPhaseDpsWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && !leotherasPhase3Demon)
    {
        if (leotherasHumanFormDpsWaitTimer.count(mapId) == 0)
            leotherasHumanFormDpsWaitTimer[mapId] = now;

        if (leotherasDemonFormDpsWaitTimer.count(mapId))
            leotherasDemonFormDpsWaitTimer.erase(mapId);

        if (leotherasFinalPhaseDpsWaitTimer.count(mapId))
            leotherasFinalPhaseDpsWaitTimer.erase(mapId);
    }
    else if (leotherasPhase2Demon)
    {
        if (leotherasDemonFormDpsWaitTimer.count(mapId) == 0)
            leotherasDemonFormDpsWaitTimer[mapId] = now;

        if (leotherasHumanFormDpsWaitTimer.count(mapId))
            leotherasHumanFormDpsWaitTimer.erase(mapId);

        if (leotherasFinalPhaseDpsWaitTimer.count(mapId))
            leotherasFinalPhaseDpsWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        if (leotherasFinalPhaseDpsWaitTimer.count(mapId) == 0)
            leotherasFinalPhaseDpsWaitTimer[mapId] = now;

        if (leotherasHumanFormDpsWaitTimer.count(mapId))
            leotherasHumanFormDpsWaitTimer.erase(mapId);

        if (leotherasDemonFormDpsWaitTimer.count(mapId))
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

    if (karathress->GetVictim() == bot)
    {
        const Position& position = KarathressTankPosition;
        if (!bot->IsWithinMeleeRange(karathress))
        {
            return MoveTo(karathress->GetMapId(), karathress->GetPositionX(),
                          karathress->GetPositionY(), karathress->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
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

    if (sharkkis->GetVictim() == bot)
    {
        const Position& position = SharkkisTankPosition;
        if (!bot->IsWithinMeleeRange(sharkkis))
        {
            return MoveTo(sharkkis->GetMapId(), sharkkis->GetPositionX(),
                          sharkkis->GetPositionY(), sharkkis->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
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

    if (tidalvess->GetVictim() == bot)
    {
        const Position& position = TidalvessTankPosition;
        if (!bot->IsWithinMeleeRange(tidalvess))
        {
            return MoveTo(tidalvess->GetMapId(), tidalvess->GetPositionX(),
                          tidalvess->GetPositionY(), tidalvess->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
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
    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        // 3rd assist tank of index (index 2)
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

    // Target priority 3a: Caribdis for ranged
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

    // Target priority 3b: Sharkkis for melee (and ranged if Caribdis down first)
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

    // Target priority 4: Sharkkis pets for all dps
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

    // Target priority 5: Karathress for all dps
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

    uint32 mapId = karathress->GetMapId();
    const time_t now = std::time(nullptr);

    if (karathress->GetHealth() == karathress->GetMaxHealth())
    {
        if (karathressDpsWaitTimer.count(mapId))
            karathressDpsWaitTimer.erase(mapId);
    }
    else
    {
        if (karathressDpsWaitTimer.count(mapId) == 0)
            karathressDpsWaitTimer[mapId] = now;
    }

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

    if (tidewalker->GetVictim() == bot)
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
    else if (!bot->IsWithinMeleeRange(tidewalker))
    {
        return MoveTo(tidewalker->GetMapId(), tidewalker->GetPositionX(),
                      tidewalker->GetPositionY(), tidewalker->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition(Unit* tidewalker)
{
    const Position& phase2 = TidewalkerPhase2TankPosition;
    const Position& transition = TidewalkerPhaseTransitionWaypoint;

    ObjectGuid botGuid = bot->GetGUID();
    uint8 step = tidewalkerTankStep.count(botGuid) ? tidewalkerTankStep[botGuid] : 0;

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
            tidewalkerTankStep[botGuid] = 1;
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

    ObjectGuid botGuid = bot->GetGUID();
    uint8 step = tidewalkerRangedStep.count(botGuid) ? tidewalkerRangedStep[botGuid] : 0;

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
            tidewalkerRangedStep[botGuid] = 1;
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

    ObjectGuid botGuid = bot->GetGUID();

    if (tidewalkerTankStep.count(botGuid))
        tidewalkerTankStep.erase(botGuid);

    if (tidewalkerRangedStep.count(botGuid))
        tidewalkerRangedStep.erase(botGuid);

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

    if (vashj->GetVictim() == bot)
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
            else if (!bot->IsWithinMeleeRange(vashj))
            {
                return MoveTo(vashj->GetMapId(), vashj->GetPositionX(),
                            vashj->GetPositionY(), vashj->GetPositionZ(),
                            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        if (IsLadyVashjInPhase3(botAI))
        {
            Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
            if (enchanted && bot->GetExactDist2d(enchanted) < 10.0f)
            {
                return MoveAway(enchanted, 10.0f, false);
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

    // Collect all ranged and healer bots
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

    // Assign initial positions if not already assigned
    if (!vashjRangedPositions.count(bot->GetGUID()))
    {
        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        uint8 botIndex = (it != spreadMembers.end()) ? std::distance(spreadMembers.begin(), it) : 0;
        uint8 count = spreadMembers.size();

        float referenceAngle = M_PI / 2.0f; // π/2 radians = north
        const float span = M_PI; // half circle
        const float startAngle = referenceAngle - span / 2.0f;
        float angle;
        if (count <= 1)
            angle = referenceAngle;
        else
            angle = startAngle + (float)botIndex / (count - 1) * span;

        uint32 botSeed = bot->GetGUID().GetCounter();
        float radius = minSpreadRadius + (botSeed % 1000) / 1000.0f * (maxSpreadRadius - minSpreadRadius);
        float targetX = center.GetPositionX() + radius * cos(angle);
        float targetY = center.GetPositionY() + radius * sin(angle);
        vashjRangedPositions[bot->GetGUID()] = Position(targetX, targetY, center.GetPositionZ());
        vashjHasReachedRangedPosition[bot->GetGUID()] = false;
     }

    Position targetPosition = vashjRangedPositions[bot->GetGUID()];
    if (!vashjHasReachedRangedPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return false;

            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        vashjHasReachedRangedPosition[bot->GetGUID()] = true;
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
        botAI->ChangeStrategy("grounding totem", BotState::BOT_STATE_COMBAT);

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

    // Find main tank with static charge
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
        if (bot->GetExactDist2d(mainTank) < 11.0f)
            return MoveAway(mainTank, 11.5f, false);
    }

    // Otherwise, if bot has static charge, move away from other group members
    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < 11.0f)
                return MoveAway(member, 11.5f, false);
        }
    }

    return false;
}

bool LadyVashjAttackAndMoveAwayFromStriderAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    if (!vashj || !strider)
        return false;

    if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsTank(bot))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD_CHEAT))
            bot->AddAura(SPELL_FEAR_WARD_CHEAT, bot);

        if (bot->GetVictim() == strider && bot->GetExactDist2d(vashj) < 30.0f)
            return MoveAway(vashj, 32.0f, false);
    }

    // Don't move away only if raid cheats are enabled AND bot is a tank
    if (bot->GetExactDist2d(strider) < 15.0f &&
        (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsTank(bot)))
        return MoveAway(strider, 16.0f, false);

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

    // Get all nearby hostile NPCs
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
        LOG_DEBUG("playerbots", "Phase2DpsPriorityAction: Stopping attack {} due to Vashj still being victim", bot->GetName());
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    // Role-based search / pursue limits
    const Position& center = VashjPlatformCenterPosition;
    const float maxSearchRange = botAI->IsRangedDps(bot) ? 60.0f : (botAI->IsMelee(bot) ? 55.0f : 40.0f);
    const float maxPursueRange = maxSearchRange - 5.0f; // won't initiate attack beyond this

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidPhase2CombatNpc(unit, botAI))
            continue;

        // skip NPCs outside the encounter search radius (prevents chasing far adds/triggers)
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

    // Set priorities
    std::vector<Unit*> targets;
    if (IsLadyVashjInPhase2(botAI))
    {
        if (botAI->IsRanged(bot))
        {
            // uint8 tab = AiFactory::GetPlayerSpecTab(bot);
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { tainted, enchanted, strider, elite };
            else
                targets = { tainted, strider, elite, enchanted };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { tainted, enchanted, elite };
        else if (botAI->IsTank(bot))
        {
            // With raid cheats enabled, first assist tank should prioritize the strider first.
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
                targets = { vashj };
            else if (botAI->IsAssistTankOfIndex(bot, 0))
            {
                if (botAI->HasCheat(BotCheatMask::raid))
                    targets = { strider, enchanted, vashj };
                if (!botAI->HasCheat(BotCheatMask::raid))
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

    // Pick the first valid target
    for (Unit* t : targets)
    {
        if (t && t->IsAlive())
        {
            target = t;
            LOG_DEBUG("playerbots", "Phase2DpsPriorityAction: Picked target {} (entry: {}) for {}", t->GetName(), t->GetEntry(), bot->GetName());
            break;
        }
    }

    // If already targeting the same valid target, do nothing
    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    if (target && currentTarget == target && IsValidPhase2CombatNpc(currentTarget, botAI))
    {
        LOG_DEBUG("playerbots", "No action: current target equals chosen target for {} (target entry: {}, is player: {})",
            bot->GetName(),
            currentTarget ? currentTarget->GetEntry() : 0,
            currentTarget ? currentTarget->IsPlayer() : false);
        return false;
    }

    // Wonder if combine target and victim conditions?
    if (target && bot->GetExactDist2d(target) <= maxPursueRange && bot->GetTarget() != target->GetGUID())
    {
        bot->SetTarget(target->GetGUID());
        LOG_DEBUG("playerbots", "Attacking chosen target {} for {} (dist={})", target->GetName(), bot->GetName(), bot->GetExactDist2d(target));
        return Attack(target);
    }

    // Minimal cleanup: clear invalid current target to avoid null==null early-return issues
    if (currentTarget && (!currentTarget->IsAlive() || !IsValidPhase2CombatNpc(currentTarget, botAI)))
    {
        LOG_DEBUG("playerbots", "Clearing invalid current target for {}: {}", bot->GetName(),
                  currentTarget ? currentTarget->GetName() : std::string("null"));
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    if (botAI->HasCheat(BotCheatMask::raid) && !bot->GetVictim())
    {
        Player* master = botAI->GetMaster();
        Player* designatedLooter = GetDesignatedCoreLooter(bot->GetGroup(), master, botAI);
        if (designatedLooter && designatedLooter == bot && tainted && designatedLooter->GetExactDist2d(tainted) < 10.0f)
            return false;

        const Position& center = VashjPlatformCenterPosition;
        if (bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 35.0f)
        {
            LOG_DEBUG("playerbots", "LadyVashjCheat: teleporting/moving {} back to center ({}, {})", bot->GetName(), center.GetPositionX(), center.GetPositionY());
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);

            /* if (botAI->HasCheat(BotCheatMask::raid))
            {
                bot->TeleportTo(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(), bot->GetOrientation());
                return false;
            }
            else
            {
                return MoveTo(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            } */
            return MoveInside(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(), 30.0f,
                              MovementPriority::MOVEMENT_COMBAT);
        }
    }
    LOG_DEBUG("playerbots", "Phase2DpsPriorityAction: End of action for {}", bot->GetName());
    return false;
}

// Only if cheats are off, intended to support core handling
bool LadyVashjAssistantsFollowMasterInPhase2Action::Execute(Event event)
{
    Player* master = botAI->GetMaster();
    if (!master || master == bot)
        return false;

    if (bot->GetExactDist2d(master) > 18.0f)
        return MoveTo(master, 15.0f, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool LadyVashjTeleportToTaintedElementalAction::Execute(Event event)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    /* Item* coreCheck = bot->GetItemByEntry(ITEM_TAINTED_CORE);

    // Check for despawn even if tainted is nullptr
    if (lastTaintedGuid && !botAI->GetUnit(lastTaintedGuid) && !coreCheck)
    {
        LOG_DEBUG("playerbots", "TaintedElementalCheat: Elemental despawned, treating as dead for core logic");
        ItemPosCountVec dest;
        uint32 count = 1;
        int canStore = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);
        LOG_DEBUG("playerbots", "TaintedElementalCheat: {} CanStoreNewItem -> {}", bot->GetName(), canStore);

        if (canStore == EQUIP_ERR_OK)
        {
            Item* created = bot->StoreNewItem(dest, ITEM_TAINTED_CORE, true, Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            if (created)
            {
                LOG_DEBUG("playerbots", "TaintedElementalCheat: {} created core (guid={})", bot->GetName(), created->GetGUID().ToString());
                ScheduleCoreReconcile(botAI, bot, bot, 500);
            }
        }
        lastTaintedGuid = ObjectGuid::Empty;
        return true;
    } */

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

    if (bot->GetExactDist2d(tainted) < 10.0f)
    {
        bot->SetFullHealth();
        bot->RemoveAura(SPELL_POISON_BOLT);
    }

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event)
{
    LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: Execute start for bot {}", bot->GetName());

    GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig->lootDistance;

    for (auto const& guid : corpses)
    {
        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: inspecting guid {}", guid.ToString());

        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
        {
            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: loot not possible on guid {}", guid.ToString());
            continue;
        }

        WorldObject* obj = loot.GetWorldObject(bot);
        if (!obj)
        {
            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: no world object for guid {}", guid.ToString());
            continue;
        }

        // Expect the dead tainted elemental to be exposed as a Creature object
        Creature* cr = obj->ToCreature();
        if (!cr)
        {
            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: world object for guid {} is not a creature, skipping", guid.ToString());
            continue;
        }

        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: found CREATURE guid={} entry={} hp={}/{} alive={}",
                  guid.ToString(), cr->GetEntry(), cr->GetHealth(), cr->GetMaxHealth(), cr->IsAlive());

        // Only consider dead tainted elemental creatures
        if (cr->GetEntry() != NPC_TAINTED_ELEMENTAL || cr->IsAlive())
        {
            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: creature {} is not a dead tainted elemental, skipping", guid.ToString());
            continue;
        }

        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: found dead tainted elemental target {}", guid.ToString());

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(obj);
        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: target {} dist={}", guid.ToString(), dist);

        if (dist > maxLootRange)
        {
            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: moving to target {} (dist={} > maxLootRange={})", guid.ToString(), dist, maxLootRange);
            return MoveTo(obj, 2.0f, MovementPriority::MOVEMENT_FORCED);
        }

        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: in range of target {}, invoking OpenLootAction", guid.ToString());

        // Invoke OpenLootAction to request the server's StoreLoot packet for this corpse.
        // We'll attempt a forced autostore from SSC (without modifying LootAction) by
        // scheduling a short-timer to send CMSG_AUTOSTORE_LOOT_ITEM (index 0) once the
        // server has had time to send the StoreLoot packet.
        OpenLootAction open(botAI);
        bool opened = open.Execute(Event());
        LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: OpenLootAction returned {}", opened);

        // If OpenLootAction failed, nothing more to do for this corpse
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
        ObjectGuid botGuid = bot->GetGUID();
        ObjectGuid corpseGuid = guid;
        const uint8 guessedIndex = 0; // best-effort guess (most single-item corpses use index 0)

        botAI->AddTimedEvent([this, botGuid, corpseGuid, guessedIndex]()
        {
            Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver || !receiver->IsInWorld())
                return;

            // Double-check someone else didn't obtain the core in the meantime
            if (Group* group = receiver->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                        return;
                }
            }

            // Ensure we still don't already have it.
            if (receiver->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                return;

            // Set the loot GUID so server treats the following autostore as targeted to this corpse
            receiver->SetLootGUID(corpseGuid);

            LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: SSC autostore sending CMSG_AUTOSTORE_LOOT_ITEM guessedIndex={} for bot={} corpse={}",
                      guessedIndex, receiver->GetName(), corpseGuid.ToString());

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << guessedIndex;
            receiver->GetSession()->QueuePacket(packet);
        }, 600);

        return true;
    }

    LOG_DEBUG("playerbots", "LadyVashjLootTaintedCoreAction: no matching dead tainted target found, returning false");
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
    Unit* closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(bot, designatedLooter);

    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: roles - designatedLooter={} first={} second={} third={} closestTrigger={}",
              designatedLooter ? designatedLooter->GetName() : "null",
              firstCorePasser ? firstCorePasser->GetName() : "null",
              secondCorePasser ? secondCorePasser->GetName() : "null",
              thirdCorePasser ? thirdCorePasser->GetName() : "null",
              closestTrigger ? std::to_string(closestTrigger->GetEntry()) : std::string("null"));

    if (!firstCorePasser || !secondCorePasser || !thirdCorePasser || !closestTrigger)
        return false;

    // If this bot currently holds the core, don't perform any lineup/movement logic.
    // Still allow passing/using the core below.
    if (bot == firstCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
        LineUpFirstCorePasser(designatedLooter, closestTrigger);
    else if (bot == secondCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
        LineUpSecondCorePasser(firstCorePasser, closestTrigger);
    else if (bot == thirdCorePasser && !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
        LineUpThirdCorePasser(secondCorePasser, closestTrigger);

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    bool hasInInventory = botAI->HasItemInInventory(ITEM_TAINTED_CORE);
    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: item check for bot={} itemPtr={} inInventory={}", bot->GetName(),
              item ? item->GetGUID().ToString() : std::string("null"), hasInInventory);

    if (item && hasInInventory)
    {
        // Designated core looter logic--applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter)
        {
            LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: designated looter {} holding core; checking first passer position", bot->GetName());
            if (IsFirstCorePasserInIntendedPosition(designatedLooter, firstCorePasser, closestTrigger))
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: Imbue attempt from {} (item={}) -> {} (will still ensure with fallback)",
                          bot->GetName(), item ? item->GetGUID().ToString() : std::string("null"), firstCorePasser->GetName());

                const ObjectGuid giverGuid = bot->GetGUID();
                const time_t now = std::time(nullptr);
                auto it = lastImbueAttempt.find(giverGuid);
                bool imbueQueued = false;

                if (it == lastImbueAttempt.end() || (now - it->second) >= 3)
                {
                    botAI->ImbueItem(item, firstCorePasser);
                    lastImbueAttempt[giverGuid] = now;
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: ImbueItem queued from {} -> {}", bot->GetName(), firstCorePasser->GetName());
                    imbueQueued = true;
                }
                else
                {
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: skipping Imbue visual for {} (cooldown={}s)", bot->GetName(), now - it->second);
                }

                // guaranteed fallback to create/store the core after delay
                ScheduleStoreCoreAfterImbue(botAI, bot, firstCorePasser);
                if (imbueQueued)
                    return true;
            }
            else
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: first passer NOT in position yet for designated looter {}", bot->GetName());
            }
        }
        // First core passer logic
        else if (bot == firstCorePasser)
        {
            LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: first passer {} holding core; checking second passer position", bot->GetName());
            if (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger))
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: Imbue attempt from {} (item={}) -> {} (will still ensure with fallback)",
                          bot->GetName(), item ? item->GetGUID().ToString() : std::string("null"), secondCorePasser->GetName());

                const ObjectGuid giverGuid = bot->GetGUID();
                const time_t now = std::time(nullptr);
                auto it = lastImbueAttempt.find(giverGuid);
                bool imbueQueued = false;

                if (it == lastImbueAttempt.end() || (now - it->second) >= 3)
                {
                    botAI->ImbueItem(item, secondCorePasser);
                    lastImbueAttempt[giverGuid] = now;
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: ImbueItem queued from {} -> {}", bot->GetName(), secondCorePasser->GetName());
                    imbueQueued = true;
                }
                else
                {
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: skipping Imbue visual for {} (cooldown={}s)", bot->GetName(), now - it->second);
                }

                ScheduleStoreCoreAfterImbue(botAI, bot, secondCorePasser);
                if (imbueQueued)
                    return true;
            }
            else
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: second passer NOT in position yet for first passer {}", bot->GetName());
            }
        }
        // Second core passer logic
        else if (bot == secondCorePasser)
        {
            LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: second passer {} holding core; canUseGenerator={} thirdDistToTrigger={}",
                      bot->GetName(), CanUseGenerator(), thirdCorePasser ? thirdCorePasser->GetExactDist(closestTrigger) : -1.0f);

            if (CanUseGenerator())
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: second passer using generator now: {}", bot->GetName());
                UseCoreOnNearestGenerator();
            }
            else if (thirdCorePasser && thirdCorePasser->GetExactDist(closestTrigger) <= 4.0f)
            {
                const ObjectGuid giverGuid = bot->GetGUID();
                const time_t now = std::time(nullptr);
                auto it = lastImbueAttempt.find(giverGuid);
                bool imbueQueued = false;

                if (it == lastImbueAttempt.end() || (now - it->second) >= 3)
                {
                    botAI->ImbueItem(item, thirdCorePasser);
                    lastImbueAttempt[giverGuid] = now;
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: ImbueItem queued from {} -> {}", bot->GetName(), thirdCorePasser->GetName());
                    imbueQueued = true;
                }
                else
                {
                    LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: skipping Imbue visual for {} (cooldown={}s)", bot->GetName(), now - it->second);
                }

                ScheduleStoreCoreAfterImbue(botAI, bot, thirdCorePasser);
                if (imbueQueued)
                    return true;
            }
        }
        // Third core passer logic
        else if (bot == thirdCorePasser)
        {
            LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: third passer {} holding core; canUseGenerator={}", bot->GetName(), CanUseGenerator());
            if (CanUseGenerator())
            {
                LOG_DEBUG("playerbots", "LadyVashjPassTheTaintedCoreAction: third passer using generator now: {}", bot->GetName());
                UseCoreOnNearestGenerator();
            }
        }
    }

    return false;
}

void LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* designatedLooter, Unit* closestTrigger)
{
    const float centerX = VashjPlatformCenterPosition.GetPositionX();
    const float centerY = VashjPlatformCenterPosition.GetPositionY();
    const float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * cos(angle);
    float targetY = centerY + radius * sin(angle);
    float targetZ = 41.097f;

    LOG_DEBUG("playerbots", "LineUpFirstCorePasser: designatedLooter={} targetPos=({}, {}, {})", designatedLooter->GetName(), targetX, targetY, targetZ);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
           false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

void LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(Player* firstCorePasser, Unit* closestTrigger)
{
    float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();
    float fz = firstCorePasser->GetPositionZ();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float dz = closestTrigger->GetPositionZ() - fz;
    float distToTrigger = std::sqrt(dx*dx + dy*dy + dz*dz);

    LOG_DEBUG("playerbots", "LineUpSecondCorePasser: firstCorePasser={} closestTriggerEntry={} distToTrigger={}", firstCorePasser->GetName(), closestTrigger->GetEntry(), distToTrigger);

    if (distToTrigger == 0.0f)
        return;

    dx /= distToTrigger; dy /= distToTrigger; dz /= distToTrigger;

    float targetX, targetY, targetZ; // Target is on a line between firstCorePasser and closestTrigger
    const float thresholdDist = 42.0f; // if firstCorePasser is within this distance of the closestTrigger, go to nearTriggerDist short of the closestTrigger
    const float nearTriggerDist = 2.0f;
    const float farDistance = 39.0f;  // if firstCorePasser is not thresholdDist yards from the closestTrigger, go to farDistance from the firstCorePasser

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

    LOG_DEBUG("playerbots", "LineUpSecondCorePasser: moving bot={} to ({}, {}, {})", bot->GetName(), targetX, targetY, targetZ);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
           false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInIntendedPosition(Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger)
{
    const float centerX = VashjPlatformCenterPosition.GetPositionX();
    const float centerY = VashjPlatformCenterPosition.GetPositionY();
    const float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * cos(angle);
    float targetY = centerY + radius * sin(angle);
    float targetZ = 41.097f;

    float dist = firstCorePasser->GetExactDist(Position(targetX, targetY, targetZ));

    LOG_DEBUG("playerbots", "IsFirstCorePasserInIntendedPosition: designatedLooter={} firstCorePasser={} dist={}", designatedLooter->GetName(), firstCorePasser->GetName(), dist);

    return dist <= 2.0f;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInIntendedPosition(Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();
    float fz = firstCorePasser->GetPositionZ();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float dz = closestTrigger->GetPositionZ() - fz;
    float distToTrigger = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (distToTrigger == 0.0f)
    {
        LOG_DEBUG("playerbots", "IsSecondCorePasserInIntendedPosition: distToTrigger == 0 (firstCorePasser={})", firstCorePasser->GetName());
        return false;
    }

    dx /= distToTrigger; dy /= distToTrigger; dz /= distToTrigger;

    float moveDist = std::max(distToTrigger - 2.0f, 0.0f);
    float pos1X = fx + dx * moveDist;
    float pos1Y = fy + dy * moveDist;
    const float pos1Z = 42.985f;

    float pos2X = fx + dx * 39.0f;
    float pos2Y = fy + dy * 39.0f;
    const float pos2Z = 42.985f;

    float dist1 = secondCorePasser->GetExactDist(Position(pos1X, pos1Y, pos1Z));
    float dist2 = secondCorePasser->GetExactDist(Position(pos2X, pos2Y, pos2Z));

    LOG_DEBUG("playerbots", "IsSecondCorePasserInIntendedPosition: secondCorePasser={} dist1={} dist2={}", secondCorePasser->GetName(), dist1, dist2);

    return dist1 <= 2.0f || dist2 <= 0.5f;
}

void LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger)
{
    if (!secondCorePasser->HasItemCount(ITEM_TAINTED_CORE, 1, false))
    {
        LOG_DEBUG("playerbots", "LineUpThirdCorePasser: secondCorePasser {} does not hold tainted core, no move needed",
                  secondCorePasser ? secondCorePasser->GetName() : "null");
        return;
    }

    if (secondCorePasser->GetExactDist(closestTrigger) <= 4.0f)
    {
        LOG_DEBUG("playerbots", "LineUpThirdCorePasser: secondCorePasser already near trigger, no move needed");
        return;
    }

    float sx = secondCorePasser->GetPositionX();
    float sy = secondCorePasser->GetPositionY();
    float sz = secondCorePasser->GetPositionZ();

    float tx = closestTrigger->GetPositionX();
    float ty = closestTrigger->GetPositionY();
    float tz = closestTrigger->GetPositionZ();

    float dx = tx - sx;
    float dy = ty - sy;
    float dz = tz - sz;
    float length = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (length == 0.0f)
    {
        LOG_DEBUG("playerbots", "LineUpThirdCorePasser: length==0, aborting");
        return;
    }

    dx /= length; dy /= length; dz /= length;

    float targetX = tx - dx * 2.0f;
    float targetY = ty - dy * 2.0f;
    const float targetZ = 42.985f;

    LOG_DEBUG("playerbots", "LineUpThirdCorePasser: moving bot={} to ({}, {}, {}) to line up behind secondCorePasser={}", bot->GetName(), targetX, targetY, targetZ, secondCorePasser->GetName());

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

void LadyVashjPassTheTaintedCoreAction::ScheduleStoreCoreAfterImbue(PlayerbotAI* botAI, Player* giver, Player* receiver)
{
    if (!receiver)
        return;

    const uint32 delayMs = 1500;

    ObjectGuid giverGuid    = giver ? giver->GetGUID() : ObjectGuid::Empty;
    ObjectGuid receiverGuid = receiver->GetGUID();

    LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: scheduling create for {} in {} ms (giver={})",
              receiver ? receiver->GetName() : "null", delayMs, giver ? giver->GetName() : "null");

    botAI->AddTimedEvent([botAI, giverGuid, receiverGuid]() mutable
    {
        LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: timed lambda firing for receiverGuid={} giverGuid={}",
                receiverGuid.ToString(), giverGuid.ToString());

        Player* receiverPlayer = receiverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(receiverGuid);
        Player* giverPlayer    = giverGuid.IsEmpty()    ? nullptr : ObjectAccessor::FindPlayer(giverGuid);

        if (!receiverPlayer)
        {
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: receiver not found (nil)");
            return;
        }
        LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: resolved receiver name={} guid={} mapId={}",
                receiverPlayer->GetName(), receiverPlayer->GetGUID().ToString(), receiverPlayer->GetMapId());

        if (!receiverPlayer->IsInWorld())
        {
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: receiver found but not in world (name={})", receiverPlayer->GetName());
            return;
        }

        if (giverPlayer)
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: resolved giver name={} guid={} mapId={} inWorld={}",
                    giverPlayer->GetName(), giverPlayer->GetGUID().ToString(), giverPlayer->GetMapId(), giverPlayer->IsInWorld());
        else
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: giver not resolved (may be empty or left)");

        // Group scan: detect if anyone already has the core
        if (Group* group = receiverPlayer->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member) continue;
                if (member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                {
                    LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: abort - group member {} already has core", member->GetName());
                    return;
                }
            }
        }

        if (receiverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        {
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: abort - receiver already has core (name={})", receiverPlayer->GetName());
            return;
        }

        // Store a new core into receiver inventory (sends client/db update)
        ItemPosCountVec dest;
        uint32 count = 1;
        int canStore = receiverPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);

        LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: CanStoreNewItem for {} -> {}", receiverPlayer->GetName(), canStore);

        if (canStore == EQUIP_ERR_OK)
        {
            Item* created = receiverPlayer->StoreNewItem(dest, ITEM_TAINTED_CORE, true, Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            if (created && giverPlayer)
                LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: created core for {} (guid={})", receiverPlayer->GetName(), created->GetGUID().ToString());
                lastImbueAttempt.erase(giverPlayer->GetGUID());
        }
        else
        {
            LOG_DEBUG("playerbots", "ScheduleStoreCoreAfterImbue: cannot store core for {} (CanStoreNewItem -> {})", receiverPlayer->GetName(), canStore);
        }
    }, delayMs);
}

bool LadyVashjPassTheTaintedCoreAction::CanUseGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);

    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
    {
        LOG_DEBUG("playerbots", "CanUseGenerator: no nearby generator for bot={}", bot->GetName());
        return false;
    }

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
    {
        LOG_DEBUG("playerbots", "CanUseGenerator: generator object not found for guid {} for bot={}", nearestGen->guid.ToString(), bot->GetName());
        return false;
    }

    float dist = bot->GetExactDist(generator);
    LOG_DEBUG("playerbots", "CanUseGenerator: nearest generator guid={} dist={} bot={}", nearestGen->guid.ToString(), dist, bot->GetName());
    return dist <= 4.0f;
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
    {
        LOG_DEBUG("playerbots", "UseCoreOnNearestGenerator: no generator found for bot={}", bot->GetName());
        return false;
    }

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
    {
        LOG_DEBUG("playerbots", "UseCoreOnNearestGenerator: generator object missing for guid={} bot={}", nearestGen->guid.ToString(), bot->GetName());
        return false;
    }

    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        uint8 bagIndex = core->GetBagSlot();
        uint8 slot = core->GetSlot();
        uint8 cast_count = 0;
        uint32 spellId = 0;
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            if (core->GetTemplate()->Spells[i].SpellId > 0) { spellId = core->GetTemplate()->Spells[i].SpellId; break; }
        ObjectGuid item_guid = core->GetGUID();
        uint32 glyphIndex = 0;
        uint8 castFlags = 0;

        LOG_DEBUG("playerbots", "UseCoreOnNearestGenerator: using core for bot={} item_guid={} bag={} slot={} spellId={} generator={}",
                  bot->GetName(), item_guid.ToString(), bagIndex, slot, spellId, generator->GetGUID().ToString());

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
        LOG_DEBUG("playerbots", "UseCoreOnNearestGenerator: HandleUseItemOpcode called for bot={} generator={}", bot->GetName(), generator->GetGUID().ToString());
        return true;
    }
    else
    {
        LOG_DEBUG("playerbots", "UseCoreOnNearestGenerator: no core item found in bot inventory for bot={}", bot->GetName());
    }

    return false;
}

/* bool LadyVashjAvoidToxicSporesAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    auto sporeTriggers = GetAllSporeDropTriggers(botAI, bot);
    if (sporeTriggers.empty())
        return false;

    const float SAFETY_BUFFER = 5.0f; // tune (yards)
    const float ROOM_MAX_RADIUS = 55.0f; // don't go past this from room center

    // Choose the nearest trigger (prefer closest hazard)
    Unit* nearest = nullptr;
    float bestDist = std::numeric_limits<float>::max();
    for (Unit* t : sporeTriggers)
    {
        if (!t || !t->IsInWorld())
            continue;
        float d = bot->GetExactDist2d(t);
        if (d < bestDist)
        {
            bestDist = d;
            nearest = t;
        }
    }

    if (!nearest)
        return false;

    float radius = HAZARD_RADIUS + SAFETY_BUFFER;

    // If already outside hazard + buffer, don't move
    if (bot->GetDistance(nearest) > radius)
        return false;

    // conservative estimate: how far outward the bot must move to clear the hazard
    float curDistToHazard = bot->GetDistance(nearest);
    float requiredOutwardMove = std::max(0.0f, radius - curDistToHazard);

    // check room center limit
    Position roomCenterPos(VashjPlatformCenterPosition.x, VashjPlatformCenterPosition.y, VashjPlatformCenterPosition.z);
    float curDistFromCenter = bot->GetExactDist2d(roomCenterPos.GetPositionX(), roomCenterPos.GetPositionY());

    if (curDistFromCenter + requiredOutwardMove > ROOM_MAX_RADIUS)
    {
        // Too close to room edge; avoid fleeing farther out. Try other triggers but only if they don't push past limit.
        for (Unit* t : sporeTriggers)
        {
            if (!t || t == nearest)
                continue;
            float curDistToHaz = bot->GetDistance(t);
            float reqMove = std::max(0.0f, radius - curDistToHaz);
            if (curDistFromCenter + reqMove > ROOM_MAX_RADIUS)
                continue;
            Position c(t->GetPositionX(), t->GetPositionY(), t->GetPositionZ());
            if (FleePosition(c, radius))
                return true;
        }

        // If no acceptable trigger to flee from without exceeding room radius, don't flee.
        return false;
    }

    // Safe to flee from nearest trigger
    Position center(nearest->GetPositionX(), nearest->GetPositionY(), nearest->GetPositionZ());
    if (FleePosition(center, radius))
        return true;

    // Fallback: try other triggers (in case nearest had no valid flee spots)
    for (Unit* t : sporeTriggers)
    {
        if (!t || t == nearest)
            continue;
        float curDistToHaz = bot->GetDistance(t);
        float reqMove = std::max(0.0f, radius - curDistToHaz);
        if (curDistFromCenter + reqMove > ROOM_MAX_RADIUS)
            continue; // skip triggers that would push us past the room limit

        Position c(t->GetPositionX(), t->GetPositionY(), t->GetPositionZ());
        if (FleePosition(c, radius))
            return true;
    }

    return false;
} */

bool LadyVashjAvoidToxicSporesAction::Execute(Event event)
{
    // Get all spore triggers
    std::vector<Unit*> spores = GetAllSporeDropTriggers(botAI, bot);
    if (spores.empty())
        return false;

    // Check if bot is in danger
    const float hazardRadius = 7.0f; // Customizable hazard radius
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

    // Find safe position with minimal movement
    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);

    // Move to safe position
    return MoveTo(bot->GetMapId(), safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& spores,
    const Position& vashjCenter,
    float maxRadius,
    float hazardRadius)
{
    const float searchStep = M_PI / 8.0f; // 22.5 degree increments
    const float minDistance = 2.0f;
    const float maxDistance = 20.0f;
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

            // Check if within arena bounds
            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            Position testPos(x, y, z);

            // Check if position is safe from all spores
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

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(
    const Position& start,
    const Position& end,
    const std::vector<Unit*>& spores,
    float hazardRadius)
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

        // Check if this point is too close to any spore
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
        const float maxSearchRadius = 30.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SPORE_DROP_TRIGGER && bot->GetExactDist2d(unit) < maxSearchRadius)
            sporeDropTriggers.push_back(unit);
    }

    return sporeDropTriggers;
}

// Prelog version
/* bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->HasAura(SPELL_ENTANGLE) &&
        (bot->HasAura(SPELL_TOXIC_SPORES) || bot->HasAura(SPELL_STATIC_CHARGE)))
    {
        if (bot->getClass() == CLASS_ROGUE && botAI->CanCastSpell("cloak of shadows", bot))
            return botAI->CastSpell("cloak of shadows", bot);
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!member->HasAura(SPELL_ENTANGLE))
            continue;

        // Priority 1: member entangled + static charge -> freedom
        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            if (bot->getClass() == CLASS_PALADIN && botAI->CanCastSpell("hand of freedom", member))
                return botAI->CastSpell("hand of freedom", member);
            continue;
        }

        // Priority 2: main tank entangled + toxic spores -> freedom
        if (member->HasAura(SPELL_TOXIC_SPORES) && botAI->IsMainTank(member))
        {
            if (bot->getClass() == CLASS_PALADIN && botAI->CanCastSpell("hand of freedom", member))
                return botAI->CastSpell("hand of freedom", member);
        }
    }

    return false;
} */
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

    if (vashj->GetHealth() == vashj->GetMaxHealth() || IsLadyVashjInPhase2(botAI))
    {
        if (!vashjRangedPositions.empty())
            vashjRangedPositions.clear();

        if (!vashjHasReachedRangedPosition.empty())
            vashjHasReachedRangedPosition.clear();

        if (!lastImbueAttempt.empty())
            lastImbueAttempt.clear();
    }

    return false;
}

bool LadyVashjCheatToTestAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    static std::unordered_map<uint32, std::chrono::steady_clock::time_point> vashjFightStart;
    uint32 mapId = vashj->GetMapId();
    auto now = std::chrono::steady_clock::now();
    if (vashjFightStart.find(mapId) == vashjFightStart.end())
    {
        vashjFightStart[mapId] = now;
        LOG_DEBUG("playerbots", "LadyVashjCheatToTestAction: Vashj fight START on map={} at steady_clock", mapId);
    }
    else
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - vashjFightStart[mapId]).count();
        LOG_DEBUG("playerbots", "LadyVashjCheatToTestAction: Vashj fight elapsed_ms={} on map={}", elapsed, mapId);
    }

    bot->SetFullHealth();

    if (botAI->IsTank(bot))
    {
        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        if (enchanted && enchanted->IsAlive())
        {
            Unit::DealDamage(bot, enchanted, enchanted->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        }
        Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
        if (strider && strider->IsAlive())
        {
            Unit::DealDamage(bot, strider, strider->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        }
        Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
        if (elite && elite->IsAlive())
        {
            Unit::DealDamage(bot, elite, elite->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        }
    }

    return false;
}
