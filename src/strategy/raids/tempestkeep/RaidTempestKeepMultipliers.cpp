#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "EquipAction.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float AlarDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (alar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
        (dynamic_cast<MovementAction*>(action) &&
         !(dynamic_cast<AlarMoveAwayFromRebirthAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))))
        return 0.0f;

    return 1.0f;
}

float AlarPhase2NoTankingIfArmorMeltedMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (botAI->IsTank(bot) && bot->HasAura(SPELL_MELT_ARMOR))
    {
        if (dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action))
           return 0.0f;
    }

    return 1.0f;
}

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return 1.0f;

    if (botAI->IsRanged(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float HighAstromancerSolarianStayStackedMultiplier::GetValue(Action* action)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Includes phase 3 wait for melee (i.e., if Sanguinar is their first target)
/* float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    // Allow misdirection actions to proceed
    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    const uint32 mapId = kaelthas->GetMapId();
    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 8;

    // Check each tankable advisor's state
    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    auto isAdvisorActive = [](Unit* advisor)
    {
        Creature* advisorCreature = advisor ? advisor->ToCreature() : nullptr;
        return advisorCreature && advisorCreature->IsAlive() &&
               advisorCreature->GetReactState() == REACT_AGGRESSIVE &&
               !advisorCreature->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
    };

    bool sanguinarActive = isAdvisorActive(sanguinar);
    bool capernianActive = isAdvisorActive(capernian);
    bool telonicusActive = isAdvisorActive(telonicus);

    // Count active tankable advisors
    uint8 activeAdvisors = (sanguinarActive ? 1 : 0) +
                           (capernianActive ? 1 : 0) +
                           (telonicusActive ? 1 : 0);

    // Phase 1: Only one advisor active - all non-tanks wait
    if (activeAdvisors == 1)
    {
        auto it = advisorDpsWaitTimer.find(mapId);
        if (it == advisorDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
        {
            // If only Capernian is active, exclude the Capernian tank
            bool isCapernianTank = capernianActive && !sanguinarActive && !telonicusActive &&
                                   (GetCapernianTank(botAI, bot) == bot);

            if (!isCapernianTank)
            {
                if ((!botAI->IsTank(bot) && dynamic_cast<AttackAction*>(action)) ||
                    (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                    return 0.0f;
            }
        }
    }
    // Phase 3: All 3 advisors active - melee only wait
    else if (activeAdvisors == 3)
    {
        auto it = advisorDpsWaitTimer.find(mapId);
        if (it == advisorDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
        {
            if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
            {
                if (dynamic_cast<AttackAction*>(action) ||
                    dynamic_cast<CastSpellAction*>(action))
                    return 0.0f;
            }
        }
    }

    return 1.0f;
} */

float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    // Allow misdirection actions to proceed
    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    // Only apply wait logic in Phase 1
    if (!IsKaelthasInPhase1(botAI))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 10;

    // Check if timer has elapsed
    auto it = advisorDpsWaitTimer.find(kaelthas->GetMapId());
    if (it == advisorDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        // Check if any advisor is active
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

        auto isAdvisorActive = [](Unit* advisor)
        {
            return advisor && advisor->IsAlive() && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        };

        bool sanguinarActive = isAdvisorActive(sanguinar);
        bool capernianActive = isAdvisorActive(capernian);
        bool telonicusActive = isAdvisorActive(telonicus);

        // If any advisor is active, apply wait logic
        if (sanguinarActive || capernianActive || telonicusActive)
        {
            // Capernian tank is excluded from wait
            bool isCapernianTank = capernianActive && (GetCapernianTank(botAI, bot) == bot);

            if (!isCapernianTank && !botAI->IsTank(bot))
            {
                // All non-tanks wait
                if (dynamic_cast<AttackAction*>(action) ||
                    (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)))
                    return 0.0f;
            }
        }
    }

    return 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderDisableTankAssistMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) && bot->GetVictim() != nullptr)
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (!IsKaelthasInPhase3(botAI) && !IsKaelthasInPhase5(botAI))
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true) || !botAI->IsTank(bot))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    // Check if any raid member is mind controlled
    bool hasMindControlledPlayer = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
        {
            hasMindControlledPlayer = true;
            break;
        }
    }

    // If someone is MC'd, suppress all attacks except the MC-breaking action
    if (hasMindControlledPlayer)
    {
        if (dynamic_cast<AttackAction*>(action) &&
            !dynamic_cast<KaelthasSunstriderBreakMindControlWithInfinityBladeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderAllDpsOnBossDuringPyroblastMultiplier::GetValue(Action* action)
{
    if (!botAI->IsDps(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasUnitState(UNIT_STATE_CASTING))
        return 1.0f;

    Spell* currentSpell = kaelthas->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (currentSpell && currentSpell->m_spellInfo->Id == SPELL_KAELTHAS_PYROBLAST)
    {
        if (dynamic_cast<KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
