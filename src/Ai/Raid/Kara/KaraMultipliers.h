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
#include <string>

// General

class KarazhanEncounterMultiplier : public Multiplier
{
public:
    KarazhanEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, KaraHelpers::KARA_MAP_ID) ?
            GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
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

// Attumen the Huntsman

class AttumenTheHuntsmanDisableAutoTargetingMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanDisableAutoTargetingMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "attumen the huntsman disable auto targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AttumenTheHuntsmanStayStackedMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanStayStackedMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "attumen the huntsman stay stacked") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AttumenTheHuntsmanWaitForDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    AttumenTheHuntsmanWaitForDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "attumen the huntsman wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Maiden of Virtue

class MaidenOfVirtueDisableCombatFormationMoveMultiplier : public KarazhanEncounterMultiplier
{
public:
    MaidenOfVirtueDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "maiden of virtue disable combat formation move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MaidenOfVirtueSetGroundingTotemMultiplier : public KarazhanEncounterMultiplier
{
public:
    MaidenOfVirtueSetGroundingTotemMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "maiden of virtue set grounding totem") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// The Curator

class TheCuratorDisableTankAssistMultiplier : public KarazhanEncounterMultiplier
{
public:
    TheCuratorDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "the curator disable tank assist") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheCuratorDisableCombatFormationMoveMultiplier : public KarazhanEncounterMultiplier
{
public:
    TheCuratorDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "the curator disable combat formation move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Terestian Illhoof

class TerestianIllhoofDontDotFiendishImpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    TerestianIllhoofDontDotFiendishImpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "terestian illhoof don't dot fiendish imps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shade of Aran

class ShadeOfAranArcaneExplosionRunAwayMultiplier : public KarazhanEncounterMultiplier
{
public:
    ShadeOfAranArcaneExplosionRunAwayMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "shade of aran arcane explosion run away") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ShadeOfAranFlameWreathDisableMovementMultiplier : public KarazhanEncounterMultiplier
{
public:
    ShadeOfAranFlameWreathDisableMovementMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "shade of aran flame wreath disable movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Netherspite

class NetherspiteKeepBlockingBeamMultiplier : public KarazhanEncounterMultiplier
{
public:
    NetherspiteKeepBlockingBeamMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "netherspite keep blocking beam") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NetherspiteWaitForDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NetherspiteWaitForDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "netherspite wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Prince Malchezaar

class PrinceMalchezaarEnfeebleMultiplier : public KarazhanEncounterMultiplier
{
public:
    PrinceMalchezaarEnfeebleMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "prince malchezaar enfeeble") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Nightbane

class NightbaneDisablePetsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisablePetsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "nightbane disable pets") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneWaitForDpsMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneWaitForDpsMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "nightbane wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneDisableAvoidAoeMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "nightbane disable avoid aoe") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class NightbaneDisableMovementMultiplier : public KarazhanEncounterMultiplier
{
public:
    NightbaneDisableMovementMultiplier(PlayerbotAI* botAI)
        : KarazhanEncounterMultiplier(botAI, "nightbane disable movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
