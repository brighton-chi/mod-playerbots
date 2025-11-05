#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "DestroyItemAction.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ServerFacade.h"
#include "UseItemAction.h"

using namespace SerpentShrineCavernHelpers;
using namespace SerpentShrineCavernLocations;

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
        if (bot->GetVictim() != hydross)
        {
            /* const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, hydross))
                    return botAI->CastSpell(spellName, hydross);
            } */
            return Attack(hydross);
        }

        if (hydross->GetVictim() == bot)
        {
            const Location& position = HydrossFrostTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 0.5f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(7.0f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
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
        ObjectGuid botGuid = bot->GetGUID();
        time_t now = time(nullptr);

        if (now - hydrossChangeToNaturePhaseTimer[botGuid] >= 5)
        {
            const Location& position = HydrossNatureTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 0.5f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(7.0f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                            MovementPriority::MOVEMENT_COMBAT, true, false);
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
        const Location& position = HydrossFrostTankPosition;

        if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
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

bool HydrossTheUnstablePositionNatureTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfCorruptionAt100Percent(bot))
    {
        if (bot->GetVictim() != hydross)
        {
            /* const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, hydross))
                    return botAI->CastSpell(spellName, hydross);
            } */
            return Attack(hydross);
        }

        if (hydross->GetVictim() == bot)
        {
            const Location& position = HydrossNatureTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 0.5f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(7.0f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
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
        ObjectGuid botGuid = bot->GetGUID();
        time_t now = time(nullptr);

        if (now - hydrossChangeToFrostPhaseTimer[botGuid] >= 5)
        {
            const Location& position = HydrossFrostTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 0.5f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveDist = std::min(7.0f, dist);
                float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
                float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
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
        const Location& position = HydrossNatureTankPosition;

        if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
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
        if (member && member->IsAlive() && botAI->IsMainTank(member))
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

bool HydrossTheUnstableManagePhaseChangeTimersAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    ObjectGuid botGuid = bot->GetGUID();

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        if (hydrossChangeToNaturePhaseTimer.find(botGuid) != hydrossChangeToNaturePhaseTimer.end())
            hydrossChangeToNaturePhaseTimer.erase(botGuid);

        if (hydrossChangeToFrostPhaseTimer.find(botGuid) != hydrossChangeToFrostPhaseTimer.end())
            hydrossChangeToFrostPhaseTimer.erase(botGuid);
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossChangeToFrostPhaseTimer.find(botGuid) != hydrossChangeToFrostPhaseTimer.end())
            hydrossChangeToFrostPhaseTimer.erase(botGuid);

        if (HasMarkOfHydrossAt100Percent(bot) &&
            hydrossChangeToNaturePhaseTimer.find(botGuid) == hydrossChangeToNaturePhaseTimer.end())
            hydrossChangeToNaturePhaseTimer[botGuid] = time(nullptr);
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossChangeToNaturePhaseTimer.find(botGuid) != hydrossChangeToNaturePhaseTimer.end())
            hydrossChangeToNaturePhaseTimer.erase(botGuid);

        if (HasMarkOfCorruptionAt100Percent(bot) &&
            hydrossChangeToFrostPhaseTimer.find(botGuid) == hydrossChangeToFrostPhaseTimer.end())
            hydrossChangeToFrostPhaseTimer[botGuid] = time(nullptr);
    }

    return false;
}

bool HydrossTheUnstableManageDpsTimersAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 mapId = hydross->GetMapId();

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        if (hydrossFrostDpsWaitTimer.find(mapId) != hydrossFrostDpsWaitTimer.end())
            hydrossFrostDpsWaitTimer.erase(mapId);

        if (hydrossNatureDpsWaitTimer.find(mapId) != hydrossNatureDpsWaitTimer.end())
            hydrossNatureDpsWaitTimer.erase(mapId);
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossFrostDpsWaitTimer.find(mapId) == hydrossFrostDpsWaitTimer.end())
            hydrossFrostDpsWaitTimer[mapId] = time(nullptr);

        if (hydrossNatureDpsWaitTimer.find(mapId) != hydrossNatureDpsWaitTimer.end())
            hydrossNatureDpsWaitTimer.erase(mapId);
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossNatureDpsWaitTimer.find(mapId) == hydrossNatureDpsWaitTimer.end())
            hydrossNatureDpsWaitTimer[mapId] = time(nullptr);

        if (hydrossFrostDpsWaitTimer.find(mapId) != hydrossFrostDpsWaitTimer.end())
            hydrossFrostDpsWaitTimer.erase(mapId);
    }

    return false;
}

// The Lurker Below

