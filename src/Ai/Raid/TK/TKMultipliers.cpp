/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include "ThreatHelpers.h"
#include "WarlockActions.h"

using namespace TkHelpers;
using namespace EncounterHelpers;

// Shared

float TempestKeepDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    std::optional<uint32> const bossId = GetBossInProgress(bot, TK_MAP_ID);
    if (!bossId)
        return 1.0f;

    TkBosses const bossInProgress = static_cast<TkBosses>(*bossId);
    Unit* boss = GetTkBoss(bot, bossInProgress);

    // A boss the bot can't find mid-encounter usually means the bot was just resurrected and isn't
    // on the boss's threat list yet, so hold cooldowns.
    if (!boss)
        return 0.0f;

    switch (bossInProgress)
    {
        case TkBosses::DATA_KAELTHAS_SUNSTRIDER:
        {
            uint32 const phase = GetKaelthasTkPhase(boss);
            if (phase == PHASE_NONE)
                return 1.0f;

            bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
                (dynamic_cast<CastBloodlustAction*>(action) ||
                 dynamic_cast<CastHeroismAction*>(action));

            // Cooldowns other than Bloodlust become available during phase 2.
            if (isBloodlust && phase == PHASE_WEAPONS)
                return 0.0f;

            // Bloodlust is not available until phase 3. Other cooldowns are suppressed during the
            // phase 2->3 transition.
            return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_TRANSITION ? 0.0f : 1.0f;
        }
        // For bosses other than Kael'thas, hold cooldowns until 95%.
        default:
            break;
    }

    return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

// Al'ar <Phoenix God>

float AlarSuppressGapClosersMultiplier::GetValueInEncounter(Action* action)
{
    bool const isBlockedMovement =
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action);

    if (!isBlockedMovement && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    Unit* alar = GetEncounterBoss();
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // Block Charge, etc. for non-tanks when not at a platform.
    int8 const currentLocationIndex = GetAlarCurrentLocationIndex(alar);
    return currentLocationIndex < PLATFORM_0_IDX ||
        currentLocationIndex > PLATFORM_3_IDX ? 0.0f : 1.0f;
}

float AlarControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    // No tank face or set behind in phase 1 (positions are fixed), but allow in phase 2.
    if (!dynamic_cast<TankFaceAction*>(action) && !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    Unit* alar = GetEncounterBoss();
    return alar && IsAlarInPhase2(alar->GetInstanceId()) ? 1.0f : 0.0f;
}

float AlarDisableAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    return dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action) ?
        0.0f : 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AlarMoveAwayFromRebirthAction*>(action))
        return 1.0f;

    // Also don't block Flame Quills avoidance in case of bad timing for the transition.
    if (dynamic_cast<AlarJumpFromPlatformAction*>(action))
        return 1.0f;

    Unit* alar = GetEncounterBoss();
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return 0.0f;

    constexpr float phase1AlmostEndedHpThreshold = 5.0f;
    return alar->GetHealthPct() <= phase1AlmostEndedHpThreshold ? 0.0f : 1.0f;
}

float AlarControlTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsTauntAction(bot, action))
        return 1.0f;

    bool const isFirstAlarTank = IsFirstAlarTank(bot);

    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return 1.0f;

    Unit* alar = GetEncounterBoss();
    if (!alar)
        return 1.0f;

    // Phase 2: do not taunt Al'ar when armor is melted.
    if (IsAlarInPhase2(alar->GetInstanceId()))
    {
        return bot->HasAura(Id(TkSpells::SPELL_MELT_ARMOR)) &&
            (AI_VALUE(Unit*, "current target") == alar ||
             action->GetTarget() == alar) ? 0.0f : 1.0f;
    }

    // Phase 1: only taunt when Al'ar is at an assigned platform.
    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (isFirstAlarTank)
        return platformIndex == PLATFORM_0_IDX || platformIndex == PLATFORM_2_IDX ? 1.0f : 0.0f;

    // Second Al'ar tank.
    return platformIndex == PLATFORM_1_IDX || platformIndex == PLATFORM_3_IDX ? 1.0f : 0.0f;
}

