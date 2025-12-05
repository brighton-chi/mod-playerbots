#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepBossAI.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    boss_alar* alarAI = dynamic_cast<boss_alar*>(alar->GetAI());
    if (!alarAI || alarAI->HasPretendedToDie())
        return 1.0f;

    if (IsAlarAddTank(botAI, bot))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    return 1.0f;
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

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint8 dpsWaitSeconds = 12;

    auto it = advisorDpsWaitTimer.find(TEMPESTKEEP_MAP_ID);
    if (it == advisorDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

        auto isAdvisorActive = [](Unit* advisor)
        {
            return advisor && advisor->IsAlive() &&
                   !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
                   !advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
        };

        if (isAdvisorActive(sanguinar) && !botAI->IsMainTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }

        if (isAdvisorActive(telonicus) && !botAI->IsAssistTankOfIndex(bot, 0))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }

        Player* capernianTank = GetCapernianTank(botAI, bot);
        if (isAdvisorActive(capernian) && capernianTank && capernianTank != bot &&
            !botAI->IsMainTank(bot))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
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

// Rename if just Shadow Ward ultimately
float KaelthasSunstriderDisableTankActionsMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (dynamic_cast<CastShadowWardAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot))
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return 1.0f;

    if (thaladred->GetVictim() == bot)
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if ((kaelAI->GetPhase() != PHASE_ALL_ADVISORS && kaelAI->GetPhase() != PHASE_FINAL) ||
        thaladred && thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
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

    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true) || botAI->IsTank(bot))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

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

    if (kaelthas->HasAura(SPELL_SHOCK_BARRIER))
    {
        if (dynamic_cast<KaelthasSunstriderRoundUpPhoenixesAndFocusDownEggsAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderMeleeDpsDoNotAttackDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (bot->HasAura(SPELL_GRAVITY_LAPSE))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
