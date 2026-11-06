/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "ObjectAccessor.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Random.h"
#include "RtiTargetValue.h"
#include "ThreatHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <string>

using namespace KaraHelpers;
using namespace EncounterHelpers;

// Shared

bool KarazhanResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    bool reset = false;

    Action* redAction = context->GetAction("netherspite block red beam");
    if (redAction &&
        static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
    {
        reset = true;
    }

    Action* areaAction = context->GetAction("netherspite move to beam area");
    if (areaAction &&
        static_cast<NetherspiteMoveToBeamAreaAction*>(areaAction)->ResetDestination())
    {
        reset = true;
    }

    Action* wolfAction = context->GetAction("big bad wolf red hood run away");
    if (wolfAction &&
        static_cast<BigBadWolfRedHoodRunAwayAction*>(wolfAction)->ResetRun())
    {
        reset = true;
    }

    Action* petAction = context->GetAction("nightbane control pet aggression");
    if (petAction &&
        static_cast<NightbaneControlPetAggressionAction*>(petAction)->ResetPetStance())
    {
        reset = true;
    }

    if (!IsMechanicTrackerBot(bot, KARA_MAP_ID))
        return reset;

    reset |= ClearStrategyTargetIcon(bot, RtiTargetValue::skullIndex, {
        Id(KaraNpcs::NPC_BARONESS_DOROTHEA_MILLSTIPE),
        Id(KaraNpcs::NPC_LADY_CATRIONA_VON_INDI),
        Id(KaraNpcs::NPC_LADY_KEIRA_BERRYBUCK),
        Id(KaraNpcs::NPC_BARON_RAFE_DREUGER),
        Id(KaraNpcs::NPC_LORD_ROBIN_DARIS),
        Id(KaraNpcs::NPC_LORD_CRISPIN_FERENCE),
        Id(KaraNpcs::NPC_ROMULO),
        Id(KaraNpcs::NPC_JULIANNE),
        Id(KaraNpcs::NPC_DOROTHEE),
        Id(KaraNpcs::NPC_TITO),
        Id(KaraNpcs::NPC_ROAR),
        Id(KaraNpcs::NPC_STRAWMAN),
        Id(KaraNpcs::NPC_TINHEAD),
        Id(KaraNpcs::NPC_KILREK),
        Id(KaraNpcs::NPC_TERESTIAN_ILLHOOF),
    });

    reset |= KaraResetInstance(instanceId);

    return reset;
}

bool KarazhanSetTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot);
}

// Trash

ManaWarpStunWarpBreachAction::ManaWarpStunWarpBreachAction(PlayerbotAI* botAI)
    : Action(botAI, "mana warp stun warp breach"),
      _reactionMs(urand(WARP_BREACH_MIN_REACTION_MS, WARP_BREACH_MAX_REACTION_MS)) {}

bool ManaWarpStunWarpBreachAction::Execute(Event /*event*/)
{
    Unit* warp = GetManaWarpToStun(botAI, _reactionMs);
    if (!warp)
        return false;

    auto const castStun = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, warp) && botAI->CastSpell(spell, warp);
    };

    // The warp is within the bot's stun reach, but a Tauren's reach is War Stomp's, so melee stuns
    // still test melee range: CanCastSpell passes targets out of range, and a failed cast still
    // turns the bot.
    bool const inMeleeRange = bot->IsWithinMeleeRange(warp);
    bool stunned = false;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            ShapeshiftForm const form = bot->GetShapeshiftForm();
            if (form == FORM_BEAR || form == FORM_DIREBEAR)
                stunned = inMeleeRange && castStun("bash");
            else if (form == FORM_CAT)
                stunned = inMeleeRange && castStun("maim");
            break;
        }
        case CLASS_PALADIN:
            stunned = castStun("hammer of justice");
            break;
        case CLASS_ROGUE:
            stunned = inMeleeRange && castStun("kidney shot");
            break;
        case CLASS_WARLOCK:
            stunned = castStun("shadowfury");
            break;
        case CLASS_WARRIOR:
            stunned = inMeleeRange && (castStun("concussion blow") || castStun("shockwave"));
            break;
        default:
            break;
    }

    if (stunned)
        return true;

    return bot->getRace() == RACE_TAUREN && bot->GetExactDist(warp) <= WAR_STOMP_RADIUS &&
        botAI->CanCastSpell("war stomp", bot) && botAI->CastSpell("war stomp", bot);
}

// Holding casts nothing, so the stun is off the global cooldown when the cast bar comes.
bool ManaWarpStunnerHoldForWarpBreachAction::Execute(Event /*event*/)
{
    Unit* warp = GetManaWarpToHoldFor(botAI);
    if (!warp)
        return false;

    float const reach = GetManaWarpStunReach(bot);
    if (bot->GetExactDist(warp) <= reach)
        return true;

    // MoveTo measures edge to edge.
    float const distance = std::max(
        0.0f, reach - MANA_WARP_STUN_APPROACH_MARGIN - bot->GetCombatReach() -
            warp->GetCombatReach());
    return MoveTo(warp, distance, MovementPriority::MOVEMENT_COMBAT);
}

// Attumen the Huntsman

