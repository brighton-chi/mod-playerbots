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

float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
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

    if (kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
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

    if (!kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        return 1.0f;

    if (!AreAllAdvisorsActive(botAI))
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderReequipGearMultiplier::GetValue(Action* action)
{
    // Only apply to "equip upgrades" action
    if (!dynamic_cast<EquipUpgradesAction*>(action))
        return 1.0f;

    // Only check when out of combat
    if (bot->IsInCombat())
        return 1.0f;

    // Check if we're in Tempest Keep
    if (bot->GetMapId() != 550) // Tempest Keep map ID
        return 1.0f;

    // Try to find Kael'thas using creature search (works outside combat)
    std::list<ObjectGuid> targets;
    AiObjectContext* context = botAI->GetAiObjectContext();

    // Use the "possible targets" value which doesn't require combat
    GuidVector possibleTargets = context->GetValue<GuidVector>("possible targets")->Get();

    Unit* kaelthas = nullptr;
    for (auto guid : possibleTargets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_KAELTHAS_SUNSTRIDER)
        {
            kaelthas = unit;
            break;
        }
    }

    // If no Kael'thas found via possible targets, do a direct creature search
    /* if (!kaelthas)
    {
        std::list<Creature*> creatures;
        acore::AllCreaturesOfEntryInRange check(bot, NPC_KAELTHAS_SUNSTRIDER, 200.0f);
        acore::CreatureListSearcher<acore::AllCreaturesOfEntryInRange> searcher(bot, creatures, check);
        Cell::VisitGridObjects(bot, searcher, 200.0f);

        if (!creatures.empty())
            kaelthas = creatures.front();
    } */

    if (kaelthas && bot->GetExactDist2d(kaelthas) < 200.0f)
    {
        // Check if bot has empty equipment slots (lost legendary weapon)
        bool hasEmptySlots = false;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                continue;

            if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                hasEmptySlots = true;
                break;
            }
        }

        // If has empty slots, massively boost "equip upgrades" priority
        if (hasEmptySlots)
        {
            LOG_DEBUG("playerbots", "KaelthasSunstriderReequipGearMultiplier: {} has empty slots after wipe, forcing reequip",
                      bot->GetName());
            return 100.0f; // Very high multiplier to force immediate execution
        }
    }

    return 1.0f;
}
