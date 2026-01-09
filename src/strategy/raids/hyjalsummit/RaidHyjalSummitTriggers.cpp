#include "RaidHyjalSummitTriggers.h"
#include "RaidHyjalSummitHelpers.h"
#include "RaidHyjalSummitActions.h"
#include "AiFactory.h"
#include "Playerbots.h"

using namespace HyjalSummitHelpers;

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
    if (!botAI->IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillBossCastsDeathAndDecayTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "rage winterchill");
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
    if (!botAI->IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) || bot->HasAura(SPELL_IMMOLATION))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    return !IsBotTargetedByInferno(anetheron, bot);
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (IsBotTargetedByInferno(anetheron, bot))
        return true;

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    return infernal && infernal->GetVictim() == bot;
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronInfernalsDespawnWhenBossDiesTrigger::IsActive()
{
    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "anetheron");
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
    if (!botAI->IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "kaz'rogal");
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

    return bot->GetPower(POWER_MANA) > 2000 ||
           (bot->GetPower(POWER_MANA) > 3000 && bot->HasAura(SPELL_MARK_OF_KAZROGAL));
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT ||
        (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL))
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    // Try to see if I can keep bots in for longer unless they have mark (if not, just move away at <=3000 mana)
    return bot->GetPower(POWER_MANA) <= 2000 ||
           (bot->GetPower(POWER_MANA) <= 3000 && bot->HasAura(SPELL_MARK_OF_KAZROGAL));
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
    if (!botAI->IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "azgalor");
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
    if (!botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    if (AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    return AI_VALUE2(Unit*, "find target", "lesser doomguard") ||
           AnyGroupMemberHasDoom(bot);
}

bool AzgalorDoomguardsContinueToSpawnTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "azgalor");
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

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool ArchimondeBossCastsAirBurstTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (bot->HasAura(SPELL_DOOMFIRE))
        return false;

    return AI_VALUE2(Unit*, "find target", "archimonde");
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