// Midnight's CombatReach is 1.6 yards
// Unmounted Attumen's CombatReach is 1.5 yards
// Steps the tank away from every player in reach of Shadow Cleave (from the tank) or Intangible
// Presence (from Attumen), until each is clear.
bool AttumenTheHuntsmanAssistTankPositionUnmountedAction::Execute(Event /*event*/)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    Unit* attumen = GetKaraBoss(bot, KaraBosses::DATA_ATTUMEN);
    if (!midnight || !attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() != bot || midnight->GetVictim() == bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float shortfall = 0.0f;
    float sumX = 0.0f;
    float sumY = 0.0f;
    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID)
            continue;

        // A cleave's jump adds the member's reach; Intangible Presence doesn't.
        float const cleaveJump = SHADOW_CLEAVE_JUMP_RADIUS + member->GetCombatReach();
        float const memberShortfall = ATTUMEN_HAZARD_CLEARANCE + std::max(
            cleaveJump - bot->GetExactDist2d(member),
            INTANGIBLE_PRESENCE_RADIUS - attumen->GetExactDist2d(member));
        if (memberShortfall <= 0.0f)
            continue;

        shortfall = std::max(shortfall, memberShortfall);
        sumX += member->GetPositionX();
        sumY += member->GetPositionY();
        ++count;
    }

    if (!count)
        return false;

    float const angle = bot->GetAngle(sumX / count, sumY / count) + M_PI;
    Position const destination(
        bot->GetPositionX() + std::cos(angle) * shortfall,
        bot->GetPositionY() + std::sin(angle) * shortfall, bot->GetPositionZ());

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, 0.0f, attumen, moveX, moveY, backwards))
        return false;

    float moveZ = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), moveX, moveY,
            moveZ, false))
    {
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool AttumenTheHuntsmanAttackMidnightAction::Execute(Event /*event*/)
{
    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && AI_VALUE(Unit*, "current target") != midnight && Attack(midnight);
}

bool AttumenTheHuntsmanTankPositionMountedAction::Execute(Event /*event*/)
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() != bot || !bot->IsWithinMeleeRange(attumen))
        return false;

    Position const& position = ATTUMEN_TANK_POSITION;
    if (!bot->IsWithinLOS(
            position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()))
    {
        return false;
    }

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, attumen, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Mounted Attumen's CombatReach is 0 yards so defaults to 1.5 yards
bool AttumenTheHuntsmanStackBehindAction::Execute(Event /*event*/)
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen)
        return false;

    float const distanceBehind = bot->getClass() == CLASS_HUNTER ?
        ATTUMEN_HUNTER_STACK_DISTANCE : ATTUMEN_STACK_DISTANCE;
    float const orientation = attumen->GetOrientation() + M_PI;
    float const rearX = attumen->GetPositionX() + std::cos(orientation) * distanceBehind;
    float const rearY = attumen->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (bot->GetExactDist2d(rearX, rearY) <= ATTUMEN_STACK_TOLERANCE)
        return false;

    return MoveTo(
        KARA_MAP_ID, rearX, rearY, attumen->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AttumenTheHuntsmanManageDpsTimerAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();
    return EmplaceIfUnset(KaraState(bot->GetInstanceId()).attumenDpsWaitTimer, now);
}

// Moroes

bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    static constexpr std::array moroesGuests = {
        "baroness dorothea millstipe",
        "lady catriona von'indi",
        "lady keira berrybuck",
        "baron rafe dreuger",
        "lord robin daris",
        "lord crispin ference",
    };

    for (char const* name : moroesGuests)
    {
        if (Unit* guest = FindThreatTarget(bot, name))
            return MarkTargetWithSkull(bot, guest);
    }

    return false;
}

// Maiden of Virtue
// CombatReach is 4.8 yards

bool MaidenOfVirtueTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* maiden = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
    if (!maiden)
        return false;

    if (AI_VALUE(Unit*, "current target") != maiden)
        return Attack(maiden);

    if (maiden->GetVictim() != bot || !bot->IsWithinMeleeRange(maiden))
        return false;

    // While a healer is stunned she is taken to them, and stays until Holy Ground breaks it.
    if (GetNearestRepentantHealer(bot, *maiden))
        return false;

    Position const& position = MAIDEN_OF_VIRTUE_POSITION;
    float const distToPosition = maiden->GetExactDist2d(position);
    if (distToPosition <= MAIDEN_OF_VIRTUE_POSITION_TOLERANCE)
        return false;

    // The tank leads her along the line from her through her spot, so it walks through her when
    // she is between it and the spot.
    float const ratio = MAIDEN_OF_VIRTUE_TANK_LEAD_DISTANCE / distToPosition;
    Position const destination(
        position.GetPositionX() + (position.GetPositionX() - maiden->GetPositionX()) * ratio,
        position.GetPositionY() + (position.GetPositionY() - maiden->GetPositionY()) * ratio,
        bot->GetPositionZ());

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, arrivalDist, maiden, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MaidenOfVirtueMoveBossToStunnedHealerAction::Execute(Event /*event*/)
{
    Unit* maiden = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
    if (!maiden || maiden->GetVictim() != bot)
        return false;

    Player* healer = GetNearestRepentantHealer(bot, *maiden);
    if (!healer)
        return false;

    if (maiden->GetExactDist2d(healer) <= HOLY_GROUND_RADIUS - MAIDEN_REPENTANCE_BREAK_CLEARANCE)
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, *healer, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MaidenOfVirtuePositionRangedBetweenPillarsAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr uint8 maxIndex = 7;
    uint8 index = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (member == bot)
            break;

        if (index >= maxIndex)
        {
            index = 0;
            continue;
        }
        index++;
    }

    Position const& position = MAIDEN_OF_VIRTUE_RANGED_POSITIONS[index];
    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        KARA_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtueSetGroundingTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot);
}

// The Big Bad Wolf

bool BigBadWolfPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    if (AI_VALUE(Unit*, "current target") != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() != bot || !bot->IsWithinMeleeRange(wolf))
        return false;

    Position const& position = BIG_BAD_WOLF_TANK_POSITION;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, wolf, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Run away, little girl, run away
bool BigBadWolfRedHoodRunAwayAction::Execute(Event /*event*/)
{
    Aura* hood = bot->GetAura(Id(KaraSpells::SPELL_LITTLE_RED_RIDING_HOOD));
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!hood || !wolf)
        return false;

    // Hood silences and pacifies, but only stops silence-type casts. A cast like Steady Shot
    // would keep filling and hold the bot still.
    bot->CastStop();

    if (GetMSTimeDiffToNow(_lastRunStepTime) > BIG_BAD_WOLF_RUN_RESTART_MS)
    {
        _run = GetBigBadWolfRunPlan(
            bot, *wolf, hood->GetDuration() / static_cast<float>(IN_MILLISECONDS));
    }

    if (bot->GetExactDist2d(BIG_BAD_WOLF_RUN_POSITIONS[_run.index]) <
        BIG_BAD_WOLF_RUN_TURN_DISTANCE)
    {
        _run.index = GetNextBigBadWolfRunIndex(_run.index, _run.direction);
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, BIG_BAD_WOLF_RUN_POSITIONS[_run.index], BIG_BAD_WOLF_RUN_TURN_DISTANCE, nullptr,
            moveX, moveY, backwards))
    {
        return false;
    }

    if (!MoveTo(
            KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, backwards))
    {
        return false;
    }

    _lastRunStepTime = getMSTime();
    return true;
}

bool BigBadWolfCastHandOfProtectionOnRedHoodAction::Execute(Event /*event*/)
{
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    Player* target = GetBigBadWolfTargetToProtect(*wolf);
    if (!target || target == bot || !bot->IsWithinCombatRange(target, HAND_OF_PROTECTION_RANGE))
        return false;

    return botAI->CanCastSpell("hand of protection", target) &&
        botAI->CastSpell("hand of protection", target);
}

// Romulo and Julianne

