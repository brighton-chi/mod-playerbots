#include "RaidBlackTempleMultipliers.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "AiFactory.h"
#include "ChooseTargetActions.h"
#include "HunterActions.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace BlackTempleHelpers;

// High Warlord Naj'entus

// Supremus
float SupremusKiteBossMultiplier::GetValue(Action* action)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || !supremus->HasAura(SPELL_SNARE_SELF))
        return 1.0f;

    if (supremus->GetVictim() == bot)
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<SupremusKiteBossAction*>(action) &&
            !dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
