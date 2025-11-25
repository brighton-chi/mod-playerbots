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

bool UnderbogColossusSpawnedToxicPoolAfterDeathTrigger::IsActive()
{
    return bot->HasAura(SPELL_TOXIC_POOL);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);
    return totem != nullptr;
}

bool RancidMushroomSpawnedTrigger::IsActive()
{
    Unit* mushroom = GetFirstAliveUnitByEntry(botAI, NPC_RANCID_MUSHROOM);
    return mushroom != nullptr;
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableBotIsFrostTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

bool HydrossTheUnstableBotIsNatureTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || hydross->GetHealthPct() < 10.0f)
        return false;

    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
    return waterElemental != nullptr || natureElemental != nullptr;
}

bool HydrossTheUnstableDangerFromWaterTombsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() == CLASS_HUNTER)
        return false;

    if (!botAI->IsDps(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

bool HydrossTheUnstableNeedToManageTimersTrigger::IsActive()
{
    if (!IsMapIDTimerManager(botAI, bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    return hydross != nullptr;
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMapId());
    return it != lurkerSpoutTimer.end() && it->second > now;
}

bool TheLurkerBelowBossIsActiveForMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMapId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() || it->second <= now);
}

bool TheLurkerBelowBossCastsGeyserTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMapId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() || it->second <= now);
}

bool TheLurkerBelowNeedToPrepareTimerForSpoutTrigger::IsActive()
{
    if (!IsMapIDTimerManager(botAI, bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker != nullptr;
}

// Leotheras the Blind

bool LeotherasTheBlindBossIsInactiveTrigger::IsActive()
{
    Unit* spellbinder = AI_VALUE2(Unit*, "find target", "greyheart spellbinder");
    return spellbinder && spellbinder->IsAlive();
}

bool LeotherasTheBlindEngagedByDemonFormTankTrigger::IsActive()
{
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && bot != demonFormTank)
        return false;

    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    return leotherasDemon != nullptr;
}

bool LeotherasTheBlindBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && demonFormTank == bot)
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED);
}

bool LeotherasTheBlindBossChannelingWhirlwindTrigger::IsActive()
{
    if (botAI->IsTank(bot) && botAI->IsMelee(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED) &&
           (leotheras->HasAura(SPELL_WHIRLWIND) || leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL));
}

bool LeotherasTheBlindDemonFormEngagedByMeleeWithoutWarlockTankTrigger::IsActive()
{
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && demonFormTank->getClass() == CLASS_WARLOCK)
        return false;

    if (!botAI->IsMelee(bot) || botAI->IsMainTank(bot))
        return false;

    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    return leotherasDemon != nullptr;
}

bool LeotherasTheBlindInnerDemonCheatTrigger::IsActive()
{
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    if (!bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras != nullptr;
}

bool LeotherasTheBlindEnteredFinalPhaseTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (demonFormTank && demonFormTank == bot)
        return false;

    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasPhase3Demon)
        return false;

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    return leotherasHuman != nullptr;
}

bool LeotherasTheBlindDemonFormTankNeedsAggro::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras != nullptr;
}

bool LeotherasTheBlindNeedToManageTimersAndTrackersTrigger::IsActive()
{
    if (!IsMapIDTimerManager(botAI, bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras != nullptr;
}

// Fathom-Lord Karathress

bool FathomLordKarathressBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    return karathress != nullptr;
}

bool FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    return sharkkis && sharkkis->IsAlive();
}

bool FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1))
        return false;

    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    return tidalvess && tidalvess->IsAlive();
}

bool FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 2))
        return false;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    return caribdis && caribdis->IsAlive();
}

bool FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger::IsActive()
{
    if (!botAI->IsHealAssistantOfIndex(bot, 0))
        return false;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    return caribdis && caribdis->IsAlive();
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
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    return (botAI->IsDps(bot) ||
           (botAI->IsAssistTankOfIndex(bot, 0) && (!sharkkis || !sharkkis->IsAlive())) ||
           (botAI->IsAssistTankOfIndex(bot, 1) && (!tidalvess || !tidalvess->IsAlive())) ||
           (botAI->IsAssistTankOfIndex(bot, 2) && (!caribdis || !caribdis->IsAlive())));
}

