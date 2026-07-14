/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraMultipliers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"

using namespace KarazhanHelpers;

float KarazhanSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return 0.0f;

    if (AI_VALUE2(Unit*, "find target", "spectral charger") ||
        AI_VALUE2(Unit*, "find target", "the big bad wolf"))
    {
        return 0.0f;
    }

    return 1.0f;
}

float AttumenTheHuntsmanDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AttumenTheHuntsmanStayStackedMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return 1.0f;

    if (!GetAttumenMounted(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<AttumenTheHuntsmanHandlePhaseTwoAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Give the main tank 5 seconds to grab aggro when Attumen mounts Midnight
float AttumenTheHuntsmanWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return 1.0f;

    Unit* attumen = GetAttumenMounted(bot);
    if (!attumen)
        return 1.0f;

    if (botAI->IsMainTank(bot))
        return 1.0f;

    uint32 const instanceId = attumen->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = attumenDpsWaitTimer.find(instanceId);
    if (it != attumenDpsWaitTimer.end() && (now - it->second) >= dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Disables co +disperse and co +tank face
float MaidenOfVirtueDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "maiden of virtue"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MaidenOfVirtueSetGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "maiden of virtue"))
        return 1.0f;

    if (dynamic_cast<CastWrathOfAirTotemAction*>(action) ||
        dynamic_cast<CastNatureResistanceTotemAction*>(action) ||
        dynamic_cast<CastWindfuryTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float TheCuratorDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "the curator"))
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Disables co +disperse and co +tank face
float TheCuratorDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "the curator"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Save Bloodlust/Heroism for Evocation (100% increased damage)
float TheCuratorDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator || curator->HasAura(
            static_cast<uint32>(KarazhanSpells::SPELL_CURATOR_EVOCATION)))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Don't charge back in or move in any other way when running from Arcane Explosion
float ShadeOfAranArcaneExplosionRunAwayMultiplier::GetValue(Action* action)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return 1.0f;

    if (!IsCastingArcaneExplosion(aran))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<ShadeOfAranRunAwayFromArcaneExplosionAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "shade of aran"))
        return 1.0f;

    if (!IsFlameWreathActive(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<ShadeOfAranStopMovingDuringFlameWreathAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float NetherspiteKeepBlockingBeamMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot == redBlocker &&
        dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 0.0f;
    }

    if (bot == blueBlocker &&
        (dynamic_cast<CombatFormationMoveAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action)))
    {
        return 0.0f;
    }

    if (bot == greenBlocker &&
        (dynamic_cast<CombatFormationMoveAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CastKillingSpreeAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Give tanks 5 seconds to get aggro during phase transitions
float NetherspiteWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    if (botAI->IsTank(bot))
        return 1.0f;

    uint32 const instanceId = netherspite->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = netherspiteDpsWaitTimer.find(instanceId);
    if (it != netherspiteDpsWaitTimer.end() &&
        (now - it->second) >= dpsWaitSeconds)
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

     return 1.0f;
}

// Don't run back into Shadow Nova when Enfeebled
float PrinceMalchezaarEnfeebleKeepDistanceMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "prince malchezaar"))
        return 1.0f;

    if (!bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_ENFEEBLE)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<PrinceMalchezaarEnfeebledAvoidHazardAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Wait until Phase 3 to use Bloodlust/Heroism
float PrinceMalchezaarDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar || malchezaar->GetHealthPct() <= 30.0f)
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Pets tend to run out of bounds and cause skeletons to spawn off the map
// Pets also tend to pull adds from inside of the tower through the floor
// This multiplier DOES NOT impact Hunter and Warlock pets
// Hunter and Warlock pets are addressed in ControlPetAggressionAction
float NightbaneDisablePetsMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastSummonWaterElementalAction*>(action) ||
        dynamic_cast<CastShadowfiendAction*>(action))
    {
        return 0.0f;
    }

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z &&
        dynamic_cast<PetAttackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Give the main tank 8 seconds to get aggro during phase transitions
float NightbaneWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    if (botAI->IsMainTank(bot))
        return 1.0f;

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 8;

    auto it = nightbaneDpsWaitTimer.find(instanceId);
    if (it != nightbaneDpsWaitTimer.end() && (now - it->second) >= dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// The "avoid aoe" strategy must be disabled for the main tank
// Otherwise, the main tank will spin Nightbane to avoid Charred Earth and wipe the raid
// It is also disabled for all bots during the flight phase
float NightbaneDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return 0.0f;

    if (botAI->IsMainTank(bot) || botAI->IsRanged(bot))
        return 0.0f;

    return 1.0f;
}

// Disable some movement actions that conflict with the strategies
float NightbaneDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z &&
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    constexpr uint8 flightPhaseDurationSeconds = 35;

    if (nightbaneFlightPhaseStartTimer.find(instanceId) ==
        nightbaneFlightPhaseStartTimer.end())
    {
        return 1.0f;
    }

    if (now - nightbaneFlightPhaseStartTimer[instanceId] >= flightPhaseDurationSeconds)
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}
