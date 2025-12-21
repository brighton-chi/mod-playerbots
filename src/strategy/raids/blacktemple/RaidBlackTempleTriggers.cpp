#include "RaidBlackTempleTriggers.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBlackTempleActions.h"
#include "AiFactory.h"
#include "Playerbots.h"

using namespace BlackTempleHelpers;

// High Warlord Naj'entus

bool HighWarlordNajentusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus && najentus->GetHealthPct() > 95.0f;
}

bool HighWarlordNajentusBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus != nullptr;
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus != nullptr;
}

// Supremus

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>
