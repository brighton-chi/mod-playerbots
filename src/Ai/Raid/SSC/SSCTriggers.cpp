/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "EncounterHelpers.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// Shared

bool SscNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID);
}

bool SscHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool UnderbogColossusInToxicPoolTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID) &&
        IsNearToxicPool(botAI, TOXIC_POOL_HAZARD_RADIUS);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    if (!PlayerbotAI::IsDps(bot) || IsEncounterInProgress(bot, SSC_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "greyheart tidecaller"))
        return false;

    return GetWaterElementalTotem(botAI) && !IsSkullOnWaterElementalTotem(botAI);
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableShouldBeTankedByFrostTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        IsHydrossFrostTank(bot);
}

bool HydrossTheUnstableShouldBeTankedByNatureTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        IsHydrossNatureTank(bot);
}

bool HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || !IsHydrossInFrostPhase(hydross))
        return false;

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, HYDROSS_FROST_RANGED_SPREAD_DISTANCE);
    return nearestPlayer && !PlayerbotAI::IsTank(nearestPlayer);
}

bool HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    // No Mark of the current phase yet means the phase began less than 15s ago.
    return IsHydrossInFrostPhase(hydross) ? HasNoMarkOfHydross(bot) : HasNoMarkOfCorruption(bot);
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    HydrossDpsHoldWindow const window = GetHydrossDpsHoldWindow(hydross);
    // Hunters keep going after the change to misdirect Hydross to the new tank.
    return window == HydrossDpsHoldWindow::BeforePhaseChange ||
        (window == HydrossDpsHoldWindow::AfterPhaseChange && bot->getClass() != CLASS_HUNTER);
}

// Nothing else drops Hydross for a tank other than the phase tank: one sent in on the pull
// (attack my target), or the old phase tank walking back to its spot after a swap.
bool HydrossTheUnstableNonPhaseTankAttackingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    bool const phaseTank =
        IsHydrossInFrostPhase(hydross) ? IsHydrossFrostTank(bot) : IsHydrossNatureTank(bot);
    if (phaseTank)
        return false;

    return bot->GetVictim() == hydross || AI_VALUE(Unit*, "current target") == hydross;
}

bool HydrossTheUnstableShouldManagePhaseTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActiveInEncounter()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && IsLurkerSpouting(lurker);
}

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || !IsLurkerSurfacedAndCalm(lurker))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && IsLurkerSurfacedAndCalm(lurker);
}

bool TheLurkerBelowGuardiansShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    return GetLurkerGuardianTankIndex(botAI) >= 0;
}

// Reach melee can't take melee across the water to an Ambusher or back (absent a charge spell),
// so a direct move to land does it.
bool TheLurkerBelowMeleeCannotReachTargetTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || bot->IsWithinMeleeRange(target))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || IsLurkerSpouting(lurker))
        return false;

    // Stuck: still, with its target out of melee range. Also a bot still running to an add once
    // its target is Lurker; the move to him is forced, so once under way this stops firing.
    if (!bot->isMoving())
        return true;

    return target == lurker &&
        AI_VALUE(LastMovement&, "last movement").priority < MovementPriority::MOVEMENT_FORCED;
}

