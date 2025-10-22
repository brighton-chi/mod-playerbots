#include "RaidSSCTriggers.h"
#include "RaidSSCHelpers.h"
#include "RaidSSCActions.h"
#include "Playerbots.h"

using namespace SerpentShrineCavernHelpers;

// Trash Mobs

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    Unit* totem = AI_VALUE2(Unit*, "find target", "water elemental totem");

    return totem && botAI->IsDps(bot);
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableBotIsFrostTankTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && botAI->IsMainTank(bot);
}

bool HydrossTheUnstableBossInFrostPhaseTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && !hydross->HasAura(SPELL_CORRUPTION) && botAI->IsMainTank(bot) && 
    !bot->HasAura(SPELL_MARK_OF_HYDROSS_100) && !bot->HasAura(SPELL_MARK_OF_HYDROSS_250) &&
    !bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
}

bool HydrossTheUnstableBossTransitioningToNaturePhaseTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && !hydross->HasAura(SPELL_CORRUPTION) && botAI->IsMainTank(bot) && 
    (bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) || bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
    bot->HasAura(SPELL_MARK_OF_CORRUPTION_500));
}

bool HydrossTheUnstableBossInNaturePhaseTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && hydross->HasAura(SPELL_CORRUPTION) && botAI->IsAssistTankOfIndex(bot, 0) && 
    !bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) && !bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) &&
    !bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
}

bool HydrossTheUnstableBossTransitioningToFrostPhaseTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && hydross->HasAura(SPELL_CORRUPTION) && botAI->IsAssistTankOfIndex(bot, 0) && 
    (bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) || bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
    bot->HasAura(SPELL_MARK_OF_CORRUPTION_500));
}

bool HydrossTheUnstableWaitingForDPSTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && IsMapIDTimerManager(botAI, bot);
}

bool HydrossTheUnstableElementalsSpawnedTrigger::IsActive()
{
    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");

    return botAI->IsDps(bot) && (waterElemental || natureElemental);
}

bool HydrossTheUnstableDangerFromWaterTombsTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && !hydross->HasAura(SPELL_CORRUPTION) && botAI->IsRanged(bot);
}

// The Lurker Below

// Leotheras the Blind

// Fathom-Lord Karathress

// Morogrim Tidewalker