bool FathomLordKarathressTanksNeedToEstablishAggroTrigger::IsActive()
{
    if (!IsMapIDTimerManager(botAI, bot))
        return false;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    return karathress != nullptr;
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
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker != nullptr;
}

bool MorogrimTidewalkerWaterGlobulesAreIncomingTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() < 25.0f;
}

bool MorogrimTidewalkerEncounterResetTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealth() == tidewalker->GetMaxHealth();
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjBossEngagedByRangedInPhase1Trigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && IsLadyVashjInPhase1(botAI);
}

bool LadyVashjCastsShockBlastOnHighestAggroTrigger::IsActive()
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() != CLASS_SHAMAN || tab != 2)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && (IsLadyVashjInPhase1(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjBotHasStaticChargeTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_STATIC_CHARGE))
                return true;
        }
    }

    return false;
}

bool LadyVashjPullingBossInPhase1AndPhase3Trigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");

    return vashj && ((vashj->GetHealthPct() <= 100.0f && vashj->GetHealthPct() > 90.0f) ||
            (!vashj->HasUnitState(UNIT_STATE_ROOT) && vashj->GetHealthPct() <= 50.0f && vashj->GetHealthPct() > 40.0f));
}

bool LadyVashjCoilfangStriderIsApproachingTrigger::IsActive()
{
    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    return strider && strider->IsAlive() &&
           (IsLadyVashjInPhase2(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjDeterminingKillOrderOfAddsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && (IsLadyVashjInPhase2(botAI) || IsLadyVashjInPhase3(botAI));
}

bool LadyVashjTaintedElementalCheatTrigger::IsActive()
{
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    bool taintedPresent = false;
    Unit* taintedUnit = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (taintedUnit)
        taintedPresent = true;
    else
    {
        GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
        for (auto const& guid : corpses)
        {
            LootObject loot(bot, guid);
            WorldObject* object = loot.GetWorldObject(bot);
            if (!object)
                continue;

            if (Creature* creature = object->ToCreature())
            {
                if (creature->GetEntry() == NPC_TAINTED_ELEMENTAL && !creature->IsAlive())
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
    if (!vashj)
        return false;

    if (!IsLadyVashjInPhase2(botAI))
        return false;

    Player* master = botAI->GetMaster();
    Group* group = bot->GetGroup();
    if (!master || !group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, master, botAI);
    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);

    auto hasCore = [](Player* p) -> bool { return p && p->HasItemCount(ITEM_TAINTED_CORE, 1, false); };

    if (bot == designatedLooter)
    {
        if (hasCore(firstCorePasser) || hasCore(secondCorePasser) || hasCore(thirdCorePasser) || hasCore(fourthCorePasser))
            return false;
    }
    else if (bot == firstCorePasser)
    {
        if (hasCore(secondCorePasser) || hasCore(thirdCorePasser) || hasCore(fourthCorePasser))
            return false;
    }
    else if (bot == secondCorePasser)
    {
        if (hasCore(thirdCorePasser) || hasCore(fourthCorePasser))
            return false;
    }
    else if (bot == thirdCorePasser)
    {
        if (hasCore(fourthCorePasser))
            return false;
    }

    if (AnyRecentParalyze(group, vashj->GetMapId()))
        return true;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && designatedLooter->GetExactDist2d(tainted) < 5.0f)
        return true;

    return false;
}

bool LadyVashjCoreHandlerIsDeadTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    Group* group = bot->GetGroup();
    if (!master || !group)
        return false;

    Player* coreHandlers[] =
    {
        GetDesignatedCoreLooter(group, master, botAI),
        GetFirstTaintedCorePasser(group, botAI),
        GetSecondTaintedCorePasser(group, botAI),
        GetThirdTaintedCorePasser(group, botAI),
        GetFourthTaintedCorePasser(group, botAI)
    };

    if (bot->HasItemCount(ITEM_TAINTED_CORE, 1, false))
    {
        for (Player* coreHandler : coreHandlers)
        {
            if (coreHandler && bot == coreHandler)
                return false;
        }
        return true;
    }

    return false;
}

bool LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && IsLadyVashjInPhase3(botAI);
}

bool LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!member->HasAura(SPELL_ENTANGLE))
                continue;

            if (botAI->IsMelee(member))
                return true;
        }
    }

    return false;
}

bool LadyVashjNeedToManageTrackersTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && vashj->GetHealth() == vashj->GetMaxHealth();
}
