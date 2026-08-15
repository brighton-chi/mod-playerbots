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

bool RageWinterchillRangedIsStandingInDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
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

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    Unit* infernal = GetFocusedInfernal(botAI);
    if (infernal && anetheron->GetHealthPct() > 10.0f &&
        bot->GetDistance2d(infernal) < 50.0f)
    {
        return false;
    }

    return true;
}

// Whoever is holding Anetheron stays put: walking him across the platform costs the raid more than
// a two second stun costs one bot. The Inferno target itself is excluded because it has its own job
// -- carrying the summon to the gathering spot -- and nothing it does avoids a stun centred on it
bool AnetheronBotIsNearInfernoTargetTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return bot->GetExactDist2d(infernoTarget) < INFERNAL_ESCAPE_DISTANCE;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return true;

    if (IsInfernalTank(bot))
        return false;

    return GetInfernalTargetingBot(botAI, bot);
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    if (!IsInfernalTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        return false;

    Unit* infernal = GetInfernalTargetingBot(botAI, bot);
    return infernal && bot->IsWithinMeleeRange(infernal);
}

bool AnetheronShouldDetermineDpsPriorityTrigger::IsActive()
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

    // An assist tank only steps in front while it could still eat a full Mark
    return bot->GetPower(POWER_MANA) > MARK_SURVIVAL_MANA;
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
    else if (bot->GetPower(POWER_MANA) > MARK_RECOVERED_MANA)
    {
        isBelowManaThreshold.erase(bot->GetGUID());
        if (PlayerbotAI::IsMelee(bot))
            return false;
        else
            return true;
    }

    return false;
}

bool KazrogalBotShouldPreserveManaTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->GetPower(POWER_MANA) >= MARK_RECOVERED_MANA)
        return false;

    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            return !bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER));

        // Life Tap is priced in health, so below the threshold the AI life taps at it stops being
        // available and Shadow Ward takes over
        case CLASS_WARLOCK:
            return bot->GetHealthPct() > sPlayerbotAIConfig.lowHealth;

        default:
            return false;
    }
}

// Spending Ice Block or Divine Shield costs a cooldown longer than the pull, so it waits until the
// Mark is certain to detonate and running can no longer open enough ground before it does
bool KazrogalBotMustCancelMarkTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_DIVINE_SHIELD)) ||
        bot->HasAura(Id(HyjalSpells::SPELL_ICE_BLOCK)))
    {
        return false;
    }

    if (!HasMarkOfKazrogal(bot))
        return false;

    if (bot->GetPower(POWER_MANA) >= MARK_SURVIVAL_MANA)
        return false;

    return !CanOutrunMarkDetonation(bot);
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

    // The latch belongs here rather than in the action it drives: the movement multiplier reads it
    // too, and a flag two files depend on should not be a side effect of one of them running
    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
    {
        isBelowManaThreshold.try_emplace(bot->GetGUID(), true);
        return true;
    }

    return isBelowManaThreshold.count(bot->GetGUID()) > 0;
}

// Below the survival line with Life Tap priced out of reach: the detonation is coming and there is
// nothing left but to soften it
bool KazrogalMarkDealsShadowDamageTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!HasMarkOfKazrogal(bot))
        return false;

    return bot->GetPower(POWER_MANA) < MARK_SURVIVAL_MANA &&
        bot->GetHealthPct() <= sPlayerbotAIConfig.lowHealth;
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

    if (bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return false;

    constexpr float suppressionRadius = RAIN_OF_FIRE_RADIUS + 10.0f;
    return !IsNearRainOfFire(bot, suppressionRadius);
}

bool AzgalorMeleeIsStandingInRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_DOOM)))
        return false;

    return IsInRainOfFire(bot);
}

bool AzgalorRangedIsStandingInRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
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
    if (!PlayerbotAI::IsTank(bot))
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

bool AzgalorMeleeAndRangedShouldDivideDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
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
