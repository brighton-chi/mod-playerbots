#ifndef _PLAYERBOT_RAIDBLACKTEMPLETRIGGERS_H
#define _PLAYERBOT_RAIDBLACKTEMPLETRIGGERS_H

#include "Trigger.h"

// High Warlord Naj'entus

class HighWarlordNajentusPullingBossTrigger : public Trigger
{
public:
    HighWarlordNajentusPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus pulling boss") {}
    bool IsActive() override;
};

class HighWarlordNajentusBossEngagedByMainTankTrigger : public Trigger
{
public:
    HighWarlordNajentusBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus boss engaged by main tank") {}
    bool IsActive() override;
};

class HighWarlordNajentusCastsNeedleSpinesTrigger : public Trigger
{
public:
    HighWarlordNajentusCastsNeedleSpinesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus casts needle spines") {}
    bool IsActive() override;
};

// Supremus

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
