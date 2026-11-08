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

    Action* areaAction = context->GetAction("netherspite move between beams");
    if (areaAction &&
        static_cast<NetherspiteMoveBetweenBeamsAction*>(areaAction)->ResetDestination())
    {
        reset = true;
    }

    Action* wolfAction = context->GetAction("big bad wolf red hood run away");
    if (wolfAction &&
        static_cast<BigBadWolfRedHoodRunAwayAction*>(wolfAction)->ResetRun())
    {
        reset = true;
    }

    Action* enfeebleAction = context->GetAction("prince malchezaar enfeebled bot avoid hazard");
    if (enfeebleAction &&
        static_cast<PrinceMalchezaarEnfeebledBotAvoidHazardAction*>(enfeebleAction)
            ->ResetDestination())
    {
        reset = true;
    }

    Action* hazardAction = context->GetAction("prince malchezaar move clear of hazards");
    if (hazardAction &&
        static_cast<PrinceMalchezaarMoveClearOfHazardsAction*>(hazardAction)->ResetDestination())
    {
        reset = true;
    }

    Action* tankAction = context->GetAction("prince malchezaar tank position boss");
    if (tankAction &&
        static_cast<PrinceMalchezaarTankPositionBossAction*>(tankAction)->ResetWalk())
    {
        reset = true;
    }

    Action* netherspitePetAction = context->GetAction("netherspite control pet aggression");
    if (netherspitePetAction &&
        static_cast<NetherspiteControlPetAggressionAction*>(netherspitePetAction)->ResetPetStance())
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

// Mounted Attumen's CombatReach is 2.25 yards
bool AttumenTheHuntsmanStackBehindAction::Execute(Event /*event*/)
{
    Unit* attumen = GetMountedAttumen(bot);
    if (!attumen)
        return false;

    std::optional<Position> const position = GetAttumenStackPosition(bot, *attumen);
    if (!position)
        return false;

    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(),
        position->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED,
        true, false);
}

bool AttumenTheHuntsmanStopAttackingAction::Execute(Event /*event*/)
{
    return StopAttackingAndChanneling(bot);
}

bool AttumenTheHuntsmanSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

// Midnight turning invisible makes her an invalid target, so every bot on her drops to its
// non-combat engine at the merge.
bool AttumenTheHuntsmanSelectTargetForHoldAction::Execute(Event /*event*/)
{
    return SelectTarget(
        botAI, AI_VALUE(Unit*, PlayerbotAI::IsTank(bot) ? "tank target" : "dps target"));
}

bool AttumenTheHuntsmanManageDpsTimerAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();
    return EmplaceIfUnset(KaraState(bot->GetInstanceId()).attumenDpsHoldTimer, now);
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

    // The tank leads her along the line from her through her position, so it walks through her when
    // she is between it and the position.
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
    // A bot only stands on its own position, so the group isn't searched once it arrives.
    constexpr float arrivalDist = 2.0f;
    for (Position const& position : MAIDEN_OF_VIRTUE_RANGED_POSITIONS)
    {
        if (bot->GetExactDist2d(position) <= arrivalDist)
            return false;
    }

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

bool BigBadWolfTankPositionBossAction::Execute(Event /*event*/)
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

    // Feared or stunned: the first step after it replans from where both are then.
    if (!botAI->CanMove())
        return false;

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

bool BigBadWolfRedHoodUseEscapeAction::Execute(Event /*event*/)
{
    // He drops a feigning hunter despite the taunt, and comes back if it stands up during Hood.
    // A hunter whose Feign Death is on cooldown gets the boots below.
    if (bot->getClass() == CLASS_HUNTER &&
        !bot->HasSpellCooldown(Id(KaraSpells::SPELL_FEIGN_DEATH)))
    {
        return botAI->CanCastSpell("feign death", bot) && botAI->CastSpell("feign death", bot);
    }

    // A cheat, for a bugged mechanic: with AC's reach (4.5) and speed (8.5 yd/s) he catches any
    // runner at Hood's +50%. The largest speed aura wins, so the boots' +70% replaces Hood's, and
    // like Hood it lasts 20 s, so it is never removed.
    return bot->AddAura(Id(KaraSpells::SPELL_GOBLIN_ROCKET_BOOTS), bot);
}

