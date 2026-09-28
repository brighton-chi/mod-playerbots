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

// General

class KarazhanEncounterTrigger : public Trigger
{
public:
    KarazhanEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, KaraHelpers::KARA_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
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

class ManaWarpIsAboutToExplodeTrigger : public Trigger
{
public:
    ManaWarpIsAboutToExplodeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp is about to explode") {}
    bool IsActive() override;
};

// Attumen the Huntsman

class AttumenTheHuntsmanPhaseOneActiveTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanPhaseOneActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "attumen the huntsman phase one active") {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanPhaseTwoActiveTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanPhaseTwoActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "attumen the huntsman phase two active") {}

protected:
    bool IsActiveInEncounter() override;
};

class AttumenTheHuntsmanPhaseTransitionTrigger : public KarazhanEncounterTrigger
{
public:
    AttumenTheHuntsmanPhaseTransitionTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "attumen the huntsman phase transition") {}

protected:
    bool IsActiveInEncounter() override;
};

// Moroes <Tower Steward>

class MoroesShouldPrioritizeAddsTrigger : public KarazhanEncounterTrigger
{
public:
    MoroesShouldPrioritizeAddsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "moroes should prioritize adds") {}

protected:
    bool IsActiveInEncounter() override;
};

// Maiden of Virtue

class MaidenOfVirtueShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "maiden of virtue should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "maiden of virtue ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger : public KarazhanEncounterTrigger
{
public:
    MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "maiden of virtue grounding totem consumes holy fire") {}

protected:
    bool IsActiveInEncounter() override;
};

// The Big Bad Wolf

class BigBadWolfShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "big bad wolf should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class BigBadWolfChasingLittleRedRidingHoodTrigger : public KarazhanEncounterTrigger
{
public:
    BigBadWolfChasingLittleRedRidingHoodTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "big bad wolf chasing little red riding hood") {}

protected:
    bool IsActiveInEncounter() override;
};

// Romulo and Julianne

class RomuloAndJulianneBothBossesRevivedTrigger : public KarazhanEncounterTrigger
{
public:
    RomuloAndJulianneBothBossesRevivedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "romulo and julianne both bosses revived") {}

protected:
    bool IsActiveInEncounter() override;
};

// The Wizard of Oz

class WizardOfOzNeedTargetPriorityTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzNeedTargetPriorityTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "wizard of oz need target priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class WizardOfOzStrawmanIsVulnerableToFireTrigger : public KarazhanEncounterTrigger
{
public:
    WizardOfOzStrawmanIsVulnerableToFireTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "wizard of oz strawman is vulnerable to fire") {}

protected:
    bool IsActiveInEncounter() override;
};

// The Curator

class TheCuratorAstralFlareSpawnedTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorAstralFlareSpawnedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "the curator astral flare spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "the curator should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheCuratorRangedShouldSpreadTrigger : public KarazhanEncounterTrigger
{
public:
    TheCuratorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "the curator ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

// Terestian Illhoof

class TerestianIllhoofShouldPrioritizeChainsTrigger : public KarazhanEncounterTrigger
{
public:
    TerestianIllhoofShouldPrioritizeChainsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "terestian illhoof should prioritize chains") {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Aran

class ShadeOfAranArcaneExplosionIsCastingTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranArcaneExplosionIsCastingTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "shade of aran arcane explosion is casting") {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranFlameWreathIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranFlameWreathIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "shade of aran flame wreath is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranConjuredElementalsSummonedTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranConjuredElementalsSummonedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "shade of aran conjured elementals summoned") {}

protected:
    bool IsActiveInEncounter() override;
};

class ShadeOfAranRangedShouldMaintainDistanceTrigger : public KarazhanEncounterTrigger
{
public:
    ShadeOfAranRangedShouldMaintainDistanceTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "shade of aran ranged should maintain distance") {}

protected:
    bool IsActiveInEncounter() override;
};

// Netherspite

class NetherspiteRedBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteRedBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite red beam is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteBlueBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteBlueBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite blue beam is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteGreenBeamIsActiveTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteGreenBeamIsActiveTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite green beam is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteBotIsNotBeamBlockerTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteBotIsNotBeamBlockerTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite bot is not beam blocker") {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteInBanishPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteInBanishPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite in banish phase") {}

protected:
    bool IsActiveInEncounter() override;
};

class NetherspiteShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NetherspiteShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "netherspite should manage timers and trackers") {}

protected:
    bool IsActiveInEncounter() override;
};

// Prince Malchezaar

class PrinceMalchezaarBotIsEnfeebledTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "prince malchezaar bot is enfeebled") {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarEngagedByNonTanksTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarEngagedByNonTanksTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "prince malchezaar engaged by non-tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class PrinceMalchezaarShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    PrinceMalchezaarShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "prince malchezaar should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

// Nightbane

class NightbaneShouldBeTankedTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldBeTankedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneGroundPhaseEngagedByRangedTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneGroundPhaseEngagedByRangedTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane ground phase engaged by ranged") {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbanePetsChaseFlyingBossOutOfBoundsTrigger : public KarazhanEncounterTrigger
{
public:
    NightbanePetsChaseFlyingBossOutOfBoundsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane pets chase flying boss out of bounds") {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneInFlightPhaseTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneInFlightPhaseTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane in flight phase") {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneBotWentOutOfBoundsTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneBotWentOutOfBoundsTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane bot went out of bounds") {}

protected:
    bool IsActiveInEncounter() override;
};

class NightbaneShouldManageTimersAndTrackersTrigger : public KarazhanEncounterTrigger
{
public:
    NightbaneShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : KarazhanEncounterTrigger(botAI, "nightbane should manage timers and trackers") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
