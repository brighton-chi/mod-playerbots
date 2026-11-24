/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <limits>
#include <mutex>
#include <unordered_map>

using namespace EncounterHelpers;

namespace GruulHelpers
{

// Shared

namespace
{

std::mutex gruulStateMutex;
std::unordered_map<uint32, GruulInstanceState> gruulStates;

} // end anonymous namespace (Shared)

Unit* GetGruulsLairBoss(Player* bot, GruulBosses boss)
{
    if (boss == GruulBosses::DATA_HIGH_KING_MAULGAR)
        return GetMaulgarCouncilMember(bot, GruulNpcs::NPC_HIGH_KING_MAULGAR);

    if (!IsBossInProgress(bot, GRUUL_MAP_ID, Id(boss)))
        return nullptr;

    return FindThreatTarget(bot, "gruul the dragonkiller");
}

// High King Maulgar

bool IsMaulgarTank(Player* bot)
{
    // Note: IsMainTank() is not necessarily a tank (by either strategy or spec). It can be anybody
    // with the main tank flag. Raid strategies will have problems with non-tank main tanks so this
    // assumes you are using a real tank for your main tank.
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    // Although passing true for indexLivingOnly means a death will swap the Blindeye tank to Olm,
    // this is intended since Blindeye dies first and Olm is more important to control.
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsBlindeyeTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool UpdateGruulClassRoles(PlayerbotAI* botAI)
{
    GruulInstanceState& state = GruulState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAssignments(botAI, GRUUL_MAP_ID, {
        { KROSH_MAGE_TANK_ROLE, state.kroshMageTank },
        { KIGGLER_MOONKIN_TANK_ROLE, state.kigglerMoonkinTank },
    });
}

bool UpdateMaulgarClassRoleAnnouncement(PlayerbotAI* botAI)
{
    GruulInstanceState& state = GruulState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAnnouncement(
        botAI, Id(GruulNpcs::NPC_HIGH_KING_MAULGAR), MAULGAR_SPAWN_POSITION,
        MAULGAR_CLASS_ROLE_ANNOUNCE_DISTANCE, MAULGAR_CLASS_ROLE_REARM_DISTANCE, {
            { KROSH_MAGE_TANK_ROLE, state.kroshMageTank },
            { KIGGLER_MOONKIN_TANK_ROLE, state.kigglerMoonkinTank },
        });
}

Player* GetKroshMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->kroshMageTank) : nullptr;
}

bool IsKroshMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_MAGE)
        return false;

    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->kroshMageTank);
}

Player* GetKigglerMoonkinTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->kigglerMoonkinTank) : nullptr;
}

bool IsKigglerMoonkinTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_DRUID)
        return false;

    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->kigglerMoonkinTank);
}

GuidVector FindWildFelStalkerGuids(Player* bot)
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return {};

    GuidVector guids = FindCreatureGuidsAround(
        bot, MAULGAR_SPAWN_POSITION, WILD_FEL_STALKER_SEARCH_RADIUS,
        { Id(GruulNpcs::NPC_WILD_FEL_STALKER) });

    std::sort(guids.begin(), guids.end(), [](ObjectGuid const& lhs, ObjectGuid const& rhs)
    {
        return lhs.GetCounter() < rhs.GetCounter();
    });

    return guids;
}

std::vector<Unit*> GetWildFelStalkers(Player* bot)
{
    GuidVector const& guids = GetInstanceCreatureList(
        GruulState(bot->GetInstanceId()).wildFelStalkers, WILD_FEL_STALKER_CACHE_INTERVAL_MS, bot,
        FindWildFelStalkerGuids);

    std::vector<Unit*> felStalkers;
    felStalkers.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Creature* felStalker = ObjectAccessor::GetCreature(*bot, guid);
        if (felStalker && felStalker->IsAlive())
            felStalkers.push_back(felStalker);
    }

    return felStalkers;
}

