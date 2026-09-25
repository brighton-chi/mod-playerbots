/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCHelpers.h"
#include "EncounterHelpers.h"
#include "Map.h"
#include "PathGenerator.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCValueContext.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <utility>

using namespace EncounterHelpers;

namespace SscHelpers
{

namespace
{

Creature* GetCachedCreature(PlayerbotAI* botAI, char const* value)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    Creature* creature = botAI->GetCreature(AI_VALUE(ObjectGuid, value));
    return creature && creature->IsAlive() ? creature : nullptr;
}

std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, std::string const& value)
{
    return botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet();
}

} // end anonymous namespace

// General

bool FindHazardEscapeStep(
    Player* bot, Position const& hazard, float moveDist, float& stepX, float& stepY, float& stepZ)
{
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botDistance = bot->GetExactDist2d(hazard);

    float escapeAngle = std::atan2(botY - hazard.GetPositionY(), botX - hazard.GetPositionX());
    if (botDistance <= 0.1f)
        escapeAngle = bot->GetOrientation();

    constexpr uint8 fanSteps = 16;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        float const delta = fanStep * step;
        uint8 const candidates = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < candidates; ++i)
        {
            float const angle = escapeAngle + (i == 0 ? delta : -delta);
            float const candidateX = botX + std::cos(angle) * moveDist;
            float const candidateY = botY + std::sin(angle) * moveDist;

            if (hazard.GetExactDist2d(candidateX, candidateY) <= botDistance)
                continue;

            if (!IsDryGround(bot, candidateX, candidateY))
                continue;

            if (CanTakeStepTowards(bot, candidateX, candidateY, moveDist, stepX, stepY, stepZ))
                return true;
        }
    }

    return false;
}

bool IsDryGround(Player* bot, float x, float y)
{
    float const ground = bot->GetMapHeight(x, y, bot->GetPositionZ());
    if (ground <= INVALID_HEIGHT)
        return false;

    LiquidData const liquid = bot->GetMap()->GetLiquidData(
        bot->GetPhaseMask(), x, y, bot->GetPositionZ(), bot->GetCollisionHeight(), {});

    constexpr float clearance = 0.5f;
    return liquid.Level <= INVALID_HEIGHT || ground > liquid.Level - clearance;
}

bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY)
{
    return GetPathStepTowardPoint(
        bot, target->GetPosition(), stopDistance, PATH_STEP_DISTANCE, stepX, stepY);
}

bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY)
{
    if (bot->GetExactDist(destination) < stopDistance)
        return false;

    PathGenerator path(bot);
    path.CalculatePath(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
    if (!(path.GetPathType() & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT)))
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2)
        return false;

    G3D::Vector3 const targetPos(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());

    float remaining = stepDistance;
    for (std::size_t i = 1; i < points.size(); ++i)
    {
        G3D::Vector3 const& from = points[i - 1];
        G3D::Vector3 const& to = points[i];

        float const segment = (to - from).length();
        if (segment <= 0.0f)
            continue;

        float const toDist = (to - targetPos).length();
        float ratio = 1.0f;

        if (toDist < stopDistance)
        {
            float const fromDist = (from - targetPos).length();
            if (fromDist <= stopDistance)
                break;

            ratio = (fromDist - stopDistance) / (fromDist - toDist);
        }

        if (segment * ratio >= remaining)
            ratio = remaining / segment;

        remaining -= segment * ratio;

        G3D::Vector3 const step = from + (to - from) * ratio;
        stepX = step.x;
        stepY = step.y;

        if (remaining <= 0.0f || ratio < 1.0f)
            return true;
    }

    return remaining < stepDistance;
}

// Trash

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

std::unordered_map<uint32, std::array<ObjectGuid, LURKER_GUARDIAN_TANK_COUNT>>
    lurkerGuardianTankAssignments;

bool IsLurkerSpouting(Unit* lurker)
{
    Creature* creature = lurker ? lurker->ToCreature() : nullptr;
    return creature && creature->IsInCombat() && creature->GetReactState() == REACT_PASSIVE;
}

bool IsLurkerSurfacedAndCalm(Unit* lurker)
{
    return lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
        !IsLurkerSpouting(lurker);
}

bool DoesPathRoundLurker(Player* bot, Unit* lurker, float x, float y, float z, int8 direction)
{
    PathGenerator path(bot);
    if (!path.CalculatePath(x, y, z) || (path.GetPathType() & PATHFIND_NOPATH))
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2)
        return false;

    float const startAngle = std::atan2(
        points[0].y - lurker->GetPositionY(), points[0].x - lurker->GetPositionX());
    float const cornerAngle = std::atan2(
        points[1].y - lurker->GetPositionY(), points[1].x - lurker->GetPositionX());
    float delta = Position::NormalizeOrientation(cornerAngle - startAngle);
    if (delta > M_PI)
        delta -= 2.0f * static_cast<float>(M_PI);

    return delta * direction > 0.0f;
}

