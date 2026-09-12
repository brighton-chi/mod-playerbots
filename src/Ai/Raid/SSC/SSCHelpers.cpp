/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCHelpers.h"
#include "EncounterHelpers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCValueContext.h"
#include <cmath>
#include <limits>
#include <list>

using namespace EncounterHelpers;

namespace SscHelpers
{

namespace
{

// A step towards a point on a circle, at the angle nearest to preferred that the bot can reach.
bool FindStepToCircle(
    Player* bot, Position const& center, float radius, float preferredAngle, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    float const centerX = center.GetPositionX();
    float const centerY = center.GetPositionY();

    constexpr uint8 fanSteps = 8;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        float const delta = fanStep * step;
        uint8 const candidates = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < candidates; ++i)
        {
            float const angle = preferredAngle + (i == 0 ? delta : -delta);
            float const targetX = centerX + std::cos(angle) * radius;
            float const targetY = centerY + std::sin(angle) * radius;

            if (CanTakeStepTowards(bot, targetX, targetY, moveDist, stepX, stepY, stepZ))
                return true;
        }
    }

    return false;
}

} // end anonymous namespace

// Trash

std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, std::string const& value)
{
    return botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet();
}

bool GetHazardEscapeStep(
    Player* bot, Position const& hazard, float escapeRadius, float moveDist, float& stepX,
    float& stepY, float& stepZ)
{
    float const centerX = hazard.GetPositionX();
    float const centerY = hazard.GetPositionY();
    float escapeAngle =
        std::atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);

    if (bot->GetExactDist2d(centerX, centerY) <= 0.1f)
        escapeAngle = bot->GetOrientation();

    return FindStepToCircle(
        bot, hazard, escapeRadius, escapeAngle, moveDist, stepX, stepY, stepZ);
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
    return IsNearToxicPool(botAI, TOXIC_POOL_HAZARD_RADIUS);
}

// Hydross the Unstable <Duke of Currents>

std::unordered_map<uint32, uint32> hydrossFrostDpsWaitTimer;
std::unordered_map<uint32, uint32> hydrossNatureDpsWaitTimer;
std::unordered_map<uint32, uint32> hydrossChangeToFrostPhaseTimer;
std::unordered_map<uint32, uint32> hydrossChangeToNaturePhaseTimer;

bool IsHydrossPhaseTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) &&
        (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true));
}

bool IsHydrossAddTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && !IsHydrossPhaseTank(bot);
}

bool IsHydrossInFrostPhase(Unit* hydross)
{
    return hydross && !hydross->HasAura(Id(SscSpells::SPELL_HYDROSS_CORRUPTION));
}

bool IsHydrossInNaturePhase(Unit* hydross)
{
    return hydross && hydross->HasAura(Id(SscSpells::SPELL_HYDROSS_CORRUPTION));
}

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

// Leotheras the Blind

std::unordered_map<uint32, uint32> leotherasHumanoidPhaseDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasDemonPhaseDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasFinalPhaseDpsWaitTimer;

bool IsSpellbinderPhase(Creature* leotheras)
{
    return leotheras && leotheras->HasAura(Id(SscSpells::SPELL_LEOTHERAS_BANISHED));
}

Creature* GetActiveLeotherasHumanoid(Player* bot)
{
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), LEOTHERAS_SEARCH_DISTANCE);

    if (!leotheras || IsSpellbinderPhase(leotheras))
        return nullptr;

    if (!leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasHumanoidPhase(Player* bot)
{
    return GetActiveLeotherasHumanoid(bot) && !GetPhase3LeotherasDemon(bot);
}

Creature* GetPhase2LeotherasDemon(Player* bot)
{
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), LEOTHERAS_SEARCH_DISTANCE);

    if (leotheras && leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasDemonPhase(Player* bot)
{
    return GetPhase2LeotherasDemon(bot);
}

Creature* GetPhase3LeotherasDemon(Player* bot)
{
    return bot->FindNearestCreature(
        Id(SscNpcs::NPC_SHADOW_OF_LEOTHERAS), LEOTHERAS_SEARCH_DISTANCE);
}

bool IsLeotherasFinalPhase(Player* bot)
{
    return GetPhase3LeotherasDemon(bot);
}

Creature* GetActiveLeotherasDemon(Player* bot)
{
    if (Creature* phase2Demon = GetPhase2LeotherasDemon(bot))
        return phase2Demon;

    if (Creature* phase3Demon = GetPhase3LeotherasDemon(bot))
        return phase3Demon;

    return nullptr;
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
Player* GetLeotherasWarlockTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK ||
            member->GetMapId() != SSC_MAP_ID)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool IsLeotherasWarlockTank(Player* bot)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    return GetLeotherasWarlockTank(bot) == bot;
}

