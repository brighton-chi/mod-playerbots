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
#include "Timer.h"

using namespace BlackTempleHelpers;
using namespace EncounterHelpers;

// General

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BT_MAP_ID);
}

// Shared Bosses

bool BlackTempleHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool SisterOfPleasureShouldBeMarkedTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* skull = botAI->GetUnit(bot->GetGroup()->GetTargetIcon(RtiTargetValue::skullIndex));
    return !IsLinkedSisterOfPleasure(skull) && FindLinkedSisterOfPleasure(botAI);
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
    if (!najentus || !najentus->HasAura(Id(BlackTempleSpells::SPELL_TIDAL_SHIELD)))
        return false;

    return botAI->HasItemInInventory(Id(BlackTempleItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && !IsSupremusKitePhase(supremus) &&
        GetNearestPlayerInRadius(bot, SUPREMUS_RANGED_SPREAD_DISTANCE);
}

bool SupremusFixatesOnBotTrigger::IsActiveInEncounter()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && supremus->GetVictim() == bot && IsSupremusKitePhase(supremus) &&
        bot->GetDistance2d(supremus) < SUPREMUS_KITE_DISTANCE;
}

bool SupremusNearVolcanoTrigger::IsActiveInEncounter()
{
    return AI_VALUE2(Unit*, "find target", "supremus") &&
        IsInEruptingSupremusVolcano(
            GetSupremusVolcanoes(botAI), bot->GetPositionX(), bot->GetPositionY());
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
        gorefiend->FindCurrentSpellBySpellId(Id(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    return spell && spell->getState() == SPELL_STATE_PREPARING &&
        spell->m_targets.GetUnitTarget() == bot;
}

bool TeronGorefiendShouldPositionForVengefulSpiritTrigger::IsActiveInEncounter()
{
    Aura* aura = bot->GetAura(Id(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    return aura && aura->GetDuration() <= GOREFIEND_SHADOW_OF_DEATH_MOVE_MS;
}

bool TeronGorefiendTransformedIntoVengefulSpiritTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BlackTempleSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(Id(BlackTempleSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilShouldPositionForBloodboilTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) &&
        !bot->HasAura(Id(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)) &&
        AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
}

bool GurtoggBloodboilFelRageOnBotTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)) &&
        bot->GetExactDist2d(GURTOGG_TANK_POSITION) > GURTOGG_POSITION_TOLERANCE;
}

// Reliquary of Souls

bool ReliquaryOfSoulsHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    for (char const* name : { "essence of desire", "essence of anger" })
    {
        Unit* essence = AI_VALUE2(Unit*, "find target", name);
        if (essence && essence->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
            return true;
    }

    return false;
}

bool ReliquaryOfSoulsShouldPositionForSufferingTrigger::IsActiveInEncounter()
{
    return IsOutOfSufferingPosition(bot, AI_VALUE2(Unit*, "find target", "essence of suffering"));
}

bool ReliquaryOfSoulsHealersShouldAttackSufferingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Discipline priests keep shielding.
    if (bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("disc", BOT_STATE_COMBAT))
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
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageHunterShouldMisdirectTrigger::IsActiveInEncounter()
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
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->GetHealth() > 1;
}

// Destroying hazards behind phases is not gated behind CheatMask
// The strategy simply cannot work without doing this
bool IllidanStormrageShouldClearHazardsBetweenPhasesTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
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
