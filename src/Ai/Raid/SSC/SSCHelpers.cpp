/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCValueContext.h"
#include "Timer.h"
#include <limits>
#include <list>

namespace SscHelpers
{

// Trash

std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, std::string const& value)
{
    return botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet();
}

bool GetToxicPoolPosition(PlayerbotAI* botAI, Position& toxicPool)
{
    std::vector<Position> const& positions =
        GetCachedHazardPositions(botAI, "ssc toxic pool");
    if (positions.empty())
        return false;

    toxicPool = positions.front();
    return true;
}

bool IsNearToxicPool(PlayerbotAI* botAI, float radius)
{
    Position toxicPool;
    return GetToxicPoolPosition(botAI, toxicPool) &&
        botAI->GetBot()->GetExactDist2d(toxicPool) < radius;
}

bool IsInToxicPool(PlayerbotAI* botAI)
{
    return IsNearToxicPool(botAI, TOXIC_POOL_RADIUS);
}

// Hydross the Unstable <Duke of Currents>

std::unordered_map<uint32, uint32> hydrossFrostDpsWaitTimer;
std::unordered_map<uint32, uint32> hydrossNatureDpsWaitTimer;
std::unordered_map<uint32, uint32> hydrossChangeToFrostPhaseTimer;
std::unordered_map<uint32, uint32> hydrossChangeToNaturePhaseTimer;

bool HasMarkOfHydrossAt100Percent(Player* bot)
{
    return bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_100)) ||
        bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_250)) ||
        bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_500));
}

bool HasNoMarkOfHydross(Player* bot)
{
    return !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_10)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_25)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_50)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_100)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_250)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_500));
}

bool HasMarkOfCorruptionAt100Percent(Player* bot)
{
    return bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_100)) ||
        bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_250)) ||
        bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_500));
}

bool HasNoMarkOfCorruption(Player* bot)
{
    return !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_10)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_25)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_50)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_100)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_250)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_500));
}

// The Lurker Below

std::unordered_map<uint32, uint32> lurkerSpoutTimer;
std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;

bool IsLurkerCastingSpout(Unit* lurker)
{
    if (!lurker || !lurker->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* currentSpell = lurker->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell)
        return false;

    uint32 spellId = currentSpell->m_spellInfo->Id;
    bool isSpout = spellId == Id(SscSpells::SPELL_SPOUT_VISUAL);

    return isSpout;
}

// Leotheras the Blind

std::unordered_map<uint32, uint32> leotherasHumanFormDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasDemonFormDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasFinalPhaseDpsWaitTimer;

