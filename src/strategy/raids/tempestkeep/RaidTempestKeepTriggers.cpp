#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

bool CrimsonHandCenturionCastsArcaneVolleyTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* centurion = AI_VALUE2(Unit*, "find target", "crimson hand centurion");

    return centurion && centurion->IsAlive() && centurion->HasAura(SPELL_ARCANE_FLURRY);
}

bool AlarPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetHealthPct() > 98.0f;
}

bool AlarEngagedByTanksInPhase1Trigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && !isPhase2[alar->GetMapId()];
}

bool AlarBossEngagedByMeleeDpsTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || botAI->IsTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && !isPhase2[alar->GetMapId()];
}

bool AlarBossEngagedByRangedDpsTrigger::IsActive()
{
    if (!botAI->IsRangedDps(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar != nullptr;
}

bool AlarBossEngagedByHealerTrigger::IsActive()
{
    if (!botAI->IsHeal(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && !isPhase2[alar->GetMapId()];
}

bool AlarEmbersOfAlarSpawnedTrigger::IsActive()
{
    if (!IsAlarAddTank(botAI, bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar != nullptr;
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetHealthPct() < 95.0f && alar->GetPositionZ() >= 22.0f &&
           bot->GetPositionZ() >= 17.0f && !isPhase2[alar->GetMapId()];
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool AlarEngagedByTanksInPhase2Trigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && isPhase2[alar->GetMapId()];
}

bool AlarPhase2EncounterIsAtRoomCenterTrigger::IsActive()
{
    if (bot->GetVictim())
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetPositionZ() < 42.0f && isPhase2[alar->GetMapId()];
}

bool AlarBossIsPreparingToDiveBombTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar && alar->GetPositionZ() >= 42.0f && isPhase2[alar->GetMapId()];
}

bool AlarNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");

    return alar != nullptr;
}

bool VoidReaverBossEngagedByTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossLaunchesArcaneOrbsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver != nullptr;
}

bool VoidReaverTanksLostAggroTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && voidReaver->GetVictim() == bot;
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
    if (!botAI->IsMelee(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");

    return solariumPriest != nullptr;
}

bool HighAstromancerSolarianTransformedIntoVoidwalkerTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBossCastsPsychicScreamTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
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

    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderNeedToTestStrategiesOnBossTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && IsKaelthasMapIDTimerManager(botAI, bot);
}

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");

    return thaladred && thaladred->GetVictim() == bot;
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");

    return sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && sanguinar->IsAlive();
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    if (!(IsKaelthasInPhase1(botAI) || IsKaelthasInPhase2(botAI) || IsKaelthasInPhase3(botAI)))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false; // Defensive check for edge case

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

    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderCapernianRequiresAWarlockTankTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return bot == GetCapernianTank(botAI, bot);
}

bool KaelthasSunstriderCapernianEngagedByWarlockTankTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || !capernian->IsAlive())
        return false;

    Player* capernianTank = GetCapernianTank(botAI, bot);

    return capernianTank && bot == capernianTank;
}

bool KaelthasSunstriderCapernianCastsArcaneBurstTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || !capernian->IsAlive())
        return false;

    Player* capernianTank = GetCapernianTank(botAI, bot);

    return !capernianTank || bot != capernianTank;
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    return telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           telonicus->IsAlive() && botAI->IsAssistTankOfIndex(bot, 0);
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && (IsKaelthasInPhase1(botAI) || IsKaelthasInPhase3(botAI));
}

bool KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && IsKaelthasMapIDTimerManager(botAI, bot) && IsKaelthasInPhase1(botAI);
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;
    
    Player* longbowTank = GetNetherstrandLongbowTank(botAI, bot);
    if (longbowTank && longbowTank == bot)
        return false;
    
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && IsKaelthasInPhase2(botAI);
}

bool KaelthasSunstriderDevastationChannelsWhirlwindTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;
    
    Unit* devastation = AI_VALUE2(Unit*, "find target", "devastation");

    return devastation && devastation->IsAlive();
}

bool KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger::IsActive()
{
    Player* longbowTank = GetNetherstrandLongbowTank(botAI, bot);
    if (longbowTank != bot)
        return false;
    
    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");

    return longbow && longbow->IsAlive();
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && AreAllLegendaryWeaponsDead(botAI, bot);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return bot->HasItemCount(ITEM_STAFF_OF_DISINTEGRATION, 1, false) ||
           bot->HasItemCount(ITEM_NETHERSTRAND_LONGBOW, 1, false) ||
           bot->HasItemCount(ITEM_PHASESHIFT_BULWARK, 1, false);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas && (IsKaelthasInPhase1(botAI) || IsKaelthasInPhase3(botAI));
}

// phase 4 onward break MC (use weak abilities)

// phase 4 round up phoenixes

// phase 4 kill eggs

// phase 5 spread in air

bool KaelthasSunstriderCheatToTestTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");

    return kaelthas != nullptr;
}
