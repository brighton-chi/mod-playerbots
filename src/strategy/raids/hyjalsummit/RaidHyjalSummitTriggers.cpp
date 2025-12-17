#include "RaidHyjalSummitTriggers.h"
#include "RaidHyjalSummitHelpers.h"
#include "RaidHyjalSummitActions.h"
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

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    return winterchill != nullptr;
}

bool RageWinterchillBossCastsDeathAndDecayTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    return winterchill != nullptr;
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

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    return anetheron != nullptr;
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) || bot->HasAura(SPELL_IMMOLATION))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (IsBotTargetedByInferno(anetheron, bot))
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

    // Debug logging for inferno targeting
    Spell* spell = anetheron->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (spell)
    {
        LOG_DEBUG("playerbots", "Anetheron current spell id: {}", spell->m_spellInfo->Id);
        if (spell->m_spellInfo->Id == SPELL_INFERNO)
        {
            Unit* spellTarget = spell->m_targets.GetUnitTarget();
            if (spellTarget)
            {
                LOG_DEBUG("playerbots", "Inferno spell target guid: {} bot guid: {}", spellTarget->GetGUID().ToString(), bot->GetGUID().ToString());
            }
            else
            {
                LOG_DEBUG("playerbots", "Inferno spell target is nullptr");
            }
        }
    }
    else
    {
        LOG_DEBUG("playerbots", "Anetheron has no current spell");
    }

    if (IsBotTargetedByInferno(anetheron, bot))
        return true;

    Unit* infernal = AI_VALUE2(Unit*, "find target", "towering infernal");
    if (infernal && infernal->GetVictim() == bot)
        return true;

    return false;
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    return anetheron != nullptr;
}

bool AnetheronInfernalsDespawnWhenBossDiesTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    return anetheron != nullptr;
}

// Kaz'rogal

// Azgalor

// Archimonde

