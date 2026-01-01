#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
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

// Al'ar <Phoenix God>

float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return 1.0f;

    if (dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsDps(bot))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AlarDisableDisperseMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AlarDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
        return 1.0f;

    if (bot->GetVictim() != nullptr)
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<AlarMoveAwayFromRebirthAction*>(action) &&
            !dynamic_cast<AlarAvoidFlamePatchesAndDiveBombsAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float AlarPhase2NoTankingIfArmorMeltedMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_MELT_ARMOR))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (alar && bot->GetTarget() == alar->GetGUID())
    {
        if (dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action) ||
            dynamic_cast<CastDarkCommandAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "void reaver"))
        return 1.0f;

    if (botAI->IsTank(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    if (botAI->IsRanged(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianMaintainPositionMultiplier::GetValue(Action* action)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    if (botAI->IsRanged(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;
    }

    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action)))
             return 0.0f;
    }

    return 1.0f;
}

float HighAstromancerSolarianDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "solarium priest") &&
        bot->GetVictim() != nullptr)
    {
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

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
    const uint8 dpsWaitSeconds = 10;

    auto it = advisorDpsWaitTimer.find(kaelthas->GetMap()->GetInstanceId());
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

        if ((isAdvisorActive(sanguinar) && !botAI->IsMainTank(bot)) ||
            (isAdvisorActive(telonicus) && !botAI->IsAssistTankOfIndex(bot, 0)) ||
            (isAdvisorActive(capernian) && !botAI->IsMainTank(bot) && GetCapernianTank(botAI, bot) != bot))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                 return 0.0f;
        }
    }

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (botAI->IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
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

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI && kaelAI->GetPhase() != PHASE_FINAL)
    {
        if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// For disabling Capernian Tank's Shadow Ward, which is part of the standard
// tank strategy for Warlocks (made for Twin Emps but useless here)
float KaelthasSunstriderDisableShadowWardMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
    {
        if (dynamic_cast<CastShadowWardAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderManageTankTargetingMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe
    if (kaelAI->GetPhase() == PHASE_WEAPONS)
    {
        if (botAI->IsMainTank(bot))
        {
            if (dynamic_cast<TankAssistAction*>(action) ||
                dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastChallengingShoutAction*>(action) ||
                dynamic_cast<CastThunderClapAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastCleaveAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action) ||
                dynamic_cast<CastSwipeAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastAvengersShieldAction*>(action) ||
                dynamic_cast<CastConsecrationAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastDeathAndDecayAction*>(action) ||
                dynamic_cast<CastPestilenceAction*>(action) ||
                dynamic_cast<CastBloodBoilAction*>(action))
                return 0.0f;
        }
    }
    else
    {
        if (dynamic_cast<TankFaceAction*>(action))
            return 0.0f;
    }

    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
        kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        if (bot->GetVictim() != nullptr)
        {
            if (dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Bloodlust/Heroism should be used at the start of Phase 3
float KaelthasSunstriderDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI &&
        kaelAI->GetPhase() != PHASE_ALL_ADVISORS && kaelAI->GetPhase() != PHASE_FINAL)
    {
        if (dynamic_cast<CastBloodlustAction*>(action) ||
            dynamic_cast<CastHeroismAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderTryNonfatalBreakingOfMindControlMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot) || !bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
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
            !dynamic_cast<KaelthasSunstriderBreakMindControlAction*>(action))
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
        if (dynamic_cast<KaelthasSunstriderHandlePhoenixesAndEggsAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_GRAVITY_LAPSE))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