bool DoesPathArrive(Player* bot, float x, float y, float z, float tolerance)
{
    PathGenerator path(bot);
    if (!path.CalculatePath(x, y, z) || (path.GetPathType() & PATHFIND_NOPATH))
        return false;

    G3D::Vector3 const& end = path.GetActualEndPosition();
    return std::hypot(end.x - x, end.y - y) <= tolerance;
}

int8 GetLurkerSpoutSpin(Unit* lurker)
{
    if (lurker->HasAura(Id(SscSpells::SPELL_SPOUT_COUNTERCLOCKWISE)))
        return 1;

    if (lurker->HasAura(Id(SscSpells::SPELL_SPOUT_CLOCKWISE)))
        return -1;

    return 0;
}

GuidVector FindLurkerGuardianGuids(Player* bot)
{
    GuidVector guids;

    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(SscNpcs::NPC_COILFANG_GUARDIAN), LURKER_GUARDIAN_SEARCH_RADIUS);

    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end());

    return guids;
}

std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI)
{
    std::vector<Unit*> guardians;

    for (ObjectGuid const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("ssc lurker guardians")->RefGet())
    {
        Unit* guardian = botAI->GetUnit(guid);
        if (guardian && guardian->IsAlive())
            guardians.push_back(guardian);
    }

    return guardians;
}

std::vector<Player*> GetLurkerGuardianTanks(Player* bot)
{
    std::vector<Player*> tanks = {
        GetGroupMainTank(bot), GetGroupAssistTank(bot, 0), GetGroupAssistTank(bot, 1) };

    if (std::any_of(tanks.begin(), tanks.end(), [](Player* tank) { return !tank; }))
        return {};

    return tanks;
}

// Leotheras the Blind

std::unordered_map<uint32, uint32> leotherasHumanoidPhaseDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasWhirlwindEndTime;
std::unordered_map<uint32, uint32> leotherasDemonPhaseDpsWaitTimer;
std::unordered_map<uint32, uint32> leotherasFinalPhaseDpsWaitTimer;

ObjectGuid FindLeotherasGuid(Player* bot)
{
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), LEOTHERAS_SEARCH_DISTANCE);
    return leotheras ? leotheras->GetGUID() : ObjectGuid::Empty;
}

ObjectGuid FindShadowOfLeotherasGuid(Player* bot)
{
    Creature* shadow =
        bot->FindNearestCreature(Id(SscNpcs::NPC_SHADOW_OF_LEOTHERAS), LEOTHERAS_SEARCH_DISTANCE);
    return shadow ? shadow->GetGUID() : ObjectGuid::Empty;
}

Creature* GetLeotheras(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc leotheras");
}

bool IsSpellbinderPhase(Unit* leotheras)
{
    return leotheras && leotheras->HasAura(Id(SscSpells::SPELL_LEOTHERAS_BANISHED));
}

