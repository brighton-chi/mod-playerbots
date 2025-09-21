#include "RaidMagsLairTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool MagtheridonTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");

    return (magtheridon && magtheridon->IsAlive()) ||
           (channeler && channeler->IsAlive());
}
