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
#include "DruidActions.h"
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
#include "WarriorActions.h"
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

bool IsDpsHoldCandidate(Player* bot, Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return false;

    if (!PlayerbotAI::IsTank(bot) && dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    // AttackAction for healers only keeps them in their combat engine; their damage, wands
    // included, comes from CastSpellAction.
    return !dynamic_cast<AttackAction*>(action) || !PlayerbotAI::IsHeal(bot);
}

// Buffs, cures, protection and resurrection go through a dps hold. Totems don't, since some are
// offensive.
float GetDpsHoldValue(Player* bot, Action* action)
{
    if (bot->getClass() == CLASS_SHAMAN && dynamic_cast<CastTotemAction*>(action))
        return 0.0f;

    bool const castOnRaidSpell = dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action);

    return castOnRaidSpell ? 1.0f : 0.0f;
}

// At Illidan's landing stack a bot still picks targets and still runs off with a Shadowfiend
// debuff.
bool IsHeldAtIllidanLandingStack(Action* action)
{
    return dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<IllidanStormragePositionForLandingAction*>(action) &&
        !dynamic_cast<IllidanStormrageIsolateBotWithParasiteAction*>(action);
}

// A bot waiting out a Shadowfiend debuff at the run spot chases nothing, since its run there
// stops once it arrives. In Demon Form everyone keeps to their place; melee leave theirs only to
// reach a Shadow Demon.
bool IsHeldIllidanChase(PlayerbotAI* botAI, Action* action, IllidanPhase phase)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return false;
    }

    Player* bot = botAI->GetBot();
    if (HasParasiticShadowfiend(bot))
        return true;

    if (phase != IllidanPhase::Demon)
        return false;

    if (!PlayerbotAI::IsMelee(bot))
        return true;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return !target || target->GetEntry() != Id(BtNpcs::NPC_SHADOW_DEMON);
}

// Summons that fight on their own, unlike pets, which the strategy can set passive.
bool IsGuardianSummon(Player* bot, Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return dynamic_cast<CastSummonGargoyleAction*>(action) ||
                dynamic_cast<CastArmyOfTheDeadAction*>(action);
        case CLASS_DRUID:
            return dynamic_cast<CastForceOfNatureAction*>(action);
        case CLASS_MAGE:
            return dynamic_cast<CastMirrorImageAction*>(action);
        case CLASS_PRIEST:
            return dynamic_cast<CastShadowfiendAction*>(action);
        case CLASS_SHAMAN:
            return dynamic_cast<CastFeralSpiritAction*>(action) ||
                dynamic_cast<CastFireElementalTotemAction*>(action) ||
                dynamic_cast<CastFireElementalTotemMeleeAction*>(action);
        default:
            return false;
    }
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

float IllidariCouncilControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    bool const inCombat = botAI->GetState() == BOT_STATE_COMBAT;
    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return inCombat && AI_VALUE2(Unit*, "find target", "gathios the shatterer") ? 0.0f : 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return inCombat ? GetTankValue(action) : 1.0f;

    return GetNonTankValue(action);
}

float IllidariCouncilControlMovementAndTargetingMultiplier::GetTankValue(Action* action)
{
    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action))
    {
        Player* mageTank = GetZerevorMageTank(botAI);
        if (mageTank && action->GetTarget() == mageTank)
            return 0.0f;
    }

    bool const isStockMove =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action);
    bool const isAoeThreat = IsAoeThreatAction(bot, action);
    if (!isStockMove && !isAoeThreat && !IsTauntAction(bot, action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return 1.0f;

    if (isAoeThreat)
        return IsAnotherCouncilMemberWithin(botAI, COUNCIL_AOE_THREAT_CLEARANCE) ? 0.0f : 1.0f;

    if (dynamic_cast<TankFaceAction*>(action) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return 1.0f;

    return 0.0f;
}

// While Zerevor is on the mage tank, its healer moves only by its own action. No one else reaches
// to heal the mage tank.
float IllidariCouncilControlMovementAndTargetingMultiplier::GetNonTankValue(Action* action)
{
    bool const isHealerMove = PlayerbotAI::IsHeal(bot) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action);
    bool const isHealReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isBlockedMove =
        (dynamic_cast<CombatFormationMoveAction*>(action) &&
         !dynamic_cast<SetBehindTargetAction*>(action)) ||
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action);
    if (!isHealerMove && !isHealReach && !isBlockedMove)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return 1.0f;

    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    bool const isMageTankHealer = PlayerbotAI::IsAssistHealOfIndex(bot, 0, true);
    if (isHealerMove && isMageTankHealer && IsZerevorOnMageTank(botAI, zerevor))
        return 0.0f;

    if (isHealReach && !isMageTankHealer && zerevor)
    {
        Player* mageTank = GetZerevorMageTank(botAI);
        if (mageTank && AI_VALUE(Unit*, "party member to heal") == mageTank)
            return 0.0f;
    }

    return isBlockedMove ? 0.0f : 1.0f;
}

float IllidariCouncilDisableMageTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action) && !dynamic_cast<CastIceBlockAction*>(action) &&
        action->getThreatType() != Action::ActionThreatType::Aoe)
    {
        return 1.0f;
    }

    if (!IsZerevorMageTank(botAI))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor") ? 0.0f : 1.0f;
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
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    if (dynamic_cast<IllidariCouncilMisdirectToTanksAction*>(action))
        return 1.0f;

    auto it = councilDpsWaitTimer.find(bot->GetInstanceId());
    if (it == councilDpsWaitTimer.end() ||
        getMSTimeDiff(it->second, getMSTime()) >= COUNCIL_DPS_WAIT_MS)
    {
        return 1.0f;
    }

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ||
        IsZerevorMageTank(botAI))
    {
        return 1.0f;
    }

    return GetDpsHoldValue(bot, action);
}

// Illidan Stormrage <The Betrayer>

// While he talks, bots move only to their pull places.
float IllidanStormrageHoldPullPositionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT || bot->GetMapId() != BT_MAP_ID)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<IllidanStormragePositionForPullAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE(bool, "illidan stormrage speech in progress") ? 0.0f : 1.0f;
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    bool const inCombat = botAI->GetState() == BOT_STATE_COMBAT;
    if (inCombat && action->getThreatType() != Action::ActionThreatType::None)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        uint32 const entry = target ? target->GetEntry() : 0;
        if (entry == Id(BtNpcs::NPC_SHADOW_DEMON) ||
            entry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
        {
            // The opt-in threat strategy reads an add no tank has touched as 100% threat.
            context->GetValue<bool>("neglect threat")->Set(true);
            // A dot on an attacker would go on something other than the add.
            if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
        }
    }

    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);
    if (isTankAssist || dynamic_cast<DpsAssistAction*>(action))
        return inCombat ? GetAssistValue(isTankAssist) : 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return GetTankValue(action);

    return GetNonTankValue(action);
}

// Tanks take their targets from their own actions or dps prioritize adds, except the assist tanks
// while he lands. Ranged DPS, melee in Phases 1, 3 and 5, and everyone in Demon Form take theirs
// from dps prioritize adds.
float IllidanStormrageControlMovementAndTargetingMultiplier::GetAssistValue(bool isTankAssist)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (isTankAssist)
        return phase == IllidanPhase::Landing && !PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;

    if (phase == IllidanPhase::Demon || PlayerbotAI::IsRangedDps(bot))
        return 0.0f;

    bool const isGrounded = phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;
    return isGrounded && PlayerbotAI::IsMelee(bot) ? 0.0f : 1.0f;
}

