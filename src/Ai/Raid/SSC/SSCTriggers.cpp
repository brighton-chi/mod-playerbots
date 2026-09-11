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
#include "Timer.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// General
bool SscNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != SSC_MAP_ID)
        return false;

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

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || hydross->GetHealthPct() < 10.0f)
        return false;

    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "pure spawn of hydross") ||
           AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
}

bool HydrossTheUnstableRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_HUNTER ||
        PlayerbotAI::IsHeal(bot) ||
        PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "hydross the unstable");
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
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetInstanceId());
    return it != lurkerSpoutTimer.end() &&
           getMSTimeDiff(it->second, now) < LURKER_SPOUT_DURATION_MS;
}

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetInstanceId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() ||
            getMSTimeDiff(it->second, now) >= LURKER_SPOUT_DURATION_MS);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetInstanceId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() ||
            getMSTimeDiff(it->second, now) >= LURKER_SPOUT_DURATION_MS);
}

// Trigger will be active only if there are at least 3 tanks in the raid
bool TheLurkerBelowIsSubmergedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);

    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    return bot == mainTank || bot == firstAssistTank || bot == secondAssistTank;
}

bool TheLurkerBelowShouldManageSpoutTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "the lurker below");
}

// Leotheras the Blind

bool LeotherasTheBlindDemonFormShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (GetLeotherasDemonFormTank(bot) != bot)
        return false;

    return GetActiveLeotherasDemon(bot);
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (!GetLeotherasDemonFormTank(bot))
        return false;

    return GetPhase2LeotherasDemon(bot);
}

bool LeotherasTheBlindRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    return !leotheras->HasAura(Id(SscSpells::SPELL_LEOTHERAS_BANISHED)) &&
           !leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND)) &&
           !leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND_CHANNEL));
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)))
        return false;

    return leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND)) ||
           leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND_CHANNEL));
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)))
        return false;

    Aura* chaosBlast = bot->GetAura(Id(SscSpells::SPELL_CHAOS_BLAST));
    if (!chaosBlast || chaosBlast->GetStackAmount() < 5)
        return false;

    if (!GetLeotherasDemonFormTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return GetPhase2LeotherasDemon(bot);
}

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)) &&
           GetLeotherasDemonFormTank(bot) != bot;
}

bool LeotherasTheBlindInFinalPhaseTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)))
        return false;

    if (bot->getClass() == CLASS_WARLOCK && GetLeotherasDemonFormTank(bot) == bot)
        return false;

    return GetPhase3LeotherasDemon(bot);
}

bool LeotherasTheBlindDemonFormTankNeedsAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    return !bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER));
}

bool LeotherasTheBlindShouldManageDpsWaitTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "leotheras the blind");
}

// Fathom-Lord Karathress

bool FathomLordKarathressShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressCaribdisShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
}

bool FathomLordKarathressSharkkisShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
}

bool FathomLordKarathressTidalvessShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 2, true) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
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

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    return karathress && karathress->GetHealthPct() > 98.0f;
}

bool FathomLordKarathressDeterminingKillOrderTrigger::IsActiveInEncounter()
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
    return tidewalker && tidewalker->GetHealthPct() > 95.0f;
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
    return tidewalker && tidewalker->GetHealthPct() < 25.0f;
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
