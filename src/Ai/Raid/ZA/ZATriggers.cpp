/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"
#include "ZAHelpers.h"
#include <optional>

using namespace ZaHelpers;
using namespace EncounterHelpers;

// Shared

bool ZulAmanNoEncounterInProgressTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, ZA_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

bool ZulAmanHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetEncounterBoss();
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        FindThreatTarget(bot, "amani'shi medicine man");
}

// Akil'zon <Eagle Avatar>

bool AkilzonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonSpreadForStaticDisruptionTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    std::optional<uint32> const& stormTimer = ZaState(bot->GetInstanceId()).akilzonStormTimer;
    if (!stormTimer)
        return true;

    return !IsInStormWindow(*stormTimer);
}

bool AkilzonElectricalStormIncomingTrigger::IsActiveInEncounter()
{
    std::optional<uint32> const& stormTimer = ZaState(bot->GetInstanceId()).akilzonStormTimer;
    if (!stormTimer)
        return false;

    return IsInStormWindow(*stormTimer);
}

bool AkilzonShouldTrackElectricalStormTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

// Nalorakk <Bear Avatar>

bool NalorakkBothFormsShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool NalorakkSpreadForSurgeTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* janalai = GetEncounterBoss();
    return janalai && !IsJanalaiBombing(janalai);
}

bool JanalaiSpreadForFlameBreathTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* janalai = GetEncounterBoss();
    if (!janalai)
        return false;

    if (!GetJanalaiAdds(bot, ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING).empty())
        return false;

    return !IsJanalaiBombing(janalai);
}

bool JanalaiFireBombingTrigger::IsActiveInEncounter()
{
    Unit* janalai = GetEncounterBoss();
    if (!janalai)
        return false;

    return IsJanalaiBombing(janalai);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    Unit* janalai = GetEncounterBoss();
    if (!janalai || janalai->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
        return false;

    for (Unit* hatcher : GetJanalaiAdds(bot, ZaNpcs::NPC_AMANISHI_HATCHER))
    {
        if (bot->GetDistance(hatcher) <= ZA_CREATURE_SEARCH_RADIUS)
            return true;
    }

    return false;
}

// Halazzi <Lynx Avatar>

bool HalazziShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot);
}

bool HalazziSpiritLynxHasAppearedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool HalazziShouldFocusDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot);
}

// Hex Lord Malacrass

bool HexLordMalacrassShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot);
}

bool HexLordMalacrassChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* malacrass = GetEncounterBoss();
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassFreezingTrapPlacedTrigger::IsActiveInEncounter()
{
    return GetNearbyFreezingTrap(botAI);
}

// Zul'jin

bool ZuljinShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* zuljin = GetEncounterBoss();
    if (!zuljin)
        return false;

    // Eagle can't be tanked, and flexibility for the tank to turn Zul'jin during Dragonhawk
    // is preferable.
    return !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
        !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinChannelingWhirlwindInTrollFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = GetEncounterBoss();
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !PlayerbotAI::IsTank(bot) || zuljin->GetVictim() != bot;
}

bool ZuljinCreepingParalysisInBearFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* zuljin = GetEncounterBoss();
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_BEAR)))
        return false;

    return GetZuljinCreepingParalysisDispelTarget(bot);
}

bool ZuljinSummoningCyclonesInEagleFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = GetEncounterBoss();
    if (!zuljin)
        return false;

    if (zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)))
        return true;

    // The aura check is cleaner, but the health check here allows ranged to head to their
    // positions during the phase transition sequence.
    float const healthPct = zuljin->GetHealthPct();
    return healthPct <= 60.0f && healthPct > 40.0f;
}

bool ZuljinSpreadForDragonhawkAoeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = GetEncounterBoss();
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
