/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include <array>

using namespace KarazhanHelpers;

// General

bool KarazhanEraseEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    bool const isMechanicTracker = IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID);
    bool erased = false;

    if (isMechanicTracker)
    {
        if (!AI_VALUE2(Unit*, "find target", "midnight") &&
            attumenDpsWaitTimer.erase(instanceId) > 0)
        {
            erased = true;
        }

        if (!AI_VALUE2(Unit*, "find target", "nightbane"))
        {
            if (nightbaneDpsWaitTimer.erase(instanceId) > 0)
                erased = true;
            if (nightbaneFlightPhaseStartTimer.erase(instanceId) > 0)
                erased = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "the big bad wolf"))
    {
        Action* wolfAction = botAI->GetAiObjectContext()->GetAction(
            "big bad wolf little red riding hood run away");
        if (wolfAction &&
            static_cast<BigBadWolfLittleRedRidingHoodRunAwayAction*>(wolfAction)->ResetRunIndex())
        {
            erased = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "netherspite"))
    {
        if (isMechanicTracker && netherspiteDpsWaitTimer.erase(instanceId) > 0)
            erased = true;

        Action* redAction = botAI->GetAiObjectContext()->GetAction("netherspite block red beam");
        if (redAction &&
            static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
        {
            erased = true;
        }

        Action* blueAction = botAI->GetAiObjectContext()->GetAction("netherspite block blue beam");
        if (blueAction &&
            static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
        {
            erased = true;
        }

        Action* greenAction = botAI->GetAiObjectContext()->GetAction("netherspite block green beam");
        if (greenAction &&
            static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
        {
            erased = true;
        }
    }

    return erased;
}

bool KarazhanCastFearProtectionSpellAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PRIEST)
        return CastFearWardOnMainTank();
    else
        return SetTremorTotem();
}

bool KarazhanCastFearProtectionSpellAction::CastFearWardOnMainTank()
{
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !mainTank->IsAlive() ||
        mainTank->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_FEAR_WARD)))
    {
        return false;
    }

    return botAI->CanCastSpell("fear ward", mainTank) &&
        botAI->CastSpell("fear ward", mainTank);
}

bool KarazhanCastFearProtectionSpellAction::SetTremorTotem()
{
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return false;

    return botAI->CanCastSpell(static_cast<uint32>(KarazhanSpells::SPELL_TREMOR_TOTEM), bot) &&
        botAI->CastSpell(static_cast<uint32>(KarazhanSpells::SPELL_TREMOR_TOTEM), bot);
}

// Trash

bool ManaWarpStunCreatureBeforeWarpBreachAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> manaWarps;
    bot->GetCreatureListWithEntryInGrid(
        manaWarps, static_cast<uint32>(KarazhanNpcs::NPC_MANA_WARP), searchRadius);

    for (Creature* manaWarp : manaWarps)
    {
        if (!manaWarp || !manaWarp->IsAlive() || manaWarp->GetHealthPct() > 15.0f)
            continue;

        if (!target || manaWarp->GetGUID() < target->GetGUID())
            target = manaWarp;
    }

    if (!target)
        return false;

    static const std::array<const char*, 7> spells =
    {
        "bash",
        "concussion blow",
        "hammer of justice",
        "kidney shot",
        "maim",
        "shadowfury",
        "shockwave",
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target))
            return true;
    }

    return false;
}

// Attumen the Huntsman

// Midnight's CombatReach is 1.6 yards
// Unmounted Attumen's CombatReach is 1.5 yards
bool AttumenTheHuntsmanHandlePhaseOneAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    if (botAI->IsAssistTank(bot))
    {
        Unit* attumen = GetAttumenMounted(bot);
        return attumen && AssistTankMoveAttumenFromGroup(midnight, attumen);
    }

    if (AI_VALUE(Unit*, "current target") != midnight)
        return Attack(midnight);

    return false;
}

bool AttumenTheHuntsmanHandlePhaseOneAction::AssistTankMoveAttumenFromGroup(
    Unit* midnight, Unit* attumen)
{
    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() == bot && midnight->GetVictim() != bot)
    {
        constexpr float safeDistance = 8.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (nearestPlayer && attumen->GetDistance2d(nearestPlayer) < safeDistance)
            return MoveFromGroup(safeDistance);
    }

    return false;
}

