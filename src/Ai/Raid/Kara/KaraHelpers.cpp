/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraHelpers.h"
#include "CellImpl.h"
#include "DynamicObject.h"
#include "EncounterHelpers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "Random.h"
#include "Spell.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

using namespace EncounterHelpers;

namespace KaraHelpers
{

namespace
{

std::mutex karaStateMutex;
std::unordered_map<uint32, KaraInstanceState> karaStates;

float GetDistanceToSegment2d(Position const& point, Position const& start, Position const& end)
{
    float const segmentX = end.GetPositionX() - start.GetPositionX();
    float const segmentY = end.GetPositionY() - start.GetPositionY();
    float const lengthSquared = segmentX * segmentX + segmentY * segmentY;
    if (lengthSquared <= 0.0f)
        return point.GetExactDist2d(start);

    float const along =
        ((point.GetPositionX() - start.GetPositionX()) * segmentX +
         (point.GetPositionY() - start.GetPositionY()) * segmentY) / lengthSquared;
    float const fraction = std::clamp(along, 0.0f, 1.0f);
    return point.GetExactDist2d(
        start.GetPositionX() + segmentX * fraction, start.GetPositionY() + segmentY * fraction);
}

bool IsInBearForm(Player* bot)
{
    return bot->GetShapeshiftForm() == FORM_BEAR || bot->GetShapeshiftForm() == FORM_DIREBEAR;
}

bool NeedsComboPointsForManaWarpStun(Player* bot)
{
    return bot->getClass() == CLASS_ROGUE ||
        (bot->getClass() == CLASS_DRUID && bot->GetShapeshiftForm() == FORM_CAT);
}

bool IsManaWarpCastingWarpBreach(Unit* warp, uint32 reactionMs)
{
    Spell* breach = warp->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_WARP_BREACH));
    return breach && breach->getState() == SPELL_STATE_PREPARING &&
        breach->GetCastTime() - breach->GetCastTimeRemaining() >= static_cast<int32>(reactionMs);
}

bool IsManaWarpToStun(Player* bot, Unit* unit, float reach, uint32 reactionMs)
{
    return unit && unit->GetEntry() == Id(KaraNpcs::NPC_MANA_WARP) &&
        bot->GetExactDist(unit) <= reach && IsManaWarpCastingWarpBreach(unit, reactionMs);
}

void StepToward(Position& from, Position const& to, float distance)
{
    float const remaining = from.GetExactDist2d(to);
    if (remaining <= distance)
    {
        from.Relocate(to.GetPositionX(), to.GetPositionY());
        return;
    }

    float const ratio = distance / remaining;
    from.Relocate(
        from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * ratio,
        from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * ratio);
}

float GetSmallestBigBadWolfRunGap(
    Position runner, Position wolf, BigBadWolfRunPlan plan, float runnerStep, float wolfStep,
    float wolfStopDistance, uint32 steps)
{
    float smallestGap = std::numeric_limits<float>::max();
    for (uint32 step = 0; step < steps; ++step)
    {
        if (runner.GetExactDist2d(BIG_BAD_WOLF_RUN_POSITIONS[plan.index]) <
            BIG_BAD_WOLF_RUN_TURN_DISTANCE)
        {
            plan.index = GetNextBigBadWolfRunIndex(plan.index, plan.direction);
        }

        StepToward(runner, BIG_BAD_WOLF_RUN_POSITIONS[plan.index], runnerStep);
        if (wolf.GetExactDist2d(runner) > wolfStopDistance)
            StepToward(wolf, runner, wolfStep);

        smallestGap = std::min(smallestGap, wolf.GetExactDist2d(runner));
    }

    return smallestGap;
}

// The Opera is three plays with several actors each, and the Chess Event has no boss on a
// threat list, so neither has a name.
constexpr std::string_view BossName(KaraBosses boss)
{
    switch (boss)
    {
        case KaraBosses::DATA_ATTUMEN:
            return "attumen the huntsman";
        case KaraBosses::DATA_MOROES:
            return "moroes";
        case KaraBosses::DATA_MAIDEN:
            return "maiden of virtue";
        case KaraBosses::DATA_CURATOR:
            return "the curator";
        case KaraBosses::DATA_TERESTIAN:
            return "terestian illhoof";
        case KaraBosses::DATA_ARAN:
            return "shade of aran";
        case KaraBosses::DATA_NETHERSPITE:
            return "netherspite";
        case KaraBosses::DATA_MALCHEZAAR:
            return "prince malchezaar";
        case KaraBosses::DATA_NIGHTBANE:
            return "nightbane";
        default:
            return {};
    }
}

} // end anonymous namespace

