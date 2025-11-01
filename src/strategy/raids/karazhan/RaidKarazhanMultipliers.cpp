#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

using namespace KarazhanHelpers;

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float AttumenTheHuntsmanDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!midnight && !attumen)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AttumenTheHuntsmanStayStackedMultiplier::GetValue(Action* action)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted || !attumenMounted->IsAlive())
        return 1.0f;

    if (!botAI->IsMainTank(bot) && attumenMounted->GetVictim() != bot &&
        (dynamic_cast<CombatFormationMoveAction*>(action) ||
         dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CastBlinkBackAction*>(action) ||
         dynamic_cast<CastDisengageAction*>(action) ||
         IsChargeAction(action)))
        return 0.0f;

    return 1.0f;
}

// Give the main tank 8 seconds to grab aggro when Attumen mounts Midnight
// In reality it's a lot shorter because it takes Attumen a few seconds to aggro after mounting
float AttumenTheHuntsmanWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted || !attumenMounted->IsAlive())
        return 1.0f;

    const uint8 dpsWaitSeconds = 8;
    auto it = attumenDPSWaitTimer.find(bot->GetMapId());
    if (it == attumenDPSWaitTimer.end() || (time(nullptr) - it->second) < dpsWaitSeconds)
    {
        if (!botAI->IsMainTank(bot) && (dynamic_cast<AttackAction*>(action) ||
            (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
            return 0.0f;
    }

    return 1.0f;
}

// The assist tank should stay on the boss to be 2nd on aggro and eat Hateful Bolts
float TheCuratorDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Don't charge back in when running from Arcane Explosion
float ShadeOfAranArcaneExplosionDisableChargeMultiplier::GetValue(Action* action)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return 1.0f;

    if (aran->HasUnitState(UNIT_STATE_CASTING) &&
        aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION))
    {
        if (IsChargeAction(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action))
        {
            const float safeDistance = 20.0f;
            if (bot->GetDistance2d(aran) >= safeDistance)
                return 0.0f;
        }
    }

    return 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    Group* group = bot->GetGroup();
    if (!aran || !group)
        return 1.0f;

    bool flameWreathActive = aran->HasAura(SPELL_FLAME_WREATH_CAST);

    if (!flameWreathActive)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura(SPELL_FLAME_WREATH_AURA))
            {
                flameWreathActive = true;
                break;
            }
        }
    }

    if (flameWreathActive)
    {
        if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
            return 0.0f;
    }

    return 1.0f;
}

// Try to rid of the jittering when blocking beams
float NetherspiteKeepBlockingBeamMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);

    if (bot == redBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    if (bot == blueBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    if (bot == greenBlocker)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            IsChargeAction(action))
            return 0.0f;
    }

    return 1.0f;
}

// I'm not sure this is working properly, but I haven't had problems with results
float NetherspiteWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return 1.0f;

    const uint8 dpsWaitSeconds = 5;
    auto it = netherspiteDPSWaitTimer.find(bot->GetMapId());
    if (it != netherspiteDPSWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < dpsWaitSeconds)
        {
            if (!botAI->IsTank(bot) && (dynamic_cast<AttackAction*>(action) ||
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

     return 1.0f;
}

// Disable standard avoid aoe strategy, which may interfere with scripted avoidance
float PrinceMalchezaarDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Don't run back into Shadow Nova when Enfeebled
float PrinceMalchezaarEnfeebleKeepDistanceMultiplier::GetValue(Action* action)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return 1.0f;

    if (botAI->IsMelee(bot) && bot->HasAura(SPELL_ENFEEBLE) &&
        !dynamic_cast<PrinceMalchezaarEnfeebledAvoidHazardAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot) && bot->HasAura(SPELL_ENFEEBLE) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<PrinceMalchezaarEnfeebledAvoidHazardAction*>(action)))
        return 0.0f;

    return 1.0f;
}

// Pets tend to run out of bounds and cause skeletons to spawn off the map
// Or pull adds from inside of the tower through the floor
// This multiplier DOES NOT impact permanent pets (i.e., Hunter and Warlock pets)
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
        return 0.0f;

    if (nightbane->GetPositionZ() > 95.0f && dynamic_cast<PetAttackAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Give the main tank 8 seconds to get aggro during phase transitions
float NightbaneWaitForDPSMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > 95.0f)
        return 1.0f;

    const uint8 dpsWaitSeconds = 8;
    auto it = nightbaneDPSWaitTimer.find(bot->GetMapId());
    if (it != nightbaneDPSWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < dpsWaitSeconds)
        {
            if (!botAI->IsMainTank(bot) && (dynamic_cast<AttackAction*>(action) ||
                (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action))))
                return 0.0f;
        }
    }

    return 1.0f;
}

// The avoid aoe strategy must be disabled for the main tank
// Or they will spin Nightbane to avoid Charred Earth and wipe the raid
float NightbaneDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if ((nightbane->GetPositionZ() > 95.0f || botAI->IsMainTank(bot)) &&
        dynamic_cast<AvoidAoeAction*>(action))
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
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (!(botAI->IsMelee(bot) && nightbane->GetPositionZ() <= 95.0f) && !botAI->IsMainTank(bot) &&
        dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}
