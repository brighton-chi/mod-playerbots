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
#include "MoveSpline.h"
#include "NonCombatActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
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

// The taunts that take more than the tank's own target
bool IsAoeTauntAction(Player* bot, Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return dynamic_cast<CastChallengingRoarAction*>(action);
        case CLASS_PALADIN:
            return dynamic_cast<CastRighteousDefenseAction*>(action);
        case CLASS_WARRIOR:
            return dynamic_cast<CastChallengingShoutAction*>(action);
        default:
            return false;
    }
}

} // end anonymous namespace

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID)
        return 1.0f;

    // Don't sit and drink in a toxic pool. Come on...
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

    if (Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind"))
    {
        if (HasInnerDemon(bot))
            return 0.0f;

        if (IsLeotherasChannelingWhirlwind(leotheras))
            return 0.0f;

        if (GetLeotherasWarlockTank(bot) && GetActiveLeotherasDemon(bot))
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
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
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

    // Auto-attack only while the Warlock has him: abilities would spend the rage being banked for
    // an Inner Demon, and the target choosers would take the tanks off him
    if (GetPhase2LeotherasDemon(bot) &&
        (dynamic_cast<TankAssistAction*>(action) ||
         (dynamic_cast<CastSpellAction*>(action) &&
          !dynamic_cast<CastDireBearFormAction*>(action) &&
          !dynamic_cast<CastBearFormAction*>(action))))
    {
        return 0.0f;
    }

    if (bot->getClass() == CLASS_WARRIOR && GetActiveLeotherasDemon(bot))
    {
        Player* warlockTank = GetLeotherasWarlockTank(bot);
        if (!warlockTank)
            return 1.0f;

        if (dynamic_cast<CastVigilanceAction*>(action) && action->GetTarget() == warlockTank)
            return 0.0f;
    }

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

    // Don't waste time moving. Just kill the Inner Demon asap.
    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindMeleeRunAwayFromChaosBlastAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindPositionRangedAction*>(action) &&
        !dynamic_cast<MeleeAction*>(action))
    {
        return 0.0f;
    }

    if (IsRepositionAction(bot, action))
        return 0.0f;

    // Per class: spells that prevent attacking or drop threat, AoEs with no threat type, useless
    // spells, and spells that need to be blocked to facilitate the custom Inner Demon action.
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (dynamic_cast<CastTreeFormAction*>(action))
                return 0.0f;
            break;

        case CLASS_HUNTER:
            if (dynamic_cast<CastDeterrenceAction*>(action) ||
                dynamic_cast<CastFeignDeathAction*>(action) ||
                dynamic_cast<CastWingClipAction*>(action) ||
                dynamic_cast<CastFreezingTrap*>(action) ||
                dynamic_cast<CastExplosiveTrapAction*>(action) ||
                dynamic_cast<CastImmolationTrapAction*>(action) ||
                dynamic_cast<CastAspectOfTheHawkAction*>(action) ||
                dynamic_cast<CastAspectOfTheWildAction*>(action) ||
                dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) ||
                dynamic_cast<CastAspectOfThePackAction*>(action) ||
                dynamic_cast<CastAspectOfTheCheetahAction*>(action) ||
                dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
            {
                return 0.0f;
            }
            break;

        case CLASS_MAGE:
            if (dynamic_cast<CastIceBlockAction*>(action) ||
                dynamic_cast<CastInvisibilityAction*>(action))
            {
                return 0.0f;
            }
            break;

        case CLASS_PALADIN:
            if (dynamic_cast<CastDivineShieldAction*>(action))
                return 0.0f;
            break;

        case CLASS_PRIEST:
            if (dynamic_cast<CastFadeAction*>(action))
                return 0.0f;
            break;

        case CLASS_ROGUE:
            if (dynamic_cast<CastFeintAction*>(action) || dynamic_cast<CastVanishAction*>(action))
                return 0.0f;
            break;

        case CLASS_WARLOCK:
            if (dynamic_cast<CastCurseOfDoomAction*>(action))
                return 0.0f;
            break;

        case CLASS_WARRIOR:
            if (dynamic_cast<CastThunderClapAction*>(action) ||
                dynamic_cast<CastCleaveAction*>(action) ||
                dynamic_cast<CastChallengingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutWithoutLifeTimeCheckAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastPiercingHowlAction*>(action) ||
                dynamic_cast<CastIntimidatingShoutAction*>(action) ||
                dynamic_cast<CastSweepingStrikesAction*>(action) ||
                dynamic_cast<CastVigilanceAction*>(action))
            {
                return 0.0f;
            }
            break;

        default:
            break;
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

        auto it = leotherasDemonPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_DEMON_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    if (IsLeotherasFinalPhase(bot))
    {
        if (warlockTank == bot || PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_FINAL_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    return 1.0f;
}

// Soulshatter is eligible to be cast when there are at least two attackers, which is the case in
// the final phase. This is needed to keep the Warlock tank from dropping threat on the Shadow.
float LeotherasTheBlindDisableTankSoulshatterMultiplier::GetValueInEncounter(
    Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return 1.0f;

    return IsLeotherasWarlockTank(bot) && GetActiveLeotherasDemon(bot) ? 0.0f : 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    // Single-target taunts land on the tank's own target and are how a guard that latched onto
    // the wrong tank, a loose pet or a knocked-off guard is taken back. AoE threat and the AoE
    // taunts are held only while somebody else's council member is close enough to be caught.
    if (IsAoeThreatAction(bot, action) || IsAoeTauntAction(bot, action))
    {
        return IsAnotherCouncilMemberWithin(botAI, KARATHRESS_AOE_THREAT_CLEARANCE) ?
            0.0f : 1.0f;
    }

    return 1.0f;
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
    // Normally I don't blanket exempt healers as a role and instead only let healing spells through
    // only, but this is a pretty chaotic pull so I don't want to limit healers' abilities.
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    auto it = karathressDpsWaitTimer.find(karathress->GetInstanceId());
    if (it == karathressDpsWaitTimer.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) < KARATHRESS_DPS_WAIT_MS ? 0.0f : 1.0f;
}

float FathomLordKarathressMaintainPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action))
        return 0.0f;

    // Caribdis healer needs to maintain position.
    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressPositionCaribdisTankHealerAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropFromCycloneAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "fathom-guard caribdis") ? 0.0f : 1.0f;
}

