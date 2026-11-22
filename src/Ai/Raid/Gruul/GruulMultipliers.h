/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULMULTIPLIERS_H
#define PLAYERBOTS_GRUULMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Multiplier.h"
#include <optional>
#include <string>

// Shared

class GruulsLairEncounterMultiplier : public Multiplier
{
public:
    GruulsLairEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    GruulsLairEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, GruulHelpers::GruulBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, GruulHelpers::GRUUL_MAP_ID, GruulHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, GruulHelpers::GRUUL_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
    Unit* GetEncounterBoss()
    {
        return _boss ? GruulHelpers::GetGruulsLairBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<GruulHelpers::GruulBosses> const _boss;
};

class GruulsLairDelayDpsCooldownsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulsLairDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul's lair delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarControlTankActionsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlTankActionsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar control tank actions",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarRestrictTauntingMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarRestrictTauntingMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar restrict taunting",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarDisableDpsAssistMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarDisableDpsAssistMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar disable dps assist",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar avoid whirlwind",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarLetMageTankStealSpellShieldMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarLetMageTankStealSpellShieldMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar let mage tank steal spell shield",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarControlMisdirectionMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar control misdirection",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarControlMageTankActionsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlMageTankActionsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "high king maulgar control mage tank actions",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerControlTankMovementMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerControlTankMovementMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "gruul the dragonkiller control tank movement",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerStaySpreadForShatterMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerStaySpreadForShatterMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "gruul the dragonkiller stay spread for shatter",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerControlAvoidanceMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "gruul the dragonkiller control avoidance",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerHoldWhileSnaredMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerHoldWhileSnaredMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(
            botAI, "gruul the dragonkiller hold while snared",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
