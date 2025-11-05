#include "RaidSSCTriggers.h"
#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "AiFactory.h"
#include "Playerbots.h"

using namespace SerpentShrineCavernHelpers;

// Trash Mobs

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);

    return totem && botAI->IsDps(bot);
}

bool RancidMushroomSpawnedTrigger::IsActive()
{
    Unit* mushroom = GetFirstAliveUnitByEntry(botAI, NPC_RANCID_MUSHROOM);

    return mushroom;
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableBotIsFrostTankTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && botAI->IsMainTank(bot);
}

bool HydrossTheUnstableBotIsNatureTankTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && botAI->IsAssistTankOfIndex(bot, 0);
}

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActive()
{
    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");

    return botAI->IsDps(bot) && (waterElemental || natureElemental);
}

bool HydrossTheUnstableDangerFromWaterTombsTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && !hydross->HasAura(SPELL_CORRUPTION) && botAI->IsRanged(bot);
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && bot->getClass() == CLASS_HUNTER;
}

bool HydrossTheUnstableNeedToTransitionBeforeFourthMarkTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0));
}

bool HydrossTheUnstableNeedToManagePhaseChangeAggroResetTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && IsMapIDTimerManager(botAI, bot);
}

// The Lurker Below

/* bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");

    return lurker && (lurker->HasAura(SPELL_SPOUT_VISUAL) ||
           lurker->HasAura(SPELL_SPOUT_PERIODIC_1) || lurker->HasAura(SPELL_SPOUT_PERIODIC_2));
} */

bool TheLurkerBelowBossIsActiveForMainTankTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    return botAI->IsMainTank(bot) && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

bool TheLurkerBelowBossIsActiveForOtherMeleeTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    return botAI->IsMelee(bot) && !botAI->IsMainTank(bot) &&
           lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

// Temp making this for all bots
bool TheLurkerBelowSpoutIsActiveForMeleeTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    return /* botAI->IsMelee(bot) && */
           lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now;
}

bool TheLurkerBelowBossIsActiveForRangedDpsTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");

    return botAI->IsRanged(bot) && lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED;
}

bool TheLurkerBelowBossIsActiveForHealerTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");

    return botAI->IsHeal(bot) && lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED;
}

bool TheLurkerBelowNeedToPrepareTimerForSpoutTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");

    return lurker && IsMapIDTimerManager(botAI, bot);
}

// Leotheras the Blind

bool LeotherasTheBlindBossIsInactiveTrigger::IsActive()
{
    Unit* spellbinder = AI_VALUE2(Unit*, "find target", "greyheart spellbinder");

    return spellbinder && spellbinder->IsAlive();
}

bool LeotherasTheBlindEngagedByDemonFormTankTrigger::IsActive()
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    return leotherasDemon && demonFormTank == bot;
}

bool LeotherasTheBlindBossEngagedByRangedTrigger::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    return botAI->IsRanged(bot) && demonFormTank != bot &&
           leotheras && !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED);
}

bool LeotherasTheBlindBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    return !(botAI->IsTank(bot) && botAI->IsMelee(bot)) && leotheras &&
           !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED) &&
           (leotheras->HasAura(SPELL_WHIRLWIND) || leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL));
}

/* bool LeotherasTheBlindDemonFormEngagedByMeleeTrigger::IsActive()
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);

    return leotherasDemon && botAI->IsMelee(bot) && !botAI->IsMainTank(bot);
} */

bool LeotherasTheBlindInnerDemonHasTakenForm::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    return leotheras && bot->HasAura(SPELL_INSIDIOUS_WHISPER);
}

bool LeotherasTheBlindEnteredFinalPhaseTrigger::IsActive()
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    return leotherasHuman && leotherasPhase3Demon && !botAI->IsHeal(bot) &&
           demonFormTank != bot;
}

bool LeotherasTheBlindDemonFormTankNeedsAggro::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    return leotheras && demonFormTank && bot->getClass() == CLASS_HUNTER &&
           !bot->HasAura(SPELL_INSIDIOUS_WHISPER);
}

bool LeotherasTheBlindNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    return leotheras && IsMapIDTimerManager(botAI, bot);
}

// Fathom-Lord Karathress

bool FathomLordKarathressBossEngagedByMainTankTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");

    return karathress && karathress->IsAlive() &&
           botAI->IsMainTank(bot);
}

bool FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");

    return sharkkis && sharkkis->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 0);
}

bool FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger::IsActive()
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");

    return tidalvess && tidalvess->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 1);
}

bool FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger::IsActive()
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    return caribdis && caribdis->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 2);
}

bool FathomLordKarathressPullingBossesTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");

    return karathress && karathress->GetHealthPct() > 98.0f && bot->getClass() == CLASS_HUNTER;
}