// Shared

Unit* GetKaraBoss(Player* bot, KaraBosses boss)
{
    if (!IsBossInProgress(bot, KARA_MAP_ID, Id(boss)))
        return nullptr;

    return FindThreatTarget(bot, BossName(boss));
}

Unit* GetOperaActor(Player* bot, std::string_view name)
{
    if (!IsBossInProgress(bot, KARA_MAP_ID, Id(KaraBosses::DATA_OPERA_PERFORMANCE)))
        return nullptr;

    Unit* actor = FindThreatTarget(bot, name);
    return actor && !actor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ? actor : nullptr;
}

bool IsSafePosition(float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = hazard->GetDistance2d(x, y);
        if (dist < hazardRadius)
            return false;
    }

    return true;
}

bool IsKarazhanFearCasterActive(Player* bot)
{
    if (FindThreatTarget(bot, "spectral charger") || GetOperaActor(bot, "the big bad wolf"))
        return true;

    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (nightbane && !IsNightbaneFlying(*nightbane))
        return true;

    return GetOperaActor(bot, "dorothee") || GetOperaActor(bot, "roar");
}

// Trash

bool IsNearManaWarpPacks(Player* bot)
{
    return bot->GetExactDist(MANA_WARP_PACKS_CENTER) <= MANA_WARP_PACKS_RADIUS;
}

float GetManaWarpStunReach(Player* bot)
{
    float reach = 0.0f;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (IsInBearForm(bot) || bot->GetShapeshiftForm() == FORM_CAT)
                reach = MELEE_STUN_REACH;
            break;
        case CLASS_PALADIN:
            reach = HAMMER_OF_JUSTICE_REACH;
            break;
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            reach = MELEE_STUN_REACH;
            break;
        case CLASS_WARLOCK:
            reach = SHADOWFURY_REACH;
            break;
        default:
            break;
    }

    if (bot->getRace() == RACE_TAUREN)
        reach = std::max(reach, WAR_STOMP_RADIUS);

    return reach;
}

// The current target first, then any warp in reach that is casting.
Unit* GetManaWarpToStun(PlayerbotAI* botAI, uint32 reactionMs)
{
    Player* bot = botAI->GetBot();
    float const reach = GetManaWarpStunReach(bot);
    if (reach <= 0.0f || !IsNearManaWarpPacks(bot))
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    bool const needsComboPoints = NeedsComboPointsForManaWarpStun(bot);
    if (IsManaWarpToStun(bot, target, reach, reactionMs) &&
        (!needsComboPoints || bot->GetComboPoints(target)))
    {
        return target;
    }

    if (needsComboPoints)
        return nullptr;

    for (auto const& attackers : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(attackers);
        if (IsManaWarpToStun(bot, attacker, reach, reactionMs))
            return attacker;
    }

    return nullptr;
}

Unit* GetManaWarpToHoldFor(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (GetManaWarpStunReach(bot) <= 0.0f || PlayerbotAI::IsTank(bot) || !IsNearManaWarpPacks(bot))
        return nullptr;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || target->GetEntry() != Id(KaraNpcs::NPC_MANA_WARP) || !target->IsAlive() ||
        target->GetHealthPct() > MANA_WARP_STUNNER_HOLD_HEALTH_PCT)
    {
        return nullptr;
    }

    // A rogue or cat druid with no combo points on it keeps building them.
    return !NeedsComboPointsForManaWarpStun(bot) || bot->GetComboPoints(target) ? target : nullptr;
}

// Attumen the Huntsman

bool IsAttumenMounted(Unit const& midnight)
{
    return !midnight.IsVisible();
}