// Player point movement neither launches nor continues while a cast is up, and a bot lifted by a
// Cyclone still has its target in range. Casting is held through the tosses, and through the drop
// afterwards: a cast started on the way down stops the fall where it is, and by then the trigger
// that issued it has nothing left to fire on.
float FathomLordKarathressNoCastingWhileLiftedMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)))
        return 0.0f;

    // Only a bot that is moving can be partway down, so the height is looked up for no one else
    if (bot->movespline->Finalized())
        return 1.0f;

    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > CYCLONE_DROP_HEIGHT ?
        0.0f : 1.0f;
}

// The walk out to Caribdis is long and out of sight the whole way, and a bot spread out of sight
// once there has the same walk back. Anything else that moves the bot pulls it the other way:
// the spread, and the stock reach on whatever it was shooting before her. Only the walk itself
// and the Cyclone drop are left running.
float FathomLordKarathressApproachingCaribdisMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressAssignDpsPriorityAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropFromCycloneAction*>(action))
    {
        return 1.0f;
    }

    // Healers are ranged too, but the walk is not theirs and they need to move freely
    if (!PlayerbotAI::IsRanged(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return 1.0f;

    // Only while she is the kill target: a totem in reach and Tidalvess come before her
    if (ShouldAttackSpitfireTotem(bot, GetSpitfireTotem(bot)) ||
        AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess"))
    {
        return 1.0f;
    }

    return bot->IsWithinLOSInMap(caribdis) ? 1.0f : 0.0f;
}

// A target out of line of sight is invalid, and the drop hands the bot back to whatever is in
// sight from where it stands. The ledge between Sharkkis and Karathress does that to a totem at
// his feet while melee climb it, and the walk out to Caribdis does it to her. Both stay the
// target as long as they stand.
float FathomLordKarathressKeepTargetOutOfSightMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    if (target == GetSpitfireTotem(bot))
        return 0.0f;

    return target == AI_VALUE2(Unit*, "find target", "fathom-guard caribdis") ? 0.0f : 1.0f;
}

// Morogrim Tidewalker

float MorogrimTidewalkerDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "morogrim tidewalker") ? 0.0f : 1.0f;
}

float MorogrimTidewalkerStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<MorogrimTidewalkerPhase2RepositionRangedAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_HEALTH_PCT ? 0.0f : 1.0f;
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
