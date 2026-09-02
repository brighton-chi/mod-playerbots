/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"

using namespace GruulHelpers;

// General

bool GruulsLairNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActiveInEncounter()
{
    if (IsBlindeyeTank(bot))
        return AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsOlmTank(bot))
        return AI_VALUE2(Unit*, "find target", "olm the summoner");

    return IsMaulgarTank(bot) && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActiveInEncounter()
{
    return IsKroshMageTank(bot) && AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActiveInEncounter()
{
    return IsKigglerMoonkinTank(bot) && AI_VALUE2(Unit*, "find target", "kiggler the crazed");
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return false;

    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !AI_VALUE2(Unit*, "find target", "olm the summoner");

    if (IsBlindeyeTank(bot))
        return !AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsKroshMageTank(bot))
        return !AI_VALUE2(Unit*, "find target", "krosh firehand");

    if (IsKigglerMoonkinTank(bot))
        return !AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return true;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarShouldStandBackFromKroshTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK && !GetNearbyWildFelStalkers(botAI).empty();
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_ENGAGED_HEALTH_PCT;
}

bool HighKingMaulgarBossCastsIntimidatingRoarTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActiveInEncounter()
{
    return HasGroundSlam(bot);
}
