/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTTriggers.h"
#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Spell.h"
#include <algorithm>
#include <vector>

using namespace BtHelpers;
using namespace EncounterHelpers;

// Shared

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BT_MAP_ID);
}

bool BlackTempleClassRolesShouldBeAssignedTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID);
}

bool BlackTempleHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool SisterOfPleasureShouldBeFocusedTrigger::IsActive()
{
    if (IsMechanicTrackerBot(bot, BT_MAP_ID))
        return !GetSkulledSisterOfPain(bot) && FindLinkedSisterOfPleasure(botAI);

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == Id(BtNpcs::NPC_SISTER_OF_PAIN) &&
        GetSkulledSisterOfPain(bot) == target;
}

bool ShadowmoonReaverWandBuildsChargesTrigger::IsActive()
{
    Spell* wand = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    if (!wand || GetChaoticChargeReach(wand->m_spellInfo) == ChaoticChargeReach::None)
        return false;

    return IsShadowmoonReaverUnsafeForMagic(wand->m_targets.GetUnitTarget());
}

bool ShadowmoonReaverShouldControlCasterPetTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!IsChargeBuildingPet(pet))
        return false;

    // Also true while the pet is passive, so the action can restore a stance it set.
    return pet->HasReactState(REACT_PASSIVE) ||
        !context->GetValue<GuidVector>("shadowmoon reavers")->RefGet().empty();
}

// High Warlord Naj'entus

bool HighWarlordNajentusShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    return GetNearestPlayerInRadius(bot, NAJENTUS_RANGED_SPREAD_DISTANCE);
}

bool HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    return FindNajentusUnassignedImpaledPlayer(bot);
}

bool HighWarlordNajentusImpalingSpineOnGroupMemberTrigger::IsActiveInEncounter()
{
    return GetNajentusImpaledPlayerToFree(bot);
}

bool HighWarlordNajentusNeedsSpineThrowerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    return !GetNajentusSpineThrower(bot);
}

bool HighWarlordNajentusHasTidalShieldTrigger::IsActiveInEncounter()
{
    if (!IsNajentusSpineThrower(bot) || IsNajentusSpineInFlight(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus || !najentus->HasAura(Id(BtSpells::SPELL_TIDAL_SHIELD)))
        return false;

    return bot->HasItemCount(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || IsSupremusKitePhase(supremus))
        return false;

    // The spread step can go any way and doesn't see fire, so it waits while one step could carry
    // the bot into a volcano's safe zone.
    float const fleeStep =
        std::min(SUPREMUS_RANGED_SPREAD_DISTANCE + 1.0f, sPlayerbotAIConfig.FleeDistance);
    if (IsInSupremusHazard(
            GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
            SupremusHazardZone::Safe, fleeStep))
    {
        return false;
    }

    return GetNearestPlayerInRadius(bot, SUPREMUS_RANGED_SPREAD_DISTANCE);
}

bool SupremusFixatedOnBotTrigger::IsActiveInEncounter()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || supremus->GetVictim() != bot || !IsSupremusKitePhase(supremus))
        return false;

    // The kite multiplier holds every other mover, fire avoidance included.
    if (CanSupremusCatchStandingBot(bot, supremus))
        return true;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusNearFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    if (bot->isMoving() && CanWalkThroughSupremusFire(bot))
        return false;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusReachBlockedByFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    return GetSupremusFireBlockedReachTarget(botAI);
}

// Shade of Akama

bool ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger::IsActiveInEncounter()
{
    bool const isMeleeDps = PlayerbotAI::IsDps(bot) && PlayerbotAI::IsMelee(bot);
    bool const isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    if (!isMeleeDps && !isShadowPriest && !IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    return GetShadeOfAkamaKillTarget(botAI);
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
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    // Once the bolt is launched, nothing cancels it.
    Spell* shadow = gorefiend->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->getState() == SPELL_STATE_PREPARING &&
        shadow->m_targets.GetUnitTarget() == bot;
}

bool TeronGorefiendShouldPositionForVengefulSpiritTrigger::IsActiveInEncounter()
{
    Aura* shadow = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->GetDuration() <= GOREFIEND_SHADOW_OF_DEATH_MOVE_MS;
}

bool TeronGorefiendTransformedIntoVengefulSpiritTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilShouldPositionForBloodboilTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // He doesn't cast Bloodboil during Fel Rage.
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg || gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)))
        return false;

    return !IsGurtoggFelRageTarget(bot);
}

