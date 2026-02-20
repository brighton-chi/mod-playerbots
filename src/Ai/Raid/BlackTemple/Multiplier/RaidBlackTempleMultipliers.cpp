#include "RaidBlackTempleMultipliers.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace BlackTempleHelpers;

// Trash
float ShadowmoonReaverDontBuildChaoticChargesMultiplier::GetValue(Action* action)
{
    if (bot->getClass() == CLASS_HUNTER || !botAI->IsRangedDps(bot))
        return 1.0f;

    Unit* reaver = AI_VALUE2(Unit*, "find target", "shadowmoon reaver");
    if (!reaver || !reaver->HasAura(SPELL_SPELL_ABSORPTION))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction &&
        (castSpellAction->getThreatType() == Action::ActionThreatType::Aoe ||
         (bot->GetVictim() == reaver &&
          castSpellAction->getThreatType() == Action::ActionThreatType::Single)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// High Warlord Naj'entus
float HighWarlordNajentusDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Supremus
float SupremusFocusOnAvoidanceInPhase2Multiplier::GetValue(Action* action)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || !supremus->HasAura(SPELL_SNARE_SELF) ||
        supremus->GetVictim() != bot)
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<SupremusKiteBossAction*>(action) &&
        !dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action))
        return 0.0f;

    return 1.0f;
}

float SupremusHitboxIsBuggedMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE ||
        !AI_VALUE2(Unit*, "find target", "supremus"))
        return 1.0f;

    if (dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Teron Gorefiend
float TeronGorefiendControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValue(Action* action)
{
    Aura* aura = bot->GetAura(SPELL_SHADOW_OF_DEATH);
    if (!aura || aura->GetDuration() >= 15000)
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;
    else if (!dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_SPIRITUAL_VENGEANCE))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;
    else if (!dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendDisableAttackingConstructsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (!botAI->IsRangedDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction &&
        castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (bot->HasAura(SPELL_PLAYER_FEL_RAGE) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<AttackAction*>(action)))
         return 0.0f;

    return 1.0f;
}

// Reliquary of Souls

float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "essence of suffering"))
        return 1.0f;

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        if (bot->getClass() == CLASS_PRIEST && botAI->IsHeal(bot))
            return 10.0f;
    }
    else if (dynamic_cast<CastHealingSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_FATAL_ATTRACTION))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;
    else if (!dynamic_cast<MotherShahrazRunAwayToBreakFatalAttractionAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Delay is to allow bots to get positioned first
float MotherShahrazDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < 85.0f)
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Illidari Council

float IllidariCouncilDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || gathios->GetHealthPct() < 90.0f)
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return 1.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastChallengingShoutAction*>(action) ||
        dynamic_cast<CastShockwaveAction*>(action) ||
        dynamic_cast<CastCleaveAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastSwipeAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastDeathAndDecayAction*>(action) ||
        dynamic_cast<CastBloodBoilAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsAssistHealOfIndex(bot, 0, true) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action)))
         return 0.0f;

    if ((botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0, false) ||
        botAI->IsAssistTankOfIndex(bot, 1, false) ||
        GetZerevorMageTank(bot) == bot) &&
        dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return 1.0f;

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableArcaneShotOnZerevorMultiplier::GetValue(Action* action)
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

float IllidariCouncilManageInterruptsMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARRIOR)
        return 1.0f;

    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return 1.0f;

    if (malande->HasAura(SPELL_BLESSING_OF_PROTECTION) &&
        (dynamic_cast<CastKickAction*>(action) ||
         dynamic_cast<CastPummelAction*>(action) ||
         dynamic_cast<CastShieldBashAction*>(action)))
         return 0.0f;

    if (malande->HasAura(SPELL_BLESSING_OF_SPELL_WARDING) &&
        (dynamic_cast<CastWindShearAction*>(action) ||
         dynamic_cast<CastWindShearOnEnemyHealerAction*>(action)))
         return 0.0f;

    return 1.0f;
}

float IllidariCouncilWaitForDpsMultiplier::GetValue(Action* action)
{
    if (botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0, false) ||
        botAI->IsAssistTankOfIndex(bot, 1, false) ||
        GetZerevorMageTank(bot) == bot)
        return 1.0f;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return 1.0f;

    if (dynamic_cast<IllidariCouncilMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = councilDpsWaitTimer.find(gathios->GetMap()->GetInstanceId());
    if (it != councilDpsWaitTimer.end() || (now - it->second) >= dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) &&
         !dynamic_cast<CastHealingSpellAction*>(action)))
         return 0.0f;

    return 1.0f;
}

// Illidan Stormrage <The Betrayer>

float IllidanStormrageDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (illidan->GetHealthPct() > 62.0f &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
         return 0.0f;

    if (illidan->GetHealthPct() <= 62.0f || illidan->GetHealthPct() > 95.0f)
        return 1.0f;

    if (!botAI->IsDps(bot))
        return 1.0f;

    if (dynamic_cast<CastMetamorphosisAction*>(action) ||
        dynamic_cast<CastAdrenalineRushAction*>(action) ||
        dynamic_cast<CastBladeFlurryAction*>(action) ||
        dynamic_cast<CastIcyVeinsAction*>(action) ||
        dynamic_cast<CastColdSnapAction*>(action) ||
        dynamic_cast<CastArcanePowerAction*>(action) ||
        dynamic_cast<CastPresenceOfMindAction*>(action) ||
        dynamic_cast<CastCombustionAction*>(action) ||
        dynamic_cast<CastRapidFireAction*>(action) ||
        dynamic_cast<CastReadinessAction*>(action) ||
        dynamic_cast<CastAvengingWrathAction*>(action) ||
        dynamic_cast<CastElementalMasteryAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastArmyOfTheDeadAction*>(action) ||
        dynamic_cast<CastSummonGargoyleAction*>(action) ||
        dynamic_cast<CastBerserkingAction*>(action) ||
        dynamic_cast<CastBloodFuryAction*>(action) ||
        dynamic_cast<UseTrinketAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidanStormrageControlMovementMultiplier::GetValue(Action* action)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (!botAI->IsMainTank(bot) && dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (GetIllidanPhase(illidan) == 2)
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;

        if (!botAI->IsAssistTankOfIndex(bot, 0, true) &&
            !botAI->IsAssistTankOfIndex(bot, 1, true))
            return 1.0f;

        if  ((dynamic_cast<MovementAction*>(action) &&
              !dynamic_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(action)) ||
              dynamic_cast<CastHealingSpellAction*>(action))
              return 0.0f;
    }

    return 1.0f;
}

// RENAME TO TANK ASSIST IF DPS ASSIST DISABLE APPROACH NOT TAKEN
float IllidanStormrageDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    if (bot->GetVictim() == nullptr)
        return 1.0f;

    // if (!botAI->IsTank(bot))
    //     return 1.0f;
    if (botAI->IsHeal(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    // Issue with limit to 2/4 is it makes tanks go after Shadowfiends; can I get away with disabling?
    // if (GetIllidanPhase(illidan) != 2 && GetIllidanPhase(illidan) != 4)
    //     return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Testing more of a generic movement suppression multiplier
float IllidanStormrageStayWithinGrateMultiplier::GetValue(Action* action)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (botAI->IsMainTank(bot) && GetIllidanPhase(illidan) == 2 &&
        ((dynamic_cast<MovementAction*>(action) &&
          !dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action)) ||
          dynamic_cast<CastMeleeSpellAction*>(action) ||
          dynamic_cast<CastReachTargetSpellAction*>(action)))
          return 0.0f;

    if (botAI->IsRanged(bot) &&
        (GetIllidanPhase(illidan) != 1 || bot->HasAura(SPELL_PARASITIC_SHADOWFIEND)) &&
        (dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<FollowAction*>(action) ||
         dynamic_cast<CastDisengageAction*>(action) ||
         dynamic_cast<CastBlinkBackAction*>(action)))
         return 0.0f;

    if (botAI->IsMelee(bot) && (GetIllidanPhase(illidan) == 2 ||
        bot->HasAura(SPELL_PARASITIC_SHADOWFIEND)) &&
        (dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<FollowAction*>(action) ||
         dynamic_cast<SetBehindTargetAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<CastKillingSpreeAction*>(action)))
         return 0.0f;

    return 1.0f;
}

float IllidanStormrageWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (dynamic_cast<IllidanStormrageMisdirectToTankAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = illidan->GetMap()->GetInstanceId();

    if ((GetIllidanPhase(illidan) == 1 || GetIllidanPhase(illidan) == 3) &&
        !botAI->IsMainTank(bot))
    {
        constexpr uint8 elfPhaseDpsWaitSeconds = 3;
        auto it = illidanBossDpsWaitTimer.find(instanceId);

        if ((it == illidanBossDpsWaitTimer.end() || (now - it->second) < elfPhaseDpsWaitSeconds) &&
            (dynamic_cast<AttackAction*>(action) ||
             (dynamic_cast<CastSpellAction*>(action) &&
              !dynamic_cast<CastHealingSpellAction*>(action))))
              return 0.0f;
    }

    if (GetIllidanPhase(illidan) == 4 && GetIllidanWarlockTank(bot) != bot)
    {
        constexpr uint8 demonPhaseDpsWaitSeconds = 8;
        auto it = illidanBossDpsWaitTimer.find(instanceId);

        if ((it == illidanBossDpsWaitTimer.end() || (now - it->second) < demonPhaseDpsWaitSeconds) &&
            (dynamic_cast<AttackAction*>(action) ||
             (dynamic_cast<CastSpellAction*>(action) &&
              !dynamic_cast<CastHealingSpellAction*>(action))))
              return 0.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "flame of azzinoth") &&
        !botAI->IsAssistTankOfIndex(bot, 0, true) &&
        !botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        constexpr uint8 flamePhaseDpsWaitSeconds = 7;
        auto it = illidanFlameDpsWaitTimer.find(instanceId);

        if ((it == illidanFlameDpsWaitTimer.end() || (now - it->second) < flamePhaseDpsWaitSeconds) &&
            (dynamic_cast<AttackAction*>(action) ||
             (dynamic_cast<CastSpellAction*>(action) &&
              !dynamic_cast<CastHealingSpellAction*>(action))))
              return 0.0f;
    }

    return 1.0f;
}