bool BigBadWolfProtectRedHoodAction::Execute(Event /*event*/)
{
    Unit* wolf = GetOperaActor(bot, "the big bad wolf");
    if (!wolf)
        return false;

    switch (bot->getClass())
    {
        // Human targets only, once he can swing at them, near the end of Hood or below half health.
        case CLASS_PALADIN:
        {
            return CastOnTarget(
                "hand of protection", GetBigBadWolfTargetToProtect(*wolf),
                HAND_OF_PROTECTION_RANGE);
        }
        // Weakened Soul fails CanCastSpell, so only one priest shields at a time.
        case CLASS_PRIEST:
        {
            return CastOnTarget(
                "power word: shield", GetBigBadWolfRedHood(*wolf), POWER_WORD_SHIELD_RANGE);
        }
        // The next swing at the target hits the warrior instead. Defensive Stance only, which
        // CanCastSpell tests.
        case CLASS_WARRIOR:
        {
            Player* target = GetBigBadWolfRedHood(*wolf);
            return target && !target->HasAura(Id(KaraSpells::SPELL_INTERVENE)) &&
                !bot->IsWithinCombatRange(target, INTERVENE_MIN_RANGE) &&
                CastOnTarget("intervene", target, INTERVENE_MAX_RANGE);
        }
        default:
            return false;
    }
}

// Range edge to edge, as spell range: CanCastSpell passes targets out of range, and a failed cast
// still turns the bot.
bool BigBadWolfProtectRedHoodAction::CastOnTarget(char const* spell, Player* target, float range)
{
    return target && target != bot && bot->IsWithinCombatRange(target, range) &&
        botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target);
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

bool TheCuratorTankPositionBossAction::Execute(Event /*event*/)
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
    // Neither is on a threat list for sure: chains never attack, and a resummoned Kil'rek isn't
    // zoned in. So it is a search, but only for what can be up: chains during a Sacrifice, and
    // Kil'rek while Terestian has no Broken Pact.
    Unit* terestian = GetKaraBoss(bot, KaraBosses::DATA_TERESTIAN);
    std::vector<uint32> entries;
    if (IsTerestianSacrificeActive(bot))
        entries.push_back(Id(KaraNpcs::NPC_DEMON_CHAINS));

    if (!terestian || !terestian->HasAura(Id(KaraSpells::SPELL_BROKEN_PACT)))
        entries.push_back(Id(KaraNpcs::NPC_KILREK));

    if (!entries.empty())
    {
        // It also returns corpses.
        constexpr float searchRadius = 100.0f;
        std::list<Creature*> adds;
        bot->GetCreatureListWithEntryInGrid(adds, entries, searchRadius);

        Creature* kilrek = nullptr;
        for (Creature* add : adds)
        {
            if (!add->IsAlive())
                continue;

            if (add->GetEntry() == Id(KaraNpcs::NPC_DEMON_CHAINS))
                return MarkTargetWithSkull(bot, add);

            kilrek = add;
        }

        if (kilrek)
            return MarkTargetWithSkull(bot, kilrek);
    }

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
    if (bot->GetExactDist2d(aran) > ARAN_PULLED_ONTO_BOSS_DISTANCE)
        return MoveAway(aran, ARAN_ARCANE_EXPLOSION_CLEARANCE - distance);

    uint32 const direction = bot->GetGUID().GetCounter() % ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS;
    float const angle =
        2.0f * static_cast<float>(M_PI) * direction / ARAN_ARCANE_EXPLOSION_EXIT_DIRECTIONS;
    float x = aran->GetPositionX() + ARAN_ARCANE_EXPLOSION_CLEARANCE * std::cos(angle);
    float y = aran->GetPositionY() + ARAN_ARCANE_EXPLOSION_CLEARANCE * std::sin(angle);
    float z = bot->GetPositionZ();
    bool const exact = bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z);

    return MoveTo(
        KARA_MAP_ID, x, y, z, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false);
}

