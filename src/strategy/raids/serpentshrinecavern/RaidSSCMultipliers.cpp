#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DestroyItemAction.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WipeAction.h"

using namespace SerpentShrineCavernHelpers;

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_TOXIC_POOL))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        if (hydross->HasAura(SPELL_CORRUPTION))
        {
            if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                (dynamic_cast<AttackAction*>(action) &&
                 !dynamic_cast<HydrossTheUnstablePositionFrostTankAction*>(action)))
                 return 0.0f;
        }
    }

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (!hydross->HasAura(SPELL_CORRUPTION))
        {
            if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) ||
                (dynamic_cast<AttackAction*>(action) &&
                 !dynamic_cast<HydrossTheUnstablePositionNatureTankAction*>(action)))
                 return 0.0f;
        }
    }

    return 1.0f;
}

float HydrossTheUnstableWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
    if (botAI->IsAssistTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0) &&
        (waterElemental || natureElemental))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstableMisdirectBossToTankAction*>(action))
        return 1.0f;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    const uint8 phaseChangeWaitSeconds = 1;
    const uint8 dpsWaitSeconds = 5;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !botAI->IsMainTank(bot))
    {
        auto itDps = hydrossFrostDpsWaitTimer.find(instanceId);
        auto itPhase = hydrossChangeToFrostPhaseTimer.find(instanceId);

        bool justChanged = (itDps == hydrossFrostDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);
        bool aboutToChange = (itPhase != hydrossChangeToFrostPhaseTimer.end() &&
                            (now - itPhase->second) > phaseChangeWaitSeconds);

        if (justChanged || aboutToChange)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                 return 0.0f;
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && !botAI->IsAssistTankOfIndex(bot, 0))
    {
        auto itDps = hydrossNatureDpsWaitTimer.find(instanceId);
        auto itPhase = hydrossChangeToNaturePhaseTimer.find(instanceId);

        bool justChanged = (itDps == hydrossNatureDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);
        bool aboutToChange = (itPhase != hydrossChangeToNaturePhaseTimer.end() &&
                            (now - itPhase->second) > phaseChangeWaitSeconds);


        if (justChanged || aboutToChange)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                 return 0.0f;
        }
    }

    return 1.0f;
}

float HydrossTheUnstableControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (hydross)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValue(Action* action)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    if (it != lurkerSpoutTimer.end() && it->second > now)
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float TheLurkerBelowMaintainRangedSpreadMultiplier::GetValue(Action* action)
{
    if (!botAI->IsRanged(bot))
        return 1.0f;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (lurker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Disable tank assist during Submerge only if there are 3 or more tanks in the raid
float TheLurkerBelowDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    uint8 tankCount = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (botAI->IsTank(member))
            ++tankCount;
    }

    if (tankCount >= 3)
    {
        if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot))
        return 1.0f;

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (!leotherasHuman)
        return 1.0f;

    if (!leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        (leotherasHuman->HasAura(SPELL_WHIRLWIND) ||
         leotherasHuman->HasAura(SPELL_WHIRLWIND_CHANNEL)))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Applies only if there is a Warlock tank
float LeotherasTheBlindDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return 1.0f;

    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (!demonFormTank)
        return 1.0f;

    // (1) Warlock tank will not use Shadow Ward
    // Shadow Ward is coded into the Warlock tank strategy (for Twin Emps) but is useless here
    if (dynamic_cast<CastShadowWardAction*>(action))
        return 0.0f;

    // (2) Phase 2 only: Tanks other than the Warlock tank should do absolutely nothing
    Unit* leotherasDemonPhase2 = GetPhase2LeotherasDemon(botAI);
    if (botAI->IsTank(bot) && bot != demonFormTank && leotherasDemonPhase2)
    {
        if ((dynamic_cast<AttackAction*>(action) &&
             !dynamic_cast<LeotherasTheBlindInnerDemonCheatAction*>(action)) ||
             dynamic_cast<CastSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMelee(bot) || botAI->IsTank(bot))
        return 1.0f;

    Unit* leotherasDemonPhase2 = GetPhase2LeotherasDemon(botAI);
    if (!leotherasDemonPhase2)
        return 1.0f;

    Aura* chaosBlast = bot->GetAura(SPELL_CHAOS_BLAST);
    if (chaosBlast && chaosBlast->GetStackAmount() >= 5)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    if (dynamic_cast<LeotherasTheBlindMisdirectBossToDemonFormTankAction*>(action))
        return 1.0f;

    const uint32 instanceId = leotheras->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    const uint8 dpsWaitSecondsPhase1 = 5;
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        !leotherasPhase3Demon)
    {
        if (botAI->IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanFormDpsWaitTimer.find(instanceId);
        if (it == leotherasHumanFormDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase1)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    const uint8 dpsWaitSecondsPhase2 = 10;
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);
    if (leotherasPhase2Demon)
    {
        if (demonFormTank && demonFormTank == bot)
            return 1.0f;

        if (!demonFormTank && botAI->IsTank(bot))
            return 1.0f;

        auto it = leotherasDemonFormDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonFormDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase2)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    const uint8 dpsWaitSecondsPhase3 = 8;
    if (leotherasPhase3Demon)
    {
        if ((demonFormTank && demonFormTank == bot) || botAI->IsTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase3)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Wait until the final phase to use Bloodlust/Heroism
float LeotherasTheBlindDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasPhase3Demon)
    {
        if (dynamic_cast<CastHeroismAction*>(action) ||
            dynamic_cast<CastBloodlustAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if ((bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action)) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress)
    {
        if (dynamic_cast<DpsAoeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<FathomLordKarathressMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 10;

    auto it = karathressDpsWaitTimer.find(karathress->GetMap()->GetInstanceId());
    if (it == karathressDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) &&
             !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier::GetValue(Action* action)
{
    if (!botAI->IsHealAssistantOfIndex(bot, 0))
        return 1.0f;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (caribdis && caribdis->IsAlive())
    {
        if (dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Use Bloodlust/Heroism after the first Murloc spawn
float MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    Unit* murloc = AI_VALUE2(Unit*, "find target", "tidewalker lurker");
    if (!murloc)
    {
        if (dynamic_cast<CastHeroismAction*>(action) ||
            dynamic_cast<CastBloodlustAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float MorogrimTidewalkerDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMainTank(bot))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (tidewalker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float MorogrimTidewalkerMaintainPhase2StackingMultiplier::GetValue(Action* action)
{
    if (!botAI->IsRanged(bot))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    if (tidewalker->GetHealthPct() < 25.0f)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Wait until phase 3 to use Bloodlust/Heroism
float LadyVashjDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || IsLadyVashjInPhase3(botAI))
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValue(Action* action)
{
    if (!botAI->IsRanged(bot))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj && IsLadyVashjInPhase1(botAI))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    if (botAI->IsMainTank(bot) || !bot->HasAura(SPELL_STATIC_CHARGE))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// If raid cheat (which enables bot looting of the core) is not enabled
// Bots should not loot the core
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    if (!botAI->HasCheat(BotCheatMask::raid))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj)
    {
        if (dynamic_cast<LootAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !IsLadyVashjInPhase2(botAI))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<DestroyItemAction*>(action) ||
        dynamic_cast<LadyVashjDestroyTaintedCoreAction*>(action))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);

    auto hasCore = [](Player* player)
    {
        return player && player->HasItemCount(ITEM_TAINTED_CORE, 1, false);
    };

    if (bot == designatedLooter)
    {
        if (!hasCore(bot))
            return 1.0f;
    }
    else if (bot == firstCorePasser)
    {
        if (hasCore(secondCorePasser) || hasCore(thirdCorePasser) ||
            hasCore(fourthCorePasser))
            return 1.0f;
    }
    else if (bot == secondCorePasser)
    {
        if (hasCore(thirdCorePasser) || hasCore(fourthCorePasser))
            return 1.0f;
    }
    else if (bot == thirdCorePasser)
    {
        if (hasCore(fourthCorePasser))
            return 1.0f;
    }
    else if (bot != fourthCorePasser)
        return 1.0f;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && (bot == firstCorePasser || bot == secondCorePasser))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
            return 0.0f;
    }

    if (AnyRecentCoreInInventory(group, botAI))
    {
        if (!dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// All of phases 2 and 3 require a custom movement and targeting system
// So the standard target selection system must be disabled
float LadyVashjDisableAutomaticTargetingAndMovementModifier::GetValue(Action *action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (IsLadyVashjInPhase2(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action))
            return 0.0f;

        if (!botAI->IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        if (enchanted && enchanted->IsAlive() && bot->GetVictim() == enchanted)
        {
            if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
        }
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action))
            return 0.0f;

        Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
        Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");

        if (enchanted && enchanted->IsAlive())
        {
            if (bot->GetVictim() == enchanted)
            {
                if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
            }
        }

        if ((!enchanted || !enchanted->IsAlive()) && (!strider || !strider->IsAlive()) &&
            (!elite || !elite->IsAlive()))
        {
            if (dynamic_cast<SetBehindTargetAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}
