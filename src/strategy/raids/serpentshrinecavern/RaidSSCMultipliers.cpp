#include "RaidSSCMultipliers.h"
#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
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

float HydrossTheUnstableDisableTankAssistMultiplier::GetValue(Action* action)
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

        if ((bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) || bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
             bot->HasAura(SPELL_MARK_OF_CORRUPTION_500)) &&
            (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastGrowlAction*>(action) || 
             dynamic_cast<CastHandOfReckoningAction*>(action)))
            return 0.0f;

        if (!hydross->HasAura(SPELL_CORRUPTION) && 
            (dynamic_cast<CombatFormationMoveAction*>(action) || IsChargeAction(action)))
            return 0.0f;
    }

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

float LeotherasTheBlindDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* leotheras = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);
    if (!leotheras)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");

    if (leotheras && (leotheras->HasAura(SPELL_WHIRLWIND) || leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL)))
    {
        if (IsChargeAction(action) || (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float LeotherasTheBlindWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    uint32 mapId = leotheras->GetMapId();

    Unit* leotherasHuman = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_HUMAN_FORM);
    Unit* leotherasDemon = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);
    Player* demonTank = GetLeotherasDemonFormTank(botAI, bot);

    const uint8 dpsWaitSeconds = 5;
    if (leotherasHuman && !leotherasDemon)
    {
        auto it = leotherasHumanFormDPSWaitTimer.find(mapId);
        if (it != leotherasHumanFormDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!botAI->IsMainTank(bot) && (dynamic_cast<AttackAction*>(action) || 
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }
    else if (leotherasDemon && !leotherasHuman)
    {
        auto it = leotherasDemonFormDPSWaitTimer.find(mapId);
        if (it != leotherasDemonFormDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!demonTank && (dynamic_cast<AttackAction*>(action) || 
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }
    else if (leotherasHuman && leotherasDemon)
    {
        auto it = leotherasFinalPhaseDPSWaitTimer.find(mapId);
        if (it != leotherasFinalPhaseDPSWaitTimer.end() && (time(nullptr) - it->second) < dpsWaitSeconds)
        {
            if (!botAI->IsMainTank(bot) && !demonTank && 
                (dynamic_cast<AttackAction*>(action) || (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    return 1.0f;
}

float LeotherasTheBlindWaitForBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    Unit* leotherasHuman = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_HUMAN_FORM);
    Unit* leotherasDemon = GetFirstAliveUnitByEntry(botAI, NPC_LEOTHERAS_THE_BLIND_DEMON_FORM);
    if (!(leotherasHuman && leotherasDemon) && (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}