bool AttumenTheHuntsmanHandlePhaseTwoAction::Execute(Event /*event*/)
{
    constexpr uint32 searchRadius = 40.0f;
    Unit* attumen = GetAttumenMounted(bot);
    if (!attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if ((botAI->IsTank(bot) && attumen->GetVictim() == bot) || botAI->IsMainTank(bot))
        return CurrentTankPositionAttumen(attumen);

    return StackBehindAttumen(attumen);
}

bool AttumenTheHuntsmanHandlePhaseTwoAction::CurrentTankPositionAttumen(Unit* attumen)
{
    if (attumen->GetVictim() != bot)
        return false;

    Position const tankPosition = { -11123.762f, -1926.619f, 49.215f };
    float const distanceToPosition = bot->GetExactDist2d(tankPosition);

    if (distanceToPosition < 2.0f || !bot->IsWithinLOS(
            tankPosition.GetPositionX(), tankPosition.GetPositionY(), tankPosition.GetPositionZ()))
    {
        return false;
    }

    float const dX = tankPosition.GetPositionX() - bot->GetPositionX();
    float const dY = tankPosition.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

    return MoveTo(
        KARAZHAN_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

// Mounted Attumen's CombatReach is 0 yards
bool AttumenTheHuntsmanHandlePhaseTwoAction::StackBehindAttumen(Unit* attumen)
{
    float const distanceBehind = bot->getClass() == CLASS_HUNTER? 8.0f : 2.0f;
    float const orientation = attumen->GetOrientation() + M_PI;
    float const rearX = attumen->GetPositionX() + std::cos(orientation) * distanceBehind;
    float const rearY = attumen->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (bot->GetDistance2d(rearX, rearY) < 0.2f)
        return false;

    return MoveTo(
        KARAZHAN_MAP_ID, rearX, rearY, attumen->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AttumenTheHuntsmanSetDpsTimerAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    if (attumenDpsWaitTimer.try_emplace(instanceId, now).second)
        return true;

    return false;
}

// Moroes

bool MoroesMainTankAttackBossAction::Execute(Event /*event*/)
{
    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    if (!moroes || AI_VALUE(Unit*, "current target") == moroes)
        return false;

    return Attack(moroes);
}

bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    static const std::array<const char*, 6> moroesGuests =
    {
        "baroness dorothea millstipe",
        "lady catriona von'indi",
        "lady keira berrybuck",
        "baron rafe dreuger",
        "lord robin daris",
        "lord crispin ference",
    };

    for (const char* name : moroesGuests)
    {
        if (Unit* guest = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, guest);
    }

    return false;
}

// Maiden of Virtue
// CombatReach is 4.8 yards

bool MaidenOfVirtueTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    if (AI_VALUE(Unit*, "current target") != maiden)
        return Attack(maiden);

    if (maiden->GetVictim() != bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* healer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !botAI->IsHeal(member) ||
            !member->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_REPENTANCE)))
        {
            continue;
        }

        healer = member;
        break;
    }

    if (healer)
        return MoveBossToStunnedHealer(healer);

    Position const tankPosition = { -10945.881f, -2103.782f, 92.712f };
    float distanceToPosition = bot->GetExactDist2d(tankPosition);
    if (distanceToPosition < 2.0f)
        return false;

    float const dX = tankPosition.GetPositionX() - bot->GetPositionX();
    float const dY = tankPosition.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

    return MoveTo(
        KARAZHAN_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool MaidenOfVirtueTankPositionBossAction::MoveBossToStunnedHealer(Unit* healer)
{
    constexpr float endDistance = 6.0;
    float const angle = healer->GetOrientation();
    float const targetX = healer->GetPositionX() + std::cos(angle) * endDistance;
    float const targetY = healer->GetPositionY() + std::sin(angle) * endDistance;

    return MoveTo(
        KARAZHAN_MAP_ID, targetX, targetY, healer->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtuePositionRangedBetweenPillarsAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static const Position rangedPositions[8] =
    {
        { -10931.178f, -2116.580f, 92.179f },
        { -10925.828f, -2102.425f, 92.180f },
        { -10933.089f, -2088.502f, 92.180f },
        { -10947.590f, -2082.815f, 92.180f },
        { -10960.912f, -2090.437f, 92.179f },
        { -10966.017f, -2105.288f, 92.175f },
        { -10959.242f, -2119.617f, 92.180f },
        { -10944.495f, -2123.857f, 92.180f },
    };

    constexpr uint8 maxIndex = 7;
    uint8 index = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
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

    Position const position = rangedPositions[index];
    if (bot->GetExactDist2d(position) < 2.0f)
        return false;

    return MoveTo(
        KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtueSetGroundingTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(static_cast<uint32>(KarazhanSpells::SPELL_GROUNDING_TOTEM), bot) &&
        botAI->CastSpell(static_cast<uint32>(KarazhanSpells::SPELL_GROUNDING_TOTEM), bot);
}

// The Big Bad Wolf
// CombatReach is 0 yards

bool BigBadWolfPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    if (AI_VALUE(Unit*, "current target") != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() != bot)
        return false;

    Position const tankPosition = { -10913.391f, -1773.508f, 90.477f };
    float const distanceToPosition = bot->GetExactDist2d(tankPosition);

    if (distanceToPosition < 2.0f)
        return false;

    float const dX = tankPosition.GetPositionX() - bot->GetPositionX();
    float const dY = tankPosition.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

    return MoveTo(
        KARAZHAN_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

// Run away, little girl, run away
bool BigBadWolfLittleRedRidingHoodRunAwayAction::Execute(Event /*event*/)
{
    static const Position runPositions[4] =
    {
        { -10875.456f, -1779.036f, 90.477f },
        { -10872.281f, -1751.638f, 90.477f },
        { -10910.492f, -1747.401f, 90.477f },
        { -10913.391f, -1773.508f, 90.477f },
    };

    Position const& target = runPositions[_runIndex];

    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < 1.0f)
        _runIndex = (_runIndex + 1) % 4;

    Position const position = runPositions[_runIndex];

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Romulo and Julianne

bool RomuloAndJulianneMarkTargetAction::Execute(Event /*event*/)
{
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!romulo)
        return false;

    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!julianne)
        return false;

    Unit* target = nullptr;
    constexpr float maxPctDifference = 10.0f;

    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() ||
        julianne->GetHealthPct() < 1.0f)
    {
        target = romulo;
    }
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() ||
        romulo->GetHealthPct() < 1.0f)
    {
        target = julianne;
    }
    else
    {
        target = (romulo->GetHealthPct() >= julianne->GetHealthPct()) ? romulo : julianne;
    }

    if (target)
        return MarkTargetWithSkull(bot, target);

    return false;
}

