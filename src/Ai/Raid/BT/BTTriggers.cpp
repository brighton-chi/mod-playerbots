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

using namespace BtHelpers;
using namespace EncounterHelpers;

// Shared

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BT_MAP_ID);
}

bool BlackTempleHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

// The mechanic tracker marks a linked Sister of Pleasure. Other non-tanks on her Sister of Pain
// drop Pain while the two are linked.
bool SisterOfPleasureShouldBeFocusedTrigger::IsActive()
{
    if (IsMechanicTrackerBot(bot, BT_MAP_ID))
        return !GetSkullSisterOfPain(bot) && FindLinkedSisterOfPleasure(botAI);

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == Id(BtNpcs::NPC_SISTER_OF_PAIN) &&
        GetSkullSisterOfPain(bot) == target;
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

    return botAI->HasItemInInventory(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || IsSupremusKitePhase(supremus))
        return false;

    // FleePosition's step goes any way and doesn't see fire, so hold while one could carry the bot
    // into a volcano's safe zone. Outside the kite phase the hazards are volcanoes alone.
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
    return CanSupremusCatchStandingBot(bot, supremus) ||
        IsInSupremusHazard(
            GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
            SupremusHazardZone::Safe);
}

bool SupremusNearFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusReachBlockedByFireTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    Unit* target;
    float range;
    return GetSupremusReachBlockedByFire(botAI, target, range);
}

// Shade of Akama

bool ShadeOfAkamaShouldPrioritizeChannelersTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && PlayerbotAI::IsMelee(bot) && GetShadeOfAkamaKillTarget(botAI);
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

// Feign Death and Vanish cancel his cast. Ice Block and Divine Shield don't help: Shadow of Death
// pierces invulnerability in AC.
bool TeronGorefiendCastsShadowOfDeathTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    // Once the bolt is launched, nothing cancels it.
    Spell* spell =
        gorefiend->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return spell && spell->getState() == SPELL_STATE_PREPARING &&
        spell->m_targets.GetUnitTarget() == bot;
}

bool TeronGorefiendShouldPositionForVengefulSpiritTrigger::IsActiveInEncounter()
{
    Aura* aura = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return aura && aura->GetDuration() <= GOREFIEND_SHADOW_OF_DEATH_MOVE_MS;
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

    return !bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE));
}

bool GurtoggBloodboilMeleeShouldStayBehindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    if (bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    return bot->GetExactDist2d(GetGurtoggFelRageMeleePosition(bot, gurtogg, felRageTarget)) >
        GURTOGG_FEL_RAGE_MELEE_TOLERANCE;
}

// Melee go behind him instead.
bool GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMelee(bot) || bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
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
    if (felRage &&
        felRage->GetMaxDuration() - felRage->GetDuration() < GURTOGG_FEL_RAGE_EARLY_AVOID_MS)
    {
        return true;
    }

    return gurtogg->HasInArc(static_cast<float>(M_PI), bot);
}

bool GurtoggBloodboilFelRageOnBotTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)) &&
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
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != Id(BtSpells::SPELL_DEADEN))
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

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazTanksArePositioningBossTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    constexpr float meleeEngageHealthPct = 90.0f;
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < meleeEngageHealthPct)
        return false;

    Unit* victim = shahraz->GetVictim();
    return !victim ||
        victim->GetExactDist2d(SHAHRAZ_TANK_POSITION) > SHAHRAZ_POSITIONED_DISTANCE;
}

bool MotherShahrazRangedShouldPositionUnderPillarTrigger::IsActiveInEncounter()
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

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilGathiosCastsJudgementOfCommandTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || !gathios->HasUnitState(UNIT_STATE_CASTING) ||
        !gathios->HasAura(Id(BtSpells::SPELL_SEAL_OF_COMMAND)))
    {
        return false;
    }

    Spell* spell = gathios->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != Id(BtSpells::SPELL_JUDGEMENT))
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID() && PlayerbotAI::IsMainTank(bot);
}