Creature* GetActiveLeotherasHumanoid(PlayerbotAI* botAI)
{
    Creature* leotheras = GetLeotheras(botAI);
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return nullptr;

    if (!leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasHumanoidPhase(PlayerbotAI* botAI)
{
    return GetActiveLeotherasHumanoid(botAI) && !GetPhase3LeotherasDemon(botAI);
}

Creature* GetPhase2LeotherasDemon(PlayerbotAI* botAI)
{
    Creature* leotheras = GetLeotheras(botAI);
    if (leotheras && leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasDemonPhase(PlayerbotAI* botAI)
{
    return GetPhase2LeotherasDemon(botAI);
}

Creature* GetPhase3LeotherasDemon(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc shadow of leotheras");
}

bool IsLeotherasFinalPhase(PlayerbotAI* botAI)
{
    return GetPhase3LeotherasDemon(botAI);
}

Creature* GetActiveLeotherasDemon(PlayerbotAI* botAI)
{
    if (Creature* phase2Demon = GetPhase2LeotherasDemon(botAI))
        return phase2Demon;

    if (Creature* phase3Demon = GetPhase3LeotherasDemon(botAI))
        return phase3Demon;

    return nullptr;
}

// (1) First priority is an assistant Warlock (real player or bot).
// (2) If no assistant Warlock, then look for any Warlock bot.
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
    ObjectGuid const botGuid = botAI->GetBot()->GetGUID();
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

ObjectGuid FindSpitfireTotemGuid(Player* bot)
{
    Creature* totem =
        bot->FindNearestCreature(Id(SscNpcs::NPC_SPITFIRE_TOTEM), SPITFIRE_TOTEM_SEARCH_DISTANCE);
    return totem ? totem->GetGUID() : ObjectGuid::Empty;
}

Creature* GetSpitfireTotem(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc spitfire totem");
}

bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem)
{
    return totem && (PlayerbotAI::IsMelee(bot) ||
        bot->GetDistance(totem) < SPITFIRE_TOTEM_RANGED_ATTACK_DISTANCE);
}

namespace // Karathress
{

struct CouncilAssignment
{
    char const* name;
    int8 assistTankIndex; // -1 for the main tank
};

constexpr std::array<CouncilAssignment, 4> KARATHRESS_COUNCIL = {{
    { "fathom-lord karathress", -1 },
    { "fathom-guard caribdis", 0 },
    { "fathom-guard sharkkis", 1 },
    { "fathom-guard tidalvess", 2 },
}};

// GetGroupAssistTank does not allow dead tanks to be indexed, so this helper serves that purpose.
Player* GetCouncilTank(Player* bot, int8 assistTankIndex)
{
    if (assistTankIndex < 0)
        return GetGroupMainTank(bot);

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() &&
            PlayerbotAI::IsAssistTankOfIndex(member, assistTankIndex, false))
        {
            return member;
        }
    }

    return nullptr;
}

} // end anonymous namespace (Karathress)

Unit* GetAssignedCouncilMember(PlayerbotAI* botAI)
{
    Player* tank = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (CouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        bool const assigned = assignment.assistTankIndex < 0 ?
            PlayerbotAI::IsMainTank(tank) :
            PlayerbotAI::IsAssistTankOfIndex(tank, assignment.assistTankIndex, false);
        if (assigned)
            return AI_VALUE2(Unit*, "find target", assignment.name);
    }

    return nullptr;
}

bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (CouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", assignment.name);
        if (!member || member->GetVictim() != bot)
            continue;

        Player* tank = GetCouncilTank(bot, assignment.assistTankIndex);
        if (tank && tank != bot)
            return true;
    }

    return false;
}

bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* ownMember = GetAssignedCouncilMember(botAI);
    for (CouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", assignment.name);
        if (member && member != ownMember && bot->GetDistance(member) < range)
            return true;
    }

    return false;
}

// Sharkkis's tank holds his pets too. Sharkkis on somebody else comes first, then a pet on
// somebody else, then Sharkkis, then a pet that is already on the tank; null once none is left.
Unit* GetSharkkisTankTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->GetVictim() != bot)
        return sharkkis;

    Unit* heldPet = nullptr;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* pet = ref->GetOwner();
        if (!pet || !pet->IsAlive())
            continue;

        uint32 const entry = pet->GetEntry();
        if (entry != Id(SscNpcs::NPC_FATHOM_LURKER) && entry != Id(SscNpcs::NPC_FATHOM_SPOREBAT))
            continue;

        if (pet->GetVictim() != bot)
            return pet;

        heldPet = pet;
    }

    if (sharkkis)
        return sharkkis;

    return heldPet;
}

// Morogrim Tidewalker

Position GetTidewalkerStackPoint(Unit* tidewalker)
{
    Unit* victim = tidewalker->GetVictim();
    float const behindAngle = (victim ? tidewalker->GetAngle(victim) :
        tidewalker->GetOrientation()) + static_cast<float>(M_PI);

    return Position(
        tidewalker->GetPositionX() + std::cos(behindAngle) * TIDEWALKER_RANGED_BEHIND_DISTANCE,
        tidewalker->GetPositionY() + std::sin(behindAngle) * TIDEWALKER_RANGED_BEHIND_DISTANCE,
        tidewalker->GetPositionZ());
}

// Lady Vashj <Coilfang Matron>

namespace // Vashj
{

// Even-odd ray cast in 2D.
template <std::size_t N>
bool IsInPolygon(float x, float y, std::array<Position, N> const& polygon)
{
    bool inside = false;
    for (std::size_t i = 0, j = N - 1; i < N; j = i++)
    {
        float const xi = polygon[i].GetPositionX();
        float const yi = polygon[i].GetPositionY();
        float const xj = polygon[j].GetPositionX();
        float const yj = polygon[j].GetPositionY();
        if ((yi > y) != (yj > y) && x < (xj - xi) * (y - yi) / (yj - yi) + xi)
            inside = !inside;
    }

    return inside;
}

} // end anonymous namespace (Vashj)

std::unordered_map<uint32, ObjectGuid> nearestVashjGeneratorTriggerGuid;
std::unordered_map<ObjectGuid, Position> intendedVashjCorePasserLineup;
std::unordered_map<uint32, uint32> lastVashjCoreImbueAttempt;
std::unordered_map<ObjectGuid, uint32> lastVashjCoreInInventoryTime;

