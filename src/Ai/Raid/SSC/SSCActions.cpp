/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCActions.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "EncounterHelpers.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SSCHelpers.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// General

bool SscResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    bool reset = false;

    reset |= hasReachedVashjRangedPosition.erase(guid) > 0;
    reset |= intendedVashjCorePasserLineup.erase(guid) > 0;
    reset |= lastVashjCoreInInventoryTime.erase(guid) > 0;
    reset |= tidewalkerTankStep.erase(guid) > 0;
    reset |= tidewalkerRangedStep.erase(guid) > 0;
    reset |= lurkerRangedPositions.erase(guid) > 0;

    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return reset;

    reset |= lastVashjCoreImbueAttempt.erase(instanceId) > 0;
    reset |= nearestVashjGeneratorTriggerGuid.erase(instanceId) > 0;
    reset |= karathressDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    reset |= lurkerSpoutTimer.erase(instanceId) > 0;
    reset |= hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0;
    reset |= hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0;
    reset |= hydrossNatureDpsWaitTimer.erase(instanceId) > 0;
    reset |= hydrossFrostDpsWaitTimer.erase(instanceId) > 0;

    return reset;
}

// Trash Mobs

// Move straight out of the toxic pool left behind by some colossi upon death. The pool is centred
// on the corpse, and while that corpse exists it is still the bot's "current target", which
// FleePosition would steer around rather than away from. Nothing else needs to redirect the bot
// mid-escape (the multiplier suppresses all other movement inside the holding radius), so the
// step can be large.
bool UnderbogColossusEscapeToxicPoolAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetToxicPoolPosition(botAI, pool))
        return false;

    constexpr float moveDist = 10.0f;
    constexpr float escapeMargin = 2.0f;
    float stepX;
    float stepY;
    float stepZ;
    if (!GetHazardEscapeStep(
            bot, pool, TOXIC_POOL_HAZARD_RADIUS + escapeMargin, moveDist, stepX, stepY, stepZ))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, stepX, stepY, stepZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event /*event*/) // Deleted GetFirstAliveUnitByEntry, remains in helpers. Can FindNearestCreature get this totem?
{
    constexpr float searchRadius = 20.0f;
    Creature* totem =
        bot->FindNearestCreature(Id(SscNpcs::NPC_WATER_ELEMENTAL_TOTEM), searchRadius);
    return totem && MarkTargetWithSkull(bot, totem);
}

// Shared Bosses

bool SscMisdirectTargetToTankAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE2(Unit*, "find target", _targetName);
    if (!target)
        return false;

    Player* tank = _assistTankIndex == MAIN_TANK ?
        GetGroupMainTank(bot) : GetGroupAssistTank(bot, _assistTankIndex);
    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

// Hydross the Unstable <Duke of Currents>

// Tank Hydross during my phase; once my mark is maxed and the hand-over timer has run, walk him to
// the other tank's position. During the other phase, wait at my own position.
bool HydrossTheUnstablePositionAndSwapTanksAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    bool const myPhase = _frostTank ? IsHydrossInFrostPhase(hydross) : IsHydrossInNaturePhase(hydross);
    bool const markMaxed =
        _frostTank ? HasMarkOfHydrossAt100Percent(bot) : HasMarkOfCorruptionAt100Percent(bot);
    Position const& myPosition = _frostTank ? HYDROSS_FROST_TANK_POSITION : HYDROSS_NATURE_TANK_POSITION;
    Position const& otherPosition = _frostTank ? HYDROSS_NATURE_TANK_POSITION : HYDROSS_FROST_TANK_POSITION;
    std::unordered_map<uint32, uint32> const& handOverTimer =
        _frostTank ? hydrossChangeToNaturePhaseTimer : hydrossChangeToFrostPhaseTimer;

    if (!myPhase)
        return StepTo(myPosition, hydross);

    if (!markMaxed)
    {
        if (AI_VALUE(Unit*, "current target") != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() != bot || !bot->IsWithinMeleeRange(hydross))
            return false;

        return StepTo(myPosition, hydross);
    }

    if (hydross->GetVictim() != bot || !bot->IsWithinMeleeRange(hydross))
        return false;

    constexpr uint32 phaseChangeDelayMs = 1 * IN_MILLISECONDS;
    auto it = handOverTimer.find(hydross->GetInstanceId());
    if (it != handOverTimer.end() && getMSTimeDiff(it->second, getMSTime()) >= phaseChangeDelayMs)
        return StepTo(otherPosition, hydross);

    bot->AttackStop();
    bot->CastStop();
    return true;
}

bool HydrossTheUnstablePositionAndSwapTanksAction::StepTo(
    Position const& position, Unit* hydross)
{
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, hydross, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// To mitigate the effect of Water Tomb
bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event /*event*/)
{
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return false;

    constexpr float safeDistance = 6.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistance);
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    Player* tank = nullptr;
    if (HasNoMarkOfHydross(bot) && IsHydrossInFrostPhase(hydross))
        tank = GetGroupMainTank(bot);
    else if (HasNoMarkOfCorruption(bot) && IsHydrossInNaturePhase(hydross))
        tank = GetGroupAssistTank(bot, 0);

    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", hydross) && botAI->CastSpell("steady shot", hydross);
}

// Ends the auto-attack already running, which a multiplier cannot; WaitForDps keeps it from restarting
bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();
    constexpr uint32 phaseStartStopMs = 5 * IN_MILLISECONDS;
    constexpr uint32 phaseEndStopMs = 1 * IN_MILLISECONDS;

    bool shouldStopDps = false;

    // 1 second after 100% Mark of Hydross, stop DPS
    auto itNature = hydrossChangeToNaturePhaseTimer.find(instanceId);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() &&
        getMSTimeDiff(itNature->second, now) >= phaseEndStopMs)
    {
        shouldStopDps = true;
    }

    // Keep DPS stopped for 5 seconds after transition into nature phase
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(instanceId);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() &&
        getMSTimeDiff(itNatureDps->second, now) < phaseStartStopMs)
    {
        shouldStopDps = true;
    }

    // 1 second after 100% Mark of Corruption, stop DPS
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(instanceId);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() &&
        getMSTimeDiff(itFrost->second, now) >= phaseEndStopMs)
    {
        shouldStopDps = true;
    }

    // Keep DPS stopped for 5 seconds after transition into frost phase
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(instanceId);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() &&
        getMSTimeDiff(itFrostDps->second, now) < phaseStartStopMs)
    {
        shouldStopDps = true;
    }

    if (!shouldStopDps)
        return false;

    bot->AttackStop();
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());

    return true;
}

bool HydrossTheUnstableManageTimersAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    uint32 const instanceId = hydross->GetInstanceId();
    uint32 const now = getMSTime();

    bool changed = false;

    if (IsHydrossInFrostPhase(hydross))
    {
        if (hydrossFrostDpsWaitTimer.try_emplace(instanceId, now).second ||
            hydrossNatureDpsWaitTimer.erase(instanceId) > 0 ||
            hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0)
            changed = true;

        if (HasMarkOfHydrossAt100Percent(bot) &&
            hydrossChangeToNaturePhaseTimer.try_emplace(instanceId, now).second)
            changed = true;
    }
    else // Nature phase
    {
        if (hydrossNatureDpsWaitTimer.try_emplace(instanceId, now).second ||
            hydrossFrostDpsWaitTimer.erase(instanceId) > 0 ||
            hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0)
            changed = true;

        if (HasMarkOfCorruptionAt100Percent(bot) &&
            hydrossChangeToFrostPhaseTimer.try_emplace(instanceId, now).second)
            changed = true;
    }

    return changed;
}

// The Lurker Below

// Run around behind Lurker during Spout
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    float radius = frand(19.0f, 20.0f);
    float botAngle = std::atan2(
        bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    float relativeAngle = Position::NormalizeOrientation(botAngle - lurker->GetOrientation());
    constexpr float safeArc = M_PI / 2.0f;

    if (!PlayerbotAI::IsMainTank(bot) &&
        std::fabs(Position::NormalizeOrientation(relativeAngle - M_PI)) > safeArc / 2.0f)
    {
        float tangentAngle = botAngle + (relativeAngle > M_PI ? -0.1f : 0.1f);
        float moveX = lurker->GetPositionX() + radius * std::cos(tangentAngle);
        float moveY = lurker->GetPositionY() + radius * std::sin(tangentAngle);

        bot->CastStop();
        return MoveTo(SSC_MAP_ID, moveX, moveY, lurker->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        float behindAngle = lurker->GetOrientation() + M_PI + frand(-0.5f, 0.5f) * safeArc;
        float targetX = lurker->GetPositionX() + radius * std::cos(behindAngle);
        float targetY = lurker->GetPositionY() + radius * std::sin(behindAngle);

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            bot->CastStop();
            return MoveTo(SSC_MAP_ID, targetX, targetY, lurker->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool TheLurkerBelowPositionMainTankAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (AI_VALUE(Unit*, "current target") != lurker)
        return Attack(lurker);

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, LURKER_TANK_POSITION, arrivalDist, lurker, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Assign ranged positions within a 120-degree arc behind Lurker
bool TheLurkerBelowSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SSC_MAP_ID || !GET_PLAYERBOT_AI(bot) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    ObjectGuid const guid = bot->GetGUID();

    auto it = lurkerRangedPositions.find(guid);
    if (it == lurkerRangedPositions.end())
    {
        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ?
            std::distance(rangedMembers.begin(), findIt) : 0;

        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.262f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        constexpr float radius = 27.0f;

        float targetX = lurker->GetPositionX() + radius * std::sin(angle);
        float targetY = lurker->GetPositionY() + radius * std::cos(angle);

        lurkerRangedPositions.try_emplace(guid, Position(targetX, targetY, lurker->GetPositionZ()));
        it = lurkerRangedPositions.find(guid);
    }

    if (it == lurkerRangedPositions.end())
        return false;

    Position const& position = it->second;
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// During the submerge phase, if there are >= 3 tanks in the raid,
// the first 3 will each pick up 1 Guardian
bool TheLurkerBelowTanksPickUpAddsAction::Execute(Event /*event*/)
{
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);
    if (!secondAssistTank)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    if (!firstAssistTank)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    std::vector<Unit*> guardians;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == Id(SscNpcs::NPC_COILFANG_GUARDIAN))
            guardians.push_back(unit);
    }

    if (guardians.size() < 3)
        return false;

    std::vector<Player*> tanks = { mainTank, firstAssistTank, secondAssistTank };
    std::vector<uint8> rtiIndices =
    {
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex
    };
    std::vector<std::string> rtiNames = { "star", "circle", "diamond" };

    for (size_t i = 0; i < 3; ++i)
    {
        Player* tank = tanks[i];
        Unit* guardian = guardians[i];
        if (bot == tank)
        {
            if (MarkTargetWithIcon(bot, guardian, rtiIndices[i]))
                return true;

            SetRtiTarget(botAI, rtiNames[i]);

            if (AI_VALUE(Unit*, "current target") != guardian)
                return Attack(guardian);
        }
    }

    return false;
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    uint32 const instanceId = lurker->GetInstanceId();
    uint32 const now = getMSTime();

    bool changed = false;

    auto it = lurkerSpoutTimer.find(instanceId);
    if (it != lurkerSpoutTimer.end() &&
        getMSTimeDiff(it->second, now) >= LURKER_SPOUT_DURATION_MS)
    {
        lurkerSpoutTimer.erase(it);
        changed = true;
        it = lurkerSpoutTimer.end();
    }

    if (lurker->FindCurrentSpellBySpellId(Id(SscSpells::SPELL_SPOUT_VISUAL)) &&
        it == lurkerSpoutTimer.end())
    {
        lurkerSpoutTimer.try_emplace(instanceId, now);
        changed = true;
    }

    return changed;
}

// Leotheras the Blind

// Warlock tank action--see GetLeotherasWarlockTank in RaidSSCHelpers.cpp
// Use tank strategy for Demon Form and DPS strategy for Human Form
bool LeotherasTheBlindWarlockTankAttackBossAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetActiveLeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    return botAI->CanCastSpell("searing pain", leotherasDemon) &&
        botAI->CastSpell("searing pain", leotherasDemon);
}

// Stop melee tanks from attacking upon transformation so they don't take aggro
// Applies only if there is a Warlock tank present
bool LeotherasTheBlindMeleeTanksDontAttackDemonFormAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());

    return true;
}