bool GurtoggBloodboilMeleeShouldStayBehindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    if (IsGurtoggFelRageTarget(bot))
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    return bot->GetExactDist2d(GetGurtoggFelRageMeleePosition(bot, *gurtogg, *felRageTarget)) >
        GURTOGG_FEL_RAGE_MELEE_TOLERANCE;
}

// Melee go behind him instead.
bool GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMelee(bot) || IsGurtoggFelRageTarget(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget || bot->GetExactDist2d(felRageTarget) >= GURTOGG_FEL_RAGE_AVOID_DISTANCE)
        return false;

    // Arcing Smash only hits in front of him, but he turns faster than bots can react, so at the
    // start of Fel Rage everyone near the target moves regardless of side.
    Aura* felRage = gurtogg->GetAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
    if (felRage && GetAuraElapsedMs(felRage) < GURTOGG_FEL_RAGE_EARLY_AVOID_MS)
        return true;

    return gurtogg->HasInArc(static_cast<float>(M_PI), bot);
}

bool GurtoggBloodboilFelRageOnBotTrigger::IsActiveInEncounter()
{
    return IsGurtoggFelRageTarget(bot) &&
        bot->GetExactDist2d(GURTOGG_TANK_POSITION) > GURTOGG_POSITION_TOLERANCE;
}

// Reliquary of Souls

bool ReliquaryOfSoulsHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (desire && desire->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return true;

    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    return anger && anger->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool ReliquaryOfSoulsShouldPositionForSufferingTrigger::IsActiveInEncounter()
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        IsOutOfSufferingPosition(bot, suffering);
}

bool ReliquaryOfSoulsHealersShouldAttackSufferingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Discipline priests keep shielding.
    if (bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("disc", BOT_STATE_COMBAT))
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(Id(BtSpells::SPELL_RUNE_SHIELD));
}

bool ReliquaryOfSoulsEssenceOfDesireCastsDeadenTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire)
        return false;

    Spell* deaden = desire->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_DEADEN));
    return deaden && deaden->getState() == SPELL_STATE_PREPARING &&
        deaden->m_targets.GetUnitTarget() == bot;
}

// Mother Shahraz

bool MotherShahrazShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazBeingPositionedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    return IsShahrazBeingPositioned(AI_VALUE2(Unit*, "find target", "mother shahraz"));
}

bool MotherShahrazRangedShouldPositionUnderStatueTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazFatalAttractionTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

// Illidari Council

bool IllidariCouncilGathiosShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetIllidariCouncilMembers(botAI).gathios)
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilGathiosCastsJudgementOfCommandTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* gathios = GetIllidariCouncilMembers(botAI).gathios;
    if (!gathios || !gathios->HasAura(Id(BtSpells::SPELL_SEAL_OF_COMMAND)))
        return false;

    Spell* judgement = gathios->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_JUDGEMENT));
    return judgement && judgement->getState() == SPELL_STATE_PREPARING &&
        judgement->m_targets.GetUnitTarget() == bot && PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilMalandeCastsCircleOfHealingTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_HUNTER ||
        bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_PRIEST)
    {
        return false;
    }

    if (!PlayerbotAI::IsDps(bot) || IsZerevorMageTank(botAI))
        return false;

    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.malande || !members.gathios || !members.zerevor)
        return false;

    if (PlayerbotAI::IsRanged(bot) &&
        !ShouldRangedInterruptCircleOfHealing(members.malande, members.gathios, members.zerevor))
    {
        return false;
    }

    return CanInterruptCircleOfHealing(members.malande);
}

bool IllidariCouncilMalandeShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetIllidariCouncilMembers(botAI).malande &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
}

bool IllidariCouncilVerasShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* veras = GetIllidariCouncilMembers(botAI).veras;
    if (!veras || IsVerasVanished(veras))
        return false;

    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
}

bool IllidariCouncilZerevorShouldBeTankedByMageTrigger::IsActiveInEncounter()
{
    return IsZerevorMageTank(botAI) && GetIllidariCouncilMembers(botAI).zerevor;
}

bool IllidariCouncilMageTankHealerShouldRepositionTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Unit* zerevor = GetIllidariCouncilMembers(botAI).zerevor;
    if (!zerevor)
        return false;

    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) || !IsZerevorOnMageTank(botAI, zerevor))
        return false;

    Player* mageTank = GetZerevorMageTank(botAI);
    return !IsMageTankHealerPositionSafe(bot->GetPosition(), mageTank, zerevor) ||
        !bot->IsWithinLOSInMap(mageTank);
}

