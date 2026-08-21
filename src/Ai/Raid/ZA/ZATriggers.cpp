/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "amani'shi medicine man");
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
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonBossCastsStaticDisruptionTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) ||
        !AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetMap()->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return true;

    return !IsInStormWindow(it->second, std::time(nullptr));
}

bool AkilzonElectricalStormIncomingTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetMap()->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second, std::time(nullptr));
}

bool AkilzonBotsNeedToPrepareForElectricalStormTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
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
    return (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true)) &&
           AI_VALUE2(Unit*, "find target", "nalorakk");
}

bool NalorakkBossCastsSurgeTrigger::IsActive()
{
    return botAI->IsRanged(bot) && AI_VALUE2(Unit*, "find target", "nalorakk");
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
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossCastsFlameBreathTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) ||
        !AI_VALUE2(Unit*, "find target", "jan'alai") ||
        AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling"))
        return false;

    return !HasFireBombNearby(bot);
}

bool JanalaiBossSummoningFireBombsTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "jan'alai") && HasFireBombNearby(bot);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActive()
{
    if (!botAI->IsRangedDps(bot) ||
        !AI_VALUE2(Unit*, "find target", "jan'alai"))
        return false;

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

bool HalazziBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziBossSummonsSpiritLynxTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, true) && AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziDeterminingDpsTargetTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

// Hex Lord Malacrass

bool HexLordMalacrassPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass && malacrass->GetHealthPct() > ZA_PULL_COMPLETE_HP_PERCENT;
}

bool HexLordMalacrassDeterminingKillOrderTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "hex lord malacrass");
}

bool HexLordMalacrassBossIsChannelingWhirlwindTrigger::IsActive()
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass || !malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND)))
        return false;

    return !(botAI->IsTank(bot) && malacrass->GetVictim() == bot);
}

bool HexLordMalacrassBossHasSpellReflectionTrigger::IsActive()
{
    if (!botAI->IsCaster(bot))
        return false;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    return malacrass && malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_SPELL_REFLECTION));
}

bool HexLordMalacrassBossPlacedFreezingTrapTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return false;

    constexpr float searchRadius = 20.0f;
    return bot->FindNearestGameObject(Id(ZaObjects::GO_FREEZING_TRAP), searchRadius, true);
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
    if (!botAI->IsTank(bot))
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

    return !(botAI->IsTank(bot) && zuljin->GetVictim() == bot);
}

bool ZuljinBossIsSummoningCyclonesInEagleFormTrigger::IsActive()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE));
}

bool ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
