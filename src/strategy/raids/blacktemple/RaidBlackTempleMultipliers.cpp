#include "RaidBlackTempleMultipliers.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
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
float TeronGorefiendDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (gorefiend)
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) &&
            !dynamic_cast<SetBehindTargetAction*>(action))
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

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
