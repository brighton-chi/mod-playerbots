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
#include "WarlockActions.h"
#include "WarriorActions.h"
#include <algorithm>

using namespace SscHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsEnchantedElemental(Unit* unit)
{
    return unit && unit->GetEntry() == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL);
}

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

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

bool IsMeleeReachSpell(Player* bot, Action* action)
{
    return dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (bot->getClass() == CLASS_ROGUE && dynamic_cast<CastKillingSpreeAction*>(action));
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

        if (GetActiveLeotherasDemon(botAI) && GetLeotherasWarlockTank(bot))
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

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

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
        // Bloodlust/Heroism are saved for the last phase, once the raid is stacked in the corner.
        if (isBloodlust)
            return tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_HEALTH_PCT ? 1.0f : 0.0f;

        return tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        // Held until Tidalvess, the first council member in the kill order, is engaged
        Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    for (char const* name : { "the lurker below", "hydross the unstable" })
    {
        if (Unit* boss = AI_VALUE2(Unit*, "find target", name))
            return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (Unit* leotheras = GetLeotheras(botAI))
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

    bool const offPhaseTank =
        IsHydrossInFrostPhase(hydross) ? IsHydrossNatureTank(bot) : IsHydrossFrostTank(bot);
    return offPhaseTank ? 0.0f : 1.0f;
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

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || GetHydrossDpsHoldWindow(hydross) == HydrossDpsHoldWindow::None)
        return 1.0f;

    // The tank waiting for its phase is held; the phase tank and add tanks carry on.
    if (PlayerbotAI::IsTank(bot) &&
        !(IsHydrossInFrostPhase(hydross) ? IsHydrossNatureTank(bot) : IsHydrossFrostTank(bot)))
    {
        return 1.0f;
    }

    // Spells cast on the raid don't touch Hydross. Totems are the exception, as some attack.
    if (bot->getClass() == CLASS_SHAMAN && dynamic_cast<CastTotemAction*>(action))
        return 0.0f;

    bool const castOnRaid = dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action);
    return castOnRaid ? 1.0f : 0.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !IsMeleeReachSpell(bot, action) && !IsRepositionAction(bot, action))
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
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

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

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    auto const& assignments = instanceIt->second;
    return std::find(assignments.begin(), assignments.end(), target->GetGUID()) !=
        assignments.end() ? 0.0f : 1.0f;
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
    if (GetPhase2LeotherasDemon(botAI) &&
        (dynamic_cast<TankAssistAction*>(action) ||
         (dynamic_cast<CastSpellAction*>(action) &&
          !dynamic_cast<CastDireBearFormAction*>(action) &&
          !dynamic_cast<CastBearFormAction*>(action))))
    {
        return 0.0f;
    }

    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action) &&
        GetActiveLeotherasDemon(botAI))
    {
        Player* warlockTank = GetLeotherasWarlockTank(bot);
        if (warlockTank && action->GetTarget() == warlockTank)
            return 0.0f;
    }

    // Keep Berserk until Phase 3 in case the bear gets Inner Demon.
    if (bot->getClass() == CLASS_DRUID && dynamic_cast<CastBerserkAction*>(action) &&
        !GetPhase3LeotherasDemon(botAI))
    {
        return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindFocusOnInnerDemonMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!HasInnerDemon(bot))
        return 1.0f;

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    // Don't waste time moving. Just kill the Inner Demon asap.
    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindMeleeRunFromChaosBlastAction*>(action) &&
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

    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) && !IsMeleeReachSpell(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action))
        return 1.0f;

    if (!HasTooManyChaosBlastStacks(bot))
        return 1.0f;

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
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

    if (IsLeotherasHumanoidPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanoidPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasHumanoidPhaseDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < LEOTHERAS_HUMANOID_DPS_WAIT_MS)
        {
            return 0.0f;
        }

        auto whirlwind = leotherasWhirlwindEndTime.find(instanceId);
        if (whirlwind == leotherasWhirlwindEndTime.end() || now < whirlwind->second)
            return 1.0f;

        return now - whirlwind->second < LEOTHERAS_HUMANOID_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    if (IsLeotherasDemonPhase(botAI))
    {
        if (IsLeotherasWarlockTank(bot))
            return 1.0f;

        if (PlayerbotAI::IsTank(bot) && !GetLeotherasWarlockTank(bot))
            return 1.0f;

        auto it = leotherasDemonPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_DEMON_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    if (IsLeotherasFinalPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot) || IsLeotherasWarlockTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end())
            return 0.0f;

        return getMSTimeDiff(it->second, now) < LEOTHERAS_FINAL_DPS_WAIT_MS ? 0.0f : 1.0f;
    }

    return 1.0f;
}

// Soulshatter is eligible to be cast when there are at least two attackers, which is the case in
// the final phase. So this is needed to keep the Warlock tank from dropping threat on the Shadow.
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

    return GetActiveLeotherasDemon(botAI) && IsLeotherasWarlockTank(bot) ? 0.0f : 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    // Hold AoE threat and taunts only when another tank's target is close enough to be hit.
    if (!IsAoeThreatAction(bot, action) && !IsAoeTauntAction(bot, action))
        return 1.0f;

    return IsAnotherCouncilMemberWithin(botAI, KARATHRESS_AOE_THREAT_CLEARANCE) ? 0.0f : 1.0f;
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
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction || castSpellAction->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
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

