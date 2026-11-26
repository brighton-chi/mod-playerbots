/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALMULTIPLIERS_H
#define PLAYERBOTS_HYJALMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Multiplier.h"
#include <optional>
#include <string>

// Shared

class HyjalEncounterMultiplier : public Multiplier
{
public:
    HyjalEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    HyjalEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, HyjalHelpers::HyjalBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, HyjalHelpers::HYJAL_MAP_ID, HyjalHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, HyjalHelpers::HYJAL_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
    Unit* GetEncounterBoss()
    {
        return _boss ? HyjalHelpers::GetHyjalBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<HyjalHelpers::HyjalBosses> const _boss;
};

class HyjalDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    HyjalDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hyjal delay dps cooldowns") {}
    float GetValue(Action* action) override;
};

class HyjalDisableDisperseAndTankFaceMultiplier : public HyjalEncounterMultiplier
{
public:
    HyjalDisableDisperseAndTankFaceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "hyjal disable disperse and tank face") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Rage Winterchill

class RageWinterchillMeleeControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    RageWinterchillMeleeControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "rage winterchill melee control avoidance",
            HyjalHelpers::HyjalBosses::DATA_RAGE_WINTERCHILL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class RageWinterchillRangedControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    RageWinterchillRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "rage winterchill ranged control avoidance",
            HyjalHelpers::HyjalBosses::DATA_RAGE_WINTERCHILL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Anetheron

class AnetheronDisableAssistTargetingMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronDisableAssistTargetingMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "anetheron disable assist targeting",
            HyjalHelpers::HyjalBosses::DATA_ANETHERON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronAvoidAccidentalInfernalAggroMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronAvoidAccidentalInfernalAggroMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "anetheron avoid accidental infernal aggro",
            HyjalHelpers::HyjalBosses::DATA_ANETHERON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronInfernalTargetRunToPositionMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronInfernalTargetRunToPositionMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "anetheron infernal target run to position",
            HyjalHelpers::HyjalBosses::DATA_ANETHERON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronControlMovementMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronControlMovementMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "anetheron control movement", HyjalHelpers::HyjalBosses::DATA_ANETHERON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronControlMisdirectionMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "anetheron control misdirection", HyjalHelpers::HyjalBosses::DATA_ANETHERON) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kaz'rogal

class KazrogalControlLowManaMovementMultiplier : public HyjalEncounterMultiplier
{
public:
    KazrogalControlLowManaMovementMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "kaz'rogal control low mana movement",
            HyjalHelpers::HyjalBosses::DATA_KAZROGAL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public HyjalEncounterMultiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "kaz'rogal keep aspect of the viper active",
            HyjalHelpers::HyjalBosses::DATA_KAZROGAL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Azgalor

class AzgalorDisableAssistTargetingAndPositioningMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorDisableAssistTargetingAndPositioningMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "azgalor disable assist targeting and positioning",
            HyjalHelpers::HyjalBosses::DATA_AZGALOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "azgalor doomed bot prioritize positioning",
            HyjalHelpers::HyjalBosses::DATA_AZGALOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorMeleeDpsControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorMeleeDpsControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "azgalor melee dps control avoidance",
            HyjalHelpers::HyjalBosses::DATA_AZGALOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorRangedControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "azgalor ranged control avoidance", HyjalHelpers::HyjalBosses::DATA_AZGALOR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Archimonde

class ArchimondeKeepTremorTotemMultiplier : public HyjalEncounterMultiplier
{
public:
    ArchimondeKeepTremorTotemMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "archimonde keep tremor totem", HyjalHelpers::HyjalBosses::DATA_ARCHIMONDE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ArchimondeControlDoomfireAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    ArchimondeControlDoomfireAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(
            botAI, "archimonde control doomfire avoidance",
            HyjalHelpers::HyjalBosses::DATA_ARCHIMONDE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