bool RomuloAndJulianneMarkTargetAction::Execute(Event /*event*/)
{
    Unit* romulo = GetOperaActor(bot, "romulo");
    if (!romulo)
        return false;

    Unit* julianne = GetOperaActor(bot, "julianne");
    if (!julianne)
        return false;

    // Faking death
    bool const romuloDown = romulo->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    bool const julianneDown = julianne->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    if (romuloDown && julianneDown)
        return false;

    if (romuloDown)
        return MarkTargetWithSkull(bot, julianne);

    if (julianneDown)
        return MarkTargetWithSkull(bot, romulo);

    // Within the gap the skull stays where it is; phase 2 leaves it on Romulo.
    Unit* healthier = romulo->GetHealthPct() >= julianne->GetHealthPct() ? romulo : julianne;
    Unit* other = healthier == romulo ? julianne : romulo;
    if (healthier->GetHealthPct() - other->GetHealthPct() <= ROMULO_AND_JULIANNE_HEALTH_GAP_PCT)
        return false;

    return MarkTargetWithSkull(bot, healthier);
}

// The Wizard of Oz

bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    for (char const* actor : OZ_TARGETS)
    {
        if (Unit* target = GetOperaActor(bot, actor))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

bool WizardOfOzCastScorchOnStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = GetOperaActor(bot, "strawman");
    if (!strawman)
        return false;

    // Both measure edge to edge.
    if (!bot->IsWithinCombatRange(strawman, SCORCH_RANGE))
    {
        return MoveTo(
            strawman, SCORCH_RANGE - SCORCH_APPROACH_MARGIN, MovementPriority::MOVEMENT_COMBAT);
    }

    return botAI->CanCastSpell("scorch", strawman) && botAI->CastSpell("scorch", strawman);
}

// The Curator

bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = FindThreatTarget(bot, "astral flare");
    return flare && MarkTargetWithSkull(bot, flare);
}

bool TheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = GetKaraBoss(bot, KaraBosses::DATA_CURATOR);
    if (!curator)
        return false;

    if (AI_VALUE(Unit*, "current target") != curator)
        return Attack(curator);

    if (curator->GetVictim() != bot || !bot->IsWithinMeleeRange(curator))
        return false;

    Position const& position = THE_CURATOR_TANK_POSITION;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, curator, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, THE_CURATOR_RANGED_SPREAD_DISTANCE);
    return nearestPlayer &&
        FleePosition(nearestPlayer->GetPosition(), THE_CURATOR_RANGED_SPREAD_DISTANCE);
}

// Terestian Illhoof

bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 100.0f;
    static constexpr std::array illhoofAdds = {
        KaraNpcs::NPC_DEMON_CHAINS,
        KaraNpcs::NPC_KILREK,
    };

    for (KaraNpcs add : illhoofAdds)
    {
        if (Unit* target = bot->FindNearestCreature(Id(add), searchRadius))
            return MarkTargetWithSkull(bot, target);
    }

    Unit* terestian = GetKaraBoss(bot, KaraBosses::DATA_TERESTIAN);
    return terestian && MarkTargetWithSkull(bot, terestian);
}

// Shade of Aran

bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
    if (!aran)
        return false;

    float const distance = bot->GetExactDist(aran);
    if (distance >= ARAN_ARCANE_EXPLOSION_CLEARANCE)
        return false;

    bot->CastStop();
    return MoveAway(aran, ARAN_ARCANE_EXPLOSION_CLEARANCE - distance);
}

// Ranged only: a patch's reach stops short of melee range on Aran.
bool ShadeOfAranRunOutOfBlizzardAction::Execute(Event /*event*/)
{
    if (!IsMovingAllowed() || IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        return false;

    std::optional<Position> const spot = GetBlizzardEscapeSpot(bot);
    if (!spot)
        return false;

    // A filling cast would hold the bot still.
    bot->CastStop();

    return MoveTo(
        KARA_MAP_ID, spot->GetPositionX(), spot->GetPositionY(), spot->GetPositionZ(), false,
        false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// I will not move when Flame Wreath is cast or the raid blows up
bool ShadeOfAranStopMovingDuringFlameWreathAction::Execute(Event /*event*/)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    if (!bot->isMoving())
        return false;

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    return true;
}

bool ShadeOfAranMarkConjuredElementalAction::Execute(Event /*event*/)
{
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 75.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(KaraNpcs::NPC_CONJURED_ELEMENTAL), searchRadius);

    for (Creature* elemental : creatureList)
    {
        if (!elemental || !elemental->IsAlive())
            continue;

        if (!botAI->HasAura("banish", elemental))
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    Unit* aran = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
    if (!aran)
        return false;

    float const distance = bot->GetExactDist2d(aran);
    float goal;
    if (distance > ARAN_RANGED_MAX_DISTANCE)
        goal = ARAN_RANGED_MAX_DISTANCE;
    else if (distance < ARAN_RANGED_MIN_DISTANCE)
        goal = ARAN_RANGED_MIN_DISTANCE;
    else
        return false;

    float const angle = aran->GetAngle(bot);
    Position const spot(
        aran->GetPositionX() + goal * std::cos(angle),
        aran->GetPositionY() + goal * std::sin(angle), bot->GetPositionZ());

    // Wait out a Blizzard rather than walk back into it.
    float const patchReach = ARAN_BLIZZARD_HAZARD_RADIUS + bot->GetCombatReach();
    for (Position const& patch : GetDynamicObjectPositions(
             bot, ARAN_ROOM_SEARCH_RADIUS, Id(KaraSpells::SPELL_BLIZZARD)))
    {
        if (patch.GetExactDist2d(spot) <= patchReach)
            return false;
    }

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, spot, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Netherspite
// CombatReach is 18 yards

// The red beam dance (5 seconds in, 5 seconds out)
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state)
        return false;

    NetherspiteBeamState const& beam = state->netherspiteBeams[Id(NetherspiteBeam::Red)];
    if (beam.blocker != bot->GetGUID() || !beam.portal)
        return false;

    if (bot->HasAura(Id(KaraSpells::SPELL_RED_BEAM_DEBUFF)))
    {
        if (!_redBeamTimerWasSet)
        {
            _redBeamMoveTimer = getMSTime();
            _redBeamTimerWasSet = true;
        }
    }
    else
    {
        _redBeamTimerWasSet = false;
        _lastBeamMoveSideways = false;
    }

    constexpr uint32 intervalMs = 5 * IN_MILLISECONDS;
    if (_redBeamTimerWasSet && GetMSTimeDiffToNow(_redBeamMoveTimer) >= intervalMs)
    {
        _lastBeamMoveSideways = !_lastBeamMoveSideways;
        _redBeamMoveTimer = getMSTime();
    }

    float x = beam.blockerSpot.GetPositionX();
    float y = beam.blockerSpot.GetPositionY();
    if (_lastBeamMoveSideways)
    {
        constexpr float sideDistance = 3.0f;
        float const length = netherspite->GetExactDist2d(*beam.portal);
        if (length == 0.0f)
            return false;

        x -= (beam.portal->GetPositionY() - netherspite->GetPositionY()) / length * sideDistance;
        y += (beam.portal->GetPositionX() - netherspite->GetPositionX()) / length * sideDistance;
    }

    if (bot->GetExactDist2d(x, y) <= NETHERSPITE_BEAM_SPOT_TOLERANCE)
        return false;

    return MoveTo(
        KARA_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteBlockBeamAction::Execute(Event /*event*/)
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* spot = state ? FindNetherspiteBlockerSpot(*state, bot->GetGUID()) : nullptr;
    if (!spot)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, spot->GetPositionX(), spot->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteWaitToBlockBeamAction::Execute(Event /*event*/)
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* spot = state ? FindNetherspiteNextSpot(*state, bot->GetGUID()) : nullptr;
    if (!spot)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, spot->GetPositionX(), spot->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteMoveToBeamAreaAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state || !AreNetherspitePortalsKnown(*state))
        return false;

    if (!_destination || !IsNetherspiteStandingSpotGood(
            bot, *netherspite, *state, _destination->GetPositionX(), _destination->GetPositionY()))
    {
        _destination = FindNetherspiteStandingSpot(bot, *netherspite, *state);
        if (!_destination)
            return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, _destination->GetPositionX(), _destination->GetPositionY(),
        bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
        false);
}

bool NetherspiteMoveToBanishSlotAction::Execute(Event /*event*/)
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    NetherspiteBanishSlot const* slot =
        state ? FindNetherspiteBanishSlot(*state, bot->GetGUID()) : nullptr;
    if (!slot)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, slot->spot.GetPositionX(), slot->spot.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteStopAttackingAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    return StopAttackingTarget(bot, *netherspite);
}