// The Wizard of Oz

bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    static const std::array<const char*, 5> ozTargets =
    {
        "dorothee",
        "tito",
        "roar",
        "strawman",
        "tinhead",
    };

    for (const char* name : ozTargets)
    {
        if (Unit* target = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

bool WizardOfOzScorchStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    return strawman &&
        botAI->CanCastSpell("scorch", strawman) &&
        botAI->CastSpell("scorch", strawman);
}

// The Curator

bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = AI_VALUE2(Unit*, "find target", "astral flare");
    return flare && MarkTargetWithSkull(bot, flare);
}

bool TheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    if (AI_VALUE(Unit*, "current target") != curator)
        return Attack(curator);

    if (curator->GetVictim() != bot)
        return false;

    Position const tankPosition = { -11139.463f, -1884.645f, 165.765f };
    float const distanceToPosition = bot->GetExactDist2d(tankPosition);

    if (distanceToPosition < 2.0f)
        return false;

    float const dX = tankPosition.GetPositionX() - bot->GetPositionX();
    float const dY = tankPosition.GetPositionY() - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

    return MoveTo(
        KARAZHAN_MAP_ID, moveX, moveY, tankPosition.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistance);
}

// Terestian Illhoof

bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    static const std::array<const char*, 3> illhoofTargets =
    {
        "demon chains",
        "kil'rek",
        "terestian illhoof",
    };

    for (const char* name : illhoofTargets)
    {
        if (Unit* target = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

// Shade of Aran

bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    constexpr float safeDistance = 20.0f;
    float const distance = bot->GetDistance2d(aran);
    if (distance >= safeDistance)
        return false;

    botAI->InterruptSpell();
    return MoveAway(aran, safeDistance - distance);
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
        creatureList, static_cast<uint32>(KarazhanNpcs::NPC_CONJURED_ELEMENTAL), searchRadius);

    for (Creature* elemental : creatureList)
    {
        if (!elemental || !elemental->IsAlive())
            continue;

        if (!botAI->HasAura("banish", elemental))
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

// Reasoning: get too close, get Counterspelled;
// get too far, get stuck in alcoves when running away from Blizzard
bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    constexpr float minDistance = 11.0f;
    constexpr float maxDistance = 15.0f;
    float const distanceToBoss = bot->GetExactDist2d(aran);

    if (distanceToBoss > maxDistance)
        return MoveTo(aran, maxDistance, MovementPriority::MOVEMENT_COMBAT);

    if (distanceToBoss < minDistance)
        return MoveTo(aran, minDistance, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

// Netherspite
// CombatReach is 18 yards

// The red beam dance (5 seconds in, 5 seconds out)
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    constexpr float searchRadius = 150.0f;
    Unit* redPortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_RED_PORTAL), searchRadius);
    if (!redPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    bool isBlockingNow = (bot == redBlocker);
    bool wasBlocking = _wasBlockingRedBeam;

    constexpr float idealDistance = 18.0f;
    std::vector<Unit*> voidZones;
    Position beamPos;
    FindBeamPosition(netherspite, redPortal, voidZones, idealDistance, beamPos);

    if (!isBlockingNow)
    {
        _wasBlockingRedBeam = false;
        return false;
    }

    if (!wasBlocking)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_red", "%player is moving to block the red beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingRedBeam = true;

    constexpr uint8 intervalSecs = 5;
    if (std::time(nullptr) - _redBeamMoveTimer >= intervalSecs)
    {
        _lastBeamMoveSideways = !_lastBeamMoveSideways;
        _redBeamMoveTimer = std::time(nullptr);
    }
    if (!_lastBeamMoveSideways)
    {
        return MoveTo(
            KARAZHAN_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(),
            bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
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
            KARAZHAN_MAP_ID, sideX, sideY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

// Two non-Rogue/Warrior DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
bool NetherspiteBlockBlueBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    constexpr float searchRadius = 150.0f;
    Unit* bluePortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_BLUE_PORTAL), searchRadius);
    if (!bluePortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    bool isBlockingNow = (bot == blueBlocker);
    bool wasBlocking = _wasBlockingBlueBeam;

    if (!isBlockingNow)
    {
        if (wasBlocking)
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

    if (!wasBlocking)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_blue",
            "%player is moving to block the blue beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingBlueBeam = true;

    float idealDistance = botAI->IsRanged(bot) ? 25.0f : 18.0f;
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (FindBeamPosition(netherspite, bluePortal, voidZones, idealDistance, beamPos))
    {
        botAI->InterruptSpell();
        return MoveTo(
            KARAZHAN_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(),
            bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one rogue or DPS warrior bot will block the green beam for an entire phase
bool NetherspiteBlockGreenBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    constexpr float searchRadius = 150.0f;
    Unit* greenPortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_GREEN_PORTAL), searchRadius);
    if (!greenPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);
    bool isBlockingNow = (bot == greenBlocker);
    bool wasBlocking = _wasBlockingGreenBeam;

    if (!isBlockingNow)
    {
        if (wasBlocking)
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

    if (!wasBlocking)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_green",
            "%player is moving to block the green beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingGreenBeam = true;

    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (FindBeamPosition(netherspite, greenPortal, voidZones, 18.0f, beamPos))
    {
        botAI->InterruptSpell();
        return MoveTo(
            KARAZHAN_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(),
            bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(bot);

    constexpr float hazardRadius = 4.0f;
    bool const nearVoidZone = !IsSafePosition(
        bot->GetPositionX(), bot->GetPositionY(), voidZones, hazardRadius);

    std::vector<BeamAvoid> beams;
    constexpr float searchRadius = 150.0f;

    Unit* redPortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_RED_PORTAL), searchRadius);
    if (redPortal)
        beams.push_back({redPortal, 0.0f, netherspite->GetExactDist2d(redPortal)});

    Unit* bluePortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_BLUE_PORTAL), searchRadius);
    if (bluePortal)
        beams.push_back({bluePortal, 0.0f, netherspite->GetExactDist2d(bluePortal)});

    Unit* greenPortal = bot->FindNearestCreature(
        static_cast<uint32>(KarazhanNpcs::NPC_GREEN_PORTAL), searchRadius);
    if (greenPortal)
        beams.push_back({greenPortal, 0.0f, netherspite->GetExactDist2d(greenPortal)});

    bool const nearBeam = !IsAwayFromBeams(
        bot->GetPositionX(), bot->GetPositionY(), beams, netherspite);

    if (!nearVoidZone && !nearBeam)
        return false;

    constexpr float maxSearchDist = 30.0f;
    constexpr float stepAngle = M_PI/18.0f;
    constexpr float stepDist = 0.5f;

    Position bestCandidate;
    float bestDistSq = std::numeric_limits<float>::max();
    bool found = false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (float angle = 0; angle < 2 * M_PI; angle += stepAngle)
    {
        for (float dist = 2.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = botX + std::cos(angle) * dist;
            float cy = botY + std::sin(angle) * dist;

            if (!IsSafePosition(cx, cy, voidZones, hazardRadius) ||
                !IsAwayFromBeams(cx, cy, beams, netherspite))
                continue;

            float dx = cx - botX;
            float dy = cy - botY;
            float moveDistSq = dx*dx + dy*dy;

            if (!found || moveDistSq < bestDistSq)
            {
                bestCandidate = Position(cx, cy, bot->GetPositionZ());
                bestDistSq = moveDistSq;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
        bestCandidate.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool NetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(
     float x, float y, const std::vector<BeamAvoid>& beams, Unit* netherspite)
{
    for (auto const& beam : beams)
    {
        float const bx = netherspite->GetPositionX();
        float const by = netherspite->GetPositionY();
        float dx = beam.portal->GetPositionX() - bx;
        float dy = beam.portal->GetPositionY() - by;
        float const length = netherspite->GetExactDist2d(beam.portal);

        if (length == 0.0f)
            continue;

        dx /= length;
        dy /= length;
        float botdx = x - bx, botdy = y - by;
        float distanceAlongBeam = (botdx * dx + botdy * dy);
        float beamX = bx + dx * distanceAlongBeam, beamY = by + dy * distanceAlongBeam;
        float distToBeamSq = (x - beamX) * (x - beamX) + (y - beamY) * (y - beamY);

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
    for (Unit* vz : voidZones)
    {
        if (vz->GetEntry() == static_cast<uint32>(KarazhanNpcs::NPC_VOID_ZONE) &&
            bot->GetExactDist2d(vz) < safeDistance)
        {
            return FleePosition(vz->GetPosition(), safeDistance);
        }
    }

    return false;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    uint32 const instanceId = netherspite->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    bool const isMechanicTracker = IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID);
    bool didSomething = false;

    if (IsBanishPhase(netherspite))
    {
        if (isMechanicTracker && netherspiteDpsWaitTimer.erase(instanceId) > 0)
            didSomething = true;

        Action* redAction = botAI->GetAiObjectContext()->GetAction("netherspite block red beam");
        if (redAction &&
            static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
        {
            didSomething = true;
        }

        Action* blueAction = botAI->GetAiObjectContext()->GetAction("netherspite block blue beam");
        if (blueAction &&
            static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
        {
            didSomething = true;
        }

        Action* greenAction = botAI->GetAiObjectContext()->GetAction("netherspite block green beam");
        if (greenAction &&
            static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
        {
            didSomething = true;
        }
    }
    else
    {
        if (isMechanicTracker && netherspiteDpsWaitTimer.try_emplace(instanceId, now).second)
            didSomething = true;

        if (bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_RED_BEAM_DEBUFF)))
        {
            Action* redAction = botAI->GetAiObjectContext()->GetAction("netherspite block red beam");
            if (redAction &&
                static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState(now))
            {
                didSomething = true;
            }
        }
    }

    return didSomething;
}

// Prince Malchezaar
// CombatReach is 4 yards

bool PrinceMalchezaarEnfeebledBotAvoidHazardAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    constexpr float minSafeBossDistance = 32.0f;
    constexpr float minSafeBossDistanceSq = minSafeBossDistance * minSafeBossDistance;
    constexpr float maxSafeBossDistance = 60.0f;
    constexpr float safeInfernalDistance = 22.0f;
    constexpr float distIncrement = 0.5f;
    constexpr uint8 numAngles = 64;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const malchezaarX = malchezaar->GetPositionX();
    float const malchezaarY = malchezaar->GetPositionY();
    float bestMoveDistSq = std::numeric_limits<float>::max();
    float bestDestX = 0.0f;
    float bestDestY = 0.0f;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2 * M_PI * i) / numAngles;
        float dx = std::cos(angle);
        float dy = std::sin(angle);

        for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += distIncrement)
        {
            float destX = malchezaarX + dx * dist;
            float destY = malchezaarY + dy * dist;
            float destZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bx, by, destZ, destX, destY, destZ, true))
            {
                continue;
            }

            float ddx = destX - malchezaarX;
            float ddy = destY - malchezaarY;
            float distFromBossSq = ddx*ddx + ddy*ddy;
            if (distFromBossSq < minSafeBossDistanceSq)
                continue;

            float mdx = destX - bx;
            float mdy = destY - by;
            float moveDistSq = mdx*mdx + mdy*mdy;

            if (IsStraightPathSafe(bx, by, destX, destY, infernals, safeInfernalDistance) &&
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
        return false

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    constexpr float safeInfernalDistance = 22.0f;
    constexpr float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    constexpr float maxSafeBossDistance = 35.0f;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const malchezaarX = malchezaar->GetPositionX();
    float const malchezaarY = malchezaar->GetPositionY();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float const dx = bx - infernal->GetPositionX();
        float const dy = by - infernal->GetPositionY();
        float const infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    if (!nearInfernal)
        return false;

    float bestDestX = bx;
    float bestDestY = by;
    bool found = TryFindSafePositionWithSafePath(
        bot, Position(bx, by, bot->GetPositionZ()),
        Position(malchezaarX, malchezaarY, malchezaar->GetPositionZ()),
        infernals, safeInfernalDistance, maxSafeBossDistance, bestDestX, bestDestY);

    if (!found)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool PrinceMalchezaarTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    constexpr float safeInfernalDistance = 30.0f;
    constexpr float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    constexpr float maxSampleDist = 75.0f;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float const dx = bx - infernal->GetPositionX();
        float const dy = by - infernal->GetPositionY();
        float const infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    if (!nearInfernal)
        return false;

    float bestDestX = bx;
    float bestDestY = by;
    bool found = TryFindSafePositionWithSafePath(
        bot, Position(bx, by, bot->GetPositionZ()), Position(bx, by, bot->GetPositionZ()),
        infernals, safeInfernalDistance, maxSampleDist, bestDestX, bestDestY);

    if (!found)
        return false;

    return MoveTo(
        KARAZHAN_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

// Nightbane
// CombatReach is 10.5 yards

bool NightbaneGroundPhaseTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") != nightbane)
        return Attack(nightbane);

    if (nightbane->GetVictim() != bot)
        return false;

    Position const domeCenter      = { -11126.015f, -1925.271f, 91.473f };
    Position const terraceEastEnd  = { -11115.958f, -1972.058f, 91.457f };
    Position const terraceWestEnd  = { -11077.521f, -1913.315f, 91.471f };

    float const radius = domeCenter.GetExactDist2d(terraceEastEnd);
    float const thetaA = atan2(
        terraceEastEnd.GetPositionY() - domeCenter.GetPositionY(),
        terraceEastEnd.GetPositionX() - domeCenter.GetPositionX());
    float const thetaB = atan2(
        terraceWestEnd.GetPositionY() - domeCenter.GetPositionY(),
        terraceWestEnd.GetPositionX() - domeCenter.GetPositionX());

    float deltaAB = thetaB - thetaA;
    if (deltaAB < 0.0f) deltaAB += 2.0f * M_PI;

    float arcStart = 0.0f;
    float arcEnd = 0.0f;
    if (deltaAB < M_PI)
    {
        arcStart = thetaB;
        arcEnd = thetaA + 2.0f * M_PI;
    }
    else
    {
        arcStart = thetaA;
        arcEnd = thetaB;
    }

    float thetaN = atan2(
        nightbane->GetPositionY() - domeCenter.GetPositionY(),
        nightbane->GetPositionX() - domeCenter.GetPositionX());

    if (thetaN < arcStart)
        thetaN += 2.0f * M_PI;
    else if (thetaN >= arcStart + 2.0f * M_PI)
        thetaN -= 2.0f * M_PI;

    float const thetaClamped = std::max(arcStart, std::min(arcEnd, thetaN));
    float const destX = domeCenter.GetPositionX() + radius * cos(thetaClamped);
    float const destY = domeCenter.GetPositionY() + radius * sin(thetaClamped);
    float const distanceToPosition = bot->GetExactDist2d(destX, destY);

    if (distanceToPosition < 0.5f)
        return false;

    float const dX = destX - bot->GetPositionX();
    float const dY = destY - bot->GetPositionY();
    float const moveDist = std::min(2.25f, distanceToPosition);
    float const moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
    float const moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;

    bool backwards = nightbane->GetExactDist2d(destX, destY) >=
        distanceToPosition ? true : false;
    return MoveTo(
        KARAZHAN_MAP_ID, destX, destY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// Ranged bots will stack on one "ranged leader" that can be designated by the assistant flag
// The ranged leader will lead the ranged group out of Charred Earths
bool NightbaneGroundPhaseCoordinateRangedMovementAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* rangedLeader = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != KARAZHAN_MAP_ID ||
            !member->IsAlive() || !botAI->IsRanged(member))
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

bool NightbaneGroundPhaseCoordinateRangedMovementAction::MoveRangedLeaderToSafeSpot()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    constexpr float searchRadius = 40.0f;
    constexpr float safeDistance = 12.0f;
    constexpr float minBossDist = 15.0f;
    constexpr float maxBossDist = 35.0f;
    constexpr float angleStep = M_PI / 16.0f;
    constexpr float distStep = 1.0f;

    std::vector<Position> charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, static_cast<uint32>(KarazhanSpells::SPELL_CHARRED_EARTH));

    if (charredEarths.empty())
    {
        float const distToBoss = bot->GetExactDist2d(nightbane);
        if (distToBoss < minBossDist)
        {
            botAI->InterruptSpell();
            return MoveAway(nightbane, minBossDist - distToBoss, true);
        }

        return false;
    }

    float const safeDistSq = safeDistance * safeDistance;
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    bool inDanger = false;
    for (auto const& ce : charredEarths)
    {
        float dx = bx - ce.GetPositionX();
        float dy = by - ce.GetPositionY();
        if (dx * dx + dy * dy < safeDistSq)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    float const nx = nightbane->GetPositionX();
    float const ny = nightbane->GetPositionY();
    float bestDistSq = std::numeric_limits<float>::max();
    float bestX = bx, bestY = by;
    bool found = false;

    for (float dist = minBossDist; dist <= maxBossDist; dist += distStep)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI; angle += angleStep)
        {
            float cx = nx + cos(angle) * dist;
            float cy = ny + sin(angle) * dist;

            bool safe = true;
            for (auto const& ce : charredEarths)
            {
                float dx = cx - ce.GetPositionX();
                float dy = cy - ce.GetPositionY();
                if (dx * dx + dy * dy < safeDistSq)
                {
                    safe = false;
                    break;
                }
            }

            if (!safe || nightbane->GetExactDist2d(cx, cy) > maxBossDist)
                continue;

            float testX = cx;
            float testY = cy;
            float testZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(),
                    bot->GetPositionZ(), testX, testY, testZ))
            {
                continue;
            }

            if (!nightbane->IsWithinLOS(cx, cy, bot->GetPositionZ()))
                continue;

            float dx = cx - bx;
            float dy = cy - by;
            float moveDistSq = dx * dx + dy * dy;
            if (moveDistSq < bestDistSq)
            {
                bestDistSq = moveDistSq;
                bestX = cx;
                bestY = cy;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, bestX, bestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneGroundPhaseCoordinateRangedMovementAction::StackOnRangedLeader(Player* rangedLeader)
{
    if (bot->GetExactDist2d(rangedLeader) < 0.5f)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, rangedLeader->GetPositionX(), rangedLeader->GetPositionY(),
        rangedLeader->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z && pet->GetReactState() == REACT_PASSIVE)
        pet->SetReactState(REACT_DEFENSIVE);

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z && pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }

    return false;
}

// 1. Stack at the "Flight Stack Position" near Nightbane so he doesn't use Fireball Barrage
// 2. Once Rain of Bones hits, the whole party moves to a new stack position
// This lasts for the first 35 seconds of the flight phase, after which Nightbane begins landing
bool NightbaneFlightPhaseStackAndMoveTogetherAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") == nightbane)
    {
        bot->AttackStop();
        botAI->InterruptSpell();
    }

    if (bot->HasAura(static_cast<uint32>(KarazhanSpells::SPELL_RAIN_OF_BONES)))
        _rainOfBonesHit = true;

    Position const rainOfBonesPositions[2] =
    {
        { -11166.516f, -1901.405f, 91.473f },  // primary
        { -11158.752f, -1909.394f, 91.473f },  // backup in case of charred earth
    };
    Position const flightStackPositions[2] =
    {
        { -11156.233f, -1888.353f, 91.473f },  // primary
        { -11149.115f, -1897.154f, 91.473f },  // backup in case of charred earth
    };

    auto const& posArray = _rainOfBonesHit ? rainOfBonesPositions : flightStackPositions;
    Position destPos;
    bool foundSafe = false;

    constexpr float searchRadius = 40.0f;
    constexpr float charredEarthSafeDist = 12.0f;
    std::vector<Position> charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, static_cast<uint32>(KarazhanSpells::SPELL_CHARRED_EARTH));

    for (uint8 i = 0; i < 2; i++)
    {
        destPos = posArray[i];

        bool inCharredEarth = false;
        for (auto const& charredEarth : charredEarths)
        {
            if (charredEarth.GetExactDist2d(destPos) < charredEarthSafeDist)
            {
                inCharredEarth = true;
                break;
            }
        }

        if (!inCharredEarth)
        {
            foundSafe = true;
            break;
        }
    }

    if (!foundSafe)
        return false;

    if (bot->GetExactDist2d(destPos) < 0.5f)
        return false;

    botAI->InterruptSpell();
    return MoveTo(
        KARAZHAN_MAP_ID, destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Failsafe in case bots fall through the world or off the terrace
bool NightbaneTeleportBackToTerraceAction::Execute(Event /*event*/)
{
    Position const flightStackPosition = { -11159.555f, -1893.526f, 91.473f };
    return bot->TeleportTo(
        KARAZHAN_MAP_ID, flightStackPosition.GetPositionX(), flightStackPosition.GetPositionY(),
        flightStackPosition.GetPositionZ(), bot->GetOrientation());
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    bool const isMechanicTracker = IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID);
    bool didSomething = false;

    // Ground Phase: Erase flight phase timer and Rain of Bones tracker and start DPS wait timer
    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
    {
        Action* action = botAI->GetAiObjectContext()->GetAction(
            "nightbane flight phase stack and move together");
        if (action &&
            static_cast<NightbaneFlightPhaseStackAndMoveTogetherAction*>(action)->ResetRainOfBonesHit())
        {
            didSomething = true;
        }

        if (isMechanicTracker)
        {
            if (nightbaneFlightPhaseStartTimer.erase(instanceId) > 0)
                didSomething = true;
            if (nightbaneDpsWaitTimer.try_emplace(instanceId, now).second)
                didSomething = true;
        }
    }
    // Flight Phase: Erase DPS wait timer and start flight phase timer
    else if (isMechanicTracker)
    {
        if (nightbaneDpsWaitTimer.erase(instanceId) > 0)
            didSomething = true;
        if (nightbaneFlightPhaseStartTimer.try_emplace(instanceId, now).second)
            didSomething = true;
    }

    return didSomething;
}