// Ranged only: a patch's reach stops short of melee range on Aran.
bool ShadeOfAranRunOutOfBlizzardAction::Execute(Event /*event*/)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
        return false;

    std::optional<Position> const position = GetBlizzardEscapePosition(bot);
    if (!position)
        return false;

    // A filling cast would hold the bot still.
    bot->CastStop();

    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(),
        position->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED,
        true, false);
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
    GuidVector const elementals =
        context->GetValue<GuidVector>("shade of aran unbanished elementals")->Get();
    for (ObjectGuid const& guid : elementals)
    {
        Unit* elemental = botAI->GetUnit(guid);
        if (elemental && elemental->IsAlive())
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
    Position const position(
        aran->GetPositionX() + goal * std::cos(angle),
        aran->GetPositionY() + goal * std::sin(angle), bot->GetPositionZ());

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, ARAN_RANGED_BAND_TOLERANCE, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    // Wait out a Blizzard rather than walk back into it.
    float const patchReach = ARAN_BLIZZARD_HAZARD_RADIUS + bot->GetCombatReach();
    for (Position const& patch : GetDynamicObjectPositions(
             bot, ARAN_ROOM_SEARCH_RADIUS, Id(KaraSpells::SPELL_BLIZZARD)))
    {
        if (patch.GetExactDist2d(position) <= patchReach)
            return false;
    }

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

    float x = beam.blockerPosition.GetPositionX();
    float y = beam.blockerPosition.GetPositionY();
    if (_lastBeamMoveSideways)
    {
        constexpr float sideDistance = 3.0f;
        float const length = netherspite->GetExactDist2d(*beam.portal);
        if (length == 0.0f)
            return false;

        x -= (beam.portal->GetPositionY() - netherspite->GetPositionY()) / length * sideDistance;
        y += (beam.portal->GetPositionX() - netherspite->GetPositionX()) / length * sideDistance;
    }

    if (bot->GetExactDist2d(x, y) <= NETHERSPITE_BEAM_POSITION_TOLERANCE)
        return false;

    return MoveTo(
        KARA_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteBlockGreenOrBlueBeamAction::Execute(Event /*event*/)
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* position =
        state ? FindNetherspiteBlockerPosition(*state, bot->GetGUID()) : nullptr;
    if (!position)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteWaitToBlockBeamAction::Execute(Event /*event*/)
{
    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    Position const* position =
        state ? FindNetherspiteNextPosition(*state, bot->GetGUID()) : nullptr;
    if (!position)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteMoveBetweenBeamsAction::Execute(Event /*event*/)
{
    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    KaraInstanceState const* state = FindKaraState(netherspite->GetInstanceId());
    if (!state || !AreNetherspitePortalsKnown(*state))
        return false;

    if (!_destination || !IsBetweenNetherspiteBeams(
            bot, *netherspite, *state, _destination->GetPositionX(), _destination->GetPositionY()))
    {
        _destination = FindPositionBetweenNetherspiteBeams(bot, *netherspite, *state);
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
        KARA_MAP_ID, slot->position.GetPositionX(), slot->position.GetPositionY(),
        bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
        false);
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

// Pets on him through the banish keep hitting after its threat reset, ahead of the tank.
bool NetherspiteControlPetAggressionAction::Execute(Event /*event*/)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    Unit* netherspite = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
    if (!netherspite)
        return false;

    if (IsNetherspiteDpsHeld(bot, *netherspite))
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

bool NetherspiteControlPetAggressionAction::ResetPetStance()
{
    if (!_setPassive)
        return false;

    if (Pet* pet = bot->GetPet())
        pet->SetReactState(_previousReactState);

    _setPassive = false;
    return true;
}

namespace
{

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

// The blocker's position on the line: the clear point nearest where it wants to stand.
float FindBeamPositionDistance(
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
        if (gap < bestGap && IsClearOfVoidZones(state, x, y, VOID_ZONE_BEAM_POSITION_CLEARANCE))
        {
            best = distance;
            bestGap = gap;
        }
    }

    return best;
}

Position GetPositionOnBeam(Unit const& netherspite, Position const& portal, float distance)
{
    float const length = netherspite.GetExactDist2d(portal);
    return Position(
        netherspite.GetPositionX() +
            (portal.GetPositionX() - netherspite.GetPositionX()) / length * distance,
        netherspite.GetPositionY() +
            (portal.GetPositionY() - netherspite.GetPositionY()) / length * distance,
        netherspite.GetPositionZ());
}

bool IsNextPositionGood(
    Unit const& netherspite, Position const& portal, KaraInstanceState const& state, float side,
    Position const& position, float low)
{
    NetherspiteBeamOffset const offset = GetNetherspiteBeamOffset(
        netherspite, portal, position.GetPositionX(), position.GetPositionY());
    float const off = offset.off * side;
    return off >= NETHERSPITE_NEXT_MIN_OFF_LINE - NETHERSPITE_BEAM_POSITION_TOLERANCE &&
        off <= NETHERSPITE_NEXT_MAX_OFF_LINE + NETHERSPITE_BEAM_POSITION_TOLERANCE &&
        offset.along >= low - NETHERSPITE_BEAM_POSITION_TOLERANCE &&
        offset.along <= low + NETHERSPITE_NEXT_ALONG_RANGE + NETHERSPITE_BEAM_POSITION_TOLERANCE &&
        IsClearOfVoidZones(
            state, position.GetPositionX(), position.GetPositionY(), VOID_ZONE_CLEARANCE);
}

// Off the line on the side facing the other beam, so a move out of a void zone can't cross it.
std::optional<Position> FindNextPosition(
    Player* bot, Unit const& netherspite, Position const& portal,
    KaraInstanceState const& state, float side, float low)
{
    float const length = netherspite.GetExactDist2d(portal);
    float const dirX = (portal.GetPositionX() - netherspite.GetPositionX()) / length;
    float const dirY = (portal.GetPositionY() - netherspite.GetPositionY()) / length;
    for (uint8 i = 0; i < NETHERSPITE_POSITION_TRIES; ++i)
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
            FindBeamPositionDistance(netherspite, *beamState.portal, state, blocker);
        beamState.blockerPosition = GetPositionOnBeam(netherspite, *beamState.portal, distance);
    }

    if (!next)
        return;

    taken.push_back(nextGuid);
    if (blocker && HasBeamStackLimit(blocker, data.beam) &&
        GetBeamStacks(blocker, data) >= NETHERSPITE_NEXT_STEPS_ON_AT_STACKS)
    {
        beamState.nextPosition = GetPositionOnBeam(
            netherspite, *beamState.portal,
            FindBeamPositionDistance(netherspite, *beamState.portal, state, next));
        return;
    }

    float const side = GetNetherspiteInnerSide(netherspite, state, data.beam);
    float const low = next->getClass() == CLASS_HUNTER ?
        NETHERSPITE_HUNTER_BLOCKER_DISTANCE : NETHERSPITE_BLOCKER_MIN_DISTANCE;
    if (isNextNew || !IsNextPositionGood(
            netherspite, *beamState.portal, state, side, beamState.nextPosition, low))
    {
        // If there's nowhere for it yet, it stays where it is.
        std::optional<Position> const position =
            FindNextPosition(bot, netherspite, *beamState.portal, state, side, low);
        if (position)
            beamState.nextPosition = *position;
        else if (isNextNew)
            beamState.nextPosition = next->GetPosition();
    }
}

// A slot keeps its bearing and comes in along it until it's on the floor and clear of void zones.
Position GetBanishPosition(Player* bot, Unit const& netherspite, KaraInstanceState const& state,
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
                    Position const position =
                        GetBanishPosition(bot, netherspite, state, *angle, radius);
                    score = netherspite.GetExactDist2d(position);
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
                member->GetGUID(), GetBanishPosition(bot, netherspite, state, *best, radius),
            });
            angles.erase(best);
        }

        return;
    }

    for (NetherspiteBanishSlot& slot : slots)
    {
        if (IsClearOfVoidZones(
                state, slot.position.GetPositionX(), slot.position.GetPositionY(),
                VOID_ZONE_CLEARANCE))
        {
            continue;
        }

        slot.position = GetBanishPosition(
            bot, netherspite, state,
            netherspite.GetAngle(slot.position.GetPositionX(), slot.position.GetPositionY()),
            netherspite.GetExactDist2d(slot.position));
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

// One tick kills at 1 health, so this run never crosses a landed infernal. Shadow Nova lands 7.5 s
// after Enfeeble, which is about 52 yd of walking.
bool PrinceMalchezaarEnfeebledBotAvoidHazardAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (!IsMovingAllowed())
        return false;

    // Taking the tick keeps a new cast from starting between steps.
    if (IsWaitingForLastMove(priority))
        return true;

    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (_destination &&
        (!IsClearOfInfernals(*_destination, infernals, HELLFIRE_CLEARANCE) ||
         malchezaar->GetExactDist2d(*_destination) < SHADOW_NOVA_MOVE_DISTANCE ||
         !IsLineClearOfInfernals(bot->GetPosition(), *_destination, infernals)))
    {
        _destination.reset();
    }

    if (!_destination)
    {
        constexpr float searchDistance = 52.0f;
        _destination = FindMalchezaarPosition(
            bot, bot->GetPosition(), infernals, searchDistance, false,
            [&](Position const& candidate)
            {
                return IsClearOfInfernals(candidate, infernals, HELLFIRE_CLEARANCE) &&
                    malchezaar->GetExactDist2d(candidate) >= SHADOW_NOVA_CLEARANCE;
            });

        if (!_destination)
            return false;
    }

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, *_destination, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

// Stock reach walks at the target and this takes over at a landed infernal. It holds its position
// until it gets there, or reach would walk the bot straight back in. The position is in range, so
// reach has nothing left to do on arrival.
bool PrinceMalchezaarMoveClearOfHazardsAction::Execute(Event /*event*/)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (!IsMovingAllowed())
        return false;

    // Taking the tick keeps a new cast from starting between steps.
    if (IsWaitingForLastMove(priority))
        return true;

    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (_destination && !IsClearOfMalchezaarHazards(bot, *malchezaar, infernals, *_destination))
        _destination.reset();

    if (!_destination)
    {
        _destination = FindMalchezaarClearPosition(botAI, *malchezaar, infernals);
        if (!_destination)
            return false;
    }

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, *_destination, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false, priority, true,
        backwards);
}

