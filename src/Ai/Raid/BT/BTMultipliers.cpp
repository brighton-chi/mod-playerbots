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
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "WipeAction.h"
#include <array>

using namespace BlackTempleHelpers;
using namespace EncounterHelpers;

// General

// Illidan is handled separately
float BlackTempleDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    static constexpr std::array blackTempleBosses = {
        "gathios the shatterer", "mother shahraz", "essence of suffering", "gurtogg bloodboil",
        "teron gorefiend", "supremus", "high warlord naj'entus",
    };

    Unit* boss = nullptr;
    for (char const* name : blackTempleBosses)
    {
        if (Unit* candidate = AI_VALUE2(Unit*, "find target", name))
        {
            boss = candidate;
            break;
        }
    }

    if (boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    return 1.0f;
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
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action) ||
        dynamic_cast<SupremusKiteBossAction*>(action))
    {
        return 1.0f;
    }

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || supremus->GetVictim() != bot)
        return 1.0f;

    return IsSupremusKitePhase(supremus) ? 0.0f : 1.0f;
}

float SupremusDelayDpsCooldownsInKitePhaseMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return IsSupremusKitePhase(supremus) ? 0.0f : 1.0f;
}

float SupremusDisableKillingSpreeMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "supremus") ? 0.0f : 1.0f;
}

// Teron Gorefiend

float TeronGorefiendControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValueInEncounter(Action* action)
{
    Aura* aura = bot->GetAura(Id(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    if (!aura || aura->GetDuration() >= 15000)
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;
    else if (!dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BlackTempleSpells::SPELL_SPIRITUAL_VENGEANCE)) ||
        dynamic_cast<WipeAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendDisableAttackingConstructsMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (bot->GetVictim() && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (!PlayerbotAI::IsRangedDps(bot))
        return 1.0f;

    CastSpellAction* castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction && castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (bot->HasAura(Id(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)) &&
        dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Reliquary of Souls

float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "essence of suffering"))
        return 1.0f;

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastTreeFormAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz") ||
        !bot->HasAura(Id(BlackTempleSpells::SPELL_FATAL_ATTRACTION)))
    {
        return 1.0f;
    }

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MotherShahrazRunAwayToBreakFatalAttractionAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Illidari Council

float IllidariCouncilDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action) &&
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
    {
        return 0.0f;
    }

    if (dynamic_cast<TankFaceAction*>(action) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return 0.0f;

    if (dynamic_cast<AvoidAoeAction*>(action) &&
        ((PlayerbotAI::IsTank(bot) &&
            (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
                PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))) ||
            (bot->getClass() == CLASS_MAGE && GetZerevorMageTank(botAI) == bot)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidariCouncilControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableIceBlockMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_MAGE ||
        !AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
    {
        return 1.0f;
    }

    if (GetZerevorMageTank(botAI) != bot)
        return 1.0f;

    if (dynamic_cast<CastIceBlockAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableArcaneShotOnZerevorMultiplier::GetValueInEncounter(Action* action)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetGUID() != zerevor->GetGUID())
        return 1.0f;

    if (dynamic_cast<CastArcaneShotAction*>(action))
        return 0.0f;

    return 1.0f;
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

    if ((PlayerbotAI::IsTank(bot) &&
            (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
                PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))) ||
        (bot->getClass() == CLASS_MAGE && GetZerevorMageTank(botAI) == bot))
    {
        return 1.0f;
    }

    return 0.0f;
}

// Illidan Stormrage <The Betrayer>

float IllidanStormrageDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (illidan->GetHealthPct() <= 62.0f)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (illidan->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 1.0f;

    return 0.0f;
}

float IllidanStormrageControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
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
    if (!illidan || illidan->GetHealth() == 1)
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
    Unit* shadowDemon = bot->FindNearestCreature(Id(BlackTempleNpcs::NPC_SHADOW_DEMON), searchRadius);
    Unit* shadowfiend = bot->FindNearestCreature(
        Id(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND), searchRadius);

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
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
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
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) == 2)
        return 1.0f;

    if (dynamic_cast<CastStrengthOfEarthTotemAction*>(action) ||
        dynamic_cast<CastStoneskinTotemAction*>(action) ||
        dynamic_cast<CastStoneclawTotemAction*>(action) ||
        dynamic_cast<CastTremorTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
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
