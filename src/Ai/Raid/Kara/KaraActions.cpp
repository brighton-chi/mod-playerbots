/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "EncounterHelpers.h"
#include "KaraHelpers.h"
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

    Action* blueAction = context->GetAction("netherspite block blue beam");
    if (blueAction &&
        static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
    {
        reset = true;
    }

    Action* greenAction = context->GetAction("netherspite block green beam");
    if (greenAction &&
        static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
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
    constexpr float searchRadius = 150.0f;
    Unit* redPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
    if (!redPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != redBlocker)
    {
        _wasBlockingRedBeam = false;
        _redBeamTimerWasSet = false;
        return false;
    }

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
    }

    if (!_wasBlockingRedBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_red", "%player is moving to block the red beam!",
            placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingRedBeam = true;

    constexpr uint32 intervalMs = 5 * IN_MILLISECONDS;
    if (_redBeamTimerWasSet && GetMSTimeDiffToNow(_redBeamMoveTimer) >= intervalMs)
    {
        _lastBeamMoveSideways = !_lastBeamMoveSideways;
        _redBeamMoveTimer = getMSTime();
    }

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    constexpr float idealDistance = 18.0f;
    std::vector<Unit*> voidZones;
    Position beamPos;
    if (!FindBeamPosition(netherspite, redPortal, voidZones, idealDistance, beamPos))
        return false;

    if (!_lastBeamMoveSideways)
    {
        return MoveTo(
            KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    float const length = netherspite->GetExactDist2d(redPortal);
    if (length == 0.0f)
        return false;

    float const dx = (redPortal->GetPositionX() - netherspite->GetPositionX()) / length;
    float const dy = (redPortal->GetPositionY() - netherspite->GetPositionY()) / length;
    float const perpDx = -dy;
    float const perpDy = dx;
    float const sideX = beamPos.GetPositionX() + perpDx * 3.0f;
    float const sideY = beamPos.GetPositionY() + perpDy * 3.0f;

    return MoveTo(
        KARA_MAP_ID, sideX, sideY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Two mana-user DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
bool NetherspiteBlockBlueBeamAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 150.0f;
    Unit* bluePortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
    if (!bluePortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != blueBlocker)
    {
        if (_wasBlockingBlueBeam)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_leaving_blue",
                "%player is leaving the blue beam. Next blocker up!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingBlueBeam = false;
        return false;
    }

    if (!_wasBlockingBlueBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_blue",
            "%player is moving to block the blue beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingBlueBeam = true;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    float const idealDistance = bot->getClass() == CLASS_HUNTER ? 25.0f : 18.0f;
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (!FindBeamPosition(netherspite, bluePortal, voidZones, idealDistance, beamPos))
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one non-mana-user dps bot will block the green beam for an entire phase
bool NetherspiteBlockGreenBeamAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 150.0f;
    Unit* greenPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
    if (!greenPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != greenBlocker)
    {
        if (_wasBlockingGreenBeam)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_leaving_green",
                "%player is leaving the green beam. Next blocker up!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingGreenBeam = false;
        return false;
    }

    if (!_wasBlockingGreenBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_green",
            "%player is moving to block the green beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingGreenBeam = true;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    constexpr float idealDistance = 18.0f;
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (!FindBeamPosition(netherspite, greenPortal, voidZones, idealDistance, beamPos))
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(bot);

    constexpr float hazardRadius = 4.0f;
    bool const nearVoidZone = !IsSafePosition(
        bot->GetPositionX(), bot->GetPositionY(), voidZones, hazardRadius);

    constexpr float searchRadius = 150.0f;
    float const bossX = netherspite->GetPositionX();
    float const bossY = netherspite->GetPositionY();
    std::vector<BeamAvoid> beams;

    if (Unit* redPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(redPortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (redPortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (redPortal->GetPositionY() - bossY) / len : 0.0f});
    }

    if (Unit* bluePortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(bluePortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (bluePortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (bluePortal->GetPositionY() - bossY) / len : 0.0f});
    }

    if (Unit* greenPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(greenPortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (greenPortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (greenPortal->GetPositionY() - bossY) / len : 0.0f});
    }

    bool const nearBeam = !IsAwayFromBeams(
        bot->GetPositionX(), bot->GetPositionY(), bossX, bossY, beams);

    if (!nearVoidZone && !nearBeam)
        return false;

    constexpr uint8 numAngles = 36;
    constexpr float stepDist = 0.5f;
    constexpr uint8 numSteps = 56;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    Position bestCandidate;
    float bestDistSq = std::numeric_limits<float>::max();
    bool found = false;

    for (uint8 i = 0; i < numAngles; ++i)
    {
        float const angle = i * M_PI / 18.0f;
        for (uint8 j = 0; j <= numSteps; ++j)
        {
            float const dist = 2.0f + j * stepDist;
            float candidateX = botX + std::cos(angle) * dist;
            float candidateY = botY + std::sin(angle) * dist;

            if (!IsSafePosition(candidateX, candidateY, voidZones, hazardRadius) ||
                !IsAwayFromBeams(candidateX, candidateY, bossX, bossY, beams))
            {
                continue;
            }

            float dx = candidateX - botX;
            float dy = candidateY - botY;
            float moveDistSq = dx*dx + dy*dy;

            if (moveDistSq < bestDistSq)
            {
                bestCandidate = Position(candidateX, candidateY, bot->GetPositionZ());
                bestDistSq = moveDistSq;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
        bestCandidate.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool NetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(
     float x, float y, float botX, float botY, std::vector<BeamAvoid> const& beams)
{
    for (auto const& beam : beams)
    {
        if (beam.maxDist == 0.0f)
            continue;

        float const botdx = x - botX;
        float const botdy = y - botY;
        float const distanceAlongBeam = (botdx * beam.dirX + botdy * beam.dirY);
        float const beamX = botX + beam.dirX * distanceAlongBeam;
        float const beamY = botY + beam.dirY * distanceAlongBeam;
        float const distToBeamSq = (x - beamX) * (x - beamX) + (y - beamY) * (y - beamY);

        constexpr float minDistFromBeamSq = 25.0f;
        if (distToBeamSq < minDistFromBeamSq && distanceAlongBeam > beam.minDist &&
            distanceAlongBeam < beam.maxDist)
        {
            return false;
        }
    }

    return true;
}

bool NetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event /*event*/)
{
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    constexpr float safeDistance = 4.0f;
    constexpr uint32 minInterval = 0;

    for (Unit* voidZone : voidZones)
    {
        if (voidZone->GetEntry() == Id(KaraNpcs::NPC_VOID_ZONE) &&
            bot->GetExactDist2d(voidZone) < safeDistance)
        {
            return FleePosition(voidZone->GetPosition(), safeDistance, minInterval);
        }
    }

    return false;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    std::optional<uint32>& dpsWaitTimer =
        KaraState(netherspite->GetInstanceId()).netherspiteDpsWaitTimer;
    uint32 const now = getMSTime();
    bool const isMechanicTracker = IsMechanicTrackerBot(bot, KARA_MAP_ID);
    bool didSomething = false;

    if (IsBanishPhase(netherspite))
    {
        if (isMechanicTracker && ResetIfSet(dpsWaitTimer))
            didSomething = true;

        Action* redAction = context->GetAction("netherspite block red beam");
        if (redAction &&
            static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
        {
            didSomething = true;
        }

        Action* blueAction = context->GetAction("netherspite block blue beam");
        if (blueAction &&
            static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
        {
            didSomething = true;
        }

        Action* greenAction = context->GetAction("netherspite block green beam");
        if (greenAction &&
            static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
        {
            didSomething = true;
        }
    }
    else if (isMechanicTracker && EmplaceIfUnset(dpsWaitTimer, now))
    {
        didSomething = true;
    }

    return didSomething;
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
