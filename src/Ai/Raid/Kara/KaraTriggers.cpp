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
#include <algorithm>
#include <cmath>

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
bool AttumenTheHuntsmanShouldBeTankedTrigger::IsActiveInEncounter()
{
    // First assist tank
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && !IsAttumenMounted(*midnight) && GetEncounterBoss();
}

bool AttumenTheHuntsmanMidnightShouldBeFocusedTrigger::IsActiveInEncounter()
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || IsAttumenMounted(*midnight) || AI_VALUE(Unit*, "current target") == midnight)
        return false;

    // The first assist tank takes Attumen once he is up.
    return !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) || !GetEncounterBoss();
}

bool AttumenTheHuntsmanMountedShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* attumen = GetMountedAttumen(bot);
    return attumen && attumen->GetVictim() == bot;
}

bool AttumenTheHuntsmanShouldStackBehindTrigger::IsActiveInEncounter()
{
    Unit* attumen = GetMountedAttumen(bot);
    return attumen && attumen->GetVictim() != bot;
}

bool AttumenTheHuntsmanShouldHoldDpsTrigger::IsActiveInEncounter()
{
    return IsAttumenDpsHeld(bot);
}

bool AttumenTheHuntsmanShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && IsAttumenMounted(*midnight);
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

bool MaidenOfVirtueShouldBreakRepentanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* maiden = GetEncounterBoss();
    return maiden && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueShouldGroundHolyFireTrigger::IsActiveInEncounter()
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

bool BigBadWolfChasingRedHoodTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

bool BigBadWolfPaladinShouldProtectRedHoodTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    Player* target = GetBigBadWolfTargetToProtect(*wolf);
    return target && target != bot;
}

// Romulo and Julianne

bool RomuloAndJulianneShouldBeKilledTogetherTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return false;

    return GetOperaActor(bot, "romulo") && GetOperaActor(bot, "julianne");
}

// The Wizard of Oz

bool WizardOfOzShouldPrioritizeTargetsTrigger::IsActiveInEncounter()
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

bool WizardOfOzMageShouldBurnStrawmanTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_MAGE && GetOperaActor(bot, "strawman");
}

// The Curator

bool TheCuratorShouldPrioritizeAstralFlaresTrigger::IsActiveInEncounter()
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
    return aran && IsAranCastingArcaneExplosion(aran);
}

bool ShadeOfAranRangedIsInBlizzardTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(KaraSpells::SPELL_BLIZZARD)) && PlayerbotAI::IsRanged(bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActiveInEncounter()
{
    Unit* aran = GetEncounterBoss();
    return IsFlameWreathInProgress(bot, aran) && !CanReachTargetDuringFlameWreath(botAI, aran);
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

    return !IsAranCastingArcaneExplosion(aran) && !IsFlameWreathInProgress(bot, aran);
}

// Netherspite

bool NetherspiteShouldBlockRedBeamTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    return state &&
        state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID();
}

bool NetherspiteShouldBlockBeamTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state || state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID())
        return false;

    Position const* spot = FindNetherspiteBlockerSpot(*state, bot->GetGUID());
    return spot && bot->GetExactDist2d(*spot) > NETHERSPITE_BEAM_SPOT_TOLERANCE;
}

bool NetherspiteNextBeamBlockerShouldPrepareTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* spot = state ? FindNetherspiteNextSpot(*state, bot->GetGUID()) : nullptr;
    return spot && bot->GetExactDist2d(*spot) > NETHERSPITE_BEAM_SPOT_TOLERANCE;
}

bool NetherspiteShouldPositionAroundBeamsTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state || !AreNetherspitePortalsKnown(*state) ||
        IsNetherspiteBlockerOrNext(*state, bot->GetGUID()))
    {
        return false;
    }

    Unit* netherspite = GetEncounterBoss();
    return netherspite && !IsBanishPhase(netherspite) && !IsNetherspiteStandingSpotGood(
        bot, *netherspite, *state, bot->GetPositionX(), bot->GetPositionY());
}

bool NetherspiteShouldSpreadForNetherbreathTrigger::IsActiveInEncounter()
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    NetherspiteBanishSlot const* slot =
        state ? FindNetherspiteBanishSlot(*state, bot->GetGUID()) : nullptr;
    if (!slot)
        return false;

    if (!IsClearOfVoidZones(*state, bot->GetPositionX(), bot->GetPositionY(), VOID_ZONE_CLEARANCE))
        return true;

    Unit* netherspite = GetEncounterBoss();
    if (!netherspite)
        return false;

    float const gap = Position::NormalizeOrientation(
        netherspite->GetAngle(bot->GetPositionX(), bot->GetPositionY()) -
        netherspite->GetAngle(slot->spot.GetPositionX(), slot->spot.GetPositionY()));
    return std::min(gap, 2.0f * static_cast<float>(M_PI) - gap) >
        NETHERSPITE_BANISH_SLOT_TOLERANCE;
}

bool NetherspiteShouldHoldDpsTrigger::IsActiveInEncounter()
{
    Unit* netherspite = GetEncounterBoss();
    return netherspite && IsNetherspiteDpsHeld(bot, *netherspite);
}

bool NetherspiteShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID) && GetEncounterBoss();
}

// Prince Malchezaar

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActiveInEncounter()
{
    if (!bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return false;

    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    if (bot->GetExactDist(malchezaar) < SHADOW_NOVA_MOVE_DISTANCE ||
        !IsClearOfInfernals(*bot, GetLandedInfernals(botAI), HELLFIRE_CLEARANCE))
    {
        return true;
    }

    Action* action = context->GetAction("prince malchezaar enfeebled bot avoid hazard");
    return action &&
        static_cast<PrinceMalchezaarEnfeebledBotAvoidHazardAction*>(action)->HasDestination();
}

bool PrinceMalchezaarBotInHazardTrigger::IsActiveInEncounter()
{
    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)) ||
        (PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot))
    {
        return false;
    }

    if ((PlayerbotAI::IsRanged(bot) && bot->GetExactDist(malchezaar) < SHADOW_NOVA_MOVE_DISTANCE) ||
        !IsClearOfInfernals(*bot, GetLandedInfernals(botAI), HELLFIRE_CLEARANCE))
    {
        return true;
    }

    Action* action = context->GetAction("prince malchezaar move clear of hazards");
    return action &&
        static_cast<PrinceMalchezaarMoveClearOfHazardsAction*>(action)->HasDestination();
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
    return nightbane && !IsNightbaneFlying(*nightbane);
}

bool NightbaneRangedShouldStackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && !IsNightbaneFlying(*nightbane);
}

bool NightbanePetsShouldBeHeldTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* nightbane = GetEncounterBoss();
    return nightbane && IsNightbaneFlying(*nightbane) != pet->HasReactState(REACT_PASSIVE);
}

bool NightbaneShouldHoldDpsTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    return nightbane && IsNightbaneDpsHeld(bot, *nightbane);
}

bool NightbaneShouldStackInFlightTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || !IsNightbaneFlying(*nightbane))
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    return state && state->nightbaneFlightSpot;
}

bool NightbaneShouldStandDownTrigger::IsActiveInEncounter()
{
    Unit* nightbane = GetEncounterBoss();
    return nightbane && IsNightbaneFlying(*nightbane) &&
        !IsNightbaneFlightPhaseActive(*nightbane) &&
        AI_VALUE(Unit*, "current target") == nightbane;
}

bool NightbaneShouldManageTimersAndTrackersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, KARA_MAP_ID);
}