Unit* GetMountedAttumen(Player* bot)
{
    Unit* attumen = GetKaraBoss(bot, KaraBosses::DATA_ATTUMEN);
    if (!attumen || attumen->GetEntry() != Id(KaraNpcs::NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED))
        return nullptr;

    return attumen;
}

// Maiden of Virtue

Player* GetNearestRepentantHealer(Player* bot, Unit const& maiden)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearest = nullptr;
    float nearestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() ||
            !member->HasAura(Id(KaraSpells::SPELL_REPENTANCE)) || !PlayerbotAI::IsHeal(member))
        {
            continue;
        }

        float const dist = maiden.GetExactDist2d(member);
        if (dist < nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    return nearest;
}

// The Big Bad Wolf

uint8 GetNextBigBadWolfRunIndex(uint8 index, int32 direction)
{
    int32 const count = static_cast<int32>(BIG_BAD_WOLF_RUN_POSITIONS.size());
    return static_cast<uint8>((index + direction + count) % count);
}

BigBadWolfRunPlan GetBigBadWolfRunPlan(Player* bot, Unit const& wolf, float seconds)
{
    constexpr float forecastStepSeconds = 0.1f;
    uint32 const steps = static_cast<uint32>(seconds / forecastStepSeconds);
    float const runnerStep = bot->GetSpeed(MOVE_RUN) * forecastStepSeconds;
    float const wolfStep = wolf.GetSpeed(MOVE_RUN) * forecastStepSeconds;
    float const wolfStopDistance =
        CONTACT_DISTANCE + wolf.GetCombatReach() + bot->GetCombatReach();

    BigBadWolfRunPlan best;
    float bestGap = -1.0f;
    for (uint8 index = 0; index < BIG_BAD_WOLF_RUN_POSITIONS.size(); ++index)
    {
        for (int32 const direction : { 1, -1 })
        {
            BigBadWolfRunPlan const plan{ index, direction };
            float const gap = GetSmallestBigBadWolfRunGap(
                bot->GetPosition(), wolf.GetPosition(), plan, runnerStep, wolfStep,
                wolfStopDistance, steps);
            if (gap > bestGap)
            {
                best = plan;
                bestGap = gap;
            }
        }
    }

    return best;
}

Player* GetBigBadWolfTargetToProtect(Unit const& wolf)
{
    Unit* victim = wolf.GetVictim();
    Player* target = victim ? victim->ToPlayer() : nullptr;
    if (!target || PlayerbotAI::IsTank(target) ||
        wolf.GetExactDist2d(target) > BIG_BAD_WOLF_PROTECT_DISTANCE)
    {
        return nullptr;
    }

    Aura const* hood = target->GetAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
    if (!hood || hood->GetMaxDuration() - hood->GetDuration() < BIG_BAD_WOLF_PROTECT_REACTION_MS)
        return nullptr;

    return target;
}

// Shade of Aran

bool IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_ARCANE_EXPLOSION));
}

std::optional<Position> GetBlizzardEscapeSpot(Player* bot)
{
    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(bot, ARAN_ROOM_SEARCH_RADIUS);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objects, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, ARAN_ROOM_SEARCH_RADIUS);

    DynamicObject const* oldest = nullptr;
    for (WorldObject* object : objects)
    {
        DynamicObject const* patch = object->ToDynObject();
        if (patch && patch->GetSpellId() == Id(KaraSpells::SPELL_BLIZZARD) &&
            (!oldest || patch->GetDuration() < oldest->GetDuration()))
        {
            oldest = patch;
        }
    }

    if (!oldest)
        return std::nullopt;

    float const angle = ARAN_ROOM_CENTER.GetAngle(oldest) + ARAN_BLIZZARD_ESCAPE_ANGLE;
    return Position(
        ARAN_ROOM_CENTER.GetPositionX() + ARAN_BLIZZARD_ESCAPE_DISTANCE * std::cos(angle),
        ARAN_ROOM_CENTER.GetPositionY() + ARAN_BLIZZARD_ESCAPE_DISTANCE * std::sin(angle),
        bot->GetPositionZ());
}

bool IsFlameWreathInProgress(Player* bot, Unit* aran)
{
    if (bot->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
        return true;

    return aran && (aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)) ||
                    aran->GetDynObject(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)));
}