bool PrinceMalchezaarTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* malchezaar = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    if (malchezaar->GetVictim() != bot || !bot->IsWithinMeleeRange(malchezaar))
        return false;

    std::vector<Position> const infernals = GetLandedInfernals(botAI);
    if (IsClearOfInfernals(*malchezaar, infernals, MALCHEZAAR_RING_CLEARANCE))
    {
        ResetWalk();
        return false;
    }

    if (_destination && !IsClearOfInfernals(*_destination, infernals, _destinationClearance))
        _destination.reset();

    if (!_destination)
    {
        if (_searchedInfernalCount == infernals.size())
            return false;

        _searchedInfernalCount = infernals.size();
        _destination = FindMalchezaarTankPosition(bot, infernals, _destinationClearance);
        if (!_destination)
            return false;
    }

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, *_destination, arrivalDist, malchezaar, moveX, moveY, backwards))
    {
        _destination.reset();
        return false;
    }

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, backwards);
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
bool NightbaneRangedStackAndMoveFromCharredEarthAction::Execute(Event /*event*/)
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
        return MoveRangedLeaderToSafePosition();

    return StackOnRangedLeader(rangedLeader);
}

bool NightbaneRangedStackAndMoveFromCharredEarthAction::MoveRangedLeaderToSafePosition()
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

    constexpr float positionMinBossDist = minBossDist + 1.0f;
    constexpr float positionMaxBossDist = 35.0f;
    std::optional<Position> const position = FindNightbaneStackPosition(
        GetTerraceAngle(*bot), TERRACE_ARC_START, TERRACE_ARC_END,
        [&](Position const& candidate)
        {
            float const bossDist = nightbane->GetExactDist2d(candidate);
            return bossDist >= positionMinBossDist && bossDist <= positionMaxBossDist &&
                IsClearOfCharredEarth(candidate, charredEarths);
        });

    if (!position)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, position->GetPositionX(), position->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneRangedStackAndMoveFromCharredEarthAction::StackOnRangedLeader(Player* rangedLeader)
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

    if (IsNightbaneDpsHeld(bot, *nightbane))
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