bool NetherspiteSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool NetherspiteSelectTargetForHoldAction::Execute(Event /*event*/)
{
    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

namespace
{

constexpr uint8 NETHERSPITE_BEAM_STACK_LIMIT = 25;
// The next blocker needs a moment, and a cast it can't stop, to get on the beam.
constexpr uint8 NETHERSPITE_NEXT_STEPS_ON_AT_STACKS = 24;
constexpr uint8 NETHERSPITE_SPOT_TRIES = 12;
constexpr float NETHERSPITE_BLOCKER_DISTANCE = 18.0f;
constexpr float NETHERSPITE_BLOCKER_MIN_DISTANCE = 17.0f;
constexpr float NETHERSPITE_HUNTER_BLOCKER_DISTANCE = 25.0f;
constexpr float NETHERSPITE_BEAM_SEARCH_MAX_DISTANCE = 34.0f;
constexpr float NETHERSPITE_NEXT_MIN_OFF_LINE = 3.5f;
constexpr float NETHERSPITE_NEXT_MAX_OFF_LINE = 6.0f;
constexpr float NETHERSPITE_NEXT_ALONG_RANGE = 2.5f;

bool HasBeamStackLimit(Player* member, NetherspiteBeam beam)
{
    if (beam == NetherspiteBeam::Red)
        return false;

    return beam == NetherspiteBeam::Blue || member->getPowerType() == POWER_MANA ||
        PlayerbotAI::IsHeal(member);
}

uint8 GetBeamStacks(Player* member, NetherspiteBeamData const& data)
{
    Aura const* buff = member->GetAura(Id(data.buff));
    return buff ? buff->GetStackAmount() : 0;
}

// Green: non-mana dps, who can hold it a whole phase, then healers, then mana dps.
// Blue: mana dps, then non-mana dps. Red: tanks.
// The current blocker holds until the limit. Anyone else is done a stack before it, as they'd
// reach the limit on their first tick after stepping on.
std::vector<Player*> GetBeamCandidates(
    Player* bot, NetherspiteBeamData const& data, NetherspiteBeamState const& beamState,
    std::vector<ObjectGuid> const& taken)
{
    std::vector<Player*> tiers[3];
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || member->HasAura(Id(data.exhaustion)))
        {
            continue;
        }

        ObjectGuid const guid = member->GetGUID();
        if (guid != beamState.blocker && guid != beamState.next &&
            std::find(taken.begin(), taken.end(), guid) != taken.end())
        {
            continue;
        }

        uint8 const stackLimit = guid == beamState.blocker ?
            NETHERSPITE_BEAM_STACK_LIMIT : NETHERSPITE_NEXT_STEPS_ON_AT_STACKS;
        if (HasBeamStackLimit(member, data.beam) && GetBeamStacks(member, data) >= stackLimit)
            continue;

        bool const isMana = member->getPowerType() == POWER_MANA;
        bool const isDps = PlayerbotAI::IsDps(member);
        bool const isHealer = PlayerbotAI::IsHeal(member);
        switch (data.beam)
        {
            case NetherspiteBeam::Red:
                if (PlayerbotAI::IsTank(member))
                    tiers[0].push_back(member);
                break;
            case NetherspiteBeam::Green:
                if (isDps && !isMana)
                    tiers[0].push_back(member);
                else if (isHealer)
                    tiers[1].push_back(member);
                else if (isDps)
                    tiers[2].push_back(member);
                break;
            case NetherspiteBeam::Blue:
                if (isDps && isMana)
                    tiers[0].push_back(member);
                else if (isDps)
                    tiers[1].push_back(member);
                break;
        }
    }

    std::vector<Player*> candidates;
    for (std::vector<Player*> const& tier : tiers)
        candidates.insert(candidates.end(), tier.begin(), tier.end());

    return candidates;
}

// The blocker's spot on the line: the clear point nearest where it wants to stand.
float FindBeamSpotDistance(
    Unit const& netherspite, Position const& portal, KaraInstanceState const& state,
    Player* blocker)
{
    bool const isHunter = blocker->getClass() == CLASS_HUNTER;
    float const preferred =
        isHunter ? NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_DISTANCE;
    float const low =
        isHunter ? NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_MIN_DISTANCE;
    float const length = netherspite.GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;

    constexpr float searchStep = 0.5f;
    float best = preferred;
    float bestGap = std::numeric_limits<float>::max();
    for (float distance = low; distance <= NETHERSPITE_BEAM_SEARCH_MAX_DISTANCE;
         distance += searchStep)
    {
        float const x = netherspite.GetPositionX() + dirX * distance;
        float const y = netherspite.GetPositionY() + dirY * distance;
        float const gap = std::abs(distance - preferred);
        if (gap < bestGap && IsClearOfVoidZones(state, x, y, VOID_ZONE_BEAM_SPOT_CLEARANCE))
        {
            best = distance;
            bestGap = gap;
        }
    }

    return best;
}

