/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidBlackTempleTriggers.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleActions.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace BlackTempleHelpers;

// General

bool BlackTempleBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
}

// High Warlord Naj'entus

bool HighWarlordNajentusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus && najentus->GetHealthPct() > 95.0f;
}

bool HighWarlordNajentusBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusPlayerIsImpaledTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;
    // Find any player with Impaling Spine (other than bot itself, duh)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            impaledPlayer = member;
            break;
        }
    }

    Player* closestBot = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    // Find the closest non-tank bot to the impaled player
    if (impaledPlayer)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == impaledPlayer ||
                !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
                continue;

            float dist = member->GetDistance(impaledPlayer);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestBot = member;
            }
        }
    }

    return closestBot == bot;
}

bool HighWarlordNajentusBossHasTidalShieldTrigger::IsActive()
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus || !najentus->HasAura(SPELL_TIDAL_SHIELD))
        return false;

    return botAI->HasItemInInventory(ITEM_NAJENTUS_SPINE);
}

// Supremus

bool SupremusPullingBossOrChangingPhaseTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    auto it = supremusPhaseTimer.find(supremus->GetMap()->GetInstanceId());
    if (it == supremusPhaseTimer.end())
        return false; // Timer not started yet

    time_t now = time(nullptr);
    time_t elapsed = now - it->second;

    // Fire during first 10 seconds, or during 60-70, 120-130, etc.
    if ((elapsed < 10) || ((elapsed % 60) < 10 && elapsed >= 60))
        return true;

    return false;
}

bool SupremusBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && !supremus->HasAura(SPELL_SNARE_SELF);
}

bool SupremusBossIsFixatedOnBotTrigger::IsActive()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && supremus->HasAura(SPELL_SNARE_SELF) &&
           supremus->GetVictim() == bot;
}

bool SupremusVolcanoIsNearbyTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "supremus") &&
           HasSupremusVolcanoNearby(botAI, bot);
}

bool SupremusNeedToManagePhaseTimerTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    return IsMechanicTrackerBot(botAI, bot, BLACK_TEMPLE_MAP_ID);
}

// Teron Gorefiend

bool TeronGorefiendPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    return gorefiend && gorefiend->GetHealthPct() > 95.0f;
}

bool TeronGorefiendBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendBossEngagedByRangedTrigger::IsActive()
{
    return botAI->IsRanged(bot) /* && !bot->HasAura(SPELL_SPIRITUAL_VENGEANCE)*/ &&
           AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendBossIsCastingShadowOfDeathTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER &&
        bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend || !gorefiend->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = gorefiend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_SHADOW_OF_DEATH)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool TeronGorefiendBotHasShadowOfDeathTrigger::IsActive()
{
    Aura* aura = bot->GetAura(SPELL_SHADOW_OF_DEATH);
    return aura && aura->GetDuration() < 12000;
}

bool TeronGorefiendBotTransformedIntoVengefulSpiritTrigger::IsActive()
{
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE);
}

// Gurtogg Bloodboil

bool GurtoggBloodboilPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return false;

    time_t elapsed = std::time(nullptr) - it->second;
    return elapsed < 10;
}

bool GurtoggBloodboilBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE);
}

bool GurtoggBloodboilBossCastsBloodboilTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE);
}

bool GurtoggBloodboilBotHasFelRageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg || !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura(SPELL_PLAYER_FEL_RAGE))
                return true;
        }
    }

    return false;
}

bool GurtoggBloodboilNeedToManagePhaseTimerTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, BLACK_TEMPLE_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
}

// Reliquary of Souls

bool ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER &&
           AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}

bool ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "essence of suffering");
}

bool ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger::IsActive()
{
    if (!botAI->IsHeal(bot) || bot->getClass() == CLASS_PRIEST)
        return false;

    return AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(SPELL_RUNE_SHIELD);
}

bool ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->getClass() != CLASS_WARRIOR)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_DEADEN)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

// Mother Shahraz

bool MotherShahrazPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    return shahraz && shahraz->GetHealthPct() > 95.0f;
}

bool MotherShahrazBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(SPELL_FATAL_ATTRACTION);
}

bool MotherShahrazTanksArePositioningBossTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < 90.0f)
        return false;

    return shahraz->GetDistance2d(SHAHRAZ_TANK_POSITION.GetPositionX(),
                                  SHAHRAZ_TANK_POSITION.GetPositionY()) > 10.0f;
}