int8 GetLadyVashjPhase(Unit* vashj)
{
    if (!vashj)
        return -1;

    float const healthPct = vashj->GetHealthPct();
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

std::vector<Position> const& GetToxicSporePositions(PlayerbotAI* botAI)
{
    return GetCachedHazardPositions(botAI, "ssc toxic spores");
}

bool IsOnVashjDais(float x, float y, float margin)
{
    float const dx = x - VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    float const dy = y - VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();

    // Distance from the center square to the nearest edge. The edges face 15, 45, 75... degrees,
    // so fold the angle into one 30 degree sector and measure off the middle of it.
    constexpr float sector = static_cast<float>(M_PI) / 6.0f;
    float const angle = Position::NormalizeOrientation(std::atan2(dy, dx));
    float const offset = std::fmod(angle, sector) - sector / 2.0f;
    float const edgeDistance = std::hypot(dx, dy) * std::cos(offset);

    return edgeDistance <= VASHJ_DAIS_APOTHEM - margin && !IsInPolygon(x, y, VASHJ_NORTH_ROCK);
}

bool FindVashjDaisStepAwayFromPositions(
    Player* bot, std::vector<Position> const& positions, Unit* facing, float& stepX, float& stepY,
    float& stepZ, bool& backwards, std::vector<Position> const* spores)
{
    // Vashj trails her tank, so she stays on the dais as long as it does. The margin is only slack
    // for the notch the rock cuts and for pathing near the edge.
    constexpr float daisMargin = 1.0f;
    constexpr uint8 directions = 24;

    auto closestPosition = [&positions](float x, float y)
    {
        float closest = std::numeric_limits<float>::max();
        for (Position const& position : positions)
            closest = std::min(closest, position.GetExactDist2d(x, y));

        return closest;
    };

    auto nearSpore = [spores](float x, float y)
    {
        return spores && std::any_of(spores->begin(), spores->end(), [x, y](Position const& spore)
        {
            return spore.GetExactDist2d(x, y) < TOXIC_SPORES_AVOID_RADIUS;
        });
    };

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    // Angle and distance to the closest position for each step that stays on the dais
    std::vector<std::pair<float, float>> candidates;
    for (uint8 i = 0; i < directions; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * i / directions;
        float const x = botX + std::cos(angle) * PATH_STEP_DISTANCE;
        float const y = botY + std::sin(angle) * PATH_STEP_DISTANCE;
        if (IsOnVashjDais(x, y, daisMargin) && !nearSpore(x, y))
            candidates.emplace_back(angle, closestPosition(x, y));
    }

    std::sort(candidates.begin(), candidates.end(),
        [](auto const& a, auto const& b) { return a.second > b.second; });

    bool const tanking = facing && facing->GetVictim() == bot;
    float const current = closestPosition(botX, botY);
    for (auto const& [angle, closest] : candidates)
    {
        if (closest <= current)
            break;

        float const dirX = std::cos(angle);
        float const dirY = std::sin(angle);
        backwards = tanking && dirX * (facing->GetPositionX() - botX) +
            dirY * (facing->GetPositionY() - botY) < 0.0f;

        float const moveDist = backwards ? PATH_BACKWARD_STEP_DISTANCE : PATH_STEP_DISTANCE;
        if (CanTakeStepTowards(
                bot, botX + dirX * PATH_STEP_DISTANCE, botY + dirY * PATH_STEP_DISTANCE,
                moveDist, stepX, stepY, stepZ))
        {
            return true;
        }
    }

    return false;
}

bool FindVashjDaisStepAwayFromUnits(
    Player* bot, std::vector<Unit*> const& units, Unit* facing, float& stepX, float& stepY,
    float& stepZ, bool& backwards, std::vector<Position> const* spores)
{
    std::vector<Position> positions;
    positions.reserve(units.size());
    for (Unit* unit : units)
        positions.push_back(unit->GetPosition());

    return FindVashjDaisStepAwayFromPositions(
        bot, positions, facing, stepX, stepY, stepZ, backwards, spores);
}

bool HasStaticCharge(Player* player)
{
    return player->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE));
}

bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj)
{
    Player* vashjVictim = vashj->GetVictim() ? vashj->GetVictim()->ToPlayer() : nullptr;
    if (bot == vashjVictim)
        return false;

    return HasStaticCharge(bot) || (vashjVictim && HasStaticCharge(vashjVictim));
}

Player* GetVashjGroundingShaman(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->getClass() == CLASS_SHAMAN && member->IsAlive() &&
            member->GetMapId() == SSC_MAP_ID && group->SameSubGroup(mainTank, member) &&
            GET_PLAYERBOT_AI(member))
        {
            return member;
        }
    }

    return nullptr;
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

    uint32 const now = getMSTime();
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
