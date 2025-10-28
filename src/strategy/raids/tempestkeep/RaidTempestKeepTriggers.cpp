#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "Playerbots.h"

using namespace TempestKeepHelpers;

bool VoidReaverBossEngagedByTankTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && botAI->IsTank(bot) && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossEngagedByRangedTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");

    return voidReaver && botAI->IsRanged(bot);
}

bool VoidReaverArcaneOrbLaunchedTrigger::IsActive()
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || botAI->IsTank(bot))
        return false;

    std::vector<Unit*> arcaneOrbTargets = GetAllArcaneOrbTargets(botAI, bot);
    for (Unit* orbTarget : arcaneOrbTargets)
    {
        if (bot->GetExactDist2d(orbTarget) < 25.0f)
            return true;
    }

    return false;
}

bool HighAstromancerSolarianPhase1And2MovementTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    Unit* solariumPriest = AI_VALUE2(Unit*, "find target", "solarium priest");

    return astromancer && solariumPriest && botAI->IsMelee(bot);
}

bool HighAstromancerSolarianTransformedIntoVoidwalkerTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");

    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && botAI->IsMainTank(bot);
}

bool HighAstromancerSolarianCanCastFearWardOnMainTankTrigger::IsActive()
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    Player* mainTank = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM) && 
           bot->getClass() == CLASS_PRIEST && mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}
