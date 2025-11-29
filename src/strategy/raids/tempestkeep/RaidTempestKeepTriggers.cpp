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
    return centurion && centurion->HasAura(SPELL_ARCANE_FLURRY);
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
    return alar && !isAlarInPhase2[TEMPESTKEEP_MAP_ID];
}

bool AlarBossEngagedByMeleeDpsTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar != nullptr;
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
    return alar != nullptr;
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
           bot->GetPositionZ() >= 17.0f && !isAlarInPhase2[TEMPESTKEEP_MAP_ID];
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
    return alar && isAlarInPhase2[TEMPESTKEEP_MAP_ID];
}

bool AlarPhase2EncounterIsAtRoomCenterTrigger::IsActive()
{
    if (bot->GetVictim())
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && alar->GetPositionZ() < 42.0f && isAlarInPhase2[TEMPESTKEEP_MAP_ID];
}

bool AlarBossIsPreparingToDiveBombTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && alar->GetPositionZ() >= 42.0f && isAlarInPhase2[TEMPESTKEEP_MAP_ID];
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

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    return thaladred && thaladred->GetVictim() == bot;
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot) &&
        !(IsKaelthasInPhase1(botAI) && botAI->IsTank(bot)))
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    return sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (!sanguinar)
        return false;

    if (IsKaelthasInPhase4(botAI) || IsKaelthasInPhase5(botAI))
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

bool KaelthasSunstriderCapernianRequiresAWarlockTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (!capernianTank || capernianTank != bot)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas != nullptr;
}

bool KaelthasSunstriderCapernianEngagedByWarlockTankTrigger::IsActive()
{
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (!capernianTank || capernianTank != bot)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    return capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderCapernianCastsArcaneBurstTrigger::IsActive()
{
    Player* capernianTank = GetCapernianTank(botAI, bot);
    if (capernianTank && capernianTank == bot)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    return capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0) &&
        !(IsKaelthasInPhase1(botAI) && botAI->IsTank(bot)))
        return false;

    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    return telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
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
    return devastation != nullptr;
}

bool KaelthasSunstriderNetherstrandLongbowFiresMultiShotTrigger::IsActive()
{
    Player* longbowTank = GetNetherstrandLongbowTank(botAI, bot);
    if (!longbowTank || longbowTank != bot)
        return false;

    Unit* longbow = AI_VALUE2(Unit*, "find target", "netherstrand longbow");
    return longbow != nullptr;
    }

bool KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && IsAnyLegendaryWeaponDead(botAI, bot);
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

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != 550)
        return false;

    const uint32 KAELTHAS_DB_GUID = 158218;

    Map* map = bot->GetMap();
    if (!map)
        return false;

    auto it = map->GetCreatureBySpawnIdStore().find(KAELTHAS_DB_GUID);
    if (it == map->GetCreatureBySpawnIdStore().end())
        return false;

    Creature* kaelthas = it->second;
    if (!kaelthas)
        return false;

    float distance = bot->GetExactDist2d(kaelthas);
    if (distance > 150.0f)
        return false;

    // Check if bot has a 2H weapon equipped in mainhand
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    bool has2HWeapon = mainHand && mainHand->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        // Skip offhand check if bot has 2H weapon equipped
        if (slot == EQUIPMENT_SLOT_OFFHAND && has2HWeapon)
            continue;

        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            return true;
    }

    return false;
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && (IsKaelthasInPhase1(botAI) || IsKaelthasInPhase3(botAI));
}

bool KaelthasSunstriderFlameStrikeAppearedUnderBotTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && (IsKaelthasInPhase4(botAI) || IsKaelthasInPhase5(botAI));
}

bool KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix");
    Unit* phoenixEgg = AI_VALUE2(Unit*, "find target", "phoenix egg");
    return phoenix != nullptr || phoenixEgg != nullptr;
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActive()
{
    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true) || botAI->IsTank(bot))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
                return true;
        }
    }

    return false;
}

bool KaelthasSunstriderBossIsCastingPyroblastTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && kaelthas->HasAura(SPELL_SHOCK_BARRIER);
}

bool KaelthasSunstriderBossIsManipulatingGravityTrigger::IsActive()
{
    return bot->HasAura(SPELL_GRAVITY_LAPSE);
}
