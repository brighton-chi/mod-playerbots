#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

// Trash

bool CrimsonHandCenturionCastsArcaneVolleyTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    return AI_VALUE2(Unit*, "find target", "crimson hand centurion");
}

// Al'ar <Phoenix God>

bool AlarPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && alar->GetHealthPct() > 98.0f;
}

bool AlarBossIsFlyingBetweenPlatformsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || !alar->IsVisible())
        return false;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    if (locationIndex == POINT_QUILL_OR_DIVE_IDX ||
        locationIndex == POINT_MIDDLE_IDX)
        return false;

    return true;
}

bool AlarBossSpawnsEmbersOfAlarTrigger::IsActive()
{
    if (!botAI->IsTank(bot) && !botAI->IsRangedDps(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    return locationIndex == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    Creature* alarCreature = alar->ToCreature();
    return alarCreature && alarCreature->GetReactState() == REACT_PASSIVE;
}

bool AlarEverythingIsOnFireInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return isAlarInPhase2[alar->GetMap()->GetInstanceId()];
}

bool AlarPhase2EncounterIsAtRoomCenterTrigger::IsActive()
{
    if (bot->GetVictim())
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    return locationIndex != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarStrategyChangesBetweenPhasesTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsDps(member) &&
                GET_PLAYERBOT_AI(member))
                return member == bot;
        }
    }

    return false;
}

// Void Reaver

bool VoidReaverKnockAwayReducesTankAggroTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) && !botAI->IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossLaunchesArcaneOrbsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "void reaver");
}

// High Astromancer Solarian

bool HighAstromancerSolarianBossCastsWrathOfTheAstromancerTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    return astromancer && !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool HighAstromancerSolarianBossHasVanishedTrigger::IsActive()
{
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Creature* astromancerCreature = astromancer->ToCreature();
    return astromancerCreature &&
           astromancerCreature->GetReactState() == REACT_PASSIVE;
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "solarium priest");
}

bool HighAstromancerSolarianTransformedIntoVoidwalkerTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (botAI->IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
        return false;

    return true;
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
           kaelAI->GetPhase() == PHASE_ALL_ADVISORS);
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    return sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR &&
        kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
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

    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderCapernianShouldBeTankedByAWarlockTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (GetCapernianTank(botAI, bot) != bot)
        return false;

    return AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
}

bool KaelthasSunstriderCapernianCastsArcaneBurstAndConflagrationTrigger::IsActive()
{
    if (GetCapernianTank(botAI, bot) == bot)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    return capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    return telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderNeedDedicatedBotsForSanguinarAndTelonicusInPhase3Trigger::IsActive()
{
    if (!botAI->IsHealAssistantOfIndex(bot, 0) &&
        !botAI->IsMainTank(bot) &&
        !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return false;

    if (AI_VALUE2(Unit*, "find target", "master engineer telonicus") ||
        AI_VALUE2(Unit*, "find target", "lord sanguinar"))
        return true;

    return false;
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
           kaelAI->GetPhase() == PHASE_ALL_ADVISORS);
}

bool KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger::IsActive()
{
    if (!IsKaelthasInstanceTimerManager(botAI, bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR;
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_WEAPONS;
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && IsAnyLegendaryWeaponDead(botAI, bot);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return false;

    return bot->HasItemCount(ITEM_STAFF_OF_DISINTEGRATION, 1, false) ||
           bot->HasItemCount(ITEM_NETHERSTRAND_LONGBOW, 1, false) ||
           bot->HasItemCount(ITEM_PHASESHIFT_BULWARK, 1, false);
}

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TEMPEST_KEEP_MAP_ID)
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

    if (bot->GetExactDist2d(kaelthas) > 150.0f)
        return false;

    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    bool has2HWeapon = mainHand && mainHand->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        if (slot == EQUIPMENT_SLOT_OFFHAND && has2HWeapon)
            continue;

        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (slot == EQUIPMENT_SLOT_OFFHAND)
            {
                if (!mainHand || mainHand->GetTemplate()->InventoryType != INVTYPE_WEAPON)
                    continue;
                if (HasEquippableOffhand(bot))
                    return true;
            }
            else if (HasEquippableItemForSlot(bot, slot))
                return true;
        }
    }

    return false;
}

bool KaelthasSunstriderBossHasEnteredTheFightTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_FINAL;
}

bool KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger::IsActive()
{
    if (AI_VALUE2(Unit*, "find target", "phoenix"))
        return true;

    if (AI_VALUE2(Unit*, "find target", "phoenix egg"))
        return true;

    return false;
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActive()
{
    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true) || botAI->IsTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
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