// The only movement allowed around Flame Wreath is a stuck bot walking to its own target. That is
// the one stock destination a check can see, and the bot stops well short of every ring.
bool CanReachTargetDuringFlameWreath(PlayerbotAI* botAI, Unit* aran)
{
    Player* bot = botAI->GetBot();
    if (!aran || bot->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (!target)
        return false;

    if (Spell* cast = aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)))
        return cast->GetCastTimeRemaining() > ARAN_FLAME_WREATH_STOP_LEAD_MS;

    DynamicObject* ring = aran->GetDynObject(Id(KaraSpells::SPELL_FLAME_WREATH_AURA));
    if (!ring)
        return true;

    if (ring->GetDuration() > ARAN_FLAME_WREATH_RING_DURATION_MS - ARAN_FLAME_WREATH_SETTLE_MS)
        return false;

    std::vector<Position> const rings =
        context->GetValue<std::vector<Position>>("shade of aran flame wreath rings")->Get();
    if (rings.empty())
        return false;

    float const clearance = ARAN_FLAME_WREATH_RING_HAZARD_RADIUS + bot->GetCombatReach() +
        ARAN_FLAME_WREATH_REACH_CLEARANCE;
    for (Position const& position : rings)
    {
        if (GetDistanceToSegment2d(position, *bot, *target) <= clearance)
            return false;
    }

    return true;
}

// Netherspite

namespace
{

constexpr float NETHERSPITE_MAX_OWN_LINE_ARC = static_cast<float>(M_PI) * 60.0f / 180.0f;
constexpr float NETHERSPITE_MAX_HEALER_ARC = static_cast<float>(M_PI) * 170.0f / 180.0f;
constexpr float NETHERSPITE_HEALER_TANK_RANGE = 38.0f;
constexpr float NETHERSPITE_BOT_SPEED = 7.0f;
constexpr uint8 NETHERSPITE_SPOT_TRIES = 12;

struct NetherspiteArc
{
    float start;
    float width;
};

struct NetherspiteRadiusBand
{
    float low;
    float high;
};

float GetBeamAngle(
    Unit const& netherspite, KaraInstanceState const& state, NetherspiteBeam beam)
{
    Position const& portal = *state.netherspiteBeams[Id(beam)].portal;
    return netherspite.GetAngle(portal.GetPositionX(), portal.GetPositionY());
}

// The bearings between green and blue that don't include red.
NetherspiteArc GetSideWithoutRed(Unit const& netherspite, KaraInstanceState const& state)
{
    float const red = GetBeamAngle(netherspite, state, NetherspiteBeam::Red);
    float const green = GetBeamAngle(netherspite, state, NetherspiteBeam::Green);
    float const blue = GetBeamAngle(netherspite, state, NetherspiteBeam::Blue);

    float const greenToBlue = Position::NormalizeOrientation(blue - green);
    if (Position::NormalizeOrientation(red - green) < greenToBlue)
        return { blue, Position::NormalizeOrientation(green - blue) };

    return { green, greenToBlue };
}

// At most 60 degrees from the bot's own line, so a wedge under 180 degrees: a straight move inside
// it can't cross a beam. Healers take the middle of the side instead, which keeps the tank in
// range. One that becomes the next blocker has about 25 s to get to its beam.
NetherspiteArc GetNetherspiteStandingArc(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state)
{
    NetherspiteArc const side = GetSideWithoutRed(netherspite, state);
    if (PlayerbotAI::IsHeal(bot))
    {
        float const width = std::min(side.width, NETHERSPITE_MAX_HEALER_ARC);
        return { side.start + (side.width - width) / 2.0f, width };
    }

    NetherspiteBeam const ownBeam = bot->getPowerType() == POWER_MANA ?
        NetherspiteBeam::Blue : NetherspiteBeam::Green;
    float const width = std::min(side.width, NETHERSPITE_MAX_OWN_LINE_ARC);
    float const own = GetBeamAngle(netherspite, state, ownBeam);
    bool const isStart = GetNetherspiteInnerSide(netherspite, state, ownBeam) > 0.0f;
    return { isStart ? own : own - width, width };
}

NetherspiteRadiusBand GetNetherspiteRadiusBand(Player* bot)
{
    if (PlayerbotAI::IsHeal(bot))
        return { 14.0f, 25.0f };
    if (bot->getClass() == CLASS_HUNTER)
        return { 25.0f, 32.0f };
    if (PlayerbotAI::IsMelee(bot))
        return { 14.0f, 20.0f };
    return { 18.0f, 25.0f };
}

// Up to his centre, past where the script counts a player: a bot shouldn't rely on that cutoff.
bool IsClearOfBeams(
    Unit const& netherspite, KaraInstanceState const& state, float x, float y)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (!beam.portal)
            continue;

