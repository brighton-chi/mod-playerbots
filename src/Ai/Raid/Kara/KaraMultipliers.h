/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAMULTIPLIERS_H
#define PLAYERBOTS_KARAMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "Multiplier.h"
#include <optional>
#include <string>

// Shared

class KarazhanEncounterMultiplier : public Multiplier
{
public:
    KarazhanEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    KarazhanEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, KaraHelpers::KaraBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, KaraHelpers::KARA_MAP_ID, KaraHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, KaraHelpers::KARA_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;

    Unit* GetEncounterBoss()
    {
        return _boss ? KaraHelpers::GetKaraBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<KaraHelpers::KaraBosses> const _boss;
};

class KarazhanSetTremorTotemMultiplier : public Multiplier
{
public:
    KarazhanSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "karazhan set tremor totem") {}
    float GetValue(Action* action) override;
};

class KarazhanDelayDpsCooldownsMultiplier : public KarazhanEncounterMultiplier
{
public:
    KarazhanDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "karazhan delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class ManaWarpSaveStunsMultiplier : public Multiplier
{
public:
    ManaWarpSaveStunsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "mana warp save stuns") {}
    float GetValue(Action* action) override;
};

// Attumen the Huntsman

class AttumenTheHuntsmanDisableAutoTargetingMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanDisableAutoTargetingMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "attumen the huntsman disable auto targeting",
            KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AttumenTheHuntsmanStayStackedMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanStayStackedMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "attumen the huntsman stay stacked", KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AttumenTheHuntsmanWaitForDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanWaitForDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "attumen the huntsman wait for dps", KaraHelpers::KaraBosses::DATA_ATTUMEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Maiden of Virtue

class MaidenOfVirtueDisableCombatFormationMoveMultiplier : public KarazhanEncounterMultiplier
{
public:
    MaidenOfVirtueDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "maiden of virtue disable combat formation move",
            KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MaidenOfVirtueSetGroundingTotemMultiplier : public KarazhanEncounterMultiplier
{
public:
    MaidenOfVirtueSetGroundingTotemMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "maiden of virtue set grounding totem", KaraHelpers::KaraBosses::DATA_MAIDEN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// The Curator

class TheCuratorDisableTankAssistMultiplier : public KarazhanEncounterMultiplier
{
public:
    TheCuratorDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "the curator disable tank assist", KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheCuratorDisableCombatFormationMoveMultiplier : public KarazhanEncounterMultiplier
{
public:
    TheCuratorDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "the curator disable combat formation move",
            KaraHelpers::KaraBosses::DATA_CURATOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Terestian Illhoof

class TerestianIllhoofDontDotFiendishImpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    TerestianIllhoofDontDotFiendishImpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "terestian illhoof don't dot fiendish imps",
            KaraHelpers::KaraBosses::DATA_TERESTIAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shade of Aran

class ShadeOfAranArcaneExplosionRunAwayMultiplier : public KarazhanEncounterMultiplier
{
public:
    ShadeOfAranArcaneExplosionRunAwayMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "shade of aran arcane explosion run away", KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ShadeOfAranDisableReactiveMovementMultiplier : public KarazhanEncounterMultiplier
{
public:
    ShadeOfAranDisableReactiveMovementMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "shade of aran disable reactive movement",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ShadeOfAranFlameWreathDisableMovementMultiplier : public KarazhanEncounterMultiplier
{
public:
    ShadeOfAranFlameWreathDisableMovementMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "shade of aran flame wreath disable movement",
            KaraHelpers::KaraBosses::DATA_ARAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Netherspite

class NetherspiteKeepBlockingBeamMultiplier : public KarazhanEncounterMultiplier
{
public:
    NetherspiteKeepBlockingBeamMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "netherspite keep blocking beam", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NetherspiteWaitForDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NetherspiteWaitForDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "netherspite wait for dps", KaraHelpers::KaraBosses::DATA_NETHERSPITE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Prince Malchezaar

class PrinceMalchezaarEnfeebleMultiplier : public KarazhanEncounterMultiplier
{
public:
    PrinceMalchezaarEnfeebleMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "prince malchezaar enfeeble", KaraHelpers::KaraBosses::DATA_MALCHEZAAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Nightbane

class NightbaneDisablePetsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisablePetsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "nightbane disable pets", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneHoldDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneHoldDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "nightbane hold dps", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneDisableAvoidAoeMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "nightbane disable avoid aoe", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneDisableMovementMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisableMovementMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(
            botAI, "nightbane disable movement", KaraHelpers::KaraBosses::DATA_NIGHTBANE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
