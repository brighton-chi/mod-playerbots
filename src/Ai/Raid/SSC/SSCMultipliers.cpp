/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "NonCombatActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "ShamanActions.h"
#include "Timer.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"
#include <algorithm>

using namespace SscHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

bool IsBloodlustAction(Player* bot, Action* action)
{
    return bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));
}

} // end anonymous namespace

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID)
        return 1.0f;

    // Stop bots from sitting and drinking in a toxic pool. Come on...
    if (dynamic_cast<DrinkAction*>(action) || dynamic_cast<EatAction*>(action))
        return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
        return 1.0f;

    return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;
}

// Shared Bosses

float SscControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj && GetLadyVashjPhase(vashj) != 1)
        return 0.0f;

    if (AI_VALUE2(Unit*, "find target", "leotheras the blind") &&
        GetLeotherasWarlockTank(bot) && GetActiveLeotherasDemon(bot))
    {
        return 0.0f;
    }

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ||
        AI_VALUE2(Unit*, "find target", "hydross the unstable") ? 0.0f : 1.0f;
}

float SscDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID || botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    bool const isBloodlust = IsBloodlustAction(bot, action);

    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj"))
    {
        int8 const phase = GetLadyVashjPhase(vashj);
        if (phase == 3)
            return 1.0f;

        // Bloodlust/Heroism are phase 3 only; other dps cooldowns can be used from phase 2.
        return !isBloodlust && phase == 2 ? 1.0f : 0.0f;
    }

    if (Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
    {
        // Bloodlust/Heroism are for the murloc waves.
        if (isBloodlust)
            return AI_VALUE2(Unit*, "find target", "tidewalker lurker") ? 1.0f : 0.0f;

        return tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        // Tidalvess is the first kill target; once he is down the rest of the fight is open.
        Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    for (char const* name : { "the lurker below", "hydross the unstable" })
    {
        if (Unit* boss = AI_VALUE2(Unit*, "find target", name))
            return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (Unit* leotheras = GetLeotheras(bot))
        return leotheras->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableOffPhaseTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) && !IsTauntAction(bot, action))
    {
        return 1.0f;
    }

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (IsHydrossInFrostPhase(hydross) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return 0.0f;

    return IsHydrossInNaturePhase(hydross) && PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float HydrossTheUnstableDisablePhaseTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (!IsHydrossPhaseTank(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "hydross the unstable") ? 0.0f : 1.0f;
}

// Phase changes reset threat. Hold DPS from 1s after Marks hit 100% until 5s post-phase change.
float HydrossTheUnstableWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstablePositionAndSwapTanksAction*>(action))
        return 1.0f;

    if (IsHydrossAddTank(bot))
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    bool const frostPhase = IsHydrossInFrostPhase(hydross);
    if (PlayerbotAI::IsTank(bot) &&
        (frostPhase ?
            PlayerbotAI::IsMainTank(bot) : PlayerbotAI::IsAssistTankOfIndex(bot, 0, true)))
    {
        return 1.0f;
    }

    std::unordered_map<uint32, uint32> const& phaseStartTimer =
        frostPhase ? hydrossFrostDpsWaitTimer : hydrossNatureDpsWaitTimer;
    std::unordered_map<uint32, uint32> const& handOverTimer =
        frostPhase ? hydrossChangeToNaturePhaseTimer : hydrossChangeToFrostPhaseTimer;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();
    constexpr uint32 handOverWaitMs = 1 * IN_MILLISECONDS;
    constexpr uint32 phaseStartWaitMs = 5 * IN_MILLISECONDS;

    auto itStart = phaseStartTimer.find(instanceId);
    bool const justChanged =
        itStart == phaseStartTimer.end() || getMSTimeDiff(itStart->second, now) < phaseStartWaitMs;

    auto itHandOver = handOverTimer.find(instanceId);
    bool const aboutToChange =
        itHandOver != handOverTimer.end() && getMSTimeDiff(itHandOver->second, now) >= handOverWaitMs;

    return justChanged || aboutToChange ? 0.0f : 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !IsRepositionAction(bot, action) &&
        (bot->getClass() != CLASS_ROGUE || !dynamic_cast<CastKillingSpreeAction*>(action)))
    {
        return 1.0f;
    }

    if (dynamic_cast<TheLurkerBelowRunAroundBehindBossAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return IsLurkerSpouting(AI_VALUE2(Unit*, "find target", "the lurker below")) ? 0.0f : 1.0f;
}

float TheLurkerBelowMaintainRangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "the lurker below") ? 0.0f : 1.0f;
}

float TheLurkerBelowTanksFocusAssignedGuardianMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    auto const instanceIt = lurkerGuardianTankAssignments.find(bot->GetInstanceId());
    if (instanceIt == lurkerGuardianTankAssignments.end())
        return 1.0f;

    std::vector<Player*> const tanks = GetLurkerGuardianTanks(bot);
    auto const myIt = std::find(tanks.begin(), tanks.end(), bot);
    if (myIt == tanks.end())
        return 1.0f;

    Unit* guardian = botAI->GetUnit(instanceIt->second[std::distance(tanks.begin(), myIt)]);
    return guardian && guardian->IsAlive() ? 0.0f : 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (HasInnerDemon(bot))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && IsLeotherasChannelingWhirlwind(leotheras) ? 0.0f : 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || HasInnerDemon(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return 1.0f;

    if (GetPhase2LeotherasDemon(bot) && dynamic_cast<AttackAction*>(action))
        return 0.0f;

    // Keep Berserk until Phase 3 in case the bear gets Inner Demon.
    if (bot->getClass() == CLASS_DRUID && dynamic_cast<CastBerserkAction*>(action) &&
        !GetPhase3LeotherasDemon(bot))
    {
        return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindFocusOnInnerDemonMultiplier::GetValueInEncounter(Action* action)
{
    if (!HasInnerDemon(bot))
        return 1.0f;

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    // Don't waste time moving. Just kill the Inner Demon asap. Hunters are the exception, as they
    // need to be allowed to attempt kiting to get some shots off.
    if (bot->getClass() != CLASS_HUNTER)
    {
        if (IsRepositionAction(bot, action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindMeleeRunAwayFromChaosBlastAction*>(action) &&
            !dynamic_cast<MeleeAction*>(action))
        {
            return 0.0f;
        }
    }

    if (bot->getClass() == CLASS_DRUID &&
        (dynamic_cast<CastDireBearFormAction*>(action) ||
         dynamic_cast<CastBearFormAction*>(action) ||
         dynamic_cast<CastTreeFormAction*>(action)))
    {
        return 0.0f;
    }

    // Warrior AoE abilities have no threat type.
    if (bot->getClass() == CLASS_WARRIOR &&
        (dynamic_cast<CastThunderClapAction*>(action) ||
         dynamic_cast<CastCleaveAction*>(action) ||
         dynamic_cast<CastChallengingShoutAction*>(action) ||
         dynamic_cast<CastDemoralizingShoutAction*>(action) ||
         dynamic_cast<CastDemoralizingShoutWithoutLifeTimeCheckAction*>(action) ||
         dynamic_cast<CastShockwaveAction*>(action) ||
         dynamic_cast<CastPiercingHowlAction*>(action) ||
         dynamic_cast<CastIntimidatingShoutAction*>(action) ||
         dynamic_cast<CastSweepingStrikesAction*>(action) ||
         dynamic_cast<CastBladestormAction*>(action) ||
         dynamic_cast<CastWhirlwindAction*>(action) ||
         dynamic_cast<CastVigilanceAction*>(action)))
    {
        return 0.0f;
    }

    // Exclude abilities with a target that isn't the bot or the Inner Demon, plus self heals.
    return dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastSnareSpellAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<PartyMemberActionNameSupport*>(action) ||
        dynamic_cast<MainTankActionNameSupport*>(action) ||
        dynamic_cast<GroupBuffSpellAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action) ||
        dynamic_cast<CastInnervateOnHealerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnMeleeAttackerAction*>(action) ? 0.0f : 1.0f;
}

float LeotherasTheBlindMeleeAvoidChaosBlastMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        (bot->getClass() != CLASS_ROGUE || !dynamic_cast<CastKillingSpreeAction*>(action)))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action))
        return 1.0f;

    if (!HasTooManyChaosBlastStacks(bot))
        return 1.0f;

    Creature* leotherasDemon = GetActiveLeotherasDemon(bot);
    return leotherasDemon && leotherasDemon->GetVictim() != bot ? 0.0f : 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    if (HasInnerDemon(bot))
        return 1.0f;

    uint32 const instanceId = leotheras->GetInstanceId();
    uint32 const now = getMSTime();

    if (IsLeotherasHumanoidPhase(bot))
    {
        if (PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanoidPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasHumanoidPhaseDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < LEOTHERAS_HUMANOID_DPS_WAIT_MS)
        {
            return 0.0f;
        }

        // Hold only after the Whirlwind ends; ranged keep attacking from outside it
        auto whirlwind = leotherasWhirlwindEndTime.find(instanceId);
        if (whirlwind == leotherasWhirlwindEndTime.end() || now < whirlwind->second)
            return 1.0f;

        return now - whirlwind->second < LEOTHERAS_HUMANOID_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    Player* warlockTank = GetLeotherasWarlockTank(bot);
    if (IsLeotherasDemonPhase(bot))
    {
        if (warlockTank == bot)
            return 1.0f;

        if (!warlockTank && PlayerbotAI::IsTank(bot))
            return 1.0f;

        constexpr uint32 dpsWaitMsDemonPhase = 12 * IN_MILLISECONDS;
        auto it = leotherasDemonPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < dpsWaitMsDemonPhase ? 0.0f : 1.0f;
    }

    if (IsLeotherasFinalPhase(bot))
    {
        if (warlockTank == bot || PlayerbotAI::IsTank(bot))
            return 1.0f;

        constexpr uint32 dpsWaitMsFinalPhase = 8 * IN_MILLISECONDS;
        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < dpsWaitMsFinalPhase ? 0.0f : 1.0f;
    }

    return 1.0f;
}

