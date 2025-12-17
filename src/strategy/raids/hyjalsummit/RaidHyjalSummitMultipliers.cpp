#include "RaidHyjalSummitMultipliers.h"
#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "ChooseTargetActions.h"
#include "HunterActions.h"
#include "ShamanActions.h"

using namespace HyjalSummitHelpers;

// Rage Winterchill

float RageWinterchillDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (winterchill && winterchill->GetHealthPct() > 90.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

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
    if (botAI->IsDps(bot) || botAI->IsHeal(bot))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron)
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;

        if (!botAI->IsAssistTankOfIndex(bot, 0))
        {
            if (dynamic_cast<TankAssistAction*>(action))
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

float AnetheronDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (anetheron && anetheron->GetHealthPct() > 80.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
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

float KazrogalDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (kazrogal && kazrogal->GetHealthPct() > 90.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Azgalor

float AzgalorDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (azgalor && azgalor->GetHealthPct() > 90.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Archimonde

float ArchimondeDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (archimonde && archimonde->GetHealthPct() > 90.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
