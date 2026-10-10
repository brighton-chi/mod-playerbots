/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTMultipliers.h"
#include "BTActions.h"
#include "BTHelpers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "WipeAction.h"
#include <array>

using namespace BtHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

}

// Shared

// Held for each boss's first 5%, and also through Supremus's kite phase, between Reliquary
// essences, and on Illidan from 95% to 62% (Bloodlust all the way until 62%).
float BlackTempleDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    if (Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage"))
    {
        if (illidan->GetHealthPct() <= 62.0f)
            return 1.0f;

        if (bot->getClass() == CLASS_SHAMAN &&
            (dynamic_cast<CastHeroismAction*>(action) ||
             dynamic_cast<CastBloodlustAction*>(action)))
        {
            return 0.0f;
        }

        return illidan->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 1.0f : 0.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "reliquary of the lost"))
    {
        // An essence walking home at 1 health stays on the threat lists.
        for (char const* name :
             { "essence of suffering", "essence of desire", "essence of anger" })
        {
            Unit* essence = AI_VALUE2(Unit*, "find target", name);
            if (essence && !essence->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                return essence->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }

        return 0.0f;
    }

    if (Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus"))
    {
        return IsSupremusKitePhase(supremus) ||
            supremus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    static constexpr std::array blackTempleBosses = {
        "high warlord naj'entus", "teron gorefiend", "gurtogg bloodboil", "mother shahraz",
        "gathios the shatterer",
    };

    for (char const* name : blackTempleBosses)
    {
        if (Unit* boss = AI_VALUE2(Unit*, "find target", name))
            return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    return 1.0f;
}

float BlackTempleDisableKillingSpreeMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ||
        AI_VALUE2(Unit*, "find target", "supremus") ? 0.0f : 1.0f;
}

// Trash

float ShadowmoonReaverHoldChargeBuildingSpellsMultiplier::GetValue(Action* action)
{
    auto* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    auto const& reavers = context->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    if (reavers.empty())
        return 1.0f;

    // Summons whose own casts build charges, held for the whole pull.
    if (dynamic_cast<CastMirrorImageAction*>(action) ||
        dynamic_cast<CastSummonGargoyleAction*>(action) ||
        dynamic_cast<CastSearingTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action))
    {
        return 0.0f;
    }

    SpellInfo const* spellInfo =
        sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", spellAction->getSpell()));
    ChaoticChargeReach const reach = GetChaoticChargeReach(spellInfo);
    if (reach == ChaoticChargeReach::None)
        return 1.0f;

    if (reach == ChaoticChargeReach::Target)
        return IsShadowmoonReaverUnsafeForMagic(action->GetTarget()) ? 0.0f : 1.0f;

    return IsAnyShadowmoonReaverUnsafeForMagic(botAI) ? 0.0f : 1.0f;
}

// High Warlord Naj'entus

float HighWarlordNajentusDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "high warlord naj'entus") ? 0.0f : 1.0f;
}

// Supremus

float SupremusFocusOnAvoidanceInKitePhaseMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !(bot->getClass() == CLASS_MAGE && dynamic_cast<CastEvocationAction*>(action)))
    {
        return 1.0f;
    }

    if (dynamic_cast<SupremusKiteBossAction*>(action))
        return 1.0f;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || supremus->GetVictim() != bot)
        return 1.0f;

    return IsSupremusKitePhase(supremus) ? 0.0f : 1.0f;
}

float SupremusDoNotReachThroughFireMultiplier::GetValueInEncounter(Action* action)
{
    bool const isHealerReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isDpsReach =
        dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachSpellAction*>(action);
    if (PlayerbotAI::IsHeal(bot) ? !isHealerReach : !isDpsReach)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return 1.0f;

    Unit* target;
    float range;
    return GetSupremusReachBlockedByFire(botAI, target, range) ? 0.0f : 1.0f;
}

float SupremusDisableAvoidAoeInKitePhaseMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    return IsSupremusKitePhase(AI_VALUE2(Unit*, "find target", "supremus")) ? 0.0f : 1.0f;
}

// Shade of Akama

float ShadeOfAkamaDontDropOutOfSightTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    return target->GetEntry() == Id(BtNpcs::NPC_ASHTONGUE_CHANNELER) ||
        target->GetEntry() == Id(BtNpcs::NPC_ASHTONGUE_SORCERER) ? 0.0f : 1.0f;
}

// Teron Gorefiend

float TeronGorefiendControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isRangedReach =
        PlayerbotAI::IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action);

    if (!isRangedReach && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "teron gorefiend") ? 0.0f : 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValueInEncounter(Action* action)
{
    Aura* aura = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    if (!aura || aura->GetDuration() > GOREFIEND_SHADOW_OF_DEATH_MOVE_MS)
        return 1.0f;

    if (dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

float TeronGorefiendSpiritsAttackShadowyConstructsMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE)))
        return 1.0f;

    if (dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

float TeronGorefiendDisableAttackingConstructsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsRangedDps(bot))
        return 1.0f;

    auto* castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction || castSpellAction->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "teron gorefiend") ? 0.0f : 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isFormationMove = dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action);
    bool const isFollowOrFlee =
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action);
    // The Fel Rage target only walks him to the tank spot.
    bool const isFelRageTargetMove = bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)) &&
        dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<BlackTemplePositionBossAction*>(action);

    if (!isFormationMove && !isFollowOrFlee && !isFelRageTargetMove)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "gurtogg bloodboil") ? 0.0f : 1.0f;
}

float GurtoggBloodboilNoThreatRedirectsDuringFelRageMultiplier::GetValueInEncounter(
    Action* action)
{
    if (!IsTauntAction(bot, action) &&
        !dynamic_cast<CastMisdirectionOnMainTankAction*>(action) &&
        !dynamic_cast<BlackTempleMisdirectToMainTankAction*>(action))
    {
        return 1.0f;
    }

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)) ? 0.0f : 1.0f;
}

float GurtoggBloodboilFelRageTargetHoldsAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BtSpells::SPELL_PLAYER_FEL_RAGE)))
        return 1.0f;

    // Feign Death, Fade, Invisibility, Vanish, Cower and Soulshatter are left out: their class
    // strategies use them on "medium threat", which needs two attackers on the bot, and he is
    // alone.
    bool const dropsThreat =
        dynamic_cast<CastIceBlockAction*>(action) ||
        dynamic_cast<CastDivineShieldAction*>(action) ||
        dynamic_cast<CastFeintAction*>(action);
        // dynamic_cast<CastFeignDeathAction*>(action) ||
        // dynamic_cast<CastFadeAction*>(action) ||
        // dynamic_cast<CastInvisibilityAction*>(action) ||
        // dynamic_cast<CastVanishAction*>(action) ||
        // dynamic_cast<CastCowerAction*>(action) ||
        // dynamic_cast<CastSoulshatterAction*>(action);

    return dropsThreat ? 0.0f : 1.0f;
}

float GurtoggBloodboilHoldThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<BlackTempleMisdirectToMainTankAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        bot->HasAura(Id(BtSpells::SPELL_INSIGNIFICANCE)))
    {
        return 1.0f;
    }

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return 1.0f;

    if (action->GetTarget() != gurtogg && AI_VALUE(Unit*, "current target") != gurtogg)
        return 1.0f;

    float const secondTankThreat = AI_VALUE(float, "gurtogg bloodboil second tank threat");
    if (secondTankThreat <= 0.0f)
        return 1.0f;

    return gurtogg->GetThreatMgr().GetThreat(bot) >= secondTankThreat * GURTOGG_THREAT_HOLD_RATIO ?
        0.0f : 1.0f;
}

// Reliquary of Souls

float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastTreeFormAction*>(action) &&
        !dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ? 0.0f : 1.0f;
}

// Tank face, set behind and disperse move tanks and melee around Suffering, which can carry
// them nearer to her than the tanks.
float ReliquaryOfSoulsControlMeleeMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) || PlayerbotAI::IsRanged(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ? 0.0f : 1.0f;
}

float ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastPurgeAction*>(action) &&
        !dynamic_cast<CastTranquilizingShotAction*>(action))
    {
        return 1.0f;
    }

    Unit* target = action->GetTarget();
    if (!target || target->GetEntry() != Id(BtNpcs::NPC_ESSENCE_OF_DESIRE))
        return 1.0f;

    Aura* runeShield = target->GetAura(Id(BtSpells::SPELL_RUNE_SHIELD));
    if (!runeShield)
        return 1.0f;

    uint32 const elapsed =
        static_cast<uint32>(runeShield->GetMaxDuration() - runeShield->GetDuration());
    return elapsed < RUNE_SHIELD_MAGE_PRIORITY_MS ? 0.0f : 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "mother shahraz") ? 0.0f : 1.0f;
}

float MotherShahrazFatalAttractionRunAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    return dynamic_cast<MotherShahrazBreakFatalAttractionAction*>(action) ? 1.0f : 0.0f;
}

// Illidari Council

float IllidariCouncilDisableMeleeTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<TankFaceAction*>(action) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer") ? 0.0f : 1.0f;
}

float IllidariCouncilDisableMageTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action) && !dynamic_cast<CastIceBlockAction*>(action))
        return 1.0f;

    if (!IsZerevorMageTank(botAI))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor") ? 0.0f : 1.0f;
}

float IllidariCouncilControlNonTankMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isMovementAction = dynamic_cast<MovementAction*>(action);

    if (!isMovementAction && !IsRepositionAction(bot, action))
        return 1.0f;

    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
    {
        if (dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action))
            return 1.0f;

        return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor") ? 0.0f : 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (isMovementAction &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer") ? 0.0f : 1.0f;
}

float IllidariCouncilDisableAutoTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer") ? 0.0f : 1.0f;
}

float IllidariCouncilControlHunterActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action) &&
        !dynamic_cast<CastTranquilizingShotAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer") ? 0.0f : 1.0f;
}

float IllidariCouncilWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return 1.0f;

    if (dynamic_cast<IllidariCouncilMisdirectToTanksAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        (!dynamic_cast<CastSpellAction*>(action) || dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 1.0f;
    }

    uint32 const now = getMSTime();
    constexpr uint32 dpsWaitMs = 5 * IN_MILLISECONDS;

    auto it = councilDpsWaitTimer.find(gathios->GetMap()->GetInstanceId());
    if (it == councilDpsWaitTimer.end() || getMSTimeDiff(it->second, now) >= dpsWaitMs)
        return 1.0f;

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ||
        IsZerevorMageTank(botAI))
    {
        return 1.0f;
    }

    return 0.0f;
}

// Illidan Stormrage <The Betrayer>

float IllidanStormrageControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    // if (dynamic_cast<TankFaceAction*>(action))
    //    return 0.0f;

    if (GetIllidanPhase(illidan) != 2)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<CastMeleeSpellAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
        {
            return 0.0f;
        }
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<TankFaceAction*>(action) ||
            dynamic_cast<CastHealingSpellAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float IllidanStormrageDisableDefaultTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    int const phase = GetIllidanPhase(illidan);

    if (phase == 4 && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    if (PlayerbotAI::IsRangedDps(bot))
    {
        if (phase != 2)
            context->GetValue<bool>("neglect threat")->Set(true);

        if (dynamic_cast<DpsAssistAction*>(action))
            return 0.0f;
    }

    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    constexpr float searchRadius = 40.0f;
    Unit* shadowDemon = bot->FindNearestCreature(Id(BtNpcs::NPC_SHADOW_DEMON), searchRadius);
    Unit* shadowfiend = bot->FindNearestCreature(
        Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND), searchRadius);

    if ((shadowDemon && bot->GetTarget() == shadowDemon->GetGUID()) ||
        (shadowfiend && bot->GetTarget() == shadowfiend->GetGUID()))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageControlNonTankMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        IsRepositionAction(bot, action))
    {
        return 0.0f;
    }

    int const phase = GetIllidanPhase(illidan);

    if (phase == 2 &&
        (dynamic_cast<SetBehindTargetAction*>(action) ||
         dynamic_cast<CastKillingSpreeAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<AvoidAoeAction*>(action)))
    {
        return 0.0f;
    }

    if (phase == 4 && PlayerbotAI::IsHeal(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidanStormrageUseEarthbindTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastTremorTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && !IsIllidanDeathScene(illidan) && GetIllidanPhase(illidan) != 2 ? 0.0f : 1.0f;
}

float IllidanStormrageWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (dynamic_cast<IllidanStormrageMisdirectToTanksAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        (!dynamic_cast<CastSpellAction*>(action) || dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 1.0f;
    }

    uint32 const now = getMSTime();
    uint32 const instanceId = illidan->GetMap()->GetInstanceId();

    int const phase = GetIllidanPhase(illidan);

    if ((phase == 1 || phase == 3 || phase == 5) &&
        (!PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsMainTank(bot)))
    {
        constexpr uint32 humanoidPhaseDpsWaitMs = 3 * IN_MILLISECONDS;
        auto it = illidanBossDpsWaitTimer.find(instanceId);
        if (it == illidanBossDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < humanoidPhaseDpsWaitMs)
        {
            return 0.0f;
        }
    }

    if (phase == 4 && GetIllidanWarlockTank(botAI) != bot)
    {
        constexpr uint32 demonPhaseDpsWaitMs = 8 * IN_MILLISECONDS;
        auto it = illidanBossDpsWaitTimer.find(instanceId);
        if (it == illidanBossDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < demonPhaseDpsWaitMs)
        {
            return 0.0f;
        }
    }

    if (AI_VALUE2(Unit*, "find target", "flame of azzinoth") &&
        !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        !PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        constexpr uint32 flamePhaseDpsWaitMs = 6 * IN_MILLISECONDS;
        auto it = illidanFlameDpsWaitTimer.find(instanceId);
        if (it == illidanFlameDpsWaitTimer.end() ||
            getMSTimeDiff(it->second, now) < flamePhaseDpsWaitMs)
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
