/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraTriggers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace KarazhanHelpers;

bool KarazhanBotIsNotInCombatTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        !AI_VALUE2(bool, "combat", "self target");
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN && bot->getClass() != CLASS_PRIEST)
        return false;

    return AI_VALUE2(Unit*, "find target", "nightbane") ||
        AI_VALUE2(Unit*, "find target", "spectral charger") ||
        AI_VALUE2(Unit*, "find target", "the big bad wolf");
}

bool ManaWarpIsAboutToExplodeTrigger::IsActive()
{
    Unit* manaWarp = AI_VALUE2(Unit*, "find target", "mana warp");
    return manaWarp && manaWarp->GetHealthPct() < 15.0f;
}

// Midnight is still present as a separate (invisible) unit after Attumen mounts.
// A Midnight threat list check will capture the entire encounter.
bool AttumenTheHuntsmanPhaseOneActiveTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return false;

    return !GetFirstAliveUnitByEntry(
        botAI, static_cast<uint32>(KarazhanNpcs::NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED));
}

bool AttumenTheHuntsmanPhaseTwoActiveTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return false;

    return GetFirstAliveUnitByEntry(
        botAI, static_cast<uint32>(KarazhanNpcs::NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED));
}

bool AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "midnight");
}

bool MoroesBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "moroes");
}

bool MoroesDpsShouldPrioritizeAddsTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "moroes");
}

bool MaidenOfVirtueBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "maiden of virtue");
}

bool MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem") &&
        AI_VALUE2(Unit*, "find target", "maiden of virtue");
}

bool MaidenOfVirtueHolyWrathDealsChainDamageTrigger::IsActive()
{
    return botAI->IsRanged(bot) && AI_VALUE2(Unit*, "find target", "maiden of virtue");
}

bool BigBadWolfBossEngagedByTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "the big bad wolf"))
        return false;

    return !bot->HasAura(
        static_cast<uint32>(KarazhanSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfBossIsChasingLittleRedRidingHoodTrigger::IsActive()
{
    return bot->HasAura(
        static_cast<uint32>(KarazhanSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID))
        return false;

    return AI_VALUE2(Unit*, "find target", "romulo") &&
        AI_VALUE2(Unit*, "find target", "julianne");
}

bool WizardOfOzNeedTargetPriorityTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID))
        return false;

    static const std::array<const char*, 6> ozTargets =
    {
        "dorothee",
        "tito",
        "roar",
        "strawman",
        "tinhead",
        "the crone"
    };

    for (const char* name : ozTargets)
    {
        if (Unit* target = AI_VALUE2(Unit*, "find target", name))
            return true;
    }

    return false;
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && AI_VALUE2(Unit*, "find target", "strawman");
}

bool TheCuratorAstralFlareSpawnedTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "astral flare");
}

bool TheCuratorBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) && AI_VALUE2(Unit*, "find target", "the curator");
}

bool TheCuratorBossAstralFlaresCastArcingSearTrigger::IsActive()
{
    return botAI->IsRanged(bot) && AI_VALUE2(Unit*, "find target", "the curator");
}

bool TerestianIllhoofNeedTargetPriorityTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "terestian illhoof");
}

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && IsCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "shade of aran") &&
        IsFlameWreathActive(bot);
}

bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "conjured elemental");
}

bool ShadeOfAranBossUsesCounterspellAndBlizzardTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && !IsCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool NetherspiteRedBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_RED_PORTAL), searchRadius);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_BLUE_PORTAL), searchRadius);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_GREEN_PORTAL), searchRadius);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || !IsBanishPhase(netherspite))
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    constexpr float safeDistance = 4.0f;
    for (Unit* vz : voidZones)
    {
        if (bot->GetExactDist2d(vz) < safeDistance)
            return true;
    }

    return false;
}

bool NetherspiteNeedToManageTimersAndTrackersTrigger::IsActive()
{
    if (!botAI->IsTank(bot) && !IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID))
        return false;

    return AI_VALUE2(Unit*, "find target", "netherspite");
}

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_ENFEEBLE));
}

bool PrinceMalchezaarInfernalsAreSpawnedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "prince malchezaar"))
        return false;

    if (bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_ENFEEBLE)))
        return false;

    return !botAI->IsMainTank(bot);
}

bool PrinceMalchezaarBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
        AI_VALUE2(Unit*, "find target", "prince malchezaar");
}

bool NightbaneBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneRangedBotsAreInCharredEarthTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "nightbane"))
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneBossIsFlyingTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 flightPhaseDurationSeconds = 35;

    if (nightbaneFlightPhaseStartTimer.find(instanceId) ==
        nightbaneFlightPhaseStartTimer.end())
    {
        return false;
    }

    return now - nightbaneFlightPhaseStartTimer[instanceId] <
        flightPhaseDurationSeconds;
}

bool NightbaneNeedToManageTimersAndTrackersTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "nightbane");
}