        NetherspiteBeamOffset const offset =
            GetNetherspiteBeamOffset(netherspite, *beam.portal, x, y);
        if (offset.along > 0.0f && offset.along < netherspite.GetExactDist2d(*beam.portal) &&
            std::abs(offset.off) < NETHERSPITE_BEAM_CLEARANCE)
        {
            return false;
        }
    }

    return true;
}

bool CrossesBeam(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state, float x, float y)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (!beam.portal)
            continue;

        NetherspiteBeamOffset const from = GetNetherspiteBeamOffset(
            netherspite, *beam.portal, bot->GetPositionX(), bot->GetPositionY());
        NetherspiteBeamOffset const to = GetNetherspiteBeamOffset(netherspite, *beam.portal, x, y);
        // A bot already on the beam, such as a blocker stepping off, can leave by either side.
        if (std::abs(from.off) < NETHERSPITE_BEAM_WIDTH || from.off * to.off >= 0.0f)
            continue;

        float const along =
            from.along + (to.along - from.along) * from.off / (from.off - to.off);
        if (along > 0.0f && along < netherspite.GetExactDist2d(*beam.portal))
            return true;
    }

    return false;
}

// Crossing costs nothing if the bot is across before the beams start.
bool CrossesBeamAfterStart(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state, float x, float y)
{
    uint32 const elapsed = state.netherspiteDpsHoldTimer ?
        getMSTimeDiff(*state.netherspiteDpsHoldTimer, getMSTime()) : NETHERSPITE_BEAMS_START_MS;
    float const travelMs = bot->GetExactDist2d(x, y) / NETHERSPITE_BOT_SPEED * IN_MILLISECONDS;
    if (elapsed + travelMs < NETHERSPITE_BEAMS_START_MS)
        return false;

    return CrossesBeam(bot, netherspite, state, x, y);
}

// False if the floor stops the way there short of it.
bool GetFloorAt(Player* bot, float x, float y, float& outZ)
{
    float clippedX = x;
    float clippedY = y;
    outZ = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), clippedX, clippedY,
            outZ, false))
    {
        return false;
    }

    return std::abs(clippedX - x) < 1.0f && std::abs(clippedY - y) < 1.0f;
}

}

bool IsBanishPhase(Unit* netherspite)
{
    return netherspite && netherspite->HasAura(Id(KaraSpells::SPELL_NETHERSPITE_BANISHED));
}

bool IsNetherspiteDpsHeld(Player* bot, Unit const& netherspite)
{
    if (PlayerbotAI::IsTank(bot) || netherspite.HasAura(Id(KaraSpells::SPELL_NETHERSPITE_BANISHED)))
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite.GetInstanceId());
    if (!state || !state->netherspiteDpsHoldTimer)
        return true;

    constexpr uint32 minimumHoldMs = 5 * IN_MILLISECONDS;
    uint32 const elapsed = getMSTimeDiff(*state->netherspiteDpsHoldTimer, getMSTime());
    if (elapsed < minimumHoldMs)
        return true;
    if (elapsed >= NETHERSPITE_BEAMS_START_MS)
        return false;

    Unit* victim = netherspite.GetVictim();
    Player* player = victim ? victim->ToPlayer() : nullptr;
    return !player || !PlayerbotAI::IsTank(player);
}

bool AreNetherspitePortalsKnown(KaraInstanceState const& state)
{
    return std::all_of(
        state.netherspiteBeams.begin(), state.netherspiteBeams.end(),
        [](NetherspiteBeamState const& beam) { return beam.portal.has_value(); });
}