bool MotherShahrazSinisterBeamKnocksBackPlayersTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(SPELL_FATAL_ATTRACTION);
}

bool MotherShahrazBotsAreLinkedByFatalAttractionTrigger::IsActive()
{
    return bot->HasAura(SPELL_FATAL_ATTRACTION);
}

// Illidari Council

bool IllidariCouncilPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    return gathios && gathios->GetHealthPct() > 95.0f;
}

bool IllidariCouncilGathiosEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

bool IllidariCouncilGathiosCastingJudgementOfCommandTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARRIOR || !botAI->IsMainTank(bot))
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || !gathios->HasAura(SPELL_SEAL_OF_COMMAND) ||
        !gathios->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = gathios->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_JUDGEMENT)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool IllidariCouncilMalandeEngagedByFirstAssistTankTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "lady malande");
}

bool IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, false))
        return false;

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    return darkshadow && !darkshadow->HasAura(SPELL_VANISH);
}

bool IllidariCouncilZerevorEngagedByMageTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE || GetZerevorMageTank(bot) != bot)
        return false;

    return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilMageTankNeedsDedicatedHealerTrigger::IsActive()
{
    return botAI->IsAssistHealOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilZerevorCastsDangerousAoesTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return false;

    return !HasDangerousCouncilAura(bot);
}

bool IllidariCouncilPetsScrewUpThePullTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

bool IllidariCouncilDeterminingDpsAssignmentsTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, false))
        return false;

    if (GetZerevorMageTank(bot) == bot)
        return false;

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (botAI->IsTank(bot) && botAI->IsAssistTankOfIndex(bot, 1, false) &&
        darkshadow && !darkshadow->HasAura(SPELL_VANISH))
        return false;

    return true;
}

bool IllidariCouncilNeedToManageDpsTimerTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    return IsMechanicTrackerBot(botAI, bot, BLACK_TEMPLE_MAP_ID, GetZerevorMageTank(bot));
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageTankNeedsAggroTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->GetHealth() > 1;
}

bool IllidanStormrageBossCastsFlameCrashTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);
    return phase == 1 || phase == 3 || phase == 5;
}

bool IllidanStormrageBotHasParasiticShadowfiendTrigger::IsActive()
{
    if (botAI->IsMelee(bot))
        return false;

    if (!bot->HasAura(SPELL_PARASITIC_SHADOWFIEND_1) &&
        !bot->HasAura(SPELL_PARASITIC_SHADOWFIEND_2))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 1;
}

bool IllidanStormrageBossSummonedFlamesOfAzzinothTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true) &&
        !botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 2;
}

bool IllidanStormragePetsDieToFireTrigger::IsActive()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool IllidanStormrageGrateIsSafeFromFlamesTrigger::IsActive()
{
    if (botAI->IsAssistTankOfIndex(bot, 0, true) ||
        botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 2;
}

bool IllidanStormrageBotStruckByDarkBarrageTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE)
        return false;

    return bot->HasAura(SPELL_DARK_BARRAGE);
}

bool IllidanStormrageBossIsPreparingToLandTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 0;
}

bool IllidanStormrageBossDealsSplashDamageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 4 && GetIllidanWarlockTank(bot) == bot)
        return false;

    return GetIllidanPhase(illidan) == 3 ||
           GetIllidanPhase(illidan) == 4 ||
           GetIllidanPhase(illidan) == 5;
}

bool IllidanStormrageThisExpansionHatesMeleeTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 4;
}

bool IllidanStormrageBossTransformsIntoDemonTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 4)
        return false;

    return GetIllidanWarlockTank(bot) == bot;
}

bool IllidanStormrageBossSpawnsAddsTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    if (botAI->IsTank(bot) && GetIllidanPhase(illidan) != 4)
        return false;

    return true;
}

bool IllidanStormrageNeedToManageDpsTimerTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    return IsMechanicTrackerBot(
        botAI, bot, BLACK_TEMPLE_MAP_ID, GetIllidanWarlockTank(bot));
}

bool IllidanStormrageCheatTrigger::IsActive()
{
    if (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    int phase = GetIllidanPhase(illidan);
    if (phase != 0 && phase != 2 && phase != 4)
        return false;

    return IsMechanicTrackerBot(
        botAI, bot, BLACK_TEMPLE_MAP_ID, GetIllidanWarlockTank(bot));
}
