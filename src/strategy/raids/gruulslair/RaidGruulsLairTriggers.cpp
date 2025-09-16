#include "RaidGruulsLairTriggers.h"
#include "Playerbots.h"

bool HighKingMaulgarTrigger::IsActive()
{
    // Store original sight distance
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    
    // Temporarily increase sight distance for target detection
    sPlayerbotAIConfig->sightDistance = 150.0f;
    
    // Try to find targets with increased sight
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    
    // Store result before restoring sight distance
    bool foundBosses = (maulgar && maulgar->IsAlive()) ||
                      (kiggler && kiggler->IsAlive()) ||
                      (krosh && krosh->IsAlive()) ||
                      (olm && olm->IsAlive()) ||
                      (blindeye && blindeye->IsAlive());
    
    // If any boss is found, leave increased sight distance
    // Otherwise restore original sight distance
    if (!foundBosses)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;
    
    return foundBosses;
}

bool GruulTheDragonkillerTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return boss && boss->IsAlive();
}
