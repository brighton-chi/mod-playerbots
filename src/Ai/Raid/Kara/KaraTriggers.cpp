/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraTriggers.h"
#include "EncounterHelpers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

bool KarazhanNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, KARA_MAP_ID);
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    return IsKarazhanFearCasterActive(bot) && !AI_VALUE2(bool, "has totem", "tremor totem");
}

// Trash

bool ManaWarpCastsWarpBreachTrigger::IsActive()
{
    return GetManaWarpToStun(botAI, WARP_BREACH_MIN_REACTION_MS);
}

bool ManaWarpStunnerShouldHoldTrigger::IsActive()
{
    return GetManaWarpToHoldFor(botAI);
}

// Attumen the Huntsman

// Midnight is still present as a separate (invisible) unit after Attumen mounts.
// A Midnight threat list check will capture the entire encounter.
bool AttumenTheHuntsmanPhaseOneActiveTrigger::IsActiveInEncounter()
{
    return FindThreatTarget(bot, "midnight") && !GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTwoActiveTrigger::IsActiveInEncounter()
{
    return FindThreatTarget(bot, "midnight") && GetAttumenMounted(bot);
}

bool AttumenTheHuntsmanPhaseTransitionTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    if (!FindThreatTarget(bot, "midnight"))
        return false;

    return GetAttumenMounted(bot);
}

// Moroes

bool MoroesShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && GetEncounterBoss();
}

// Maiden of Virtue

bool MaidenOfVirtueShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!GetEncounterBoss())
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem");
}

bool MaidenOfVirtueRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && GetEncounterBoss();
}

// The Big Bad Wolf

bool BigBadWolfShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetOperaActor(bot, "the big bad wolf"))
        return false;

    return !bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfChasingLittleRedRidingHoodTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

// Romulo and Julianne

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    return GetOperaActor(bot, "romulo") && GetOperaActor(bot, "julianne");
}

// The Wizard of Oz

bool WizardOfOzNeedTargetPriorityTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    for (char const* actor : OZ_TARGETS)
    {
        if (GetOperaActor(bot, actor))
            return true;
    }

    return false;
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_MAGE && GetOperaActor(bot, "strawman");
}

// The Curator

bool TheCuratorAstralFlareSpawnedTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && FindThreatTarget(bot, "astral flare");
}

bool TheCuratorShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool TheCuratorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && GetEncounterBoss();
}

// Terestian Illhoof

bool TerestianIllhoofShouldPrioritizeChainsTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && GetEncounterBoss();
}

// Shade of Aran

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActiveInEncounter()
{
    Unit* aran = GetEncounterBoss();
    return aran && IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss() && IsFlameWreathActive(bot);
}

bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && FindThreatTarget(bot, "conjured elemental");
}

bool ShadeOfAranRangedShouldMaintainDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* aran = GetEncounterBoss();
    if (!aran)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)))
        return false;

    return !IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(bot);
}

// Netherspite

bool NetherspiteRedBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    constexpr float searchRadius = 150.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteInBanishPhaseTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    return netherspite && IsBanishPhase(netherspite);
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss();
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE));
}

bool PrinceMalchezaarEngagedByNonTanksTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return false;

    if ((PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) ||
        PlayerbotAI::IsMainTank(bot))
    {
        return false;
    }

    return true;
}

bool PrinceMalchezaarShouldBeTankedTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    return (PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) ||
        PlayerbotAI::IsMainTank(bot);
}

// Nightbane

bool NightbaneShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneGroundPhaseEngagedByRangedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbanePetsChaseFlyingBossOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!GetEncounterBoss())
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneInFlightPhaseTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    constexpr uint32 flightPhaseDurationMs = 35 * IN_MILLISECONDS;
    // After 35s, Nightbane goes to land, and bots freely follow their master
    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneFlightPhaseStartTimer)
        return false;

    return getMSTimeDiff(*state->nightbaneFlightPhaseStartTimer, getMSTime()) <
        flightPhaseDurationMs;
}

bool NightbaneBotWentOutOfBoundsTrigger::IsActiveInEncounter()
{
    if (!GetEncounterBoss())
        return false;

    constexpr float outOfBoundsLeeway = 5.0f;
    return bot->GetPositionZ() < NIGHTBANE_GROUND_Z - outOfBoundsLeeway;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return GetEncounterBoss();
}