Position GetSpotOnBeam(Unit const& netherspite, Position const& portal, float distance)
{
    float const length = netherspite.GetExactDist2d(portal);
    return Position(
        netherspite.GetPositionX() +
            (portal.GetPositionX() - netherspite.GetPositionX()) / length * distance,
        netherspite.GetPositionY() +
            (portal.GetPositionY() - netherspite.GetPositionY()) / length * distance,
        netherspite.GetPositionZ());
}

bool IsNextSpotGood(
    Unit const& netherspite, Position const& portal, KaraInstanceState const& state, float side,
    Position const& spot, float low)
{
    NetherspiteBeamOffset const offset = GetNetherspiteBeamOffset(
        netherspite, portal, spot.GetPositionX(), spot.GetPositionY());
    float const off = offset.off * side;
    return off >= NETHERSPITE_NEXT_MIN_OFF_LINE - NETHERSPITE_BEAM_SPOT_TOLERANCE &&
        off <= NETHERSPITE_NEXT_MAX_OFF_LINE + NETHERSPITE_BEAM_SPOT_TOLERANCE &&
        offset.along >= low - NETHERSPITE_BEAM_SPOT_TOLERANCE &&
        offset.along <= low + NETHERSPITE_NEXT_ALONG_RANGE + NETHERSPITE_BEAM_SPOT_TOLERANCE &&
        IsClearOfVoidZones(state, spot.GetPositionX(), spot.GetPositionY(), VOID_ZONE_CLEARANCE);
}

// Off the line on the side facing the other beam, so a move out of a void zone can't cross it.
std::optional<Position> FindNextSpot(
    Player* bot, Unit const& netherspite, Position const& portal,
    KaraInstanceState const& state, float side, float low)
{
    float const length = netherspite.GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;
    for (uint8 i = 0; i < NETHERSPITE_SPOT_TRIES; ++i)
    {
        float const along = low + frand(0.0f, NETHERSPITE_NEXT_ALONG_RANGE);
        float const off =
            side * frand(NETHERSPITE_NEXT_MIN_OFF_LINE, NETHERSPITE_NEXT_MAX_OFF_LINE);
        float x = netherspite.GetPositionX() + dirX * along - dirY * off;
        float y = netherspite.GetPositionY() + dirY * along + dirX * off;
        if (!IsClearOfVoidZones(state, x, y, VOID_ZONE_CLEARANCE))
            continue;

        float z = netherspite.GetPositionZ();
        float const wantedX = x;
        float const wantedY = y;
        if (bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, netherspite.GetPositionX(), netherspite.GetPositionY(),
                netherspite.GetPositionZ(), x, y, z, false) &&
            std::abs(x - wantedX) < 1.0f && std::abs(y - wantedY) < 1.0f)
        {
            return Position(x, y, z);
        }
    }

    return std::nullopt;
}

void YellBeamText(Player* member, std::string const& key, std::string const& text)
{
    std::map<std::string, std::string> placeholders{{"%player", member->GetName()}};
    member->Yell(
        PlayerbotTextMgr::instance().GetBotTextOrDefault(key, text, placeholders), LANG_UNIVERSAL);
}

void YellBeamHandover(Player* leaving, Player* blocking, NetherspiteBeam beam)
{
    if (leaving && leaving->IsAlive())
    {
        if (beam == NetherspiteBeam::Green)
        {
            YellBeamText(
                leaving, "netherspite_beam_leaving_green",
                "%player is leaving the green beam. Next blocker up!");
        }
        else if (beam == NetherspiteBeam::Blue)
        {
            YellBeamText(
                leaving, "netherspite_beam_leaving_blue",
                "%player is leaving the blue beam. Next blocker up!");
        }
    }

    if (!blocking)
        return;

    switch (beam)
    {
        case NetherspiteBeam::Red:
            YellBeamText(
                blocking, "netherspite_beam_blocking_red",
                "%player is moving to block the red beam!");
            break;
        case NetherspiteBeam::Green:
            YellBeamText(
                blocking, "netherspite_beam_blocking_green",
                "%player is moving to block the green beam!");
            break;
        case NetherspiteBeam::Blue:
            YellBeamText(
                blocking, "netherspite_beam_blocking_blue",
                "%player is moving to block the blue beam!");
            break;
    }
}