GuidVector FindMaulgarCouncilGuids(Player* bot)
{
    std::vector<uint32> const entries = {
        Id(GruulNpcs::NPC_HIGH_KING_MAULGAR),
        Id(GruulNpcs::NPC_KROSH_FIREHAND),
        Id(GruulNpcs::NPC_OLM_THE_SUMMONER),
        Id(GruulNpcs::NPC_KIGGLER_THE_CRAZED),
        Id(GruulNpcs::NPC_BLINDEYE_THE_SEER),
    };

    return FindCreatureGuidsAround(
        bot, MAULGAR_SPAWN_POSITION, MAULGAR_COUNCIL_SEARCH_RADIUS, entries);
}

Unit* GetMaulgarCouncilMember(Player* bot, GruulNpcs npc)
{
    if (!IsBossInProgress(bot, GRUUL_MAP_ID, Id(GruulBosses::DATA_HIGH_KING_MAULGAR)))
        return nullptr;

    GuidVector const& guids = GetInstanceCreatureList(
        GruulState(bot->GetInstanceId()).maulgarCouncil, MAULGAR_COUNCIL_CACHE_INTERVAL_MS, bot,
        FindMaulgarCouncilGuids);
    for (ObjectGuid const& guid : guids)
    {
        if (guid.GetEntry() != Id(npc))
            continue;

        Creature* member = ObjectAccessor::GetCreature(*bot, guid);
        return member && member->IsAlive() && member->IsInWorld() ? member : nullptr;
    }

    return nullptr;
}

// Pets follow the ranged kill order. Krosh can be attacked because he casts Blast Wave only when
// a player is within 15 yards, not a pet.
Unit* GetMaulgarPetTarget(Player* bot, Creature* pet)
{
    Unit* target = nullptr;
    if (Unit* blindeye = GetMaulgarCouncilMember(bot, GruulNpcs::NPC_BLINDEYE_THE_SEER))
        target = blindeye;
    else if (Unit* olm = GetMaulgarCouncilMember(bot, GruulNpcs::NPC_OLM_THE_SUMMONER))
        target = olm;
    else if (Unit* krosh = GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KROSH_FIREHAND))
        target = krosh;
    else if (Unit* kiggler = GetMaulgarCouncilMember(bot, GruulNpcs::NPC_KIGGLER_THE_CRAZED))
        target = kiggler;
    else if (Unit* maulgar = GetGruulsLairBoss(bot, GruulBosses::DATA_HIGH_KING_MAULGAR))
        target = maulgar;

    return target && pet->IsValidAttackTarget(target) ? target : nullptr;
}

// Gruul the Dragonkiller

namespace
{
std::vector<Position> const& GetCaveInPositions(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("gruul the dragonkiller cave in")->RefGet();
}
} // end anonymous namespace (Gruul the Dragonkiller)

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool)
{
    Player* bot = botAI->GetBot();
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearCaveIn(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        if (bot->GetExactDist2d(position) < radius)
            return true;
    }

    return false;
}

bool IsInCaveIn(PlayerbotAI* botAI)
{
    return IsNearCaveIn(botAI, CAVE_IN_RADIUS);
}

bool HasGroundSlam(Player* bot)
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_1)) ||
        bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_2));
}

// Shared encounter state

GruulInstanceState& GruulState(uint32 instanceId)
{
    std::lock_guard lock(gruulStateMutex);
    return gruulStates[instanceId];
}

GruulInstanceState* FindGruulState(uint32 instanceId)
{
    std::lock_guard lock(gruulStateMutex);
    auto it = gruulStates.find(instanceId);
    return it != gruulStates.end() ? &it->second : nullptr;
}

void GruulResetInstance(uint32 instanceId)
{
    GruulInstanceState* state = FindGruulState(instanceId);
    if (!state)
        return;

    ResetInstanceCreatureList(state->maulgarCouncil);
    ResetInstanceCreatureList(state->wildFelStalkers);
}

}
