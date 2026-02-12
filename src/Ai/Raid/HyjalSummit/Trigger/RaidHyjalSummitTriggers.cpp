#include "RaidHyjalSummitTriggers.h"
#include "RaidHyjalSummitHelpers.h"
#include "RaidHyjalSummitActions.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitRangedBotIsNotInCombatTrigger::IsActive()
{
    // For Hyjal, the only maps are for ranged bot positioning
    return botAI->IsRanged(bot) && !bot->IsInCombat();
}

// Rage Winterchill

bool RageWinterchillPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    return winterchill && winterchill->GetHealthPct() > 95.0f;
}

bool RageWinterchillBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillBossCastsDeathAndDecayTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "rage winterchill");
}

// Anetheron

bool AnetheronPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    return anetheron && anetheron->GetHealthPct() > 95.0f;
}

bool AnetheronBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (infernal || IsBotTargetedByInferno(anetheron, bot))
        return false;

    return true;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    return IsBotTargetedByInferno(anetheron, bot);
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    return botAI->IsAssistTank(bot) &&
           AI_VALUE2(Unit*, "find target", "towering infernal");
}

bool AnetheronInfernalsContinueToSpawnTrigger::IsActive()
{
    return botAI->IsDps(bot) &&
           AI_VALUE2(Unit*, "find target", "anetheron");
}

// Kaz'rogal

bool KazrogalPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    return kazrogal && kazrogal->GetHealthPct() > 95.0f;
}

bool KazrogalBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "kaz'rogal");
}

bool KazrogalBossEngagedByAssistTanksTrigger::IsActive()
{
    if (!botAI->IsAssistTank(bot))
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    return bot->GetPower(POWER_MANA) > 3000;
}

bool KazrogalLowManaBotsNeedEscapePathTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->getClass() == CLASS_HUNTER)
        return true;
    else if (bot->GetPower(POWER_MANA) > 3000)
        return true;

    return false;
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return false;

    if (bot->getClass() == CLASS_HUNTER && bot->GetPower(POWER_MANA) <= 6000)
        return true;
    else if (bot->GetPower(POWER_MANA) <= 3000)
        return true;

    return false;
}

bool KazrogalMageOrPaladinHasMarkOfKazrogalTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    return bot->HasAura(SPELL_MARK_OF_KAZROGAL);
}

// Azgalor

bool AzgalorPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    return azgalor && azgalor->GetHealthPct() > 95.0f;
}

bool AzgalorBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "azgalor");
}

// Spread to mitigate Rain of Fire, but GTFO if Rain of Fire is on the bot
bool AzgalorBossCastsRainOfFireTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (bot->HasAura(SPELL_RAIN_OF_FIRE) || bot->HasAura(SPELL_DOOM))
        return false;

    return AI_VALUE2(Unit*, "find target", "azgalor");
}

bool AzgalorBotIsDoomedTrigger::IsActive()
{
    return bot->HasAura(SPELL_DOOM);
}

// Lol did this fight without this trigger--maybe should just ignore doomguards?
bool AzgalorDoomguardsMustBeControlledTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "lesser doomguard") ||
           AnyGroupMemberHasDoom(bot);
}

bool AzgalorDoomguardsContinueToSpawnTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
}

// Archimonde

bool ArchimondePullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 95.0f;
}

bool ArchimondeBossCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool ArchimondeBossCastsAirBurstTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetVictim() != bot;
}

bool ArchimondeBossSummonedDoomfireTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "archimonde");
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    return bot->HasAura(SPELL_DOOMFIRE);
}
