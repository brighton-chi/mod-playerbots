#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

bool AlarNeedToLogInfoToDevelopBossStrategyTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && (IsMapIDTimerManager(botAI, bot) || botAI->IsMainTank(bot));
}

bool AlarPullingBossTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetHealthPct() > 98.0f && bot->getClass() == CLASS_HUNTER;
}

bool AlarTanksNeedToBeAtPlatformsToAggroBossTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return !isPhase2[mapId] && alar->GetPositionZ() <= 25.0f && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0));
}

bool AlarBossEngagedByMeleeDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return !isPhase2[mapId] && botAI->IsMelee(bot) && !botAI->IsTank(bot);
}

bool AlarBossEngagedByRangedDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return !isPhase2[mapId] && botAI->IsRangedDps(bot);
}

bool AlarBossEngagedByHealerTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return !isPhase2[mapId] && botAI->IsHeal(bot);
}

bool AlarNeedTankToPickUpAddsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && IsAlarAddTank(botAI, bot);
}

// DON'T FORGET THE 95% HEALTH CHECK YOU ADDED WHILE TESTING YOU IDIOT
bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();
    return !isPhase2[mapId] && alar->GetPositionZ() > 25.0f && alar->GetHealthPct() < 95.0f;
}

bool AlarNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && IsMapIDTimerManager(botAI, bot);
}

bool VoidReaverBossEngagedByTankTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && botAI->IsTank(bot) && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossEngagedByRangedTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && botAI->IsRanged(bot);
}

bool HighAstromancerSolarianPhase1And2MovementTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");

    return astromancer && solariumPriest && botAI->IsMelee(bot);
}

bool HighAstromancerSolarianTransformedIntoVoidwalkerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && botAI->IsMainTank(bot);
}

bool HighAstromancerSolarianCanCastFearWardOnMainTankTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    Player* mainTank = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && 
           bot->getClass() == CLASS_PRIEST && mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}
