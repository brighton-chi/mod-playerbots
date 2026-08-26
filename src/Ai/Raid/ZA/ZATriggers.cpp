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

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "amani'shi medicine man");
}

// Akil'zon <Eagle Avatar>

bool AkilzonPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    return akilzon && akilzon->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

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

bool NalorakkPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    return nalorakk && nalorakk->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

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

bool JanalaiPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    return janalai && janalai->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

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

bool HalazziPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    return halazzi && halazzi->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

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

bool HexLordMalacrassPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass && malacrass->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

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

bool ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    float hp = zuljin->GetHealthPct();

    return (hp <= 100.0f && hp > ZA_PULL_COMPLETE_HP_PERCENT) ||
           (hp <= 80.0f && hp > 75.0f &&
            zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_BEAR))) ||
           (hp <= 40.0f && hp > 35.0f &&
            zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_LYNX))) ||
           (hp <= 20.0f && hp > 15.0f &&
            zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)));
}

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
