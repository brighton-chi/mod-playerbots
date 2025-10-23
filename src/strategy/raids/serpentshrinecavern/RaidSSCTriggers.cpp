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

bool HydrossTheUnstableBotIsNatureTankTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && botAI->IsAssistTankOfIndex(bot, 0);
}

bool HydrossTheUnstableWaitingForDPSTrigger::IsActive()
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");

    return hydross && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0));
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

/* bool TheLurkerBelowSpoutIsActiveTrigger::IsActive()

bool TheLurkerBelowAmbushersAndGuardiansSpawnedTrigger::IsActive() */

// Leotheras the Blind

/* bool LeotherasTheBlindHumanFormEngagedByMainTankTrigger::IsActive()

bool LeotherasTheBlindDemonFormEngagedByWarlockTankTrigger::IsActive()

bool LeotherasTheBlindBossChannelingWhirlwindTrigger::IsActive()

bool LeotherasTheBlindDeterminingKillOrderTrigger::IsActive()

bool LeotherasTheBlindNeedToManageTimersTrigger::IsActive() */

// Fathom-Lord Karathress

/* bool FathomLordKarathressBossEngagedByMainTankTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");

    return karathress && karathress->IsAlive() >= 25 &&
           botAI->IsMainTank(bot);
}

bool FathomLordKarathressSharkkisEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");

    return sharkkis && sharkkis->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 0);
}

bool FathomLordKarathressTidalvessEngagedBySecondAssistTankTrigger::IsActive()
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");

    return tidalvess && tidalvess->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 1);
}

bool FathomLordKarathressCaribdisEngagedByThirdAssistTankTrigger::IsActive()
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    return caribdis && caribdis->IsAlive() &&
           botAI->IsAssistTankOfIndex(bot, 2);
}

bool FathomLordKarathressCaribdisTankNeedsHealerTrigger::IsActive()

bool FathomLordKarathressPullingBossTrigger::IsActive()
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");

    return karathress && karathress->GetHealthPct() > 98 && bot->getClass() == CLASS_HUNTER;
}

bool FathomLordKarathressDeterminingMeleeDPSKillOrderTrigger::IsActive()

bool FathomLordKarathressDeterminingRangedDPSKillOrderTrigger::IsActive() */

// Morogrim Tidewalker

bool MorogrimTidewalkerPullingBossTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && tidewalker->GetHealthPct() > 95 && bot->getClass() == CLASS_HUNTER;
}


bool MorogrimTidewalkerBossEngagedByMainTankTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && botAI->IsMainTank(bot);
}

bool MorogrimTidewalkerWaterGlobulesAreIncomingTrigger::IsActive()
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");

    return tidewalker && tidewalker->GetHealthPct() < 25 &&
           !botAI->IsTank(bot);
}