// Hold casts through the Cyclone and the drop after it. Point moves stall mid-cast, so a cast on
// the way down leaves the bot stuck in the air.
float FathomLordKarathressNoCastingWhileLiftedMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
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

// Out of sight of Caribdis, only the walk to her and the Cyclone drop can move the bot. Spread
// and the stock reach on its old target would just pull it back.
float FathomLordKarathressApproachingCaribdisMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressAssignDpsPriorityAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropFromCycloneAction*>(action))
    {
        return 1.0f;
    }

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return 1.0f;

    // Only while she is the kill target. A totem in reach and Tidalvess come before her.
    if (ShouldAttackSpitfireTotem(bot, GetSpitfireTotem(botAI)) ||
        AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess"))
    {
        return 1.0f;
    }

    return bot->IsWithinLOSInMap(caribdis) ? 1.0f : 0.0f;
}

// Keep the totem and Caribdis targeted when LoS breaks (the ledge, the walk out to her).
// Otherwise the bot drops them and grabs whatever it can see.
float FathomLordKarathressDontDropOutOfSightTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    if (target == GetSpitfireTotem(botAI))
        return 0.0f;

    return target == AI_VALUE2(Unit*, "find target", "fathom-guard caribdis") ? 0.0f : 1.0f;
}

// Morogrim Tidewalker

float MorogrimTidewalkerDisableTankFaceMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "morogrim tidewalker") ? 0.0f : 1.0f;
}

// This doesn't apply en route to the stack, only when actually stacked.
float MorogrimTidewalkerStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<MorogrimTidewalkerStackRangedBehindBossAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return 1.0f;

    return bot->GetExactDist(GetTidewalkerStackPoint(tidewalker)) <=
        TIDEWALKER_RANGED_STACK_RADIUS ? 0.0f : 1.0f;
}

// Lady Vashj <Coilfang Matron>

float LadyVashjSetGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastWindfuryTotemAction*>(action) &&
        !dynamic_cast<SetWindfuryTotemAction*>(action) &&
        !dynamic_cast<CastWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<SetWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<CastNatureResistanceTotemAction*>(action) &&
        !dynamic_cast<SetNatureResistanceTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    // Shock Blast is cast in phases 1 and 3 only
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return 1.0f;

    return GetVashjGroundingShaman(bot) == bot ? 0.0f : 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 1 ? 0.0f : 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValueInEncounter(Action* action)
{
    // Only melee need holding back from a charged tank. Everyone else still has to reach targets to
    // heal or cast, and ReachPartyMemberToHealAction is a ReachTargetAction.
    if (!PlayerbotAI::IsMelee(bot) && !HasVashjStaticCharge(bot))
        return 1.0f;

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !IsMeleeReachSpell(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && ShouldAvoidVashjStaticCharge(bot, vashj) ? 0.0f : 1.0f;
}

// Bots should not loot the core with normal looting logic.
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 1.0f;

    if (!dynamic_cast<LootAction*>(action) && !dynamic_cast<OpenLootAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2 ? 0.0f : 1.0f;
}

// Chain members stay where the chain needs them: the looter beside the elemental until it has the
// core, a holder (rooted anyway), and a catcher walking to or standing on its spot. Only what moves
// them is held, movement actions and spells that carry the caster (charges, Disengage, Blink,
// Killing Spree), so they still heal, cast and attack what is in reach. An AttackAction is a
// MovementAction but only faces its target, so it goes through.
float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain)
        return 1.0f;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    bool const isStart = chain->start == bot->GetGUID();
    if (index < 0 && !isStart)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action) ||
        dynamic_cast<LadyVashjLootTaintedCoreAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !IsMeleeReachSpell(bot, action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        !vashj || GetLadyVashjPhase(vashj) != 2)
    {
        return 1.0f;
    }

    if (HasTaintedCore(bot))
        return 0.0f;

    if (isStart)
    {
        Creature* tainted = GetAssignedTaintedElemental(bot);
        if (tainted && IsTaintedCoreStillToLoot(tainted))
            return 0.0f;
    }

    return index >= 0 && IsVashjCoreCatcherActive(bot, *chain, index) ? 0.0f : 1.0f;
}