bool IsLeotherasChannelingWhirlwind(Unit* leotheras)
{
    return leotheras &&
        (leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND)) ||
         leotheras->HasAura(Id(SscSpells::SPELL_WHIRLWIND_CHANNEL)));
}

bool HasTooManyChaosBlastStacks(Player* bot)
{
    Aura* chaosBlast = bot->GetAura(Id(SscSpells::SPELL_CHAOS_BLAST));
    return chaosBlast && chaosBlast->GetStackAmount() >= 5;
}

bool HasInnerDemon(Player* bot)
{
    return bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER));
}

Creature* GetPersonalInnerDemon(PlayerbotAI* botAI)
{
    ObjectGuid const botGuid = botAI->GetBot()->GetGuid();
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& innerDemons = AI_VALUE(GuidVector, "possible targets no los");

    Creature* innerDemon = nullptr;
    for (auto creatureGuid : innerDemons)
    {
        Creature* creature = botAI->GetCreature(creatureGuid);
        if (creature && creature->GetEntry() == Id(SscNpcs::NPC_INNER_DEMON) &&
            creature->GetSummonerGUID() == botGuid)
        {
            innerDemon = creature;
            break;
        }
    }

    return innerDemon;
}

// Fathom-Lord Karathress

std::unordered_map<uint32, uint32> karathressDpsWaitTimer;

// Morogrim Tidewalker

std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

// Lady Vashj <Coilfang Matron>

std::unordered_map<ObjectGuid, bool> hasReachedVashjRangedPosition;
std::unordered_map<uint32, ObjectGuid> nearestVashjGeneratorTriggerGuid;
std::unordered_map<ObjectGuid, Position> intendedVashjCorePasserLineup;
std::unordered_map<uint32, uint32> lastVashjCoreImbueAttempt;
std::unordered_map<ObjectGuid, uint32> lastVashjCoreInInventoryTime;

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

int8 GetLadyVashjPhase(Unit* vashj)
{
    if (!vashj)
        return -1;

    float healthPct = vashj->GetHealthPct();
    constexpr uint32 magicBarrier = Id(SscSpells::SPELL_MAGIC_BARRIER);

    // Transitioning from Phase 1 to Phase 2
    if (healthPct <= 70.0f && healthPct > 50.0f && !vashj->HasAura(magicBarrier))
        return 0;

    // Phase 1
    if (healthPct > 70.0f)
        return 1;

    // Phase 2
    if (healthPct <= 70.0f && vashj->HasAura(magicBarrier))
        return 2;

    // Phase 3
    if (healthPct <= 50.0f) // and no Magic Barrier
        return 3;

    return -1;
}

// This can just be replaced by a target exclusion of Vashj for Phase 2 I think
bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* botAI, Unit* vashj)
{
    if (!unit || !unit->IsAlive())
        return false;

    int8 phase = GetLadyVashjPhase(vashj);
    uint32 entry = unit->GetEntry();

    if (phase == 2)
    {
        return entry == Id(SscNpcs::NPC_TAINTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL) ||
            entry == Id(SscNpcs::NPC_COILFANG_ELITE) ||
            entry == Id(SscNpcs::NPC_COILFANG_STRIDER);
    }
    else if (phase == 3)
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

        auto it = lastVashjCoreInInventoryTime.find(handler->GetGUID());
        if (it != lastVashjCoreInInventoryTime.end() &&
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
