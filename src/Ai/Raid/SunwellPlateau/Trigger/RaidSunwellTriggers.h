/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDSUNWELLTRIGGERS_H
#define _PLAYERBOT_RAIDSUNWELLTRIGGERS_H

#include "Trigger.h"

// General

class SunwellPlateauBotIsNotInCombatTrigger : public Trigger
{
public:
    SunwellPlateauBotIsNotInCombatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "sunwell plateau bot is not in combat") {}
    bool IsActive() override;
};

// Kalecgos & Sathrovarr the Corruptor

class KalecgosBossEngagedByTankTrigger : public Trigger
{
public:
    KalecgosBossEngagedByTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos boss engaged by tank") {}
    bool IsActive() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public Trigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos spectral rift is open") {}
    bool IsActive() override;
};

class KalecgosBotsTakeSplashDamageTrigger : public Trigger
{
public:
    KalecgosBotsTakeSplashDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos bots take splash damage") {}
    bool IsActive() override;
};

class KalecgosBothBossesMustBeDefeatedTrigger : public Trigger
{
public:
    KalecgosBothBossesMustBeDefeatedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kalecgos both bosses must be defeated") {}
    bool IsActive() override;
};

// Brutallus

class BrutallusPullingBossTrigger : public Trigger
{
public:
    BrutallusPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus pulling boss") {}
    bool IsActive() override;
};

class BrutallusBossEngagedByTanksTrigger : public Trigger
{
public:
    BrutallusBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus boss engaged by tanks") {}
    bool IsActive() override;
};

class BrutallusBossEngagedByMeleeTrigger : public Trigger
{
public:
    BrutallusBossEngagedByMeleeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus boss engaged by melee") {}
    bool IsActive() override;
};

class BrutallusBossEngagedByRangedTrigger : public Trigger
{
public:
    BrutallusBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus boss engaged by ranged") {}
    bool IsActive() override;
};

class BrutallusBotIsBurningTrigger : public Trigger
{
public:
    BrutallusBotIsBurningTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "brutallus bot is burning") {}
    bool IsActive() override;
};

// Felmyst

class FelmystPullingBossTrigger : public Trigger
{
public:
    FelmystPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst pulling boss") {}
    bool IsActive() override;
};

class FelmystBossEngagedByMainTankOnGroundTrigger : public Trigger
{
public:
    FelmystBossEngagedByMainTankOnGroundTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst boss engaged by main tank on ground") {}
    bool IsActive() override;
};

class FelmystBossEngagedByRangedOnGroundTrigger : public Trigger
{
public:
    FelmystBossEngagedByRangedOnGroundTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst boss engaged by ranged on ground") {}
    bool IsActive() override;
};

class FelmystBossEngagedByMeleeOnGroundTrigger : public Trigger
{
public:
    FelmystBossEngagedByMeleeOnGroundTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst boss engaged by melee on ground") {}
    bool IsActive() override;
};

class FelmystBotIsEncapsulatedTrigger : public Trigger
{
public:
    FelmystBotIsEncapsulatedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst bot is encapsulated") {}
    bool IsActive() override;
};

class FelmystBotNearEncapsulatedPlayerTrigger : public Trigger
{
public:
    FelmystBotNearEncapsulatedPlayerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst bot near encapsulated player") {}
    bool IsActive() override;
};

class FelmystPlayerHasGasNovaTrigger : public Trigger
{
public:
    FelmystPlayerHasGasNovaTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst player has gas nova") {}
    bool IsActive() override;
};

class FelmystBossSummonsDemonicVaporTrigger : public Trigger
{
public:
    FelmystBossSummonsDemonicVaporTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst boss summons demonic vapor") {}
    bool IsActive() override;
};

class FelmystBotIsDemonicVaporTargetTrigger : public Trigger
{
public:
    FelmystBotIsDemonicVaporTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst bot is demonic vapor target") {}
    bool IsActive() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public Trigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst fog of corruption is active") {}
    bool IsActive() override;
};

class FelmystManualTargetingIsRequiredForAirPhaseTrigger : public Trigger
{
public:
    FelmystManualTargetingIsRequiredForAirPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "felmyst manual targeting is required for air phase") {}
    bool IsActive() override;
};

// Eredar Twins (Alythess & Sacrolash)

class EredarTwinsEncounterJustStartedTrigger : public Trigger
{
public:
    EredarTwinsEncounterJustStartedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins encounter just started") {}
    bool IsActive() override;
};

class EredarTwinsPullingBossesTrigger : public Trigger
{
public:
    EredarTwinsPullingBossesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins pulling bosses") {}
    bool IsActive() override;
};

class EredarTwinsSacrolashEngagedByTwoTanksTrigger : public Trigger
{
public:
    EredarTwinsSacrolashEngagedByTwoTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins sacrolash engaged by two tanks") {}
    bool IsActive() override;
};

class EredarTwinsAlythessEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    EredarTwinsAlythessEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins alythess engaged by first assist tank") {}
    bool IsActive() override;
};

class EredarTwinsBossesEngagedByRangedTrigger : public Trigger
{
public:
    EredarTwinsBossesEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bosses engaged by ranged") {}
    bool IsActive() override;
};

class EredarTwinsOnlyOneBossRemainsTrigger : public Trigger
{
public:
    EredarTwinsOnlyOneBossRemainsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins only one boss remains") {}
    bool IsActive() override;
};

class EredarTwinsBotHasTooManyFlameTouchedStacksTrigger : public Trigger
{
public:
    EredarTwinsBotHasTooManyFlameTouchedStacksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bot has too many flame touched stacks") {}
    bool IsActive() override;
};

class EredarTwinsDeterminingDpsPriorityTrigger : public Trigger
{
public:
    EredarTwinsDeterminingDpsPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins determining dps priority") {}
    bool IsActive() override;
};

class EredarTwinsBotHasConflagrationTrigger : public Trigger
{
public:
    EredarTwinsBotHasConflagrationTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "eredar twins bot has conflagration") {}
    bool IsActive() override;
};

// M'uru & Entropius

class MuruVoidSentinelOrEntropiusHasAppearedTrigger : public Trigger
{
public:
    MuruVoidSentinelOrEntropiusHasAppearedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru void sentinel or entropius has appeared") {}
    bool IsActive() override;
};

class MuruBossesEngagedByRangedTrigger : public Trigger
{
public:
    MuruBossesEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru bosses engaged by ranged") {}
    bool IsActive() override;
};

class MuruDeterminingDpsPriorityTrigger : public Trigger
{
public:
    MuruDeterminingDpsPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru determining dps priority") {}
    bool IsActive() override;
};

class MuruVoidSentinelNeedsTankTrigger : public Trigger
{
public:
    MuruVoidSentinelNeedsTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru void sentinel needs tank") {}
    bool IsActive() override;
};

class MuruVoidSentinelCastsVoidBlastOnTankTrigger : public Trigger
{
public:
    MuruVoidSentinelCastsVoidBlastOnTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru void sentinel casts void blast on tank") {}
    bool IsActive() override;
};

class MuruDarkFiendsSpawnedTrigger : public Trigger
{
public:
    MuruDarkFiendsSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru dark fiends spawned") {}
    bool IsActive() override;
};

class MuruDarknessIsComingTrigger : public Trigger
{
public:
    MuruDarknessIsComingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru darkness is coming") {}
    bool IsActive() override;
};

class MuruTheSingularityIsNearTrigger : public Trigger
{
public:
    MuruTheSingularityIsNearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru the singularity is near") {}
    bool IsActive() override;
};

class MuruBerserkerIsBuffedWithFlurryTrigger : public Trigger
{
public:
    MuruBerserkerIsBuffedWithFlurryTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru berserker is buffed with flurry") {}
    bool IsActive() override;
};

class MuruFuryMageCastingFelFireballTrigger : public Trigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru fury mage casting fel fireball") {}
    bool IsActive() override;
};

class MuruFuryMageIsBuffedWithSpellFuryTrigger : public Trigger
{
public:
    MuruFuryMageIsBuffedWithSpellFuryTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru fury mage is buffed with spell fury") {}
    bool IsActive() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public Trigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru void spawn available for enslave") {}
    bool IsActive() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public Trigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "m'uru warlock has enslaved void spawn") {}
    bool IsActive() override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenHandsOfTheDeceiverAreActiveTrigger : public Trigger
{
public:
    KiljaedenHandsOfTheDeceiverAreActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden hands of the deceiver are active") {}
    bool IsActive() override;
};

class KiljaedenItsRainingMeteorsTrigger : public Trigger
{
public:
    KiljaedenItsRainingMeteorsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden its raining meteors") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByTanksTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden boss engaged by tanks") {}
    bool IsActive() override;
};

class KiljaedenSaysChaosDestructionOblivionTrigger : public Trigger
{
public:
    KiljaedenSaysChaosDestructionOblivionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden says: Chaos! Destruction! Oblivion!") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByMeleeTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByMeleeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden boss engaged by melee") {}
    bool IsActive() override;
};

class KiljaedenBossEngagedByRangedTrigger : public Trigger
{
public:
    KiljaedenBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden boss engaged by ranged") {}
    bool IsActive() override;
};

/* class KiljaedenDeterminingDpsPriorityTrigger : public Trigger
{
public:
    KiljaedenDeterminingDpsPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "kil'jaeden determining dps priority") {}
    bool IsActive() override;
}; */

#endif