bool FathomLordKarathressDeterminingKillOrderTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    return karathress && (botAI->IsDps(bot) ||
           (botAI->IsAssistTankOfIndex(bot, 0) && (!sharkkis || !sharkkis->IsAlive())) ||
           (botAI->IsAssistTankOfIndex(bot, 1) && (!tidalvess || !tidalvess->IsAlive())) ||
           (botAI->IsAssistTankOfIndex(bot, 2) && (!caribdis || !caribdis->IsAlive())));
}

bool FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger::IsActive()
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    return caribdis && caribdis->IsAlive() && botAI->IsHealAssistantOfIndex(bot, 0);
}

bool FathomLordKarathressTanksNeedToEstablishAggroTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");

    return karathress && IsMapIDTimerManager(botAI, bot);
}

// Morogrim Tidewalker

bool MorogrimTidewalkerPullingBossTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && tidewalker->GetHealthPct() > 95.0f && bot->getClass() == CLASS_HUNTER;
}

bool MorogrimTidewalkerBossEngagedByMainTankTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && botAI->IsMainTank(bot);
}

bool MorogrimTidewalkerWaterGlobulesAreIncomingTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && tidewalker->GetHealthPct() < 25.0f &&
           botAI->IsRanged(bot);
}

bool MorogrimTidewalkerEncounterResetTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && tidewalker->GetHealth() == tidewalker->GetMaxHealth();
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjBossEngagedByMainTankInPhase1Trigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase1(botAI) && botAI->IsMainTank(bot);
}

bool LadyVashjBossEngagedByRangedInPhase1AndPhase3Trigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI)) &&
           botAI->IsRanged(bot);
}

bool LadyVashjCastsShockBlastOnHighestAggroTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    int8 tab = AiFactory::GetPlayerSpecTab(bot);

    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI)) &&
           bot->getClass() == CLASS_SHAMAN && tab == 2;
}

bool LadyVashjBotHasStaticChargeTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI)) &&
           bot->HasAura(SPELL_STATIC_CHARGE);
}

bool LadyVashjPullingBossInPhase1AndPhase3Trigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && bot->getClass() == CLASS_HUNTER &&
           ((vashj->GetHealthPct() <= 100.0f && vashj->GetHealthPct() > 90.0f) ||
            (!vashj->HasUnitState(UNIT_STATE_ROOT) && vashj->GetHealthPct() <= 50.0f && vashj->GetHealthPct() > 40.0f));
}

bool LadyVashjEnchantedElementalsAreMovingToBossTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase2(botAI) && botAI->IsRangedDps(bot) &&
           !botAI->IsRangedDpsAssistantOfIndex(bot, 0) && !botAI->IsRangedDpsAssistantOfIndex(bot, 1);
}

bool LadyVashjDeterminingPhase2KillOrderTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase2(botAI) && !botAI->IsHeal(bot);
}

bool LadyVashjPlayerNeedsBotSupportToDisableGeneratorsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase2(botAI) && botAI->IsRangedDpsAssistantOfIndex(bot, 0);
}

bool LadyVashjPlayerLootedTaintedCoreTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !IsLadyVashjInPhase2(botAI))
        return false;

    bool isCorePasser = botAI->IsHealAssistantOfIndex(bot, 0) ||
                        botAI->IsRangedDpsAssistantOfIndex(bot, 1) ||
                        botAI->IsRangedDpsAssistantOfIndex(bot, 0);

    if (!isCorePasser)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_PARALYZE))
            return true;
    }

    return false;
}
/* bool LadyVashjPlayerLootedTaintedCoreTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!vashj || !IsLadyVashjInPhase2(botAI) || !group)
        return false;

    static std::map<uint32, time_t> firstParalyze[1]; // [0] for static storage
    bool foundParalyze = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_PARALYZE))
        {
            foundParalyze = true;
            break;
        }
    }

    uint32 mapId = vashj->GetMapId();
    time_t now = time(nullptr);

    // Start the timer when Paralyze is first found
    if (foundParalyze && !firstParalyze[0][mapId])
        firstParalyze[0][mapId] = now;

    // Reset the timer if we leave phase 2 or reset encounter
    if (!foundParalyze && (now - firstParalyze[0][mapId] > 20))
        firstParalyze[0][mapId] = 0;

    bool isCorePasser = botAI->IsHealAssistantOfIndex(bot, 0) ||
                        botAI->IsRangedDpsAssistantOfIndex(bot, 1) ||
                        botAI->IsRangedDpsAssistantOfIndex(bot, 0);

    if (!isCorePasser)
        return false;

    // Keep the trigger active if Paralyze is present, or within 10s of first Paralyze seen
    return foundParalyze || (firstParalyze[0][mapId] && (now - firstParalyze[0][mapId] < 10));
} */

bool LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase3(botAI);
}

bool LadyVashjNeedToManageTrackersTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase2(botAI));
}
