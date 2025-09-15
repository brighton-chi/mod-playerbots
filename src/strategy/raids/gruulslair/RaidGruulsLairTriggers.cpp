#include "RaidGruulsLairTriggers.h"
#include "Playerbots.h"

bool HighKingMaulgarTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if ((!maulgar || !maulgar->IsAlive()) &&
        (!kiggler || !kiggler->IsAlive()) &&
        (!krosh || !krosh->IsAlive()) &&
        (!olm || !olm->IsAlive()) &&
        (!blindeye || !blindeye->IsAlive()))
        return false;
    
    return true;
}

/* bool GruulTheDragonkillerTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return boss && boss->IsAlive();
} */
