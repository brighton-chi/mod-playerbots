/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "Timer.h"

using namespace EncounterHelpers;

namespace BlackTempleHelpers
{

// Supremus

std::unordered_map<uint32, uint32> supremusPhaseTimer;

bool HasSupremusVolcanoNearby(Player* bot)
{
    constexpr float searchRadius = 20.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BlackTempleNpcs::NPC_SUPREMUS_VOLCANO), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            return true;
    }

    return false;
}

// Shade of Akama

std::unordered_set<ObjectGuid> hasReachedAkamaChannelerPosition;

// Gurtogg Bloodboil

std::unordered_map<uint32, uint32> gurtoggPhaseTimer;

std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot)
{
    Group* group = bot->GetGroup();
    std::vector<Player*> rangedMembers;
    std::vector<std::vector<Player*>> groups(3);

    if (!group)
        return groups;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BLACK_TEMPLE_MAP_ID && member->IsAlive() &&
            GET_PLAYERBOT_AI(member) && PlayerbotAI::IsRanged(member))
        {
            rangedMembers.push_back(member);
        }
    }

    for (size_t i = 0; i < rangedMembers.size(); ++i)
    {
        groups[i / 5].push_back(rangedMembers[i]);
        if (groups[2].size() == 5)
            break;
    }

    return groups;
}

int GetGurtoggActiveRotationGroup(Unit* gurtogg)
{
    if (!gurtogg)
        return -1;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return -1;

    constexpr uint32 groupSwapIntervalMs = 10 * IN_MILLISECONDS;
    constexpr uint32 rotationCycleMs = 3 * groupSwapIntervalMs;
    uint32 const elapsed = GetMSTimeDiffToNow(it->second);
    int const groupIndex = (elapsed % rotationCycleMs) / groupSwapIntervalMs;

    return groupIndex;
}

// Mother Shahraz

std::unordered_map<ObjectGuid, TankPositionState> shahrazTankStep;

TankPositionState GetShahrazTankPositionState(Player* bot)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return TankPositionState::Unknown;

    auto it = shahrazTankStep.find(mainTank->GetGUID());
    if (it != shahrazTankStep.end())
        return it->second;

    return TankPositionState::Unknown;
}

// Illidari Council

std::unordered_map<uint32, uint32> councilDpsWaitTimer;
std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
std::unordered_map<ObjectGuid, uint8> zerevorHealStep;

// (1) First priority is an assistant Mage (real player or bot)
// (2) If no assistant Mage, then look for any Mage bot
Player* GetZerevorMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackMage = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BLACK_TEMPLE_MAP_ID || !member->IsAlive() ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackMage && GET_PLAYERBOT_AI(member))
            fallbackMage = member;
    }

    return fallbackMage;
}

bool HasDangerousCouncilAura(Unit* unit)
{
    if (!unit)
        return false;

    static constexpr std::array dangerousAuras = {
        Id(BlackTempleSpells::SPELL_CONSECRATION),
        Id(BlackTempleSpells::SPELL_BLIZZARD),
        Id(BlackTempleSpells::SPELL_FLAMESTRIKE),
    };

    for (uint32 aura : dangerousAuras)
    {
        if (unit->HasAura(aura))
            return true;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>

std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
std::unordered_map<ObjectGuid, ObjectGuid> illidanShadowTrapGuid;
std::unordered_map<ObjectGuid, Position> illidanShadowTrapDestination;
std::unordered_map<uint32, int> illidanLastPhase;
std::unordered_map<uint32, uint32> illidanBossDpsWaitTimer;
std::unordered_map<uint32, uint32> illidanFlameDpsWaitTimer;
std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
std::unordered_map<uint32, ObjectGuid> westFlameGuid;

int GetIllidanPhase(Unit* illidan)
{
    if (!illidan || illidan->GetHealth() == 1 ||
        illidan->HasAura(Id(BlackTempleSpells::SPELL_SHADOW_PRISON)))
    {
        return -1;
    }

    // Transitioning from Phase 2 to Phase 3
    float x, y, z;
    illidan->GetMotionMaster()->GetDestination(x, y, z);
    Position const dest(x, y, z);
    if ((dest.GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
         illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f) &&
        illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return 0;
    }

    // Phase 2: Flying
    if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 2;

    // Phase 1: Health > 65%
    if (illidan->GetHealthPct() > 65.0f)
        return 1;

    // Phase 4: Demon Form
    if (!illidan->HasAura(Id(BlackTempleSpells::SPELL_CAGED)) &&
        (illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_FORM)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_1)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_2)) ||
         illidan->HasAura(Id(BlackTempleSpells::SPELL_DEMON_TRANSFORM_3))))
    {
        return 4;
    }

    // Phase 3: Normal (ground, 65-30%, not demon)
    if (illidan->GetHealthPct() > 30.0f)
        return 3;

    // Phase 5: Health <= 30%
    if (illidan->GetHealthPct() <= 30.0f)
        return 5;

    return -1;
}