// Intent is to keep enough distance from Leotheras and spread to prepare for Whirlwind
// And stay away from the Warlock tank to avoid Chaos Blasts
bool LeotherasTheBlindPositionRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistFromBoss = 15.0f;
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(bot);
    if (leotherasHumanoid && bot->GetExactDist2d(leotherasHuman) < safeDistFromBoss &&
        leotherasHumanoid->GetVictim() != bot)
    {
        if (FleePosition(leotherasHumanoid->GetPosition(), safeDistFromBoss))
            return true;
    }

    if (!GetActiveLeotherasDemon(bot))
        return false;

    constexpr float searchRadius = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, searchRadius);
    if (!nearestPlayer)
        return false;

    Player* warlockTank = GetLeotherasWarlockTank(bot);
    float safeDistance = std::numeric_limits<float>::max();
    uint32 minInterval = std::numeric_limits<uint32>::max();
    if (warlockTank != bot && warlockTank == nearestPlayer)
    {
        safeDistance = 10.0f;
        minInterval = 0;
    }
    else
    {
        safeDistance = 5.0f;
        minInterval = 1000;
    }

    if (bot->GetExactDist2d(nearestPlayer) >= safeDistance)
        return false;

    return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(bot);
    if (!leotherasHumanoid)
        return false;

    float const currentDistance = bot->GetExactDist2d(leotherasHumanoid);
    constexpr float safeDistance = 25.0f;
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(leotherasHumanoid, safeDistance - currentDistance);
}

// This method is likely unnecessary unless the player does not use a Warlock tank
// If a melee tank is used, other melee needs to run away after too many Chaos Blast stacks
bool LeotherasTheBlindMeleeDpsRunAwayFromBossAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE &&
        botAI->CanCastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS), bot))
    {
        return true;
    }

    Creature* leotheras = GetPhase2LeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    Unit* demonVictim = leotherasDemon->GetVictim();
    if (!demonVictim || demonVictim == bot)
        return false;

    float currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 10.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(demonVictim, safeDistance - currentDistance);
}

// Hardcoded actions for healers and bear tanks to kill Inner Demons
bool LeotherasTheBlindDestroyInnerDemonAction::Execute(Event /*event*/)
{
    Creature* innerDemon = GetPersonalInnerDemon(botAI);
    if (!innerDemon)
        return false;

    if (bot->getClass() == CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return HandleFeralTankStrategy(innerDemon);

    if (PlayerbotAI::IsHeal(bot))
        return HandleHealerStrategy(innerDemon);

    return AI_VALUE(Unit*, "current target") != innerDemon && Attack(innerDemon);
}

// At 50% nerfed damage, bears have trouble killing their Inner Demons without a specific strategy
// Warrior and Paladin tanks have no trouble in my experience (Prot Warriors have high DPS, and
// Prot Paladins have an advantage in that Inner Demons are weak to Holy)
bool LeotherasTheBlindDestroyInnerDemonAction::HandleFeralTankStrategy(Unit* innerDemon)
{
    if (bot->HasAura(Id(SscSpells::SPELL_DIRE_BEAR_FORM)))
    {
        bot->RemoveOwnedAura(
            Id(SscSpells::SPELL_DIRE_BEAR_FORM), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    if (bot->HasAura(Id(SscSpells::SPELL_BEAR_FORM)))
    {
        bot->RemoveOwnedAura(
            Id(SscSpells::SPELL_BEAR_FORM), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    if (!bot->HasAura(Id(SscSpells::SPELL_CAT_FORM)) &&
        botAI->CanCastSpell(Id(SscSpells::SPELL_CAT_FORM), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_CAT_FORM), bot))
        return true;

    if (botAI->CanCastSpell(Id(SscSpells::SPELL_DRUID_BERSERK), bot) &&
        botAI->CastSpell(Id(SscSpells::SPELL_DRUID_BERSERK), bot))
        return true;

    if (bot->GetPower(POWER_ENERGY) < 30 &&
        botAI->CanCastSpell("tiger's fury", bot) && botAI->CastSpell("tiger's fury", bot))
        return true;

    if (bot->GetComboPoints() >= 4 &&
        botAI->CanCastSpell("ferocious bite", innerDemon) &&
        botAI->CastSpell("ferocious bite", innerDemon))
        return true;

    if (bot->GetComboPoints() == 0 && innerDemon->GetHealthPct() > 25.0f &&
        botAI->CanCastSpell("rake", innerDemon) && botAI->CastSpell("rake", innerDemon))
        return true;

    return botAI->CanCastSpell("mangle (cat)", innerDemon) &&
        botAI->CastSpell("mangle (cat)", innerDemon);
}

bool LeotherasTheBlindDestroyInnerDemonAction::HandleHealerStrategy(Unit* innerDemon)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        if (bot->HasAura(Id(SscSpells::SPELL_TREE_OF_LIFE)))
        {
            bot->RemoveOwnedAura(
                Id(SscSpells::SPELL_TREE_OF_LIFE), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
        }

        if (botAI->CanCastSpell("barkskin", bot) &&
            botAI->CastSpell("barkskin", bot))
            return true;

        return botAI->CanCastSpell("wrath", innerDemon) && botAI->CastSpell("wrath", innerDemon);
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        if (botAI->CanCastSpell(Id(SscSpells::SPELL_AVENGING_WRATH), bot) &&
            botAI->CastSpell(Id(SscSpells::SPELL_AVENGING_WRATH), bot))
            return true;

        if (botAI->CanCastSpell("consecration", bot) &&
            botAI->CastSpell("consecration", bot))
            return true;

        if (botAI->CanCastSpell("exorcism", innerDemon) &&
            botAI->CastSpell("exorcism", innerDemon))
            return true;

        if (botAI->CanCastSpell("hammer of wrath", innerDemon) &&
            botAI->CastSpell("hammer of wrath", innerDemon))
            return true;

        if (botAI->CanCastSpell("holy shock", innerDemon) &&
            botAI->CastSpell("holy shock", innerDemon))
            return true;

        return botAI->CanCastSpell("judgement of light", innerDemon) &&
            botAI->CastSpell("judgement of light", innerDemon);
    }

    if (bot->getClass() == CLASS_PRIEST)
        return botAI->CanCastSpell("smite", innerDemon) && botAI->CastSpell("smite", innerDemon);

    if (bot->getClass() == CLASS_SHAMAN)
    {
        if (botAI->CanCastSpell("earth shock", innerDemon) &&
            botAI->CastSpell("earth shock", innerDemon))
            return true;

        if (botAI->CanCastSpell("chain lightning", innerDemon) &&
            botAI->CastSpell("chain lightning", innerDemon))
            return true;

        return botAI->CanCastSpell("lightning bolt", innerDemon) &&
            botAI->CastSpell("lightning bolt", innerDemon);
    }

    return false;
}

// Everybody except the Warlock tank should focus on Leotheras in Phase 3
bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(bot);
    if (!leotherasHumanoid)
        return false;

    if (AI_VALUE(Unit*, "current target") != leotherasHumanoid)
        return Attack(leotherasHumanoid);

    if (leotherasHumanoid->GetVictim() != bot)
        return false;

    return MoveLeotherasFromWarlockTank(leotherasHumanoid);
}

bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::MoveLeotherasFromWarlockTank(
    Creature* leotherasHumanoid)
{
    Creature* leotherasDemon = GetPhase3LeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    Unit* demonVictim = leotherasDemon->GetVictim();
    if (!demonVictim || demonVictim == bot)
        return false;

    float const currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 20.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(demonVictim, safeDistance - safeDistance, true);
}

// Misdirect to Warlock tank or to main tank if there is no Warlock tank
bool LeotherasTheBlindMisdirectBossToWarlockTankAction::Execute(Event /*event*/)
{
    Creature* leotherasDemon = GetActiveLeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    Player* tank = GetLeotherasWarlockTank(bot);
    if (!tank)
        tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION))
        return false;

    return botAI->CanCastSpell("steady shot", leotherasDemon)) &&
        botAI->CastSpell("steady shot", leotherasDemon);
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool LeotherasTheBlindManageDpsWaitTimersAction::Execute(Event /*event*/)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    uint32 const instanceId = leotheras->GetInstanceId();
    uint32 const now = getMSTime();

    bool changed = false;

    if (IsLeotherasHumanoidPhase(bot))
    {
        changed |= leotherasHumanoidPhaseDpsWaitTimer.try_emplace(instanceId, now).second;
        changed |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    }
    else if (IsLeotherasDemonPhase(bot))
    {
        changed |= leotherasDemonPhaseDpsWaitTimer.try_emplace(instanceId, now).second;
        changed |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0;
    }
    else if (IsLeotherasFinalPhase(bot))
    {
        changed |= leotherasFinalPhaseDpsWaitTimer.try_emplace(instanceId, now).second;
        changed |= leotherasHumanoidPhaseDpsWaitTimer.erase(instanceId) > 0;
        changed |= leotherasDemonPhaseDpsWaitTimer.erase(instanceId) > 0;
    }

    return changed;
}

