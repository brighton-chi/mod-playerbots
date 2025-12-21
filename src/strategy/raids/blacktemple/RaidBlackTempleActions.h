#ifndef _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H
#define _PLAYERBOT_RAIDBLACKTEMPLEACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// High Warlord Naj'entus

class HighWarlordNajentusMisdirectBossToMainTankAction : public AttackAction
{
public:
    HighWarlordNajentusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusMainTankPositionBossAction : public AttackAction
{
public:
    HighWarlordNajentusMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus main tank position boss") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusDisperseRangedAction : public MovementAction
{
public:
    HighWarlordNajentusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "naj'entus disperse ranged") {}
    bool Execute(Event event) override;
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
