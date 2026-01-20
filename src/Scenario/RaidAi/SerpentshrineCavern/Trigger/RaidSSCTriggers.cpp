#include "RaidSSCTriggers.h"
#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"

using namespace SerpentShrineCavernHelpers;

// General
bool SerpentShrineCavernBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
}

// Trash Mobs

bool UnderbogColossusSpawnedToxicPoolAfterDeathTrigger::IsActive()
{
    return bot->HasAura(SPELL_TOXIC_POOL);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    return GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableBotIsFrostTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
           botAI->IsMainTank(bot);
}

bool HydrossTheUnstableBotIsNatureTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
           botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (hydross && hydross->GetHealthPct() < 10.0f)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "pure spawn of hydross") &&
        !AI_VALUE2(Unit*, "find target", "tainted spawn of hydross"))
        return false;

    return !botAI->IsHeal(bot) && !botAI->IsMainTank(bot) &&
           !botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool HydrossTheUnstableDangerFromWaterTombsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    return AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return false;

    return bot->getClass() != CLASS_HUNTER &&
           !botAI->IsHeal(bot) &&
           !botAI->IsMainTank(bot) &&
           !botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool HydrossTheUnstableNeedToManageTimersTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
           IsInstanceTimerManager(botAI, bot);
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    return it != lurkerSpoutTimer.end() && it->second > now;
}

bool TheLurkerBelowBossIsActiveForMainTankTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
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

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    return lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
           (it == lurkerSpoutTimer.end() || it->second <= now);
}

// Trigger will be active only if there are at least 3 tanks in the raid
bool TheLurkerBelowBossIsSubmergedTrigger::IsActive()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    Player* mainTank = nullptr;
    Player* firstAssistTank = nullptr;
    Player* secondAssistTank = nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI)
            continue;

        if (!mainTank && memberAI->IsMainTank(member))
            mainTank = member;
        else if (!firstAssistTank && memberAI->IsAssistTankOfIndex(member, 0, true))
            firstAssistTank = member;
        else if (!secondAssistTank && memberAI->IsAssistTankOfIndex(member, 1, true))
            secondAssistTank = member;
    }

    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    return bot == mainTank || bot == firstAssistTank || bot == secondAssistTank;
}

bool TheLurkerBelowNeedToPrepareTimerForSpoutTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "the lurker below") &&
           IsInstanceTimerManager(botAI, bot);
}

// Leotheras the Blind

bool LeotherasTheBlindBossIsInactiveTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "greyheart spellbinder");
}

bool LeotherasTheBlindBossTransformedIntoDemonFormTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "leotheras the blind") &&
           GetLeotherasDemonFormTank(botAI, bot) == bot;
}

bool LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (!botAI->IsTank(bot))
        return false;

    if (!GetPhase2LeotherasDemon(botAI))
        return false;

    Aura* chaosBlast = bot->GetAura(SPELL_CHAOS_BLAST);
    if (chaosBlast && chaosBlast->GetStackAmount() >= 5)
        return false;

    if (!GetLeotherasDemonFormTank(botAI, bot))
        return false;

    return true;
}

bool LeotherasTheBlindBossEngagedByRangedTrigger::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (!botAI->IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    return !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED) &&
           !leotheras->HasAura(SPELL_WHIRLWIND) &&
           !leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL);
}

bool LeotherasTheBlindBossChannelingWhirlwindTrigger::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (botAI->IsTank(bot) && botAI->IsMelee(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
        return false;

    return leotheras->HasAura(SPELL_WHIRLWIND) ||
           leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL);
}

bool LeotherasTheBlindBotHasTooManyChaosBlastStacksTrigger::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (botAI->IsRanged(bot))
        return false;

    Aura* chaosBlast = bot->GetAura(SPELL_CHAOS_BLAST);
    if (!chaosBlast || chaosBlast->GetStackAmount() < 5)
        return false;

    if (!GetPhase2LeotherasDemon(botAI))
        return false;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!demonFormTank && botAI->IsMainTank(bot))
        return false;

    return true;
}

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    return GetLeotherasDemonFormTank(botAI, bot) != bot;
}

bool LeotherasTheBlindEnteredFinalPhaseTrigger::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (botAI->IsHeal(bot))
        return false;

    if (GetLeotherasDemonFormTank(botAI, bot) == bot)
        return false;

    return GetPhase3LeotherasDemon(botAI) &&
           GetLeotherasHuman(botAI);
}

bool LeotherasTheBlindDemonFormTankNeedsAggro::IsActive()
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (bot->getClass() != CLASS_HUNTER)
        return false;

    return AI_VALUE2(Unit*, "find target", "leotheras the blind");
}

bool LeotherasTheBlindBossWipesAggroUponPhaseChangeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    return IsInstanceTimerManager(botAI, bot);
}

// Fathom-Lord Karathress

bool FathomLordKarathressBossEngagedByMainTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") &&
           botAI->IsMainTank(bot);
}

bool FathomLordKarathressCaribdisEngagedByFirstAssistTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "fathom-guard caribdis") &&
           botAI->IsAssistTankOfIndex(bot, 0, false);
}

bool FathomLordKarathressSharkkisEngagedBySecondAssistTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis") &&
           botAI->IsAssistTankOfIndex(bot, 1, false);
}

bool FathomLordKarathressTidalvessEngagedByThirdAssistTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess") &&
           botAI->IsAssistTankOfIndex(bot, 2, false);
}

bool FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger::IsActive()
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    if (!botAI->IsHealAssistantOfIndex(bot, 0))
        return false;

    Player* firstAssistTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (botAI->IsAssistTankOfIndex(member, 0, false))
            {
                firstAssistTank = member;
                break;
            }
        }
    }

    return firstAssistTank;
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
    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    if (botAI->IsDps(bot))
        return true;

    if (botAI->IsAssistTankOfIndex(bot, 0, false) &&
        !AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return true;

    if (botAI->IsAssistTankOfIndex(bot, 1, false) &&
        !AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis"))
        return true;

    if (botAI->IsAssistTankOfIndex(bot, 2, false) &&
        !AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess"))
        return true;

    return false;
}

bool FathomLordKarathressTanksNeedToEstablishAggroTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    return IsInstanceTimerManager(botAI, bot);
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
    return AI_VALUE2(Unit*, "find target", "morogrim tidewalker") &&
           botAI->IsMainTank(bot);
}

bool MorogrimTidewalkerWaterGlobulesAreIncomingTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() < 25.0f;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjBossEngagedByMainTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "lady vashj") &&
           !IsLadyVashjInPhase2(botAI) && botAI->IsMainTank(bot);
}

bool LadyVashjBossEngagedByRangedInPhase1Trigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    return IsLadyVashjInPhase1(botAI);
}

bool LadyVashjCastsShockBlastOnHighestAggroTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj") ||
        IsLadyVashjInPhase2(botAI))
        return false;

    if (!IsMainTankInSameSubgroup(bot))
        return false;

    return true;
}

bool LadyVashjBotHasStaticChargeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura(SPELL_STATIC_CHARGE))
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
            (!vashj->HasUnitState(UNIT_STATE_ROOT) && vashj->GetHealthPct() <= 50.0f &&
             vashj->GetHealthPct() > 40.0f));
}

bool LadyVashjAddsSpawnInPhase2AndPhase3Trigger::IsActive()
{
    if (botAI->IsHeal(bot))
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
    if (!group)
        return false;

    return (GetDesignatedCoreLooter(group, botAI) == bot &&
            !bot->HasItemCount(ITEM_TAINTED_CORE, 1, false));
}

bool LadyVashjTaintedCoreWasLootedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj") ||
        !IsLadyVashjInPhase2(botAI))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);

    auto hasCore = [](Player* player) -> bool
    {
        return player && player->HasItemCount(ITEM_TAINTED_CORE, 1, false);
    };

    if (bot == designatedLooter)
    {
        if (!hasCore(bot))
            return false;
    }
    else if (bot == firstCorePasser)
    {
        if (hasCore(secondCorePasser) || hasCore(thirdCorePasser) ||
            hasCore(fourthCorePasser))
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
    else if (bot != fourthCorePasser)
        return false;

    if (AnyRecentCoreInInventory(group, botAI))
        return true;

    // First and second passers move to positions as soon as the elemental appears
    if (AI_VALUE2(Unit*, "find target", "tainted elemental") &&
        (bot == firstCorePasser || bot == secondCorePasser))
        return true;

    return false;
}

bool LadyVashjTaintedCoreIsUnusableTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* coreHandlers[] =
    {
        GetDesignatedCoreLooter(group, botAI),
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

    if (!IsLadyVashjInPhase2(botAI))
        return bot->HasItemCount(ITEM_TAINTED_CORE, 1, false);

    return false;
}

bool LadyVashjNeedToResetCorePassingTrackersTrigger::IsActive()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || IsLadyVashjInPhase2(botAI))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    return IsInstanceTimerManager(botAI, bot) ||
           GetDesignatedCoreLooter(group, botAI) == bot ||
           GetFirstTaintedCorePasser(group, botAI) == bot ||
           GetSecondTaintedCorePasser(group, botAI) == bot ||
           GetThirdTaintedCorePasser(group, botAI) == bot ||
           GetFourthTaintedCorePasser(group, botAI) == bot;
}

bool LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger::IsActive()
{
    return IsLadyVashjInPhase3(botAI);
}

bool LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->HasAura(SPELL_ENTANGLE))
                continue;

            if (botAI->IsMelee(member))
                return true;
        }
    }

    return false;
}
