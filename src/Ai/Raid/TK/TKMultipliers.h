/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKMULTIPLIERS_H
#define PLAYERBOTS_TKMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "TKHelpers.h"
#include <optional>
#include <string>

// Shared

class TempestKeepEncounterMultiplier : public Multiplier
{
public:
    TempestKeepEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    TempestKeepEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, TkHelpers::TkBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(bot, TkHelpers::TK_MAP_ID, TkHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, TkHelpers::TK_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
    Unit* GetEncounterBoss()
    {
        return _boss ? TkHelpers::GetTkBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<TkHelpers::TkBosses> const _boss;
};

class TempestKeepDelayDpsCooldownsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    TempestKeepDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "tempest keep delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Al'ar <Phoenix God>

class AlarSuppressGapClosersMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarSuppressGapClosersMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar suppress gap closers", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarControlMovementMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarControlMovementMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar control movement", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarDisableAssistTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarDisableAssistTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar disable assist targeting", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarStayAwayFromRebirthMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar stay away from rebirth", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarControlTauntingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarControlTauntingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar control taunting", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarDisableTotemsBelowPlatformsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarDisableTotemsBelowPlatformsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "al'ar disable totems below platforms", TkHelpers::TkBosses::DATA_ALAR) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "void reaver maintain positions", TkHelpers::TkBosses::DATA_VOID_REAVER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableMeleeTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    HighAstromancerSolarianDisableMeleeTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "high astromancer solarian disable melee targeting",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighAstromancerSolarianWrathStayAwayMultiplier : public TempestKeepEncounterMultiplier
{
public:
    HighAstromancerSolarianWrathStayAwayMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "high astromancer solarian wrath stay away",
            TkHelpers::TkBosses::DATA_HIGH_ASTROMANCER_SOLARIAN) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderHoldDpsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderHoldDpsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider hold dps",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderKiteThaladredMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider kiting thaladred",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderControlMisdirectionMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider control misdirection",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier
    : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider disable warlock tank soulshatter",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderKeepDistanceFromCapernianMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderKeepDistanceFromCapernianMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider keep distance from capernian",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderManageWeaponTankingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderManageWeaponTankingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider manage weapon tanking",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderSuppressEquipUpgradeMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderSuppressEquipUpgradeMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider suppress equip upgrade",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderManageAssistTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderManageAssistTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider manage assist targeting",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderKeepMeleeInCombatMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderKeepMeleeInCombatMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider keep melee in combat",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderDisableDisperseMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider disable disperse",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderPrepareForPhase3Multiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderPrepareForPhase3Multiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider prepare for phase 3",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier
    : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider stay spread during gravity lapse",
            TkHelpers::TkBosses::DATA_KAELTHAS_SUNSTRIDER) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
