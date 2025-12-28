#ifndef _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEMULTIPLIERS_H

#include "Multiplier.h"

// High Warlord Naj'entus

// Supremus
class SupremusKiteBossMultiplier : public Multiplier
{
public:
    SupremusKiteBossMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus kite boss multiplier") {}
    virtual float GetValue(Action* action);
};

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
