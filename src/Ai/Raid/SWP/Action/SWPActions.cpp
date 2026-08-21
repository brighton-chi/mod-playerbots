/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "SWPSharedConstants.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include <list>

using namespace SwpHelpers;

bool SunwellPlateauResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();
    uint32 const instanceId = bot->GetInstanceId();
    bool const isMechanicTracker = IsMechanicTrackerBot(bot, SWP_MAP_ID);

    bool didSomething = false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        didSomething |= isMechanicTracker && kalecgosEncounterStates.erase(instanceId) > 0;

        Action* kalecAction = context->GetAction("kalecgos disperse ranged");
        if (kalecAction && static_cast<KalecgosDisperseRangedAction*>(
                kalecAction)->ResetInitialRangedPositionReached())
        {
            didSomething = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
    {
        if (bot->HasAura(Id(SwpSpells::SPELL_BURN)))
        {
            bot->RemoveAura(Id(SwpSpells::SPELL_BURN));
            didSomething = true;
        }

        auto const stateItr = brutallusEncounterStates.find(instanceId);
        if (stateItr != brutallusEncounterStates.end())
            didSomething |= stateItr->second.rangedBurnStates.erase(guid) > 0;

        didSomething |= ReleaseBrutallusBurnPad(bot);

        if (isMechanicTracker)
            didSomething |= brutallusEncounterStates.erase(instanceId) > 0;

        Action* brutallusAction = context->GetAction("brutallus tanks position and swap");
        if (brutallusAction && static_cast<BrutallusTanksPositionAndSwapAction*>(
                brutallusAction)->ResetInitialPositionReached())
        {
            didSomething = true;
        }
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "felmyst"))
        didSomething |= felmystEncounterStates.erase(instanceId) > 0;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        if (isMechanicTracker)
        {
            didSomething |= eredarTwinsIncomingConflagrationStates.erase(instanceId) > 0;
            didSomething |= eredarTwinsDpsHoldStartMs.erase(instanceId) > 0;
        }

        Action* twinsAction = context->GetAction(
            "eredar twins first assist tank move out of blaze");
        if (twinsAction && static_cast<EredarTwinsFirstAssistTankMoveOutOfBlazeAction*>(
                twinsAction)->ResetAlythessTankStep())
        {
            didSomething = true;
        }
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "m'uru"))
    {
        didSomething |= muruDarknessStates.erase(instanceId) > 0;
        didSomething |= muruVoidSentinelTankAssignments.erase(instanceId) > 0;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        didSomething |= kiljaedenEncounterStates.erase(instanceId) > 0;

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
    {
        didSomething |= ResetKiljaedenDragonOrbUserAnnouncement(instanceId);
        didSomething |= kiljaedenHandTankAssignments.erase(instanceId) > 0;
    }

    return didSomething;
}

bool SunwellPlateauRemoveProtectiveAuraAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_MAGE)
    {
        bot->RemoveAura(Id(SwpSpells::SPELL_ICE_BLOCK));
        return true;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bot->RemoveAura(Id(SwpSpells::SPELL_DIVINE_SHIELD));
        return true;
    }

    return false;
}

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 25.0f;
    Creature* volatileFiend = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_VOLATILE_FIEND), searchRadius, true);
    if (!volatileFiend)
        return false;

    if (PlayerbotAI::IsTank(bot))
    {
        if (AI_VALUE(Unit*, "current target") != volatileFiend)
            return Attack(volatileFiend);
    }
    else
    {
        constexpr float safeDistance = 20.0f;
        float const currentDistance = bot->GetDistance(volatileFiend);
        if (currentDistance < safeDistance)
        {
            bot->CastStop();
            return MoveAway(volatileFiend, safeDistance - currentDistance);
        }
    }

    return false;
}

bool ApocalypseGuardAttackWithHolyMagicAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> apocalypseGuards;
    bot->GetCreatureListWithEntryInGrid(
        apocalypseGuards, Id(SwpNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

    for (Creature* apocalypseGuard : apocalypseGuards)
    {
        if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
            !apocalypseGuard->HasAura(Id(SwpSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
            target = apocalypseGuard;
    }

    if (bot->HasAura(Id(SwpSpells::SPELL_SHADOWFORM)))
        bot->RemoveAura(Id(SwpSpells::SPELL_SHADOWFORM));

    return botAI->CanCastSpell("smite", target) && botAI->CastSpell("smite", target);
}

bool SunwellPlateauMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(Id(SwpSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", boss))
    {
        return botAI->CastSpell("steady shot", boss);
    }

    return false;
}
