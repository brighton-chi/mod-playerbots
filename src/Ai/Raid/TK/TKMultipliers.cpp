/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKMultipliers.h"
#include "ChooseTargetActions.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include <ctime>

using namespace TkHelpers;

// Al'ar <Phoenix God>

float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isBlockedMovement =
        dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action);

    if (!isBlockedMovement && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // Block Charge, etc. for non-tanks when not at a platform
    int8 const currentLocationIndex = GetAlarCurrentLocationIndex(alar);
    if (currentLocationIndex < PLATFORM_0_IDX || currentLocationIndex > PLATFORM_3_IDX)
        return 0.0f;

    return 1.0f;
}

float AlarControlMovementMultiplier::GetValue(Action* action)
{
    bool const isDisperseOrFlee =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action);

    if (!isDisperseOrFlee && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action) || dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (isDisperseOrFlee)
        return 0.0f;

    if (!IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    // Enable FollowAction only in non-combat engine in Phase 2
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 0.0f;

    return 1.0f;
}

float AlarDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "al'ar"))
        return 0.0f;

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AlarMoveAwayFromRebirthAction*>(action))
        return 1.0f;

    // Don't block Flame Quills avoidance in case of bad timing for the transition
    if (dynamic_cast<AlarJumpFromPlatformAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return 0.0f;

    if (alar->GetHealthPct() <= 5.0f) // Melee dps activate logic for the P2 transition at 5% HP
        return 0.0f;

    return 1.0f;
}

float AlarControlTauntingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    bool const isFirstAlarTank = IsFirstAlarTank(bot);

    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (bot->HasAura(Id(TkSpells::SPELL_MELT_ARMOR)) && AI_VALUE(Unit*, "current target") == alar)
        return 0.0f;

    if (IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (isFirstAlarTank)
    {
        if (platformIndex != PLATFORM_0_IDX && platformIndex != PLATFORM_2_IDX)
            return 0.0f;
    }
    else // isSecondAlarTank
    {
        if (platformIndex != PLATFORM_1_IDX && platformIndex != PLATFORM_3_IDX)
            return 0.0f;
    }

    return 1.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "void reaver"))
        return 0.0f;

    return 1.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianWrathStayAwayMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(Id(TkSpells::SPELL_SOLARIAN_TRANSFORM)))
        return 1.0f;

    if (HasWrathOfTheAstromancer(bot))
        return 0.0f;

    return 1.0f;
}

float HighAstromancerSolarianDisableMeleeTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Creature* astromancerCreature = astromancer->ToCreature();
        if (astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE)
            return 0.0f;
    }
    else if (AI_VALUE2(Unit*, "find target", "solarium priest"))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasPhase(kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 10;

    auto it = advisorDpsWaitTimer.find(kaelthas->GetMap()->GetInstanceId());
    if (it != advisorDpsWaitTimer.end() && it->second != -1 &&
        (now - it->second) >= dpsWaitSeconds)
    {
        return 1.0f;
    }

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    auto isAdvisorActive = [](Unit* advisor)
    {
        return advisor && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
            !IsFeigningDeath(advisor);
    };

    bool isMainTank = PlayerbotAI::IsMainTank(bot);
    bool isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
    bool isWarlockTank = GetCapernianTank(bot) == bot;

    if ((isAdvisorActive(sanguinar) && isMainTank) ||
        (isAdvisorActive(telonicus) && isFirstAssistTank) ||
        (isAdvisorActive(capernian) && (isMainTank || isWarlockTank)))
    {
        return 1.0f;
    }

    bool shouldHoldDps =
        (isAdvisorActive(sanguinar) && !isMainTank) ||
        (isAdvisorActive(telonicus) && !isFirstAssistTank) ||
        (isAdvisorActive(capernian) && !isMainTank && !isWarlockTank);

    if (shouldHoldDps)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (thaladred && thaladred->GetVictim() == bot)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase != PHASE_NONE && phase != PHASE_FINAL)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasPhase(kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderManageWeaponTankingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe
    if (!IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
        return 1.0f;

    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasPhase(kaelthas) == PHASE_WEAPONS)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderSuppressEquipUpgradeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<EquipUpgradeAction*>(action) &&
        !dynamic_cast<EquipUpgradesPacketAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderManageAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);

    if (!isDpsAssist && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    // TankAssistAction
    if (PlayerbotAI::IsMainTank(bot))
        return 0.0f;

    if (phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS)
    {
        return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderPrepareForPhase3Multiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderHandleAdvisorRolesInPhase3Action*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return 1.0f;

    // Proxy for revival/Kael talk phase (could pick any advisor here)
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, false) ||
        (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be saved until Phase 3
float KaelthasSunstriderDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    bool const isLustAction = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) ||
         dynamic_cast<CastHeroismAction*>(action));

    if (isLustAction && phase == PHASE_WEAPONS)
        return 0.0f;

    if (phase == PHASE_SINGLE_ADVISOR || phase == PHASE_TRANSITION)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
        return 0.0f;

    return 1.0f;
}