void UpdateBeamBlockers(
    Player* bot, Unit const& netherspite, KaraInstanceState& state,
    NetherspiteBeamData const& data, std::vector<ObjectGuid>& taken)
{
    NetherspiteBeamState& beamState = state.netherspiteBeams[Id(data.beam)];
    if (!beamState.portal || netherspite.GetExactDist2d(*beamState.portal) == 0.0f)
        return;

    std::vector<Player*> const candidates = GetBeamCandidates(bot, data, beamState, taken);
    Player* blocker = ObjectAccessor::GetPlayer(*bot, beamState.blocker);
    Player* next = nullptr;
    bool const isBlockerEligible = blocker &&
        std::find(candidates.begin(), candidates.end(), blocker) != candidates.end();
    bool const isBlockerAtLimit = blocker && !isBlockerEligible && blocker->IsAlive() &&
        !blocker->HasAura(Id(data.exhaustion)) && HasBeamStackLimit(blocker, data.beam);

    if (data.beam != NetherspiteBeam::Red)
    {
        Player* wanted = ObjectAccessor::GetPlayer(*bot, beamState.next);
        for (Player* candidate : candidates)
        {
            if (candidate == blocker)
                continue;
            if (!next || candidate == wanted)
                next = candidate;
            if (candidate == wanted)
                break;
        }
    }

    Player* newBlocker = blocker;
    bool const isDueToStepOn = blocker && next && HasBeamStackLimit(blocker, data.beam) &&
        GetBeamStacks(blocker, data) >= NETHERSPITE_NEXT_STEPS_ON_AT_STACKS;
    if (!isBlockerEligible && !isBlockerAtLimit)
    {
        // Dead, gone or exhausted.
        newBlocker = next ? next : (candidates.empty() ? nullptr : candidates.front());
    }
    else if (isDueToStepOn && IsOnNetherspiteBeam(netherspite, *beamState.portal, *next))
    {
        newBlocker = next;
    }
    else if (isBlockerAtLimit && !next)
    {
        newBlocker = nullptr;
    }

    if (newBlocker != blocker)
    {
        YellBeamHandover(blocker, newBlocker, data.beam);
        blocker = newBlocker;
        next = nullptr;
        if (data.beam != NetherspiteBeam::Red)
        {
            for (Player* candidate : candidates)
            {
                if (candidate != blocker)
                {
                    next = candidate;
                    break;
                }
            }
        }
    }

    ObjectGuid const blockerGuid = blocker ? blocker->GetGUID() : ObjectGuid::Empty;
    ObjectGuid const nextGuid = next ? next->GetGUID() : ObjectGuid::Empty;
    bool const isNextNew = nextGuid != beamState.next;
    beamState.blocker = blockerGuid;
    beamState.next = nextGuid;
    if (blocker)
    {
        taken.push_back(blockerGuid);
        // Red ignores void zones: moving out of one could leave melee range and walk him off.
        float const distance = data.beam == NetherspiteBeam::Red ? NETHERSPITE_BLOCKER_DISTANCE :
            FindBeamSpotDistance(netherspite, *beamState.portal, state, blocker);
        beamState.blockerSpot = GetSpotOnBeam(netherspite, *beamState.portal, distance);
    }

    if (!next)
        return;

    taken.push_back(nextGuid);
    if (blocker && HasBeamStackLimit(blocker, data.beam) &&
        GetBeamStacks(blocker, data) >= NETHERSPITE_NEXT_STEPS_ON_AT_STACKS)
    {
        beamState.nextSpot = GetSpotOnBeam(
            netherspite, *beamState.portal,
            FindBeamSpotDistance(netherspite, *beamState.portal, state, next));
        return;
    }

    float const side = GetNetherspiteInnerSide(netherspite, state, data.beam);
    float const low = next->getClass() == CLASS_HUNTER ?
        NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_MIN_DISTANCE;
    if (isNextNew || !IsNextSpotGood(
            netherspite, *beamState.portal, state, side, beamState.nextSpot, low))
    {
        // If there's nowhere for it yet, it stays where it is.
        std::optional<Position> const spot =
            FindNextSpot(bot, netherspite, *beamState.portal, state, side, low);
        if (spot)
            beamState.nextSpot = *spot;
        else if (isNextNew)
            beamState.nextSpot = next->GetPosition();
    }
}

// A slot keeps its bearing and comes in along it until it's on the floor and clear of void zones.
Position GetBanishSpot(Player* bot, Unit const& netherspite, KaraInstanceState const& state,
    float angle, float startRadius)
{
    constexpr float minRadius = 6.0f;
    constexpr float radiusStep = 1.0f;
    float const bossX = netherspite.GetPositionX();
    float const bossY = netherspite.GetPositionY();
    float const bossZ = netherspite.GetPositionZ();

    float radius = startRadius;
    float x = bossX;
    float y = bossY;
    float z = bossZ;
    while (radius >= minRadius)
    {
        x = bossX + std::cos(angle) * radius;
        y = bossY + std::sin(angle) * radius;
        z = bossZ;
        if (bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bossX, bossY, bossZ, x, y, z, false) &&
            IsClearOfVoidZones(state, x, y, VOID_ZONE_CLEARANCE))
        {
            break;
        }

        radius = std::min(radius, netherspite.GetExactDist2d(x, y)) - radiusStep;
    }

    return Position(x, y, z);
}

float GetBanishRadius(Player* member)
{
    constexpr float radius = 20.0f;
    constexpr float hunterRadius = 25.0f;
    return member->getClass() == CLASS_HUNTER ? hunterRadius : radius;
}

// One breath hits one bot: the cone is 45 degrees and the slots are 360 / bots apart.
void UpdateBanishSlots(Player* bot, Unit const& netherspite, KaraInstanceState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == KARA_MAP_ID &&
            GET_PLAYERBOT_AI(member))
        {
            members.push_back(member);
        }
    }

    std::vector<NetherspiteBanishSlot>& slots = state.netherspiteBanishSlots;
    if (slots.empty())
    {
        if (members.empty())
            return;

        // Hunters first, for the slots with the most floor: they can't shoot inside 24.5 yd.
        std::stable_partition(members.begin(), members.end(), [](Player* member)
        {
            return member->getClass() == CLASS_HUNTER;
        });

        std::vector<float> angles;
        for (size_t i = 0; i < members.size(); ++i)
            angles.push_back(i * 2.0f * M_PI / members.size());

        for (Player* member : members)
        {
            float const radius = GetBanishRadius(member);
            bool const isHunter = member->getClass() == CLASS_HUNTER;
            auto best = angles.begin();
            float bestScore = std::numeric_limits<float>::lowest();
            for (auto angle = angles.begin(); angle != angles.end(); ++angle)
            {
                float score;
                if (isHunter)
                {
                    Position const spot =
                        GetBanishSpot(bot, netherspite, state, *angle, radius);
                    score = netherspite.GetExactDist2d(spot);
                }
                else
                {
                    float const gap = Position::NormalizeOrientation(
                        netherspite.GetAngle(member->GetPositionX(), member->GetPositionY()) -
                        *angle);
                    score = -std::min(gap, 2.0f * static_cast<float>(M_PI) - gap);
                }

                if (score > bestScore)
                {
                    bestScore = score;
                    best = angle;
                }
            }

            slots.push_back({
                member->GetGUID(), GetBanishSpot(bot, netherspite, state, *best, radius),
            });
            angles.erase(best);
        }

        return;
    }

    for (NetherspiteBanishSlot& slot : slots)
    {
        if (IsClearOfVoidZones(
                state, slot.spot.GetPositionX(), slot.spot.GetPositionY(), VOID_ZONE_CLEARANCE))
        {
            continue;
        }

        slot.spot = GetBanishSpot(
            bot, netherspite, state,
            netherspite.GetAngle(slot.spot.GetPositionX(), slot.spot.GetPositionY()),
            netherspite.GetExactDist2d(slot.spot));
    }

    // A bot rezzed since the slots were made takes the slot of one that has died.
    for (Player* member : members)
    {
        if (FindNetherspiteBanishSlot(state, member->GetGUID()))
            continue;

        for (NetherspiteBanishSlot& slot : slots)
        {
            Player* holder = ObjectAccessor::GetPlayer(*bot, slot.guid);
            if (!holder || !holder->IsAlive())
            {
                slot.guid = member->GetGUID();
                break;
            }
        }
    }
}