// In or over the deep water, not the shallows of Lurker's ring: a path between islets can drop
// a bot in next to an islet with no shore to climb.
bool TheLurkerBelowMeleeInWaterTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (bot->GetLiquidData().Status == LIQUID_MAP_NO_WATER)
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || IsLurkerSpouting(lurker))
        return false;

    if (IsDryGround(bot, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    // His return to the tank spot crosses a spillway on purpose
    return !PlayerbotAI::IsMainTank(bot);
}

// Leotheras the Blind

bool LeotherasTheBlindRangedShouldSpreadUponPullTrigger::IsActive()
{
    if (bot->GetMapId() != SSC_MAP_ID || !PlayerbotAI::IsRanged(bot))
        return false;

    Creature* leotheras = GetLeotheras(botAI);
    if (!leotheras || !IsSpellbinderPhase(leotheras))
        return false;

    return GetNearestPlayerInRadius(bot, LEOTHERAS_RANGED_SPREAD_DISTANCE);
}

bool LeotherasTheBlindWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetLeotherasDemonOrShadow(botAI))
        return false;

    return IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindTanksShouldAutoAttackDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetLeotherasDemon(botAI))
        return false;

    // If there is no Warlock tank, then traditional tanks will have to tank the demon form.
    return GetLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindRangedShouldKeepDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return false;

    if (IsLeotherasChannelingWhirlwind(leotheras))
        return false;

    return GetLeotherasHumanoidToAvoid(botAI) || GetChaosBlastTargetToAvoid(botAI);
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!IsLeotherasChannelingWhirlwind(leotheras))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return bot->GetExactDist2d(leotheras) < LEOTHERAS_WHIRLWIND_SAFE_DISTANCE;
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!HasTooManyChaosBlastStacks(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
    if (!leotherasDemon || leotherasDemon->GetVictim() == bot)
        return false;

    if (bot->getClass() == CLASS_ROGUE &&
        !bot->HasSpellCooldown(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS)))
    {
        return true;
    }

    return GetDemonTargetToAvoid(bot, leotherasDemon);
}

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActiveInEncounter()
{
    return HasInnerDemon(bot);
}

bool LeotherasTheBlindInFinalPhaseTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !IsLeotherasFinalPhase(botAI))
        return false;

    return !IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindShouldSeparateBossFromDemonTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetShadowTargetToSeparateFrom(botAI))
        return false;

    return !IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)) &&
        bot->HasSpellCooldown(Id(SscSpells::SPELL_MISDIRECTION_CAST)))
    {
        return false;
    }

    return GetLeotherasDemonOrShadow(botAI);
}

bool LeotherasTheBlindAggroResetsTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || HasInnerDemon(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && IsLeotherasDpsHoldActive(botAI, leotheras);
}

bool LeotherasTheBlindShouldManageDpsWaitTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "leotheras the blind");
}

// Fathom-Lord Karathress

bool FathomLordKarathressTargetsShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressShouldHealCaribdisTankTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return false;

    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true);
}

bool FathomLordKarathressShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    if (PlayerbotAI::IsDps(bot))
        return true;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return !AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return !GetSharkkisTankTarget(botAI);

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, false))
        return !AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");

    return false;
}

bool FathomLordKarathressShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    if (karathressDpsWaitTimer.find(bot->GetInstanceId()) != karathressDpsWaitTimer.end())
        return false;

    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis || bot->GetDistance(caribdis) >= CARIBDIS_CYCLONE_SUMMON_RANGE)
        return false;

    return GetNearestPlayerInRadius(bot, CARIBDIS_RANGED_SPREAD_DISTANCE);
}

// Left hanging after the tosses: the spline never finished, or a toss from mid-air ended at the
// bot's own height. While the aura is up, more tosses are coming.
bool FathomLordKarathressStuckMidairAfterCycloneTrigger::IsActiveInEncounter()
{
    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)))
        return false;

    // A bot with a move under way, the drop itself included, is left to finish it
    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != EFFECT_MOTION_TYPE &&
        !bot->movespline->Finalized())
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    // Only a bot left well off the floor. Any other knockback, such as Knock Away from Sharkkis's
    // pets (a flat shove topping out under half a yard), is left to run its course.
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > CARIBDIS_CYCLONE_DROP_HEIGHT;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "morogrim tidewalker") &&
        PlayerbotAI::IsMainTank(bot);
}

bool MorogrimTidewalkerRangedShouldStackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return false;

    return bot->GetExactDist(GetTidewalkerStackPoint(*bot, *tidewalker)) >
        TIDEWALKER_RANGED_STACK_RADIUS;
}

// This is to keep bots from chasing murlocs across the room, which is particularly prone to
// happening with bots that leave Watery Graves right as murlocs spawn.
bool MorogrimTidewalkerTooFarFromBossTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT &&
        bot->GetExactDist(tidewalker) >= TIDEWALKER_MAX_DISTANCE_FROM_BOSS;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return (phase == 1 || phase == 3) && PlayerbotAI::IsMainTank(bot);
}

