#include "RaidBlackTempleMultipliers.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace BlackTempleHelpers;

// High Warlord Naj'entus
// N/A

// Supremus
float SupremusFocusOnAvoidanceInPhase2Multiplier::GetValue(Action* action)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (supremus && supremus->HasAura(SPELL_SNARE_SELF) && supremus->GetVictim() == bot)
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<SupremusKiteBossAction*>(action) &&
            !dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Shade of Akama
// N/A

// Teron Gorefiend
float TeronGorefiendDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (gorefiend)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) &&
            !dynamic_cast<SetBehindTargetAction*>(action))
            return 0.0f;

        if (dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;

        if (botAI->IsRanged(bot))
        {
            if (dynamic_cast<ReachTargetAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValue(Action* action)
{
    Aura* aura = bot->GetAura(SPELL_SHADOW_OF_DEATH);
    if (aura && aura->GetDuration() < 15000)
    {
        if (dynamic_cast<WipeAction*>(action))
            return 1.0f;

        if (!dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_SPIRITUAL_VENGEANCE))
    {
        if (dynamic_cast<WipeAction*>(action))
            return 1.0f;

        if (!dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float TeronGorefiendDisableAttackingConstructsMultiplier::GetValue(Action* action)
{
    if (botAI->IsHeal(bot))
        return 1.0f;

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (auto castSpellAction = dynamic_cast<CastSpellAction*>(action))
    {
        if (castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
            return 0.0f;
    }

    return 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilDisableMovementMultiplier::GetValue(Action* action)
{
    Unit* bloodboil = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (bloodboil)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) &&
            !dynamic_cast<SetBehindTargetAction*>(action))
            return 0.0f;

        if (dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Reliquary of Souls

float ReliquaryOfSoulsWaitForDpsMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot))
        return 1.0f;

    Unit* reliquary = AI_VALUE2(Unit*, "find target", "reliquary of souls");
    if (!reliquary)
        return 1.0f;

    if (dynamic_cast<ReliquaryOfSoulsMisdirectBossToMainTankAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);

    auto it = reliquaryDpsWaitTimer.find(reliquary->GetMap()->GetInstanceId());
    if (it == reliquaryDpsWaitTimer.end() || (now - it->second) < 5)
    {
        Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
        Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
        Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
        if ((suffering && suffering->GetHealthPct() > 99.8f) ||
            (desire && desire->GetHealthPct() > 99.8f) ||
            (anger && anger->GetHealthPct() > 99.8f))
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                 return 0.0f;
        }
    }

    return 1.0f;
}

float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValue(Action* action)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return 1.0f;

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        return 10.0f;
    }
    else if (dynamic_cast<CastHealingSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

float ReliquaryOfSoulsDontInterruptDeadenIfReflectableMultiplier::GetValue(Action* action)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire)
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    if (desire->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && spell->m_spellInfo->Id == SPELL_DEADEN)
        {
            Unit* target = spell->m_targets.GetUnitTarget();
            Player* playerTarget = target ? target->ToPlayer() : nullptr;

            if (playerTarget && botAI->IsTank(playerTarget) &&
                playerTarget->getClass() == CLASS_WARRIOR)
            {
                if (dynamic_cast<CastBashAction*>(action) ||
                    dynamic_cast<CastShieldBashAction*>(action) ||
                    dynamic_cast<CastPummelAction*>(action) ||
                    dynamic_cast<CastCounterspellAction*>(action) ||
                    dynamic_cast<CastKickAction*>(action) ||
                    dynamic_cast<CastSilencingShotAction*>(action) ||
                    dynamic_cast<CastWindShearAction*>(action) ||
                    dynamic_cast<CastMindFreezeAction*>(action))
                {
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