// In phase 1, melee shamans drop totems only up on the platforms. This keeps them from pointlessly
// dropping new totems after jumping off for Flame Quills.
float AlarDisableTotemsBelowPlatformsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastTotemAction*>(action) &&
        !dynamic_cast<CastCallOfTheElementsAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsMelee(bot) || IsAlarInPhase2(bot->GetInstanceId()))
        return 1.0f;

    return bot->GetPositionZ() > ALAR_BALCONY_Z ? 1.0f : 0.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return dynamic_cast<SetBehindTargetAction*>(action) ? 1.0f : 0.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianWrathStayAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
    {
        return 1.0f;
    }

    Unit* astromancer = GetEncounterBoss();
    if (!astromancer || IsInVoidwalkerForm(astromancer))
        return 1.0f;

    return HasWrathOfTheAstromancer(bot) ? 0.0f : 1.0f;
}

float HighAstromancerSolarianDisableMeleeTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    Unit* astromancer = GetEncounterBoss();
    if (!astromancer)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Creature* astromancerCreature = astromancer->ToCreature();
        if (astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE)
            return 0.0f;

        return 1.0f;
    }

    return GetTkAdds(botAI, TkNpcs::NPC_SOLARIUM_PRIEST).empty() ? 1.0f : 0.0f;
}

float HighAstromancerSolarianKeepTremorTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* astromancer = GetEncounterBoss();
    return astromancer && IsInVoidwalkerForm(astromancer) ? 0.0f : 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float KaelthasSunstriderHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) ||
        dynamic_cast<CastFearWardOnMainTankAction*>(action) ||
        dynamic_cast<KaelthasSunstriderAssignAdvisorDpsPriorityAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = GetEncounterBoss();
    return kaelthas && IsKaelthasAdvisorDpsHoldActive(botAI, kaelthas) ?
        GetDpsHoldValue(bot, action) : 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = FindThreatTarget(bot, "thaladred the darkener");
    return thaladred && thaladred->GetVictim() == bot ? 0.0f : 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase != PHASE_NONE && phase != PHASE_FINAL ? 0.0f : 1.0f;
}

float KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase != PHASE_SINGLE_ADVISOR && phase != PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* capernian = FindThreatTarget(bot, "grand astromancer capernian");
    if (!IsAdvisorActive(capernian))
        return 1.0f;

    return IsCapernianTank(bot) ? 0.0f : 1.0f;
}

float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = FindThreatTarget(bot, "grand astromancer capernian");
    if (!capernian)
        return 1.0f;

    return IsAdvisorActive(capernian) ? 0.0f : 1.0f;
}

float KaelthasSunstriderManageWeaponTankingMultiplier::GetValueInEncounter(Action* action)
{
    // Try to keep main tank from grabbing aggro on any weapon other than the axe.
    if (!IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
        return 1.0f;

    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    return GetKaelthasTkPhase(kaelthas) == PHASE_WEAPONS ? 0.0f : 1.0f;
}

float KaelthasSunstriderSuppressEquipUpgradeMultiplier::GetValueInEncounter(Action* action)
{
    return dynamic_cast<EquipUpgradeAction*>(action) ||
        dynamic_cast<EquipUpgradesPacketAction*>(action) ? 0.0f : 1.0f;
}

float KaelthasSunstriderManageAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);
    if (!isDpsAssist && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    // Kael drops every attacker when he becomes unattackable for his RP scene.
    if (botAI->GetState() == BOT_STATE_NON_COMBAT &&
        !(phase == PHASE_FINAL && kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE)))
    {
        return 1.0f;
    }

    if (isDpsAssist)
        return 0.0f;

    // TankAssistAction: all tanks are blocked during advisor and final phases.
    if (phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS || phase == PHASE_FINAL)
        return 0.0f;

    // Only main tank is blocked during weapons/transition phases.
    return PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

// Stop melee from dropping to their non-combat engines during the weapon phase (in the event that
// the only weapons remaining are in range of the axe).
float KaelthasSunstriderKeepMeleeInCombatMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    return kaelthas && GetKaelthasTkPhase(kaelthas) == PHASE_WEAPONS ? 0.0f : 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return dynamic_cast<SetBehindTargetAction*>(action) ? 1.0f : 0.0f;
}

float KaelthasSunstriderPrepareForPhase3Multiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderHandleAdvisorRolesInPhase3Action*>(action))
        return 1.0f;

    Unit* kaelthas = GetEncounterBoss();
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* sanguinar = FindThreatTarget(bot, "lord sanguinar");
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
    {
        if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
            return 1.0f;

        return sanguinar && sanguinar->IsAlive() ? 0.0f : 1.0f;
    }

    if (!sanguinar || !sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 1.0f;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        IsCapernianTank(bot) ? 0.0f : 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE_AURA)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action) ? 1.0f : 0.0f;
}
