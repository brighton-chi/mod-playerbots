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
#include <algorithm>
#include <vector>

using namespace SscHelpers;
using namespace EncounterHelpers;

// General

bool SscNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID);
}

// Trash Mobs

bool UnderbogColossusInToxicPoolTrigger::IsActive()
{
    return IsInToxicPool(botAI);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "greyheart tidecaller");
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableShouldBeTankedByFrostTankTrigger::IsActiveInEncounter()
{
    return IsHydrossFrostTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableShouldBeTankedByNatureTankTrigger::IsActiveInEncounter()
{
    return IsHydrossNatureTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
        IsHydrossInFrostPhase(AI_VALUE2(Unit*, "find target", "hydross the unstable"));
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
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
    return PlayerbotAI::IsMainTank(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "the lurker below"));
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
        IsLurkerSurfacedAndCalm(AI_VALUE2(Unit*, "find target", "the lurker below"));
}

bool TheLurkerBelowIsSubmergedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    std::vector<Player*> const tanks = GetLurkerGuardianTanks(bot);
    return std::find(tanks.begin(), tanks.end(), bot) != tanks.end();
}

// Bots are unable to move across the water via ReachMeleeAction. Only bots with charge moves can
// cross onto the isles to attack Ambushers during the submerge phase. They are then stuck there
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

    if (HasInnerDemon(bot) || !GetActiveLeotherasDemon(botAI))
        return false;

    return IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetPhase2LeotherasDemon(botAI))
        return false;

    // If there is no Warlock tank, then traditional tanks will have to tank the demon form.
    return GetLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return false;

    return !IsLeotherasChannelingWhirlwind(leotheras);
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!IsLeotherasChannelingWhirlwind(AI_VALUE2(Unit*, "find target", "leotheras the blind")))
        return false;

    return !HasInnerDemon(bot);
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (!HasTooManyChaosBlastStacks(bot))
        return false;

    Creature* leotherasDemon = GetActiveLeotherasDemon(botAI);
    return leotherasDemon && leotherasDemon->GetVictim() != bot;
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

bool LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return GetActiveLeotherasDemon(botAI);
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
    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
}

bool FathomLordKarathressPullingBossesTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
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
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    return caribdis && bot->GetDistance(caribdis) < CARIBDIS_CYCLONE_SUMMON_RANGE;
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

bool MorogrimTidewalkerPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool MorogrimTidewalkerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
}

bool MorogrimTidewalkerRangedShouldStackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT;
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
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 1 || phase == 3;
}

bool LadyVashjRangedShouldSpreadInPhase1Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 1)
        return false;

    return !HasVashjStaticCharge(bot);
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
    return vashj && GetLadyVashjPhase(vashj) == 3;
}

bool LadyVashjShamanShouldGroundShockBlastTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return false;

    return GetVashjGroundingShaman(bot) == bot;
}

bool LadyVashjStaticChargeOnGroupMemberTrigger::IsActiveInEncounter()
{
    return ShouldAvoidVashjStaticCharge(bot, AI_VALUE2(Unit*, "find target", "lady vashj"));
}

bool LadyVashjPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && vashj->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Healers too. Healer dps and a priest's wand get a target the tiers allow, never a Sporebat,
// which walks them up into the air, and in phase 2 the target keeps them in their combat engine.
// The phase 2 multiplier keeps them from walking to it.
bool LadyVashjAddsSpawnInPhase2AndPhase3Trigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 2 || phase == 3;
}

// Strider actions are predicated on the fact that you will have only one Strider up at once.
// If you have more than one up at a time, you likely do not have the DPS to complete the fight.
bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "coilfang strider");
}

bool LadyVashjCoilfangEliteShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) || elite->GetVictim() != bot)
        return false;

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

// The ranged dps of the cluster nearest the elemental. Its looter waits beside it instead (see
// the loot action).
bool LadyVashjBotShouldAttackTaintedElementalTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    return tainted && IsAssignedToAttackTaintedElemental(bot, tainted);
}

// From the looter's pick until the core is taken from the corpse.
bool LadyVashjBotIsTaintedCoreLooterTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || GetDesignatedCoreLooter(bot) != bot)
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

    return tainted && IsTaintedCoreStillToLoot(tainted);
}

// A core with nowhere to go: in phase 3, with no generator left; from a chain that found no way; or
// still held when the next core is ready to loot. Its Paralyze roots the holder until it leaves the
// bags.
bool LadyVashjBotShouldDestroyTaintedCoreTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 3)
        return HasTaintedCore(bot);

    if (phase != 2)
        return false;

    // In phase 2 only a member of this chain or the one it replaced can hold a core
    VashjCoreChain const* chain = GetVashjCoreChain(bot);
    if (!chain)
        return false;

    ObjectGuid const guid = bot->GetGUID();
    bool const member = chain->start == guid || GetVashjCoreCatcherIndex(*chain, bot) >= 0 ||
        std::find(chain->earlier.begin(), chain->earlier.end(), guid) != chain->earlier.end();
    if (!member)
        return false;

    if (!chain->failed)
    {
        Creature* nextTainted = GetAssignedTaintedElemental(bot);
        if (!nextTainted || nextTainted->IsAlive() || GetTaintedCoreLootSlot(nextTainted) < 0)
            return false;
    }

    return HasTaintedCore(bot);
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

    return index >= 0 && IsVashjCoreCatcherActive(bot, *chain, index);
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
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > 1.5f;
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
    return IsNearToxicSpores(botAI, bot, radius);
}

bool LadyVashjMeleeNearToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 && IsVashjRingMelee(bot, vashj) &&
        IsNearToxicSpores(botAI, bot, TOXIC_SPORES_MELEE_CONTROL_RADIUS);
}

bool LadyVashjRangedReachBlockedByToxicSporesTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, bot, target, range);
}

bool LadyVashjEntangleOnMeleeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // In phase 1 only a melee holding Static Charge needs freeing, and never her target, who
    // doesn't move for it. The stock Hand of Freedom takes the nearest rooted member otherwise.
    Unit* vashjVictim = vashj->GetVictim();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)))
            continue;

        if (phase == 1 && (member == vashjVictim || !HasVashjStaticCharge(member)))
            continue;

        if (PlayerbotAI::IsMelee(member))
            return true;
    }

    return false;
}

bool LadyVashjRogueHasStaticChargeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_ROGUE && HasVashjStaticCharge(bot);
}
