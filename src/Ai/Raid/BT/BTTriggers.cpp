/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTTriggers.h"
#include "AiFactory.h"
#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Timer.h"

using namespace BlackTempleHelpers;
using namespace EncounterHelpers;

// General

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BLACK_TEMPLE_MAP_ID);
}

// Shared Bosses

bool BlackTemplePullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// High Warlord Naj'entus

bool HighWarlordNajentusShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusPlayerIsImpaledTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(Id(BlackTempleSpells::SPELL_IMPALING_SPINE)))
        {
            impaledPlayer = member;
            break;
        }
    }

    Player* closestBot = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    if (impaledPlayer)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == impaledPlayer ||
                !GET_PLAYERBOT_AI(member) || PlayerbotAI::IsTank(member))
            {
                continue;
            }

            float const dist = member->GetDistance(impaledPlayer);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestBot = member;
            }
        }
    }

    return closestBot == bot;
}

bool HighWarlordNajentusHasTidalShieldTrigger::IsActiveInEncounter()
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus || !najentus->HasAura(Id(BlackTempleSpells::SPELL_TIDAL_SHIELD)))
        return false;

    return botAI->HasItemInInventory(Id(BlackTempleItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusPullingBossOrChangingPhaseTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    auto it = supremusPhaseTimer.find(supremus->GetMap()->GetInstanceId());
    if (it == supremusPhaseTimer.end())
        return false;

    constexpr uint32 activeWindowMs = 10 * IN_MILLISECONDS;
    constexpr uint32 phaseCycleMs = 60 * IN_MILLISECONDS;
    uint32 const elapsed = GetMSTimeDiffToNow(it->second);

    // Active during first 10 seconds, or during 60-70, 120-130, etc.
    return (elapsed < activeWindowMs) ||
        ((elapsed % phaseCycleMs) < activeWindowMs && elapsed >= phaseCycleMs);
}

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && !supremus->HasAura(Id(BlackTempleSpells::SPELL_SNARE_SELF));
}

bool SupremusFixatesOnBotTrigger::IsActiveInEncounter()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && supremus->GetVictim() == bot &&
        supremus->HasAura(Id(BlackTempleSpells::SPELL_SNARE_SELF));
}

bool SupremusNearVolcanoTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "supremus") && HasSupremusVolcanoNearby(botAI);
}

bool SupremusShouldManagePhaseTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "supremus");
}

// Shade of Akama

bool ShadeOfAkamaKillingChannelersStartsPhase2Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot) || !PlayerbotAI::IsMelee(bot))
        return false;

    constexpr float searchRadius = 30.0f;
    Unit* channeler = bot->FindNearestCreature(
        Id(BlackTempleNpcs::NPC_ASHTONGUE_CHANNELER), searchRadius, true);

    return channeler && !channeler->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

// Teron Gorefiend

bool TeronGorefiendShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendRangedShouldPositionOnBalconyTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendCastsShadowOfDeathTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_ROGUE)
    {
        return false;
    }

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    if (botAI->HasAura("feign death", bot))
    {
        botAI->RemoveAura("feign death");
        return true;
    }
    else if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!PlayerbotAI::IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    if (!gorefiend->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = gorefiend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != Id(BlackTempleSpells::SPELL_SHADOW_OF_DEATH))
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool TeronGorefiendShadowOfDeathTrigger::IsActiveInEncounter()
{
    Aura* aura = bot->GetAura(Id(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    return aura && aura->GetDuration() < 12000;
}

bool TeronGorefiendTransformedIntoVengefulSpiritTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BlackTempleSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return false;

    constexpr uint32 engageWindowMs = 10 * IN_MILLISECONDS;
    return GetMSTimeDiffToNow(it->second) < engageWindowMs;
}

bool GurtoggBloodboilShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(Id(BlackTempleSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilCastsBloodboilTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(Id(BlackTempleSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilFelRageOnGroupMemberTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg || !gurtogg->HasAura(Id(BlackTempleSpells::SPELL_BOSS_FEL_RAGE)))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura(Id(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)))
                return true;
        }
    }

    return false;
}

bool GurtoggBloodboilShouldManagePhaseTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
}

// Reliquary of Souls

bool ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER && AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}

bool ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "essence of suffering");
}

bool ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    if (bot->getClass() == CLASS_PRIEST && AiFactory::GetPlayerSpecTab(bot) == PRIEST_TAB_DISCIPLINE)
        return false;

    return AI_VALUE2(Unit*, "find target", "essence of suffering");
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(Id(BlackTempleSpells::SPELL_RUNE_SHIELD));
}

bool ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != Id(BlackTempleSpells::SPELL_DEADEN))
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

// Mother Shahraz

bool MotherShahrazShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(Id(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazTanksArePositioningBossTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < 90.0f)
        return false;

    TankPositionState const tankState = GetShahrazTankPositionState(bot);
    return tankState != TankPositionState::Positioned;
}

bool MotherShahrazRangedShouldPositionUnderPillarTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(Id(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazFatalAttractionTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

// Illidari Council

bool IllidariCouncilGathiosShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "gathios the shatterer") &&
        PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilGathiosCastsJudgementOfCommandTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || !gathios->HasUnitState(UNIT_STATE_CASTING) ||
        !gathios->HasAura(Id(BlackTempleSpells::SPELL_SEAL_OF_COMMAND)))
    {
        return false;
    }

    Spell* spell = gathios->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != Id(BlackTempleSpells::SPELL_JUDGEMENT))
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID() && PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilMalandeShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "lady malande") &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
}

bool IllidariCouncilDarkshadowShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    return darkshadow && !darkshadow->HasAura(Id(BlackTempleSpells::SPELL_VANISH)) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
}

bool IllidariCouncilZerevorShouldBeTankedByMageTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor") &&
        GetZerevorMageTank(botAI) == bot;
}

bool IllidariCouncilMageTankNeedsDedicatedHealerTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsHeal(bot) && AI_VALUE2(Unit*, "find target", "high nethermancer zerevor") &&
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, true);
}

bool IllidariCouncilRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return false;

    return !HasDangerousCouncilAura(bot);
}

bool IllidariCouncilPetsScrewUpThePullTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

bool IllidariCouncilShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    bool const isTank = PlayerbotAI::IsTank(bot);
    if ((isTank && (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))) ||
        (bot->getClass() == CLASS_MAGE && GetZerevorMageTank(botAI) == bot))
    {
        return false;
    }

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (isTank && darkshadow && !darkshadow->HasAura(Id(BlackTempleSpells::SPELL_VANISH)) &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        return false;
    }

    return true;
}

bool IllidariCouncilShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageTankNeedsAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->GetHealth() > 1;
}

bool IllidanStormrageCastsFlameCrashTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int const phase = GetIllidanPhase(illidan);
    return (phase == 1 || phase == 3 || phase == 5) && PlayerbotAI::IsMainTank(bot);
}

bool IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 || illidan->GetVictim() == bot)
        return false;

    int const phase = GetIllidanPhase(illidan);
    if (phase == 2 || phase == 4)
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    if (phase == 5 && FindNearestTrap(botAI))
        return false;

    Player* infected = GetBotWithParasiticShadowfiend(botAI);
    if (!infected)
        return false;

    if (infected == bot || (phase != 1 && bot->getClass() == CLASS_HUNTER))
        return true;

    return false;
}

bool IllidanStormrageParasiticShadowfiendsRunWildTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 || GetIllidanPhase(illidan) == 2)
        return false;

    ObjectGuid const guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_EARTH];
    if (guid.IsEmpty())
        return true;

    Creature* totem = bot->GetMap()->GetCreature(guid);
    return !totem || totem->GetDistance(bot) > 20.0f ||
        totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) != Id(BlackTempleSpells::SPELL_EARTHBIND_TOTEM);
}

bool IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 2)
        return false;

    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool IllidanStormragePetsDieToFireTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool IllidanStormrageGrateIsSafeFromFlamesTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 2)
        return false;

    return !PlayerbotAI::IsTank(bot) ||
        (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            !PlayerbotAI::IsAssistTankOfIndex(bot, 1, true));
}

bool IllidanStormrageDarkBarrageOnImmunityClassTrigger::IsActiveInEncounter()
{
    if (!GetSelfImmunitySpell(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "illidan stormrage"))
        return false;

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!PlayerbotAI::IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    return bot->HasAura(Id(BlackTempleSpells::SPELL_DARK_BARRAGE));
}

bool IllidanStormragePreparesToLandTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 0)
        return false;

    return !PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsMainTank(bot);
}

bool IllidanStormrageRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->HasAura(Id(BlackTempleSpells::SPELL_CAGED)))
        return false;

    int const phase = GetIllidanPhase(illidan);

    if (phase == 4 && GetIllidanWarlockTank(botAI) == bot)
        return false;

    return phase == 3 || phase == 4 || phase == 5;
}

bool IllidanStormrageThisExpansionHatesMeleeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 4;
}

bool IllidanStormrageWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 4)
        return false;

    return GetIllidanWarlockTank(botAI) == bot;
}

bool IllidanStormrageShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    if (PlayerbotAI::IsTank(bot) && GetIllidanPhase(illidan) != 4)
        return false;

    return true;
}

bool IllidanStormrageMaievPlacedShadowTrapTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 || GetIllidanPhase(illidan) != 5)
        return false;

    GameObject* trap = FindNearestTrap(botAI);
    if (!trap)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* closestBot = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || illidan->GetVictim() == member ||
            !GET_PLAYERBOT_AI(member) ||
            (PlayerbotAI::IsTank(member) && PlayerbotAI::IsMainTank(member)))
        {
            continue;
        }

        float const dist = member->GetDistance(trap);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestBot = member;
        }
    }

    return closestBot == bot;
}

bool IllidanStormrageShouldManageDpsTimerAndRtiTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->GetHealth() > 1;
}

// Destroying hazards behind phases is not gated behind CheatMask
// The strategy simply cannot work without doing this
bool IllidanStormrageShouldClearHazardsBetweenPhasesTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    int const phase = GetIllidanPhase(illidan);
    return phase == 0 || phase == 2 || phase == 4;
}

bool IllidanStormrageCheatTrigger::IsActiveInEncounter()
{
    if (!botAI->HasCheat(BotCheatMask::raid) || !PlayerbotAI::IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int const phase = GetIllidanPhase(illidan);
    return phase == 2 || phase == 4;
}
