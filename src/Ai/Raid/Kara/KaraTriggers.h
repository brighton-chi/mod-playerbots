/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERS_H
#define PLAYERBOTS_KARATRIGGERS_H

#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "Trigger.h"

// Shared

class KarazhanEncounterTrigger : public Trigger
{
public:
    KarazhanEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, KaraHelpers::KaraBosses boss,
        int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, KaraHelpers::KARA_MAP_ID, KaraHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;

    Unit* GetEncounterBoss() { return KaraHelpers::GetKaraBoss(bot, _boss); }

private:
    KaraHelpers::KaraBosses const _boss;
};

class KarazhanNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    KarazhanNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan no encounter in progress", 1000) {}
    bool IsActive() override;
};

class KarazhanEnemiesCastFearTrigger : public Trigger
{
public:
    KarazhanEnemiesCastFearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan enemies cast fear") {}
    bool IsActive() override;
};

// Trash

class ManaWarpCastsWarpBreachTrigger : public Trigger
{
public:
    ManaWarpCastsWarpBreachTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp casts warp breach") {}
    bool IsActive() override;
};

class ManaWarpStunnerShouldHoldTrigger : public Trigger
{
public:
    ManaWarpStunnerShouldHoldTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp stunner should hold") {}
    bool IsActive() override;
};

// Attumen the Huntsman

class AttumenTheHuntsmanShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should be tanked",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanMidnightShouldBeFocusedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanMidnightShouldBeFocusedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman midnight should be focused",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanMountedShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanMountedShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman mounted should be tanked",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanShouldStackBehindTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldStackBehindTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should stack behind",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanShouldManageDpsTimerTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "attumen the huntsman should manage dps timer",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Moroes <Tower Steward>

class MoroesShouldPrioritizeAddsTrigger : public KarazhanEncounterTrigger
{
public:
    MoroesShouldPrioritizeAddsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "moroes should prioritize adds", KaraHelpers::KaraBosses::DATA_MOROES) {}

protected:
    bool IsActiveInEncounter() override;
};

// Maiden of Virtue

class MaidenOfVirtueShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue should be tanked", KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueShouldBreakRepentanceTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldBreakRepentanceTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue should break repentance",
            KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue ranged should spread", KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueShouldGroundHolyFireTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldGroundHolyFireTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "maiden of virtue should ground holy fire",
            KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Big Bad Wolf

class BigBadWolfShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf should be tanked",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class BigBadWolfChasingRedHoodTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfChasingRedHoodTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf chasing red hood",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class BigBadWolfPaladinShouldProtectRedHoodTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfPaladinShouldProtectRedHoodTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "big bad wolf paladin should protect red hood",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// Romulo and Julianne

class RomuloAndJulianneShouldBeKilledTogetherTrigger : public KarazhanEncounterTrigger
{
public:
    RomuloAndJulianneShouldBeKilledTogetherTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "romulo and julianne should be killed together",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Wizard of Oz

class WizardOfOzShouldPrioritizeTargetsTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzShouldPrioritizeTargetsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "wizard of oz should prioritize targets",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

class WizardOfOzMageShouldBurnStrawmanTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzMageShouldBurnStrawmanTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "wizard of oz mage should burn strawman",
            KaraHelpers::KaraBosses::DATA_OPERA_PERFORMANCE) {}

protected:
    bool IsActiveInEncounter() override;
};

// The Curator

class TheCuratorShouldPrioritizeAstralFlaresTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorShouldPrioritizeAstralFlaresTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator should prioritize astral flares",
            KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator should be tanked", KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "the curator ranged should spread", KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Terestian Illhoof

class TerestianIllhoofShouldPrioritizeChainsTrigger : public KarazhanEncounterTrigger
{
public:
    TerestianIllhoofShouldPrioritizeChainsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "terestian illhoof should prioritize chains",
            KaraHelpers::KaraBosses::DATA_TERESTIAN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Aran

class ShadeOfAranArcaneExplosionIsCastingTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranArcaneExplosionIsCastingTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran arcane explosion is casting",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranRangedIsInBlizzardTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranRangedIsInBlizzardTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran ranged is in blizzard", KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranFlameWreathIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranFlameWreathIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran flame wreath is active", KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranConjuredElementalsSummonedTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranConjuredElementalsSummonedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran conjured elementals summoned",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranRangedShouldMaintainDistanceTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranRangedShouldMaintainDistanceTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "shade of aran ranged should maintain distance",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    bool IsActiveInEncounter() override;
};

// Netherspite

class NetherspiteRedBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteRedBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite red beam is active", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteBlueBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteBlueBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite blue beam is active", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteGreenBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteGreenBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite green beam is active", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteBotIsNotBeamBlockerTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteBotIsNotBeamBlockerTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite bot is not beam blocker",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteInBanishPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteInBanishPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite in banish phase", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "netherspite should manage timers and trackers",
            KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    bool IsActiveInEncounter() override;
};

// Prince Malchezaar

class PrinceMalchezaarBotIsEnfeebledTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar bot is enfeebled",
            KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarEngagedByNonTanksTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarEngagedByNonTanksTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar engaged by non-tanks",
            KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "prince malchezaar should be tanked",
            KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Nightbane

class NightbaneShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should be tanked", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneGroundPhaseEngagedByRangedTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneGroundPhaseEngagedByRangedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane ground phase engaged by ranged",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbanePetsChaseFlyingBossOutOfBoundsTrigger : public KarazhanEncounterTrigger
{
public:
    NightbanePetsChaseFlyingBossOutOfBoundsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane pets chase flying boss out of bounds",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneInFlightPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneInFlightPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane in flight phase", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneBotWentOutOfBoundsTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneBotWentOutOfBoundsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane bot went out of bounds", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(
            botAI, "nightbane should manage timers and trackers",
            KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
