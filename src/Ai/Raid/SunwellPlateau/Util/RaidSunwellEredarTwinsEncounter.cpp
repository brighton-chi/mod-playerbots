/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <list>

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "Playerbots.h"
#include "RaidSunwellEredarTwinsEncounter.h"
#include "Spell.h"
#include "ThreatManager.h"

namespace SunwellHelpers
{

const Position SACROLASH_TANK_POSITION  = { 1804.255f, 630.193f, 33.404f };
const std::array<Position, 5> ALYTHESS_TANK_POSITIONS = {{
    { 1816.830f, 620.792f, 33.404f },
    { 1824.211f, 625.169f, 33.404f },
    { 1818.701f, 631.196f, 33.404f },
    { 1829.375f, 631.110f, 33.404f },
    { 1830.007f, 620.924f, 33.404f }
}};
const Position EREDAR_TWINS_P1_RANGED_POSITION =       { 1808.076f, 603.460f, 51.684f };
const Position EREDAR_TWINS_P2_MELEE_STACK_POSITION =  { 1814.327f, 625.645f, 33.404f };
const Position EREDAR_TWINS_P2_RANGED_STACK_POSITION = { 1805.587f, 625.653f, 33.404f };
const Position EREDAR_TWINS_RANGED_CONFLAG_POSITION =  { 1801.133f, 584.456f, 50.696f };
const Position EREDAR_TWINS_MELEE_CONFLAG_POSITION =   { 1814.337f, 607.771f, 33.404f };

std::unordered_map<ObjectGuid, uint8> alythessTankStep;
std::unordered_map<ObjectGuid, ObjectGuid> alythessTankLastBlazeGuid;
std::unordered_map<uint32, EredarTwinsIncomingConflagrationState>
    eredarTwinsIncomingConflagrationStates;

bool IsSacrolashTank(PlayerbotAI* botAI, Player* bot)
{
    return botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 1, true);
}

bool IsAlythessTank(PlayerbotAI* botAI, Player* bot)
{
    return botAI->IsAssistTankOfIndex(bot, 0, false);
}

bool ShouldHoldSacrolashThreat(
    PlayerbotAI* botAI, Player* bot, Unit* alythess, Unit* sacrolash)
{
    if (!alythess || !sacrolash || IsSacrolashTank(botAI, bot) ||
        IsAlythessTank(botAI, bot))
    {
        return false;
    }

    uint8 playerThreatEntries = 0;

    auto const threatList = sacrolash->GetThreatMgr().GetSortedThreatList();
    for (auto itr = threatList.begin();
         itr != threatList.end() && playerThreatEntries < 2; ++itr)
    {
        ThreatReference const* threatRef = *itr;
        if (!threatRef || !threatRef->IsAvailable())
            continue;

        Player* threatPlayer = threatRef->GetVictim()->ToPlayer();
        if (!threatPlayer || !threatPlayer->IsAlive())
            continue;

        ++playerThreatEntries;
        if (threatPlayer == bot)
            return true;
    }

    return false;
}

bool IsAlythessTankPositionSafe(Player* bot, const Position& position)
{
    constexpr float blazeDangerRadius = 4.5f;
    constexpr float blazeSearchRadius = 30.0f;

    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, blazeSearchRadius);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, blazeSearchRadius);

    for (GameObject* go : targets)
    {
        if (!go || go->GetEntry() != static_cast<uint32>(SunwellObjects::GO_BLAZE))
            continue;

        if (go->GetExactDist2d(
                position.GetPositionX(), position.GetPositionY()) <= blazeDangerRadius)
        {
            return false;
        }
    }

    return true;
}

bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot)
{
    if (!alythess)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    constexpr float blazeObjectRadius = 5.0f;

    GameObject* blazeObject = bot->FindNearestGameObject(
        static_cast<uint32>(SunwellObjects::GO_BLAZE),
        blazeObjectRadius);

    if (!blazeObject)
    {
        alythessTankLastBlazeGuid.erase(botGuid);
        return false;
    }

    const ObjectGuid blazeGuid = blazeObject->GetGUID();
    auto const lastBlaze = alythessTankLastBlazeGuid.find(botGuid);
    if (lastBlaze != alythessTankLastBlazeGuid.end() && lastBlaze->second == blazeGuid)
        return false;

    alythessTankLastBlazeGuid[botGuid] = blazeGuid;
    return true;
}

void RecordEredarTwinsIncomingConflagrationTarget(Player* target, uint32 durationMs)
{
    if (!target || !durationMs)
        return;

    const uint32 now = getMSTime();
    EredarTwinsIncomingConflagrationState& state =
        eredarTwinsIncomingConflagrationStates[target->GetInstanceId()];

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + EREDAR_TWINS_INCOMING_CONFLAGRATION_DELAY_MS;

    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
}

bool IsEredarTwinsConflagrationTarget(Unit* alythess, Player* bot)
{
    if (!bot)
        return false;

    auto const incomingItr = eredarTwinsIncomingConflagrationStates.find(bot->GetInstanceId());

    if (incomingItr == eredarTwinsIncomingConflagrationStates.end())
        return false;

    EredarTwinsIncomingConflagrationState const& state = incomingItr->second;
    const uint32 now = getMSTime();
    if (state.targetGuid != bot->GetGUID())
    {
        if (state.expireMs <= now)
            eredarTwinsIncomingConflagrationStates.erase(incomingItr);

        return false;
    }

    if (state.expireMs <= now)
    {
        eredarTwinsIncomingConflagrationStates.erase(incomingItr);
        return false;
    }

    return state.delayMs <= now;
}

}
