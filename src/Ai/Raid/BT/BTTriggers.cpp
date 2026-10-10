/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTTriggers.h"
#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
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
    {
        Unit* skull = botAI->GetUnit(bot->GetGroup()->GetTargetIcon(RtiTargetValue::skullIndex));
        return !IsLinkedSisterOfPleasure(skull) && FindLinkedSisterOfPleasure(botAI);
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == Id(BtNpcs::NPC_SISTER_OF_PAIN) &&
        IsLinkedSisterOfPleasure(AI_VALUE2(Unit*, "find target", "sister of pleasure"));
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

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    return bot->GetExactDist2d(najentus) < NAJENTUS_RANGED_DISTANCE_FROM_BOSS ||
        GetNearestPlayerInRadius(bot, NAJENTUS_RANGED_SPREAD_DISTANCE);
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
    if (!IsNajentusSpineThrower(bot))
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
    if (!PlayerbotAI::IsMelee(bot) || bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Unit* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    return felRageTarget &&
        bot->GetExactDist2d(GetGurtoggFelRageMeleePosition(bot, gurtogg, felRageTarget)) >
            GURTOGG_FEL_RAGE_MELEE_TOLERANCE;
}

// Melee go behind him instead.
bool GurtoggBloodboilShouldAvoidFelRageTargetTrigger::IsActiveInEncounter()
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
    return (felRage && felRage->GetMaxDuration() - felRage->GetDuration() <
            GURTOGG_FEL_RAGE_EARLY_AVOID_MS) ||
        gurtogg->HasInArc(static_cast<float>(M_PI), bot);
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

    constexpr float shahrazMeleeStartHealthPct = 90.0f;
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < shahrazMeleeStartHealthPct)
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

// Ranged DPS interrupt only while Blessing of Protection stops melee interrupts. Paladins, priests
// and hunters have nothing she isn't immune to.
bool IllidariCouncilMalandeCastsCircleOfHealingTrigger::IsActiveInEncounter()
{
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_ROGUE:
        case CLASS_SHAMAN:
        case CLASS_WARLOCK:
        case CLASS_WARRIOR:
            break;
        default:
            return false;
    }

    bool const isRanged = PlayerbotAI::IsRanged(bot);
    if (isRanged && (!PlayerbotAI::IsRangedDps(bot) || IsZerevorMageTank(botAI)))
        return false;

    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    if (isRanged && !malande->HasAura(Id(BtSpells::SPELL_BLESSING_OF_PROTECTION)))
        return false;

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

bool IllidariCouncilMageTankNeedsDedicatedHealerTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return false;

    return !HasDangerousCouncilAura(bot);
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

bool IllidanStormrageHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && !IsIllidanDeathScene(illidan);
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
    if (!illidan || IsIllidanDeathScene(illidan) || illidan->GetVictim() == bot)
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
    if (!illidan || IsIllidanDeathScene(illidan) || GetIllidanPhase(illidan) == 2)
        return false;

    ObjectGuid const guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_EARTH];
    if (guid.IsEmpty())
        return true;

    Creature* totem = bot->GetMap()->GetCreature(guid);
    return !totem || totem->GetDistance(bot) > 20.0f ||
        totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) != Id(BtSpells::SPELL_EARTHBIND_TOTEM);
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
    if (!illidan || IsIllidanDeathScene(illidan))
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

    return bot->HasAura(Id(BtSpells::SPELL_DARK_BARRAGE));
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
    if (!illidan || illidan->HasAura(Id(BtSpells::SPELL_CAGED)))
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
    if (!illidan || IsIllidanDeathScene(illidan))
        return false;

    if (PlayerbotAI::IsTank(bot) && GetIllidanPhase(illidan) != 4)
        return false;

    return true;
}

bool IllidanStormrageMaievPlacedShadowTrapTrigger::IsActiveInEncounter()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan) || GetIllidanPhase(illidan) != 5)
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
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && !IsIllidanDeathScene(illidan);
}

// Destroying hazards behind phases is not gated behind CheatMask
// The strategy simply cannot work without doing this
bool IllidanStormrageShouldClearHazardsBetweenPhasesTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
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
