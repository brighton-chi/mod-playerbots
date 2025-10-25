#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

using namespace SerpentShrineCavernHelpers;

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
            const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist);
                float moveY = bot->GetPositionY() + (dY / dist);

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
            const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist);
                float moveY = bot->GetPositionY() + (dY / dist);

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
        const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;

        if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist);
            float moveY = bot->GetPositionY() + (dY / dist);

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
            const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist);
                float moveY = bot->GetPositionY() + (dY / dist);

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
            const Location& position = SerpentShrineCavernLocations::HydrossFrostTankPosition;

            if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
            {
                float dX = position.x - bot->GetPositionX();
                float dY = position.y - bot->GetPositionY();
                float dist = sqrt(dX * dX + dY * dY);
                float moveX = bot->GetPositionX() + (dX / dist);
                float moveY = bot->GetPositionY() + (dY / dist);

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
        const Location& position = SerpentShrineCavernLocations::HydrossNatureTankPosition;

        if (bot->GetExactDist2d(position.x, position.y) > 1.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist);
            float moveY = bot->GetPositionY() + (dY / dist);

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

    if (hydross->GetHealthPct() > 99.0f)
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

bool HydrossTheUnstableManageDPSTimersAction::Execute(Event event)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 mapId = hydross->GetMapId();

    if (hydross->GetHealth() == hydross->GetMaxHealth())
    {
        if (hydrossFrostDPSWaitTimer.find(mapId) != hydrossFrostDPSWaitTimer.end())
            hydrossFrostDPSWaitTimer.erase(mapId);

        if (hydrossNatureDPSWaitTimer.find(mapId) != hydrossNatureDPSWaitTimer.end())
            hydrossNatureDPSWaitTimer.erase(mapId);
    }

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

bool LeotherasTheBlindTargetSpellbindersAction::Execute(Event event)
{
    Unit* spellbinder = GetFirstAliveUnitByEntry(botAI, NPC_GREYHEART_SPELLBINDER);
    if (!spellbinder || !spellbinder->IsInCombat())
        return false;

    MarkTargetWithSkull(bot, spellbinder);

    return false;
}

/* bool LeotherasTheBlindDemonFormPositionBossAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    if (bot->GetVictim() != leotherasDemon)
        return Attack(leotherasDemon);

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (leotherasHuman && bot->GetExactDist2d(leotherasHuman) < 15.0f && 
        !(leotherasHuman->HasAura(SPELL_WHIRLWIND) || leotherasHuman->HasAura(SPELL_WHIRLWIND_CHANNEL)))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        MoveAway(leotherasHuman, 20.0f, false);
        return false;
    }

    if (leotherasDemon->GetVictim() == bot)
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
} */

bool LeotherasTheBlindDemonFormPositionBossAction::Execute(Event event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasDemon);
    SetRtiTarget(botAI, "square", leotherasDemon);

    if (bot->GetVictim() != leotherasDemon)
        return Attack(leotherasDemon);

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

/* bool LeotherasTheBlindMarkAndAttackInnerDemonAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<uint8> rtiIcons = 
    {
        RtiTargetValue::squareIndex,
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex,
        RtiTargetValue::triangleIndex
    };
    size_t iconIndex = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref && iconIndex < rtiIcons.size(); ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(SPELL_INSIDIOUS_WHISPER))
            continue;

        if (member == bot)
        {
            Unit* innerDemon = GetFirstAliveUnitByEntry(botAI, NPC_INNER_DEMON);
            if (innerDemon)
            {
                // MarkTargetWithIcon(bot, innerDemon, rtiIcons[iconIndex]);
                // SetRtiTarget(botAI, GetRtiName(rtiIcons[iconIndex]), innerDemon);

                if (bot->GetVictim() != innerDemon)
                    return Attack(innerDemon);

                if (botAI->HasCheat(BotCheatMask::raid))
                {
                    // Healers and tanks immediately kill their demons
                    if (botAI->IsHeal(bot) || botAI->IsTank(bot))
                    {
                        Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
                        return true;
                    }
                    // DPS bots immediately deal 50% damage to their demons
                    if (botAI->IsDps(bot) && innerDemon->GetHealthPct() > 60.0f)
                    {
                        Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 2, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
                        return true;
                    }
                }
            }
        }
        ++iconIndex;
    }

    return false;
} */

bool LeotherasTheBlindMarkAndAttackInnerDemonAction::Execute(Event event)
{
    Unit* innerDemon = GetFirstAliveUnitByEntry(botAI, NPC_INNER_DEMON);
    if (innerDemon && botAI->HasCheat(BotCheatMask::raid))
    {
        // Healers and tanks immediately kill their demons
        if (botAI->IsHeal(bot) || botAI->IsTank(bot))
        {
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
            return true;
        }
        // DPS bots immediately deal 50% damage to their demons
        if (botAI->IsDps(bot) && innerDemon->GetHealthPct() > 60.0f)
        {
            Unit::DealDamage(bot, innerDemon, innerDemon->GetMaxHealth() / 2, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false, true);
            return true;
        }
    }

    return false;
}

bool LeotherasTheBlindFinalPhaseAssignDPSPriorityAction::Execute(Event event)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasHuman || !leotherasDemon)
        return false;

    MarkTargetWithSquare(bot, leotherasHuman);
    SetRtiTarget(botAI, "square", leotherasHuman);

    if (bot->GetVictim() != leotherasHuman)
        return Attack(leotherasHuman);

    if (botAI->IsTank(bot) && leotherasHuman->GetVictim() == bot)
    {
        float dist = leotherasHuman->GetExactDist2d(leotherasDemon);
        if (dist < 25.0f)
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

    // Cast Misdirection on demon form tank
    if (botAI->CanCastSpell("misdirection", demonFormTank))
        return botAI->CastSpell("misdirection", demonFormTank);

    // Only cast Steady Shot if Misdirection is active
    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    // Cast Steady Shot on Leotheras
    if (botAI->CanCastSpell("steady shot", leotherasDemon))
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
        if (leotherasFinalPhaseDPSWaitTimer.find(mapId) != leotherasFinalPhaseDPSWaitTimer.end())
            leotherasFinalPhaseDPSWaitTimer.erase(mapId);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) != leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer.erase(mapId);

        if (leotherasDemonFormDPSWaitTimer.find(mapId) != leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && !leotherasPhase3Demon)
    {
        if (leotherasHumanFormDPSWaitTimer.find(mapId) == leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer[mapId] = time(nullptr);

        if (leotherasDemonFormDPSWaitTimer.find(mapId) != leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer.erase(mapId);
    }
    else if (leotherasPhase2Demon)
    {
        if (leotherasDemonFormDPSWaitTimer.find(mapId) == leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer[mapId] = time(nullptr);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) != leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer.erase(mapId);
    }
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        if (leotherasFinalPhaseDPSWaitTimer.find(mapId) == leotherasFinalPhaseDPSWaitTimer.end())
            leotherasFinalPhaseDPSWaitTimer[mapId] = time(nullptr);

        if (leotherasHumanFormDPSWaitTimer.find(mapId) != leotherasHumanFormDPSWaitTimer.end())
            leotherasHumanFormDPSWaitTimer.erase(mapId);

        if (leotherasDemonFormDPSWaitTimer.find(mapId) != leotherasDemonFormDPSWaitTimer.end())
            leotherasDemonFormDPSWaitTimer.erase(mapId);
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

        if (!bot->IsWithinMeleeRange(karathress))
        {
            return MoveTo(karathress->GetMapId(), karathress->GetPositionX(),
                          karathress->GetPositionY(), bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, true,
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
        const Location& position = SerpentShrineCavernLocations::SharkkisTankPosition;

        if (!bot->IsWithinMeleeRange(sharkkis))
        {
            return MoveTo(sharkkis->GetMapId(), sharkkis->GetPositionX(),
                          sharkkis->GetPositionY(), bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
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
        const Location& position = SerpentShrineCavernLocations::TidalvessTankPosition;

        if (!bot->IsWithinMeleeRange(tidalvess))
        {
            return MoveTo(tidalvess->GetMapId(), tidalvess->GetPositionX(),
                          tidalvess->GetPositionY(), bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
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
        const Location& position = SerpentShrineCavernLocations::CaribdisTankPosition;

        if (!bot->IsWithinMeleeRange(caribdis))
        {
            return MoveTo(caribdis->GetMapId(), caribdis->GetPositionX(),
                          caribdis->GetPositionY(), bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            return MoveTo(bot->GetMapId(), position.x, position.y, bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Karathress healer stays with Karathress tank
bool FathomLordKarathressPositionKarathressTankHealerAction::Execute(Event event)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    Group* group = bot->GetGroup();
    if (!karathress || !karathress->IsAlive() || !group)
        return false;

    Player* karathressTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && botAI->IsMainTank(member))
        {
            karathressTank = member;
            break;
        }
    }
    if (!karathressTank)
        return false;

    if (bot->GetExactDist2d(karathressTank) > 25.0f)
        return FleePosition(karathressTank->GetPosition(), 20.0f, 0);

    return false;
}

// Caribdis healer stays with Caribdis tank
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
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && botAI->IsAssistTankOfIndex(member, 1))
        {
            caribdisTank = member;
            break;
        }
    }
    if (!caribdisTank)
        return false;

    if (bot->GetExactDist2d(caribdisTank) > 25.0f)
        return FleePosition(caribdisTank->GetPosition(), 20.0f, 0);

    return false;
}

// Tidalvess healer stays with Tidalvess tank
bool FathomLordKarathressPositionTidalvessTankHealerAction::Execute(Event event)
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    Group* group = bot->GetGroup();
    if (!tidalvess || !tidalvess->IsAlive() || !group)
        return false;

    Player* tidalvessTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && botAI->IsAssistTankOfIndex(member, 2))
        {
            tidalvessTank = member;
            break;
        }
    }
    if (!tidalvessTank)
        return false;

    if (bot->GetExactDist2d(tidalvessTank) > 25.0f)
        return FleePosition(tidalvessTank->GetPosition(), 20.0f, 0);

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
    Unit* spitfireTotem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
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
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
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

        if (bot->GetExactDist2d(position.x, position.y) > 2.0f)
        {
            float dX = position.x - bot->GetPositionX();
            float dY = position.y - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist);
            float moveY = bot->GetPositionY() + (dY / dist);

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
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

    if (bot->GetExactDist2d(position.x, position.y) > 8.0f)
    {
        float dX = position.x - bot->GetPositionX();
        float dY = position.y - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        float moveX = bot->GetPositionX() + (dX / dist);
        float moveY = bot->GetPositionY() + (dY / dist);

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Lady Vashj