void UpdateVoidZones(Player* bot, KaraInstanceState& state)
{
    constexpr float searchRadius = 150.0f;
    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(creatures, Id(KaraNpcs::NPC_VOID_ZONE), searchRadius);

    state.netherspiteVoidZones.clear();
    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive())
            state.netherspiteVoidZones.push_back(creature->GetPosition());
    }
}

}

// Only the tracker writes the beam state. Everyone else reads it.
bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState& state = KaraState(netherspite->GetInstanceId());
    UpdateVoidZones(bot, state);

    if (IsBanishPhase(netherspite))
    {
        ResetIfSet(state.netherspiteDpsHoldTimer);
        state.netherspiteBeams = {};
        UpdateBanishSlots(bot, *netherspite, state);
        return false;
    }

    EmplaceIfUnset(state.netherspiteDpsHoldTimer, getMSTime());
    state.netherspiteBanishSlots.clear();

    constexpr float searchRadius = 150.0f;
    for (NetherspiteBeamData const& data : NETHERSPITE_BEAMS)
    {
        NetherspiteBeamState& beamState = state.netherspiteBeams[Id(data.beam)];
        if (beamState.portal)
            continue;

        if (Unit* portal = bot->FindNearestCreature(Id(data.portal), searchRadius))
            beamState.portal = portal->GetPosition();
    }

    if (!AreNetherspitePortalsKnown(state))
        return false;

    // A bot on one beam or waiting for it isn't free for another, fallback or not.
    std::vector<ObjectGuid> taken;
    for (NetherspiteBeamState const& beamState : state.netherspiteBeams)
    {
        taken.push_back(beamState.blocker);
        taken.push_back(beamState.next);
    }

    for (NetherspiteBeamData const& data : NETHERSPITE_BEAMS)
        UpdateBeamBlockers(bot, *netherspite, state, data, taken);

    return false;
}

// Prince Malchezaar
// CombatReach is 4 yards

bool PrinceMalchezaarEnfeebledBotAvoidHazardAction::Execute(Event /*event*/)
{
    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    constexpr uint8 numAngles = 64;
    constexpr float minSafeBossDistance = 32.0f;
    constexpr float distIncrement = 0.5f;
    constexpr uint8 numSteps = 56;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const bossX = malchezaar->GetPositionX();
    float const bossY = malchezaar->GetPositionY();
    float bestMoveDistSq = std::numeric_limits<float>::max();
    float bestDestX = 0.0f;
    float bestDestY = 0.0f;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float const angle = (2 * M_PI * i) / numAngles;
        float const dx = std::cos(angle);
        float const dy = std::sin(angle);

        for (uint8 j = 0; j <= numSteps; ++j)
        {
            float const dist = minSafeBossDistance + j * distIncrement;
            float destX = bossX + dx * dist;
            float destY = bossY + dy * dist;
            float destZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, botX, botY, destZ, destX, destY, destZ, true))
            {
                continue;
            }

            float ddx = destX - bossX;
            float ddy = destY - bossY;

            constexpr float minSafeBossDistanceSq = minSafeBossDistance * minSafeBossDistance;
            if (ddx*ddx + ddy*ddy < minSafeBossDistanceSq)
                continue;

            float mdx = destX - botX;
            float mdy = destY - botY;
            float moveDistSq = mdx*mdx + mdy*mdy;

            constexpr float safeInfernalDistance = 22.0f;
            if (IsStraightPathSafe(botX, botY, destX, destY, infernals, safeInfernalDistance) &&
                moveDistSq < bestMoveDistSq)
            {
                bestMoveDistSq = moveDistSq;
                bestDestX = destX;
                bestDestY = destY;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event /*event*/)
{
    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    constexpr float safeInfernalDistance = 22.0f;
    if (!bot->FindNearestCreature(
            Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), safeInfernalDistance, true))
    {
        return false;
    }

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    constexpr float maxDistanceFromBoss = 33.0f;

    float bestDestX = botX;
    float bestDestY = botY;

    bool found = TryFindSafePositionWithSafePath(
        bot, botX, botY, malchezaar->GetPositionX(), malchezaar->GetPositionY(),
        infernals, safeInfernalDistance, maxDistanceFromBoss, bestDestX, bestDestY);

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool PrinceMalchezaarTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    if (malchezaar->GetVictim() != bot || !bot->IsWithinMeleeRange(malchezaar))
        return false;

    constexpr float safeInfernalDistance = 30.0f;
    if (!bot->FindNearestCreature(
            Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), safeInfernalDistance, true))
    {
        return false;
    }

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    constexpr float maxSampleDist = 75.0f;
    float bestDestX = botX;
    float bestDestY = botY;

    bool found = TryFindSafePositionWithSafePath(
        bot, botX, botY, botX, botY, infernals, safeInfernalDistance,
        maxSampleDist, bestDestX, bestDestY);

    if (!found)
        return false;

    float const toPosX = bestDestX - botX;
    float const toPosY = bestDestY - botY;

    float const toBossX = malchezaar->GetPositionX() - botX;
    float const toBossY = malchezaar->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Nightbane
// CombatReach is 10.5 yards

// On the arc, he stands about 10y inside the tank and faces the parapet with the dome wall close
// behind him, so Smoldering Breath and Tail Sweep have little of the terrace to hit. Nothing else
// may move him: the tank sits in Charred Earth rather than turn him toward the raid.
bool NightbaneTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") != nightbane)
        return Attack(nightbane);

    if (nightbane->GetVictim() != bot || !bot->IsWithinMeleeRange(nightbane))
        return false;

    float const angle =
        std::clamp(GetTerraceAngle(*nightbane), TERRACE_ARC_START, TERRACE_ARC_END);
    Position const destination(
        TERRACE_DOME_CENTER.GetPositionX() + TERRACE_ARC_RADIUS * std::cos(angle),
        TERRACE_DOME_CENTER.GetPositionY() + TERRACE_ARC_RADIUS * std::sin(angle),
        bot->GetPositionZ());

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, destination, arrivalDist, nightbane, moveX, moveY, backwards))
        return false;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// Ranged bots will stack on one "ranged leader" that can be designated by the assistant flag
// The ranged leader will lead the ranged group out of Charred Earths
bool NightbaneRangedStackAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* rangedLeader = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != KARA_MAP_ID ||
            !member->IsAlive() || !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
        {
            rangedLeader = member;
            break;
        }

        if (!rangedLeader || member->GetGUID() < rangedLeader->GetGUID())
            rangedLeader = member;
    }

    if (!rangedLeader)
        return false;

    if (bot == rangedLeader)
        return MoveRangedLeaderToSafeSpot();

    return StackOnRangedLeader(rangedLeader);
}

