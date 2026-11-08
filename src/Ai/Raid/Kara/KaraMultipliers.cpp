/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "ThreatHelpers.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"
#include <array>

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

float KarazhanSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    return IsKarazhanFearCasterActive(bot) ? 0.0f : 1.0f;
}

float KarazhanDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

    std::optional<uint32> const bossId = GetBossInProgress(bot, KARA_MAP_ID);
    if (!bossId)
        return 1.0f;

    Unit* boss = nullptr;
    switch (static_cast<KaraBosses>(*bossId))
    {
        case KaraBosses::DATA_ATTUMEN:
            boss = FindThreatTarget(bot, "midnight");
            break;
        case KaraBosses::DATA_MOROES:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MOROES);
            break;
        case KaraBosses::DATA_MAIDEN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
            break;
        case KaraBosses::DATA_OPERA_PERFORMANCE:
        {
            static constexpr std::array operaLeads = {
                "the big bad wolf",
                "dorothee",
                "romulo",
                "julianne",
            };
            for (char const* name : operaLeads)
            {
                boss = GetOperaActor(bot, name);
                if (boss)
                    break;
            }
            break;
        }
        case KaraBosses::DATA_CURATOR:
            boss = GetKaraBoss(bot, KaraBosses::DATA_CURATOR);
            break;
        case KaraBosses::DATA_TERESTIAN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_TERESTIAN);
            break;
        case KaraBosses::DATA_ARAN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
            break;
        case KaraBosses::DATA_NETHERSPITE:
            boss = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
            break;
        case KaraBosses::DATA_MALCHEZAAR:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
            break;
        case KaraBosses::DATA_NIGHTBANE:
            boss = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
            break;
        default:
            return 1.0f;
    }

    if (!boss)
        return 1.0f;

    if (isBloodlust)
    {
        if (boss->GetEntry() == Id(KaraNpcs::NPC_PRINCE_MALCHEZAAR))
        {
            constexpr float finalPhaseHpThreshold = 30.0f;
            return boss->GetHealthPct() > finalPhaseHpThreshold ? 0.0f : 1.0f;
        }

        if (boss->GetEntry() == Id(KaraNpcs::NPC_THE_CURATOR))
            return !boss->HasAura(Id(KaraSpells::SPELL_CURATOR_EVOCATION)) ? 0.0f : 1.0f;
    }

    if (boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    if (boss->GetEntry() == Id(KaraNpcs::NPC_NIGHTBANE))
        return IsNightbaneFlying(*boss) ? 0.0f : 1.0f;

    if (boss->GetEntry() == Id(KaraNpcs::NPC_NETHERSPITE))
        return IsBanishPhase(boss) ? 0.0f : 1.0f;

    return 1.0f;
}

// Trash

// Stock rotations spend these stuns on the pack (Shockwave and Shadowfury on AoE, Bash in the
// rotation), so every stunner saves its stun for the cast bar.
float ManaWarpSaveStunsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT || !IsNearManaWarpPacks(bot))
        return 1.0f;

    bool isStun = false;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            isStun = dynamic_cast<CastBashAction*>(action) ||
                dynamic_cast<CastBashOnEnemyHealerAction*>(action) ||
                dynamic_cast<CastMaimAction*>(action);
            break;
        }
        case CLASS_PALADIN:
        {
            isStun = dynamic_cast<CastHammerOfJusticeAction*>(action) ||
                dynamic_cast<CastHammerOfJusticeOnEnemyHealerAction*>(action) ||
                dynamic_cast<CastHammerOfJusticeSnareAction*>(action);
            break;
        }
        case CLASS_WARLOCK:
            isStun = dynamic_cast<CastShadowfuryAction*>(action);
            break;
        case CLASS_WARRIOR:
        {
            isStun = dynamic_cast<CastConcussionBlowAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastShockwaveSnareAction*>(action);
            break;
        }
        default:
            break;
    }

    if (isStun)
        return 0.0f;

    return bot->getRace() == RACE_TAUREN && dynamic_cast<CastWarStompAction*>(action) ? 0.0f : 1.0f;
}

