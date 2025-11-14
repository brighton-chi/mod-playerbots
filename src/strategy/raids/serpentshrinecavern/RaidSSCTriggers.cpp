#include <chrono>
#include <ctime>
#include <cstdio>
#include "LootObjectStack.h"
#include "LootObjectStack.h"
#include "LootAction.h"
#include "ObjectAccessor.h"
#include "Corpse.h"// debugging only

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

// Apply this to all bots if need to run around with all due to water issues
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
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    return botAI->IsRangedDps(bot) && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (!lurkerSpoutTimer.count(mapId) || lurkerSpoutTimer[mapId] <= now);
}

bool TheLurkerBelowBossIsActiveForHealerTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

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

/* bool LadyVashjTaintedElementalCheatTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!vashj || !tainted || !botAI->HasCheat(BotCheatMask::raid))
        return false;

    Group* group = bot->GetGroup();
    Player* master = botAI->GetMaster();
    if (!group || !master)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);

    return (designatedLooter && designatedLooter == bot &&
            !bot->HasItemCount(ITEM_TAINTED_CORE, 1, false));
} */
// Logging version of action below:
bool LadyVashjTaintedElementalCheatTrigger::IsActive()
{
    // short high-res timestamp for logs (optional)
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    char tbuf[64];
    std::strftime(tbuf, sizeof(tbuf), "%F %T", std::localtime(&tt));
    char timestr[80];
    std::snprintf(timestr, sizeof(timestr), "%s.%03d", tbuf, static_cast<int>(ms.count()));

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !botAI->HasCheat(BotCheatMask::raid))
    {
        LOG_DEBUG("playerbots", "{} inactive (no Vashj or no raid cheat)", timestr);
        return false;
    }

    // check for live tainted elemental first
    Unit* taintedUnit = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (taintedUnit)
    {
        LOG_DEBUG("playerbots", "{} live tainted found guid={} entry={} hp={}/{} alive={}",
                  timestr, taintedUnit->GetGUID().ToString(), taintedUnit->GetEntry(),
                  taintedUnit->GetHealth(), taintedUnit->GetMaxHealth(), taintedUnit->IsAlive());
    }

    // if no live unit, scan visible corpses for tainted elemental corpse
    bool taintedCorpseFound = false;
    if (!taintedUnit)
    {
        GuidVector corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
        LOG_DEBUG("playerbots", "{} nearest corpses count = {}", timestr, corpses.size());
        for (auto const& g : corpses)
        {
            LootObject loot(bot, g);
            WorldObject* obj = loot.GetWorldObject(bot);
            if (!obj)
            {
                LOG_DEBUG("playerbots", "{} nearest corpse guid={} worldobj=NULL", timestr, g.ToString());
                continue;
            }

            // Corpse object (preferred)
            if (Corpse* c = obj->ToCorpse())
            {
                LOG_DEBUG("playerbots", "{} nearest obj: CORPSE guid={} entry={} owner={}",
                          timestr, g.ToString(), c->GetEntry(), c->GetOwnerGUID().ToString());
                if (c->GetEntry() == NPC_TAINTED_ELEMENTAL)
                {
                    taintedCorpseFound = true;
                    LOG_DEBUG("playerbots", "{} found tainted CORPSE guid={} entry={}", timestr, g.ToString(), c->GetEntry());
                    break;
                }
                continue;
            }

            // Still exposed as a Creature (conversion to Corpse delayed)
            if (Creature* cr = obj->ToCreature())
            {
                LOG_DEBUG("playerbots", "{} nearest obj: CREATURE guid={} entry={} hp={}/{} alive={} guid_low={} flags=0x{:X} dynflags=0x{:X}",
                          timestr, g.ToString(), cr->GetEntry(), cr->GetHealth(), cr->GetMaxHealth(), cr->IsAlive(),
                          cr->GetGUID().GetCounter(),
                          cr->GetUInt32Value(UNIT_FIELD_FLAGS), cr->GetUInt32Value(UNIT_DYNAMIC_FLAGS));
                if (cr->GetEntry() == NPC_TAINTED_ELEMENTAL && !cr->IsAlive())
                {
                    taintedCorpseFound = true;
                    LOG_DEBUG("playerbots", "{} treating dead CREATURE as tainted corpse guid={} entry={}", timestr, g.ToString(), cr->GetEntry());
                    break;
                }
                continue;
            }

            // Other object types (GameObject, Transport, etc.) — log presence
            LOG_DEBUG("playerbots", "{} nearest obj: OTHER guid={} (not corpse/creature)", timestr, g.ToString());
        }
    }

    // nothing relevant visible?
    if (!taintedUnit && !taintedCorpseFound)
    {
        LOG_DEBUG("playerbots", "{} no live tainted and no tainted corpse visible", timestr);
        return false;
    }

    // group/master/designated looter checks
    Group* group = bot->GetGroup();
    Player* master = botAI->GetMaster();
    if (!group || !master)
    {
        LOG_DEBUG("playerbots", "{} inactive (no group/master)", timestr);
        return false;
    }

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);
    bool active = (designatedLooter && designatedLooter == bot &&
                   !bot->HasItemCount(ITEM_TAINTED_CORE, 1, false));

    LOG_DEBUG("playerbots", "{} final decision designated={} bot={} has_core={} -> active={}",
              timestr,
              (designatedLooter ? designatedLooter->GetName() : std::string("none")),
              bot->GetName(),
              bot->HasItemCount(ITEM_TAINTED_CORE, 1, false),
              active);

    return active;
}

bool LadyVashjTaintedCoreWasLootedTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Player* master = botAI->GetMaster();
    Group* group = bot->GetGroup();
    if (!vashj || !IsLadyVashjInPhase2(botAI) || !master || !group)
        return false;

    if (bot != GetFirstTaintedCorePasser(group, botAI) &&
        bot != GetSecondTaintedCorePasser(group, botAI) &&
        bot != GetThirdTaintedCorePasser(group, botAI) &&
        bot != GetDesignatedCoreLooter(group, master, botAI))
        return false;

    // remember last time Paralyze was observed so we can tolerate the brief handoff gap
    static std::map<uint32, time_t> lastParalyzeTime;
    uint32 mapId = vashj->GetMapId();
    time_t now = time(nullptr);

    bool foundParalyze = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_PARALYZE))
        {
            foundParalyze = true;
            lastParalyzeTime[mapId] = now;
            break;
        }
    }

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (foundParalyze || tainted)
        return true;

    // if we recently saw paralyze on this map, keep trigger active for the grace window
    auto it = lastParalyzeTime.find(mapId);
    if (it != lastParalyzeTime.end() && (now - it->second) <= 3)
        return true;

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
