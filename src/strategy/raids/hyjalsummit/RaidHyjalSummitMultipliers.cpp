#include "RaidHyjalSummitMultipliers.h"
#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "HunterActions.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

using namespace HyjalSummitHelpers;

float HyjalSummitTimeBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    bool canUseDuringWinterchill = false;
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (winterchill && winterchill->GetHealthPct() < 90.0f)
        canUseDuringWinterchill = true;

    bool canUseDuringAnetheron = false;
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron && anetheron->GetHealthPct() < 80.0f)
        canUseDuringAnetheron = true;

    bool canUseDuringKazrogal = false;
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (kazrogal && kazrogal->GetHealthPct() < 90.0f)
        canUseDuringKazrogal = true;

    bool canUseDuringAzgalor = false;
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (azgalor && azgalor->GetHealthPct() < 90.0f)
        canUseDuringAzgalor = true;

    bool canUseDuringArchimonde = false;
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (archimonde && archimonde->GetHealthPct() < 98.0f)
        canUseDuringArchimonde = true;

    if (!canUseDuringWinterchill &&
        !canUseDuringAnetheron &&
        !canUseDuringKazrogal &&
        !canUseDuringAzgalor &&
        !canUseDuringArchimonde)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Rage Winterchill

float RageWinterchillDisableMainTankAvoidAoeMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMainTank(bot))
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (winterchill)
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float RageWinterchillDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) && !botAI->IsRanged(bot))
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (winterchill)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Anetheron

float AnetheronDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron)
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;

        if (!botAI->IsAssistTankOfIndex(bot, 0))
        {
            if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action) ||
                dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float AnetheronDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) && !botAI->IsRanged(bot))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AnetheronInfernalTankMaintainPositionMultiplier::GetValue(Action* action)
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return 1.0f;

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (!infernal)
    {
        if (dynamic_cast<ReachTargetAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Kaz'rogal

float KazrogalLowManaBotStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT ||
        (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL))
        return 1.0f;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return 1.0f;

    if (bot->GetPower(POWER_MANA) <= 3000)
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<AttackAction*>(action) &&
             !dynamic_cast<KazrogalLowManaBotMoveFromGroupAction*>(action)))
             return 0.0f;
    }

    return 1.0f;
}

// Azgalor

float AzgalorDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return 1.0f;

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    if (doomguard)
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AzgalorTanksMaintainPositionMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return 1.0f;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (azgalor)
    {
        if (dynamic_cast<TankFaceAction*>(action) ||
            dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;
    }

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    if (doomguard || AnyGroupMemberHasDoom(bot))
    {
        if (botAI->IsAssistTankOfIndex(bot, 0))
        {
            if (dynamic_cast<MovementAction*>(action) &&
                !dynamic_cast<AttackAction*>(action) &&
                !dynamic_cast<AzgalorFirstAssistTankPositionDoomguardAction*>(action))
                return 0.0f;

            if (dynamic_cast<CastReachTargetSpellAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_DOOM))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<AttackAction*>(action) &&
            !dynamic_cast<AvoidAoeAction*>(action) &&
            !dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

// Archimonde

