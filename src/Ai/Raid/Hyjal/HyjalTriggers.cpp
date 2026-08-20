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

bool HyjalPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool HyjalBossEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > _activeAboveHealthPct;
}

// Rage Winterchill

bool RageWinterchillRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillMeleeNearDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_MELEE_CONTROL_RADIUS);
}

bool RageWinterchillRangedIsStandingInDeathAndDecayTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return false;

    return IsInDeathAndDecay(botAI);
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER && AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronRangedShouldSpreadTrigger::IsActive()
{
    if (PlayerbotAI::IsMelee(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    Unit* infernal = GetFocusedInfernal(botAI);
    if (infernal && anetheron->GetHealthPct() > 10.0f && bot->GetDistance2d(infernal) < 50.0f)
        return false;

    return true;
}

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

    return GetInfernalTargetingBot(bot);
}

bool AnetheronInfernalsShouldBeKeptAwayTrigger::IsActive()
{
    if (!IsInfernalTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "anetheron"))
        return false;

    Unit* infernal = GetInfernalTargetingBot(bot);
    return infernal && bot->IsWithinMeleeRange(infernal);
}

bool AnetheronShouldDetermineDpsPriorityTrigger::IsActive()
{
    return !PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "anetheron");
}

// Kaz'rogal

bool KazrogalMalevolentCleaveSplitsDamageTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->getClass() != CLASS_PALADIN)
        return true;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalLowManaBotsNeedEscapePathTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!IsKazrogalManaUser(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (!IsKazrogalManaUser(bot))
        return false;

    // Hunters never run away. They rely only on Aspect of the Viper.
    if (bot->getClass() == CLASS_HUNTER)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
    {
        botsBelowManaThreshold.insert(bot->GetGUID());
        return true;
    }

    return botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalHunterShouldPreserveManaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER)))
        return false;

    // Activate at 3200 mana; switch back based on normal Hunter aspect strategies
    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA;
}

bool KazrogalMarkOnMageOrPaladinTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    Aura* aura = bot->GetAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
    if (!aura)
        return false;

    uint32 const mana = bot->GetPower(POWER_MANA);
    constexpr float markFullyDrainedMana = 3000.0f;
    if (mana >= markFullyDrainedMana)
        return false;

    // Blowing Ice Block/Divine Shield is worth it only where the Mark outlasts mana.
    //   2401-3000  needs 4s left      1201-1800  needs 2s left      0-600  cast regardless
    //   1801-2400  needs 3s left       601-1200  needs 1s left
    uint32 const tickDrain = static_cast<uint32>(MARK_TICK_DRAIN);
    int32 const requiredMs =
        (static_cast<int32>((mana + tickDrain - 1) / tickDrain) - 1) * IN_MILLISECONDS;

    return requiredMs <= 0 || aura->GetDuration() >= requiredMs;
}

bool KazrogalWarlockShouldManageManaTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA &&
        bot->GetHealthPct() > sPlayerbotAIConfig.lowHealth)
    {
        return true;
    }

    if (!HasMarkOfKazrogal(bot) || botAI->HasAura("shadow ward", bot))
        return false;

    return bot->GetPower(POWER_MANA) <= MARK_TICK_DRAIN;
}

// Azgalor

bool AzgalorBossEngagedByRangedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    return !IsNearRainOfFire(botAI, RAIN_OF_FIRE_RANGED_CONTROL_RADIUS);
}

bool AzgalorMeleeNearRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    // The Doomguard tank is excluded due to needing to hold at the Doomguard tanking position.
    // This isn't ideal, but special avoidance for one role that needs specific positioning would
    // be very difficult, and it's not hard to heal through the damage for one bot.
    if (IsDoomguardTank(bot))
        return false;

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_MELEE_CONTROL_RADIUS);
}

bool AzgalorRangedIsStandingInRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    if (IsDoomed(bot))
        return false;

    return IsInRainOfFire(botAI);
}

bool AzgalorBotIsDoomedTrigger::IsActive()
{
    return IsDoomed(bot);
}

bool AzgalorDoomguardsMustBeControlledTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    if (!IsDoomguardTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "lesser doomguard") || AnyGroupMemberHasDoom(bot);
}

bool AzgalorShouldDivideDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
}

// Archimonde

bool ArchimondeBossCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() > 90.0f) // Wait for initial positioning
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBossCastingAirBurstTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetVictim() == bot)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    return GetPendingAirBurstCast(bot->GetMap()->GetInstanceId());
}

bool ArchimondeRangedShouldSpreadTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeBotIsNearDoomfireTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS);
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (HasProtectionOfElune(bot))
        return false;

    return bot->GetHealthPct() < 40.0f &&
        (bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE)) ||
         bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE_DOT)));
}