bool LadyVashjRangedShouldSpreadInPhase1Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot) || HasVashjStaticCharge(bot))
        return false;

    Action* spreadAction = context->GetAction("lady vashj phase 1 spread ranged in arc");
    if (!spreadAction || static_cast<LadyVashjPhase1SpreadRangedInArcAction*>(
            spreadAction)->HasReachedRangedPosition())
    {
        return false;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 1;
}

bool LadyVashjClusterSlotsNeedHoldersTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2 && HasVashjClusterVacancy(bot);
}

bool LadyVashjShouldHoldClusterInPhase2Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    if (!GetVashjClusterPositionToReturnTo(bot, AI_VALUE(Unit*, "current target")))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// Hunters go after Sporebats, and the Static Charge action moves a holder on its own.
bool LadyVashjRangedShouldPositionInPhase3Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsCaster(bot) || HasVashjStaticCharge(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 && IsVashjPhase3RangedTooClose(bot, vashj);
}

bool LadyVashjMainTankNeedsGroundingShamanTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return (phase == 1 || phase == 3) && !GetVashjGroundingShaman(bot);
}

bool LadyVashjShamanShouldGroundShockBlastTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN || GetVashjGroundingShaman(bot) != bot)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 1 || phase == 3;
}

bool LadyVashjStaticChargeOnGroupMemberTrigger::IsActiveInEncounter()
{
    return IsInVashjStaticChargeReach(bot, AI_VALUE2(Unit*, "find target", "lady vashj"));
}

// Healers too: their dps and a priest's wand get a tier target, never a Sporebat (which walks
// them up into the air), and in phase 2 the target keeps them in their combat engine.
bool LadyVashjShouldAssignTargetPriorityTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 2 || phase == 3;
}

bool LadyVashjTankNeedsFearWardTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && !bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)) &&
        AI_VALUE2(Unit*, "find target", "coilfang strider");
}

bool LadyVashjCoilfangStriderShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* strider = AI_VALUE(Unit*, "current target");
    if (!strider || strider->GetEntry() != Id(SscNpcs::NPC_COILFANG_STRIDER))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && ShouldTankVashjStrider(bot, strider, vashj, GetLadyVashjPhase(vashj));
}

bool LadyVashjCoilfangEliteShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) ||
        elite->GetVictim() != bot || elite->GetExactDist2d(GetVashjEliteTankPosition(*elite)) <=
            VASHJ_ADD_TANK_ARRIVAL_DISTANCE)
    {
        return false;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// Idle means not on an Elite, a Strider, or an Enchanted near her.
bool LadyVashjTankIsIdleAwayFromTheMiddleTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2 ||
        bot->GetExactDist(vashj) < VASHJ_IDLE_TANK_DISTANCE)
    {
        return false;
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return true;

    switch (target->GetEntry())
    {
        case Id(SscNpcs::NPC_COILFANG_ELITE):
        case Id(SscNpcs::NPC_COILFANG_STRIDER):
            return false;
        case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
            return vashj->GetExactDist2d(target) > VASHJ_ENCHANTED_NEAR_HER_DISTANCE;
        default:
            return true;
    }
}

// Phase 2 only, as are the attack and loot triggers below: a core looted in phase 3 has no
// generator left, and its Paralyze would root the looter.
bool LadyVashjTaintedElementalNeedsLooterTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    auto it = vashjTaintedCoreLooter.find(bot->GetInstanceId());
    if (it == vashjTaintedCoreLooter.end() || it->second.tainted != tainted->GetGUID())
        return true;

    Player* looter = ObjectAccessor::GetPlayer(*bot, it->second.looter);
    return !looter || !looter->IsAlive();
}