// Fathom-Lord Karathress
// Note: 4 tanks are required for the full strategy, and having at least 2
// is crucial to separate Caribdis from the others

// Karathress is tanked by the main tank near his starting position
// Caribdis is pulled far to the West in the corner by the first assist tank
// Sharkkis is pulled North to the other side of the ramp by the second assist tank
// Tidalvess is pulled Northwest near the pillar by the third assist tank
bool FathomLordKarathressTanksPositionTargetsAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position position;

    if (PlayerbotAI::IsMainTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
        position = KARATHRESS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        position = CARIBDIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        position = SHARKKIS_TANK_POSITION;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, true))
    {
        target = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        position = TIDALVESS_TANK_POSITION;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    constexpr float arrivalDist = 4.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, position, arrivalDist, target, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Caribdis's tank spot is far away so a dedicated healer is needed
// Use the assistant flag to select the healer
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 4.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, CARIBDIS_HEALER_POSITION, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == SSC_MAP_ID &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* enemy = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 1)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        tank = GetGroupAssistTank(bot, 2);
    }
    else if (hunterIndex == 2)
    {
        enemy = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        tank = GetGroupAssistTank(bot, 1);
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", enemy) && botAI->CastSpell("steady shot", enemy);
}

// Kill order is non-standard because bots handle Cyclones poorly and need more time
// to get her down than real players (standard is ranged DPS help with Sharkkis first)
bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event /*event*/)
{
    /* constexpr float searchRadius = 75.0f;
    Creature* totem = bot->FindNearestCreature(Id(SscNpcs::NPC_SPITFIRE_TOTEM), searchRadius);
    if (totem && PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
    {
        if (MarkTargetWithSkull(bot, totem))
            return true;

        if (AI_VALUE(Unit*, "current target") != totem)
            return Attack(totem);

        // Direct movement order due to path between Sharkkis and totem sometimes being screwy
        if (bot->IsWithinMeleeRange(totem))
            return false;

        return MoveTo(SSC_MAP_ID, totem->GetPositionX(), totem->GetPositionY(),
            bot->GetPositionZ(), false, false, false, true,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    } */

    Unit* target = nullptr;
    Unit* caribdis = nullptr;

    constexpr float searchRadius = 75.0f;
    if (PlayerbotAI::IsMelee(bot) &&
        (Unit* totem = bot->FindNearestCreature(Id(SscNpcs::NPC_SPITFIRE_TOTEM), searchRadius)))
    {
        target = totem;
    }
    else if (Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess"))
    {
        target = tidalvess;
    }
    else if (PlayerbotAI::IsRanged(bot) &&
        (Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis")))
    {
        target = caribdis;
    }
    else if (Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis"))
    {
        target = sharkkis;
    }
    else if (Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat"))
    {
        target = fathomSporebat;
    }
    else if (Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");)
    {
        target = fathomLurker;
    }
    else if (Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        target = karathress;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target = caribdis)
    {
        if (MarkTargetWithCross(bot, caribdis))
            return true;
    }
    else if (MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return false;

    /* if (!caribdis)
        return false;

    Position const& position = CARIBDIS_RANGED_DPS_POSITION;
    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    constexpr float spreadDistance = 8.0f;
    return MoveInside(
        SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        spreadDistance, MovementPriority::MOVEMENT_COMBAT); */
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    return karathressDpsWaitTimer.try_emplace(karathress->GetInstanceId(), getMSTime()).second;
}

// Morogrim Tidewalker

// Separate tanking positions are used for phase 1 and phase 2 to address the Water Globule
// mechanic in phase 2
bool MorogrimTidewalkerMoveBossToTankPositionAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (AI_VALUE(Unit*, "current target") != tidewalker)
        return Attack(tidewalker);

    if (tidewalker->GetVictim() != bot || !bot->IsWithinMeleeRange(tidewalker))
        return false;

    if (tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_HEALTH_PCT + 2.0f)
        return MoveToPhase1TankPosition();

    return MoveToPhase2TankPosition();
}

// Phase 1: tank position is up against the Northeast pillar
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase1TankPosition()
{
    const Position& phase1 = TIDEWALKER_PHASE_1_TANK_POSITION;
    float distToPhase1 = bot->GetExactDist2d(phase1.GetPositionX(), phase1.GetPositionY());
    if (distToPhase1 > 1.0f)
    {
        float dX = phase1.GetPositionX() - bot->GetPositionX();
        float dY = phase1.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToPhase1);
        float moveX = bot->GetPositionX() + (dX / distToPhase1) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPhase1) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, phase1.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Phase 2: move in two steps to get around the pillar and back up into the Northeast corner
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition()
{
    const Position& phase2 = TIDEWALKER_PHASE_2_TANK_POSITION;
    const Position& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerTankStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerTankStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else
            tidewalkerTankStep.try_emplace(bot->GetGUID(), 1);
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged stack behind the boss in the Northeast corner in phase 2
// No corresponding method for melee since they will do so anyway
bool MorogrimTidewalkerPhase2RepositionRangedAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const Position& phase2 = TIDEWALKER_PHASE_2_RANGED_POSITION;
    const Position& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerRangedStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerRangedStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            tidewalkerRangedStep.try_emplace(bot->GetGUID(), 1);
            step = 1;
        }
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (AI_VALUE(Unit*, "current target") != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() != bot || !bot->IsWithinMeleeRange(vashj))
        return false;

    // Phase 1: Position Vashj in the center of the platform
    if (GetLadyVashjPhase(vashj) == 1)
    {
        constexpr float arrivalDistance = 3.0f;
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, VASH_PLATFORM_CENTER_POSITION, arrivalDist, vashj, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    // Phase 3: No fixed position, but move Vashj away from Enchanted Elementals
    constexpr float searchRadius = 15.0f;
    Creature* enchanted =
        bot->FindNearestCreature(Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL), searchRadius); // NEED TO CHANGE THIS TO GET ALL ENCHANTED, NOT JUST ONE
    if (!enchanted)
        return false;

    float const currentDistance = bot->GetExactDist2d(enchanted);
    constexpr float safeDistance = 10.0f;
    if (currentDistance >= safeDistance)
        return false;

    return MoveAway(enchanted, safeDistance - currentDistance);
}

// Semicircle around center of the room (to allow escape paths by Static Charged bots)
bool LadyVashjPhase1SpreadRangedInArcAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> spreadMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetMapId() == SSC_MAP_ID && GET_PLAYERBOT_AI(member) &&
            PlayerbotAI::IsRanged(member))
        {
            spreadMembers.push_back(member);
        }
    }

    const ObjectGuid guid = bot->GetGUID();
    auto itReached = hasReachedVashjRangedPosition.find(guid);

    auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
    size_t botIndex = (it != spreadMembers.end()) ?
        std::distance(spreadMembers.begin(), it) : 0;
    size_t count = spreadMembers.size();
    if (count == 0)
        return false;

    constexpr float arcCenter = M_PI / 2.0f; // North
    constexpr float arcSpan = M_PI; // 180°
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    float angle;
    if (count == 1)
        angle = arcCenter;
    else
        angle = arcStart + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

    const Position& center = VASHJ_PLATFORM_CENTER_POSITION;
    float radius = 25.0f;
    float targetX = center.GetPositionX() + radius * std::cos(angle);
    float targetY = center.GetPositionY() + radius * std::sin(angle);
    float targetZ = center.GetPositionZ();

    if (itReached == hasReachedVashjRangedPosition.end() || !(itReached->second))
    {
        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            hasReachedVashjRangedPosition.try_emplace(guid, false);
            return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        hasReachedVashjRangedPosition[guid] = true;
    }

    return false;
}

// For absorbing Shock Burst
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (mainTank->HasAura(Id(SscSpells::SPELL_GROUNDING_TOTEM_EFFECT)))
        return false;

    constexpr float distFromTank = 25.0f;
    if (bot->GetDistance(mainTank) > distFromTank)
        return MoveTo(mainTank, distFromTank, MovementPriority::MOVEMENT_COMBAT);

    return botAI->CanCastSpell("grounding totem", bot) &&
           botAI->CastSpell("grounding totem", bot);
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // If the main tank has Static Charge, other group members should move away
    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && bot != mainTank && mainTank->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)))
    {
        float const currentDistance = bot->GetExactDist2d(mainTank);
        constexpr float safeDistance = 11.0f;
        if (currentDistance >= safeDistance)
            return false;

        return MoveAway(mainTank, safeDistance - currentDistance);
    }

    // If any other bot has Static Charge, it should move away from other group members
    if (PlayerbotAI::IsMainTank(bot) || !bot->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)))
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float currentDistance = bot->GetExactDist2d(member);
        constexpr float safeDistance = 11.0f;
        if (currentDistance >= safeDistance)
            return false;

        return MoveFromGroup(safeDistance);
    }

    return false;
}

