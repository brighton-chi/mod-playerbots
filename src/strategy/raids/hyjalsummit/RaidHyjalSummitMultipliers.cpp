#include "RaidHyjalSummitMultipliers.h"
#include "RaidHyjalSummitActions.h"
#include "RaidHyjalSummitHelpers.h"
#include "ShamanActions.h"

using namespace HyjalSummitHelpers;

// Rage Winterchill

float RageWinterchillDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return 1.0f;

    bool hasDeathAndDecay = false;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_DEATH_AND_DECAY))
            {
                hasDeathAndDecay = true;
                break;
            }
        }
    }

    if (!hasDeathAndDecay)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float RageWinterchillDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (winterchill)
    {
        if (dynamic_cast<TankFaceAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Anetheron

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
    if (!anetheron)
        return 1.0f;

    if (anetheron->GetHealthPct() > 80.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
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
    if (!kazrogal)
        return 1.0f;

    if (kazrogal->GetHealthPct() > 90.0f)
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
    if (!azgalor)
        return 1.0f;

    if (azgalor->GetHealthPct() > 90.0f)
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
    if (!archimonde)
        return 1.0f;

    if (archimonde->GetHealthPct() > 90.0f)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
