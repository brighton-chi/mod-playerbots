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
    if (GetManaWarpStunReach(bot) <= 0.0f || !IsNearManaWarpPacks(bot) ||
        PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
    {
        return nullptr;
    }

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

std::optional<Position> GetAttumenStackPosition(Player* bot, Unit const& attumen)
{
    float const distanceBehind = bot->getClass() == CLASS_HUNTER ?
        ATTUMEN_HUNTER_STACK_DISTANCE : ATTUMEN_STACK_DISTANCE;
    float const orientation = attumen.GetOrientation() + M_PI;
    Position const position(
        attumen.GetPositionX() + std::cos(orientation) * distanceBehind,
        attumen.GetPositionY() + std::sin(orientation) * distanceBehind,
        attumen.GetPositionZ());
    if (bot->GetExactDist2d(position) <= ATTUMEN_STACK_TOLERANCE)
        return std::nullopt;

    return position;
}

bool IsAttumenDpsHeld(Player* bot)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || !IsAttumenMounted(*midnight))
        return false;

    constexpr uint32 holdMs = 5 * IN_MILLISECONDS;
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (state && state->attumenDpsHoldTimer &&
        getMSTimeDiff(*state->attumenDpsHoldTimer, getMSTime()) >= holdMs)
    {
        return false;
    }

    return !PlayerbotAI::IsMainTank(bot);
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

bool IsLittleRedRidingHood(Player* bot)
{
    return bot->HasAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
}

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

Player* GetBigBadWolfRedHood(Unit const& wolf)
{
    Unit* victim = wolf.GetVictim();
    Player* target = victim ? victim->ToPlayer() : nullptr;
    if (!target || !IsLittleRedRidingHood(target) ||
        target->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
    {
        return nullptr;
    }

    return target;
}

Player* GetBigBadWolfTargetToProtect(Unit const& wolf)
{
    Player* target = GetBigBadWolfRedHood(wolf);
    if (!target)
        return nullptr;

    Aura const* hood = target->GetAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
    if (!hood || hood->GetMaxDuration() - hood->GetDuration() < BIG_BAD_WOLF_PROTECT_REACTION_MS)
        return nullptr;

    if (hood->GetDuration() > HAND_OF_PROTECTION_DURATION_MS &&
        target->GetHealthPct() >= BIG_BAD_WOLF_PROTECT_HEALTH_PCT)
    {
        return nullptr;
    }

    // The test his swings use, with the leeway while both move.
    if (!wolf.IsWithinMeleeRange(target) || GET_PLAYERBOT_AI(target) || PlayerbotAI::IsTank(target))
        return nullptr;

    return target;
}

// Terestian Illhoof

bool IsTerestianSacrificeActive(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(KaraSpells::SPELL_SACRIFICE)))
            return true;
    }

    return false;
}

// Shade of Aran

bool IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_ARCANE_EXPLOSION));
}

std::optional<Position> GetBlizzardEscapePosition(Player* bot)
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

bool IsFlameWreathInProgress(PlayerbotAI* botAI, Unit* aran)
{
    if (botAI->GetBot()->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
        return true;

    // A resurrected bot can't find him until he re-zones, but it can see the rings.
    if (!aran)
    {
        return !botAI->GetAiObjectContext()
            ->GetValue<std::vector<Position>>("shade of aran flame wreath rings")->Get().empty();
    }

    return aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)) ||
        aran->GetDynObject(Id(KaraSpells::SPELL_FLAME_WREATH_AURA));
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

GuidVector FindUnbanishedConjuredElementals(Player* bot)
{
    std::list<Creature*> elementals;
    bot->GetCreatureListWithEntryInGrid(
        elementals, Id(KaraNpcs::NPC_CONJURED_ELEMENTAL), ARAN_ROOM_SEARCH_RADIUS);

    GuidVector guids;
    for (Creature* elemental : elementals)
    {
        if (elemental && elemental->IsAlive() &&
            !elemental->HasAura(Id(KaraSpells::SPELL_BANISH_RANK_1)) &&
            !elemental->HasAura(Id(KaraSpells::SPELL_BANISH_RANK_2)))
        {
            guids.push_back(elemental->GetGUID());
        }
    }

    return guids;
}