// Phases 2 and 3 have their own movement and targeting, so stock targeting and the stock moves that
// would undo it are held.
float LadyVashjPhase2DisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action* action)
{
    bool const isAlwaysBlocked =
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action);
    bool const isReachAction = dynamic_cast<ReachTargetAction*>(action);
    bool const isHealSpell = dynamic_cast<CastHealingSpellAction*>(action);
    bool const isDebuffOnAttacker = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
    bool const isCombatFormationAction = dynamic_cast<CombatFormationMoveAction*>(action);
    bool const isDropTarget = dynamic_cast<DropTargetAction*>(action);
    if (!isAlwaysBlocked && !isReachAction && !isHealSpell && !isDebuffOnAttacker &&
        !isCombatFormationAction && !isDropTarget && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return 1.0f;

    // In either engine. The priority action gives every bot its targets, healers included, and
    // its Attack() switches them into the combat engine; assist would only add targets the tiers
    // don't allow, such as an Elite or Strider no tank has yet.
    // Every bot has its own way back to its post: cluster slots, the idle tank's walk to her, and
    // melee targets chosen by their distance from her
    if (isAlwaysBlocked)
        return 0.0f;

    // Healers keep their combat engine, where their healing is, while they have no target:
    // dropping a missing target would switch them to the non-combat engine
    if (isDropTarget)
        return PlayerbotAI::IsHeal(bot) ? 0.0f : 1.0f;

    // Non-healers heal only as a non-combat action, and they sit in the non-combat engine while
    // they wait for adds, at the start of phase 2 above all. It would only spend their mana.
    if (isHealSpell)
        return PlayerbotAI::IsHeal(bot) ? 1.0f : 0.0f;

    if (isDebuffOnAttacker)
        return IsEnchantedElemental(AI_VALUE(Unit*, "current target")) ? 0.0f : 1.0f;

    // Only getting behind the target goes through, and not behind an Enchanted Elemental, where it
    // gains nothing. The rest would pull bots off their posts, and tank facing doesn't work where a
    // tanking position is set, as for Elites and Striders here.
    if (isCombatFormationAction)
    {
        return dynamic_cast<SetBehindTargetAction*>(action) &&
            !IsEnchantedElemental(AI_VALUE(Unit*, "current target")) ? 1.0f : 0.0f;
    }

    // Cluster ranged shoot from their slots. Only those sent after a Tainted Elemental walk to it,
    // and those stepping in to cast range of a Strider.
    if (PlayerbotAI::IsRangedDps(bot))
    {
        if (isReachAction && IsTankedStriderInStepInReach(bot, AI_VALUE(Unit*, "current target")))
            return 1.0f;

        Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
        return tainted && IsAssignedToAttackTaintedElemental(bot, tainted) ? 1.0f : 0.0f;
    }

    // Cluster healers heal from their slots too. Other healers still reach to heal, but never walk
    // to a target, which healer dps or a priest's wand would.
    if (isReachAction && PlayerbotAI::IsHeal(bot))
    {
        if (GetVashjClusterSlot(bot).cluster >= 0 ||
            !dynamic_cast<ReachPartyMemberToHealAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

// The spore actions dodge pools, and bots move to their own targets.
float LadyVashjPhase3DisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action* action)
{
    bool const isAssist =
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action);
    bool const isDebuffOnAttacker = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);

    if (!isAssist && !isDebuffOnAttacker && !dynamic_cast<AvoidAoeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return 1.0f;

    if (isAssist)
        return botAI->GetState() == BOT_STATE_COMBAT ? 0.0f : 1.0f;

    if (isDebuffOnAttacker)
        return IsEnchantedElemental(AI_VALUE(Unit*, "current target")) ? 0.0f : 1.0f;

    // Two combat formation moves are allowed:
    // (1) Getting behind the target, except an Enchanted Elemental, where it is a waste of time.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return IsEnchantedElemental(target) ? 0.0f : 1.0f;

    // (2) a tank turning an Elite away (Cleave), which works in Phase 3 only because Elites lost
    // their hardcoded tanking positions.
    if (dynamic_cast<TankFaceAction*>(action))
        return target && target->GetEntry() == Id(SscNpcs::NPC_COILFANG_ELITE) ? 1.0f : 0.0f;

    return 0.0f;
}

float LadyVashjSaveHandOfFreedomMultiplier::GetValueInEncounter(Action *action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_PALADIN)
        return 1.0f;

    if (!dynamic_cast<CastHandOfFreedomOnPartyAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 ? 0.0f : 1.0f;
}

// Near a pool, only the melee spore action moves melee dps. Stock reach-melee would take them
// straight back through it.
float LadyVashjMeleeControlSporeAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LadyVashjMeleeMoveAroundToxicSporesAction*>(action) ||
        dynamic_cast<LadyVashjAssignPhase2AndPhase3DpsPriorityAction*>(action) ||
        dynamic_cast<LadyVashjSetGroundingTotemInMainTankGroupAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || !IsVashjRingMelee(bot, vashj))
        return 1.0f;

    return IsNearToxicSpores(botAI, bot, TOXIC_SPORES_MELEE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Stock reach-spell for ranged dps, and reach-to-heal for healers, only while its straight walk is
// clear of pools; otherwise the ranged spore action goes round them.
float LadyVashjRangedDoNotReachThroughSporesMultiplier::GetValueInEncounter(Action* action)
{
    bool const isHealerReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isSpellReach = dynamic_cast<ReachSpellAction*>(action);

    // The reach the helper below measures: a healer's reach-to-heal, anyone else's reach-spell
    if (PlayerbotAI::IsHeal(bot) ? !isHealerReach : !isSpellReach)
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, bot, target, range) ? 0.0f : 1.0f;
}
