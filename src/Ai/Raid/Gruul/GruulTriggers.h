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

class GruulsLairEncounterTrigger : public Trigger
{
public:
    GruulsLairEncounterTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, GruulHelpers::GRUUL_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class GruulsLairNoEncounterInProgressTrigger : public Trigger
{
public:
    // Checked once a second rather than every tick. This trigger is true for the whole of trash and
    // all downtime, and it is the only gate on an ACTION_EMERGENCY + 10 action, so at the default
    // interval both it and the reset ran on every tick for every bot - together 0.43% of all bot AI
    // during trash. needCheck() skips the check rather than caching a stale answer, so the reset can
    // never fire during an encounter off an old reading, and landing within a second instead of
    // ~200ms is invisible for clearing raid icons and a stored spread position between pulls.
    GruulsLairNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul's lair no encounter in progress", 1000) {}
    bool IsActive() override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarThreeOgresNeedMeleeTanksTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar three ogres need melee tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKroshNeedsMageTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKroshNeedsMageTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar krosh needs mage tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKigglerNeedsMoonkinTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKigglerNeedsMoonkinTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar kiggler needs moonkin tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarDeterminingKillOrderTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarDeterminingKillOrderTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar determining kill order") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarBossChannelingWhirlwindTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarBossChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar boss channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarShouldStandBackFromKroshTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarShouldStandBackFromKroshTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar should stand back from krosh") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarWildFelStalkerSpawnedTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarWildFelStalkerSpawnedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar wild fel stalker spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarPullingOgreCouncilTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarPullingOgreCouncilTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar pulling ogre council") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarBossCastsIntimidatingRoarTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarBossCastsIntimidatingRoarTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar boss casts intimidating roar") {}

protected:
    bool IsActiveInEncounter() override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerShouldBeTankedTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerRangedShouldSpreadTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller incoming shatter") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
