#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "Playerbots.h"

using namespace SerpentShrineCavernHelpers;

// Trash Mobs

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event event)
{
    Unit* totem = AI_VALUE2(Unit*, "find target", "water elemental totem");
    if (!totem)
        return false;

    MarkTargetWithSkull(bot, totem);
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


// Leotheras the Blind

// human form tank position (MT)
// demon form tank position
// Use warlock to tank demon form? if so, need helper to assign warlock tank
// run from whirlwind
// dps prioritize targets -- inner demon, MC'd party members, leotheras (whatever form is active), leotheras human (final phase)

// timers--hold dps after whirlwind, hold dps after phase change


// Fathom-Lord Karathress

// Each tank assigned to specific naga, go to designated position
// Melee kill order: spitfire totem, fathom lurker/sporebat, tidalvess, sharkkis, karathress
// Ranged kill order: same but caribdis before karathress
// hunters misdirect initial pull - caribdis top priority for misdirects
// consider whether caribdis healing wave needs interrupting (can it be ranged? seems not)
// consider whether to put curse of tongues on caribdis


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

    const Location& position = SerpentShrineCavernLocations::TidewalkerPhase2DPSandHealerPosition;
    const float maxDistance = 3.0f;
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