bool LadyVashjAssignPhase2AndPhase3DpsPriorityAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Position const& center = VASHJ_PLATFORM_CENTER_POSITION;
    float platformZ = center.GetPositionZ();
    if (bot->GetPositionZ() - platformZ > 2.0f)
    {
        // This block is needed to prevent bots from floating into the air to attack sporebats
        bot->AttackStop();
        bot->CastStop();
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->NearTeleportTo(
            bot->GetPositionX(), bot->GetPositionY(), platformZ, bot->GetOrientation());

        return true;
    }

    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    Unit* target = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    // Search and attack radius are intended to keep bots from going down the stairs
    const float maxSearchRange =
        PlayerbotAI::IsRanged(bot) ? 60.0f : 55.0f;
    const float maxPursueRange = maxSearchRange - 5.0f;
    int8 phase = GetLadyVashjPhase(vashj);

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidLadyVashjCombatNpc(unit, botAI, vashj))
            continue;

        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (phase == 2 && distFromCenter > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;
                break;

            case Id(SscNpcs::NPC_COILFANG_ELITE):
                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case Id(SscNpcs::NPC_COILFANG_STRIDER):
                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case Id(SscNpcs::NPC_TOXIC_SPOREBAT):
                if (!sporebat || bot->GetDistance(unit) < bot->GetDistance(sporebat))
                    sporebat = unit;
                break;

            case Id(SscNpcs::NPC_LADY_VASHJ):
                vashj = unit;
                break;

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (phase == 2)
    {
        if (PlayerbotAI::IsRanged(bot))
        {
            // Hunters and Mages prioritize Enchanted Elementals,
            // while other ranged DPS prioritize Striders
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { enchanted, strider, elite };
            else
                targets = { strider, elite, enchanted };
        }
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchanted, elite };
        else if (PlayerbotAI::IsTank(bot))
        {
            if (botAI->HasCheat(BotCheatMask::raid) &&
                PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
                targets = { strider, elite, enchanted };
            else
                targets = { elite, strider, enchanted };
        }
        else
            targets = { enchanted, elite, strider };
    }

    if (phase == 3)
    {
        if (PlayerbotAI::IsTank(bot))
        {
            if (PlayerbotAI::IsMainTank(bot))
            {
                if (MarkTargetWithDiamond(bot, vashj))
                    return true;

                SetRtiTarget(botAI, "diamond");
                targets = { vashj };
            }
            else if (botAI->HasCheat(BotCheatMask::raid) &&
                     PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
            {
                targets = { strider, elite, enchanted, vashj };
            }
            else
                targets = { elite, strider, enchanted, vashj };
        }
        else if (PlayerbotAI::IsRanged(bot))
        {
            // Hunters are assigned to kill Sporebats in Phase 3
            if (bot->getClass() == CLASS_HUNTER)
                targets = { sporebat, enchanted, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    for (Unit* candidate : targets)
    {
        if (candidate && bot->GetExactDist2d(candidate) <= maxPursueRange)
        {
            target = candidate;
            break;
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();

    if (currentTarget && !IsValidLadyVashjCombatNpc(currentTarget, botAI, vashj))
    {
        bot->AttackStop();
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
        currentTarget = nullptr;
    }

    if (target && currentTarget != target && AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    // If bots have wandered too far from the center, move them back
    if (bot->GetExactDist2d(vashj) <= maxPursueRange) // THIS DOESN'T WORK SINCE MOVETO A WORLD OBJECT IS LIMITED TO SPELL DIST
        return false;

    return MoveTo(vashj, maxPursueRange - 10.0f, MovementPriority::MOVEMENT_FORCED);
}

bool LadyVashjTankAttackAndMoveAwayStriderAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    if (!strider)
        return false;

    // Raid cheat automatically applies Fear Ward to tanks to make Strider tankable
    // This simulates the real-life strategy where the Strider can be meleed by
    // players wearing an Ogre Suit (due to the extended combat reach)
    if (botAI->HasCheat(BotCheatMask::raid) && PlayerbotAI::IsTank(bot))
    {
        if (!bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)))
            bot->AddAura(Id(SscSpells::SPELL_FEAR_WARD), bot);

        if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            AI_VALUE(Unit*, "current target") != strider)
            return Attack(strider);

        float currentDistance = bot->GetExactDist2d(vashj);
        constexpr float safeDistance = 28.0f;
        if (strider->GetVictim() != bot || currentDistance >= safeDistance)
            return false;

        return MoveAway(vashj, safeDistance - currentDistance, true);
    }

    // Don't move away if raid cheats are enabled, or in any case if the bot is a tank
    if (!botAI->HasCheat(BotCheatMask::raid))
    {
        float currentDistance = bot->GetExactDist2d(strider);
        constexpr float safeDistance = 20.0f;
        if (!PlayerbotAI::IsTank(bot) && currentDistance < safeDistance)
            return MoveAway(strider, safeDistance - currentDistance);

        // Try to root/slow the Strider if it is not tankable (poor man's kiting strategy)
        if (!botAI->HasAura("frost shock", strider) && bot->getClass() == CLASS_SHAMAN &&
            botAI->CanCastSpell("frost shock", strider))
        {
            return botAI->CastSpell("frost shock", strider);
        }
        else if (!strider->HasAura(Id(SscSpells::SPELL_CURSE_OF_EXHAUSTION)) && bot->getClass() == CLASS_WARLOCK &&
                 botAI->CanCastSpell("curse of exhaustion", strider))
        {
            return botAI->CastSpell("curse of exhaustion", strider);
        }
        else if (!strider->HasAura(Id(SscSpells::SPELL_SLOW)) && bot->getClass() == CLASS_MAGE &&
                 botAI->CanCastSpell("slow", strider))
        {
            return botAI->CastSpell("slow", strider);
        }
    }

    return false;
}

// If cheats are enabled, the first returned melee DPS bot will teleport to Tainted Elementals
// Such bot will recover HP and remove the Poison Bolt debuff while attacking the elemental
bool LadyVashjTeleportToTaintedElementalAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    bool const isWithinMeleeRange = bot->IsWithinMeleeRange(tainted);

    if (!isWithinTaintedMeleeRange)
    {
        bot->CastStop();
        bot->NearTeleportTo(
            tainted->GetPositionX(), tainted->GetPositionY(), tainted->GetPositionZ(),
            tainted->GetOrientation());
    }

    if (AI_VALUE(Unit*, "current target") != tainted)
        return Attack(tainted);

    if (!isWithinTaintedMeleeRange)
        return false;

    bot->SetFullHealth();
    bot->RemoveAura(Id(SscSpells::SPELL_POISON_BOLT));
}

bool LadyVashjLootTaintedCoreAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasItemCount(Id(SscItems::ITEM_TAINTED_CORE), 1, false))
            return false;
    }

    constexpr float searchRadius = 150.0f;
    Creature* elemental =
        bot->FindNearestCreature(Id(SscNpcs::NPC_TAINTED_ELEMENTAL), searchRadius, false);

    if (!elemental || elemental->IsAlive())
        return false;

    LootObject loot(bot, elemental->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    const float maxLootRange = sPlayerbotAIConfig.lootDistance;
    constexpr float distFromObject = 2.0f;

    if (bot->GetDistance(elemental) > maxLootRange)
        return MoveTo(elemental, distFromObject, MovementPriority::MOVEMENT_FORCED);

    OpenLootAction open(botAI);
    if (!open.Execute(Event()))
        return false;

    bot->SetLootGUID(elemental->GetGUID());
    constexpr uint8 coreIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << coreIndex;
    bot->GetSession()->QueuePacket(packet);

    const uint32 now = getMSTime();
    lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);

    return true;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(botAI, bot);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(botAI, bot);

    const uint32 instanceId = vashj->GetInstanceId();

    Unit* closestTrigger = nullptr;
    if (Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
        (closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(tainted)))
    {
        nearestVashjGeneratorTriggerGuid.try_emplace(instanceId, closestTrigger->GetGUID());
    }

    auto itSnap = nearestVashjGeneratorTriggerGuid.find(instanceId);
    if (itSnap != nearestVashjGeneratorTriggerGuid.end() && !itSnap->second.IsEmpty())
    {
        if (Unit* snapUnit = botAI->GetUnit(itSnap->second))
            closestTrigger = snapUnit;
        else
            nearestVashjGeneratorTriggerGuid.erase(instanceId);
    }

    if (!closestTrigger)
        return false;

    // Not gated behind CheatMask because the auto application of Fear Ward is necessary
    // to address an issue with bot movement, which is that bots cannot be rooted and
    // therefore will move when feared while holding the Tainted Core
    if (!bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)))
        bot->AddAura(Id(SscSpells::SPELL_FEAR_WARD), bot);

    Item* item = bot->GetItemByEntry(Id(SscItems::ITEM_TAINTED_CORE));
    if (!item || !botAI->HasItemInInventory(Id(SscItems::ITEM_TAINTED_CORE)))
    {
        // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
        if (bot == firstCorePasser &&
            LineUpFirstCorePasser(designatedLooter))
        {
            return true;
        }
        else if (bot == secondCorePasser &&
                 LineUpSecondCorePasser(firstCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == thirdCorePasser && LineUpThirdCorePasser(
                 designatedLooter, firstCorePasser, secondCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == fourthCorePasser && LineUpFourthCorePasser(
                 firstCorePasser, secondCorePasser, thirdCorePasser, closestTrigger))
        {
            return true;
        }
    }
    else if (item && botAI->HasItemInInventory(Id(SscItems::ITEM_TAINTED_CORE)))
    {
        // Designated core looter logic
        // Applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter &&
            IsFirstCorePasserInPosition(firstCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            const uint32 now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, firstCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // First core passer: receive core from looter at the top of the stairs,
        // pass to second core passer
        else if (bot == firstCorePasser &&
                 IsSecondCorePasserInPosition(secondCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            const uint32 now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, secondCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Second core passer: if closest usable generator is within passing distance
        // of the first passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == secondCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsThirdCorePasserInPosition(thirdCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            const uint32 now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, thirdCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Third core passer: if closest usable generator is within passing distance
        // of the second passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == thirdCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsFourthCorePasserInPosition(fourthCorePasser))
        {
            constexpr uint32 imbueRetryDelayMs = 2 * IN_MILLISECONDS;
            const uint32 now = getMSTime();
            auto it = lastVashjCoreImbueAttempt.find(instanceId);
            if (it == lastVashjCoreImbueAttempt.end() ||
                getMSTimeDiff(it->second, now) >= imbueRetryDelayMs)
            {
                lastVashjCoreImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, fourthCorePasser);
                lastVashjCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                return true;
            }
        }
        // Fourth core passer: the fourth passer is rarely needed and no more than
        // four ever should be, so it should use the Core on the nearest generator
        else if (bot == fourthCorePasser && UseCoreOnNearestGenerator(instanceId))
            return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(
    Player* designatedLooter)
{
    if (!designatedLooter)
        return false;

    const float centerX = VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    const float centerY = VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();
    constexpr float radius = 57.5f;

    auto it = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (it == intendedVashjCorePasserLineup.end())
    {
        float mx = designatedLooter->GetPositionX();
        float my = designatedLooter->GetPositionY();
        float angle = atan2(my - centerY, mx - centerX);

        float targetX = centerX + radius * std::cos(angle);
        float targetY = centerY + radius * std::sin(angle);
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedVashjCorePasserLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        it = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    const Position& pos = it->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(
    Player* firstCorePasser, Unit* closestTrigger)
{
    if (!firstCorePasser || !closestTrigger)
        return false;

    auto itFirst = intendedVashjCorePasserLineup.find(firstCorePasser->GetGUID());
    if (itFirst == intendedVashjCorePasserLineup.end())
        return false;

    auto itSecond = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itSecond == intendedVashjCorePasserLineup.end())
    {
        float fx = itFirst->second.GetPositionX();
        float fy = itFirst->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - fx;
        float dy = closestTrigger->GetPositionY() - fy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX, targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = fx + dx * moveDist;
            targetY = fy + dy * moveDist;
        }
        else
        {
            targetX = fx + dx * farDistance;
            targetY = fy + dy * farDistance;
        }

        intendedVashjCorePasserLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itSecond = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    const Position& pos = itSecond->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(
    Player*, Player* firstCorePasser,
    Player* secondCorePasser, Unit* closestTrigger)
{
    if (!secondCorePasser || !closestTrigger)
        return false;

    bool needThirdPasser =
        (IsFirstCorePasserInPosition(firstCorePasser) &&
         firstCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needThirdPasser)
        return false;

    auto itSecond = intendedVashjCorePasserLineup.find(secondCorePasser->GetGUID());
    if (itSecond == intendedVashjCorePasserLineup.end())
        return false;

    auto itThird = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itThird == intendedVashjCorePasserLineup.end())
    {
        float sx = itSecond->second.GetPositionX();
        float sy = itSecond->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - sx;
        float dy = closestTrigger->GetPositionY() - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX, targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = sx + dx * moveDist;
            targetY = sy + dy * moveDist;
        }
        else
        {
            targetX = sx + dx * farDistance;
            targetY = sy + dy * farDistance;
        }

        intendedVashjCorePasserLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itThird = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    const Position& pos = itThird->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(
    Player*, Player* secondCorePasser,
    Player* thirdCorePasser, Unit* closestTrigger)
{
    if (!thirdCorePasser || !closestTrigger)
        return false;

    bool needFourthPasser =
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsThirdCorePasserInPosition(thirdCorePasser) &&
         thirdCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needFourthPasser)
        return false;

    auto itThird = intendedVashjCorePasserLineup.find(thirdCorePasser->GetGUID());
    if (itThird == intendedVashjCorePasserLineup.end())
        return false;

    auto itFourth = intendedVashjCorePasserLineup.find(bot->GetGUID());
    if (itFourth == intendedVashjCorePasserLineup.end())
    {
        float sx = itThird->second.GetPositionX();
        float sy = itThird->second.GetPositionY();

        float tx = closestTrigger->GetPositionX();
        float ty = closestTrigger->GetPositionY();

        float dx = tx - sx;
        float dy = ty - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        constexpr float nearTriggerDist = 1.5f;
        float targetX = tx - dx * nearTriggerDist;
        float targetY = ty - dy * nearTriggerDist;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedVashjCorePasserLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itFourth = intendedVashjCorePasserLineup.find(bot->GetGUID());
    }

    const Position& pos = itFourth->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

// The next four functions check if the respective passer is <= 2 yards of their intended
// position and are used to determine when the prior bot in the chain can pass the core
bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInPosition(Player* firstCorePasser)
{
    if (!firstCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(firstCorePasser->GetGUID());
    if (itSnap != intendedVashjCorePasserLineup.end())
    {
        float dist2d = firstCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInPosition(Player* secondCorePasser)
{
    if (!secondCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(secondCorePasser->GetGUID());
    if (itSnap != intendedVashjCorePasserLineup.end())
    {
        float dist2d = secondCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInPosition(Player* thirdCorePasser)
{
    if (!thirdCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(thirdCorePasser->GetGUID());
    if (itSnap != intendedVashjCorePasserLineup.end())
    {
        float dist2d = thirdCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInPosition(Player* fourthCorePasser)
{
    if (!fourthCorePasser)
        return false;

    auto itSnap = intendedVashjCorePasserLineup.find(fourthCorePasser->GetGUID());
    if (itSnap != intendedVashjCorePasserLineup.end())
    {
        float dist2d = fourthCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator(const uint32 instanceId)
{
    auto const& generators =
        GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    GeneratorInfo const* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator || bot->GetExactDist2d(generator) > 4.5f)
        return false;

    Item* core = bot->GetItemByEntry(Id(SscItems::ITEM_TAINTED_CORE));
    if (!core || bot->CanUseItem(core) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    const uint8 bagIndex = core->GetBagSlot();
    const uint8 slot = core->GetSlot();
    constexpr uint8 cast_count = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (core->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = core->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    const ObjectGuid item_guid = core->GetGUID();
    constexpr uint32 glyphIndex = 0;
    constexpr uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex;
    packet << slot;
    packet << cast_count;
    packet << spellId;
    packet << item_guid;
    packet << glyphIndex;
    packet << castFlags;
    packet << (uint32)TARGET_FLAG_GAMEOBJECT;
    packet << generator->GetGUID().WriteAsPacked();

    bot->GetSession()->HandleUseItemOpcode(packet);

    lastVashjCoreImbueAttempt.erase(instanceId);
    auto coreHandlers = GetCoreHandlers(botAI, bot);
    for (Player* handler : coreHandlers)
    {
        if (handler)
        {
            intendedVashjCorePasserLineup.erase(handler->GetGUID());
            lastVashjCoreInInventoryTime.erase(handler->GetGUID());
        }
    }

    return true;
}

// The standard "avoid aoe" strategy does work for Toxic Spores, but this method
// provides more buffer distance and limits the area in which bots can move
// so that they do not go down the stairs
bool LadyVashjAvoidToxicSporesAction::Execute(Event /*event*/)
{
    auto const& spores = GetAllSporeDropTriggers(bot);
    if (spores.empty())
        return false;

    constexpr float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (bot->GetExactDist2d(spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    const Position& vashjCenter = VASHJ_PLATFORM_CENTER_POSITION;
    constexpr float maxRadius = 60.0f;

    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);
    bool backwards = vashj->GetVictim() == bot;
    MovementPriority priority = backwards ?
        MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;

    return MoveTo(SSC_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  priority, true, backwards);
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& spores, Position const& vashjCenter,
    float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 40.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance;
         distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);
            float z = bot->GetPositionZ();

            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* spore : spores)
            {
                if (spore->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, z);

            bool pathSafe =
                IsPathSafeFromSpores(bot->GetPosition(), testPos, spores, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(
    Position const& start, Position const& end,
    std::vector<Unit*> const& spores, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* spore : spores)
        {
            float distToSpore = spore->GetExactDist2d(checkX, checkY);
            if (distToSpore < hazardRadius)
                return false;
        }
    }

    return true;
}

// When Toxic Sporebats spit poison, they summon "Spore Drop Trigger" NPCs
// that create the toxic pools
std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 50.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(SscNpcs::NPC_SPORE_DROP_TRIGGER), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            sporeDropTriggers.push_back(creature);
    }

    return sporeDropTriggers;
}

bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto const& spores =
        LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(bot);
    constexpr float toxicSporeRadius = 6.0f;

    // If Rogues are Entangled and either have Static Charge or
    // are near a spore, use Cloak of Shadows
    if (bot->getClass() == CLASS_ROGUE && bot->HasAura(Id(SscSpells::SPELL_ENTANGLE)))
    {
        bool nearSpore = false;
        for (Unit* spore : spores)
        {
            if (bot->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearSpore = true;
                break;
            }
        }
        if (bot->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)) || nearSpore)
        {
            if (botAI->CanCastSpell("cloak of shadows", bot))
                return botAI->CastSpell("cloak of shadows", bot);
        }
    }

    // The remainder of the logic is for Paladins to use Hand of Freedom
    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)) ||
            !PlayerbotAI::IsMelee(member))
        {
            continue;
        }

        bool nearToxicSpore = false;
        for (Unit* spore : spores)
        {
            if (member->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearToxicSpore = true;
                break;
            }
        }

        if (nearToxicSpore)
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankToxic = member;

            if (!anyToxic)
                anyToxic = member;
        }

        if (member->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE)))
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankStatic = member;

            if (!anyStatic)
                anyStatic = member;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        // Priority 1: Entangled in Toxic Spores (prefer main tank)
        Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
        if (toxicTarget && botAI->CanCastSpell("hand of freedom", toxicTarget))
            return botAI->CastSpell("hand of freedom", toxicTarget);

        // Priority 2: Entangled with Static Charge (prefer main tank)
        Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
        if (staticTarget && botAI->CanCastSpell("hand of freedom", staticTarget))
            return botAI->CastSpell("hand of freedom", staticTarget);
    }

    return false;
}
