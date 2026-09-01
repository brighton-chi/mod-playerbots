/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "Timer.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// General
bool SerpentShrineCavernNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != SSC_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// Trash Mobs

bool UnderbogColossusSpawnedToxicPoolAfterDeathTrigger::IsActive()
{
    return bot->HasAura(Id(SscSpells::SPELL_TOXIC_POOL));
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "greyheart tidecaller");
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableBotIsFrostTankTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableBotIsNatureTankTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActive()
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

bool HydrossTheUnstableDangerFromWaterTombsTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() == CLASS_HUNTER ||
        PlayerbotAI::IsHeal(bot) ||
        PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableNeedToManageTimersTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    return it != lurkerSpoutTimer.end() &&
           getMSTimeDiff(it->second, now) < LURKER_SPOUT_DURATION_MS;
}

bool TheLurkerBelowBossIsActiveForMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() ||
            getMSTimeDiff(it->second, now) >= LURKER_SPOUT_DURATION_MS);
}

bool TheLurkerBelowBossCastsGeyserTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 now = getMSTime();

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() ||
            getMSTimeDiff(it->second, now) >= LURKER_SPOUT_DURATION_MS);
}

// Trigger will be active only if there are at least 3 tanks in the raid
bool TheLurkerBelowBossIsSubmergedTrigger::IsActive()
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

bool TheLurkerBelowNeedToPrepareTimerForSpoutTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "the lurker below");
}

// Leotheras the Blind

bool LeotherasTheBlindBossTransformedIntoDemonFormTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (GetLeotherasDemonFormTank(bot) != bot)
        return false;

    return GetActiveLeotherasDemon(bot);
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActive()
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

bool LeotherasTheBlindBossEngagedByRangedTrigger::IsActive()
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

bool LeotherasTheBlindBossChannelingWhirlwindTrigger::IsActive()
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

bool LeotherasTheBlindBotHasTooManyChaosBlastStacksTrigger::IsActive()
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

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActive()
{
    return bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER)) &&
           GetLeotherasDemonFormTank(bot) != bot;
}

bool LeotherasTheBlindEnteredFinalPhaseTrigger::IsActive()
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

bool LeotherasTheBlindDemonFormTankNeedsAggro::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    return !bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER));
}

bool LeotherasTheBlindBossWipesAggroUponPhaseChangeTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "leotheras the blind");
}

// Fathom-Lord Karathress

bool FathomLordKarathressBossEngagedByMainTankTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressCaribdisEngagedByFirstAssistTankTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
}

bool FathomLordKarathressSharkkisEngagedBySecondAssistTankTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
}

bool FathomLordKarathressTidalvessEngagedByThirdAssistTankTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 2, true) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
}

bool FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger::IsActive()
{
    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
}

bool FathomLordKarathressPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    return karathress && karathress->GetHealthPct() > 98.0f;
}

bool FathomLordKarathressDeterminingKillOrderTrigger::IsActive()
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

bool FathomLordKarathressTanksNeedToEstablishAggroTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

// Morogrim Tidewalker

bool MorogrimTidewalkerPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() > 95.0f;
}

bool MorogrimTidewalkerBossEngagedByMainTankTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
}

bool MorogrimTidewalkerWaterGlobulesAreIncomingTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() < 25.0f;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjBossEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return false;

    return !IsLadyVashjInPhase2(botAI);
}

bool LadyVashjBossEngagedByRangedInPhase1Trigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && IsLadyVashjInPhase1(botAI);
}

bool LadyVashjCastsShockBlastOnHighestAggroTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj") ||
        IsLadyVashjInPhase2(botAI))
        return false;

    return IsMainTankInSameSubgroup(bot);
}

bool LadyVashjBotHasStaticChargeTrigger::IsActive()
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

bool LadyVashjPullingBossInPhase1AndPhase3Trigger::IsActive()
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

bool LadyVashjAddsSpawnInPhase2AndPhase3Trigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "lady vashj") &&
           !IsLadyVashjInPhase1(botAI);
}

bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "coilfang strider");
}

bool LadyVashjTaintedElementalCheatTrigger::IsActive()
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

bool LadyVashjTaintedCoreWasLootedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj") || !IsLadyVashjInPhase2(botAI))
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

bool LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger::IsActive()
{
    return IsLadyVashjInPhase3(botAI);
}

bool LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
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
