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

class SupremusNeedToManagePhaseTimerTrigger : public Trigger
{
public:
    SupremusNeedToManagePhaseTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus need to manage phase timer") {}
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

class GurtoggBloodboilPullingBossTrigger : public Trigger
{
public:
    GurtoggBloodboilPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil pulling boss") {}
    bool IsActive() override;
};

class GurtoggBloodboilBossEngagedByTanksTrigger : public Trigger
{
public:
    GurtoggBloodboilBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil boss engaged by tanks") {}
    bool IsActive() override;
};

class GurtoggBloodboilBossCastsAoeSpellsTrigger : public Trigger
{
public:
    GurtoggBloodboilBossCastsAoeSpellsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil boss casts aoe spells") {}
    bool IsActive() override;
};

class GurtoggBloodboilBossCastsBloodboilOnFiveFarthestPlayersTrigger : public Trigger
{
public:
    GurtoggBloodboilBossCastsBloodboilOnFiveFarthestPlayersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil boss casts bloodboil on five farthest players") {}
    bool IsActive() override;
};

class GurtoggBloodboilBotHasFelRageTrigger : public Trigger
{
public:
    GurtoggBloodboilBotHasFelRageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil bot has fel rage") {}
    bool IsActive() override;
};

class GurtoggBloodboilNeedToManagePhaseTimerTrigger : public Trigger
{
public:
    GurtoggBloodboilNeedToManagePhaseTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil need to manage phase timer") {}
    bool IsActive() override;
};

// Reliquary of Souls

class ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger : public Trigger
{
public:
    ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls aggro resets upon phase change") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of suffering fixates on closest target") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of suffering disables healing") {}
    bool IsActive() override;
};

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

class ReliquaryOfSoulsNeedToManageDpsTimerTrigger : public Trigger
{
public:
    ReliquaryOfSoulsNeedToManageDpsTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls need to manage dps timer") {}
    bool IsActive() override;
};

// Mother Shahraz

// Illidari Council

// Illidan Stormrage <The Betrayer>

#endif