// Position melee to be as close as possible to lurker without entering water
// try to enter water before whirl?
// Spout -- melee stay away from front but keep moving and DPSing?
// Spout -- ranged run into water
// Adds--mark targets; cc ? 6 ambushers (caster) have 56k HP, 3 guardians (melee) have 70k

// If I do the run around approach, it needs some work, probably to keep min distance from lurker and tighten the permitted angle

bool TheLurkerBelowPositionMainTankAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (bot->GetVictim() != lurker)
        return Attack(lurker);

    const Location& position = LurkerMainTankPosition;
    if (bot->GetExactDist2d(position.x, position.y) > 0.2f)
    {
        return MoveTo(bot->GetMapId(), position.x, position.y, position.z, false, false, false, false,
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
    const Location* meleePositions[2] = { &LurkerEMeleePosition, &LurkerWMeleePosition };
    const Location* target = meleePositions[myGroup];

    // Move if not close enough
    if (bot->GetExactDist2d(target->x, target->y) > 0.2f)
    {
        return MoveTo(bot->GetMapId(), target->x, target->y, target->z, false, false, false, false,
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

    // Random radius between 16 and 20 yards
    float radius = 16.0f + ((rand() % 400) / 100.0f);

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
    for (int8 i = 0; i < 3; ++i)
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

    const Location* isletPositions[3] =
        { &LurkerNEIsletPosition, &LurkerNWIsletPosition, &LurkerEIsletPosition };
    const Location* swimPositions[3]  =
        { &LurkerNEIsletSwimPosition, &LurkerNWIsletSwimPosition, &LurkerEIsletSwimPosition };

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    // Log timer value
    LOG_DEBUG("playerbots", "RangedDpsAction: lurkerSpoutTimer[{}]={}, now={}", mapId, lurkerSpoutTimer.count(mapId) ? lurkerSpoutTimer[mapId] : -1, now);

    // If timer is active, send bot to swim position
    const Location* target = nullptr;
    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now)
    {
        target = swimPositions[myGroup];
        LOG_DEBUG("playerbots", "RangedDpsAction: Bot {} moving to swim position ({}, {}, {})", bot->GetName(), target->x, target->y, target->z);
    }
    else
    {
        target = isletPositions[myGroup];
        LOG_DEBUG("playerbots", "RangedDpsAction: Bot {} moving to islet position ({}, {}, {})", bot->GetName(), target->x, target->y, target->z);
    }

    // Log bot's current position
    LOG_DEBUG("playerbots", "RangedDpsAction: Bot {} current position ({}, {}, {}), swimming={}, inWater={}, underWater={}",
    bot->GetName(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->isSwimming(), bot->IsInWater(), bot->IsUnderWater());

    // Move if not close enough
    if (bot->GetExactDist2d(target->x, target->y) > 0.1f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), target->x, target->y, target->z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
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
    for (int8 i = 0; i < 3; ++i)
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

    const Location* landPositions[3] =
        { &LurkerSEHealerLandPosition, &LurkerSWHealerLandPosition, &LurkerNHealerLandPosition };
    const Location* swimPositions[3]  =
        { &LurkerSEHealerSwimPosition, &LurkerSWHealerSwimPosition, &LurkerNHealerSwimPosition };

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    // Log timer value
    LOG_DEBUG("playerbots", "HealerAction: lurkerSpoutTimer[{}]={}, now={}", mapId, lurkerSpoutTimer.count(mapId) ? lurkerSpoutTimer[mapId] : -1, now);

    // If timer is active, send bot to swim position
    const Location* target = nullptr;
    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now)
    {
        target = swimPositions[myGroup];
        LOG_DEBUG("playerbots", "HealerAction: Bot {} moving to swim position ({}, {}, {})", bot->GetName(), target->x, target->y, target->z);
    }
    else
    {
        target = landPositions[myGroup];
        LOG_DEBUG("playerbots", "HealerAction: Bot {} moving to land position ({}, {}, {})", bot->GetName(), target->x, target->y, target->z);
    }

    // Log bot's current position
    LOG_DEBUG("playerbots", "HealerAction: Bot {} current position ({}, {}, {}), swimming={}, inWater={}, underWater={}",
    bot->GetName(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->isSwimming(), bot->IsInWater(), bot->IsUnderWater());

    // Move if not close enough
    if (bot->GetExactDist2d(target->x, target->y) > 0.1f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(bot->GetMapId(), target->x, target->y, target->z, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    // Log current timer value
    LOG_DEBUG("playerbots", "SpoutTimerAction: lurkerSpoutTimer[{}]={}, now={}", mapId, lurkerSpoutTimer.count(mapId) ? lurkerSpoutTimer[mapId] : -1, now);

    // Set timer if Spout starts
    if (IsLurkerCastingSpout(lurker) && (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now)) {
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

bool LeotherasTheBlindDemonFormPositionBossAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasDemon);
    SetRtiTarget(botAI, "square", leotherasDemon);

    if (bot->GetTarget() != leotherasDemon->GetGUID())
    {
        bot->SetSelection(leotherasDemon->GetGUID());
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

/* bool LeotherasTheBlindDemonFormPositionMeleeAction::Execute(Event event)
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
} */

bool LeotherasTheBlindInnerDemonCheatAction::Execute(Event event)
{
    Unit* innerDemon = GetFirstAliveUnitByEntry(botAI, NPC_INNER_DEMON);
    if (innerDemon && botAI->HasCheat(BotCheatMask::raid))
    {
        /* Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 20, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        return true; */
        if (botAI->IsHeal(bot) || botAI->IsTank(bot) || bot->getClass() == CLASS_HUNTER)
        {
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 20, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
            return true;
        }
        else if (innerDemon->GetHealthPct() >= 60.0f)
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 2, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
    }

    return false;
}

bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event event)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasHuman || !leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasHuman);
    SetRtiTarget(botAI, "square", leotherasHuman);

    if (bot->GetTarget() != leotherasHuman->GetGUID() ||
        (botAI->IsMelee(bot) && bot->GetVictim() != leotherasHuman))
    {
        bot->SetSelection(leotherasHuman->GetGUID());
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
    ObjectGuid botGuid = bot->GetGUID();

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);

    if (leotheras && leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        if (leotherasFinalPhaseDpsWaitTimer.find(mapId) != leotherasFinalPhaseDpsWaitTimer.end())
            leotherasFinalPhaseDpsWaitTimer.erase(mapId);

        if (leotherasHumanFormDpsWaitTimer.find(mapId) != leotherasHumanFormDpsWaitTimer.end())
            leotherasHumanFormDpsWaitTimer.erase(mapId);

        if (leotherasDemonFormDpsWaitTimer.find(mapId) != leotherasDemonFormDpsWaitTimer.end())
            leotherasDemonFormDpsWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && !leotherasPhase3Demon)
    {
        if (leotherasHumanFormDpsWaitTimer.find(mapId) == leotherasHumanFormDpsWaitTimer.end())
            leotherasHumanFormDpsWaitTimer[mapId] = time(nullptr);

        if (leotherasDemonFormDpsWaitTimer.find(mapId) != leotherasDemonFormDpsWaitTimer.end())
            leotherasDemonFormDpsWaitTimer.erase(mapId);
    }
    else if (leotherasPhase2Demon)
    {
        if (leotherasDemonFormDpsWaitTimer.find(mapId) == leotherasDemonFormDpsWaitTimer.end())
            leotherasDemonFormDpsWaitTimer[mapId] = time(nullptr);

        if (leotherasHumanFormDpsWaitTimer.find(mapId) != leotherasHumanFormDpsWaitTimer.end())
            leotherasHumanFormDpsWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        if (leotherasFinalPhaseDpsWaitTimer.find(mapId) == leotherasFinalPhaseDpsWaitTimer.end())
            leotherasFinalPhaseDpsWaitTimer[mapId] = time(nullptr);

        if (leotherasHumanFormDpsWaitTimer.find(mapId) != leotherasHumanFormDpsWaitTimer.end())
            leotherasHumanFormDpsWaitTimer.erase(mapId);

        if (leotherasDemonFormDpsWaitTimer.find(mapId) != leotherasDemonFormDpsWaitTimer.end())
            leotherasDemonFormDpsWaitTimer.erase(mapId);
    }

    return false;
}

// Fathom-Lord Karathress

// Each tank assigned to specific naga, go to designated position
// Melee kill order: spitfire totem, fathom lurker/sporebat, tidalvess, sharkkis, karathress
// Ranged kill order: same but caribdis before karathress
// hunters misdirect initial pull - caribdis top priority for misdirects
// consider whether caribdis healing wave needs interrupting (can it be ranged? seems not)
// consider whether to put curse of tongues on caribdis

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
        const Location& position = KarathressTankPosition;

        if (!bot->IsWithinMeleeRange(karathress))
        {
            return MoveTo(karathress->GetMapId(), karathress->GetPositionX(),
                          karathress->GetPositionY(), karathress->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
        const Location& position = SharkkisTankPosition;

        if (!bot->IsWithinMeleeRange(sharkkis))
        {
            return MoveTo(sharkkis->GetMapId(), sharkkis->GetPositionX(),
                          sharkkis->GetPositionY(), sharkkis->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
        const Location& position = TidalvessTankPosition;

        if (!bot->IsWithinMeleeRange(tidalvess))
        {
            return MoveTo(tidalvess->GetMapId(), tidalvess->GetPositionX(),
                          tidalvess->GetPositionY(), tidalvess->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
        const Location& position = CaribdisTankPosition;

        if (!bot->IsWithinMeleeRange(caribdis))
        {
            return MoveTo(caribdis->GetMapId(), caribdis->GetPositionX(),
                          caribdis->GetPositionY(), caribdis->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(7.0f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                        MovementPriority::MOVEMENT_COMBAT, true, false);
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
    else if (hunterIndex == 2)
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
    // Target priority 1: Spitfire Totems
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
    if (totem)
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
        {
            bot->SetSelection(totem->GetGUID());
            return Attack(totem);
        }

        return false;
    }

    // Target priority 2: Tidalvess
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetTarget() != tidalvess->GetGUID())
        {
            bot->SetSelection(tidalvess->GetGUID());
            return Attack(tidalvess);
        }

        return false;
    }

    // Target priority 3a: Fathom Sporebat
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && fathomSporebat->IsAlive())
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetTarget() != fathomSporebat->GetGUID())
        {
            bot->SetSelection(fathomSporebat->GetGUID());
            return Attack(fathomSporebat);
        }

        return false;
    }

    // Target priority 3b: Fathom Lurker
    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && fathomLurker->IsAlive())
    {
        MarkTargetWithSquare(bot, fathomLurker);
        SetRtiTarget(botAI, "square", fathomLurker);

        if (bot->GetTarget() != fathomLurker->GetGUID())
        {
            bot->SetSelection(fathomLurker->GetGUID());
            return Attack(fathomLurker);
        }

        return false;
    }

    // Target priority 4: Sharkkis
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetTarget() != sharkkis->GetGUID())
        {
            bot->SetSelection(sharkkis->GetGUID());
            return Attack(sharkkis);
        }

        return false;
    }

    // Target priority 5a: Caribdis
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (botAI->IsRanged(bot) && caribdis && caribdis->IsAlive())
    {
        SetRtiTarget(botAI, "diamond", caribdis);

        float dist = bot->GetExactDist2d(caribdis);
        if (dist > 35.0f)
            return FleePosition(caribdis->GetPosition(), 30.0f, 0);

        if (bot->GetTarget() != caribdis->GetGUID())
        {
            bot->SetSelection(caribdis->GetGUID());
            return Attack(caribdis);
        }

        return false;
    }

    // Target priority 5b: Karathress
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetTarget() != karathress->GetGUID())
        {
            bot->SetSelection(karathress->GetGUID());
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

    float dist = bot->GetExactDist2d(caribdis);
    if (dist > 15.0f)
        return FleePosition(caribdis->GetPosition(), 12.0f, 0);

    return false;
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    uint32 mapId = karathress->GetMapId();

    if (karathress && karathress->GetHealth() == karathress->GetMaxHealth())
    {
        if (karathressDpsWaitTimer.find(mapId) != karathressDpsWaitTimer.end())
            karathressDpsWaitTimer.erase(mapId);

        if (karathressDpsWaitTimer.find(mapId) == karathressDpsWaitTimer.end())
            karathressDpsWaitTimer[mapId] = time(nullptr);
    }

    return false;
}

// Morogrim Tidewalker

// Phase 1 tank position
// Phase 2 tank position (in doorway or behind pillar?)
// Phase 2 dps/heal position by tidewalker
// necessary to prio murlocs?

// where are graves? do healers need to stay there? move to them if somebody gets graved?

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
    const Location& phase1 = TidewalkerPhase1TankPosition;
    if (bot->GetExactDist2d(phase1.x, phase1.y) > 1.0f)
    {
        float dX = phase1.x - bot->GetPositionX();
        float dY = phase1.y - bot->GetPositionY();
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
    const Location& phase2 = TidewalkerPhase2TankPosition;
    const Location& transition = TidewalkerPhaseTransitionWaypoint;

    ObjectGuid botGuid = bot->GetGUID();
    uint8 step = tidewalkerTankStep.count(botGuid) ? tidewalkerTankStep[botGuid] : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.x, transition.y) > 2.0f)
        {
            float dX = transition.x - bot->GetPositionX();
            float dY = transition.y - bot->GetPositionY();
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
        if (bot->GetExactDist2d(phase2.x, phase2.y) > 1.0f)
        {
            float dX = phase2.x - bot->GetPositionX();
            float dY = phase2.y - bot->GetPositionY();
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

    const Location& phase2 = TidewalkerPhase2RangedPosition;
    const Location& transition = TidewalkerPhaseTransitionWaypoint;

    ObjectGuid botGuid = bot->GetGUID();
    uint8 step = tidewalkerRangedStep.count(botGuid) ? tidewalkerRangedStep[botGuid] : 0;

    if (step == 0)
    {
        if (bot->GetExactDist2d(transition.x, transition.y) > 2.0f)
        {
            float dX = transition.x - bot->GetPositionX();
            float dY = transition.y - bot->GetPositionY();
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
        if (bot->GetExactDist2d(phase2.x, phase2.y) > 1.0f)
        {
            float dX = phase2.x - bot->GetPositionX();
            float dY = phase2.y - bot->GetPositionY();
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

// Reminder--Shamans need to use grounding totems, at least in MT's group

bool LadyVashjPhase1MainTankPositionBossAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (bot->GetVictim() != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() == bot)
    {
        const Location& position = VashjRoomCenterPosition;

        if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveDist = std::min(4.5f, dist);
            float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
            float moveY = bot->GetPositionY() + (dY / dist) * moveDist;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                        MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else if (!bot->IsWithinMeleeRange(vashj))
        {
            return MoveTo(vashj->GetMapId(), vashj->GetPositionX(),
                          vashj->GetPositionY(), vashj->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool LadyVashjPhase1AndPhase3PositionRangedAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Center of the room (set these to Vashj's room center coordinates)
    const Location& center = VashjRoomCenterPosition;
    const float minSpreadRadius = 28.0f;
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

        float angle = 2 * M_PI * botIndex / count;
        // Use bot GUID or index to generate a unique "random" radius per bot
        uint32 botSeed = bot->GetGUID().GetCounter(); // or use botIndex for simplicity
        float radius = minSpreadRadius + (botSeed % 1000) / 1000.0f * (maxSpreadRadius - minSpreadRadius);
        float targetX = center.x + radius * cos(angle);
        float targetY = center.y + radius * sin(angle);

        vashjRangedPositions[bot->GetGUID()] = Position(targetX, targetY, center.z);
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

    MarkTargetWithSkull(bot, vashj);

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

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot)
            continue;

        float distance = bot->GetExactDist2d(member);
        if (distance < 10.5f)
            return MoveAway(member, 11.0f, false);
    }

    return false;
}

bool LadyVashjRangedDpsMoveToPhase2AssignedPositionsAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const Location positions[8] =
    {
        { 65.087f, -878.344f, 41.097f }, // NW
        { 29.693f, -865.188f, 41.097f }, // W
        { 9.766f, -869.707f, 41.097f },  // SW
        { -25.352f, -910.754f, 41.097f },// SSW
        { -9.504f, -964.514f, 41.097f }, // SE
        { 29.701f, -982.523f, 41.097f }, // E
        { 42.143f, -978.813f, 41.097f }, // ENE
        { 83.647f, -941.901f, 41.097f }  // NNE
    };

    // First pass: prioritize hunters, non-affliction warlocks, mages, druids
    std::vector<Player*> prioritized;
    std::vector<Player*> others;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || !botAI->IsRangedDps(member) || botAI->IsRangedDpsAssistantOfIndex(member, 0) || botAI->IsRangedDpsAssistantOfIndex(member, 1))
            continue;

        uint8 cls = member->getClass();
        int8 tab = AiFactory::GetPlayerSpecTab(member);

        if (cls == CLASS_HUNTER || cls == CLASS_MAGE || cls == CLASS_DRUID || (cls == CLASS_WARLOCK && tab != 0))
            prioritized.push_back(member);
        else
            others.push_back(member);
    }

    // Assign positions
    std::vector<Player*> assigned;
    assigned.insert(assigned.end(), prioritized.begin(), prioritized.end());
    assigned.insert(assigned.end(), others.begin(), others.end());

    for (size_t i = 0; i < 8 && i < assigned.size(); ++i)
    {
        Player* member = assigned[i];
        if (member == bot)
        {
            const Location& pos = positions[i];
            if (!bot->GetVictim() && bot->GetExactDist2d(pos.x, pos.y) > 2.0f)
                return MoveTo(bot->GetMapId(), pos.x, pos.y, pos.z,
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            break;
        }
    }

    return false;
}

bool LadyVashjAssignPhase2DpsPriorityAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (IsRangedRTIMarker(botAI, bot))
        MarkTargetWithMoon(bot, vashj);

    if (bot->GetTarget() == vashj->GetGUID())
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
    }

    // Get all nearby hostile NPCs
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    Unit* target = nullptr;
    Unit* tainted = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_TAINTED_ELEMENTAL:
                if (!tainted || bot->GetDistance2d(unit) < bot->GetDistance2d(tainted))
                    tainted = unit;
                break;
            case NPC_ENCHANTED_ELEMENTAL:
                if (!enchanted || bot->GetDistance2d(unit) < bot->GetDistance2d(enchanted))
                    enchanted = unit;
                break;
            case NPC_COILFANG_ELITE:
                if (!elite || bot->GetDistance2d(unit) < bot->GetDistance2d(elite))
                    elite = unit;
                break;
            case NPC_COILFANG_STRIDER:
                if (!strider || bot->GetDistance2d(unit) < bot->GetDistance2d(strider))
                    strider = unit;
                break;
            default:
                break;
        }
    }

    // Set priorities
    std::vector<Unit*> targets;
    if (botAI->IsRangedDps(bot))
    {
        targets = { tainted, enchanted, strider };
    }
    else if (botAI->IsMelee(bot))
    {
        targets = { tainted, enchanted, elite };
    }
    else
    {
        // Default to ranged priorities if not classified
        targets = { tainted, enchanted, strider };
    }

    // Pick the first valid target
    for (Unit* t : targets)
    {
        if (t && t->IsAlive())
        {
            target = t;
            break;
        }
    }

    // If already targeting, do nothing
    if (context->GetValue<Unit*>("current target")->Get() == target)
        return false;

    // Attack the chosen target
    if (target)
        return Attack(target);

    const Location& position = VashjRoomCenterPosition;
    if (AI_VALUE(Unit*, "current target") == nullptr)
        return MoveInside(bot->GetMapId(), position.x, position.y, position.z, 20.0f, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

    /*
    // disabled to test Naxx targeting with DPS assist off
    Unit* taintedElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_ELEMENTAL);
    if (taintedElemental && taintedElemental->IsAlive() && botAI->IsRangedDps(bot))
    {
        MarkTargetWithSkull(bot, taintedElemental);
        SetRtiTarget(botAI, "skull", taintedElemental);

        if (!bot->IsWithinRange(taintedElemental, 30.0f))
            return MoveTo(taintedElemental, 25.0f, MovementPriority::MOVEMENT_COMBAT);

        if (bot->GetTarget() != taintedElemental->GetGUID())
        {
            bot->SetSelection(taintedElemental->GetGUID());
            return Attack(taintedElemental);
        }

        return false;
    }

    Unit* enchantedElemental = GetFirstAliveUnitByEntry(botAI, NPC_ENCHANTED_ELEMENTAL);
    if (enchantedElemental && enchantedElemental->IsAlive() &&
        ((enchantedElemental->GetExactDist2d(vashj) < 40.0f && botAI->IsRangedDps(bot)) ||
          enchantedElemental->GetExactDist2d(vashj) < 30.0f && botAI->IsMelee(bot) && !botAI->IsTank(bot)))
    {
        Unit* bestEnchanted = nullptr;
        float minDist = std::numeric_limits<float>::max();
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_ENCHANTED_ELEMENTAL)
            {
                float dist = unit->GetExactDist2d(vashj);
                if (dist < minDist)
                {
                    minDist = dist;
                    bestEnchanted = unit;
                }
            }
        }
        if (bestEnchanted)
        {
            MarkTargetWithStar(bot, bestEnchanted);
            LOG_DEBUG("playerbots", "Marking enchanted elemental {} with star", bestEnchanted->GetName());
        }
        else
            LOG_DEBUG("playerbots", "No enchanted elemental found to mark");

        SetRtiTarget(botAI, "star", enchantedElemental);

        if (bot->GetTarget() != enchantedElemental->GetGUID())
        {
            bot->SetSelection(enchantedElemental->GetGUID());
            return Attack(enchantedElemental);
        }

        return false;
    }

    Unit* elite = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_ELITE);
    if (elite && elite->IsAlive() && botAI->IsMelee(bot))
    {
        if (IsMeleeRTIMarker(botAI, bot))
            MarkTargetWithTriangle(bot, elite);

        SetRtiTarget(botAI, "triangle", elite);

        if (bot->GetVictim() != elite)
            return Attack(elite);

        return false;
    }

    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    if (strider && strider->IsAlive() && botAI->IsRangedDps(bot))
    {
        if (IsRangedRTIMarker(botAI, bot))
            MarkTargetWithCircle(bot, strider);

        SetRtiTarget(botAI, "circle", strider);

        if (bot->GetTarget() != strider->GetGUID())
        {
            bot->SetSelection(strider->GetGUID());
            return Attack(strider);
        }

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
} */

/* // Old
bool LadyVashjAssignPhase2DpsPriorityAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj)
    {
        if (IsRangedRTIMarker(botAI, bot))
            MarkTargetWithMoon(bot, vashj);

        if (bot->GetTarget() == vashj->GetGUID())
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            bot->SetTarget(ObjectGuid::Empty);
            bot->SetSelection(ObjectGuid());
        }
    }

    std::vector<Player*> assignedRanged = GetPhase2AssignedRangedDpsBots(group, botAI);

    // Target priority 1 for assigned ranged and melee: Tainted Elemental
    Unit* taintedElemental = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (taintedElemental && taintedElemental->IsAlive() && std::find(assignedRanged.begin(), assignedRanged.end(), bot) != assignedRanged.end())
    {
        MarkTargetWithSkull(bot, taintedElemental);
        SetRtiTarget(botAI, "skull", taintedElemental);

        if (!bot->IsWithinRange(taintedElemental, 30.0f))
            return MoveTo(taintedElemental, 25.0f, MovementPriority::MOVEMENT_COMBAT);

        if (bot->GetTarget() != taintedElemental->GetGUID())
        {
            bot->SetSelection(taintedElemental->GetGUID());
            return Attack(taintedElemental);
        }
    }
    return false;

    // Target priority 2 for melee: Coilfang Elite
    Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
    if (elite && elite->IsAlive() && botAI->IsMelee(bot))
    {
        if (IsMeleeRTIMarker(botAI, bot))
            MarkTargetWithTriangle(bot, elite);

        SetRtiTarget(botAI, "triangle", elite);

        if (bot->GetVictim() != elite)
            return Attack(elite);
    }
    return false;

    // Target priority 2 for assigned ranged and target priority 3 for melee: Enchanted Elemental
    Unit* enchantedElemental = AI_VALUE2(Unit*, "find target", "enchanted elemental");
    if (enchantedElemental && enchantedElemental->IsAlive() && bot->GetExactDist2d(enchantedElemental) < 30.0f
        && (botAI->IsMelee(bot) || std::find(assignedRanged.begin(), assignedRanged.end(), bot) != assignedRanged.end()))
    {
        MarkTargetWithStar(bot, enchantedElemental);
        SetRtiTarget(botAI, "star", enchantedElemental);

        if (bot->GetTarget() != enchantedElemental->GetGUID())
        {
            bot->SetSelection(enchantedElemental->GetGUID());
            return Attack(enchantedElemental);
        }
    }
    return false;

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    // Target priority for unassigned ranged: Coilfang Strider
    if (strider && strider->IsAlive() && botAI->IsRangedDps(bot) && !botAI->IsRangedDpsAssistantOfIndex(bot, 0) && !botAI->IsRangedDpsAssistantOfIndex(bot, 1) &&
        std::find(assignedRanged.begin(), assignedRanged.end(), bot) == assignedRanged.end())
    {
        MarkTargetWithCircle(bot, strider);
        SetRtiTarget(botAI, "circle", strider);

        if (bot->GetExactDist2d(strider) <= 35.0f && bot->GetExactDist2d(strider) > 15.0f &&
            bot->GetTarget() != strider->GetGUID())
        {
            bot->SetSelection(strider->GetGUID());
            return Attack(strider);
        }

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
} */

bool LadyVashjAssistantsFollowMasterInPhase2Action::Execute(Event event)
{
    Player* master = botAI->GetMaster();
    if (!master || master == bot)
        return false;

    if (bot->GetExactDist2d(master) > 35.0f)
        return MoveTo(master, 30.0f, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event event)
{
    Player* master = botAI->GetMaster();
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(botAI, bot);
    Unit* closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(bot, master);

    if (!master || !firstCorePasser || !secondCorePasser || !thirdCorePasser || !closestTrigger)
        return false;

    // Always move bots into position
    if (botAI->IsRangedDpsAssistantOfIndex(bot, 0)) // First core passer
        LineUpFirstCorePasser(master, closestTrigger);
    else if (botAI->IsRangedDpsAssistantOfIndex(bot, 1)) // Second core passer
        LineUpSecondCorePasser(firstCorePasser, closestTrigger);
    else if (botAI->IsHealAssistantOfIndex(bot, 0)) // Third core passer
        LineUpThirdCorePasser(secondCorePasser, closestTrigger);

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (item && botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // First core passer logic
        if (botAI->IsRangedDpsAssistantOfIndex(bot, 0))
        {
            if (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger) &&
                bot->GetExactDist(secondCorePasser) <= 40.0f)
            {
                botAI->ImbueItem(item, secondCorePasser);
                ScheduleCoreReconcile(botAI, bot, secondCorePasser, ITEM_TAINTED_CORE, 500);
            }
        }
        // Second core passer logic
        else if (botAI->IsRangedDpsAssistantOfIndex(bot, 1))
        {
            if (CanUseGenerator())
                UseCoreOnNearestGenerator();
            else if (thirdCorePasser->GetExactDist(closestTrigger) <= 4.0f &&
                     bot->GetExactDist(thirdCorePasser) <= 40.0f)
            {
                botAI->ImbueItem(item, thirdCorePasser);
                ScheduleCoreReconcile(botAI, bot, thirdCorePasser, ITEM_TAINTED_CORE, 500);
            }
        }
        // Third core passer logic
        else if (botAI->IsHealAssistantOfIndex(bot, 0) && CanUseGenerator())
            UseCoreOnNearestGenerator();
    }

    return false;
}

void LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* master, Unit* closestTrigger)
{
    const float centerX = VashjRoomCenterPosition.x;
    const float centerY = VashjRoomCenterPosition.y;
    const float radius = 55.0f;

    float mx = master->GetPositionX();
    float my = master->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * cos(angle);
    float targetY = centerY + radius * sin(angle);
    float targetZ = 41.097f;

    bot->Yell("I'm moving into position to receive the tainted core!", LANG_UNIVERSAL); // If PR'd, will need to use DB
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

    if (distToTrigger == 0.0f)
        return;

    dx /= distToTrigger; dy /= distToTrigger; dz /= distToTrigger;

    float targetX, targetY, targetZ;
    const float maxDistance = 38.0f;

    if (distToTrigger <= maxDistance)
    {
        float moveDist = std::max(distToTrigger - 2.0f, 0.0f);
        targetX = fx + dx * moveDist;
        targetY = fy + dy * moveDist;
        targetZ = 42.985f;
    }
    else
    {
        targetX = fx + dx * maxDistance;
        targetY = fy + dy * maxDistance;
        targetZ = 42.985f;
    }

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
           false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
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
        return false;

    dx /= distToTrigger; dy /= distToTrigger; dz /= distToTrigger;

    float moveDist = std::max(distToTrigger - 2.0f, 0.0f);
    float pos1X = fx + dx * moveDist;
    float pos1Y = fy + dy * moveDist;
    const float pos1Z = 42.985f;

    float pos2X = fx + dx * 38.0f;
    float pos2Y = fy + dy * 38.0f;
    const float pos2Z = 42.985f;

    float dist1 = secondCorePasser->GetExactDist(Position(pos1X, pos1Y, pos1Z));
    float dist2 = secondCorePasser->GetExactDist(Position(pos2X, pos2Y, pos2Z));

    return dist1 <= 2.0f || dist2 <= 2.0f;
}

void LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(Player* secondCorePasser, Unit* closestTrigger)
{
    if (secondCorePasser->GetExactDist(closestTrigger) <= 2.0f)
        return;

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
        return;

    dx /= length; dy /= length; dz /= length;

    float targetX = tx - dx * 2.0f;
    float targetY = ty - dy * 2.0f;
    const float targetZ = 42.985f;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::CanUseGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);

    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
        return false;

    return bot->GetExactDist(generator) <= 4.0f;
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator()
{
    std::vector<GeneratorInfo> generators = GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    GameObject* generator = botAI->GetGameObject(nearestGen->guid);

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
        return true;
    }

    return false;
}

bool LadyVashjAvoidToxicSporesAction::Execute(Event event)
{
    // Get all spore drop triggers within 30 yards
    std::vector<Unit*> sporeTriggers = GetAllSporeDropTriggers(botAI, bot);

    // If none found, nothing to avoid
    if (sporeTriggers.empty())
        return false;

    const float hazardRadius = 13.0f; // Minimum safe distance from a spore trigger
    const float searchRadius = 30.0f; // How far to search for a safe spot
    const float step = 2.0f;          // Step size for candidate positions

    // Try positions in a circle around the bot
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
    {
        for (float r = hazardRadius + 2.0f; r < searchRadius; r += step)
        {
            float x = bot->GetPositionX() + r * cos(angle);
            float y = bot->GetPositionY() + r * sin(angle);
            float z = bot->GetPositionZ(); // You may want to use Map::GetHeight for terrain

            if (IsSafePosition(x, y, z, sporeTriggers, hazardRadius))
            {
                // Move to the first safe position found
                return MoveTo(bot->GetMapId(), x, y, z,
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    // No safe position found
    return false;
}

std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    const float radius = 30.0f;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || unit->GetEntry() != NPC_SPORE_DROP_TRIGGER)
            continue;

        float dist = bot->GetExactDist2d(unit);
        if (dist < radius)
            sporeDropTriggers.push_back(unit);
    }

    return sporeDropTriggers;
}

bool LadyVashjAvoidToxicSporesAction::IsSafePosition(float x, float y, float z, const std::vector<Unit*>& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = std::sqrt(std::pow(x - hazard->GetPositionX(), 2) + std::pow(y - hazard->GetPositionY(), 2));
        if (dist < hazardRadius)
            return false;
    }

    return true;
}

bool LadyVashjManageTrackersAction::Execute(Event event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (vashj->GetHealthPct() > 99.5f || IsLadyVashjInPhase2(botAI))
    {
        if (!vashjRangedPositions.empty())
            vashjRangedPositions.clear();

        if (!vashjHasReachedRangedPosition.empty())
            vashjHasReachedRangedPosition.clear();
    }

    return false;
}
