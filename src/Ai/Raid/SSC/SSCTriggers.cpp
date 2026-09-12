/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "Corpse.h"
#include "EncounterHelpers.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include <algorithm>

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
    return PlayerbotAI::IsMainTank(bot) &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableShouldBeTankedByNatureTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

// Non-hunter DPS only. The phase tank's swap action and the healers' casts must not be stopped,
// add tanks are on the spawns, and a hunter's auto-shot with Misdirection up is the hand-off.
bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && bot->getClass() != CLASS_HUNTER &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
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

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter() // THIS IS VERY CLOSE TO THE BELOW RANGED TRIGGER, SHOULD COMBINE
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
        !IsLurkerSpouting(lurker);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
        !IsLurkerSpouting(lurker);
}

// Only the three guardian tanks, and only when all three exist
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

// Leotheras the Blind

bool LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (!IsLeotherasWarlockTank(bot))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return GetActiveLeotherasDemon(bot);
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    // If there is no Warlock tank, then traditional tanks will have to do it.
    if (!GetLeotherasWarlockTank(bot))
        return false;

    return GetPhase2LeotherasDemon(bot);
}

bool LeotherasTheBlindRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return !IsLeotherasChannelingWhirlwind(leotheras);
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    if (HasInnerDemon(bot))
        return false;

    return IsLeotherasChannelingWhirlwind(leotheras);
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    if (!HasTooManyChaosBlastStacks(bot))
        return false;

    Creature* leotherasDemon = GetActiveLeotherasDemon(bot);
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

    if (HasInnerDemon(bot))
        return false;

    if (IsLeotherasWarlockTank(bot))
        return false;

    return IsLeotherasFinalPhase(bot);
}

bool LeotherasTheBlindWarlockTankNeedsAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    return !HasInnerDemon(bot);
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

bool FathomLordKarathressDeterminingKillOrderTrigger::IsActiveInEncounter() // All I have to get healers into combat is non-combat engine exception from dps assist. will it work?
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
        return !AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, true))
        return !AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");

    return false;
}

bool FathomLordKarathressShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
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

bool MorogrimTidewalkerInPhase2Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() < TIDEWALKER_PHASE_2_HEALTH_PCT;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 phase = GetLadyVashjPhase(vashj);
    return phase == 1 || phase == 3;
}

bool LadyVashjRangedShouldSpreadInPhase1Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 1;
}

bool LadyVashjShamanShouldGroundShockBlastTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return false;

    return IsMainTankInSameSubgroup(bot);
}

bool LadyVashjStaticChargeOnGroupMemberTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)))
            return true;
    }

    return false;
}

bool LadyVashjPullingBossInPhase1AndPhase3Trigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    return (vashj->GetHealthPct() <= 100.0f && vashj->GetHealthPct() > 90.0f) ||
           (!vashj->HasUnitState(UNIT_STATE_ROOT) && vashj->GetHealthPct() <= 50.0f &&
            vashj->GetHealthPct() > 40.0f);
}

bool LadyVashjAddsSpawnInPhase2AndPhase3Trigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj")
    if (!vashj)
        return false;

    int8 phase = GetLadyVashjPhase(vashj);
    return phase == 2 || phase == 3;
}

bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "coilfang strider");
}

// Striders are not tankable without a cheat to block Fear so there is no point in misdirecting
// if raid cheats are not enabled. Unlike a boss pull, a strider already on its tank needs nothing.
bool LadyVashjHunterShouldMisdirectStriderTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER || !botAI->HasCheat(BotCheatMask::raid))
        return false;

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    if (!strider)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    return firstAssistTank && strider->GetVictim() != firstAssistTank;
}

bool LadyVashjTaintedElementalCheatTrigger::IsActiveInEncounter()
{
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return false;

    bool taintedPresent = false;
    if (AI_VALUE2(Unit*, "find target", "tainted elemental"))
    {
        taintedPresent = true;
    }
    else
    {
        GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
        for (auto const& guid : corpses)
        {
            LootObject loot(bot, guid);
            WorldObject* object = loot.GetWorldObject(bot);
            if (!object)
                continue;

            if (Creature* creature = object->ToCreature();
                creature->GetEntry() == Id(SscNpcs::NPC_TAINTED_ELEMENTAL) && !creature->IsAlive())
            {
                taintedPresent = true;
                break;
            }
        }
    }

    if (!taintedPresent)
        return false;

    return GetDesignatedCoreLooter(botAI, bot) == bot &&
           !bot->HasItemCount(Id(SscItems::ITEM_TAINTED_CORE), 1, false);
}

bool LadyVashjTaintedCoreWasLootedTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    auto coreHandlers = GetCoreHandlers(botAI, bot);

    bool isCoreHandler = false;
    for (Player* handler : coreHandlers)
        if (handler == bot)
            isCoreHandler = true;

    if (!isCoreHandler)
        return false;

    // First and second passers move to positions as soon as the elemental appears
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && coreHandlers[0] && coreHandlers[0]->GetExactDist2d(tainted) < 5.0f &&
        (bot == coreHandlers[1] || bot == coreHandlers[2]))
        return true;

    // Main logic: run if core is in play for this bot or a prior handler
    return AnyRecentCoreInInventory(botAI, bot);
}

bool LadyVashjInPhase3Trigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj")
    return vashj && GetLadyVashjPhase(vashj) == 3;
}

bool LadyVashjEntangleOnMeleeInPhase3Trigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)))
            continue;

        if (PlayerbotAI::IsMelee(member))
            return true;
    }

    return false;
}
