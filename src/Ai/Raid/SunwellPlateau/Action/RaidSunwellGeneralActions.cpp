/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <list>

#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "RaidSunwellActions.h"
#include "RaidSunwellBrutallusEncounter.h"
#include "RaidSunwellData.h"
#include "RaidSunwellEredarTwinsEncounter.h"
#include "RaidSunwellFelmystEncounter.h"
#include "RaidSunwellKalecgosEncounter.h"
#include "RaidSunwellKiljaedenEncounter.h"
#include "RaidSunwellMuruEncounter.h"

using namespace SunwellHelpers;

bool SunwellPlateauEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetInstanceId();
    const bool isMechanicTracker = IsMechanicTrackerBot(botAI, bot, SUNWELL_MAP_ID);
    const bool isRanged = botAI->IsRanged(bot);

    bool erased = false;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        if (isRanged && hasReachedKalecgosInitialRangedPosition.erase(guid) > 0)
            erased = true;

        if (!AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor") &&
            !IsKalecgosRealmTransitionGraceActive(bot))
        {
            if (isMechanicTracker && kalecgosEncounterStates.erase(instanceId) > 0)
                erased = true;

            if (kalecgosRealmStates.erase(guid) > 0)
                erased = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
    {
        if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)))
        {
            bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_BURN));
            erased = true;
        }

        if (isRanged && brutallusRangedBurnStates.erase(guid) > 0)
            erased = true;

        if (brutallusMainTankInitialPositionReached.erase(guid) > 0)
            erased = true;

        if (isMechanicTracker && brutallusRangedAssignments.erase(instanceId) > 0)
            erased = true;
    }

    if (isMechanicTracker && !AI_VALUE2(Unit*, "find target", "felmyst"))
    {
        if (felmystRangedAssignments.erase(instanceId) > 0)
            erased = true;

        if (felmystIncomingEncapsulateStates.erase(instanceId) > 0)
            erased = true;

        if (felmystFogOfCorruptionStates.erase(instanceId) > 0)
            erased = true;

        if (felmystDemonicVaporRegionIndices.erase(instanceId) > 0)
            erased = true;

        if (felmystDemonicVaporFirstRegionIndices.erase(instanceId) > 0)
            erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
    {
        if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED)))
        {
            bot->RemoveAura(static_cast<uint32>(SunwellSpells::SPELL_FLAME_TOUCHED));
            erased = true;
        }

        if (botAI->IsTank(bot) && alythessTankStep.erase(guid) > 0)
            erased = true;

        if (isMechanicTracker &&
            eredarTwinsIncomingConflagrationStates.erase(instanceId) > 0)
        {
            erased = true;
        }
    }

    if (isMechanicTracker &&
        !AI_VALUE2(Unit*, "find target", "m'uru") &&
        !AI_VALUE2(Unit*, "find target", "entropius"))
    {
        if (muruDarknessStates.erase(instanceId) > 0)
            erased = true;

        if (muruVoidSentinelTankAssignments.erase(instanceId) > 0)
            erased = true;

        if (muruEntropiusInitialRangedPositionsReached.erase(guid) > 0)
            erased = true;
    }

    if (isMechanicTracker &&
        !AI_VALUE2(Unit*, "find target", "kil'jaeden"))
    {
        if (kiljaedenArmageddons.erase(instanceId) > 0)
            erased = true;

        if (isRanged && kiljaedenRangedArmageddonAssignments.erase(instanceId) > 0)
            erased = true;
    }

    if (isMechanicTracker &&
        !AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
    {
        ResetKiljaedenDragonOrbUserAnnouncement(instanceId);
    }

    return erased;
}

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 25.0f;
    Unit* volatileFiend = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_VOLATILE_FIEND), searchRadius, true);
    if (!volatileFiend)
        return false;

    if (botAI->IsTank(bot))
    {
        if (AI_VALUE(Unit*, "current target") != volatileFiend)
            return Attack(volatileFiend);
    }
    else
    {
        constexpr float safeDistance = 20.0f;
        const float currentDistance = bot->GetDistance2d(volatileFiend);
        if (currentDistance < safeDistance)
            return MoveAway(volatileFiend, safeDistance - currentDistance);
    }

    return false;
}

bool ApocalypseGuardAttackWithHolyMagicAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> apocalypseGuards;
    bot->GetCreatureListWithEntryInGrid(
        apocalypseGuards, static_cast<uint32>(SunwellNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

    for (Creature* apocalypseGuard : apocalypseGuards)
    {
        if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
            !apocalypseGuard->HasAura(static_cast<uint32>(SunwellSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
            target = apocalypseGuard;
    }

    if (botAI->HasAura("shadowform", bot))
        botAI->RemoveAura("shadowform");

    if (botAI->CanCastSpell("smite", target))
        return botAI->CastSpell("smite", target);

    return false;
}