bool IllidariCouncilRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!GetIllidariCouncilMembers(botAI).zerevor)
        return false;

    if (IsZerevorMageTank(botAI) || PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return false;

    return !HasDangerousCouncilAura(bot) &&
        GetNearestPlayerInRadius(bot, COUNCIL_RANGED_SPREAD_DISTANCE);
}

bool IllidariCouncilPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* target = GetCouncilPetTarget(botAI, pet);
    return target && pet->GetVictim() != target;
}

bool IllidariCouncilShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.gathios)
        return false;

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        IsZerevorMageTank(botAI))
    {
        return false;
    }

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return true;

    return !members.veras || IsVerasVanished(members.veras);
}

bool IllidariCouncilShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) && GetIllidariCouncilMembers(botAI).gathios;
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageShouldPositionForPullTrigger::IsActive()
{
    // You are not prepared!
    return AI_VALUE(bool, "illidan stormrage speech in progress");
}

bool IllidanStormrageHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev)
    {
        return IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_PHASE_START_MISDIRECT_MS);
    }

    return phase == IllidanPhase::Flying || phase == IllidanPhase::Demon;
}

bool IllidanStormrageShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Flying || phase == IllidanPhase::Dying)
        return false;

    // In Maiev's arrival, only once the tank is out of Shadow Prison.
    if (phase == IllidanPhase::None && bot->HasAura(Id(BtSpells::SPELL_SHADOW_PRISON)))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger::IsActiveInEncounter()
{
    if (!HasParasiticShadowfiend(bot) && !IsIllidanTrapHunterOnDuty(botAI))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetVictim() == bot)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None)
        return false;

    if (phase == IllidanPhase::Flying && !IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    return !IsExemptFromIllidanInfectionRun(botAI, illidan, bot);
}

bool IllidanStormrageShouldTrackInfectionsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    auto const& infected = context->GetValue<GuidVector>(
        "illidan stormrage players with parasitic shadowfiend")->RefGet();
    IllidanPhase const phase = GetIllidanPhase(illidan);
    bool const hasInfectionPosition = phase != IllidanPhase::None && phase != IllidanPhase::Flying;
    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (infections)
    {
        for (auto const& [player, infection] : *infections)
        {
            if (std::find(infected.begin(), infected.end(), player) == infected.end())
                return true;

            if (!hasInfectionPosition)
                continue;

            if (!infection.isTrapAssigned)
                return true;

            if (infection.hunter.IsEmpty())
                continue;

            // Its bot has stopped running, so the hunter is freed.
            if (IsExemptFromIllidanInfectionRun(
                    botAI, illidan, ObjectAccessor::FindPlayer(player)))
            {
                return true;
            }
        }
    }

    if (!hasInfectionPosition)
        return false;

    for (ObjectGuid const& guid : infected)
    {
        if (!infections || !infections->contains(guid))
            return true;
    }

    return false;
}

bool IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    return IsFlameOfAzzinothTank(bot) && !IsIllidanInfectionPositionKeptInFlight(botAI);
}

bool IllidanStormragePetsDieToHazardsTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    return ShouldIllidanPetsBePassive(botAI, illidan) != pet->HasReactState(REACT_PASSIVE);
}

bool IllidanStormragePetShouldAttackShadowfiendTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    // A pet's hit puts its owner on the Shadowfiend's threat list, the only entry it can act on,
    // so while its owner has just birthed Shadowfiends the pet is called off them.
    Unit* victim = pet->GetVictim();
    uint32 const victimEntry = victim && victim->IsAlive() ? victim->GetEntry() : 0;
    bool const isNearNewShadowfiend = IsNearNewParasiticShadowfiend(botAI);

    if (victimEntry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
        return isNearNewShadowfiend;

    if (victimEntry == Id(BtNpcs::NPC_SHADOW_DEMON) || isNearNewShadowfiend)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Demon)
    {
        if (GetShadowDemonForPetToAttack(botAI, *pet))
            return false;
    }
    else if (phase == IllidanPhase::Initial || phase == IllidanPhase::Flying ||
        phase == IllidanPhase::Grounded || phase == IllidanPhase::Maiev ||
        phase == IllidanPhase::Dying)
    {
        // While he flies the pet stays passive, so it takes no Flame, and only the command sends
        // it onto Shadowfiends left from Phase 1.
        if ((bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK) ||
            (phase != IllidanPhase::Flying && pet->HasReactState(REACT_PASSIVE)))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return GetNearestParasiticShadowfiend(botAI, *pet);
}

bool IllidanStormragePetShouldAttackShadowDemonTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* victim = pet->GetVictim();
    if (victim && victim->IsAlive() && victim->GetEntry() == Id(BtNpcs::NPC_SHADOW_DEMON))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return GetShadowDemonForPetToAttack(botAI, *pet);
}