// Attumen the Huntsman

float AttumenTheHuntsmanDisableAutoTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    // Mounted Attumen is the only target left, so stock targeting finds him.
    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && !IsAttumenMounted(*midnight) ? 0.0f : 1.0f;
}

float AttumenTheHuntsmanStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<AttumenTheHuntsmanStackBehindAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || !IsAttumenMounted(*midnight))
        return 1.0f;

    // Whoever Attumen is on, and the main tank, move freely; everyone else stays stacked.
    Unit* attumen = GetMountedAttumen(bot);
    if (attumen && attumen->GetVictim() == bot)
        return 1.0f;

    return PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

// Give the main tank 5 seconds to grab aggro when Attumen mounts Midnight. Mounted Attumen is the
// only thing left to attack then, so the hold doesn't need to look at the target.
float AttumenTheHuntsmanHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    return IsAttumenDpsHeld(bot) ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Maiden of Virtue

float MaidenOfVirtueDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

float MaidenOfVirtueSetGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    // Call of the Elements drops whatever air totem the totem bar holds.
    if (!dynamic_cast<CastWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<CastNatureResistanceTotemAction*>(action) &&
        !dynamic_cast<CastWindfuryTotemAction*>(action) &&
        !dynamic_cast<CastCallOfTheElementsAction*>(action))
    {
        return 1.0f;
    }

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

// The Big Bad Wolf

// Moving, turning or casting ends Feign Death, and standing up with Hood on brings him back, so a
// feigning Hood target does nothing until Hood ends.
float BigBadWolfRedHoodHunterStayDownMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER || !bot->HasAura(Id(KaraSpells::SPELL_FEIGN_DEATH)))
        return 1.0f;

    if (!IsLittleRedRidingHood(bot))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

// The Curator

float TheCuratorDisableTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

float TheCuratorDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

// Terestian Illhoof

float TerestianIllhoofDontDotFiendishImpsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // Also the on-attacker DoTs, which pick their own target.
    if (!dynamic_cast<CastDebuffSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->GetEntry() == Id(KaraNpcs::NPC_FIENDISH_IMP) ? 0.0f : 1.0f;
}

// Shade of Aran

float ShadeOfAranArcaneExplosionRunAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranRunAwayFromArcaneExplosionAction*>(action))
    {
        return 1.0f;
    }

    Unit* aran = GetEncounterBoss();
    if (!aran)
        return 1.0f;

    return IsAranCastingArcaneExplosion(aran) ? 0.0f : 1.0f;
}

// Includes set behind, as he faces a random player with each cast.
float ShadeOfAranDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isReactiveMovement =
        dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action);

    return isReactiveMovement ? 0.0f : 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up.
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranStopMovingDuringFlameWreathAction*>(action))
    {
        return 1.0f;
    }

    Unit* aran = GetEncounterBoss();
    if (!IsFlameWreathInProgress(botAI, aran))
        return 1.0f;

    bool const isReachingTarget =
        dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachSpellAction*>(action);

    return isReachingTarget && CanReachTargetDuringFlameWreath(botAI, aran) ? 1.0f : 0.0f;
}

// Netherspite

// The beam area and the banish slots are where every bot stands, so nothing else moves them. Bots
// on a beam or waiting for one don't reach either, except that a red blocker can follow him.
float NetherspiteDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 0.0f;
    }

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state ||
        state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID())
    {
        return 1.0f;
    }

    return IsNetherspiteBlockerOrNext(*state, bot->GetGUID()) ? 0.0f : 1.0f;
}

float NetherspiteHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || !IsNetherspiteDpsHeld(bot, *netherspite))
        return 1.0f;

    return action->GetTarget() == netherspite ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Prince Malchezaar

float PrinceMalchezaarDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isReactiveMovement =
        dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action);

    return isReactiveMovement ? 0.0f : 1.0f;
}

float PrinceMalchezaarEnfeebleMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<PrinceMalchezaarEnfeebledBotAvoidHazardAction*>(action))
    {
        return 1.0f;
    }

    // Disable movement other than escaping Shadow Nova range
    if (dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    // Disable some cooldowns that are used upon low/critical health.
    if (dynamic_cast<CastIceBlockAction*>(action) ||
        dynamic_cast<CastManaShieldAction*>(action) ||
        dynamic_cast<CastDivineShieldAction*>(action) ||
        dynamic_cast<CastLayOnHandsAction*>(action) ||
        dynamic_cast<CastCloakOfShadowsAction*>(action) ||
        dynamic_cast<CastEvasionAction*>(action) ||
        dynamic_cast<CastShamanisticRageAction*>(action) ||
        dynamic_cast<CastBindingHealAction*>(action) ||
        dynamic_cast<CastDesperatePrayerAction*>(action) ||
        dynamic_cast<CastPainSuppressionAction*>(action) ||
        dynamic_cast<CastDispersionAction*>(action) ||
        dynamic_cast<CastEnragedRegenerationAction*>(action) ||
        dynamic_cast<CastLastStandAction*>(action) ||
        dynamic_cast<CastShieldWallAction*>(action) ||
        dynamic_cast<CastSurvivalInstinctsAction*>(action) ||
        dynamic_cast<CastDeterrenceAction*>(action) ||
        dynamic_cast<CastIceboundFortitudeAction*>(action) ||
        dynamic_cast<CastRuneTapAction*>(action) ||
        dynamic_cast<CastVampiricBloodAction*>(action) ||
        dynamic_cast<CastDeathStrikeAction*>(action) ||
        dynamic_cast<CastDeathPactAction*>(action) ||
        dynamic_cast<UseItemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Nightbane

// Pets tend to run out of bounds and cause skeletons to spawn off the map
// Pets also tend to pull adds from inside of the tower through the floor
// This multiplier DOES NOT impact Hunter and Warlock pets
// Hunter and Warlock pets are addressed in ControlPetAggressionAction
float NightbaneDisablePetsMultiplier::GetValueInEncounter(Action* action)
{
    bool const isTemporarySummonSpell =
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastSummonWaterElementalAction*>(action) ||
        dynamic_cast<CastShadowfiendAction*>(action);

    if (!isTemporarySummonSpell && !dynamic_cast<PetAttackAction*>(action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (isTemporarySummonSpell)
        return 0.0f;

    // PetAttackAction
    return IsNightbaneFlying(*nightbane) ? 0.0f : 1.0f;
}

// In flight, a unit hold on Nightbane, with drop target zeroed until he leaves the flight position
// so bots stay in their combat engine fighting skeletons and healing. After each landing, the main
// tank has 8 seconds to get aggro.
float NightbaneHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDropTarget = dynamic_cast<DropTargetAction*>(action);
    if (!isDropTarget && !IsDpsHoldCandidate(bot, action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (isDropTarget)
        return IsNightbaneFlightPhaseActive(*nightbane) ? 0.0f : 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action) || !IsNightbaneDpsHeld(bot, *nightbane))
        return 1.0f;

    return action->GetTarget() == nightbane ? GetDpsHoldValue(bot, action) : 1.0f;
}

float NightbaneDisableAvoidAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane || IsNightbaneFlying(*nightbane))
        return 1.0f;

    return PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float NightbaneDisableMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    bool const isBlockedMovement =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<FleeAction*>(action);

    if (!isReachAction && !isBlockedMovement && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (!IsNightbaneFlying(*nightbane))
        return 1.0f;

    if (isReachAction)
        return 0.0f;

    // Once he leaves the flight position to land, bots freely follow their master
    return IsNightbaneFlightPhaseActive(*nightbane) ? 0.0f : 1.0f;
}
