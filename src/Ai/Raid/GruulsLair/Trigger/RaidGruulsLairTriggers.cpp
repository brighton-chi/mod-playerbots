#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarIsMainTankTrigger::IsActive()
{
    return botAI->IsTank(bot) && botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarIsFirstAssistTankTrigger::IsActive()
{
    return botAI->IsTank(bot) && botAI->IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "olm the summoner");
}

bool HighKingMaulgarIsSecondAssistTankTrigger::IsActive()
{
    return botAI->IsTank(bot) && botAI->IsAssistTankOfIndex(bot, 1, false) &&
           AI_VALUE2(Unit*, "find target", "blindeye the seer");
}

bool HighKingMaulgarIsMageTankTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && IsKroshMageTank(botAI) &&
           AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarIsMoonkinTankTrigger::IsActive()
{
    return bot->getClass() == CLASS_DRUID && IsKigglerMoonkinTank(botAI) &&
           AI_VALUE2(Unit*, "find target", "kiggler the crazed");;
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return (botAI->IsDps(bot) || botAI->IsTank(bot)) &&
           !(botAI->IsMainTank(bot) && maulgar) &&
           !(botAI->IsAssistTankOfIndex(bot, 0, false) && olm) &&
           !(botAI->IsAssistTankOfIndex(bot, 1, false) && blindeye) &&
           !(IsKroshMageTank(bot) && krosh) &&
           !(IsKigglerMoonkinTank(bot) && kiggler);
}

bool HighKingMaulgarHealerInDangerTrigger::IsActive()
{
    return botAI->IsHeal(bot) && IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    if (botAI->IsTank(bot) && botAI->IsMainTank(bot))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    return maulgar && maulgar->HasAura(SPELL_WHIRLWIND);
}

bool HighKingMaulgarWildFelstalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK &&
           AI_VALUE2(Unit*, "find target", "wild fel stalker");
}

bool HighKingMaulgarPullingOlmAndBlindeyeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm && olm->GetHealthPct() > 98.0f)
        return true;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye && blindeye->GetHealthPct() > 90.0f)
        return true;

    return false;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerBossEngagedByRangedTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return bot->HasAura(SPELL_GROUND_SLAM_1) ||
           bot->HasAura(SPELL_GROUND_SLAM_2);
}
