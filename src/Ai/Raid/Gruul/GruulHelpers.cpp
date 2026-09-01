/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "AiFactory.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include <algorithm>
#include <list>

namespace GruulHelpers
{

bool IsMaulgarTank(Player* bot)
{
    // Note: IsMainTank() is not necessarily a tank (by either strategy or spec). It can be anybody
    // with the main tank flag. Raid strategies will have problems with non-tank main tanks so this
    // assumes you are using a real tank for your main tank.
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsBlindeyeTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

ObjectGuid FindKroshMageTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    // If an assistant Mage (player or bot) is found, return immediately.
    // Otherwise, return the bot Mage with the highest HP as fallback.
    Player* highestHpBotMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMage || hp > highestHp)
        {
            highestHpBotMage = member;
            highestHp = hp;
        }
    }

    return highestHpBotMage ? highestHpBotMage->GetGUID() : ObjectGuid::Empty;
}

Player* GetKroshMageTank(Player* bot)
{
    // Every bot in the raid walks to the same answer, and the value system caches per bot, so this
    // does not collapse 24 walks into one - it collapses each bot's walk-per-tick into one per
    // interval, which is where the cost was.
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    ObjectGuid const guid = botAI
        ? botAI->GetAiObjectContext()->GetValue<ObjectGuid>("high king maulgar krosh mage tank")->Get()
        : FindKroshMageTankGuid(bot);

    // Resolved on every call rather than held as a pointer: the tank can log out or leave inside
    // the interval, and a cached Player* would outlive them.
    return guid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(guid);
}

bool IsKroshMageTank(Player* bot)
{
    return bot->getClass() == CLASS_MAGE && GetKroshMageTank(bot) == bot;
}

ObjectGuid FindKigglerMoonkinTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    // If an assistant Balance Druid (player or bot) is found, return immediately.
    // Otherwise, return the bot Balance Druid with the highest HP as fallback.
    Player* highestHpBotMoonkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_DRUID ||
            AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMoonkin || hp > highestHp)
        {
            highestHpBotMoonkin = member;
            highestHp = hp;
        }
    }

    return highestHpBotMoonkin ? highestHpBotMoonkin->GetGUID() : ObjectGuid::Empty;
}

Player* GetKigglerMoonkinTank(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    ObjectGuid const guid = botAI
        ? botAI->GetAiObjectContext()->GetValue<ObjectGuid>("high king maulgar kiggler moonkin tank")->Get()
        : FindKigglerMoonkinTankGuid(bot);

    return guid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(guid);
}

bool IsKigglerMoonkinTank(Player* bot)
{
    return bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(bot) == bot;
}

bool HasGroundSlam(Player* bot)
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_1)) ||
        bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_2));
}

GuidVector FindNearbyWildFelStalkerGuids(Player* bot)
{
    // A grid search is the most expensive check in the module, and the instance strategy can
    // outlive leaving the instance (e.g. after a server reset), so gate it on the map rather than
    // running it wherever the bot happens to be.
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return {};

    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(GruulNpcs::NPC_WILD_FEL_STALKER), WILD_FEL_STALKER_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    // The search walks cells outward from the searcher's own cell (Cell::VisitObjects ->
    // ComputeCellCoord on the caller's position), so two bots standing apart get the same stalkers
    // back in a different order. Sorting makes the list canonical, which is what lets the banish
    // assignment pair warlock i with stalker i across the whole raid.
    std::sort(guids.begin(), guids.end(), [](ObjectGuid const& lhs, ObjectGuid const& rhs)
    {
        return lhs.GetCounter() < rhs.GetCounter();
    });

    return guids;
}

std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* botAI)
{
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>(
            "high king maulgar wild fel stalkers")->RefGet();

    // The list is up to WILD_FEL_STALKER_CACHE_INTERVAL_MS stale, so a stalker may have died since
    // the scan. Dropping it here re-compacts the assignment rather than leaving a hole in it.
    std::vector<Unit*> felStalkers;
    felStalkers.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* felStalker = botAI->GetUnit(guid);
        if (felStalker && felStalker->IsAlive())
            felStalkers.push_back(felStalker);
    }

    return felStalkers;
}

}
