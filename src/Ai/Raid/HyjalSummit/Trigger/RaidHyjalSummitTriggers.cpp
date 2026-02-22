#include "RaidHyjalSummitTriggers.h"
#include "RaidHyjalSummitHelpers.h"
#include "RaidHyjalSummitActions.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
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
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return false;

    return botAI->IsMainTank(bot);
}

bool RageWinterchillBossCastsDeathAndDecayTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "rage winterchill");
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    return AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        return false;

    return botAI->IsMainTank(bot);
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    return true;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (botAI->IsTank(bot) && botAI->IsMainTank(bot))
        return false;

    return GetInfernoTarget(anetheron) == bot;
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "towering infernal"))
        return false;

    return botAI->IsAssistTankOfIndex(bot, 0, true);
}

bool AnetheronInfernalsContinueToSpawnTrigger::IsActive()
{
    return !botAI->IsTank(bot) &&
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
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    return botAI->IsMainTank(bot);
}

bool KazrogalBossEngagedByAssistTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || !botAI->IsAssistTank(bot))
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    return bot->GetPower(POWER_MANA) > 3000;
}

bool KazrogalLowManaBotsNeedEscapePathTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
        return false;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        return true;
    }
    else if (bot->GetPower(POWER_MANA) > 4000)
    {
        isBelowManaThreshold.erase(bot->GetGUID());
        if (botAI->IsMelee(bot))
            return false;
        else
            return true;
    }

    return false;
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
        return false;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return false;

    if (botAI->HasAnyAuraOf(bot, "ice block", "divine shield", nullptr))
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    if (isBelowManaThreshold.count(bot->GetGUID()) ||
        bot->GetPower(POWER_MANA) <= 3000)
        return true;

    return false;
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
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    return botAI->IsMainTank(bot);
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

bool AzgalorDoomguardsMustBeControlledTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "lesser doomguard") &&
        !AnyGroupMemberHasDoom(bot))
        return false;

    // Expensive role check last
    return botAI->IsAssistTankOfIndex(bot, 0, true);
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
    if (!archimonde || archimonde->GetHealthPct() <= 10.0f)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool ArchimondeBossCastsAirBurstTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() <= 10.0f ||
        archimonde->GetVictim() == bot)
        return false;

    return !(botAI->IsTank(bot) && botAI->IsMainTank(bot));
}

bool ArchimondeBossSummonedDoomfireTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 10.0f;
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    return bot->HasAura(SPELL_DOOMFIRE) && bot->GetHealthPct() < 40.0f;
}