bool IsNetherspiteBlockerOrNext(KaraInstanceState const& state, ObjectGuid guid)
{
    return std::any_of(
        state.netherspiteBeams.begin(), state.netherspiteBeams.end(),
        [guid](NetherspiteBeamState const& beam)
        {
            return beam.blocker == guid || beam.next == guid;
        });
}

Position const* FindNetherspiteBlockerSpot(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.blocker == guid)
            return &beam.blockerSpot;
    }

    return nullptr;
}

Position const* FindNetherspiteNextSpot(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.next == guid)
            return &beam.nextSpot;
    }

    return nullptr;
}

NetherspiteBanishSlot const* FindNetherspiteBanishSlot(
    KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBanishSlot const& slot : state.netherspiteBanishSlots)
    {
        if (slot.guid == guid)
            return &slot;
    }

    return nullptr;
}

NetherspiteBeamOffset GetNetherspiteBeamOffset(
    Unit const& netherspite, Position const& portal, float x, float y)
{
    float const length = netherspite.GetExactDist2d(portal);
    if (length == 0.0f)
        return { 0.0f, 0.0f };

    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;
    float const dx = x - netherspite.GetPositionX();
    float const dy = y - netherspite.GetPositionY();
    return { dx * dirX + dy * dirY, dy * dirX - dx * dirY };
}

bool IsOnNetherspiteBeam(Unit const& netherspite, Position const& portal, Unit const& unit)
{
    NetherspiteBeamOffset const offset = GetNetherspiteBeamOffset(
        netherspite, portal, unit.GetPositionX(), unit.GetPositionY());
    return offset.along > NETHERSPITE_BEAM_START &&
        offset.along < netherspite.GetExactDist2d(portal) &&
        std::abs(offset.off) < NETHERSPITE_BEAM_WIDTH;
}

bool IsClearOfVoidZones(KaraInstanceState const& state, float x, float y, float clearance)
{
    return std::none_of(
        state.netherspiteVoidZones.begin(), state.netherspiteVoidZones.end(),
        [x, y, clearance](Position const& zone) { return zone.GetExactDist2d(x, y) < clearance; });
}

float GetNetherspiteInnerSide(
    Unit const& netherspite, KaraInstanceState const& state, NetherspiteBeam beam)
{
    float const toOwn = Position::NormalizeOrientation(
        GetBeamAngle(netherspite, state, beam) -
        GetSideWithoutRed(netherspite, state).start);
    return std::min(toOwn, 2.0f * static_cast<float>(M_PI) - toOwn) < 0.001f ? 1.0f : -1.0f;
}

bool IsNetherspiteStandingSpotGood(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state, float x, float y)
{
    if (!IsClearOfVoidZones(state, x, y, VOID_ZONE_CLEARANCE) ||
        !IsClearOfBeams(netherspite, state, x, y))
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return true;

    NetherspiteRadiusBand const band = GetNetherspiteRadiusBand(bot);
    float const radius = netherspite.GetExactDist2d(x, y);
    if (radius < band.low || radius > band.high)
        return false;

    NetherspiteArc const arc = GetNetherspiteStandingArc(bot, netherspite, state);
    float const angle = netherspite.GetAngle(x, y);
    if (Position::NormalizeOrientation(angle - arc.start) > arc.width)
        return false;

    if (PlayerbotAI::IsHeal(bot))
    {
        NetherspiteBeamState const& red = state.netherspiteBeams[Id(NetherspiteBeam::Red)];
        return red.blocker.IsEmpty() ||
            red.blockerSpot.GetExactDist2d(x, y) <= NETHERSPITE_HEALER_TANK_RANGE;
    }

    if (bot->getClass() == CLASS_HUNTER || !PlayerbotAI::IsMelee(bot))
        return true;

    // He faces the red blocker, and set behind is off.
    float const fromRed = Position::NormalizeOrientation(
        angle - GetBeamAngle(netherspite, state, NetherspiteBeam::Red));
    return fromRed > M_PI / 2.0f && fromRed < 3.0f * M_PI / 2.0f;
}

