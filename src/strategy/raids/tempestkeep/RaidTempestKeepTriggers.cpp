#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

bool CrimsonHandCenturionCastsArcaneVolleyTrigger::IsActive()
{
    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");

    return centurion && centurion->IsAlive() && bot->getClass() == CLASS_MAGE;
}

bool AlarPullingBossTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetHealthPct() > 98.0f && bot->getClass() == CLASS_HUNTER;
}

bool AlarEngagedByTanksInPhase1Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) &&
            !isPhase2[mapId];
}

bool AlarBossEngagedByMeleeDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return botAI->IsMelee(bot) && !botAI->IsTank(bot) && !isPhase2[mapId];
}

bool AlarBossEngagedByRangedDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return botAI->IsRangedDps(bot) && alar;
}

bool AlarBossEngagedByHealerTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return botAI->IsHeal(bot) && alar;
}

bool AlarEmbersOfAlarSpawnedTrigger::IsActive()
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

    return alar->GetHealthPct() < 95.0f && alar->GetPositionZ() >= 22.0f &&
           bot->GetPositionZ() >= 17.0f && !isPhase2[mapId];
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool AlarEngagedByTanksInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) && isPhase2[mapId];
}

bool AlarPreparingToDiveBombTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 mapId = alar->GetMapId();

    return alar->GetPositionZ() >= 42.0f && isPhase2[mapId];
}

bool AlarNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar;
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

bool HighAstromancerSolarianBossCastsPsychicScreamTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    Group* group = bot->GetGroup();
    if (!astromancer || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    return astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && bot->getClass() == CLASS_PRIEST &&
           mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderThaladredEngagedBySecondAssistTankTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");

    return thaladred && botAI->IsAssistTankOfIndex(bot, 1);
}

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladrad the darkener");

    return !botAI->IsTank(bot) && thaladred && thaladred->GetVictim() == bot;
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");

    return sanguinar && botAI->IsMainTank(bot);
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    Group* group = bot->GetGroup();
    if (!sanguinar || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    return bot->getClass() == CLASS_PRIEST && mainTank &&
           !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderCapernianEngagedByWarlockTankTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    Player* capernianTank = GetCapernianTank(botAI, bot);

    return capernian && capernianTank && bot == capernianTank;
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    return telonicus && botAI->IsAssistTankOfIndex(bot, 0);
}
