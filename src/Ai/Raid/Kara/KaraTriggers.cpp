#include "KaraTriggers.h"
#include "KaraHelpers.h"
#include "KaraActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace KarazhanHelpers;

bool KarazhanBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == KARAZHAN_MAP_ID && !AI_VALUE2(bool, "combat", "self target");
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

bool MaidenOfVirtueHolyWrathDealsChainDamageTrigger::IsActive()
{
    return botAI->IsRanged(bot) && AI_VALUE2(Unit*, "find target", "maiden of virtue");
}

bool BigBadWolfBossEngagedByTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot) ||
        bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_LITTLE_RED_RIDING_HOOD)))
    {
        return false;
    }

    return AI_VALUE2(Unit*, "find target", "the big bad wolf");
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
    return IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "moroes");
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
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    return AI_VALUE2(Unit*, "find target", "the curator");
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
    return aran && IsCastingArcaneExplosion(aran) && !IsFlameWreathActive(botAI, bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "shade of aran") && IsFlameWreathActive(botAI, bot);
}

// Exclusion of Banish is so the player may Banish elementals if they wish
bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID))
        return false;

    Unit* elemental = AI_VALUE2(Unit*, "find target", "conjured elemental");
    return elemental && !elemental->HasAura(
        static_cast<uint32>(KarazhanSpells::SPELL_WARLOCK_BANISH));
}

bool ShadeOfAranBossUsesCounterspellAndBlizzardTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && !IsCastingArcaneExplosion(aran) && !IsFlameWreathActive(botAI, bot);
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

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || !IsBanishPhase(netherspite))
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
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
    if (botAI->IsMainTank(bot))
        return false;

    if (bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_ENFEEBLE)))
        return false;

    return AI_VALUE2(Unit*, "find target", "prince malchezaar");
}

bool PrinceMalchezaarBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "prince malchezaar");
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

bool NightbaneMainTankIsSusceptibleToFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "nightbane"))
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    return mainTank &&
        !mainTank->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_FEAR_WARD)) &&
        botAI->CanCastSpell("fear ward", mainTank);
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

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    constexpr uint8 flightPhaseDurationSeconds = 35;

    return nightbaneFlightPhaseStartTimer.find(instanceId) != nightbaneFlightPhaseStartTimer.end() &&
           (now - nightbaneFlightPhaseStartTimer[instanceId] < flightPhaseDurationSeconds);
}

bool NightbaneNeedToManageTimersAndTrackersTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "nightbane");
}
