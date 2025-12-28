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

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;
};

// Supremus

class SupremusMisdirectBossToMainTankAction : public AttackAction
{
public:
    SupremusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "supremus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class SupremusMainTankPositionBossAction : public AttackAction
{
public:
    SupremusMainTankPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "supremus main tank position boss") {}
    bool Execute(Event event) override;
};

class SupremusSpreadRangedInArcAction : public MovementAction
{
public:
    SupremusSpreadRangedInArcAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus spread ranged in arc") {}
    bool Execute(Event event) override;
};

class SupremusDisperseRangedAction : public MovementAction
{
public:
    SupremusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus disperse ranged") {}
    bool Execute(Event event) override;
};

class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;
};

class SupremusManagePhaseTimerAction : public Action
{
public:
    SupremusManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "supremus manage phase timer") {}
    bool Execute(Event event) override;
};

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