std::optional<Position> FindNetherspiteStandingSpot(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state)
{
    float z = bot->GetPositionZ();
    if (!PlayerbotAI::IsTank(bot))
    {
        NetherspiteArc const arc = GetNetherspiteStandingArc(bot, netherspite, state);
        NetherspiteRadiusBand const band = GetNetherspiteRadiusBand(bot);
        for (uint8 i = 0; i < NETHERSPITE_SPOT_TRIES; ++i)
        {
            float const angle = arc.start + frand(0.0f, arc.width);
            float const radius = frand(band.low, band.high);
            float const x = netherspite.GetPositionX() + std::cos(angle) * radius;
            float const y = netherspite.GetPositionY() + std::sin(angle) * radius;
            if (IsNetherspiteStandingSpotGood(bot, netherspite, state, x, y) &&
                !CrossesBeamAfterStart(bot, netherspite, state, x, y) &&
                GetFloorAt(bot, x, y, z))
            {
                return Position(x, y, z);
            }
        }
    }

    // A bot that can't get to the area without crossing a beam stays put unless it's in danger.
    if (IsClearOfVoidZones(state, bot->GetPositionX(), bot->GetPositionY(), VOID_ZONE_CLEARANCE) &&
        IsClearOfBeams(netherspite, state, bot->GetPositionX(), bot->GetPositionY()))
    {
        return std::nullopt;
    }

    // Nearest first, so a bot only steps clear of what it's standing in.
    constexpr float maxDistance = 20.0f;
    constexpr float distanceStep = 0.5f;
    constexpr uint8 numAngles = 24;
    for (float distance = 1.0f; distance <= maxDistance; distance += distanceStep)
    {
        for (uint8 i = 0; i < numAngles; ++i)
        {
            float const angle = i * 2.0f * M_PI / numAngles;
            float const x = bot->GetPositionX() + std::cos(angle) * distance;
            float const y = bot->GetPositionY() + std::sin(angle) * distance;
            if (IsClearOfVoidZones(state, x, y, VOID_ZONE_CLEARANCE) &&
                IsClearOfBeams(netherspite, state, x, y) &&
                !CrossesBeamAfterStart(bot, netherspite, state, x, y) &&
                GetFloorAt(bot, x, y, z))
            {
                return Position(x, y, z);
            }
        }
    }

    return std::nullopt;
}

// Prince Malchezaar

std::vector<Unit*> GetSpawnedInfernals(Player* bot)
{
    std::vector<Unit*> infernals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            infernals.push_back(creature);
    }

    return infernals;
}

bool IsStraightPathSafe(
    float sx, float sy, float tx, float ty, std::vector<Unit*> const& hazards, float hazardRadius)
{
    float const totalDistX = tx - sx;
    float const totalDistY = ty - sy;
    float const totalDist = sqrt(totalDistX * totalDistX + totalDistY * totalDistY);
    if (totalDist == 0.0f)
        return true;

    constexpr float stepSize = 0.5f;
    int const numSteps = static_cast<int>(totalDist / stepSize);
    for (int i = 0; i <= numSteps; ++i)
    {
        float const checkDist = i * stepSize;
        float const t = (totalDist > 0.0f) ? checkDist / totalDist : 0.0f;
        float checkX = sx + totalDistX * t;
        float checkY = sy + totalDistY * t;
        for (Unit* hazard : hazards)
        {
            float const hx = checkX - hazard->GetPositionX();
            float const hy = checkY - hazard->GetPositionY();
            if ((hx*hx + hy*hy) < hazardRadius * hazardRadius)
                return false;
        }
    }

    return true;
}

bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY)
{
    constexpr uint8 numAngles = 64;
    constexpr float stepSize = 0.5f;

    // Attempt to find a safe path, but take any path to a safe position if no safe path is found
    for (bool requireSafePath : { true, false })
    {
        float bestMoveDistSq = std::numeric_limits<float>::max();
        bool found = false;

        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2.0f * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            int const numSteps = static_cast<int>(maxSampleDist / stepSize);
            for (int j = 1; j <= numSteps; ++j)
            {
                float const dist = j * stepSize;
                float destX = centerX + dx * dist;
                float destY = centerY + dy * dist;
                float destZ = bot->GetPositionZ();
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                        bot, centerX, centerY, destZ, destX, destY, destZ, true))
                {
                    continue;
                }

                if (!IsSafePosition(destX, destY, hazards, safeDistance))
                    continue;

                if (requireSafePath)
                {
                    if (!IsStraightPathSafe(originX, originY, destX, destY, hazards, safeDistance))
                        continue;
                }

                float const toDestX = destX - originX;
                float const toDestY = destY - originY;
                float const moveDistSq = toDestX*toDestX + toDestY*toDestY;
                if (moveDistSq < bestMoveDistSq)
                {
                    bestMoveDistSq = moveDistSq;
                    outX = destX;
                    outY = destY;
                    found = true;
                }
            }
        }

        if (found)
            return true;
    }

    return false;
}

// Nightbane

float GetTerraceAngle(Position const& position)
{
    float const angle = std::atan2(
        position.GetPositionY() - TERRACE_DOME_CENTER.GetPositionY(),
        position.GetPositionX() - TERRACE_DOME_CENTER.GetPositionX());

    return angle < TERRACE_ARC_START ? angle + 2.0f * static_cast<float>(M_PI) : angle;
}

bool IsClearOfCharredEarth(Position const& position, std::vector<Position> const& charredEarths)
{
    for (Position const& charredEarth : charredEarths)
    {
        if (charredEarth.GetExactDist2d(position) < CHARRED_EARTH_REACH + CHARRED_EARTH_CLEARANCE)
            return false;
    }

    return true;
}

bool IsOutsideNightbaneTerrace(Unit const& unit)
{
    constexpr float belowTerraceLeeway = 5.0f;
    if (unit.GetPositionZ() < NIGHTBANE_GROUND_Z - belowTerraceLeeway)
        return true;

    return unit.GetExactDist2d(TERRACE_DOME_CENTER) > TERRACE_OUTER_RADIUS &&
        GetTerraceAngle(unit) <= TERRACE_ARC_END;
}

bool IsNightbaneFlying(Unit const& nightbane)
{
    return nightbane.GetPositionZ() > NIGHTBANE_FLIGHT_Z;
}

bool IsNightbaneFlightPhaseActive(Unit const& nightbane)
{
    if (!IsNightbaneFlying(nightbane))
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane.GetInstanceId());
    return !state || state->nightbaneFlightStage != NightbaneFlightStage::Landing;
}

bool IsNightbaneDpsHeld(Player* bot, Unit const& nightbane)
{
    if (IsNightbaneFlying(nightbane))
        return true;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane.GetInstanceId());
    if (!state || !state->nightbaneDpsHoldTimer)
        return true;

    return getMSTimeDiff(*state->nightbaneDpsHoldTimer, getMSTime()) < NIGHTBANE_DPS_HOLD_MS;
}

Position const* GetRainOfBonesCenter(Unit const& nightbane)
{
    Spell const* channel = nightbane.GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!channel || channel->m_spellInfo->Id != Id(KaraSpells::SPELL_RAIN_OF_BONES) ||
        !channel->m_targets.HasDst())
    {
        return nullptr;
    }

    return channel->m_targets.GetDstPos();
}

// Shared encounter state

KaraInstanceState& KaraState(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    return karaStates[instanceId];
}

KaraInstanceState const* FindKaraState(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    auto it = karaStates.find(instanceId);
    return it != karaStates.end() ? &it->second : nullptr;
}

bool KaraResetInstance(uint32 instanceId)
{
    std::lock_guard lock(karaStateMutex);
    auto it = karaStates.find(instanceId);
    if (it == karaStates.end())
        return false;

    KaraInstanceState const& state = it->second;
    bool const wasSet = state.attumenDpsWaitTimer || state.netherspiteDpsHoldTimer ||
        AreNetherspitePortalsKnown(state) || !state.netherspiteVoidZones.empty() ||
        !state.netherspiteBanishSlots.empty() || state.nightbaneDpsHoldTimer ||
        state.nightbaneFlightStage ||
        state.nightbaneFlightSpot || state.nightbaneRainOfBonesCenter;

    karaStates.erase(it);
    return wasSet;
}

}