Creature* GetLeotherasHuman(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), searchRadius);

    if (leotheras && leotheras->IsInCombat() &&
        !leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

Creature* GetPhase2LeotherasDemon(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), searchRadius);

    if (leotheras && leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

Creature* GetPhase3LeotherasDemon(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    return bot->FindNearestCreature(Id(SscNpcs::NPC_SHADOW_OF_LEOTHERAS), searchRadius);
}

Creature* GetActiveLeotherasDemon(Player* bot)
{
    Creature* phase2 = GetPhase2LeotherasDemon(bot);
    Creature* phase3 = GetPhase3LeotherasDemon(bot);
    return phase2 ? phase2 : phase3;
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
Player* GetLeotherasDemonFormTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
            continue;

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

// Fathom-Lord Karathress

std::unordered_map<uint32, uint32> karathressDpsWaitTimer;

// Morogrim Tidewalker

std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

// Lady Vashj <Coilfang Matron>

std::unordered_map<ObjectGuid, bool> hasReachedVashjRangedPosition;
std::unordered_map<uint32, ObjectGuid> nearestTriggerGuid;
std::unordered_map<ObjectGuid, Position> intendedLineup;
std::unordered_map<uint32, uint32> lastImbueAttempt;
std::unordered_map<ObjectGuid, uint32> lastCoreInInventoryTime;

bool IsMainTankInSameSubgroup(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    uint8 botSubGroup = group->GetMemberGroup(bot->GetGUID());
    if (botSubGroup >= MAX_RAID_SUBGROUPS)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (group->GetMemberGroup(member->GetGUID()) != botSubGroup)
            continue;

        if (PlayerbotAI::IsMainTank(member))
            return true;
    }

    return false;
}

bool IsLadyVashjInPhase1(PlayerbotAI* botAI)
{
    Unit* vashj =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();

    return vashj && vashj->GetHealthPct() > 70.0f;
}

bool IsLadyVashjInPhase2(PlayerbotAI* botAI)
{
    Unit* vashj =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();

    return vashj && vashj->GetHealthPct() <= 70.0f &&
        vashj->HasAura(Id(SscSpells::SPELL_MAGIC_BARRIER));
}

bool IsLadyVashjInPhase3(PlayerbotAI* botAI)
{
    Unit* vashj =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();

    return vashj && vashj->GetHealthPct() <= 70.0f &&
        !vashj->HasAura(Id(SscSpells::SPELL_MAGIC_BARRIER));
}

// This can just be replaced by a target exclusion of Vashj for Phase 2 I think
bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* botAI)
{
    if (!unit || !unit->IsAlive())
        return false;

    uint32 entry = unit->GetEntry();

    if (IsLadyVashjInPhase2(botAI))
    {
        return entry == Id(SscNpcs::NPC_TAINTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_COILFANG_ELITE) ||
            entry == Id(SscNpcs::NPC_COILFANG_STRIDER);
    }
    else if (IsLadyVashjInPhase3(botAI))
    {
        return entry == Id(SscNpcs::NPC_TAINTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_COILFANG_ELITE) ||
            entry == Id(SscNpcs::NPC_COILFANG_STRIDER) ||
            entry == Id(SscNpcs::NPC_TOXIC_SPOREBAT) ||
            entry == Id(SscNpcs::NPC_LADY_VASHJ);
    }

    return false;
}

Player* GetDesignatedCoreLooter(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* leader = nullptr;
    ObjectGuid leaderGuid = group->GetLeaderGUID();
    if (!leaderGuid.IsEmpty())
        leader = ObjectAccessor::FindPlayer(leaderGuid);

    // If cheats are disabled, the group leader will be the designated looter
    if (!botAI->HasCheat(BotCheatMask::raid))
        return leader;

    // Priority: (1) assistant melee DPS, (2) other melee DPS, (3) any ranged DPS
    Player* meleeDpsAssistant = nullptr;
    Player* meleeDps = nullptr;
    Player* rangedDps = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == leader || !GET_PLAYERBOT_AI(member))
            continue;

        if (!meleeDpsAssistant && PlayerbotAI::IsMelee(member) &&
            PlayerbotAI::IsDps(member) && group->IsAssistant(member->GetGUID()))
        {
            meleeDpsAssistant = member;
            break;
        }

        if (!meleeDps && PlayerbotAI::IsMelee(member) && PlayerbotAI::IsDps(member))
            meleeDps = member;

        if (!rangedDps && PlayerbotAI::IsRangedDps(member))
            rangedDps = member;
    }

    if (meleeDpsAssistant)
        return meleeDpsAssistant;
    if (meleeDps)
        return meleeDps;
    if (rangedDps)
        return rangedDps;
    return leader;
}

Player* GetFirstTaintedCorePasser(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || !member->IsAlive())
            continue;

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsAssistHealOfIndex(member, 0, true))
            return member;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || !member->IsAlive())
            continue;

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            return member;
    }

    return nullptr;
}

Player* GetSecondTaintedCorePasser(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsAssistHealOfIndex(member, 0, true))
            return member;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            return member;
    }

    return nullptr;
}

