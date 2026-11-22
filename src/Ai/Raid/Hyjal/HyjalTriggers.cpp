/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalTriggers.h"
#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Playerbots.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// Shared

bool HyjalNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, HYJAL_MAP_ID);
}

bool HyjalHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetEncounterBoss();
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool HyjalBossShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (_mainTankOnly && !PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* boss = GetEncounterBoss();
    return boss && boss->GetHealthPct() > _activeAboveHealthPct;
}

// Rage Winterchill

bool RageWinterchillRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

bool RageWinterchillMeleeNearDeathAndDecayTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* winterchill = GetEncounterBoss();
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS);
}

bool RageWinterchillRangedInDeathAndDecayTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    return IsInDeathAndDecay(botAI);
}

// Anetheron

bool AnetheronHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER;
}

bool AnetheronRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* anetheron = GetEncounterBoss();
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    return !GetInfernalToAttack(botAI, anetheron);
}

bool AnetheronNearInfernoTargetTrigger::IsActiveInEncounter()
{
    Unit* anetheron = GetEncounterBoss();
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return bot->GetExactDist2d(infernoTarget) < INFERNAL_ESCAPE_DISTANCE;
}

bool AnetheronTargetedByInfernalTrigger::IsActiveInEncounter()
{
    Unit* anetheron = GetEncounterBoss();
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return true;

    if (IsInfernalTank(bot))
        return false;

    return GetInfernalTargetingBot(botAI);
}

bool AnetheronInfernalsPulseImmolationTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal || infernal->GetVictim() == bot)
        return false;

    return bot->GetExactDist2d(infernal) < INFERNAL_DANGER_RADIUS;
}

bool AnetheronInfernalsShouldBeTankedAwayTrigger::IsActiveInEncounter()
{
    if (!IsInfernalTank(bot))
        return false;

    Unit* infernal = GetInfernalTargetingBot(botAI);
    return infernal && bot->IsWithinMeleeRange(infernal);
}

bool AnetheronShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot);
}

// Kaz'rogal

bool KazrogalCanSplitMalevolentCleaveDamageTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    if (bot->getClass() != CLASS_PALADIN)
        return true;

    return !AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalRangedShouldAvoidWarStompTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && !AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalLowOnManaTrigger::IsActiveInEncounter()
{
    if (!IsKazrogalManaUser(botAI))
        return false;

    // Hunters and Warlocks never run away. They rely only on Aspect of the Viper and Life Tap/
    // Shadow Ward, respectively.
    if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_WARLOCK)
        return false;

    Unit* kazrogal = GetEncounterBoss();
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
    {
        SET_AI_VALUE(bool, "kaz'rogal below mana threshold", true);
        return true;
    }

    return AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalHunterShouldPreserveManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (bot->HasAura(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER)))
        return false;

    // Eligible to switch back at MARK_REJOIN_MANA, per the multiplier.
    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA;
}

bool KazrogalMarkOnMageOrPaladinTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* kazrogal = GetEncounterBoss();
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    Aura const* mark = bot->GetAura(Id(HyjalSpells::SPELL_MARK_OF_KAZROGAL));
    if (!mark)
        return false;

    uint32 const mana = bot->GetPower(POWER_MANA);
    if (mana >= MARK_FULL_DRAIN)
        return false;

    // Blowing Ice Block/Divine Shield is worth it only where the Mark outlasts mana.
    //   2400-2999  needs 5s left      1200-1799  needs 3s left      0-599  needs 1s left
    //   1800-2399  needs 4s left       600-1199  needs 2s left
    int32 const requiredMs = static_cast<int32>((mana / MARK_TICK_DRAIN + 1) * IN_MILLISECONDS);
    return mark->GetDuration() >= requiredMs;
}

bool KazrogalWarlockShouldManageManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA)
        return true;

    if (!HasMarkOfKazrogal(bot) || botAI->HasAura("shadow ward", bot))
        return false;

    return bot->GetPower(POWER_MANA) <= MARK_TICK_DRAIN;
}

bool KazrogalImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    if (HasMarkOfKazrogal(bot))
        return false;

    // 50% is a proxy for the bot potentially being in range of getting blown up by other bots,
    // so don't wipe the immunity if below that HP.
    constexpr float keepImmunityHealthPct = 50.0f;
    return bot->GetHealthPct() > keepImmunityHealthPct;
}

// Azgalor

bool AzgalorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* azgalor = GetEncounterBoss();
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    return !IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorMeleeNearRainOfFireTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* azgalor = GetEncounterBoss();
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    // The Doomguard tank is excluded due to needing to hold at the Doomguard tanking position.
    // This isn't ideal, but special avoidance of a not-that-dangerous ability for one role that
    // needs specific positioning is not worth the time and effort.
    if (IsDoomguardTank(bot))
        return false;

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorRangedInRainOfFireTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (IsDoomed(bot))
        return false;

    return IsInRainOfFire(botAI);
}

bool AzgalorBotIsDoomedTrigger::IsActiveInEncounter()
{
    return IsDoomed(bot);
}

bool AzgalorShouldControlDoomguardsTrigger::IsActiveInEncounter()
{
    if (!IsDoomguardTank(bot))
        return false;

    if (GetLesserDoomguard(botAI))
        return true;

    return AnyGroupMemberHasDoom(bot);
}

bool AzgalorShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot);
}

// Archimonde

bool ArchimondeShamanShouldProtectAgainstFearTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = GetEncounterBoss();
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return false;

    return !HasProtectionOfElune(bot);
}

bool ArchimondeCastingAirBurstTrigger::IsActiveInEncounter()
{
    Unit* archimonde = GetEncounterBoss();
    if (!archimonde || archimonde->GetVictim() == bot)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    AirBurstData airBurst;
    return GetPendingAirBurstCast(bot->GetInstanceId(), airBurst);
}

bool ArchimondeRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && !HasProtectionOfElune(bot);
}

bool ArchimondeNearDoomfireTrigger::IsActiveInEncounter()
{
    if (HasProtectionOfElune(bot))
        return false;

    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS);
}

bool ArchimondeStoodInDoomfireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
    {
        return false;
    }

    if (HasProtectionOfElune(bot))
        return false;

    if (!bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE)) &&
        !bot->HasAura(Id(HyjalSpells::SPELL_DOOMFIRE_DOT)))
    {
        return false;
    }

    constexpr float dangerHealthPct = 40.0f;
    return bot->GetHealthPct() < dangerHealthPct;
}