bool NightbaneRangedStackAction::MoveRangedLeaderToSafeSpot()
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    constexpr float minBossDist = 15.0f;
    if (!bot->HasAura(Id(KaraSpells::SPELL_CHARRED_EARTH)) &&
        bot->GetExactDist2d(nightbane) >= minBossDist)
    {
        return false;
    }

    constexpr float searchRadius = 60.0f;
    std::vector<Position> const charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, Id(KaraSpells::SPELL_CHARRED_EARTH));

    constexpr float spotMinBossDist = minBossDist + 1.0f;
    constexpr float spotMaxBossDist = 35.0f;
    std::optional<Position> const spot = FindNightbaneStackSpot(
        GetTerraceAngle(*bot), TERRACE_ARC_START, TERRACE_ARC_END,
        [&](Position const& candidate)
        {
            float const bossDist = nightbane->GetExactDist2d(candidate);
            return bossDist >= spotMinBossDist && bossDist <= spotMaxBossDist &&
                IsClearOfCharredEarth(candidate, charredEarths);
        });

    if (!spot)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, spot->GetPositionX(), spot->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneRangedStackAction::StackOnRangedLeader(Player* rangedLeader)
{
    if (bot->GetExactDist2d(rangedLeader) < 0.5f)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, rangedLeader->GetPositionX(), rangedLeader->GetPositionY(),
        rangedLeader->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    if (IsNightbaneFlying(*nightbane))
    {
        if (!pet->HasReactState(REACT_PASSIVE))
        {
            _previousReactState = pet->GetReactState();
            _setPassive = true;
            CallBackPets(bot);
            pet->SetReactState(REACT_PASSIVE);
        }
    }
    else
        ResetPetStance();

    return false;
}

bool NightbaneControlPetAggressionAction::ResetPetStance()
{
    if (!_setPassive)
        return false;

    if (Pet* pet = bot->GetPet())
        pet->SetReactState(_previousReactState);

    _setPassive = false;
    return true;
}

bool NightbaneStopAttackingAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    return StopAttackingTarget(bot, *nightbane);
}

bool NightbaneSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool NightbaneSelectTargetForHoldAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane || IsNightbaneFlying(*nightbane))
        return false;

    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

// The shared spot is out of Rain of Bones and clear of Charred Earth (see the manage action).
// Stacked, the skeletons come to everyone, with reach off. Fireball Barrage needs only range.
bool NightbaneStackInFlightAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneFlightSpot)
        return false;

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, *state->nightbaneFlightSpot, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// He casts nothing during the landing, so bots drop to their non-combat engine, where stock follow
// lets the master reposition them. The target exclusion keeps him from being picked again.
bool NightbaneStandDownAction::Execute(Event /*event*/)
{
    StandDown(botAI);
    return true;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState& state = KaraState(nightbane->GetInstanceId());

    if (!IsNightbaneFlying(*nightbane))
    {
        bool didSomething = ResetIfSet(state.nightbaneFlightStage);
        didSomething |= ResetIfSet(state.nightbaneFlightSpot);
        didSomething |= ResetIfSet(state.nightbaneRainOfBonesCenter);
        didSomething |= EmplaceIfUnset(state.nightbaneDpsHoldTimer, getMSTime());
        return didSomething;
    }

    bool didSomething = ResetIfSet(state.nightbaneDpsHoldTimer);

    constexpr float atFlightPositionDist = 1.0f;
    NightbaneFlightStage stage = NightbaneFlightStage::TakeOff;
    if (nightbane->GetExactDist(NIGHTBANE_FLIGHT_POSITION) < atFlightPositionDist)
        stage = NightbaneFlightStage::Hover;
    else if (state.nightbaneFlightStage &&
             *state.nightbaneFlightStage != NightbaneFlightStage::TakeOff)
    {
        stage = NightbaneFlightStage::Landing;
    }

    if (state.nightbaneFlightStage != stage)
    {
        state.nightbaneFlightStage = stage;
        didSomething = true;
    }

    if (stage == NightbaneFlightStage::Landing)
    {
        didSomething |= ResetIfSet(state.nightbaneFlightSpot);
        didSomething |= ResetIfSet(state.nightbaneRainOfBonesCenter);
        return didSomething;
    }

    Position const* rainCenter = GetRainOfBonesCenter(*nightbane);
    bool const isNewRain = rainCenter && (!state.nightbaneRainOfBonesCenter ||
        state.nightbaneRainOfBonesCenter->GetExactDist2d(*rainCenter) > 0.1f);
    if (state.nightbaneFlightSpot && !isNewRain)
        return didSomething;

    if (isNewRain)
        state.nightbaneRainOfBonesCenter = *rainCenter;

    float const rainClearDist = RAIN_OF_BONES_REACH + RAIN_OF_BONES_CLEARANCE;
    if (state.nightbaneFlightSpot &&
        state.nightbaneFlightSpot->GetExactDist2d(*rainCenter) >= rainClearDist)
    {
        return true;
    }

    constexpr float searchRadius = 80.0f;
    std::vector<Position> const charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, Id(KaraSpells::SPELL_CHARRED_EARTH));

    float const startAngle = state.nightbaneFlightSpot ?
        GetTerraceAngle(*state.nightbaneFlightSpot) : NIGHTBANE_FLIGHT_SPOT_HOME_ANGLE;
    std::optional<Position> const spot = FindNightbaneStackSpot(
        startAngle, NIGHTBANE_FLIGHT_SPOT_MIN_ANGLE, NIGHTBANE_FLIGHT_SPOT_MAX_ANGLE,
        [&](Position const& candidate)
        {
            return IsClearOfCharredEarth(candidate, charredEarths) &&
                (!rainCenter || candidate.GetExactDist2d(*rainCenter) >= rainClearDist);
        });

    if (spot)
        state.nightbaneFlightSpot = spot;
    else if (!state.nightbaneFlightSpot)
    {
        state.nightbaneFlightSpot = Position(
            TERRACE_DOME_CENTER.GetPositionX() +
                NIGHTBANE_STACK_RADIUS * std::cos(NIGHTBANE_FLIGHT_SPOT_HOME_ANGLE),
            TERRACE_DOME_CENTER.GetPositionY() +
                NIGHTBANE_STACK_RADIUS * std::sin(NIGHTBANE_FLIGHT_SPOT_HOME_ANGLE),
            NIGHTBANE_GROUND_Z);
    }

    return true;
}
