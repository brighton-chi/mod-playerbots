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

    // Reaches as far as the suppression that accompanies it, not just to the pool. Everything
    // inside that radius has had its path back to the boss zeroed, so this action has to keep
    // running across the whole of it--it is the only thing left that can walk the bot anywhere,
    // including back onto Winterchill once the pool no longer blocks the ring
    return IsNearDeathAndDecay(bot, DEATH_AND_DECAY_MELEE_CONTROL_RADIUS);
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

bool AnetheronInfernalsShouldBeKeptAwayTrigger::IsActive()
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
    // Hunters are wanted here even though they never run: this is what puts ranged on the arc, and
    // a hunter that has fallen back on Viper is still one of the ranged standing in it
    if (!IsKazrogalManaUser(botAI, bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    return !botsBelowManaThreshold.contains(bot->GetGUID());
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (!IsKazrogalManaUser(botAI, bot))
        return false;

    // Hunters never run. They rely only on Aspect of the Viper.
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

    // Activate at 3200 mana; switch back based on normal Hunter strategy
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

    constexpr float suppressionRadius = RAIN_OF_FIRE_RADIUS + 10.0f;
    return !IsNearRainOfFire(bot, suppressionRadius);
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

    // The Doomguard tank keeps its corner. This action walks bots onto Azgalor's melee ring, which
    // for that one would haul the Doomguard into the raid behind it--and at ACTION_EMERGENCY it
    // outranks the positioning that would walk it back, so it would not return until the pool died.
    //
    // That leaves the Doomguard tank with no Rain of Fire avoidance at all: stock avoid-aoe is off
    // for the whole fight and the ranged escape does not apply to it either. Deliberate, not an
    // oversight--the corner is a fixed position and shifting it far enough to clear a pool costs
    // more than it saves. One tank standing in fire is a healing problem; a loose Doomguard is not
    if (IsDoomguardTank(botAI, bot))
        return false;

    // Reaches as far as the suppression that accompanies it, not just to the fire. Everything
    // inside that radius has had its other movement zeroed, so this action has to keep running
    // across the whole of it--it is the only thing left that can walk the bot anywhere, including
    // back onto Azgalor once the tank has dragged him clear of the pool
    return IsNearRainOfFire(bot, RAIN_OF_FIRE_MELEE_CONTROL_RADIUS);
}

bool AzgalorRangedIsStandingInRainOfFireTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return false;

    if (IsDoomed(bot))
        return false;

    return IsInRainOfFire(bot);
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

    if (!IsDoomguardTank(botAI, bot))
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

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return GetPendingAirBurstCast(bot->GetMap()->GetInstanceId());
}

// No longer gated to the opening. Ranged drift back together across a fight this long, and the
// spread is cheap: the action rate limits itself and the Doomfire multiplier removes it near a
// trail, so leaving it live costs nothing where it would otherwise get in the way
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

    // The same radius the multiplier suppresses at, so the action owns movement across exactly the
    // area cleared for it. Nothing it does reaches further: the push only has a magnitude inside
    // DOOMFIRE_DANGER_RADIUS, the trapped sweep needs a patch inside DOOMFIRE_BURN_RADIUS, and past
    // this ordinary movement is free again and closes the gap to Archimonde perfectly well. Gating
    // here rather than in the action is what keeps the 18y field sweep off every bot on every tick
    // of a fight where a trail is usually nowhere near them
    return IsNearDoomfire(bot, DOOMFIRE_CONTROL_RADIUS);
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
