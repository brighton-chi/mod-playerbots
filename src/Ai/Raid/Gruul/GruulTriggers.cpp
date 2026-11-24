/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"

using namespace GruulHelpers;
using namespace EncounterHelpers;

// Shared

bool GruulsLairNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, GRUUL_MAP_ID);
}

bool GruulsLairClassRolesShouldBeAssignedTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, GRUUL_MAP_ID);
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActiveInEncounter()
{
    if (IsBlindeyeTank(bot))
        return GetMaulgarCouncilMember(bot, GruulNpcs::NPC_BLINDEYE_THE_SEER);

    if (IsOlmTank(bot))
        return GetMaulgarCouncilMember(bot, GruulNpcs::NPC_OLM_THE_SUMMONER);

    return IsMaulgarTank(bot);
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActiveInEncounter()
{
    return IsKroshMageTank(botAI) && GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KROSH_FIREHAND);
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActiveInEncounter()
{
    return IsKigglerMoonkinTank(botAI) &&
        GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KIGGLER_THE_CRAZED);
}

bool HighKingMaulgarShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !GetMaulgarCouncilMember(bot, GruulNpcs::NPC_OLM_THE_SUMMONER);

    if (IsBlindeyeTank(bot))
        return !GetMaulgarCouncilMember(bot, GruulNpcs::NPC_BLINDEYE_THE_SEER);

    if (IsKroshMageTank(botAI))
        return !GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KROSH_FIREHAND);

    if (IsKigglerMoonkinTank(botAI))
        return !GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KIGGLER_THE_CRAZED);

    return true;
}

bool HighKingMaulgarChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* maulgar = GetEncounterBoss();
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarShouldStandBackFromKroshTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(botAI))
        return false;

    return GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KROSH_FIREHAND);
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK && !GetWildFelStalkers(bot).empty();
}

bool HighKingMaulgarHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = GetMaulgarCouncilMember(bot, GruulNpcs::NPC_BLINDEYE_THE_SEER);
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_ENGAGED_HEALTH_PCT;
}

bool HighKingMaulgarPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* target = GetMaulgarPetTarget(bot, *pet);
    return target && pet->GetVictim() != target;
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot);
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot);
}

bool GruulTheDragonkillerBotInCaveInTrigger::IsActiveInEncounter()
{
    Unit* gruul = GetEncounterBoss();
    if (!gruul)
        return false;

    // The tank holding Gruul eats cave in. Moving him messes up the spread.
    if (gruul->GetVictim() == bot)
        return false;

    return IsInCaveIn(botAI);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActiveInEncounter()
{
    return HasGroundSlam(bot);
}
