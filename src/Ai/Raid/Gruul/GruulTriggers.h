/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULTRIGGERS_H
#define PLAYERBOTS_GRUULTRIGGERS_H

#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class GruulsLairEncounterTrigger : public Trigger
{
public:
    GruulsLairEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, GruulHelpers::GruulBosses boss)
        : Trigger(botAI, name), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, GruulHelpers::GRUUL_MAP_ID, GruulHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
    Unit* GetEncounterBoss() { return GruulHelpers::GetGruulsLairBoss(bot, _boss); }

private:
    GruulHelpers::GruulBosses const _boss;
};

class GruulsLairNoEncounterInProgressTrigger : public Trigger
{
public:
    GruulsLairNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul's lair no encounter in progress", 1000) {}
    bool IsActive() override;
};

class GruulsLairClassRolesShouldBeAssignedTrigger : public Trigger
{
public:
    GruulsLairClassRolesShouldBeAssignedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul's lair class roles should be assigned", 1000) {}
    bool IsActive() override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarThreeOgresNeedMeleeTanksTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar three ogres need melee tanks",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKroshNeedsMageTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKroshNeedsMageTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar krosh needs mage tank",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKigglerNeedsMoonkinTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKigglerNeedsMoonkinTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar kiggler needs moonkin tank",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarShouldAssignDpsPriorityTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar should assign dps priority",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarChannelingWhirlwindTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar channeling whirlwind",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarShouldStandBackFromKroshTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarShouldStandBackFromKroshTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar should stand back from krosh",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarWildFelStalkerSpawnedTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarWildFelStalkerSpawnedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar wild fel stalker spawned",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarHunterShouldMisdirectTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "high king maulgar hunter should misdirect",
            GruulHelpers::GruulBosses::DATA_HIGH_KING_MAULGAR) {}

protected:
    bool IsActiveInEncounter() override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerShouldBeTankedTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "gruul the dragonkiller should be tanked",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerRangedShouldSpreadTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "gruul the dragonkiller ranged should spread",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerBotInCaveInTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerBotInCaveInTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "gruul the dragonkiller bot in cave in",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(
            botAI, "gruul the dragonkiller incoming shatter",
            GruulHelpers::GruulBosses::DATA_GRUUL_THE_DRAGONKILLER) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