// Ranged DPS interrupt only while her melee can't: under Blessing of Protection, or while she
// stands in a Zerevor patch or Gathios's Consecration. Paladins, priests and hunters have nothing
// she isn't immune to.
bool IllidariCouncilMalandeCastsCircleOfHealingTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_HUNTER ||
        bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_PRIEST)
    {
        return false;
    }

    if (!PlayerbotAI::IsDps(bot) || IsZerevorMageTank(botAI))
        return false;

    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    if (PlayerbotAI::IsRanged(bot) &&
        !malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION)) &&
        !IsMalandeInCouncilPatch(
            malande, AI_VALUE2(Unit*, "find target", "gathios the shatterer"),
            AI_VALUE2(Unit*, "find target", "high nethermancer zerevor")))
    {
        return false;
    }

    return CanInterruptCircleOfHealing(malande);
}

bool IllidariCouncilMalandeShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) &&
        AI_VALUE2(Unit*, "find target", "lady malande");
}

bool IllidariCouncilVerasShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return false;

    Unit* veras = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    return veras && !IsVerasVanished(veras);
}

bool IllidariCouncilZerevorShouldBeTankedByMageTrigger::IsActiveInEncounter()
{
    return IsZerevorMageTank(botAI) && AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilMageTankHealerShouldRepositionTrigger::IsActiveInEncounter()
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
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

    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
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

// Every bot but the main tank, the Malande tank and the mage tank, and the Veras tank only while
// Veras is vanished.
bool IllidariCouncilShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        IsZerevorMageTank(botAI))
    {
        return false;
    }

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return true;

    Unit* veras = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    return !veras || IsVerasVanished(veras);
}

bool IllidariCouncilShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageShouldPositionForPullTrigger::IsActive()
{
    return AI_VALUE(bool, "illidan stormrage speech in progress");
}

// Hunters misdirect onto the Main Tank at each Phase 1, 3 and 5 start, the Flame tanks in Phase 2
// and the warlock tank in Demon Form.
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
        return IsIllidanPhaseStartWithin(*illidan, ILLIDAN_PHASE_START_MISDIRECT_MS);
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

// An infected bot runs to the infection position, and so does a trap hunter on duty, to lay its
// Frost Trap. The Main Tank never runs, and the warlock tank not in Demon Form.
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

    if (phase == IllidanPhase::Flying)
        return IsIllidanInfectionPositionKeptInFlight(botAI);

    if (phase == IllidanPhase::Demon && IsIllidanWarlockTank(botAI))
        return false;

    return !PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsMainTank(bot);
}

// The mechanic tracker records each new infection while there is a shadowfiend birthing position,
// keeps looking for a trap hunter while none is ready, and drops an entry once its debuff is gone
// or the shadowfiend phase changes.
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
    IllidanPhase const infectionPhase = GetIllidanInfectionPositionPhase(*illidan, phase);
    bool const hasInfectionPosition = phase != IllidanPhase::None && phase != IllidanPhase::Flying;
    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    auto const* infections =
        state && state->illidanInfections ? &*state->illidanInfections : nullptr;
    if (infections)
    {
        for (auto const& [player, infection] : *infections)
        {
            if (std::find(infected.begin(), infected.end(), player) == infected.end())
                return true;

            if (hasInfectionPosition &&
                (infection.phase != infectionPhase || !infection.isTrapAssigned))
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

bool IllidanStormragePetsDieToFireTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    bool const shouldBePassive = phase == IllidanPhase::Flying || phase == IllidanPhase::Demon;
    return shouldBePassive != pet->HasReactState(REACT_PASSIVE);
}

// Hunter and warlock pets take the Shadowfiends, which never attack pets.
// Hunter and warlock pets in Phases 1, 3 and 5. In Demon Form every pet, passive then, once no
// Shadow Demon is left for it.
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
    else if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev)
    {
        if ((bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK) ||
            pet->HasReactState(REACT_PASSIVE))
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

// Every pet, passive in Demon Form, is sent onto a Shadow Demon and kept on it until it dies.
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

bool IllidanStormrageGrateIsSafeFromFlamesTrigger::IsActiveInEncounter()
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

// Ice Block, Divine Shield and Cloak of Shadows each clear both debuffs on use.
bool IllidanStormrageImmunityCanClearDebuffTrigger::IsActiveInEncounter()
{
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    if (!isPaladin && bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE)
        return false;

    if (!bot->HasAura(Id(BtSpells::SPELL_DARK_BARRAGE)) &&
        !bot->HasAura(Id(BtSpells::SPELL_AGONIZING_FLAMES)))
    {
        return false;
    }

    if (isPaladin)
        return !PlayerbotAI::IsMainTank(bot);

    return true;
}

// Below half health, the immunity may be holding off something else, so it is kept.
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

    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    return AI_VALUE2(Unit*, "find target", "illidan stormrage");
}