bool IllidanStormrageShouldPositionAboveGrateTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    if (IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    return !IsFlameOfAzzinothTank(bot);
}

bool IllidanStormrageImmunityCanClearDebuffTrigger::IsActiveInEncounter()
{
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    if (!isPaladin && bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE)
        return false;

    Aura const* darkBarrage = bot->GetAura(Id(BtSpells::SPELL_DARK_BARRAGE));
    bool const isDarkBarrageDue =
        darkBarrage && GetAuraElapsedMs(darkBarrage) >= ILLIDAN_DARK_BARRAGE_REACTION_MS;
    if (!isDarkBarrageDue && !bot->HasAura(Id(BtSpells::SPELL_AGONIZING_FLAMES)))
        return false;

    if (isPaladin)
    {
        return !(PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) &&
            !IsFlameOfAzzinothTank(bot);
    }

    return true;
}

bool IllidanStormrageImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    // Below half health, the immunity may be holding off something else, so it is kept.
    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    return AI_VALUE2(Unit*, "find target", "illidan stormrage");
}

bool IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    bool hasTotem = false;
    for (uint8 slot = SUMMON_SLOT_TOTEM_FIRE; slot < MAX_TOTEM_SLOT; ++slot)
        hasTotem = hasTotem || !bot->m_SummonSlot[slot].IsEmpty();

    if (!hasTotem)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && IsIllidanTankAggroPending(*illidan, GetIllidanPhase(illidan));
}

bool IllidanStormrageShouldPositionForLandingTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Landing)
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !HasParasiticShadowfiend(bot);
}

bool IllidanStormrageMeleeShouldStopAttackingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || (bot->GetVictim() != illidan && AI_VALUE(Unit*, "current target") != illidan))
        return false;

    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    return window == IllidanDpsHold::DemonFormStart || window == IllidanDpsHold::DemonFormNoPush;
}

bool IllidanStormrageShouldStopShootingTrigger::IsActiveInEncounter()
{
    Spell* shot = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!shot && !channel)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (channel && IsIllidanPickupChannelHeld(botAI, illidan, *channel))
        return true;

    ObjectGuid const guid = illidan->GetGUID();
    bool const isOnIllidan = (shot && shot->m_targets.GetUnitTargetGUID() == guid) ||
        (channel && channel->m_targets.GetUnitTargetGUID() == guid);
    return isOnIllidan && GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None;
}

bool IllidanStormrageRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);

    if (phase == IllidanPhase::Demon)
    {
        if (IsIllidanWarlockTank(botAI))
            return false;

        // While a Demon walks to the warlock tank, ranged DPS may leave their slots to reach it.
        if (!PlayerbotAI::IsHeal(bot) && GetShadowDemonMovingToWarlockTank(botAI))
            return false;

        return true;
    }

    return phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;
}

bool IllidanStormrageRangedShouldReachShadowDemonTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    if (HasParasiticShadowfiend(bot) || IsIllidanWarlockTank(botAI))
        return false;

    Unit* demon = GetShadowDemonMovingToWarlockTank(botAI);
    return demon && !bot->IsWithinCombatRange(demon, botAI->GetRange("spell"));
}

bool IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !IsTargetingShadowDemon(botAI);
}

bool IllidanStormrageWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return IsIllidanWarlockTank(botAI);
}

bool IllidanStormrageWarlockShouldSeedShadowDemonsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (IsIllidanPhaseStartWithin(*illidan, IllidanPhase::Demon, ILLIDAN_SHADOW_DEMON_SPAWN_MS))
        return false;

    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    if (shadowDemons.size() < 2)
        return false;

    return !IsNearNewParasiticShadowfiend(botAI);
}

bool IllidanStormrageShouldAssignTargetPriorityTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None || phase == IllidanPhase::Dying)
        return false;

    if (PlayerbotAI::IsHeal(bot))
        return phase == IllidanPhase::Demon;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    if (phase != IllidanPhase::Initial && phase != IllidanPhase::Grounded &&
        phase != IllidanPhase::Demon && phase != IllidanPhase::Maiev)
    {
        return false;
    }

    if (!PlayerbotAI::IsMainTank(bot))
        return true;

    return phase == IllidanPhase::Demon &&
        !IsIllidanDemonFormEnding(*illidan, ILLIDAN_MAIN_TANK_WALK_IN_MS);
}

bool IllidanStormrageShouldManageDpsTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "illidan stormrage");
}

bool IllidanStormrageShouldClearHazardsBeforeLandingTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == IllidanPhase::Landing;
}