// A healer looter waits beside it instead (see the loot action).
bool LadyVashjShouldAttackTaintedElementalTrigger::IsActiveInEncounter()
{
    if (!GetTaintedElementalToKill(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

bool LadyVashjTaintedCoreLooterTrigger::IsActiveInEncounter()
{
    if (!IsDesignatedCoreLooter(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Creature* tainted = GetAssignedTaintedElemental(bot);
    if (!IsTaintedCoreStillToLoot(tainted))
        return false;

    // Nothing to do while the looter waits beside the living elemental
    return !tainted->IsAlive() || bot->GetDistance(tainted) > VASHJ_CORE_LOOT_RANGE;
}

// In phase 3 with no generator left, from a chain that found no way, or still held when the next
// core is ready to loot. Its Paralyze roots the holder until it leaves the bags.
bool LadyVashjShouldDestroyTaintedCoreTrigger::IsActiveInEncounter()
{
    if (!HasTaintedCore(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 3)
        return true;

    if (phase != 2)
        return false;

    VashjCorePassingChain const* chain = GetVashjCorePassingChain(bot);
    if (chain && chain->failed)
        return true;

    Creature* nextTainted = GetAssignedTaintedElemental(bot);
    return nextTainted && !nextTainted->IsAlive() && GetTaintedCoreLootSlot(nextTainted) >= 0;
}

bool LadyVashjCorePassingChainMemberTrigger::IsActiveInEncounter()
{
    VashjCorePassingChain const* chain = GetVashjCorePassingChain(bot);
    if (!chain || chain->failed)
        return false;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    if (index < 0 && chain->originBot != bot->GetGUID())
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    if (HasTaintedCore(bot))
        return true;

    if (index < 0)
        return false;

    // Nothing to do on its spot once there. The action marks the arrival, which releases the next
    // catcher, so it still runs on that tick.
    VashjCoreCatcher const& catcher = chain->catchers[index];
    if (catcher.arrived &&
        bot->GetExactDist2d(catcher.spot) <= GetVashjCoreSpotArrivalDistance(*chain, index))
    {
        return false;
    }

    return IsVashjCoreCatcherActive(bot, *chain, index);
}

bool LadyVashjPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 2 && phase != 3)
        return false;

    if (Unit* target = GetVashjPetTarget(botAI, pet, vashj))
        return pet->GetVictim() != target;

    // Nothing worth attacking, so only a pet still on an immune Vashj needs calling back
    return pet->GetVictim() == vashj;
}

// Bots going after Sporebats sometimes walk up into the air, or end up on the pipes above the
// dais. A bot never falls on its own, so it stays up there.
bool LadyVashjBotAboveTheGroundTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    // Search down from the dais, not from the bot, so a bot on the pipes still reads as high
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), VASHJ_PLATFORM_CENTER_POSITION.GetPositionZ());
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > VASHJ_ABOVE_GROUND_HEIGHT;
}

// Melee dps have their own trigger, below.
bool LadyVashjBotInToxicSporesTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || IsVashjRingMelee(bot, vashj))
        return false;

    bool const tanking = vashj->GetVictim() == bot;

    // Shielded bots walk through on their way; her tank's radius is for the melee behind her
    if (!tanking && bot->isMoving() && CanWalkThroughToxicSpores(bot))
        return false;

    float const radius = tanking ? TOXIC_SPORES_TANK_AVOID_RADIUS : TOXIC_SPORES_AVOID_RADIUS;
    return IsNearToxicSpores(botAI, radius);
}

bool LadyVashjMeleeNearToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || !IsVashjRingMelee(bot, vashj) ||
        !IsNearToxicSpores(botAI, TOXIC_SPORES_MELEE_CONTROL_RADIUS))
    {
        return false;
    }

    return !IsInMeleeRangeClearOfSpores(bot, AI_VALUE(Unit*, "current target"),
        GetToxicSporePositions(botAI), TOXIC_SPORES_AVOID_RADIUS);
}

bool LadyVashjRangedReachBlockedByToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsCaster(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, target, range);
}

bool LadyVashjEntangleOnMeleeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_PALADIN &&
        GetVashjHandOfFreedomTarget(botAI, AI_VALUE2(Unit*, "find target", "lady vashj"));
}

bool LadyVashjStaticChargeOnRogueTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_ROGUE && HasVashjStaticCharge(bot);
}