// The shared position is out of Rain of Bones and clear of Charred Earth (see the manage action).
// Stacked, the skeletons come to everyone, with reach off. Fireball Barrage needs only range.
bool NightbaneStackAwayFromRainOfBonesAction::Execute(Event /*event*/)
{
    Unit* nightbane = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return false;

    KaraInstanceState const* state = FindKaraState(nightbane->GetInstanceId());
    if (!state || !state->nightbaneFlightStackPosition)
        return false;

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, *state->nightbaneFlightStackPosition, arrivalDist, nullptr, moveX, moveY,
            backwards))
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
bool NightbaneLeaveCombatForLandingAction::Execute(Event /*event*/)
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
        didSomething |= ResetIfSet(state.nightbaneFlightStackPosition);
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
        didSomething |= ResetIfSet(state.nightbaneFlightStackPosition);
        didSomething |= ResetIfSet(state.nightbaneRainOfBonesCenter);
        return didSomething;
    }

    Position const* rainCenter = GetRainOfBonesCenter(*nightbane);
    bool const isNewRain = rainCenter && (!state.nightbaneRainOfBonesCenter ||
        state.nightbaneRainOfBonesCenter->GetExactDist2d(*rainCenter) > 0.1f);
    if (state.nightbaneFlightStackPosition && !isNewRain)
        return didSomething;

    if (isNewRain)
        state.nightbaneRainOfBonesCenter = *rainCenter;

    float const rainClearDist = RAIN_OF_BONES_REACH + RAIN_OF_BONES_CLEARANCE;
    if (state.nightbaneFlightStackPosition &&
        state.nightbaneFlightStackPosition->GetExactDist2d(*rainCenter) >= rainClearDist)
    {
        return true;
    }

    constexpr float searchRadius = 80.0f;
    std::vector<Position> const charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, Id(KaraSpells::SPELL_CHARRED_EARTH));

    float const startAngle = state.nightbaneFlightStackPosition ?
        GetTerraceAngle(*state.nightbaneFlightStackPosition) : NIGHTBANE_FLIGHT_STACK_HOME_ANGLE;
    std::optional<Position> const position = FindNightbaneStackPosition(
        startAngle, NIGHTBANE_FLIGHT_STACK_MIN_ANGLE, NIGHTBANE_FLIGHT_STACK_MAX_ANGLE,
        [&](Position const& candidate)
        {
            return IsClearOfCharredEarth(candidate, charredEarths) &&
                (!rainCenter || candidate.GetExactDist2d(*rainCenter) >= rainClearDist);
        });

    if (position)
        state.nightbaneFlightStackPosition = position;
    else if (!state.nightbaneFlightStackPosition)
    {
        state.nightbaneFlightStackPosition = Position(
            TERRACE_DOME_CENTER.GetPositionX() +
                NIGHTBANE_STACK_RADIUS * std::cos(NIGHTBANE_FLIGHT_STACK_HOME_ANGLE),
            TERRACE_DOME_CENTER.GetPositionY() +
                NIGHTBANE_STACK_RADIUS * std::sin(NIGHTBANE_FLIGHT_STACK_HOME_ANGLE),
            NIGHTBANE_GROUND_Z);
    }

    return true;
}