// Netherspite

namespace
{

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

Position const* FindNetherspiteBlockerPosition(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.blocker == guid)
            return &beam.blockerPosition;
    }

    return nullptr;
}

Position const* FindNetherspiteNextPosition(KaraInstanceState const& state, ObjectGuid guid)
{
    for (NetherspiteBeamState const& beam : state.netherspiteBeams)
    {
        if (beam.next == guid)
            return &beam.nextPosition;
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

bool IsBetweenNetherspiteBeams(
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
            red.blockerPosition.GetExactDist2d(x, y) <= NETHERSPITE_HEALER_TANK_RANGE;
    }

    if (bot->getClass() == CLASS_HUNTER || !PlayerbotAI::IsMelee(bot))
        return true;

    // He faces the red blocker, and set behind is off.
    float const fromRed = Position::NormalizeOrientation(
        angle - GetBeamAngle(netherspite, state, NetherspiteBeam::Red));
    return fromRed > M_PI / 2.0f && fromRed < 3.0f * M_PI / 2.0f;
}

std::optional<Position> FindPositionBetweenNetherspiteBeams(
    Player* bot, Unit const& netherspite, KaraInstanceState const& state)
{
    float z = bot->GetPositionZ();
    if (!PlayerbotAI::IsTank(bot))
    {
        NetherspiteArc const arc = GetNetherspiteStandingArc(bot, netherspite, state);
        NetherspiteRadiusBand const band = GetNetherspiteRadiusBand(bot);
        for (uint8 i = 0; i < NETHERSPITE_POSITION_TRIES; ++i)
        {
            float const angle = arc.start + frand(0.0f, arc.width);
            float const radius = frand(band.low, band.high);
            float const x = netherspite.GetPositionX() + std::cos(angle) * radius;
            float const y = netherspite.GetPositionY() + std::sin(angle) * radius;
            if (IsBetweenNetherspiteBeams(bot, netherspite, state, x, y) &&
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

std::vector<Position> FindLandedInfernals(Player* bot)
{
    std::vector<Position> infernals;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), INFERNAL_SEARCH_RADIUS);

    for (Creature* creature : creatureList)
    {
        if (creature->IsAlive() && creature->GetDisplayId() != INFERNAL_MODEL_INVISIBLE)
            infernals.push_back(creature->GetPosition());
    }

    return infernals;
}

std::vector<Position> GetLandedInfernals(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("prince malchezaar landed infernals")->Get();
}

bool IsClearOfInfernals(
    Position const& position, std::vector<Position> const& infernals, float clearance)
{
    return std::none_of(infernals.begin(), infernals.end(), [&](Position const& infernal)
    {
        return infernal.GetExactDist2d(position) < clearance;
    });
}

bool IsLineClearOfInfernals(
    Position const& from, Position const& to, std::vector<Position> const& infernals)
{
    return std::none_of(infernals.begin(), infernals.end(), [&](Position const& infernal)
    {
        return infernal.GetExactDist2d(from) >= HELLFIRE_RADIUS &&
            GetDistanceToSegment2d(infernal, from, to) < HELLFIRE_RADIUS;
    });
}

bool CanWalkStraightTo(Player* bot, Position const& destination)
{
    float x = destination.GetPositionX();
    float y = destination.GetPositionY();
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = bot->GetPositionZ();

    if (!bot->GetMap()->CanReachPositionAndGetValidCoords(bot, x, y, z, true, false))
        return false;

    // An incomplete path can cut the point short without counting as a collision.
    constexpr float truncationTolerance = 1.0f;
    return std::hypot(x - destination.GetPositionX(), y - destination.GetPositionY()) <=
        truncationTolerance;
}

std::optional<Position> FindMalchezaarClearPosition(
    PlayerbotAI* botAI, Unit const& malchezaar, std::vector<Position> const& infernals)
{
    Player* bot = botAI->GetBot();
    bool const isRanged = PlayerbotAI::IsRanged(bot);
    bool const isHealer = PlayerbotAI::IsHeal(bot);
    Unit const* target = botAI->GetAiObjectContext()
        ->GetValue<Unit*>(isHealer ? "party member to heal" : "current target")->Get();
    if (!target)
        target = &malchezaar;

    constexpr float rangeInset = 1.0f;
    float const reach = (isRanged ?
        botAI->GetRange(isHealer ? "heal" : "spell") + bot->GetCombatReach() +
            target->GetCombatReach() :
        bot->GetMeleeRange(target)) - rangeInset;

    // Ranged go to a ring just out of Shadow Nova and melee to his melee ring, so the nearest point
    // is straight out, or straight in, from where the bot stands.
    Unit const* center = isRanged ? &malchezaar : target;
    float const ringRadius = isRanged ? SHADOW_NOVA_CLEARANCE : reach;
    Position const origin = bot->GetPosition();

    constexpr uint8 angleCount = 72;
    std::vector<std::pair<float, Position>> candidates;
    for (uint8 i = 0; i < angleCount; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * i / angleCount;
        Position const candidate(
            center->GetPositionX() + ringRadius * std::cos(angle),
            center->GetPositionY() + ringRadius * std::sin(angle), origin.GetPositionZ());

        if (IsClearOfInfernals(candidate, infernals, HELLFIRE_CLEARANCE) &&
            (target == center || target->GetExactDist2d(candidate) <= reach))
        {
            candidates.emplace_back(origin.GetExactDist2d(candidate), candidate);
        }
    }

    std::stable_sort(candidates.begin(), candidates.end(), [](auto const& a, auto const& b)
    {
        return a.first < b.first;
    });

    for (bool const clearLine : { true, false })
    {
        for (auto const& [distance, candidate] : candidates)
        {
            if ((!clearLine || IsLineClearOfInfernals(origin, candidate, infernals)) &&
                CanWalkStraightTo(bot, candidate))
            {
                return candidate;
            }
        }
    }

    constexpr float searchDistance = 60.0f;
    return FindMalchezaarPosition(
        bot, origin, infernals, searchDistance, true, [&](Position const& candidate)
        {
            return IsClearOfInfernals(candidate, infernals, HELLFIRE_CLEARANCE) &&
                (!isRanged || malchezaar.GetExactDist2d(candidate) >= SHADOW_NOVA_CLEARANCE);
        });
}

bool IsClearOfMalchezaarHazards(
    Player* bot, Unit const& malchezaar, std::vector<Position> const& infernals,
    Position const& position)
{
    return IsClearOfInfernals(position, infernals, HELLFIRE_CLEARANCE) &&
        (!PlayerbotAI::IsRanged(bot) ||
         malchezaar.GetExactDist2d(position) >= SHADOW_NOVA_MOVE_DISTANCE);
}

std::optional<Position> FindMalchezaarTankPosition(
    Player* bot, std::vector<Position> const& infernals, float& clearance)
{
    constexpr float searchDistance = 60.0f;
    constexpr float clearances[] =
    {
        MALCHEZAAR_RING_CLEARANCE + MALCHEZAAR_TRAIL_DISTANCE,
        MALCHEZAAR_RING_CLEARANCE,
        HELLFIRE_CLEARANCE + 1.0f,
    };

    for (float const candidateClearance : clearances)
    {
        std::optional<Position> const position = FindMalchezaarPosition(
            bot, bot->GetPosition(), infernals, searchDistance, true,
            [&](Position const& candidate)
            {
                return IsClearOfInfernals(candidate, infernals, candidateClearance);
            });

        if (position)
        {
            clearance = candidateClearance;
            return position;
        }
    }

    return std::nullopt;
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
    bool const wasSet = state.attumenDpsHoldTimer || state.netherspiteDpsHoldTimer ||
        AreNetherspitePortalsKnown(state) || !state.netherspiteVoidZones.empty() ||
        !state.netherspiteBanishSlots.empty() || state.nightbaneDpsHoldTimer ||
        state.nightbaneFlightStage ||
        state.nightbaneFlightStackPosition || state.nightbaneRainOfBonesCenter;

    karaStates.erase(it);
    return wasSet;
}

}
