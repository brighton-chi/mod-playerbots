/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;
using namespace EncounterHelpers;

// General

bool ZulAmanNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != ZA_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance || instance->IsEncounterInProgress())
        return false;

    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

// The misdirect on the pull is the same job on every boss, and every Zul'Aman boss - and nothing
// else in the instance - runs a BossAI, so "boss target" resolves whichever one the raid is on.
bool ZulAmanPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE(Unit*, "boss target");
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "amani'shi medicine man");
}

// Akil'zon <Eagle Avatar>

bool AkilzonBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return true;

    return !IsInStormWindow(it->second);
}

bool AkilzonElectricalStormIncomingTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second);
}

bool AkilzonBotsNeedToPrepareForElectricalStormTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    return AI_VALUE2(Unit*, "find target", "akil'zon");
}

// Nalorakk <Bear Avatar>

bool NalorakkBossSwitchesFormsTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "nalorakk"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "nalorakk");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    if (AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "jan'alai") && HasFireBombNearby(bot);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActive()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    // Just need to find one Hatcher to fire the trigger
    constexpr float searchRadius = 40.0f;
    return bot->FindNearestCreature(Id(ZaNpcs::NPC_AMANISHI_HATCHER), searchRadius);
}

// Halazzi <Lynx Avatar>

bool HalazziShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziSpiritLynxHasAppearedTrigger::IsActive()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziShouldFocusDpsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

// Hex Lord Malacrass

bool HexLordMalacrassShouldPrioritizeAddsTrigger::IsActive()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "hex lord malacrass");
}

bool HexLordMalacrassBossIsChannelingWhirlwindTrigger::IsActive()
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassBossPlacedFreezingTrapTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return false;

    return bot->FindNearestGameObject(
        Id(ZaObjects::GO_FREEZING_TRAP), ZA_FREEZING_TRAP_SEARCH_RADIUS, true);
}

// Zul'jin

bool ZuljinBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
           !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinBossIsChannelingWhirlwindInTrollFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !(PlayerbotAI::IsTank(bot) && zuljin->GetVictim() == bot);
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE));
}

bool ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