// An infected bot runs to the infection position instead.
bool IllidanStormrageShouldPositionForLandingTrigger::IsActiveInEncounter()
{
    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (GetIllidanPhase(illidan) != IllidanPhase::Landing)
        return false;

    return !HasParasiticShadowfiend(bot);
}

// At the transform, and at 33% or below in Demon Form, melee and tanks still on him end their
// auto-attack: in his melee range they would only need 110% of the warlock tank's threat.
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

// Auto Shot and wands keep shooting him through a hold, since a multiplier can't end a running
// auto-repeat.
bool IllidanStormrageShouldStopShootingTrigger::IsActiveInEncounter()
{
    Spell* shot = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    if (!shot)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && shot->m_targets.GetUnitTargetGUID() == illidan->GetGUID() &&
        GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None;
}

bool IllidanStormrageRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);

    if (phase == IllidanPhase::Demon && IsIllidanWarlockTank(botAI))
        return false;

    // While a Demon walks to the warlock tank, ranged DPS may leave their slots to reach it.
    if (phase == IllidanPhase::Demon && !PlayerbotAI::IsHeal(bot) &&
        GetShadowDemonWalkingToWarlockTank(botAI))
    {
        return false;
    }

    return phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Demon || phase == IllidanPhase::Maiev;
}

// Ranged DPS walk into spell reach of a Shadow Demon walking to the warlock tank, whose death
// wipes the raid. Healers stay to heal.
bool IllidanStormrageRangedShouldReachShadowDemonTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot) || HasParasiticShadowfiend(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || IsIllidanWarlockTank(botAI))
        return false;

    Unit* demon = GetShadowDemonWalkingToWarlockTank(botAI);
    return demon && !bot->IsWithinCombatRange(demon, botAI->GetRange("spell"));
}

// Melee and tanks take their Demon Form slots; the Main Tank has its own positions.
bool IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    if (HasParasiticShadowfiend(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return !target || target->GetEntry() != Id(BtNpcs::NPC_SHADOW_DEMON);
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

// Every warlock, the warlock tank included, casts Seed of Corruption on the Shadow Demons while
// two or more are alive. They all walk toward the raid, so a Seed going off on one usually
// catches others.
bool IllidanStormrageWarlockShouldSeedShadowDemonsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    if (IsIllidanPhaseStartWithin(*illidan, ILLIDAN_SHADOW_DEMON_SPAWN_MS))
        return false;

    return context->GetValue<std::vector<ShadowDemon>>("shadow demons")->RefGet().size() >= 2;
}

// Tanks other than the Main Tank join the melee in Phases 1, 3, 4 and 5; while he flies the
// assist tanks have the Flames. The Main Tank joins them on Shadow Demons in Demon Form until its
// walk-in. Healers take Shadow Demons as targets in Demon Form.
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

bool IllidanStormrageMaievPlacedCageTrapTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Maiev)
        return false;

    GameObject* trap = FindNearestCageTrap(botAI);
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

bool IllidanStormrageShouldManageDpsTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "illidan stormrage");
}

// Destroying hazards after phase 2 is not gated behind CheatMask. The strategy simply cannot work
// without doing this.
bool IllidanStormrageShouldClearHazardsBeforeLandingTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == IllidanPhase::Landing;
}
