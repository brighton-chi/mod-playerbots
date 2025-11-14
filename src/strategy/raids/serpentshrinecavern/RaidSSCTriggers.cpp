#include "RaidSSCTriggers.h"
#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
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

    return IsMapIDTimerManager(botAI, bot) && (waterElemental || natureElemental);
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

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && botAI->IsDps(bot) && bot->getClass() != CLASS_HUNTER;
}

bool HydrossTheUnstableNeedToManageTimersTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && !botAI->IsTank(bot);
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");

    return lurker && (lurker->HasAura(SPELL_SPOUT_VISUAL) ||
           lurker->HasAura(SPELL_SPOUT_PERIODIC_1) || lurker->HasAura(SPELL_SPOUT_PERIODIC_2));
}

bool TheLurkerBelowBossIsActiveForMainTankTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = std::time(nullptr);

    return botAI->IsMainTank(bot) && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

bool TheLurkerBelowBossIsActiveForOtherMeleeTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = std::time(nullptr);

    return botAI->IsMelee(bot) && !botAI->IsMainTank(bot) &&
           lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

// Apply this to all bots if need to run around with all due to water issues
bool TheLurkerBelowSpoutIsActiveForMeleeTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = std::time(nullptr);

    return /* botAI->IsMelee(bot) && */
           lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now;
}

bool TheLurkerBelowBossIsActiveForRangedDpsTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = std::time(nullptr);

    return botAI->IsRangedDps(bot) && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

bool TheLurkerBelowBossIsActiveForHealerTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = std::time(nullptr);

    return botAI->IsHeal(bot) && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
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

bool LeotherasTheBlindDemonFormEngagedByMeleeWithoutWarlockTankTrigger::IsActive()
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    return leotherasDemon && botAI->IsMelee(bot) && !botAI->IsMainTank(bot) &&
           demonFormTank && demonFormTank->getClass() != CLASS_WARLOCK;
}

bool LeotherasTheBlindInnerDemonCheatTrigger::IsActive()
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    return leotheras && bot->HasAura(SPELL_INSIDIOUS_WHISPER) && botAI->HasCheat(BotCheatMask::raid);
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

bool LadyVashjBossEngagedByMainTankTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI)) && botAI->IsMainTank(bot);
}

bool LadyVashjBossEngagedByRangedInPhase1Trigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase1(botAI) && botAI->IsRanged(bot);
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
    Group* group = bot->GetGroup();
    if (!vashj || !group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_STATIC_CHARGE))
            return true;
    }

    return false;
}

bool LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!vashj || !group || IsLadyVashjInPhase2(botAI))
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_ENTANGLE) && member->HasAura(SPELL_TOXIC_SPORES))
            return true;
    }

    return false;
}

bool LadyVashjPullingBossInPhase1AndPhase3Trigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && bot->getClass() == CLASS_HUNTER &&
           ((vashj->GetHealthPct() <= 100.0f && vashj->GetHealthPct() > 90.0f) ||
            (!vashj->HasUnitState(UNIT_STATE_ROOT) && vashj->GetHealthPct() <= 50.0f && vashj->GetHealthPct() > 40.0f));
}

bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActive()
{
    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");

    return strider && strider->IsAlive() && (IsLadyVashjInPhase2(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjDeterminingKillOrderOfAddsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && (IsLadyVashjInPhase2(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjPlayerNeedsBotSupportToDisableGeneratorsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && IsLadyVashjInPhase2(botAI) && !botAI->HasCheat(BotCheatMask::raid) &&
           (botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsRangedDpsAssistantOfIndex(bot, 1));
}

bool LadyVashjTaintedElementalCheatTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !botAI->HasCheat(BotCheatMask::raid))
        return false;

    // accept either a live tainted unit OR a dead Creature object of the tainted entry
    bool taintedPresent = false;

    // live unit check (fast path)
    Unit* taintedUnit = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (taintedUnit)
        taintedPresent = true;
    else
    {
        // scan the nearest visible object GUIDs for a dead Creature of the tainted entry
        GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
        for (auto const& guid : corpses)
        {
            LootObject loot(bot, guid);
            WorldObject* obj = loot.GetWorldObject(bot);
            if (!obj)
                continue;

            if (Creature* cr = obj->ToCreature())
            {
                if (cr->GetEntry() == NPC_TAINTED_ELEMENTAL && !cr->IsAlive())
                {
                    taintedPresent = true;
                    break;
                }
            }
        }
    }

    if (!taintedPresent)
        return false;

    Group* group = bot->GetGroup();
    Player* master = botAI->GetMaster();
    if (!group || !master)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);

    return (designatedLooter && designatedLooter == bot &&
            !bot->HasItemCount(ITEM_TAINTED_CORE, 1, false));
}

bool LadyVashjTaintedCoreWasLootedTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Player* master = botAI->GetMaster();
    Group* group = bot->GetGroup();

    if (!vashj || !IsLadyVashjInPhase2(botAI) || !master || !group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);
    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);

    LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: role check for bot={} first={} second={} third={} looter={}",
              bot->GetName(), bot == firstCorePasser, bot == secondCorePasser, bot == thirdCorePasser, bot == designatedLooter);

    auto hasCore = [](Player* p) -> bool { return p && p->HasItemCount(ITEM_TAINTED_CORE, 1, false); };

    if (bot == designatedLooter)
    {
        if (hasCore(firstCorePasser) || hasCore(secondCorePasser) || hasCore(thirdCorePasser))
        {
            LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: designated looter={} exempt because a passer has core (first={} second={} third={})",
                      bot->GetName(),
                      hasCore(firstCorePasser), hasCore(secondCorePasser), hasCore(thirdCorePasser));
            return false;
        }
    }
    else if (bot == firstCorePasser)
    {
        if (hasCore(secondCorePasser) || hasCore(thirdCorePasser))
        {
            LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: first passer={} exempt because second/third have core (second={} third={})",
                      bot->GetName(), hasCore(secondCorePasser), hasCore(thirdCorePasser));
            return false;
        }
    }
    else if (bot == secondCorePasser)
    {
        if (hasCore(thirdCorePasser))
        {
            LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: second passer={} exempt because third has core (third={})",
                      bot->GetName(), hasCore(thirdCorePasser));
            return false;
        }
    }

    // remember last time Paralyze was observed so we can tolerate the brief handoff gap
    static std::map<uint32, time_t> lastParalyzeTime;
    uint32 mapId = vashj->GetMapId();
    time_t now = std::time(nullptr);

    bool foundParalyze = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        bool alive = member->IsAlive();
        bool hasParalyze = member->HasAura(SPELL_PARALYZE);
        LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: member={} alive={} paralyze={}", member->GetName(), alive, hasParalyze);
        if (alive && hasParalyze)
        {
            foundParalyze = true;
            lastParalyzeTime[mapId] = now;
            LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: observed paralyze on member={}, setting lastParalyzeTime[mapId]={}", member->GetName(), now);
            break;
        }
    }

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: foundParalyze={} taintedPresent={} taintedName={}",
              foundParalyze, tainted ? "true" : "false", tainted ? tainted->GetName() : "null");

    if (foundParalyze || tainted)
    {
        LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: returning true due to foundParalyze/tainted");
        return true;
    }

    // if we recently saw paralyze on this map, keep trigger active for the grace window
    auto it = lastParalyzeTime.find(mapId);
    if (it != lastParalyzeTime.end())
    {
        time_t last = it->second;
        LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: lastParalyzeTime for map={} = {} (now={}), age={}s", mapId, last, now, (now - last));
        if ((now - last) <= 3)
        {
            LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: returning true due to grace window");
            return true;
        }
    }
    else
    {
        LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: no lastParalyzeTime for map={}", mapId);
    }

    LOG_DEBUG("playerbots", "LadyVashjTaintedCoreWasLootedTrigger: returning false - no paralyze, no tainted, no recent paralyze");
    return false;
}

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

bool LadyVashjCheatToTestTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj;
}
