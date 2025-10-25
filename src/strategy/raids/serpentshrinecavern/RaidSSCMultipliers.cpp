#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

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

float HydrossTheUnstableWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    uint32 mapId = hydross->GetMapId();
    const uint8 dpsWaitSeconds = 5;

    if (hydross && !hydross->HasAura(SPELL_CORRUPTION))
    {
        auto it = hydrossFrostDPSWaitTimer.find(mapId);
        if (it != hydrossFrostDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!botAI->IsMainTank(bot) && (dynamic_cast<AttackAction*>(action) || 
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    if (hydross && hydross->HasAura(SPELL_CORRUPTION))
    {
        auto it = hydrossNatureDPSWaitTimer.find(mapId);
        if (it != hydrossNatureDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
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

float TheLurkerBelowStayBehindBossDuringSpoutMultiplier::GetValue(Action* action)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    if (lurker->HasAura(SPELL_SPOUT_VISUAL) || 
        lurker->HasAura(SPELL_SPOUT_PERIODIC_1) || lurker->HasAura(SPELL_SPOUT_PERIODIC_2))
    {
        if (dynamic_cast<TankFaceAction*>(action) || IsChargeAction(action) || 
            dynamic_cast<CastKillingSpreeAction*>(action))
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

float LeotherasTheBlindWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    uint32 mapId = leotheras->GetMapId();

    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    Player* demonFormTank = GetLeotherasDemonFormTank(botAI, bot);

    const uint8 dpsWaitSecondsPhase1 = 5;
    const uint8 dpsWaitSecondsPhase2 = 10;
    const uint8 dpsWaitSecondsPhase3 = 15;
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        auto it = leotherasHumanFormDPSWaitTimer.find(mapId);
        if (it != leotherasHumanFormDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase1)
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

        auto it = leotherasDemonFormDPSWaitTimer.find(mapId);
        if (it != leotherasDemonFormDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase2)
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

        auto it = leotherasFinalPhaseDPSWaitTimer.find(mapId);
        if (it != leotherasFinalPhaseDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSecondsPhase3)
        {
            if (!botAI->IsTank(bot) && (dynamic_cast<AttackAction*>(action) || 
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    return 1.0f;
}

float LeotherasTheBlindWaitForBloodlustAndHeroismMultiplier::GetValue(Action* action)
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
