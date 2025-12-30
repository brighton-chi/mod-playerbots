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

class HighWarlordNajentusPlayerIsImpaledTrigger : public Trigger
{
public:
    HighWarlordNajentusPlayerIsImpaledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus player is impaled") {}
    bool IsActive() override;
};

class HighWarlordNajentusBossHasTidalShieldTrigger : public Trigger
{
public:
    HighWarlordNajentusBossHasTidalShieldTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus boss has tidal shield") {}
    bool IsActive() override;
};

// Supremus

class SupremusPullingBossOrChangingPhaseTrigger : public Trigger
{
public:
    SupremusPullingBossOrChangingPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus pulling boss or changing phase") {}
    bool IsActive() override;
};

class SupremusBossEngagedByMainTankTrigger : public Trigger
{
public:
    SupremusBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss engaged by main tank") {}
    bool IsActive() override;
};

class SupremusBossEngagedByRangedTrigger : public Trigger
{
public:
    SupremusBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss engaged by ranged") {}
    bool IsActive() override;
};

class SupremusEncounterIsInPhase2Trigger : public Trigger
{
public:
    SupremusEncounterIsInPhase2Trigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus encounter is in phase 2") {}
    bool IsActive() override;
};

class SupremusChangesPhaseEvery60SecondsTrigger : public Trigger
{
public:
    SupremusChangesPhaseEvery60SecondsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus changes phase every 60 seconds") {}
    bool IsActive() override;
};

// Shade of Akama

// Teron Gorefiend

// Gurtogg Bloodboil

// Reliquary of Souls

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
