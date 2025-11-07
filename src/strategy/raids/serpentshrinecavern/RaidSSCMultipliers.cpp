#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

#include "FollowActions.h"
#include "WipeAction.h"

using namespace SerpentShrineCavernHelpers;

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross");
    if (!hydross)
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if ((bot->HasAura(SPELL_MARK_OF_HYDROSS_100) || bot->HasAura(SPELL_MARK_OF_HYDROSS_250) ||
             bot->HasAura(SPELL_MARK_OF_HYDROSS_500)) &&
            (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastGrowlAction*>(action) ||
             dynamic_cast<CastHandOfReckoningAction*>(action)))
            return 0.0f;

        if (hydross->HasAura(SPELL_CORRUPTION) &&
            (dynamic_cast<CombatFormationMoveAction*>(action) || IsChargeAction(action)))
            return 0.0f;
    }

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if (!hydross->HasAura(SPELL_CORRUPTION) &&
            (dynamic_cast<CombatFormationMoveAction*>(action) || IsChargeAction(action)))
            return 0.0f;
    }

    Unit* botTarget = botAI->GetUnit(bot->GetTarget());
    if (botAI->IsAssistTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
    {
        if ((hydross->GetVictim() == bot || botTarget && botTarget == hydross) &&
            dynamic_cast<AttackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float HydrossTheUnstableWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    uint32 mapId = bot->GetMapId();
    const uint8 dpsWaitSeconds = 5;

    if (hydross && !hydross->HasAura(SPELL_CORRUPTION))
    {
        auto it = hydrossFrostDpsWaitTimer.find(mapId);
        if (it != hydrossFrostDpsWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!botAI->IsMainTank(bot) && (dynamic_cast<AttackAction*>(action) ||
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    if (hydross && hydross->HasAura(SPELL_CORRUPTION))
    {
        auto it = hydrossNatureDpsWaitTimer.find(mapId);
        if (it != hydrossNatureDpsWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!botAI->IsAssistTankOfIndex(bot, 0) && (dynamic_cast<AttackAction*>(action) ||
                !botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

float HydrossTheUnstableControlMisdirectionMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValue(Action* action)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    uint32 mapId = lurker->GetMapId();
    time_t now = time(nullptr);

    if (lurkerSpoutTimer.count(mapId) && lurkerSpoutTimer[mapId] > now)
    {
        if (IsChargeAction(action) || dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);

    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        (leotherasHuman->HasAura(SPELL_WHIRLWIND) || leotherasHuman->HasAura(SPELL_WHIRLWIND_CHANNEL)))
    {
        if (IsChargeAction(action) || (!botAI->IsTank(bot) && dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return 1.0f;

    Unit* leotherasDemonPhase2 = GetPhase2LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    if (botAI->IsTank(bot) && demonFormTank && leotherasDemonPhase2 &&
        leotherasDemonPhase2->GetVictim() != demonFormTank)
        return 0.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<CastShadowWardAction*>(action))
        return 0.0f;

    return 1.0f;
}

/* float LeotherasTheBlindMainTankMaintainDemonFormPositionMultiplier::GetValue(Action* action)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return 1.0f;

    if ((botAI->IsMainTank(bot) || botAI->IsTank(bot) && leotherasDemon->GetVictim() == bot) &&
        (dynamic_cast<MovementAction*>(action) && !dynamic_cast<LeotherasTheBlindDemonFormPositionBossAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindDemonFormDisableMeleeActionsMultiplier::GetValue(Action* action)
{
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    if (!leotherasPhase2Demon || leotherasPhase2Demon->HasAura(SPELL_LEOTHERAS_BANISHED) ||
        botAI->IsRanged(bot) || leotherasPhase2Demon->GetVictim() == bot ||
        botAI->IsMainTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<LeotherasTheBlindDemonFormPositionMeleeAction*>(action)) ||
        dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
} */

float LeotherasTheBlindWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    uint32 mapId = bot->GetMapId();

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    const uint8 dpsWaitSecondsPhase1 = 5;
    const uint8 dpsWaitSecondsPhase2 = 10;
    const uint8 dpsWaitSecondsPhase3 = 15;
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        auto it = leotherasHumanFormDpsWaitTimer.find(mapId);
        if (it != leotherasHumanFormDpsWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase1)
        {
            if (!botAI->IsTank(bot) && (dynamic_cast<AttackAction*>(action) ||
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }
    else if (leotherasPhase2Demon)
    {
        if (demonFormTank == bot)
            return 1.0f;

        auto it = leotherasDemonFormDpsWaitTimer.find(mapId);
        if (it != leotherasDemonFormDpsWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase2)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                !botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))
                return 0.0f;
        }
    }
    else if (leotherasPhase3Demon)
    {
        if (demonFormTank == bot)
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(mapId);
        if (it != leotherasFinalPhaseDpsWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase3)
        {
            if (!botAI->IsTank(bot) && (dynamic_cast<AttackAction*>(action) ||
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    return 1.0f;
}

float LeotherasTheBlindDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (!leotherasHuman && !leotherasPhase2Demon)
        return 1.0f;

    if (!leotherasPhase3Demon && (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

    return 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressControlMisdirectionMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    const uint8 dpsWaitSeconds = 8;
    auto it = karathressDpsWaitTimer.find(bot->GetMapId());
    if (it == karathressDpsWaitTimer.end() || (time(nullptr) - it->second) < dpsWaitSeconds)
    {
        if (!botAI->IsTank(bot) && (dynamic_cast<AttackAction*>(action) ||
            (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier::GetValue(Action* action)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis || !caribdis->IsAlive())
        return 1.0f;

    if (botAI->IsHealAssistantOfIndex(bot, 0) && dynamic_cast<FleeAction*>(bot))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    Unit* murloc = AI_VALUE2(Unit*, "find target", "tidewalker lurker");
    if (!murloc && (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerDisablePhase2FleeActionMultiplier::GetValue(Action* action)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return 1.0f;

    if (tidewalker->GetHealthPct() < 25.0f)
    {
        if (dynamic_cast<FleeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (!IsLadyVashjInPhase3(botAI) &&
        (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

    /* if (IsLadyVashjInPhase1(botAI) && dynamic_cast<CastMetamorphosisAction*>(action))
        return 0.0f; */

    return 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || IsLadyVashjInPhase2(botAI))
        return 1.0f;

    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<LadyVashjStaticChargeMoveAwayFromGroupAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<LootAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValue(Action* action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    Group* group = bot->GetGroup();
    if (!vashj || !IsLadyVashjInPhase2(botAI)|| !group || dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (bot == GetFirstTaintedCorePasser(group, botAI) ||
        bot == GetSecondTaintedCorePasser(group, botAI) ||
        bot == GetThirdTaintedCorePasser(group, botAI))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_PARALYZE))
            {
                if (dynamic_cast<MovementAction*>(action) &&
                    !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action) &&
                    !dynamic_cast<AttackAction*>(action))
                    return 0.0f;
            }
        }
    }

    Player* master = botAI->GetMaster();
    if (master && bot == GetDesignatedCoreLooter(group, master, botAI) && bot->HasAura(SPELL_PARALYZE))
    {
        if (!dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action) &&
            !dynamic_cast<AttackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float LadyVashjDisableAutomaticTargetingAndMovementModifier::GetValue(Action *action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || !IsLadyVashjInPhase2(botAI) /* || botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsRangedDpsAssistantOfIndex(bot, 1)*/)
        return 1.0f;

    /* if (botAI->IsMainTank(bot) && dynamic_cast<TankAssistAction*>(action))
        return 0.0f; */

    if ((dynamic_cast<DpsAssistAction*>(action) ||
         dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action)))
        return 0.0f;

    Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
    if (enchanted && enchanted->IsAlive() && bot->GetVictim() == enchanted &&
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 0.0f;

    return 1.0f;
}
