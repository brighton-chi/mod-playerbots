#include "RaidBlackTempleMultipliers.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
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

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