Player* GetThirdTaintedCorePasser(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            member == secondCorePasser || !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsAssistHealOfIndex(member, 0, true))
            return member;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            member == secondCorePasser || !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            return member;
    }

    return nullptr;
}

Player* GetFourthTaintedCorePasser(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(botAI, bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            member == secondCorePasser || member == thirdCorePasser || !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsAssistHealOfIndex(member, 0, true))
            return member;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == designatedLooter || member == firstCorePasser ||
            member == secondCorePasser || member == thirdCorePasser || !member->IsAlive())
        {
            continue;
        }

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            return member;
    }

    return nullptr;
}

std::array<Player*, 5> GetCoreHandlers(PlayerbotAI* botAI, Player* bot)
{
    return
    {
        GetDesignatedCoreLooter(botAI, bot),
        GetFirstTaintedCorePasser(botAI, bot),
        GetSecondTaintedCorePasser(botAI, bot),
        GetThirdTaintedCorePasser(botAI, bot),
        GetFourthTaintedCorePasser(botAI, bot)
    };
}

// Checks if any bot from earlier in the passing sequence has the Tainted Core or
// had it within the prior 3 seconds so the chain is not broken when the Core is in transit
bool AnyRecentCoreInInventory(PlayerbotAI* botAI, Player* bot)
{
    Unit* vashj =
        botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
    if (!vashj)
        return false;

    auto coreHandlers = GetCoreHandlers(botAI, bot);

    int8 myIndex = -1;
    for (int8 i = 0; i < 5; ++i)
        if (coreHandlers[i] && coreHandlers[i] == bot)
            myIndex = i;

    if (myIndex == -1)
        return false;

    const uint32 now = getMSTime();
    constexpr uint32 lookbackMs = 3 * IN_MILLISECONDS;

    for (int8 i = 0; i <= myIndex; ++i)
    {
        Player* handler = coreHandlers[i];
        if (!handler)
            continue;

        if (handler->HasItemCount(Id(SscItems::ITEM_TAINTED_CORE), 1, false))
            return true;

        auto it = lastCoreInInventoryTime.find(handler->GetGUID());
        if (it != lastCoreInInventoryTime.end() &&
            getMSTimeDiff(it->second, now) <= lookbackMs)
            return true;
    }

    return false;
}

// Get the positions of all active Shield Generators by their database GUIDs
std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(
    Map* map, std::vector<uint32> const& generatorDbGuids)
{
    std::vector<GeneratorInfo> generators;
    if (!map)
        return generators;

    for (uint32 dbGuid : generatorDbGuids)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
        if (bounds.first == bounds.second)
            continue;

        GameObject* go = bounds.first->second;
        if (!go || go->GetGoState() != GO_STATE_READY)
            continue;

        GeneratorInfo info;
        info.guid = go->GetGUID();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        generators.push_back(info);
    }

    return generators;
}

// Returns the nearest active Shield Generator to the bot
// Active generators are powered by NPC_WORLD_INVISIBLE_TRIGGER creatures,
// which despawn after use
Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Unit* reference)
{
    if (!reference)
        return nullptr;

    std::list<Creature*> triggers;
    constexpr float searchRange = 150.0f;
    reference->GetCreatureListWithEntryInGrid(
        triggers, Id(SscNpcs::NPC_WORLD_INVISIBLE_TRIGGER), searchRange);

    Creature* nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();

    for (Creature* creature : triggers)
    {
        if (!creature->IsAlive())
            continue;

        float dist = reference->GetDistance(creature);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = creature;
        }
    }

    return nearest;
}

GeneratorInfo const* GetNearestGeneratorToBot(
    Player* bot, std::vector<GeneratorInfo> const& generators)
{
    if (generators.empty())
        return nullptr;

    GeneratorInfo const* nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();

    for (auto const& gen : generators)
    {
        float dist = bot->GetExactDist(gen.x, gen.y, gen.z);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = &gen;
        }
    }

    return nearest;
}

}
