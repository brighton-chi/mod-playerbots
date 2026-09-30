/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "EncounterHelpers.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "TemporarySummon.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// General

bool SscNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID);
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

// Shared Bosses

bool SscPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
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

    if (!IsHydrossInFrostPhase(AI_VALUE2(Unit*, "find target", "hydross the unstable")))
        return false;

    return GetNearestPlayerInRadius(bot, HYDROSS_FROST_RANGED_SPREAD_DISTANCE);
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

    HydrossDpsHoldWindow const window =
        GetHydrossDpsHoldWindow(AI_VALUE2(Unit*, "find target", "hydross the unstable"));

    // Hunters keep going after the change to misdirect Hydross to the new tank.
    return window == HydrossDpsHoldWindow::BeforePhaseChange ||
        (window == HydrossDpsHoldWindow::AfterPhaseChange && bot->getClass() != CLASS_HUNTER);
}

bool HydrossTheUnstableShouldManagePhaseTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActiveInEncounter()
{
    return IsLurkerSpouting(AI_VALUE2(Unit*, "find target", "the lurker below"));
}

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "the lurker below")) &&
        PlayerbotAI::IsMainTank(bot);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "the lurker below"));
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

// Bots are unable to move across the water via ReachMeleeAction. Only bots with charge moves can
// cross onto the islets to attack Ambushers during the submerge phase. They are then stuck there
// until their charge comes off of cooldown. To resolve, issue a direct move to a land position.
bool TheLurkerBelowMeleeCannotReachTargetTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    // Consider the bot stuck if it is not moving or casting even with a target out of melee range.
    if (bot->isMoving() || bot->IsNonMeleeSpellCast(false))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || bot->IsWithinMeleeRange(target))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && !IsLurkerSpouting(lurker);
}

// Leotheras the Blind

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

    // A rogue can Cloak off the stacks wherever it stands.
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

    // Misdirection is ready, or it is up and waiting for the Steady Shot that spends it.
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
    // Stamped once, at engage
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

// A bot left hanging still has the knockback's generator in its controlled slot once the tosses
// are over; while the aura is up, more tosses are coming and the arc is left to run
bool FathomLordKarathressLiftedByCycloneTrigger::IsActiveInEncounter()
{
    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)) ||
        bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != EFFECT_MOTION_TYPE)
    {
        return false;
    }

    // Only a bot left well off the floor. Any other knockback, such as Knock Away from Sharkkis's
    // pets (a flat shove topping out under half a yard), is left to run its course.
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ <= CYCLONE_DROP_HEIGHT)
        return false;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
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

    return bot->GetExactDist(GetTidewalkerStackPoint(*tidewalker)) >
        TIDEWALKER_RANGED_STACK_RADIUS;
}

// Phase 1 only. To keep bots from chasing murlocs across the room, which is particularly prone to
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

    // Once at its slot, the bot is never pulled back to it in phase 1
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

// Hunters are left free to go after Sporebats, and the Static Charge action moves a holder on its
// own.
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

// Healers too. Healer dps and a priest's wand get a target the tiers allow, never a Sporebat,
// which walks them up into the air, and in phase 2 the target keeps them in their combat engine.
// The phase 2 and phase 3 multipliers keep them from walking to it.
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
        elite->GetVictim() != bot ||
        elite->GetExactDist2d(GetVashjEliteTankPosition(*elite)) <=
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

// Only a new elemental, or a looter who died on the way, needs a looter chosen.
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

// The ranged dps of the cluster nearest the elemental, a ranged dps looter included. A healer
// looter waits beside it instead (see the loot action).
bool LadyVashjBotShouldAttackTaintedElementalTrigger::IsActiveInEncounter()
{
    if (!GetTaintedElementalToKill(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// From the looter's pick until the core is taken from the corpse.
bool LadyVashjBotIsTaintedCoreLooterTrigger::IsActiveInEncounter()
{
    if (!IsDesignatedCoreLooter(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Creature* tainted = GetAssignedTaintedElemental(bot);

    // TEMP LOG
    bool const hasCore = HasTaintedCore(bot);
    if (hasCore && TaintedLogFirstTime(bot, "core"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms looter {} has the core",
            TaintedLogElapsedMs(bot), bot->GetName());
    }
    if (!tainted && TaintedLogFirstTime(bot, "gone"))
    {
        LOG_INFO("playerbots", "[SSC tainted] +{}ms elemental gone, core looted: {}",
            TaintedLogElapsedMs(bot), TaintedLogSeen(bot, "core") ? "yes" : "NO");
    }

    if (!IsTaintedCoreStillToLoot(tainted))
        return false;

    // Nothing to do while the looter waits beside the living elemental
    return !tainted->IsAlive() || bot->GetDistance(tainted) > VASHJ_CORE_LOOT_RANGE;
}

// A core with nowhere to go: in phase 3, with no generator left; from a chain that found no way; or
// still held when the next core is ready to loot. Its Paralyze roots the holder until it leaves the
// bags.
bool LadyVashjBotShouldDestroyTaintedCoreTrigger::IsActiveInEncounter()
{
    if (!HasTaintedCore(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 3)
        return true;

    if (phase != 2)
        return false;

    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (chain && chain->failed)
        return true;

    Creature* nextTainted = GetAssignedTaintedElemental(bot);
    return nextTainted && !nextTainted->IsAlive() && GetTaintedCoreLootSlot(nextTainted) >= 0;
}

// The chain's start or one of its catchers, while it holds the core or is due at its spot.
bool LadyVashjBotIsInTaintedCoreChainTrigger::IsActiveInEncounter()
{
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain || chain->failed)
        return false;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    if (index < 0 && chain->start != bot->GetGUID())
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    TaintedLogGenerators(bot); // TEMP LOG

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
bool LadyVashjBotIsAboveTheGroundTrigger::IsActiveInEncounter()
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
bool LadyVashjBotIsInToxicSporesTrigger::IsActiveInEncounter()
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

    // Already clear in melee range, the action has nothing to do
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
