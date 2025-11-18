#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

bool CrimsonHandCenturionCastsArcaneVolleyTrigger::IsActive()
{
    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");
    if (!centurion)
        return false;

    return centurion->IsAlive() && bot->getClass() == CLASS_MAGE && centurion->HasAura(SPELL_ARCANE_FLURRY);
}

bool AlarPullingBossTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return alar->GetHealthPct() > 98.0f && bot->getClass() == CLASS_HUNTER;
}

bool AlarEngagedByTanksInPhase1Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) &&
            !isPhase2[mapId];
}

bool AlarBossEngagedByMeleeDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return botAI->IsMelee(bot) && !botAI->IsTank(bot) && !isPhase2[mapId];
}

bool AlarBossEngagedByRangedDpsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return botAI->IsRangedDps(bot);
}

bool AlarBossEngagedByHealerTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return !isPhase2[mapId] && botAI->IsHeal(bot);
}

bool AlarEmbersOfAlarSpawnedTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return IsAlarAddTank(botAI, bot);
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return alar->GetHealthPct() < 95.0f && alar->GetPositionZ() >= 22.0f &&
           bot->GetPositionZ() >= 17.0f && !isPhase2[mapId];
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return alar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool AlarEngagedByTanksInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) && isPhase2[mapId];
}

bool AlarPhase2EncounterIsAtRoomCenterTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return alar->GetPositionZ() < 42.0f && isPhase2[mapId] && !bot->GetVictim();
}

bool AlarBossIsPreparingToDiveBombTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    const uint32 mapId = alar->GetMapId();

    return alar->GetPositionZ() >= 42.0f && isPhase2[mapId];
}

bool AlarNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar != nullptr;
}

bool VoidReaverBossEngagedByTankTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    return botAI->IsTank(bot) && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossLaunchesArcaneOrbsTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    return botAI->IsRanged(bot);
}

bool VoidReaverTanksLostAggroTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    return !botAI->IsTank(bot) && voidReaver->GetVictim() == bot;
}

bool HighAstromancerSolarianPhase1And2MovementTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");

    return solariumPriest && botAI->IsMelee(bot);
}

bool HighAstromancerSolarianTransformedIntoVoidwalkerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && botAI->IsMainTank(bot);
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

bool KaelthasSunstriderNeedToTestStrategiesOnBossTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return IsMapIDTimerManager(botAI, bot);
}

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladrad the darkener");
    if (!thaladred)
        return false;

    return thaladred->IsAlive() && !botAI->IsTank(bot) && thaladred->GetVictim() == bot;
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    return sanguinar->IsAlive() && botAI->IsMainTank(bot);
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

    return sanguinar->IsAlive() && bot->getClass() == CLASS_PRIEST && mainTank &&
           !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderCapernianEngagedByWarlockTankTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    Player* capernianTank = GetCapernianTank(botAI, bot);

    return capernian->IsAlive() && capernianTank && bot == capernianTank;
}

bool KaelthasSunstriderCapernianCastsArcaneBurstTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    return capernian->IsAlive();
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    if (!telonicus)
        return false;

    return telonicus->IsAlive() && botAI->IsAssistTankOfIndex(bot, 0);
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return bot->getClass() == CLASS_HUNTER && IsAnyTankableAdvisorActive(botAI);
}

bool KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return IsMapIDTimerManager(botAI, bot);
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    Player* longbowTank = GetNetherstrandLongbowTank(botAI, bot);
    if (longbowTank && longbowTank == bot)
        return false;

    return IsAnyLegendaryWeaponAlive(botAI) && !botAI->IsMainTank(bot);
}

bool KaelthasSunstriderDevastationChannelsWhirlwindTrigger::IsActive()
{
    Unit* devastation = AI_VALUE2(Unit*, "find target", "devastation");
    if (!devastation)
        return false;

    return devastation->IsAlive() && botAI->IsMainTank(bot);
}

bool KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger::IsActive()
{
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    if (!longbow || !longbow->IsAlive())
        return false;

    Player* longbowTank = GetNetherstrandLongbowTank(botAI, bot);

    return longbowTank && longbowTank == bot;
}

// phase 3 kill order

// phase 2 looting weapons

// phase 3 and onward using weapons

// phase 4 onward break MC (use weak abilities)

// phase 4 round up phoenixes

// phase 4 kill eggs

// phase 5 spread in air

bool KaelthasSunstriderCheatToTestTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas != nullptr;
}
