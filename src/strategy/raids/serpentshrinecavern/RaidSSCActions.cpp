#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "Playerbots.h"

using namespace SerpentShrineCavernHelpers;

// Trash Mobs

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event event)
{
    Unit* waterTotem = AI_VALUE2(Unit*, "find target", "water elemental totem");
    if (!waterTotem)
        return false;

    MarkTargetWithSkull(bot, waterTotem);
    return false;
}

// Hydross the Unstable <Duke of Currents>

// Frost tank action, have it be MT, frost tank position if does not have mark of hydross 100, 250, or 500
// frost tank moves to nature tank position if has mark of hydross 100 +
// frost tank taunts hydross if nature tank has mark of corruption 100 +

// Nature tank action, have it be 1st assist tank, nature tank position if does not have mark of corruption 100, 250, or 500
// nature tank moves to frost tank position if has mark of hydross 100 +
// nature tank taunts hydross if frost tank has mark of hydross 100 +

// ranged bots need to spread >8 yards if hydross is in water form
// melee what to do? def need to stay away from tank

// dps mark/prioritize elemental adds with skull

bool HydrossTheUnstablePositionFrostTankAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfHydrossAt100Percent(bot))
    {
        if (bot->GetVictim() != hydross)
        {
            const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, hydross))
                    return botAI->CastSpell(spellName, hydross);
            }
            return Attack(hydross);
        }

        if (hydross->GetVictim() == bot)
        {
            const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;
            const float maxDistance = 1.0f;
            float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

            if (distanceToPosition > maxDistance)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
                float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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

        if (now - hydrossChangeToNaturePhaseTimer[botGuid] >= 10)
        {
            const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;
            const float maxDistance = 1.0f;
            float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

            if (distanceToPosition > maxDistance)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
                float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
        const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;
        const float maxDistance = 1.0f;
        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
            const char* taunts[] = { "taunt", "growl", "hand of reckoning" };
            for (const char* spellName : taunts)
            {
                if (botAI->CanCastSpell(spellName, hydross))
                    return botAI->CastSpell(spellName, hydross);
            }
            return Attack(hydross);
        }

        if (hydross->GetVictim() == bot)
        {
            const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;
            const float maxDistance = 1.0f;
            float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

            if (distanceToPosition > maxDistance)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
                float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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

        if (now - hydrossChangeToFrostPhaseTimer[botGuid] >= 10)
        {
            const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;
            const float maxDistance = 1.0f;
            float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

            if (distanceToPosition > maxDistance)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
                float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

                return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
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
        const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;
        const float maxDistance = 1.0f;
        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HydrossTheUnstableManageDPSTimerAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    ObjectGuid botGuid = bot->GetGUID();

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

/* bool HydrossTheUnstableManageDPSTimerAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 mapId = hydross->GetMapId();

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossFrostDPSWaitTimer.find(mapId) == hydrossFrostDPSWaitTimer.end())
            hydrossFrostDPSWaitTimer[mapId] = time(nullptr);

        if (hydrossNatureDPSWaitTimer.find(mapId) != hydrossNatureDPSWaitTimer.end())
            hydrossNatureDPSWaitTimer.erase(mapId);
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossNatureDPSWaitTimer.find(mapId) == hydrossNatureDPSWaitTimer.end())
            hydrossNatureDPSWaitTimer[mapId] = time(nullptr);

        if (hydrossFrostDPSWaitTimer.find(mapId) != hydrossFrostDPSWaitTimer.end())
            hydrossFrostDPSWaitTimer.erase(mapId);
    }

    return false;
} */

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

    std::vector<Player*> tooClose;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (bot->GetExactDist2d(member) < 10.0f)
            tooClose.push_back(member);
    }

    if (!tooClose.empty())
    {
        // Calculate average position of nearby members
        float avgX = 0, avgY = 0;
        for (Player* member : tooClose)
        {
            avgX += member->GetPositionX();
            avgY += member->GetPositionY();
        }
        avgX /= tooClose.size();
        avgY /= tooClose.size();

        // Move away from the average position
        float dX = bot->GetPositionX() - avgX;
        float dY = bot->GetPositionY() - avgY;
        float dist = sqrt(dX * dX + dY * dY);
        if (dist > 0.1f)
        {
            float moveX = bot->GetPositionX() + (dX / dist) * 10.0f;
            float moveY = bot->GetPositionY() + (dY / dist) * 10.0f;
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// The Lurker Below

// Position melee to be as close as possible to lurker without entering water
// try to enter water before whirl?
// Spout -- melee stay away from front but keep moving and DPSing?
// Spout -- ranged run into water
// Adds--mark targets; cc ? 6 ambushers (caster) have 56k HP, 3 guardians (melee) have 70k

bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event event)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    // Get Lurker's facing direction (orientation)
    float bossFacing = lurker->GetOrientation();

    // Get angle from Lurker to bot
    float angleToBot = atan2(bot->GetPositionY() - lurker->GetPositionY(),
                             bot->GetPositionX() - lurker->GetPositionX());
    float relativeAngle = Position::NormalizeOrientation(angleToBot - bossFacing);

    // Check if bot is within 90 degrees to either side behind Lurker (180-degree arc)
    if (fabs(relativeAngle) <= M_PI / 2)
    {
        // Bot is already behind Lurker, do nothing (allow normal actions)
        return false;
    }

    // Bot is outside the safe arc, move to a random point behind Lurker
    float behindAngle = bossFacing + M_PI + ((rand() % 100) / 100.0f - 0.5f) * M_PI;
    float distance = bot->GetExactDist2d(lurker); // Keep current distance
    float destX = lurker->GetPositionX() + distance * cos(behindAngle);
    float destY = lurker->GetPositionY() + distance * sin(behindAngle);
    float destZ = lurker->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, lurker->GetPositionX(), lurker->GetPositionY(), lurker->GetPositionZ(), destX, destY, destZ))
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(lurker->GetMapId(), destX, destY, destZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Leotheras the Blind

// human form tank position (MT)
// demon form tank position
// Use warlock to tank demon form? if so, need helper to assign warlock tank
// run from whirlwind
// dps prioritize targets -- inner demon, MC'd party members, leotheras (whatever form is active), leotheras human (final phase)

// timers--hold dps after whirlwind, hold dps after phase change

bool LeotherasTheBlindHumanFormPositionBossAction::Execute(Event event)
{
    Unit* leotheras = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_HUMAN_FORM);
    if (!leotheras)
        return false;

    if (bot->GetVictim() != leotheras)
        return Attack(leotheras);

    if (leotheras->GetVictim() == bot)
    {
        const Location& position = SerpentShrineCavernLocations::LeotherasHumanFormTankPosition;
        const float maxDistance = 1.0f;
        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(leotheras))
        {
            return MoveTo(leotheras->GetMapId(), leotheras->GetPositionX(),
                          leotheras->GetPositionY(), leotheras->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool LeotherasTheBlindDemonFormPositionBossAction::Execute(Event event)
{
    Unit* leotheras = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);
    if (!leotheras)
        return false;

    if (bot->GetVictim() != leotheras)
        return Attack(leotheras);

    if (leotheras->GetVictim() == bot)
    {
        const Location& position = SerpentShrineCavernLocations::LeotherasDemonFormTankPosition;
        const float maxDistance = 1.0f;
        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (botAI->IsMelee(bot) && !bot->IsWithinMeleeRange(leotheras))
        {
            return MoveTo(leotheras->GetMapId(), leotheras->GetPositionX(),
                          leotheras->GetPositionY(), leotheras->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (botAI->IsRanged(bot))
        {
            float dist = bot->GetExactDist2d(leotheras);
            if (dist < 12.0f)
            {
                float angle = bot->GetAngle(leotheras);
                float targetDist = 15.0f;
                float moveX = leotheras->GetPositionX() + targetDist * cos(angle);
                float moveY = leotheras->GetPositionY() + targetDist * sin(angle);

                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                return MoveTo(leotheras->GetMapId(), moveX, moveY, leotheras->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
            else if (dist > 35.0f)
            {
                float angle = bot->GetAngle(leotheras);
                float targetDist = 30.0f;
                float moveX = leotheras->GetPositionX() + targetDist * cos(angle);
                float moveY = leotheras->GetPositionY() + targetDist * sin(angle);

                return MoveTo(leotheras->GetMapId(), moveX, moveY, leotheras->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

bool LeotherasTheBlindAssignDPSPriorityAction::Execute(Event event)
{
    Unit* leotherasHuman = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_HUMAN_FORM);
    Unit* leotherasDemon = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);
    Unit* innerDemon = AI_VALUE2(Unit*, "find target", "inner demon");

    if (innerDemon)
    {
        MarkTargetWithSquare(bot, innerDemon);
        SetRtiTarget(botAI, "square", innerDemon);

        if (bot->GetVictim() != innerDemon)
            return Attack(innerDemon);
    }

    Player* demonTank = GetLeotherasDemonFormTank(botAI, bot);
    if (leotherasHuman && leotherasDemon)
    {
        if (demonTank)
        {
            MarkTargetWithCross(bot, leotherasDemon);
            SetRtiTarget(botAI, "cross", leotherasDemon);
        }

        if (!demonTank)
        {
            MarkTargetWithSquare(bot, leotherasHuman);
            SetRtiTarget(botAI, "square", leotherasHuman);

            if (!botAI->IsMainTank(bot) && bot->GetVictim() != leotherasHuman)
                return Attack(leotherasHuman);
        }
    }

    return false;
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    const float safeDistance = 15.0f;
    float distance = bot->GetExactDist2d(leotheras);

    if (distance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        MoveAway(leotheras, 18.0f, false);
    }

    return false;
}

bool LeotherasTheBlindManageDPSTimersAction::Execute(Event event)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    uint32 mapId = leotheras->GetMapId();

    Unit* leotherasHuman = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_HUMAN_FORM);
    Unit* leotherasDemon = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);

    if (leotherasHuman && leotherasHuman->GetHealth() == leotherasHuman->GetMaxHealth())
    {
        if (leotherasFinalPhaseDPSWaitTimer.find(mapId) != leotherasFinalPhaseDPSWaitTimer.end())
            leotherasFinalPhaseDPSWaitTimer.erase(mapId);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) != leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer.erase(mapId);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) == leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer[mapId] = time(nullptr);
    }
    else if (leotherasHuman && !leotherasDemon)
    {
        if (leotherasHumanFormDPSWaitTimer.find(mapId) == leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer[mapId] = time(nullptr);

        if (leotherasDemonFormDPSWaitTimer.find(mapId) != leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer.erase(mapId);
    }
    else if (leotherasDemon && !leotherasHuman)
    {
        if (leotherasDemonFormDPSWaitTimer.find(mapId) == leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer[mapId] = time(nullptr);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) != leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && leotherasDemon)
    {
        if (leotherasFinalPhaseDPSWaitTimer.find(mapId) == leotherasFinalPhaseDPSWaitTimer.end())
            leotherasFinalPhaseDPSWaitTimer[mapId] = time(nullptr);
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
        const Location& position = SerpentShrineCavernLocations::KarathressTankPosition;
        const float maxDistance = 2.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, true, 
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else if (!bot->IsWithinMeleeRange(karathress))
            return MoveTo(karathress->GetMapId(), karathress->GetPositionX(),
                          karathress->GetPositionY(), karathress->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
        const Location& position = SerpentShrineCavernLocations::SharkkisTankPosition;
        const float maxDistance = 2.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, true, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(sharkkis))
            return MoveTo(sharkkis->GetMapId(), sharkkis->GetPositionX(),
                          sharkkis->GetPositionY(), sharkkis->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
        const Location& position = SerpentShrineCavernLocations::TidalvessTankPosition;
        const float maxDistance = 2.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, true, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(tidalvess))
            return MoveTo(tidalvess->GetMapId(), tidalvess->GetPositionX(),
                          tidalvess->GetPositionY(), tidalvess->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
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
        const Location& position = SerpentShrineCavernLocations::CaribdisTankPosition;
        const float maxDistance = 2.0f;

        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, true, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (!bot->IsWithinMeleeRange(caribdis))
            return MoveTo(caribdis->GetMapId(), caribdis->GetPositionX(),
                          caribdis->GetPositionY(), caribdis->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event event)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    Group* group = bot->GetGroup();
    if (!caribdis || !caribdis->IsAlive() || !group)
        return false;

    Player* caribdisTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && botAI->IsAssistTankOfIndex(member, 2))
        {
            caribdisTank = member;
            break;
        }
    }

    if (!caribdisTank)
        return false;

    float distance = bot->GetExactDist2d(caribdisTank);
    const float maxDistance = 35.0f;

    if (distance > maxDistance)
        return FleePosition(caribdisTank->GetPosition(), 30.0f, 0);

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
        // 1st assist tank of index (index 0)
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
        // 2nd assist tank of index (index 1)
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

    // Cast Misdirection on the assigned tank
    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    // Only cast Steady Shot if Misdirection is active
    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    // Cast Steady Shot on the assigned boss
    if (botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

bool FathomLordKarathressAssignMeleeDPSPriorityAction::Execute(Event event)
{
    // Melee target priority 1: Spitfire Totems
    Unit* spitfireTotem = AI_VALUE2(Unit*, "find target", "spitfire totem");
    if (spitfireTotem)
    {
        MarkTargetWithSquare(bot, spitfireTotem);
        SetRtiTarget(botAI, "square", spitfireTotem);

        if (bot->GetVictim() != spitfireTotem)
            return Attack(spitfireTotem);

        return false;
    }

    // Melee target priority 2: Fathom Sporebat
    Unit* sporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (sporebat && sporebat->IsAlive())
    {
        MarkTargetWithCross(bot, sporebat);
        SetRtiTarget(botAI, "cross", sporebat);

        if (bot->GetVictim() != sporebat)
            return Attack(sporebat);

        return false;
    }

    // Melee target priority 3: Sharkkis
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetVictim() != sharkkis)
            return Attack(sharkkis);

        return false;
    }

    // Melee target priority 4: Tidalvess
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetVictim() != tidalvess)
            return Attack(tidalvess);

        return false;
    }

    // Melee target priority 5: Karathress
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetVictim() != karathress)
            return Attack(karathress);
    }

    return false;
}

bool FathomLordKarathressAssignRangedDPSPriorityAction::Execute(Event event)
{
    // Ranged target priority 1: Spitfire Totems
    Unit* totem = AI_VALUE2(Unit*, "find target", "spitfire totem");
    if (totem)
    {
        MarkTargetWithSquare(bot, totem);
        SetRtiTarget(botAI, "square", totem);

        if (bot->GetVictim() != totem)
            return Attack(totem);

        return false;
    }

    // Ranged target priority 2: Fathom Sporebat or Lurker
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomSporebat && fathomSporebat->IsAlive())
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetVictim() != fathomSporebat)
            return Attack(fathomSporebat);

        return false;
    }
    else if (fathomLurker && fathomLurker->IsAlive() && bot->getClass() == CLASS_WARLOCK)
        MarkTargetWithMoon(bot, fathomLurker);

    // Ranged target priority 3: Sharkkis
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->IsAlive())
    {
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetVictim() != sharkkis)
            return Attack(sharkkis);

        return false;
    }

    // Ranged target priority 4: Tidalvess
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess && tidalvess->IsAlive())
    {
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetVictim() != tidalvess)
            return Attack(tidalvess);

        return false;
    }

    // Ranged target priority 5: Caribdis
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (caribdis && caribdis->IsAlive())
    {
        SetRtiTarget(botAI, "diamond", caribdis);

        if (bot->GetVictim() != caribdis)
            return Attack(caribdis);
    }

    // Ranged target priority 6: Karathress
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathress->IsAlive())
    {
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetVictim() != karathress)
            return Attack(karathress);
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

    // Find main tank
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

    // Cast Misdirection on main tank
    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    // Only cast Steady Shot if Misdirection is active
    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    // Cast Steady Shot on Tidewalker
    if (botAI->CanCastSpell("steady shot", tidewalker))
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
        const Location& position = (tidewalker->GetHealthPct() > 25.0f) ? 
            SerpentShrineCavernLocations::TidewalkerPhase1TankPosition :
            SerpentShrineCavernLocations::TidewalkerPhase2TankPosition;

        const float maxDistance = 2.0f;
        float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

        if (distanceToPosition > maxDistance)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

            return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                            MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else if (!bot->IsWithinMeleeRange(tidewalker))
        {
            return MoveTo(tidewalker->GetMapId(), tidewalker->GetPositionX(),
                          tidewalker->GetPositionY(), tidewalker->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool MorogrimTidewalkerPhase2RepositionDPSAndHealersAction::Execute(Event event)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const Location& position = SerpentShrineCavernLocations::TidewalkerPhase2DPSAndHealerPosition;
    const float maxDistance = 8.0f;
    float distanceToPosition = bot->GetExactDist2d(position.x, position.y);

    if (distanceToPosition > maxDistance)
    {
        float dX = position.x - bot->GetPositionX();
        float dY = position.y - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
        float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;

        return MoveTo(bot->GetMapId(), moveX, moveY, position.z, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}
