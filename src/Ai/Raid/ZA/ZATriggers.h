/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZATRIGGERS_H
#define PLAYERBOTS_ZATRIGGERS_H

#include "EncounterHelpers.h"
#include "Trigger.h"
#include "ZAHelpers.h"
#include <string>

// General

class ZulAmanEncounterTrigger : public Trigger
{
public:
    ZulAmanEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, ZaHelpers::ZA_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

// General

class ZulAmanNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    ZulAmanNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "zul'aman no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Trash

class AmanishiMedicineManSummonedWardTrigger : public Trigger
{
public:
    AmanishiMedicineManSummonedWardTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "amani'shi medicine man summoned ward") {}
    bool IsActive() override;
};

class ZulAmanPullingBossTrigger : public Trigger
{
public:
    ZulAmanPullingBossTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "zul'aman pulling boss") {}
    bool IsActive() override;
};

// Akil'zon <Eagle Avatar>

class AkilzonBossEngagedByTanksTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon boss engaged by tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonSpreadForStaticDisruptionTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonSpreadForStaticDisruptionTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon spread for static disruption") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonElectricalStormIncomingTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonElectricalStormIncomingTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon electrical storm incoming") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonBotsNeedToPrepareForElectricalStormTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonBotsNeedToPrepareForElectricalStormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon bots need to prepare for electrical storm") {}

protected:
    bool IsActiveInEncounter() override;
};

// Nalorakk <Bear Avatar>

class NalorakkBossSwitchesFormsTrigger : public ZulAmanEncounterTrigger
{
public:
    NalorakkBossSwitchesFormsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "nalorakk boss switches forms") {}

protected:
    bool IsActiveInEncounter() override;
};

class NalorakkSpreadForSurgeTrigger : public ZulAmanEncounterTrigger
{
public:
    NalorakkSpreadForSurgeTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "nalorakk spread for surge") {}

protected:
    bool IsActiveInEncounter() override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiBossEngagedByTanksTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai boss engaged by tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiSpreadForFlameBreathTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiSpreadForFlameBreathTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai spread for flame breath") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiBossSummoningFireBombsTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiBossSummoningFireBombsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai boss summoning fire bombs") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiAmanishiHatchersSpawnedTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiAmanishiHatchersSpawnedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai amani'shi hatchers spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

// Halazzi <Lynx Avatar>

class HalazziShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class HalazziSpiritLynxHasAppearedTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziSpiritLynxHasAppearedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi spirit lynx has appeared") {}

protected:
    bool IsActiveInEncounter() override;
};

class HalazziShouldFocusDpsTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi should focus dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Hex Lord Malacrass

class HexLordMalacrassShouldPrioritizeAddsTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassShouldPrioritizeAddsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass should prioritize adds") {}

protected:
    bool IsActiveInEncounter() override;
};

class HexLordMalacrassBossIsChannelingWhirlwindTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassBossIsChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass boss is channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class HexLordMalacrassBossPlacedFreezingTrapTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassBossPlacedFreezingTrapTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass boss placed freezing trap") {}

protected:
    bool IsActiveInEncounter() override;
};

// Zul'jin

class ZuljinBossEngagedByTanksTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin boss engaged by tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinBossIsChannelingWhirlwindInTrollFormTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinBossIsChannelingWhirlwindInTrollFormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin boss is channeling whirlwind in troll form") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinBossIsSummoningCyclonesInEagleFormTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinBossIsSummoningCyclonesInEagleFormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin boss is summoning cyclones in eagle form") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinSpreadForDragonhawkAoeTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinSpreadForDragonhawkAoeTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin spread for dragonhawk aoe") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