std::vector<Unit*> GetAllFlameCrashes(Player* bot)
{
    std::vector<Unit*> flameCrashes;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BlackTempleNpcs::NPC_FLAME_CRASH), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            flameCrashes.push_back(creature);
    }

    return flameCrashes;
}

std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot)
{
    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;

    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    if (eastFlameGuid.find(instanceId) != eastFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, eastFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                eastFlame = unit;
        }
    }

    if (westFlameGuid.find(instanceId) != westFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, westFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                westFlame = unit;
        }
    }

    return { eastFlame, westFlame };
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
Player* GetIllidanWarlockTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != BLACK_TEMPLE_MAP_ID || !member->IsAlive() ||
            member->getClass() != CLASS_WARLOCK)
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

bool HasParasiticShadowfiend(Player* player)
{
    if (!player)
        return false;

    return player->HasAura(Id(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_1)) ||
        player->HasAura(Id(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_2));
}

// Get the first bot hunter that doesn't have Parasitic Shadowfiend
Player* GetIllidanTrapperHunter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == BLACK_TEMPLE_MAP_ID && member->IsAlive() &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member) &&
            !HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

    return nullptr;
}

Player* GetBotWithParasiticShadowfiend(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) &&
            HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

    return nullptr;
}

EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(BlackTempleNpcs::NPC_ILLIDAN_DB_TARGET), searchRadius);

    Creature* eyeBlastTrigger = nullptr;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
        {
            eyeBlastTrigger = creature;
            break;
        }
    }

    if (!eyeBlastTrigger)
        return {};

    Position const startPos = Position(eyeBlastTrigger->GetPositionX(),
        eyeBlastTrigger->GetPositionY(), eyeBlastTrigger->GetPositionZ());

    float destX, destY, destZ;
    eyeBlastTrigger->GetMotionMaster()->GetDestination(destX, destY, destZ);
    Position const endPos(destX, destY, destZ);

    if (startPos.GetExactDist2d(endPos) < 0.1f)
        return {};

    constexpr float eyeBlastWidth = 9.0f;
    return { startPos, endPos, eyeBlastWidth };
}

bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area)
{
    float const dx = area.end.GetPositionX() - area.start.GetPositionX();
    float const dy = area.end.GetPositionY() - area.start.GetPositionY();
    float const length = area.start.GetExactDist2d(area.end.GetPositionX(), area.end.GetPositionY());

    if (length < 0.1f)
        return false;

    float const projectionFactor = ((pos.GetPositionX() - area.start.GetPositionX()) * dx +
        (pos.GetPositionY() - area.start.GetPositionY()) * dy) / (length * length);

    float const clampedProjectionFactor = std::clamp(projectionFactor, 0.0f, 1.0f);

    float const closestX = area.start.GetPositionX() + clampedProjectionFactor * dx;
    float const closestY = area.start.GetPositionY() + clampedProjectionFactor * dy;

    float const distToLine = pos.GetExactDist2d(closestX, closestY);

    return distToLine < area.width;
}

GameObject* FindNearestTrap(PlayerbotAI* botAI)
{
    GuidVector const& gos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();

    GameObject* nearestTrap = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->isSpawned() && go->GetEntry() == Id(BlackTempleObjects::GO_SHADOW_TRAP))
        {
            nearestTrap = go;
            break;
        }
    }

    return nearestTrap;
}

}
