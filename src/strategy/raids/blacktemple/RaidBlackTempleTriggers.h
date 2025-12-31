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

class SupremusBossEngagedByRangedTrigger : public Trigger
{
public:
    SupremusBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss engaged by ranged") {}
    bool IsActive() override;
};

class SupremusBossIsFixatedOnBotTrigger : public Trigger
{
public:
    SupremusBossIsFixatedOnBotTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss is fixated on bot") {}
    bool IsActive() override;
};

class SupremusVolcanoIsNearbyTrigger : public Trigger
{
public:
    SupremusVolcanoIsNearbyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus volcano is nearby") {}
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
// N/A

// Teron Gorefiend
class TeronGorefiendPullingBossTrigger : public Trigger
{
public:
    TeronGorefiendPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend pulling boss") {}
    bool IsActive() override;
};

class TeronGorefiendBossEngagedTrigger : public Trigger
{
public:
    TeronGorefiendBossEngagedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss engaged") {}
    bool IsActive() override;
};

class TeronGorefiendBossIsCastingShadowOfDeathTrigger : public Trigger
{
public:
    TeronGorefiendBossIsCastingShadowOfDeathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss is casting shadow of death") {}
    bool IsActive() override;
};

class TeronGorefiendBotHasShadowOfDeathTrigger : public Trigger
{
public:
    TeronGorefiendBotHasShadowOfDeathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend bot has shadow of death") {}
    bool IsActive() override;
};

class TeronGorefiendBotTransformedIntoVengefulSpiritTrigger : public Trigger
{
public:
    TeronGorefiendBotTransformedIntoVengefulSpiritTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend bot transformed into vengeful spirit") {}
    bool IsActive() override;
};

// Gurtogg Bloodboil

// Reliquary of Souls

class ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of desire has rune shield") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of desire casting deaden") {}
    bool IsActive() override;
};

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
