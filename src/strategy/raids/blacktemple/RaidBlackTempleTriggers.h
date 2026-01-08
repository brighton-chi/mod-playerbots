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

class TeronGorefiendBossEngagedByMainTankTrigger : public Trigger
{
public:
    TeronGorefiendBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss engaged by main tank") {}
    bool IsActive() override;
};

class TeronGorefiendBossEngagedByRangedTrigger : public Trigger
{
public:
    TeronGorefiendBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss engaged by ranged") {}
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

class MotherShahrazPullingBossTrigger : public Trigger
{
public:
    MotherShahrazPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz pulling boss") {}
    bool IsActive() override;
};

class MotherShahrazBossEngagedByTanksTrigger : public Trigger
{
public:
    MotherShahrazBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz boss engaged by tanks") {}
    bool IsActive() override;
};

class MotherShahrazSinisterBeamKnocksBackPlayersTrigger : public Trigger
{
public:
    MotherShahrazSinisterBeamKnocksBackPlayersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz sinister beam knocks back players") {}
    bool IsActive() override;
};

class MotherShahrazBotsAreLinkedByFatalAttractionTrigger : public Trigger
{
public:
    MotherShahrazBotsAreLinkedByFatalAttractionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz bots are linked by fatal attraction") {}
    bool IsActive() override;
};

// Illidari Council

class IllidariCouncilPullingBossesTrigger : public Trigger
{
public:
    IllidariCouncilPullingBossesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council pulling bosses") {}
    bool IsActive() override;
};

class IllidariCouncilGathiosEngagedByMainTankTrigger : public Trigger
{
public:
    IllidariCouncilGathiosEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council gathios engaged by main tank") {}
    bool IsActive() override;
};

class IllidariCouncilGathiosCastingJudgementOfCommandTrigger : public Trigger
{
public:
    IllidariCouncilGathiosCastingJudgementOfCommandTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council gathios casting judgement of command") {}
    bool IsActive() override;
};

class IllidariCouncilMalandeEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    IllidariCouncilMalandeEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council malande engaged by first assist tank") {}
    bool IsActive() override;
};

class IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council darkshadow engaged by second assist tank") {}
    bool IsActive() override;
};

class IllidariCouncilZerevorEngagedByMageTankTrigger : public Trigger
{
public:
    IllidariCouncilZerevorEngagedByMageTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council zerevor engaged by mage tank") {}
    bool IsActive() override;
};

class IllidariCouncilDeterminingDpsAssignmentsTrigger : public Trigger
{
public:
    IllidariCouncilDeterminingDpsAssignmentsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council determining dps assignments") {}
    bool IsActive() override;
};

class IllidariCouncilNeedToManageDpsTimerTrigger : public Trigger
{
public:
    IllidariCouncilNeedToManageDpsTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council need to manage dps timer") {}
    bool IsActive() override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageTankNeedsAggroTrigger : public Trigger
{
public:
    IllidanStormrageTankNeedsAggroTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage tank needs aggro") {}
    bool IsActive() override;
};

class IllidanStormrageBossCastsFlameCrashTrigger : public Trigger
{
public:
    IllidanStormrageBossCastsFlameCrashTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss casts flame crash") {}
    bool IsActive() override;
};

class IllidanStormrageBotHasParasiticShadowfiendTrigger : public Trigger
{
public:
    IllidanStormrageBotHasParasiticShadowfiendTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage bot has parasitic shadowfiend") {}
    bool IsActive() override;
};

class IllidanStormrageBossSummonedFlamesOfAzzinothTrigger : public Trigger
{
public:
    IllidanStormrageBossSummonedFlamesOfAzzinothTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss summoned flames of azzinoth") {}
    bool IsActive() override;
};

class IllidanStormragePetsDieToFireTrigger : public Trigger
{
public:
    IllidanStormragePetsDieToFireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage pets die to fire") {}
    bool IsActive() override;
};

class IllidanStormrageGrateIsSafeFromFlamesTrigger : public Trigger
{
public:
    IllidanStormrageGrateIsSafeFromFlamesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage grate is safe from flames") {}
    bool IsActive() override;
};

class IllidanStormrageBotStruckByDarkBarrageTrigger : public Trigger
{
public:
    IllidanStormrageBotStruckByDarkBarrageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage bot struck by dark barrage") {}
    bool IsActive() override;
};

class IllidanStormrageBossDealsSplashDamageTrigger : public Trigger
{
public:
    IllidanStormrageBossDealsSplashDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss deals splash damage") {}
    bool IsActive() override;
};

class IllidanStormrageThisExpansionHatesMeleeTrigger : public Trigger
{
public:
    IllidanStormrageThisExpansionHatesMeleeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage this expansion hates melee") {}
    bool IsActive() override;
};

class IllidanStormrageBossTransformsIntoDemonTrigger : public Trigger
{
public:
    IllidanStormrageBossTransformsIntoDemonTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss transforms into demon") {}
    bool IsActive() override;
};

class IllidanStormrageBossSummonsAddsTrigger : public Trigger
{
public:
    IllidanStormrageBossSummonsAddsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss summons adds") {}
    bool IsActive() override;
};

class IllidanStormrageNeedToManageDpsTimerTrigger : public Trigger
{
public:
    IllidanStormrageNeedToManageDpsTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage need to manage dps timer") {}
    bool IsActive() override;
};

#endif
