#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

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
    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    if (waterElemental)
        MarkTargetWithSkull(bot, waterElemental);

    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
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

        if (bot->GetExactDist(member) < 6.0f)
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

    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
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

    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
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

    // If timer is active, send bot to swim position
    const Location* target = nullptr;
    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now)
        target = swimPositions[myGroup];
    else
        target = isletPositions[myGroup];

    // Move if not close enough
    if (bot->GetExactDist2d(target->x, target->y) > 0.1f)
    {
        /* float dX = target->x - bot->GetPositionX();
        float dY = target->y - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(3.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist; */

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

    // If timer is active, send bot to swim position
    const Location* target = nullptr;
    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now)
        target = swimPositions[myGroup];
    else
        target = landPositions[myGroup];

    // Move if not close enough
    if (bot->GetExactDist2d(target->x, target->y) > 0.1f)
    {
        /* float dX = target->x - bot->GetPositionX();
        float dY = target->y - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveDist = std::min(3.0f, dist);
        float moveX = bot->GetPositionX() + (dX / dist) * moveDist;
        float moveY = bot->GetPositionY() + (dY / dist) * moveDist; */

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

    if (IsLurkerCastingSpout(lurker))
        lurkerSpoutTimer[mapId] = time(nullptr) + 20; // 20 second channel time for Spout

    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] <= now)
        lurkerSpoutTimer.erase(mapId);

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

    if (bot->GetVictim() != leotherasDemon);
        Attack(leotherasDemon);

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

    if (leotheras && bot->GetExactDist(leotheras) < 10.0f)
        return FleePosition(leotheras->GetPosition(), 12.0f, minInterval);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (demonFormTank == member && leotherasDemon && bot->GetExactDist(member) < 10.0f)
            return FleePosition(member->GetPosition(), 12.0f, minInterval);

        if (bot->GetExactDist(member) < 5.0f)
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
        Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 10, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
        return true;
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

    if (bot->GetVictim() != leotherasHuman)
        Attack(leotherasHuman);

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

        if (bot->GetVictim() != totem)
            Attack(totem);

        return false;
    }

    // Target priority 2: Tidalvess
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetVictim() != tidalvess)
            Attack(tidalvess);

        return false;
    }
    
    // Target priority 3a: Fathom Sporebat
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && fathomSporebat->IsAlive())
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetVictim() != fathomSporebat)
            Attack(fathomSporebat);

        return false;
    }

    // Target priority 3b: Fathom Lurker
    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && fathomLurker->IsAlive())
    {
        MarkTargetWithSquare(bot, fathomLurker);
        SetRtiTarget(botAI, "square", fathomLurker);

        if (bot->GetVictim() != fathomLurker)
            Attack(fathomLurker);

        return false;
    }

    // Target priority 4: Sharkkis
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetVictim() != sharkkis)
            Attack(sharkkis);

        return false;
    }

    // Target priority 5a: Caribdis
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (botAI->IsRanged(bot) && caribdis && caribdis->IsAlive())
    {
        SetRtiTarget(botAI, "diamond", caribdis);

        if (bot->GetVictim() != caribdis)
            Attack(caribdis);

        return false;
    }

    // Target priority 5b: Karathress
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetVictim() != karathress)
            Attack(karathress);
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
