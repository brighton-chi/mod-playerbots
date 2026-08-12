/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalTriggers.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalHelpers;

// General

bool HyjalSummitBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == HYJAL_MAP_ID && !AI_VALUE2(bool, "combat", "self target");
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
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillBossCastsDeathAndDecayOnRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillMeleeIsStandingInDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return IsInDeathAndDecay(bot);
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER && AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossEngagedByMainTankTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (PlayerbotAI::IsMelee(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    return GetInfernoTarget(anetheron) != bot;
}

// Whoever an Infernal is on has to walk it to the gathering spot, since it cannot be taunted off
// them. That is the Inferno target during the cast, and its victim from then on. The two who stay
// put are the Infernal tank, which is already there, and whoever is holding Anetheron
bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    if (IsInfernalTank(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI, bot);
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    return IsInfernalTank(bot) && !GetInfernalGuids(botAI).empty();
}

bool AnetheronInfernalsContinueToSpawnTrigger::IsActive()
{
    return !PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "anetheron");
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
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "kaz'rogal");
}

bool KazrogalBossEngagedByAssistTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    return bot->GetPower(POWER_MANA) > 3000;
}

bool KazrogalLowManaBotsNeedEscapePathTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        return false;
    }

    if (bot->getClass() == CLASS_DRUID &&
        (botAI->HasStrategy("bear", BOT_STATE_COMBAT) ||
         botAI->HasStrategy("cat", BOT_STATE_COMBAT)))
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        return true;
    }
    else if (bot->GetPower(POWER_MANA) > 4000)
    {
        isBelowManaThreshold.erase(bot->GetGUID());
        if (PlayerbotAI::IsMelee(bot))
            return false;
        else
            return true;
    }

    return false;
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        return false;
    }

    if (bot->getClass() == CLASS_DRUID &&
        (botAI->HasStrategy("bear", BOT_STATE_COMBAT) ||
         botAI->HasStrategy("cat", BOT_STATE_COMBAT)))
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_DIVINE_SHIELD)) ||
        bot->HasAura(Id(HyjalSpells::SPELL_ICE_BLOCK)))
    {
        return false;
    }

    if (isBelowManaThreshold.count(bot->GetGUID()) || bot->GetPower(POWER_MANA) <= 3200)
        return true;

    return false;
}

bool KazrogalMarkDealsShadowDamageTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->getClass() == CLASS_PALADIN &&
        (botAI->HasAura("shadow resistance aura", bot) ||
         botAI->HasAura("prayer of shadow protection", bot) ||
         botAI->HasAura("shadow protection", bot)))
    {
        return false;
    }

    return bot->HasAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
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
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
}

bool AzgalorMainTankIsPositioningBossTrigger::IsActive()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !GET_PLAYERBOT_AI(mainTank) || PlayerbotAI::IsMainTank(bot))
        return false;

    TankPositionState tankState = GetAzgalorTankPositionState(botAI, bot);
    return tankState == TankPositionState::Unknown ||
        tankState == TankPositionState::MovingToTransition;
}

bool AzgalorBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    return !bot->HasAura(Id(HyjalSpells::SPELL_DOOM));
}

bool AzgalorBossCastsRainOfFireOnMeleeTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot)) // MT only?
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return false;

    return IsInRainOfFire(bot);
}

bool AzgalorBotIsDoomedTrigger::IsActive()
{
    return bot->HasAura(Id(HyjalSpells::SPELL_DOOM));
}

bool AzgalorDoomguardsMustBeControlledTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return AI_VALUE2(Unit*, "find target", "lesser doomguard") ||
            AnyGroupMemberHasDoom(bot);
    }

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        // Trigger for second assist tank only if first assist tank has Doom
        Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
        if (firstAssistTank && !firstAssistTank->HasAura(Id(HyjalSpells::SPELL_DOOM)))
            return false;

        return AI_VALUE2(Unit*, "find target", "lesser doomguard") || AnyGroupMemberHasDoom(bot);
    }

    return false;
}

bool AzgalorDoomguardsMustDieTrigger::IsActive()
{
    return PlayerbotAI::IsRangedDps(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
}

// Archimonde

bool ArchimondePullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 95.0f;
}

bool ArchimondeBossEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 95.0f;
}

bool ArchimondeBossCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    return archimonde->GetHealthPct() < 90.0f && archimonde->GetHealthPct() > 10.0f;
}

bool ArchimondeBossCastsAirBurstTrigger::IsActive()
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() <= 10.0f || archimonde->GetVictim() == bot)
        return false;

    return GetPendingAirBurstCast(bot->GetMap()->GetInstanceId()) != nullptr;
}

bool ArchimondeBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 90.0f;
}

bool ArchimondeBossSummonedDoomfireTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 10.0f;
    // if (!archimonde || archimonde->GetHealthPct() <= 10.0f)
    //    return false;

    // If I don't make an exception, bots refuse to enter the Doomfire even when feared
    // return !bot->HasAura(Id(HyjalSpells::SPELL_ARCHIMONDE_FEAR));
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    return bot->GetHealthPct() < 40.0f &&
        (bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE)) ||
         bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE_DOT)));
}