// This multiplier is not needed right now because Soulshatter is cast only when there are
// multiple enemies. That's probably not the right approach and should be fixed, so this multiplier
// remains here but commented out in anticipation of a future correction to Soulshatter usage.
// float LeotherasTheBlindDisableWarlockTankSoulshatterMultiplier::GetValueInEncounter(
//     Action* action)
// {
//     if (botAI->GetState() == BOT_STATE_NON_COMBAT)
//         return 1.0f;
//
//     if (bot->getClass() != CLASS_WARLOCK)
//         return 1.0f;
//
//     if (!dynamic_cast<CastSoulshatterAction*>(action))
//         return 1.0f;
//
//     if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
//         return 1.0f;
//
//     return IsLeotherasWarlockTank(bot) && GetActiveLeotherasDemon(bot) ? 0.0f : 1.0f;
// }

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressDisableAutoTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction || castSpellAction->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    auto it = karathressDpsWaitTimer.find(karathress->GetInstanceId());
    if (it == karathressDpsWaitTimer.end())
        return 0.0f;

    constexpr uint32 dpsWaitMs = 12 * IN_MILLISECONDS;
    return getMSTimeDiff(it->second, getMSTime()) < dpsWaitMs ? 0.0f : 1.0f;
}

float FathomLordKarathressMaintainPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

// Morogrim Tidewalker

float MorogrimTidewalkerDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerMaintainPhase2StackingMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > 25.0f)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action))
        return 0.0f;

    return 1.0f;
}

// Lady Vashj <Coilfang Matron>

float LadyVashjSetGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (!IsMainTankInSameSubgroup(bot))
        return 1.0f;

    if (dynamic_cast<CastWindfuryTotemAction*>(action) ||
        dynamic_cast<SetWindfuryTotemAction*>(action) ||
        dynamic_cast<CastWrathOfAirTotemAction*>(action) ||
        dynamic_cast<SetWrathOfAirTotemAction*>(action) ||
        dynamic_cast<CastNatureResistanceTotemAction*>(action) ||
        dynamic_cast<SetNatureResistanceTotemAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action))
        return 0.0f;

    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        !vashj || GetLadyVashjPhase(vashj) != 1)
    {
        return 1.0f;
    }

    return 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsMainTank(bot) || !bot->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Bots should not loot the core with normal looting logic
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<LootAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2 ? 0.0f : 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        !vashj || GetLadyVashjPhase(vashj) != 2)
    {
        return 1.0f;
    }

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    auto coreHandlers = GetCoreHandlers(botAI, bot);

    bool isCoreHandler = false;
    for (int i = 0; i < static_cast<int>(coreHandlers.size()); ++i)
    {
        if (coreHandlers[i] && coreHandlers[i] == bot)
            isCoreHandler = true;
    }
    if (!isCoreHandler)
        return 1.0f;

    auto hasCore = [](Player* player)
    {
        return player && player->HasItemCount(Id(SscItems::ITEM_TAINTED_CORE), 1, false);
    };

    // If the bot actually has the core, only allow core handling
    if (hasCore(bot) && !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
        return 0.0f;

    // The designated looter must stay on the Tainted Elemental until it has the core.
    if (botAI->HasCheat(BotCheatMask::raid) && bot == coreHandlers[0] && !hasCore(bot) &&
        dynamic_cast<LadyVashjAssignPhase2AndPhase3DpsPriorityAction*>(action))
    {
        constexpr float corpseSearchRadius = 30.0f;
        if (AI_VALUE2(Unit*, "find target", "tainted elemental") ||
            bot->FindNearestCreature(Id(SscNpcs::NPC_TAINTED_ELEMENTAL), corpseSearchRadius, false))
            return 0.0f;
    }

    // First and second passers block movement when the looter teleports to the elemental
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && coreHandlers[0] && coreHandlers[0]->GetExactDist2d(tainted) < 5.0f &&
        (bot == coreHandlers[1] || bot == coreHandlers[2]) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action)))
        return 0.0f;

    // If any prior handler (including self) recently had the core, block other movement
    if (AnyRecentCoreInInventory(botAI, bot) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
        return 0.0f;

    return 1.0f;
}

// All of phases 2 and 3 require a custom movement and targeting system
// So the standard target selection system must be disabled
float LadyVashjDisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action *action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    int8 phase = GetLadyVashjPhase(vashj);

    if (phase == 2)
    {
        if (botAI->GetState() == BOT_STATE_COMBAT &&
            (dynamic_cast<DpsAssistAction*>(action) ||
             dynamic_cast<TankAssistAction*>(action)))
        {
            return 0.0f;
        }

        if (dynamic_cast<FleeAction*>(action))
            return 0.0f;

        if (bot->GetExactDist2d(vashj) < 60.0f &&
            dynamic_cast<FollowAction*>(action))
            return 0.0f;

        if (!PlayerbotAI::IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        if (enchanted && AI_VALUE(Unit*, "current target") == enchanted &&
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
            return 0.0f;
    }

    if (phase == 3)
    {
        if (botAI->GetState() == BOT_STATE_COMBAT &&
            (dynamic_cast<DpsAssistAction*>(action) ||
             dynamic_cast<TankAssistAction*>(action)))
        {
            return 0.0f;
        }

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
        Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
        if (enchanted || strider || elite)
        {
            if (dynamic_cast<FollowAction*>(action) ||
                dynamic_cast<FleeAction*>(action))
                return 0.0f;

            if (enchanted && AI_VALUE(Unit*, "current target") == enchanted &&
                dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
        }
        else if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjSaveHandOfFreedomMultiplier::GetValueInEncounter(Action *action)
{
    if (botAI->GetState() != BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_PALADIN)
        return 1.0f;

    if (!dynamic_cast<CastHandOfFreedomOnPartyAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 ? 0.0f : 1.0f;
}