// On the ground and while he lands, the Main Tank moves only by its ring action, and its
// gap-closers are held, since he always comes to it; the other tanks wait at the landing stack
// with everyone else, and keep to their Demon Form slots as melee. While he flies, the Main Tank
// holds the grate and the assist tanks the Flames of Azzinoth, each moving only by its own action.
float IllidanStormrageControlMovementAndTargetingMultiplier::GetTankValue(Action* action)
{
    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action))
    {
        Player* warlockTank = GetIllidanWarlockTank(botAI);
        if (warlockTank && action->GetTarget() == warlockTank)
            return 0.0f;
    }

    bool const isMove = dynamic_cast<MovementAction*>(action);
    bool const isGapCloser = dynamic_cast<CastReachTargetSpellAction*>(action);
    bool const isMeleeSpell = isGapCloser || dynamic_cast<CastMeleeSpellAction*>(action);
    bool const isHeal = dynamic_cast<CastHealingSpellAction*>(action);
    if (!isMove && !isMeleeSpell && !isHeal)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Landing ||
        phase == IllidanPhase::Grounded || phase == IllidanPhase::Demon ||
        phase == IllidanPhase::Maiev)
    {
        if (!PlayerbotAI::IsMainTank(bot))
        {
            if (IsHeldIllidanChase(botAI, action, phase))
                return 0.0f;

            if (phase == IllidanPhase::Demon || !IsIllidanLandingWait(*illidan, phase))
                return 1.0f;

            return isGapCloser || IsHeldAtIllidanLandingStack(action) ? 0.0f : 1.0f;
        }

        if (isMove)
        {
            return dynamic_cast<IllidanStormrageMainTankRepositionBossAction*>(
                action) ? 1.0f : 0.0f;
        }

        return isGapCloser ? 0.0f : 1.0f;
    }

    if (phase != IllidanPhase::Flying)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        if (isMove)
            return dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action) ? 1.0f : 0.0f;

        return isMeleeSpell ? 0.0f : 1.0f;
    }

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        if (isMove)
        {
            return dynamic_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(
                action) ? 1.0f : 0.0f;
        }

        return isHeal ? 0.0f : 1.0f;
    }

    return 1.0f;
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetNonTankValue(Action* action)
{
    bool const isBlockedMove =
        (dynamic_cast<CombatFormationMoveAction*>(action) &&
         !dynamic_cast<SetBehindTargetAction*>(action)) ||
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action);
    bool const isPhaseMove = !isBlockedMove &&
        (dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<SetBehindTargetAction*>(action) ||
         dynamic_cast<AvoidAoeAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<CastKillingSpreeAction*>(action));
    bool const isLandingMove = !isBlockedMove && IsHeldAtIllidanLandingStack(action);
    if (!isBlockedMove && !isPhaseMove && !isLandingMove)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || IsIllidanDeathScene(illidan))
        return 1.0f;

    if (isBlockedMove)
        return 0.0f;

    // Everyone waits at the landing stack until he has been led out.
    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (IsIllidanLandingWait(*illidan, phase))
        return 0.0f;

    if (!isPhaseMove)
        return 1.0f;

    if (phase == IllidanPhase::Flying)
        return 0.0f;

    return IsHeldIllidanChase(botAI, action, phase) ? 0.0f : 1.0f;
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
    return illidan && !IsIllidanDeathScene(illidan) &&
        GetIllidanPhase(illidan) != IllidanPhase::Flying ? 0.0f : 1.0f;
}

// Shadowfiends spawn on the player whose debuff ended, a valid target again, and any threat it
// makes on them brings them back onto it. So near them it holds damage on them, area damage, and
// for a healer heals, since heal threat reaches them too.
float IllidanStormrageShadowfiendBirtherHoldMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isHeal =
        PlayerbotAI::IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action);
    if (!isHeal)
    {
        if (!IsDpsHoldCandidate(bot, action))
            return 1.0f;

        Unit* target = AI_VALUE(Unit*, "current target");
        bool const isOnShadowfiend =
            target && target->GetEntry() == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND);
        auto* castSpellAction = dynamic_cast<CastSpellAction*>(action);
        bool const isAreaDamage = IsAoeThreatAction(bot, action) ||
            (castSpellAction &&
             castSpellAction->getThreatType() == Action::ActionThreatType::Aoe &&
             !dynamic_cast<CastHealingSpellAction*>(action));
        if (!isOnShadowfiend && !isAreaDamage)
            return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "illidan stormrage"))
        return 1.0f;

    return IsIllidanShadowfiendBirther(botAI) ? 0.0f : 1.0f;
}

// Guardians can't be set passive like pets, so they are summoned only in Phases 3 and 5.
float IllidanStormrageHoldGuardianSummonsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsGuardianSummon(bot, action))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    return phase == IllidanPhase::Grounded || phase == IllidanPhase::Maiev ? 1.0f : 0.0f;
}

// The targeting action only picks targets and keeps melee off him during a hold itself, so it can
// still switch a bot from him to an add.
float IllidanStormrageWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) ||
        dynamic_cast<IllidanStormrageDpsPrioritizeAddsAction*>(action))
    {
        return 1.0f;
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    // The Flame tanks get 6 s on the Flames of Azzinoth.
    if (GetIllidanPhase(illidan) == IllidanPhase::Flying)
    {
        constexpr uint32 flamePhaseDpsWaitMs = 6 * IN_MILLISECONDS;
        auto const it = illidanFlamePhaseStartTime.find(illidan->GetInstanceId());
        bool const inWindow = it == illidanFlamePhaseStartTime.end() ||
            getMSTimeDiff(it->second, getMSTime()) < flamePhaseDpsWaitMs;
        if (inWindow && AI_VALUE2(Unit*, "find target", "flame of azzinoth") &&
            !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            !PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
        {
            return GetDpsHoldValue(bot, action);
        }

        return 1.0f;
    }

    if (GetIllidanDpsHold(botAI, illidan) == IllidanDpsHold::None)
        return 1.0f;

    // Only damage on him is held, never damage on the adds.
    bool const isOnIllidan = AI_VALUE(Unit*, "current target") == illidan ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        (dynamic_cast<AttackAction*>(action) && action->GetTarget() == illidan);

    return isOnIllidan ? GetDpsHoldValue(bot, action) : 1.0f;
}
